#include <rebours/MAL/loader/detail/load_elf.hpp>
#include <rebours/MAL/loader/detail/abi_loaders.hpp>
#include <rebours/MAL/loader/detail/address_fixing.hpp>
#include <rebours/MAL/loader/detail/set_file_property.hpp>
#include <rebours/utility/file_utils.hpp>
#include <rebours/utility/assumptions.hpp>
#include <rebours/utility/invariants.hpp>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <functional>
#include <tuple>
#include <unordered_set>
#include <iostream>

namespace loader { namespace detail {


std::string  load_elf_32bit(std::ifstream&  elf,
                            file_props_ptr  elf_props,
                            load_props_elf&  load_props)
{
    ASSUMPTION( elf.is_open() );
    ASSUMPTION( elf_props->num_address_bits() == 32U );

    static std::map<std::string,abi_virtual_address_selector> const  abi_loaders_choose_address{
            {abi_loaders::ld_linux_x86_32_so_2(),ld_linux_x86_32_so_2::choose_address_for_section},
            };
    static std::map<std::string,abi_pltgot_section_split_func> const  abi_loaders_split_section_by_pltgot{
            {abi_loaders::ld_linux_x86_32_so_2(),ld_linux_x86_32_so_2::split_section_by_pltgot},
            };
    static std::map<std::string,abi_special_symbol_hash_table_parser> const  abi_loaders_special_symbol_hash_table_parser{
            {abi_loaders::ld_linux_x86_32_so_2(),ld_linux_x86_32_so_2::special_symbol_hash_table_parser},
            };
    static std::map<std::string,abi_relocations_fn> const  abi_loaders_perform_relocations{
            {abi_loaders::ld_linux_x86_32_so_2(),ld_linux_x86_32_so_2::perform_relocations},
            };

    bool const  is_it_load_of_root_binary = load_props.root_file() == elf_props->path();

    if (fileutl::file_size(elf_props->path()) < 52ULL)
        LOAD_ELF_WARNING("The file is too small to contain the ELF header.");

    fileutl::skip_bytes(elf,1U); // We ignore original version.

    // Load platform props and update elf_props (is it executable or shared library)

    bool  is_it_load_of_shared_library;
    {
        abi_t abi;
        switch (fileutl::read_byte(elf))
        {
        case 0x00U:
            abi = abi_t::UNIX; break;
        case 0x03U:
            abi = abi_t::LINUX; break;
        default:
            LOAD_ELF_WARNING("[Header offset 0x07] Unknown abi type.");
            abi = abi_t::UNKNOWN_ABI;
            break;
        }

        fileutl::skip_bytes(elf,1U); // We ignore abi_version_byte.
        fileutl::skip_bytes(elf,7U); // Skip unused bytes.

        uint32_t const  file_type = fileutl::read_bytes_to_uint32_t(elf,2U,elf_props->is_in_big_endian());
        if (file_type != 2U && file_type != 3U)
            return "[Header offset 0x10] Unknown file type (neither executable nor shared library).";
        is_it_load_of_shared_library = file_type == 3U;

        set_file_property(elf_props->property_map(), file_properties::file_type(),
                          is_it_load_of_shared_library ? file_types::library() : file_types::executable() );

        architecture_t arch;
        {
            uint32_t const  arch_id = fileutl::read_bytes_to_uint32_t(elf,2U,elf_props->is_in_big_endian());
            switch (arch_id)
            {
            case 0x03U:
                arch = architecture_t::X86_32; break;
            default:
                {
                    std::ostringstream  ostr;
                    ostr << "[Header offset 0x12] Unknown architecture type ID: 0x"
                         << std::hex << arch_id;
                    return ostr.str();
                }
            }
        }

	platform_ptr const  file_platform{ new platform(arch, abi) };
        if (is_it_load_of_root_binary)
            load_props.set_platform( file_platform );
        if (*file_platform != *load_props.platform())
            return "Incompatible platforms of the executable and its shared library: " + elf_props->path();
    }

    fileutl::skip_bytes(elf,4U);// We do not use the second orig version.

    // Load of the entry point
    uint64_t  entry_point = fileutl::read_bytes_to_uint32_t(elf,4U,elf_props->is_in_big_endian());

    // Load of sections
    uint64_t const  program_header_offset = fileutl::read_bytes_to_uint32_t(elf,4U,elf_props->is_in_big_endian());

    fileutl::skip_bytes(elf,4U); // We do not use section header offset.
    fileutl::skip_bytes(elf,4U); // We do not use processor flags.

    {
        uint32_t const  elf_header_size = fileutl::read_bytes_to_uint32_t(elf,2U,elf_props->is_in_big_endian());
        if (elf_header_size != 52U)
            LOAD_ELF_WARNING("[Header offset 0x28] Wrong size of the ELF header 32bit (have to be 52 bytes). "
                             "We ignore it.");
    }

    uint32_t const  program_header_table_entry_size = fileutl::read_bytes_to_uint32_t(elf,2U,elf_props->is_in_big_endian());
    uint32_t const  program_header_table_num_entries = fileutl::read_bytes_to_uint32_t(elf,2U,elf_props->is_in_big_endian());

    fileutl::skip_bytes(elf,2U); // We do not use section header table entry size.
    fileutl::skip_bytes(elf,2U); // We do not use section header table num entries.
    fileutl::skip_bytes(elf,2U); // We do not use section header table secnames entry_index.

    // Here we are done with reading ELF's header.
    // We continue with reading the program header table.

    if (fileutl::file_size(elf_props->path()) <
            program_header_offset + program_header_table_entry_size * program_header_table_num_entries)
        return "The program header table goes behind the end of the file.";

    typedef std::tuple<section_file_header_ptr,uint32_t,section_content_ptr>  section_data;
    std::vector<section_data>  new_sections_data;

    uint64_t  string_table_file_address = 0ULL;
    uint64_t  string_table_size = 0ULL;
    std::vector<uint64_t>  string_table_offsets;

    uint64_t  preinit_fn_array_address = 0ULL;
    uint64_t  preinit_fn_array_size = 0ULL;
    uint64_t  init_fn_address = 0ULL;
    uint64_t  fini_fn_address = 0ULL;
    uint64_t  init_fn_array_address = 0ULL;
    uint64_t  init_fn_array_size = 0ULL;
    uint64_t  fini_fn_array_address = 0ULL;
    uint64_t  fini_fn_array_size = 0ULL;

    uint64_t  pltgot_address = 0ULL;
    uint64_t  got_relocations_table_address = 0ULL;
    uint64_t  got_relocations_table_size = 0ULL;
    uint64_t  got_relocations_table_entry_size = 0ULL;
    uint64_t  plt_relocations_table_type = 0ULL;
    uint64_t  plt_relocations_table_address = 0ULL;
    uint64_t  plt_relocations_table_size = 0ULL;

    uint64_t  symbol_table_address = 0ULL;
    uint64_t  symbol_table_entry_size = 0ULL;

    uint64_t  symbol_hash_table_address = 0ULL;
    uint32_t  symbol_hash_table_type = 0U;

    elf.seekg(program_header_offset);
    for (uint32_t  i = 0U; i < program_header_table_num_entries; ++i)
    {
        uint32_t const  type = fileutl::read_bytes_to_uint32_t(elf,4U,elf_props->is_in_big_endian());
        uint64_t const  offset = fileutl::read_bytes_to_uint32_t(elf,4U,elf_props->is_in_big_endian());
        uint64_t const  virtual_address = fileutl::read_bytes_to_uint32_t(elf,4U,elf_props->is_in_big_endian());

        fileutl::skip_bytes(elf,4U); // We do not use physical address.

        uint64_t const  size_in_file = fileutl::read_bytes_to_uint32_t(elf,4U,elf_props->is_in_big_endian());
        uint64_t const  size_in_memory = fileutl::read_bytes_to_uint32_t(elf,4U,elf_props->is_in_big_endian());
        uint32_t const  flags = fileutl::read_bytes_to_uint32_t(elf,4U,elf_props->is_in_big_endian());
        uint64_t const  align = fileutl::read_bytes_to_uint32_t(elf,4U,elf_props->is_in_big_endian());

        if (type != 1U && type != 2U && type != 3U) // We consider only LOAD, DYNAMIC, and INTERP segments.
            continue;

        std::string  err_prefix;
        {
            std::stringstream sstr;
            sstr << "The segment no. " << i << " ";
            err_prefix = sstr.str();
        }
        if (type != 1U && (size_in_file == 0U || size_in_memory < size_in_file))
            return err_prefix + "has wrong size.";
        if (size_in_memory == 0U)
        {
            LOAD_ELF_WARNING(err_prefix << " at " << std::hex << offset <<
                             " has size 0 in the memory. We skip it!");
            continue;
        }
        if (fileutl::file_size(elf_props->path()) < offset + size_in_file)
            return err_prefix + "goes behind the end of the file.";
        if (offset % align != virtual_address % align)
            LOAD_ELF_WARNING(err_prefix << "has an inconsistency between file and memory alignment.");

        switch (type)
        {
        case 1U: // We detected LOAD loadable segment.
            {
                if (flags > 7U)
                    LOAD_ELF_WARNING(err_prefix << "has wrong flags. Using a value "
                                                   "trimmed to the proper range.");

                section_file_header_ptr const  file_header(
                        new section_file_header{
                                offset,
                                virtual_address,
                                size_in_file,
                                size_in_memory,
                                align,align  // both in-file and in-memory alignments are equal modulo this align
                                }
                        );
                std::shared_ptr<section_content> const  content{
                        new section_content(size_in_memory,0U)
                        };
                std::ifstream::pos_type const  saved_file_pos = elf.tellg();
                elf.seekg(offset);
                elf.read(reinterpret_cast<char*>(&content->at(0U)),size_in_file);
                elf.seekg(saved_file_pos);

                new_sections_data.push_back( section_data{file_header,flags,content} );
                break;
            }
        case 2U: // We detected DYNAMIC dynamic liking segment.
            {
                std::ifstream::pos_type const  saved_file_pos = elf.tellg();
                elf.seekg(offset);
                while ((uint64_t)elf.tellg() < offset + size_in_file)
                    switch (fileutl::read_bytes_to_int32_t(elf,4U,elf_props->is_in_big_endian()))
                    {
                    case 1U: // We detected NEEDED entry
                        string_table_offsets.push_back(fileutl::read_bytes_to_uint64_t(elf,4U,elf_props->is_in_big_endian()));
                        break;
                    case 2U: // We detected PLTRELSZ entry
                        plt_relocations_table_size = fileutl::read_bytes_to_uint64_t(elf,4U,elf_props->is_in_big_endian());
                        break;
                    case 3U: // We detected PLTGOT entry
                        pltgot_address = fileutl::read_bytes_to_uint64_t(elf,4U,elf_props->is_in_big_endian());
                        break;
                    case 4U: // We detected symbol HASH table entry
                        symbol_hash_table_address = fileutl::read_bytes_to_uint64_t(elf,4U,elf_props->is_in_big_endian());
                        symbol_hash_table_type = 4ULL;
                        break;
                    case 5U: // We detected STRTAB entry
                        string_table_file_address = fileutl::read_bytes_to_uint64_t(elf,4U,elf_props->is_in_big_endian());
                        break;
                    case 6U: // We detected SYMTAB entry
                        symbol_table_address = fileutl::read_bytes_to_uint64_t(elf,4U,elf_props->is_in_big_endian());
                        break;
                    case 7U: // We detected RELA entry
                        got_relocations_table_address = fileutl::read_bytes_to_uint64_t(elf,4U,elf_props->is_in_big_endian());
                        break;
                    case 8U: // We detected RELASZ entry
                        got_relocations_table_size = fileutl::read_bytes_to_uint64_t(elf,4U,elf_props->is_in_big_endian());
                        break;
                    case 9U: // We detected RELAENT entry
                        got_relocations_table_entry_size = fileutl::read_bytes_to_uint64_t(elf,4U,elf_props->is_in_big_endian());
                        break;
                    case 10U: // We detected STRSZ entry
                        string_table_size = fileutl::read_bytes_to_uint64_t(elf,4U,elf_props->is_in_big_endian());
                        break;
                    case 11U: // We detected SYMENT entry
                        symbol_table_entry_size = fileutl::read_bytes_to_uint64_t(elf,4U,elf_props->is_in_big_endian());
                        break;
                    case 12U: // We detected INIT entry
                        init_fn_address = fileutl::read_bytes_to_uint64_t(elf,4U,elf_props->is_in_big_endian());
                        break;
                    case 13U: // We detected FINI entry
                        fini_fn_address = fileutl::read_bytes_to_uint64_t(elf,4U,elf_props->is_in_big_endian());
                        break;
                    case 17U: // We detected REL entry
                        got_relocations_table_address = fileutl::read_bytes_to_uint64_t(elf,4U,elf_props->is_in_big_endian());
                        break;
                    case 18U: // We detected RELSZ entry
                        got_relocations_table_size = fileutl::read_bytes_to_uint64_t(elf,4U,elf_props->is_in_big_endian());
                        break;
                    case 19U: // We detected RELENT entry
                        got_relocations_table_entry_size = fileutl::read_bytes_to_uint64_t(elf,4U,elf_props->is_in_big_endian());
                        break;
                    case 20U: // We detected PLTREL entry
                        plt_relocations_table_type = fileutl::read_bytes_to_uint64_t(elf,4U,elf_props->is_in_big_endian());
                        break;
                    case 23U: // We detected JMPREL entry
                        plt_relocations_table_address = fileutl::read_bytes_to_uint64_t(elf,4U,elf_props->is_in_big_endian());
                        break;
                    case 25U: // We detected INIT_ARRAY entry
                        init_fn_array_address = fileutl::read_bytes_to_uint64_t(elf,4U,elf_props->is_in_big_endian());
                        break;
                    case 26U: // We detected FINI_ARRAY entry
                        fini_fn_array_address = fileutl::read_bytes_to_uint64_t(elf,4U,elf_props->is_in_big_endian());
                        break;
                    case 27U: // We detected INIT_ARRAYSZ entry
                        init_fn_array_size = fileutl::read_bytes_to_uint64_t(elf,4U,elf_props->is_in_big_endian());
                        break;
                    case 28U: // We detected FINI_ARRAYSZ entry
                        fini_fn_array_size = fileutl::read_bytes_to_uint64_t(elf,4U,elf_props->is_in_big_endian());
                        break;
                    case 32U: // We detected PREINIT_ARRAY entry
                        preinit_fn_array_address = fileutl::read_bytes_to_uint64_t(elf,4U,elf_props->is_in_big_endian());
                        break;
                    case 33U: // We detected PREINIT_ARRAYSZ entry
                        preinit_fn_array_size = fileutl::read_bytes_to_uint64_t(elf,4U,elf_props->is_in_big_endian());
                        break;
                    case 0x6ffffef5U: // We detected DT_GNU_HASH entry
                        if (symbol_hash_table_address == 0ULL)
                        {
                            symbol_hash_table_address = fileutl::read_bytes_to_uint64_t(elf,4U,elf_props->is_in_big_endian());
                            symbol_hash_table_type = 0x6ffffef5U;
                        }
                        else
                        {
                            fileutl::skip_bytes(elf,4U);
                            if (symbol_hash_table_type != 4U)
                                return "More than one ABI-specific symbol hash table provided.";
                        }
                        break;
                    default:
                        fileutl::skip_bytes(elf,4U);
                        break;
                    }
                elf.seekg(saved_file_pos);
                break;
            }
        case 3U: // We detected INTERP interpreter segment.
            {
                if (elf_props->property(file_properties::abi_loader()) != abi_loaders::NONE())
                    return "More than one ABI loader segment INTERP is present in the file.";
                std::vector<uint8_t>  buffer(size_in_file);
                std::ifstream::pos_type const  saved_file_pos = elf.tellg();
                elf.seekg(offset);
                elf.read(reinterpret_cast<char*>(&buffer.at(0U)),size_in_file);
                elf.seekg(saved_file_pos);
                std::string  interpreter = std::string{buffer.begin(),buffer.end()};
                if (interpreter.back() == '\0')
                    interpreter.pop_back();
                if (abi_loaders_choose_address.count(interpreter) == 0ULL)
                    return err_prefix + "(INTERP) contains unknown interpreter '" + interpreter + "'.";
                set_file_property(elf_props->property_map(), file_properties::abi_loader(), interpreter);
                break;
            }
        default: UNREACHABLE();
        }
    }

    load_props.add_file_props(elf_props);

    // Now we update elf_props by the proper ABI loader retrieved from the header.

    std::string  abi_loader_name = abi_loaders::NONE();
    if (is_it_load_of_root_binary)
    {
        if (elf_props->property(file_properties::abi_loader()) == abi_loaders::NONE())
        {
            if (load_props.platform()->abi() == abi_t::UNIX ||
                load_props.platform()->abi() == abi_t::LINUX )
            {
                if (!is_it_load_of_shared_library &&
                    (!string_table_offsets.empty() ||
                    (got_relocations_table_address != 0ULL && got_relocations_table_size > 0ULL) ||
                    (plt_relocations_table_address != 0ULL && plt_relocations_table_size > 0ULL)) )
                    LOAD_ELF_WARNING("ABI loader segment INTERP is missing in the root binary. "
                                     "Using an ABI loader deduced from the platform's ABI name.");
                abi_loader_name = abi_loaders::ld_linux_x86_32_so_2();
            }
            else if (!string_table_offsets.empty() ||
                    (got_relocations_table_address != 0ULL && got_relocations_table_size > 0ULL) ||
                    (plt_relocations_table_address != 0ULL && plt_relocations_table_size > 0ULL) )
                return "ABI loader segment INTERP is missing in the root binary.";
            else
            {
                LOAD_ELF_WARNING("ABI loader segment INTERP is missing in the root binary. "
                                 "OS ABI type is also unknown. So, using the default ABI loader "
                                 "for ELF files (as it was on Linux).");
                abi_loader_name = abi_loaders::ld_linux_x86_32_so_2();
            }
        }
        else
            abi_loader_name = elf_props->property(file_properties::abi_loader());

        load_props.set_parent_abi_loader(abi_loader_name);
    }
    else
        abi_loader_name = elf_props->property(file_properties::abi_loader()) == abi_loaders::NONE() ?
                                    load_props.parent_abi_loader() :
                                    elf_props->property(file_properties::abi_loader());

    auto const abi_address_selector_iter = abi_loaders_choose_address.find(abi_loader_name);
    if (is_it_load_of_shared_library && abi_address_selector_iter == abi_loaders_choose_address.cend())
        return "Unsupported ABI loader for virtual address selection.";
    auto const abi_section_split_iter = abi_loaders_split_section_by_pltgot.find(abi_loader_name);
    if (abi_section_split_iter == abi_loaders_split_section_by_pltgot.cend())
        return "Unsupported ABI loader for section splitting.";
    section_ptr  string_table_section;
    for (auto const& elem : new_sections_data)
    {
        section_file_header_ptr const  file_header = std::get<0>(elem);
        uint32_t const  flags = std::get<1>(elem);
        section_content_ptr const  content = std::get<2>(elem);

        uint64_t const  fixed_virtual_address =
                !is_it_load_of_root_binary || is_it_load_of_shared_library ?
                            abi_address_selector_iter->second(file_header,*load_props.sections_table()):
                            file_header->virtual_address();

        load_props.add_section_address_fix(elf_props->path(),file_header->virtual_address(),fixed_virtual_address);

        std::vector<section_ptr>  new_sections;
        abi_section_split_iter->second(
                section_ptr{
                        new section{fixed_virtual_address,
                                    fixed_virtual_address + file_header->size_in_memory(),
                                    content,
                                    (flags & 4U) != 0U,
                                    (flags & 2U) != 0U,
                                    (flags & 1U) != 0U,
                                    elf_props->is_in_big_endian(),
                                    true,
                                    elf_props,
                                    file_header
                                    }
                        },
                (address)pltgot_address,
                new_sections
                );
        if (new_sections.empty())
            return "Split of sections by PLTGOT pointer has failed.";
        for (auto const secptr : new_sections)
        {
            std::string const  err_message = load_props.add_section(secptr);
            if (!err_message.empty())
                return err_message;
        }
        if (file_header->virtual_address() <= string_table_file_address &&
            string_table_file_address + string_table_size <=
                file_header->virtual_address() + file_header->size_in_file())
        {
            if (string_table_section.operator bool())
                return "String table does not start in a unique section.";
            string_table_section = new_sections.front();
        }
    }

    if (is_it_load_of_root_binary)
    {
        if (is_it_load_of_shared_library)
            load_props.set_entry_point(load_props.fixed_address_for(elf_props->path(),entry_point));
        else
            load_props.set_entry_point(entry_point);
    }

    if (is_it_load_of_root_binary)
    {
        if (load_props.sections_table()->empty())
            return "The passed executable does not contain any loadable section.";
        if (![](load_props_elf&  load_props) {
                for (sections_table::value_type const& elem : *load_props.sections_table())
                    if (elem.second->start_address() <= load_props.entry_point() &&
                        load_props.entry_point() < elem.second->end_address() &&
                        elem.second->has_execute_access())
                        return true;
                return false;
            }(load_props))
            return "The entry point address does not point to any loaded executable section.";
    }

    if (string_table_size == 0ULL && !string_table_offsets.empty())
        return "There are references into the empty string table.";

    if (!string_table_section.operator bool() && !string_table_offsets.empty())
        return "Cannot locate the string table.";

    // Now we compute addresses of PREINIT, INIT, and FINI functions.

    if (preinit_fn_array_address != 0ULL && is_it_load_of_root_binary)
        for (address  preinit_table_ptr = load_props.fixed_address_for(elf_props->path(),preinit_fn_array_address),
                      preinit_table_end = preinit_table_ptr + preinit_fn_array_size;
                preinit_table_ptr < preinit_table_end;
                preinit_table_ptr += sizeof(uint32_t))
            load_props.add_init_function(
                        elf_props->path(),
                        load_props.fixed_address_for(
                                elf_props->path(),
                                read_uint32_t(preinit_table_ptr,load_props.sections_table())
                                )
                        );
    if (init_fn_address != 0ULL)
        load_props.add_init_function(
                    elf_props->path(),
                    load_props.fixed_address_for(elf_props->path(),init_fn_address)
                    );
    if (fini_fn_address != 0ULL)
        load_props.add_fini_function(
                    elf_props->path(),
                    load_props.fixed_address_for(elf_props->path(),fini_fn_address)
                    );
    if (init_fn_array_address != 0ULL)
        for (address  init_table_ptr = load_props.fixed_address_for(elf_props->path(),init_fn_array_address),
                      init_table_end = init_table_ptr + init_fn_array_size;
                init_table_ptr < init_table_end;
                init_table_ptr += sizeof(uint32_t))
            load_props.add_init_function(
                        elf_props->path(),
                        load_props.fixed_address_for(
                                elf_props->path(),
                                read_uint32_t(init_table_ptr,load_props.sections_table())
                                )
                        );
    if (fini_fn_array_address != 0ULL)
        for (address  fini_table_ptr = load_props.fixed_address_for(elf_props->path(),fini_fn_array_address),
                      fini_table_end = fini_table_ptr + fini_fn_array_size;
                fini_table_ptr < fini_table_end;
                fini_table_ptr += sizeof(uint32_t))
            load_props.add_fini_function(
                        elf_props->path(),
                        load_props.fixed_address_for(
                                elf_props->path(),
                                read_uint32_t(fini_table_ptr,load_props.sections_table())
                                )
                        );

    if (is_it_load_of_root_binary)
        load_props.add_init_function(elf_props->path(),load_props.entry_point());

    // Now we parse symbols form the dynamic symbol table. Since there is no definition
    // of the size of this table (indeed, it is really missing in the ELF specification)
    // we determine its size by parsing the symbol hash table first. It must be always present.

    if (symbol_table_entry_size == 16ULL)
    {
        std::unordered_set<uint64_t>  symbol_table_indices;
        uint64_t  symbol_table_begin_index;
        uint64_t  symbol_table_end_index;

        if (symbol_hash_table_type == 4ULL) // DT_HASH  (the one of the ELF specification)
        {
            address const  fixed_symbol_hash_table_address = load_props.fixed_address_for(elf_props->path(),symbol_hash_table_address);
            uint32_t const  num_buckets  = read_uint32_t(fixed_symbol_hash_table_address,load_props.sections_table());
            uint32_t const  num_chains  = read_uint32_t(fixed_symbol_hash_table_address + 4ULL,load_props.sections_table());
            address const  buckets_begin  = fixed_symbol_hash_table_address + 8ULL;
            address const  buckets_end  = buckets_begin + num_buckets * 4ULL;
            address const  chains_begin  = buckets_end;

            symbol_table_begin_index = 1ULL;
            symbol_table_end_index = num_chains;

            for (address  bucket_ptr = buckets_begin; bucket_ptr != buckets_end; bucket_ptr += 4U)
            {
                uint32_t  symbol_index = read_uint32_t(bucket_ptr,load_props.sections_table());
                if (symbol_index == 0U)
                    continue;
                symbol_table_indices.insert(symbol_index);
                for (address  chain_ptr = chains_begin + symbol_index * 4ULL;
                        (symbol_index = read_uint32_t(chain_ptr,load_props.sections_table())) != 0U;
                        chain_ptr += 4ULL)
                    symbol_table_indices.insert(symbol_index);
            }
        }
        else // There is some ABI-specific hash table instead (e.g. GNU_HASH).
        {
            auto const  it = abi_loaders_special_symbol_hash_table_parser.find(abi_loader_name);
            if (it == abi_loaders_special_symbol_hash_table_parser.end())
                return "Unknown ABI loader for the symbols HASH table.";
            std::string const  error_message = it->second(symbol_hash_table_address,symbol_hash_table_type,
                                                          symbol_table_begin_index,symbol_table_end_index,
                                                          symbol_table_indices,elf_props,load_props);
            if (!error_message.empty())
                return error_message;
        }

        address const  fixed_symbol_table_address = load_props.fixed_address_for(elf_props->path(),symbol_table_address);

        for (uint64_t  i = 1U; i < symbol_table_end_index; ++i)
        {
            address const  ptr = fixed_symbol_table_address + i * symbol_table_entry_size;

            uint32_t const  symbol_name = read_uint32_t(ptr + 0ULL,load_props.sections_table());
            uint64_t const  symbol_value = read_uint32_t(ptr + 4ULL,load_props.sections_table());
            uint64_t const  symbol_size = read_uint32_t(ptr + 8ULL,load_props.sections_table());
            uint8_t const  symbol_info = read_uint8_t(ptr + 12ULL,load_props.sections_table());
            // We do not read the following (commented) elements, since we do not need them.
            //uint8_t const  symbol_other = read_uint8_t(ptr + 13ULL,load_props.sections_table());
            //uint16_t const  symbol_shndx = read_uint16_t(ptr + 14ULL,load_props.sections_table());

            uint8_t const  symbol_bind = symbol_info >> 4U;
            uint8_t const symbol_type = symbol_info & 0xfU;

            if (symbol_bind != 1U && symbol_bind != 2U) // neither GLOBAL nor WEAK?
            {
                if (symbol_table_indices.count(i) != 0ULL)
                    return "Unknown bind type of a symbol table element referenced from the symbol hash table.";
                continue;
            }

            address const string_table_address =
                    string_table_section->start_address() +
                    (string_table_file_address - string_table_section->section_file_header()->virtual_address())
                    ;
            std::string  symbol_id;
            if (!read_zero_terminated_string(string_table_address + symbol_name,load_props.sections_table(),symbol_id))
                return "Cannot read a symbol inside the string table.";

            std:: string  symbol_type_name;
            switch (symbol_type)
            {
            case 1U /*OBJECT*/: symbol_type_name = relocation_type::DATA(); break;
            case 2U /*FUNC*/: symbol_type_name = relocation_type::FUNCTION(); break;
            default: symbol_type_name = relocation_type::UNKNOWN(); break;
            }

            if (symbol_table_indices.count(i) == 0ULL)// || symbol_type_name == relocation_type::UNKNOWN())
                load_props.add_hidden_symbol(elf_props->path(),i,symbol_id,symbol_type_name,symbol_value,symbol_size);
            else
                load_props.add_visible_symbol(elf_props->path(),i,symbol_id,symbol_type_name,symbol_value,symbol_size);
        }
    }
    else if (symbol_table_entry_size != 0ULL)
        return "Unknown format of the symbol table.";

    // Now we load dependant binaries (shared libraries) of the current binary.

    for (auto const shift : string_table_offsets)
    {
        address const string_table_address =
                string_table_section->start_address() +
                (string_table_file_address - string_table_section->section_file_header()->virtual_address())
                ;
        std::string  orig_pathname;
        if (!read_zero_terminated_string(string_table_address + shift,load_props.sections_table(),orig_pathname))
            return "Cannot read string table at the first given offset.";
        if (shift + orig_pathname.size() >= string_table_size)
            return "The library name passes the end of the symbols table.";
        std::string const lib_file_name = fileutl::parse_name_in_pathname(orig_pathname);
        bool  lib_loaded = false;
        if (std::find(load_props.ignored_files().cbegin(),load_props.ignored_files().cend(),lib_file_name)
                == load_props.ignored_files().cend())
        {
            for (auto const& search_dir : load_props.search_dirs())
            {
                std::string const raw_lib_pathname = fileutl::concatenate_file_paths(search_dir,lib_file_name);
                if (!fileutl::file_exists(raw_lib_pathname))
                    continue;
                std::string const lib_pathname = fileutl::normalise_path(fileutl::absolute_path(raw_lib_pathname));
                if (load_props.files_table()->count(lib_pathname) == 0U)
                {
                    std::string const  err_message = detail::load_elf(lib_pathname,load_props);
                    if (!err_message.empty())
                    {
                        if (err_message == NOT_ELF_FILE())
                            return "The referenced library '" + lib_pathname + "' is not an ELF file.";
                        return err_message;
                    }
                }
                load_props.add_file_dependence(elf_props->path(),lib_pathname);
                lib_loaded = true;
                break;
            }
        }
        if (!lib_loaded)
        {
            load_props.add_skipped_file(lib_file_name);
            load_props.add_file_dependence(elf_props->path(),lib_file_name);
        }
    }

    // Now we apply relacation of global symbols (i.e. variables and functions shared among
    // all loaded binaries).

    {
        auto const abi_relocator_iter = abi_loaders_perform_relocations.find(abi_loader_name);
        if (abi_relocator_iter == abi_loaders_perform_relocations.cend())
            return "Unknown ABI specific relocation.";
        std::string const  error_message =
                abi_relocator_iter->second(got_relocations_table_address,got_relocations_table_size,got_relocations_table_entry_size,
                                           plt_relocations_table_address,plt_relocations_table_size,plt_relocations_table_type,
                                           pltgot_address,elf_props,load_props);
        if (!error_message.empty())
            return error_message;
    }

    return ""; // No error occured -> return the empty error message.
}


}}
