#include <rebours/MAL/loader/detail/load_mach.hpp>
#include <rebours/MAL/loader/detail/abi_loaders.hpp>
#include <rebours/MAL/loader/detail/address_fixing.hpp>
#include <rebours/MAL/loader/detail/set_file_property.hpp>
#include <rebours/MAL/loader/file_utils.hpp>
#include <rebours/MAL/loader/to_string.hpp>
#include <rebours/MAL/loader/assumptions.hpp>
#include <rebours/MAL/loader/invariants.hpp>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <functional>
#include <tuple>
#include <unordered_set>
#include <iostream>

namespace loader { namespace detail { namespace {


uint64_t  read_uleb128(std::ifstream&  mach, uint64_t const  end_offset, std::string&  error_message)
{
    uint64_t  result = 0ULL;
    uint8_t  shift = 0U;
    while (true)
    {
        if ((uint64_t)mach.tellg() >= end_offset || shift >= 8ULL * sizeof(uint64_t))
        {
            error_message = "Wrong ULEB128";
            return std::numeric_limits<uint64_t>::max();
        }
        uint8_t const  byte = ::read_byte(mach);
        result |= ((uint64_t)(byte & 0x7fU) << shift);
        shift += 7U;
        if ((byte & 0x80U) == 0)
            break;
    }
    return result;
}

int64_t  read_sleb128(std::ifstream&  mach, uint64_t const  end_offset, std::string&  error_message)
{
    int64_t  result = 0ULL;
    uint8_t  shift = 0U;
    while (true)
    {
        if ((uint64_t)mach.tellg() >= end_offset || shift >= 8ULL * sizeof(uint64_t))
        {
            error_message = "Wrong SLEB128";
            return std::numeric_limits<int64_t>::max();
        }
        uint8_t const  byte = ::read_byte(mach);
        result |= ((uint64_t)(byte & 0x7fU) << shift);
        shift += 7U;
        if ((byte & 0x80U) == 0)
        {
            if ((byte & 0x40U) != 0)
                result |= (-1LL) << shift;
            break;
        }
    }
    return result;
}

std::string  process_LC_SEGMENT_64(std::ifstream&  mach, file_props_ptr  mach_props,
                                   address const  memory_base_shift,
                                   bool const  is_fixed_dynamic_library,
                                   load_props_mach&  load_props)
{
    skip_bytes(mach,16U); // We ignore segment name

    uint64_t const  memory_address = read_bytes_to_int64_t(mach,8U,mach_props->is_in_big_endian());
    if (memory_address % 0x1000ULL != 0ULL) // Is not aligned to page size?
        return "Virtual address of the loaded segment is not page aligned.";
    uint64_t const  size_in_memory = read_bytes_to_int64_t(mach,8U,mach_props->is_in_big_endian());

    address  fixed_memory_address = memory_base_shift + memory_address;
    if (load_props.root_file() != mach_props->path() && !is_fixed_dynamic_library)
        fixed_memory_address = load_props.first_free_vm_page(fixed_memory_address,
                                                             size_in_memory + memory_address % 0x1000ULL) +
                               memory_address % 0x1000ULL;

    uint64_t const  file_offset = read_bytes_to_int64_t(mach,8U,mach_props->is_in_big_endian());
    uint64_t const  size_in_file = read_bytes_to_int64_t(mach,8U,mach_props->is_in_big_endian());
    if (file_offset + size_in_file > file_size(mach_props->path()))
        return "The loaded segment goes beyond the loaded binary.";
    if (size_in_memory < size_in_file)
        return "The loaded segment has a smaller size in the memory than in the file.";

    skip_bytes(mach,4U); // We ignore max protection flags.
    uint32_t const  init_protection = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
    if (init_protection > 7U)
        return "Invalid section protection flags.";
    bool const  is_readable = init_protection & 1U;
    bool const  is_writeable = init_protection & 2U;
    bool const  is_executable = init_protection & 4U;

    uint32_t const  num_sections = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());

    uint32_t const  flags = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
    bool const  load_to_high_addresses = (flags & 1U) != 0U;
    bool const  load_to_fixed_address = (flags & 2U) != 0U;
    if (load_to_fixed_address && memory_address != fixed_memory_address)
        return "Cannot load the binary to fixed address as that space is already occupied.";
    bool const  no_relocations_needed = (flags & 4U) != 0U;
    (void)no_relocations_needed; // We ignore this flag.

    if (memory_address == 0ULL &&
            (mach_props->has_property_value(file_properties::file_type(),file_types::executable()) ||
             is_fixed_dynamic_library))
    {
        if (size_in_file > 0ULL)
            return "The zero-segment at the address 0 has non-zero size in the binary.";
        if (init_protection != 0U)
            return "The zero-segment has access rights.";
        if (num_sections > 0U)
            return "The zero-segment has sections.";

        return ""; // We skip the zero-segment.
    }

    section_file_header_ptr const  file_header(
            new section_file_header{
                    file_offset,
                    memory_address,
                    size_in_file,
                    size_in_memory,
                    0x1ULL,
                    0x1000ULL
                    }
            );

    (void)is_fixed_dynamic_library;

    std::shared_ptr<section_content> const  content{
            new section_content(size_in_memory,0U)
            };
    {
        std::ifstream::pos_type const  saved_file_pos = mach.tellg();
        mach.seekg(file_offset);
        uint64_t const  shift = load_to_high_addresses ? size_in_memory - size_in_file : 0ULL;
        mach.read(reinterpret_cast<char*>(&content->at(shift)),size_in_file);
        mach.seekg(saved_file_pos);
    }

    load_props.add_section(
            section_ptr{ new section{
                    fixed_memory_address,
                    fixed_memory_address + file_header->size_in_memory(),
                    content,
                    is_readable,
                    is_writeable,
                    is_executable,
                    mach_props->is_in_big_endian(),
                    true,
                    mach_props,
                    file_header
                    }}
            );

    load_props.add_address_fix(mach_props->path(),memory_address,fixed_memory_address);

    // Next we go through section in order to find init and fini functions

    for (uint32_t  i = 0U; i < num_sections; ++i)
    {
        uint64_t const  setion_begin_offset = mach.tellg();

//        std::string  section_name;
//        ::read_bytes(mach,16,section_name);
//        std::string  segment_name;
//        ::read_bytes(mach,16,segment_name);
        skip_bytes(mach,16ULL+16ULL); // Skip section and segment names

        address const  section_start_address = read_bytes_to_uint64_t(mach,8U,mach_props->is_in_big_endian());
        uint64_t const  section_size = read_bytes_to_uint64_t(mach,8U,mach_props->is_in_big_endian());

        skip_bytes(mach,4ULL+4ULL+4ULL+4ULL); // Skip offset, align, and relocations offset and count

        uint32_t const  section_flags = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());

        if ((section_flags & 0xffU) == 0x9U) // is it section S_MOD_INIT_FUNC_POINTERS?
        {
            for (address  adr = section_start_address; adr < section_start_address+section_size; adr += 8ULL)
            {
                address const  init_fn_adress = read_address_64bit(adr + memory_base_shift,load_props.sections_table());
                load_props.add_init_function(mach_props->path(),init_fn_adress + memory_base_shift);
            }
        }
        else if ((section_flags & 0xffU) == 0xaU) // is it section S_MOD_TERM_FUNC_POINTERS?
        {
            std::vector<address>  finis;
            for (address  adr = section_start_address; adr < section_start_address+section_size; adr += 8ULL)
            {
                address const  fini_fn_adress = read_address_64bit(adr + memory_base_shift,load_props.sections_table());
                finis.push_back(fini_fn_adress + memory_base_shift);
            }
            std::reverse(finis.begin(),finis.end());
            for (auto const& elem : finis)
                load_props.add_fini_function(mach_props->path(),elem);
        }
        else if ((section_flags & 0xffU) == 0x15U) // is it section S_THREAD_LOCAL_INIT_FUNCTION_POINTERS?
        {
            std::vector<address>  initialisers;
            for (address  adr = section_start_address; adr < section_start_address+section_size; adr += 8ULL)
            {
                address const  init_fn_adress = read_address_64bit(adr + memory_base_shift,load_props.sections_table());
                initialisers.push_back(init_fn_adress + memory_base_shift);
            }
            std::reverse(initialisers.begin(),initialisers.end());
            load_props.add_special_section(
                    mach_props->path(),
                    special_section_name::thread_local_storage_initialisers(),
                    special_section_properties_ptr{ new special_section::thread_local_storage_initialisers{
                            section_start_address + memory_base_shift,
                            section_start_address + memory_base_shift + section_size,
                            initialisers
                            }}
                    );
        }

        mach.seekg(setion_begin_offset + 2ULL*16ULL + 2ULL*8ULL + 8ULL*4ULL); // Skip this section
    }

    return "";
}

std::string  process_LC_LOAD_DYLIB(std::ifstream&  mach, file_props_ptr  mach_props,
                                   uint64_t const  command_offset,
                                   uint32_t const  command_size,
                                   load_props_mach&  load_props)
{
    bool  is_system_lib = false;
    std::string  lib_file_name;
    {
        uint32_t const  path_name_offset = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
        std::vector<char>  buffer(command_size - path_name_offset + 1ULL,0U);
        mach.seekg(command_offset + path_name_offset);
        mach.read(buffer.data(),buffer.size()-1ULL);
        std::string const  path_name = buffer.data();
        if (path_name.find("/usr/lib/system/") == 0ULL ||
            path_name.find("/usr/lib/libSystem") == 0ULL ||
            path_name.find("/usr/lib/libc++") == 0ULL ||
            path_name.find("/usr/lib/libobjc") == 0ULL ||
            path_name.find("/usr/lib/libDiagnostic") == 0ULL )
            is_system_lib = true;
        lib_file_name = parse_name_in_pathname(path_name);
    }

    bool  lib_loaded = false;
    if (std::find(load_props.ignored_files().cbegin(),load_props.ignored_files().cend(),lib_file_name)
            == load_props.ignored_files().cend())
    {
        for (auto const& search_dir : load_props.search_dirs())
        {
            std::string const raw_lib_pathname = concatenate_file_paths(search_dir,lib_file_name);
            if (!file_exists(raw_lib_pathname))
                continue;
            std::string const lib_pathname = normalise_path(absolute_path(raw_lib_pathname));
            if (load_props.files_table()->count(lib_pathname) == 0U)
            {
                std::string const  err_message = detail::load_mach(lib_pathname,load_props);
                if (!err_message.empty())
                {
                    if (err_message == NOT_MACH_FILE())
                        return "The referenced library '" + lib_pathname + "' is not a Mach-O file.";
                    return err_message;
                }
            }
            load_props.add_file_dependence(mach_props->path(),lib_pathname);
            if (is_system_lib)
                load_props.add_system_library(lib_pathname);
            load_props.add_library(mach_props->path(),lib_pathname);
            lib_loaded = true;
            break;
        }
    }
    if (!lib_loaded)
    {
        load_props.add_skipped_file(lib_file_name);
        load_props.add_file_dependence(mach_props->path(),lib_file_name);
        load_props.add_library(mach_props->path(),lib_file_name);
    }

    return "";
}

std::string  process_LC_SYMTAB(std::ifstream&  mach, file_props_ptr  mach_props, load_props_mach&  load_props)
{
    uint32_t const  symbol_table_offset = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
    uint32_t const  num_symbols = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
    if (symbol_table_offset + 16U * num_symbols > file_size(mach_props->path()))
        return "Symbol table goes beyond the end of the file.";

    uint32_t const  string_table_offset = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
    uint32_t const  string_table_size = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
    if (string_table_offset + string_table_size > file_size(mach_props->path()))
        return "String table goes beyond the end of the file.";

    std::map<uint64_t,std::string>  strings;
    {
        mach.seekg(string_table_offset);
        while (mach.tellg() < string_table_offset + string_table_size)
        {
            uint64_t const  offset = (uint64_t)mach.tellg() - (uint64_t)string_table_offset;
            strings.insert({offset,read_bytes_as_null_terminated_string(mach)});
        }
        if (mach.tellg() != string_table_offset + string_table_size)
            return "Inconsistency between actual and expected size of the string table.";
    }

    mach.seekg(symbol_table_offset);
    for (uint32_t  i = 0U; i < num_symbols; ++i)
    {
        uint32_t const  string_offset = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
        if (string_offset > string_table_size)
            return "Symbol's offset goes beyond the string table.";
        uint8_t const  type_flags = ::read_byte(mach);
        uint8_t const  section_number = ::read_byte(mach);
        uint16_t const  description = read_bytes_to_uint16_t(mach,2U,mach_props->is_in_big_endian());
        uint64_t const  value = read_bytes_to_uint64_t(mach,8U,mach_props->is_in_big_endian());

        if ((type_flags & 0xe0U) != 0x0U) // Is is a symbolic debugging entry?
            continue;

        uint64_t  real_string_offset;
        {
            if ((type_flags & 0xeU) == 0xaU)
            {
                if (strings.count(value) == 0ULL)
                    return "Indirect symbol's offset does not reference any string in the string table.";
                real_string_offset = value;
            }
            else
            {
                if (strings.count(string_offset) == 0ULL)
                    return "Symbol's offset does not reference any string in the string table.";
                real_string_offset = string_offset;
            }
        }

        load_props.add_symbol({
                mach_props->path(),
                i,
                strings.at(real_string_offset),
                type_flags,
                section_number,
                description,
                value
                });
    }

    return "";
}

std::string  process_LC_DYSYMTAB(std::ifstream&  mach, file_props_ptr  mach_props, load_props_mach&  load_props)
{
    skip_bytes(mach,4U); // We ignore the start index of local symbols.
    skip_bytes(mach,4U); // We ignore the number of local symbols.

    uint32_t const  extern_symbol_start_index = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
    uint32_t const  num_extern_symbols = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());

    if (extern_symbol_start_index + num_extern_symbols > load_props.get_max_symbol_id(mach_props->path()) + 1U)
        return "Ordinal of an extern symbol is greater than the max ordinal in the symbol table.";

    uint32_t const  undef_symbol_start_index = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
    uint32_t const  num_undef_symbols = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());

    if (undef_symbol_start_index + num_undef_symbols > load_props.get_max_symbol_id(mach_props->path()) + 1U)
        return "Ordinal of an undefined symbol is greater than the max ordinal in the symbol table.";

//    skip_bytes(mach,4U+4U); // We ignore the table of contents.
//    skip_bytes(mach,4U+4U); // We ignore the module table.
//    skip_bytes(mach,4U+4U); // We ignore the referenced symbol table.

//    uint32_t const  indirect_symbol_table_offset = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
//    uint32_t const  num_indirect_symbol_table_entries = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());

    for (uint32_t  i = 0U; i < num_extern_symbols; ++i)
    {
        mach_symbol_id&  symbol = load_props.get_symbol(mach_props->path(),extern_symbol_start_index + i);
        if ((symbol.type_flags() & 0x1U) == 0U)
            return "The external exported symbol is undefined or unaccessible.";
        if ((symbol.type_flags() & 0xeU) == 0U)
            return "The external exported symbol defined in this binary does not appear in any of its sections.";

        symbol.set_address(
                (address)symbol.value() +
                ((symbol.type_flags() & 0xeU) == 0x2U ?
                        0ULL :
                        (mach_props->path() == load_props.root_file() ?
                                0ULL :
                                load_props.fixed_base_address_for(mach_props->path())
                                )
                        )
                );
    }

    for (uint32_t  i = 0U; i < num_undef_symbols; ++i)
    {
        mach_symbol_id&  symbol = load_props.get_symbol(mach_props->path(),undef_symbol_start_index + i);
        if ((symbol.type_flags() & 0x1U) == 0U)
            return "The external imported symbol is undefined or unaccessible.";
        if ((symbol.type_flags() & 0xeU) != 0U)
            return "The external imported symbol is marked as defined in this binary.";
    }

    return "";
}

std::string  apply_rebase(address const  rebase_address, int64_t const  shift,
                          uint8_t const  rebase_type, std::string const&  description,
                          load_props_mach&  load_props)
{

    std::string  error_message;
    switch (rebase_type)
    {
    case 1U:  // REBASE_TYPE_POINTER
        {
            address const  orig_address = read_address_64bit(rebase_address,load_props.sections_table());
            address const  new_address = (uint64_t)((int64_t)orig_address + shift);
            error_message = write_address(rebase_address,new_address,load_props.sections_table());
            if (!error_message.empty())
                return error_message;
            load_props.add_performed_relocation({
                    rebase_address,
                    8ULL,
                    "REBASE_TYPE_POINTER",
                    new_address,
                    orig_address,
                    "",
                    description
                    });
        }
        break;
    case 2U:  // REBASE_TYPE_TEXT_ABSOLUTE32
        return "REBASE_TYPE_TEXT_ABSOLUTE32 is NOT IMPLEMENTED YET!";
        break;
    case 3U:  // REBASE_TYPE_TEXT_PCREL32
        return "REBASE_TYPE_TEXT_PCREL32 is NOT IMPLEMENTED YET!";
        break;
    default:
        return "Dad rebase type";
    }

    return "";
}

std::string  perform_rebase(uint32_t const  offset, uint32_t const  size,
                                    std::ifstream&  mach, file_props_ptr  mach_props,
                                    load_props_mach&  load_props)
{
    if (offset == 0U || size == 0U)
        return "Rebase info is missing.";
    if (offset + size > file_size(mach_props->path()))
        return "Rebase info goes beyond the end of the binary file.";

    int64_t const  shift = (int64_t)load_props.fixed_base_address_for(mach_props->path()) -
                           (int64_t)load_props.prefered_base_address_for(mach_props->path());

    uint8_t rebase_type = 0U;
    uint64_t segment_index = 0ULL;
    address  rebase_address = 0ULL;
    std::string  description;
    std::string  error_message;

    mach.seekg(offset);
    while (mach.tellg() < offset + size)
    {
        uint8_t const  instruction = ::read_byte(mach);
        uint8_t const  opcode = instruction & 0xf0U;
        uint8_t const  arg = instruction & 0xfU;

        switch (opcode)
        {
        case 0x00U: // REBASE_OPCODE_DONE
            description.clear();
            break;
        case 0x10U: // REBASE_OPCODE_SET_TYPE_IMM: Values are: 1 = POINTER, 2 = TEXT_ABSOLUTE32, 3 = TEXT_PCREL32
            description += std::string(description.empty() ? "" : ", ") + "REBASE_OPCODE_SET_TYPE_IMM";
            rebase_type = arg;
            break;
        case 0x20U: // REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
            {
                description += std::string(description.empty() ? "" : ", ") + "REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB";
                segment_index = arg;
                uint64_t const  offset_in_segment = read_uleb128(mach,offset + size,error_message);
                if (!error_message.empty())
                    return error_message + " in the extra data of the REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB instruction.";
                rebase_address = load_props.fixed_address_for(mach_props->path(),segment_index) + offset_in_segment;
            }
            break;
        case 0x30U: // REBASE_OPCODE_ADD_ADDR_ULEB:
            {
                description += std::string(description.empty() ? "" : ", ") + "REBASE_OPCODE_ADD_ADDR_ULEB";
                uint64_t const  shift = read_uleb128(mach,offset + size,error_message);
                if (!error_message.empty())
                    return error_message + " in the extra data of the REBASE_OPCODE_ADD_ADDR_ULEB instruction.";
                rebase_address += shift;
            }
            break;
        case 0x40U: // REBASE_OPCODE_ADD_ADDR_IMM_SCALED:
            description += std::string(description.empty() ? "" : ", ") + "REBASE_OPCODE_ADD_ADDR_IMM_SCALED";
            rebase_address += arg * 8ULL;
            break;
        case 0x50U: // REBASE_OPCODE_DO_REBASE_IMM_TIMES:
            description += std::string(description.empty() ? "" : ", ") + "REBASE_OPCODE_DO_REBASE_IMM_TIMES";
            for (uint8_t  i = 0U; i < arg; ++i)
            {
                error_message = apply_rebase(rebase_address,shift,rebase_type,description,load_props);
                if (!error_message.empty())
                    return error_message + " of the REBASE_OPCODE_DO_REBASE_IMM_TIMES instruction.";

                rebase_address += 8ULL;
            }
            description.clear();
            break;
        case 0x60U: // REBASE_OPCODE_DO_REBASE_ULEB_TIMES:
            {
                description += std::string(description.empty() ? "" : ", ") + "REBASE_OPCODE_DO_REBASE_ULEB_TIMES";
                uint64_t const  n = read_uleb128(mach,offset + size,error_message);
                if (!error_message.empty())
                    return error_message + " in the extra data of the REBASE_OPCODE_DO_REBASE_ULEB_TIMES instruction.";
                for (uint64_t i = 0ULL; i < n; ++i)
                {
                    error_message = apply_rebase(rebase_address,shift,rebase_type,description,load_props);
                    if (!error_message.empty())
                        return error_message + " of the REBASE_OPCODE_DO_REBASE_ULEB_TIMES instruction.";

                    rebase_address += 8ULL;
                }
                description.clear();
            }
            break;
        case 0x70U: // REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB:
            {
                description += std::string(description.empty() ? "" : ", ") + "REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB";
                error_message = apply_rebase(rebase_address,shift,rebase_type,description,load_props);
                if (!error_message.empty())
                    return error_message + " of the REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB instruction.";

                uint64_t const  shift = read_uleb128(mach,offset + size,error_message);
                if (!error_message.empty())
                    return error_message + " in the extra data of the REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB instruction.";
                rebase_address += shift + 8ULL;
                description.clear();
            }
            break;
        case 0x80U: // REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB:
            {
                description += std::string(description.empty() ? "" : ", ") + "REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB";
                uint64_t const  count = read_uleb128(mach,offset + size,error_message);
                if (!error_message.empty())
                    return error_message + " (the count) in the extra data of the REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB instruction.";
                uint64_t const  skip = read_uleb128(mach,offset + size,error_message);
                if (!error_message.empty())
                    return error_message + " (the skip) in the extra data of the REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB instruction.";
                for (uint64_t i = 0ULL; i < count; ++i)
                {
                    error_message = apply_rebase(rebase_address,shift,rebase_type,description,load_props);
                    if (!error_message.empty())
                        return error_message + " of the REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB instruction.";

                    rebase_address += skip + 8ULL;
                }
                description.clear();
            }
            break;
        default:
            return "Unknown rebase instruction reached.";
        }
    }
    if (mach.tellg() > offset + size)
        return "Rebase info goes beyond the end of the binary file.";

    return "";
}

std::string  apply_binding(uint64_t const  dylib_index, uint64_t const  segment_index,
                           int64_t const  addend, address const  bind_address,
                           std::string const&  symbol_name, uint8_t const  symbol_flags,
                           uint8_t const  reloc_type, std::string const&  description,
                           bool const  is_weak_import,
                           file_props_ptr  mach_props, load_props_mach&  load_props)
{
    (void)segment_index;
    (void)symbol_flags;

    std::string  library;
    {
        if (dylib_index == 0ULL)
            library = mach_props->path();
        else if ((int64_t)dylib_index == -1LL)
            library = load_props.root_file();
        else if ((int64_t)dylib_index == -2LL)
            library.clear(); // Flat symbol lookup!
        else if ((int64_t)dylib_index < 0LL)
            return "Cannot resolve binding, since the library index has unknown negative value.";
        else if (!load_props.has_library(mach_props->path(),dylib_index - 1ULL))
            return "Cannot resolve binding, since the library index does not reference any library in the table.";
        else
            library = load_props.get_library(mach_props->path(),dylib_index - 1ULL);
    }
    if (load_props.is_skipped_file(library))
    {
        load_props.add_skipped_relocation({
                bind_address,
                8ULL,
                reloc_type == 1U ? "BIND_TYPE_POINTER" :
                reloc_type == 2U ? "BIND_TYPE_TEXT_ABSOLUTE32" :
                reloc_type == 3U ? "BIND_TYPE_TEXT_PCREL32" :
                                   "UNKNOWN",
                0ULL,
                read_address_64bit(bind_address,load_props.sections_table()),
                mach_symbol_id{library,0U,symbol_name,0U,0U,0U,0ULL}.full_name(),
                "The library was skiped, so the address of the symbol is unknown; " + description
                });
        return "";
    }
    if (!load_props.has_symbol(library,symbol_name))
    {
        load_props.add_skipped_relocation({
                bind_address,
                8ULL,
                reloc_type == 1U ? "BIND_TYPE_POINTER" :
                reloc_type == 2U ? "BIND_TYPE_TEXT_ABSOLUTE32" :
                reloc_type == 3U ? "BIND_TYPE_TEXT_PCREL32" :
                                   "UNKNOWN",
                0ULL,
                read_address_64bit(bind_address,load_props.sections_table()),
                mach_symbol_id{library,0U,symbol_name,0U,0U,0U,0ULL}.full_name(),
                "The address of the imported symbol is unknown; " + description
                });
        return "";
    }

    mach_symbol_id const& symbol = load_props.get_symbol(library,symbol_name);
    if (!symbol.is_address_set())
    {
        if (is_weak_import)
        {
            load_props.add_performed_relocation({
                    bind_address,
                    8ULL,
                    reloc_type == 1U ? "BIND_TYPE_POINTER" :
                    reloc_type == 2U ? "BIND_TYPE_TEXT_ABSOLUTE32" :
                    reloc_type == 3U ? "BIND_TYPE_TEXT_PCREL32" :
                                       "UNKNOWN",
                    0ULL,
                    read_address_64bit(bind_address,load_props.sections_table()),
                    symbol.full_name(),
                    "The weak unresolved symbol is automatically set to NULL; " + description
                    });
            return "";
        }
        if (!library.empty())
            return LOAD_MACH_ERROR(
                    "Bound symbol " << symbol.full_name()
                    << " of library '" << library
                    <<  "' without a proper address set. "
                    << "This is a result "
                    );
        load_props.add_skipped_relocation({
                bind_address,
                8ULL,
                reloc_type == 1U ? "BIND_TYPE_POINTER" :
                reloc_type == 2U ? "BIND_TYPE_TEXT_ABSOLUTE32" :
                reloc_type == 3U ? "BIND_TYPE_TEXT_PCREL32" :
                                   "UNKNOWN",
                0ULL,
                read_address_64bit(bind_address,load_props.sections_table()),
                symbol.symbol_name(),
                "Flat look-up for the symbol's name has failed. It perhaps belonds to a skipped library. " + description
                });
        return "";
    }

    std::string  error_message;
    switch (reloc_type)
    {
    case 1U:  // BIND_TYPE_POINTER
        error_message = write_address(bind_address,symbol.address() + addend,load_props.sections_table());
        if (!error_message.empty())
            return error_message;
        load_props.add_performed_relocation({
                bind_address,
                8ULL,
                "BIND_TYPE_POINTER",
                symbol.address() + addend,
                read_address_64bit(bind_address,load_props.sections_table()),
                symbol.full_name(),
                description
                });
        break;
    case 2U:  // BIND_TYPE_TEXT_ABSOLUTE32
        error_message = write_uint32_t(bind_address,(uint32_t)(symbol.address() + addend),load_props.sections_table());
        if (!error_message.empty())
            return error_message;
        load_props.add_performed_relocation({
                bind_address,
                4ULL,
                "BIND_TYPE_TEXT_ABSOLUTE32",
                (uint32_t)(symbol.address() + addend),
                read_address_64bit(bind_address,load_props.sections_table()),
                symbol.full_name(),
                description
                });
        break;
    case 3U:  // BIND_TYPE_TEXT_PCREL32
        {
            uint32_t const  value = (uint32_t)(symbol.address() + addend - (bind_address + 4ULL));
            error_message = write_uint32_t(bind_address,(uint32_t)(symbol.address() + addend),load_props.sections_table());
            if (!error_message.empty())
                return error_message;
            load_props.add_performed_relocation({
                    bind_address,
                    4ULL,
                    "BIND_TYPE_TEXT_PCREL32",
                    value,
                    read_address_64bit(bind_address,load_props.sections_table()),
                    symbol.full_name(),
                    description
                    });
        }
        break;
    default:
        return "Dad bind type ";
    }

    return "";
}

std::string  perform_symbol_binding(uint32_t const  offset, uint32_t const  size,
                                    bool const  is_weak_import, std::ifstream&  mach,
                                    file_props_ptr  mach_props,
                                    load_props_mach&  load_props)
{
    if (offset == 0U || size == 0U)
        return "Relocations are missing.";
    if (offset + size > file_size(mach_props->path()))
        return "Relocations goes beyond the end of the binary file.";

    uint64_t dylib_index = 0ULL;
    uint64_t segment_index = 0ULL;
    int64_t addend = 0LL;
    address  bind_address = 0ULL;
    std::string  symbol_name;
    uint8_t symbol_flags = 0U;
    uint8_t reloc_type = 1U;
    uint64_t count = 0ULL;
    uint64_t skip = 0ULL;
    std::string  description;
    std::string  error_message;

    mach.seekg(offset);
    while (mach.tellg() < offset + size)
    {
        uint8_t const  instruction = ::read_byte(mach);
        uint8_t const  opcode = instruction & 0xf0U;
        uint8_t const  arg = instruction & 0xfU;

        switch (opcode)
        {
        case 0x00U: // BIND_OPCODE_DONE: Finished defining a symbol.
            description.clear();
            break;
        case 0x10U: // BIND_OPCODE_SET_DYLIB_ORDINAL_IMM: Set the library ordinal of the current symbol to the imm operand.
            description += std::string(description.empty() ? "" : ", ") + "BIND_OPCODE_SET_DYLIB_ORDINAL_IMM";
            dylib_index = arg;
            if (!load_props.has_library(mach_props->path(),dylib_index - 1ULL))
                return "Argument of BIND_OPCODE_SET_DYLIB_ORDINAL_IMM instruction does not reference any library.";
            break;
        case 0x20U: // BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB: Same as above, but the library ordinary is read from the unsigned LEB128-encoded extra data.
            description += std::string(description.empty() ? "" : ", ") + "BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB";
            dylib_index = read_uleb128(mach,offset + size,error_message);
            if (!error_message.empty())
                return error_message + " in the extra data of the BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB instruction.";
            if (!load_props.has_library(mach_props->path(),dylib_index - 1ULL))
                return "Argument of BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB instruction does not reference any library.";
            break;
        case 0x30U: // BIND_OPCODE_SET_DYLIB_SPECIAL_IMM: Same as above, but the ordinary as set as negative of imm.
                    //      Typical values are:  0 = SELF, -1 = MAIN_EXECUTABLE, -2 = FLAT_LOOKUP
            description += std::string(description.empty() ? "" : ", ") + "BIND_OPCODE_SET_DYLIB_SPECIAL_IMM";
            dylib_index = (arg == 0U) ? 0ULL : (uint64_t)((int8_t)0xf0 | (int8_t)arg);
            if ((int64_t)dylib_index > 0LL && !load_props.has_library(mach_props->path(),dylib_index - 1ULL))
                return "Argument of BIND_OPCODE_SET_DYLIB_SPECIAL_IMM instruction does not reference any library.";
            break;
        case 0x40U: // BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM	Set flags of the symbol in imm, and the symbol name as a C string in the extra data.
                    //      The flags are: 1 = WEAK_IMPORT, 8 = NON_WEAK_DEFINITION
            description += std::string(description.empty() ? "" : ", ") + "BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM";
            symbol_flags = arg;
            symbol_name = ::read_bytes_as_null_terminated_string(mach);
            if (symbol_name.empty())
                return "The symbol in the extra data of the SET_SEGMENT_AND_OFFSET_ULEB is empty.";
            break;
        case 0x50U: // BIND_OPCODE_SET_TYPE_IMM:  Set the type of symbol as imm.
                    //      Known values are: 1 = POINTER, 2 = TEXT_ABSOLUTE32, 3 = TEXT_PCREL32
            description += std::string(description.empty() ? "" : ", ") + "BIND_OPCODE_SET_TYPE_IMM";
            reloc_type = arg;
            break;
        case 0x60U: // BIND_OPCODE_SET_ADDEND_SLEB: Set the addend of the symbol as the signed LEB128-encoded extra data. Usage unknown.
            description += std::string(description.empty() ? "" : ", ") + "BIND_OPCODE_SET_ADDEND_SLEB";
            addend = read_sleb128(mach,offset + size,error_message);
            if (!error_message.empty())
                return error_message + " in the extra data of the BIND_OPCODE_SET_ADDEND_SLEB instruction.";
            break;
        case 0x70U: // BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB: Set that the symbol can be found in the imm-th segment, at an offset found in the extra data.
            {
                description += std::string(description.empty() ? "" : ", ") + "BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB";
                segment_index = arg;
                uint64_t const  offset_in_segment = read_uleb128(mach,offset + size,error_message);
                if (!error_message.empty())
                    return error_message + " in the extra data of the BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB relocation instruction.";
                bind_address = load_props.fixed_address_for(mach_props->path(),segment_index) + offset_in_segment;
            }
            break;
        case 0x80U: // BIND_OPCODE_ADD_ADDR_ULEB: Increase the offset (as above) by the ULEB128-encoded extra data.
            {
                description += std::string(description.empty() ? "" : ", ") + "BIND_OPCODE_ADD_ADDR_ULEB";
                uint64_t const  shift = read_uleb128(mach,offset + size,error_message);
                if (!error_message.empty())
                    return error_message + " in the extra data of the BIND_OPCODE_ADD_ADDR_ULEB relocation instruction.";
                bind_address += shift;
            }
            break;
        case 0x90U: // BIND_OPCODE_DO_BIND: Define a symbol from the gathered information. Increase the offset by 4 (or 8 on 64-bit targets) after this operation.
            {
                description += std::string(description.empty() ? "" : ", ") + "BIND_OPCODE_DO_BIND";
                error_message = apply_binding(dylib_index,segment_index,addend,bind_address,symbol_name,
                                              symbol_flags,reloc_type,description,is_weak_import,
                                              mach_props,load_props);
                if (!error_message.empty())
                    return error_message + " of the BIND_OPCODE_DO_BIND instruction.";

                bind_address += 8ULL;
                description.clear();
            }
            break;
        case 0xA0U: // BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB: Same as above, but besides the 4 byte increment, the extra data is also added.
            {
                description += std::string(description.empty() ? "" : ", ") + "BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB";
                error_message = apply_binding(dylib_index,segment_index,addend,bind_address,symbol_name,
                                              symbol_flags,reloc_type,description,is_weak_import,
                                              mach_props,load_props);
                if (!error_message.empty())
                    return error_message + " of the BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB instruction.";

                uint64_t const  shift = read_uleb128(mach,offset + size,error_message);
                if (!error_message.empty())
                    return error_message + " in the extra data of the BIND_OPCODE_ADD_ADDR_ULEB relocation instruction.";
                bind_address += shift;
                description.clear();
            }
            break;
        case 0xB0U: // BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED: Same as DO_BIND, but an extra imm*4 bytes is also added.
            {
                description += std::string(description.empty() ? "" : ", ") + "BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED";
                error_message = apply_binding(dylib_index,segment_index,addend,bind_address,symbol_name,
                                              symbol_flags,reloc_type,description,is_weak_import,
                                              mach_props,load_props);
                if (!error_message.empty())
                    return error_message + " of the BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED instruction.";

                bind_address += arg * 8ULL + 8ULL;
                description.clear();
            }
            break;
        case 0xC0U: // BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB: This is a very complex operation. Two unsigned LEB128-encoded numbers are read off from
                    // the extra data. The first is the count of symbols to be added, and the second is the bytes to skip after a symbol is added.
                    //      In pseudocode, all it does is: for i = 1 to count do { define symbol; offset += 4 + skip; }
            description += std::string(description.empty() ? "" : ", ") + "BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB";
            count = read_uleb128(mach,offset + size,error_message);
            if (!error_message.empty())
                return error_message + " (the count) in the extra data of the BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB relocation instruction.";
            skip = read_uleb128(mach,offset + size,error_message);
            if (!error_message.empty())
                return error_message + " (the skip) in the extra data of the BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB relocation instruction.";
            for (uint64_t i = 0U; i < count; ++i)
            {
                error_message = apply_binding(dylib_index,segment_index,addend,bind_address,symbol_name,
                                              symbol_flags,reloc_type,description,is_weak_import,
                                              mach_props,load_props);
                if (!error_message.empty())
                    return error_message + " of the BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB instruction.";

                bind_address += skip + 8ULL;
            }
            description.clear();
            break;
        default:
            return "Unknown relocation instruction reached.";
        }
    }
    if (mach.tellg() > offset + size)
        return "Relocations goes beyond the end of the binary file.";

    return "";
}

}}}

namespace loader { namespace detail {


std::string  load_mach_64bit(std::ifstream&  mach, file_props_ptr  mach_props,
                             uint32_t const  num_load_commnads,
                             uint32_t const  num_bytes_of_load_commnads,
                             bool const  is_fixed_dynamic_library,
                             load_props_mach&  load_props)
{
    ASSUMPTION( mach.is_open() );
    ASSUMPTION( mach_props->num_address_bits() == 64U );

    bool const  is_it_load_of_root_binary = load_props.root_file() == mach_props->path();

    address  memory_base_shift = 0ULL;
    if (load_props.root_file() != mach_props->path() && !is_fixed_dynamic_library)
    {
        address const  system_lib_start = 0x7fff00000000ULL;
        if (load_props.is_system_library(mach_props->path()))
        {
            memory_base_shift = load_props.first_highest_free_vm_page();
            if (memory_base_shift < system_lib_start)
                memory_base_shift = system_lib_start;
        }
        else
            memory_base_shift = load_props.first_free_vm_page_before(system_lib_start);
    }
    else if (mach_props->has_property_value(file_properties::file_type(),file_types::library()))
    {
        memory_base_shift = 0x100000000ULL;
    }

    uint32_t  bind_off = 0U;
    uint32_t  bind_size = 0U;
    uint32_t  lazy_bind_off = 0U;
    uint32_t  lazy_bind_size = 0U;
    uint32_t  weak_bind_off = 0U;
    uint32_t  weak_bind_size = 0U;
//    uint32_t  export_off = 0U;
//    uint32_t  export_size = 0U;
    bool  dyld_info_loaded = false;
    bool  segments_loaded = false;
    bool  symtab_loaded = false;
    bool  main_ptr_loaded = false;

    uint64_t const  commands_begin_offset = mach.tellg();
    for (uint32_t i = 0U; i < num_load_commnads; ++i)
    {
        uint64_t const  command_offset = mach.tellg();

        uint32_t const  command_type = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
        uint32_t const  command_size = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());

        if (command_size % 8U != 0U)
            return "Reached a load command which is not 8-byte aligned.";

        std::string  error_message = "";
        switch (command_type)
        {
        case 0x2U:  // LC_SYMTAB: link-edit stab symbol table info
            error_message = process_LC_SYMTAB(mach,mach_props,load_props);
            break;
        case 0x22U: // LC_DYLD_INFO: compressed dyld information
        case 0x80000022U: // LC_DYLD_INFO_ONLY: compressed dyld information only
            {
                if (symtab_loaded || main_ptr_loaded || !segments_loaded)
                    return "Wrong order of load commands (LC_DYLD_INFO should go before LC_DYSYMTAB and LC_MAIN) due to rebase.";
                if (!segments_loaded)
                    return "Wrong order of load commands (LC_DYLD_INFO should go after LC_SEGMENT_64).";
                if (dyld_info_loaded)
                    return "Dynamic info load command appears more than once in the binary.";
                uint32_t const  rebase_off = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
                uint32_t const  rebase_size = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
                bind_off = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
                bind_size = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
                weak_bind_off = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
                weak_bind_size = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
                lazy_bind_off = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
                lazy_bind_size = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
//                export_off = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
//                export_size = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
                dyld_info_loaded = true;

                if (load_props.fixed_base_address_for(mach_props->path()) != load_props.prefered_base_address_for(mach_props->path())
                        && rebase_off != 0U)
                {
                    std::string  error_message = perform_rebase(rebase_off,rebase_size,mach,mach_props,load_props);
                    if (!error_message.empty())
                        return error_message;
                }
            }
            break;
        case 0x80000028U: // LC_MAIN: the address of main()
            if (!segments_loaded)
                return "Wrong order of load commands (LC_MAIN must be after LC_SEGMENT_64).";
            if (main_ptr_loaded)
                return "The binary contains more than one LC_MAIN load command.";
            if (is_it_load_of_root_binary && mach_props->has_property_value(file_properties::file_type(),file_types::executable()))
            {
                uint64_t const  main_offset = read_bytes_to_uint64_t(mach,8U,mach_props->is_in_big_endian());
                uint64_t const  main_address = load_props.fixed_base_address_for(mach_props->path()) + main_offset;
                if (!find_section(main_address,load_props.sections_table()).operator bool())
                    return "The address of the main function does not point to any of the loaded segments.";
                load_props.set_entry_point(main_address);
                load_props.add_init_function(mach_props->path(),main_address);
            }
            else
                return "Dynamic library with LC_MAIN load command.";
            main_ptr_loaded = true;
            break;
        case 0x24U: // LC_VERSION_MIN_MACOSX: build for MacOSX min OS version
            {
                uint32_t const  version = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
                std::ostringstream  ostr;
                ostr << (version >> 16U) << "." << ((version & 0xff00) >> 8U) << "." << (version & 0xff);
                std::string const  version_string = ostr.str();
                set_file_property(mach_props->property_map(), file_properties::os_version(),version_string);
            }
            break;
        case 0xbU:  // LC_DYSYMTAB: dynamic link-edit symbol table info
            if (!segments_loaded)
                return "Wrong order of load commands (LC_DYSYMTAB must be after LC_SEGMENT_64).";
            error_message = process_LC_DYSYMTAB(mach,mach_props,load_props);
            symtab_loaded = true;
            break;
        case 0xcU:  // LC_LOAD_DYLIB: load a dynamically linked shared library
            error_message = process_LC_LOAD_DYLIB(mach,mach_props,command_offset,command_size,load_props);
            break;
        case 0xeU:  // LC_LOAD_DYLINKER: load a dynamic linker
            {
                uint32_t const  path_name_offset = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
                std::vector<char>  buffer(command_size - path_name_offset + 1ULL,0U);
                mach.seekg(command_offset + path_name_offset);
                mach.read(buffer.data(),buffer.size()-1ULL);
                std::string const  path_name = buffer.data();
                if (path_name != abi_loaders::darwin())
                    return "Unknown ABI loader.";
                set_file_property(mach_props->property_map(), file_properties::abi_loader(), path_name);
            }
            break;
        case 0x19U: // LC_SEGMENT_64: 64-bit segment of the file to be mapped
            if (dyld_info_loaded || symtab_loaded || main_ptr_loaded)
                return "Wrong order of load commands (LC_SEGMENT_64 should go first).";
            error_message = process_LC_SEGMENT_64(mach,mach_props,memory_base_shift,
                                                  is_fixed_dynamic_library,load_props);
            segments_loaded = true;
            break;
        case 0x1aU: // LC_ROUTINES_64: 64-bit image routines
            {
                uint64_t const init_fn_address = read_bytes_to_uint64_t(mach,8U,mach_props->is_in_big_endian());
                uint64_t const init_fn_module = read_bytes_to_uint64_t(mach,8U,mach_props->is_in_big_endian());
                // 'init_fn_address' is the virtual memory address of the initialization function.
                // 'init_fn_module' is the index into the module table that the init routine is defined in.
                // Unfortunatelly, we have no test example for this command, so it is left
                // unimplemented ...
                LOAD_MACH_WARNING("The load command LC_ROUTINES_64 is NOT IMPLEMENTED YET!");
                (void)init_fn_address;
                (void)init_fn_module;
            }
            break;
        case 0x26U: // LC_FUNCTION_STARTS : compressed table of function start addresses
            {
                uint32_t const  data_offset = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
                uint32_t const  data_size = read_bytes_to_uint32_t(mach,4U,mach_props->is_in_big_endian());
                mach.seekg(data_offset);
                uint64_t  fn_address = load_props.fixed_base_address_for(mach_props->path());
                while ((uint64_t)mach.tellg() < data_offset + data_size)
                {
                    uint64_t const  shift = read_uleb128(mach,data_offset + data_size,error_message);
                    if (!error_message.empty())
                        return error_message + " when parsing LC_FUNCTION_STARTS load command.";
                    if (shift == 0ULL)
                        break;
                    fn_address += shift;
                    // Well we computed the address of a function 'fn_address',
                    // but it seems like we do not need this info afterall, since
                    // this is actually the list of all functions. We are looking
                    // only for init and fini functions. We do not know which are
                    // which...
                }
            }
            break;
        default:
            break;
        }
        if (!error_message.empty())
            return error_message;

        mach.seekg(command_offset + command_size);
    }
    if ((uint64_t)mach.tellg() > commands_begin_offset + num_bytes_of_load_commnads)
        return "Load commands went beyond their expected size.";

    if (!main_ptr_loaded && is_it_load_of_root_binary &&
            mach_props->has_property_value(file_properties::file_type(),file_types::executable()))
        return "The binary contains no LC_MAIN load command (no entry point is specified).";

    std::string  error_message;
    if (bind_off != 0U)
    {
        error_message = perform_symbol_binding(bind_off,bind_size,false,mach,mach_props,load_props);
        if (!error_message.empty())
            return error_message;
    }
    if (lazy_bind_off != 0U)
    {
        error_message = perform_symbol_binding(lazy_bind_off,lazy_bind_size,false,mach,mach_props,load_props);
        if (!error_message.empty())
            return error_message;
    }
    if (weak_bind_off != 0U || weak_bind_size != 0U)
    {
        error_message = perform_symbol_binding(weak_bind_off,weak_bind_size,true,mach,mach_props,load_props);
        if (!error_message.empty())
            return error_message;
    }

    return "";
}


}}
