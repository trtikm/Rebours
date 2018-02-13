#include <rebours/MAL/loader/detail/load_pe.hpp>
#include <rebours/MAL/loader/detail/abi_loaders.hpp>
#include <rebours/MAL/loader/detail/address_fixing.hpp>
#include <rebours/MAL/loader/detail/set_file_property.hpp>
#include <rebours/utility/file_utils.hpp>
#include <rebours/utility/to_string.hpp>
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


std::string  load_pe_32bit(std::ifstream&  pe, file_props_ptr  pe_props,
                           coff_standard_header_info const&  coff_info,
                           load_props_pe&  load_props)
{
    ASSUMPTION( pe.is_open() );
    ASSUMPTION( pe_props->num_address_bits() == 32U );

    bool const  is_it_load_of_root_binary = load_props.root_file() == pe_props->path();

    // First we read Windows specific fields of the COFF header

    uint64_t  image_base = 0ULL;
    uint64_t  section_alignment = 0ULL;
    uint64_t  file_alignment = 0ULL;
    uint32_t  dll_characteristics = 0U;
    uint32_t  num_data_directory_entries = 0U;

    {
        image_base = fileutl::read_bytes_to_uint64_t(pe,4U,pe_props->is_in_big_endian());
        if (image_base > (address)std::numeric_limits<uint32_t>::max())
            return "The image base address cannot be stored in 32bits.";

        section_alignment = fileutl::read_bytes_to_uint64_t(pe,4U,pe_props->is_in_big_endian());
        file_alignment = fileutl::read_bytes_to_uint64_t(pe,4U,pe_props->is_in_big_endian());
        if (file_alignment == 0ULL)
            return "File alignment is zero.";
        if (file_alignment > section_alignment)
            return "File alignment is greater than section alignment.";

        set_file_property(pe_props->property_map(),file_properties::os_major_version(),
                          to_string(fileutl::read_bytes_to_uint32_t(pe,2U,pe_props->is_in_big_endian())));
        set_file_property(pe_props->property_map(),file_properties::os_minor_version(),
                          to_string(fileutl::read_bytes_to_uint32_t(pe,2U,pe_props->is_in_big_endian())));

        fileutl::skip_bytes(pe,4U); // We do not use neither major nor minor image version.

        set_file_property(pe_props->property_map(),file_properties::subsytem_major_version(),
                          to_string(fileutl::read_bytes_to_uint32_t(pe,2U,pe_props->is_in_big_endian())));
        set_file_property(pe_props->property_map(),file_properties::subsytem_minor_version(),
                          to_string(fileutl::read_bytes_to_uint32_t(pe,2U,pe_props->is_in_big_endian())));

        fileutl::skip_bytes(pe,4U); // Skipping the reserved value.

        uint64_t const  size_of_image = fileutl::read_bytes_to_uint64_t(pe,4U,pe_props->is_in_big_endian());
        if (size_of_image % section_alignment != 0ULL)
            return "Size of the loaded binary is not a multiple of section alignment.";

        uint64_t const  size_of_headers = fileutl::read_bytes_to_uint64_t(pe,4U,pe_props->is_in_big_endian());
        if (size_of_headers % file_alignment != 0ULL)
            return "Size of the file headers is not a multiple of file alignment.";

        fileutl::skip_bytes(pe,4U); // We do not file's 'check-sum'

        switch (fileutl::read_bytes_to_uint32_t(pe,2U,pe_props->is_in_big_endian())) // Subsystem type
        {
        case 1U: // Native process (started during OS boot), e.g. a driver
            set_file_property(pe_props->property_map(),file_properties::subsytem(),subsystems::native());
            break;
        case 2U: // GUI - Graphical UI application
            set_file_property(pe_props->property_map(),file_properties::subsytem(),subsystems::GUI());
            break;
        case 3U: // CUI - Command-line UI application
            set_file_property(pe_props->property_map(),file_properties::subsytem(),subsystems::CUI());
            break;
        case 7U: // POSIX - Posix Command-line UI application
            set_file_property(pe_props->property_map(),file_properties::subsytem(),subsystems::posix_CUI());
            break;
        case 9U: // Windows CE GUI application
            set_file_property(pe_props->property_map(),file_properties::subsytem(),subsystems::windows_ce());
            break;
        case 10U: // EFI - Extensible Firmware Interface
            set_file_property(pe_props->property_map(),file_properties::subsytem(),subsystems::EFI());
            break;
        default:
            return "Unknown subsystem type.";
        }

        dll_characteristics = fileutl::read_bytes_to_uint32_t(pe,2U,pe_props->is_in_big_endian());

        set_file_property(pe_props->property_map(),file_properties::max_stack_size(),
                          to_string(fileutl::read_bytes_to_uint64_t(pe,4U,pe_props->is_in_big_endian())));

        fileutl::skip_bytes(pe,4U); // We do not use initial (commit) stack size.

        set_file_property(pe_props->property_map(),file_properties::max_local_heap_size(),
                          to_string(fileutl::read_bytes_to_uint64_t(pe,4U,pe_props->is_in_big_endian())));

        fileutl::skip_bytes(pe,4U); // We do not use initial (commit) local heap size.


        fileutl::skip_bytes(pe,4U); // Skipping the reserved value.

        num_data_directory_entries = fileutl::read_bytes_to_uint32_t(pe,4U,pe_props->is_in_big_endian());

        if (coff_info.optional_header_offset() + coff_info.size_of_optional_header() <
                (uint32_t)pe.tellg() + num_data_directory_entries * 8U)
            return "The number of data-directory entries is too high to fit into the optional header.";
    }

    // Now we read the data directory

    uint64_t  export_table_address = 0ULL;
    uint64_t  export_table_size = 0ULL;
    uint64_t  import_table_address = 0ULL;
    uint64_t  import_table_size = 0ULL;
    uint64_t  resource_table_address = 0ULL;
    uint64_t  resource_table_size = 0ULL;
    uint64_t  base_relocation_table_address = 0ULL;
    uint64_t  base_relocation_table_size = 0ULL;
    uint64_t  thread_local_storage_table_address = 0ULL;
    uint64_t  thread_local_storage_table_size = 0ULL;
    uint64_t  load_config_table_address = 0ULL;
    uint64_t  load_config_table_size = 0ULL;
    uint64_t  bound_import_table_address = 0ULL;
    uint64_t  bound_import_table_size = 0ULL;

    {
        for (uint64_t  num_entries = 0ULL; num_entries < num_data_directory_entries; )
        {
            export_table_address = fileutl::read_bytes_to_uint64_t(pe,4U,pe_props->is_in_big_endian());
            export_table_size = fileutl::read_bytes_to_uint64_t(pe,4U,pe_props->is_in_big_endian());
            if (++num_entries == num_data_directory_entries)
                break;

            import_table_address = fileutl::read_bytes_to_uint64_t(pe,4U,pe_props->is_in_big_endian());
            import_table_size = fileutl::read_bytes_to_uint64_t(pe,4U,pe_props->is_in_big_endian());
            if (++num_entries == num_data_directory_entries)
                break;

            resource_table_address = fileutl::read_bytes_to_uint64_t(pe,4U,pe_props->is_in_big_endian());
            resource_table_size = fileutl::read_bytes_to_uint64_t(pe,4U,pe_props->is_in_big_endian());
            if (++num_entries == num_data_directory_entries)
                break;

            fileutl::skip_bytes(pe,8ULL); // We ignore exceptions table.
            if (++num_entries == num_data_directory_entries)
                break;
            fileutl::skip_bytes(pe,8ULL); // We ignore certificate table.
            if (++num_entries == num_data_directory_entries)
                break;

            base_relocation_table_address = fileutl::read_bytes_to_uint64_t(pe,4U,pe_props->is_in_big_endian());
            base_relocation_table_size = fileutl::read_bytes_to_uint64_t(pe,4U,pe_props->is_in_big_endian());
            if (++num_entries == num_data_directory_entries)
                break;

            fileutl::skip_bytes(pe,8ULL); // We ignore debug data table.
            if (++num_entries == num_data_directory_entries)
                break;
            fileutl::skip_bytes(pe,8ULL); // We skip reserved bytes.
            if (++num_entries == num_data_directory_entries)
                break;
            fileutl::skip_bytes(pe,8ULL); // We ignore global ptr.
            if (++num_entries == num_data_directory_entries)
                break;

            thread_local_storage_table_address = fileutl::read_bytes_to_uint64_t(pe,4U,pe_props->is_in_big_endian());
            thread_local_storage_table_size = fileutl::read_bytes_to_uint64_t(pe,4U,pe_props->is_in_big_endian());
            if (++num_entries == num_data_directory_entries)
                break;

            load_config_table_address = fileutl::read_bytes_to_uint64_t(pe,4U,pe_props->is_in_big_endian());
            load_config_table_size = fileutl::read_bytes_to_uint64_t(pe,4U,pe_props->is_in_big_endian());
            if (++num_entries == num_data_directory_entries)
                break;

            bound_import_table_address = fileutl::read_bytes_to_uint64_t(pe,4U,pe_props->is_in_big_endian());
            bound_import_table_size = fileutl::read_bytes_to_uint64_t(pe,4U,pe_props->is_in_big_endian());
            if (++num_entries == num_data_directory_entries)
                break;

            fileutl::skip_bytes(pe,8ULL); // We ignore import address table address and size.
            if (++num_entries == num_data_directory_entries)
                break;

            fileutl::skip_bytes(pe,8ULL); // We ignore delay import table address and size.
            if (++num_entries == num_data_directory_entries)
                break;

            fileutl::skip_bytes(pe,8ULL); // We ignore CLG runtime header.
            if (++num_entries == num_data_directory_entries)
                break;
            fileutl::skip_bytes(pe,8ULL); // We skip reserved bytes.

            break;
        }

        if (pe.tellg() > coff_info.optional_header_offset() + coff_info.size_of_optional_header())
            return "There is wrong size of the optional header stored in the COFF header.";

        pe.seekg(coff_info.optional_header_offset() + coff_info.size_of_optional_header());
    }

    // Now we load sections.

    if (fileutl::file_size(pe_props->path()) <
            coff_info.optional_header_offset() + coff_info.size_of_optional_header() +
            coff_info.number_of_sections() * 40U)
        return "Sections in the PE file go beyond the end of the file.";

    address  fixed_address = image_base;
    {
        if (!is_it_load_of_root_binary)
        {
            address const  highest = load_props.highest_end_address();
            if (fixed_address < highest)
            {
                if (pe_props->property(file_properties::file_type()) == file_types::executable())
                {
                    if (pe_props->property(file_properties::subsytem()) == subsystems::windows_ce())
                        fixed_address = 0x10000ULL;
                    else
                        fixed_address = 0x400000ULL;
                }
                else
                {
                    INVARIANT(pe_props->property(file_properties::file_type()) == file_types::library());
                    fixed_address = 0x10000000ULL;
                }

                if (fixed_address < highest)
                {
                    uint64_t const  memory_alignment = 0x10000ULL;
                    fixed_address =  highest + memory_alignment - (highest % memory_alignment);
                }
            }
        }
    }
    uint64_t const  difference = fixed_address - image_base;

    bool  code_base_tested = false;
    bool  data_base_tested = false;
    for (uint32_t  i = 0U; i < coff_info.number_of_sections(); ++i)
    {
        fileutl::skip_bytes(pe,8ULL); // We ignore section names

        uint64_t const  memory_size = fileutl::read_bytes_to_uint64_t(pe,4U,pe_props->is_in_big_endian());
        if (memory_size == 0ULL)
            return "The size-in-memory of the section at index " + to_string(i) + " is 0.";
        uint64_t const  memory_address = fileutl::read_bytes_to_uint64_t(pe,4U,pe_props->is_in_big_endian());

        uint64_t const  aligned_size_in_file = fileutl::read_bytes_to_uint64_t(pe,4U,pe_props->is_in_big_endian());
        if (aligned_size_in_file % file_alignment != 0ULL)
            LOAD_PE_WARNING("The size-in-file of the section at index " + to_string(i) + " is not properly aligned.");
        uint64_t const  aligned_offset_in_file = fileutl::read_bytes_to_uint64_t(pe,4U,pe_props->is_in_big_endian());
        if (aligned_offset_in_file % file_alignment != 0ULL)
            return "The offset-in-file of the section at index " + to_string(i) + " is not properly aligned.";
        if (aligned_offset_in_file + std::min(aligned_size_in_file,memory_size) > fileutl::file_size(pe_props->path()))
            return "The offset-in-file of the section at index " + to_string(i) + " is not properly aligned.";

        fileutl::skip_bytes(pe,4ULL); // We ignore relocations (this field is 0 for executable images)
        fileutl::skip_bytes(pe,4ULL); // We ignore line numbers (debug info is deprecated)
        fileutl::skip_bytes(pe,2ULL); // We ignore number of relocations (this field is 0 for executable images or )
        fileutl::skip_bytes(pe,2ULL); // We ignore number of line numbers (debug info is deprecated)

        uint32_t const  flags = fileutl::read_bytes_to_uint32_t(pe,4U,pe_props->is_in_big_endian());

        bool const  is_code = (flags & 0x20U) != 0U;
        bool const  is_data_initialised = (flags & 0x40U) != 0U;
        bool const  is_data_uninitialised = (flags & 0x80U) != 0U;
        bool const  has_extended_relocations = (flags & 0x1000000U) != 0U;
        bool const  has_read_access = (flags & 0x40000000U) != 0U;
        bool const  has_write_access = (flags & 0x80000000U) != 0U;
        bool const  has_execute_access = (flags & 0x20000000U) != 0U;

        if (!is_code && !is_data_initialised && !is_data_uninitialised)
            LOAD_PE_WARNING("The section at index " << to_string(i) << " is neither code nor data.");
        if (is_code && !has_execute_access)
            return "The section at index " + to_string(i) + " is a non-executable code segment.";
        if (has_extended_relocations)
            return "Extended relocations are NOT IMPLEMENTED YET!";
        if (is_code && !code_base_tested)
        {
            if (memory_address != coff_info.base_of_code())
                return "Base of code differs from the RVA of the first code section.";
            code_base_tested = true;
        }
        if ((is_data_initialised || is_data_uninitialised) && !data_base_tested)
        {
            if (memory_address != coff_info.base_of_data())
                LOAD_PE_WARNING("Base of data differs from the RVA of the first data section.");
            data_base_tested = true;
        }

        std::shared_ptr<section_content> const  content{ new section_content(memory_size,0U) };
        {
            uint64_t const  saved_file_pos = pe.tellg();

            pe.seekg(aligned_offset_in_file);
            pe.read(reinterpret_cast<char*>(content->data()),std::min(aligned_size_in_file,memory_size));

            pe.seekg(saved_file_pos);
        }

        section_file_header_ptr const  header { new section_file_header {
                aligned_offset_in_file,
                image_base + memory_address,
                aligned_size_in_file,
                memory_size,
                file_alignment,
                section_alignment
                }};

        load_props.add_section(
                section_ptr { new section {
                    fixed_address + memory_address,
                    fixed_address + memory_address + memory_size,
                    content,
                    has_read_access,
                    has_write_access,
                    has_execute_access,
                    pe_props->is_in_big_endian(),
                    true,
                    pe_props,
                    header
                    }}
                );

        load_props.add_section_address_fix(pe_props->path(),image_base + memory_address,
                                                            fixed_address + memory_address);
    }
    if (!code_base_tested)
        LOAD_PE_WARNING("The loaded binary file has no code section.");

    // Now we set the init&fini function (it is only one for both init and fini - its parameter distinguishes the action)

    if (is_it_load_of_root_binary)
        load_props.set_entry_point(fixed_address + coff_info.address_of_entry_point());

    load_props.add_init_function(pe_props->path(),fixed_address + coff_info.address_of_entry_point());
    load_props.add_fini_function(pe_props->path(),fixed_address + coff_info.address_of_entry_point());

    // Now we perform base relocations

    if (difference != 0ULL)
    {
        if (base_relocation_table_address == 0ULL || base_relocation_table_size == 0ULL)
            return "Invalid base relocations table.";

        for (address  block_ptr = fixed_address + base_relocation_table_address,
                      block_end = block_ptr + base_relocation_table_size;
             block_ptr < block_end; )
        {
            address const  reloc_page_ptr = fixed_address + read_uint32_t(block_ptr,load_props.sections_table());
            uint32_t const  num_block_bytes = read_uint32_t(block_ptr + 4U,load_props.sections_table());

            if (num_block_bytes < 10U || (num_block_bytes % 2U) != 0U)
                return "A block of the base relocations table has wrong number of bytes.";
            if (block_ptr + num_block_bytes > block_end)
                return "A block of the base relocations table exceeds the end of the table.";

            for (address  ptr = block_ptr + 8ULL,
                          end = block_ptr + num_block_bytes;
                 ptr < end;
                 ptr += 2U)
            {
                uint16_t const  reloc_word = read_uint16_t(ptr,load_props.sections_table());
                uint8_t const  reloc_type = (uint8_t)(reloc_word >> 12U);
                uint16_t const  reloc_offset = reloc_word & 0xfff;

                address const  reloc_address = reloc_page_ptr + reloc_offset;

                switch (reloc_type)
                {
                case 0: // IMAGE_REL_BASED_ABSOLUTE
                        //   The base relocation is skipped. This type can be used to pad a block.
                    break;
                case 1: // IMAGE_REL_BASED_HIGH
                        //   The base relocation adds the high 16 bits of the difference to
                        //   the 16-bit field at offset. The 16-bit field represents the high
                        //   value of a 32-bit word.
                    {
                        address const  orig_value = read_uint32_t(reloc_address,load_props.sections_table());
                        address const  addon = difference & 0xffff0000ULL;
                        if (addon + orig_value > (address)std::numeric_limits<uint32_t>::max())
                            return "Computed relocation address cannot be stored in 32bits.";
                        write_uint32_t(reloc_address, (uint32_t)(addon + orig_value), load_props.sections_table());
                        load_props.add_performed_relocation({
                                reloc_address,
                                4ULL,
                                "IMAGE_REL_BASED_HIGH",
                                addon + orig_value,
                                orig_value,
                                "",
                                "Base relocation."
                                });
                    }
                    break;
                case 2: // IMAGE_REL_BASED_LOW
                        //   The base relocation adds the low 16 bits of the difference to
                        //   the 16-bit field at offset. The 16-bit field represents the low
                        //   half of a 32-bit word.
                    {
                        address const  orig_value = read_uint32_t(reloc_address,load_props.sections_table());
                        address const  addon = difference & 0xffffULL;
                        if (addon + orig_value > (address)std::numeric_limits<uint32_t>::max())
                            return "Computed relocation address cannot be stored in 32bits.";
                        write_uint32_t(reloc_address, (uint32_t)(addon + orig_value), load_props.sections_table());
                        load_props.add_performed_relocation({
                                reloc_address,
                                4ULL,
                                "IMAGE_REL_BASED_LOW",
                                addon + orig_value,
                                orig_value,
                                "",
                                "Base relocation."
                                });
                    }
                    break;
                case 3: // IMAGE_REL_BASED_HIGHLOW
                        //   The base relocation applies all 32 bits of the difference to the
                        //   32-bit field at offset.
                    {
                        address const  orig_value = read_uint32_t(reloc_address,load_props.sections_table());
                        if (difference + orig_value > (address)std::numeric_limits<uint32_t>::max())
                            return "Computed relocation address cannot be stored in 32bits.";
                        write_uint32_t(reloc_address, (uint32_t)(difference + orig_value), load_props.sections_table());
                        load_props.add_performed_relocation({
                                reloc_address,
                                4ULL,
                                "IMAGE_REL_BASED_HIGHLOW",
                                difference + orig_value,
                                orig_value,
                                "",
                                "Base relocation."
                                });
                    }
                    break;
                case 4: // IMAGE_REL_BASED_HIGHADJ
                        //   The base relocation adds the high 16 bits of the difference to
                        //   the 16-bit field at offset. The 16-bit field represents the high
                        //   value of a 32-bit word. The low 16 bits of the 32-bit value are
                        //   stored in the 16-bit word that follows this base relocation.
                        //   This means that this base relocation occupies two slots.
                    return "Base relocation of the type 4 is NOT IMPLEMENTED YET!";
                    break;
                case 5: // IMAGE_REL_BASED_MIPS_JMPADDR
                        //   For MIPS machine types, the base relocation applies to a
                        //   MIPS jump instruction.
                        // IMAGE_REL_BASED_ARM_MOV32A
                        //   For ARM machine types, the base relocation applies the
                        //   difference to the 32-bit value encoded in the immediate
                        //   fields of a contiguous MOVW+MOVT pair in ARM mode at offset.
                    return "Base relocation of the type 5 is NOT IMPLEMENTED YET!";
                    break;
                case 6: // Reserved.
                    break;
                case 7: // IMAGE_REL_BASED_ARM_MOV32T
                        //  The base relocation applies the difference to the 32-bit value
                        //  encoded in the immediate fields of a contiguous MOVW+MOVT pair
                        //  in Thumb mode at offset.
                    return "Base relocation of the type 7 is NOT IMPLEMENTED YET!";
                    break;
                case 9: // IMAGE_REL_BASED_MIPS_JMPADDR16
                        //  The base relocaton applies to a MIPS16 jump instruction.
                    return "Base relocation of the type 9 is NOT IMPLEMENTED YET!";
                    break;
                case 10:// IMAGE_REL_BASED_DIR64
                        //  The base relocation applies the difference to the 64-bit field at offset.
                    {
                        address const  orig_value = read_address_64bit(reloc_address,load_props.sections_table());
                        write_address(reloc_address, difference + orig_value, load_props.sections_table());
                        load_props.add_performed_relocation({
                                reloc_address,
                                8ULL,
                                "IMAGE_REL_BASED_DIR64",
                                difference + orig_value,
                                orig_value,
                                "",
                                "Base relocation."
                                });
                    }
                    break;
                default:
                    return "Unknown base relocation type.";
                }
            }

            block_ptr += num_block_bytes;
        }
    }

    // Now we read the exptort table in order to collect all exported symbols (for other binaries)

    if (export_table_size > 0ULL)
    {
        if (export_table_address == 0ULL)
            return "The non-empty export table has wrong start address.";

        address const  export_table_begin = fixed_address + export_table_address;
        address const  export_table_end = export_table_begin + export_table_size;

        std::string  this_binary_name;
        {
            address const  this_binary_name_ptr =
                    fixed_address + read_uint32_t(export_table_begin + 12ULL,load_props.sections_table());
            if (!read_zero_terminated_string(this_binary_name_ptr,load_props.sections_table(),this_binary_name))
                return "Cannot read name of the this binary in the export table.";
        }

        uint32_t const  start_ordinal_index = read_uint32_t(export_table_begin + 16ULL,load_props.sections_table());
        uint32_t const  num_address_table_entries = read_uint32_t(export_table_begin + 20ULL,load_props.sections_table());
        uint32_t const  num_symbols = read_uint32_t(export_table_begin + 24ULL,load_props.sections_table());

        address const  address_table_begin =
                fixed_address + read_uint32_t(export_table_begin + 28ULL,load_props.sections_table());
        address const  name_pointers_table_begin =
                fixed_address + read_uint32_t(export_table_begin + 32ULL,load_props.sections_table());
        address const  ordinals_table_begin =
                fixed_address + read_uint32_t(export_table_begin + 36ULL,load_props.sections_table());

        for (uint32_t  i = 0U; i < num_symbols; ++i)
        {
            uint32_t const  symbol_index = read_uint16_t(ordinals_table_begin + i * 2U,load_props.sections_table());
            if (symbol_index >= num_address_table_entries)
                return "The export table contains an ordinal which refers to an entry beyond the end of the export address table.";

            std::string  symbol_name;
            if (address_table_begin > fixed_address)
            {
                address const  symbol_name_ptr =
                        fixed_address + read_uint32_t(name_pointers_table_begin + i * 4U,load_props.sections_table());
                if (symbol_name_ptr > fixed_address &&
                    !read_zero_terminated_string(symbol_name_ptr,load_props.sections_table(),symbol_name))
                    return "Cannot read a name of a symbol in the export table.";
            }

            pe_symbol_id const  id{ this_binary_name, symbol_index + start_ordinal_index, symbol_name };

            address  symbol_address = 0ULL;
            {
                symbol_address = fixed_address + read_uint32_t(address_table_begin + 4ULL * symbol_index,load_props.sections_table());
                if (symbol_address >= export_table_begin && symbol_address < export_table_end) // Is is a forwarded symbol?
                {
                    std::string  forward_symbol_name;
                    if (!read_zero_terminated_string(symbol_address,load_props.sections_table(),forward_symbol_name))
                        return "Cannot read a name of a forwarded symbol in the export table.";

                    std::string  binary_name,symbol_name;
                    uint32_t  ordinal;
                    std::string const error_message =
                            parse_forwarder(this_binary_name,forward_symbol_name,binary_name,ordinal,symbol_name);
                    if (!error_message.empty())
                        return error_message;
                    symbol_address = load_props.symbol_address({binary_name,ordinal,symbol_name});
                }
            }

            if (symbol_address != 0ULL) // Have we succeeded to compute symbol's address?
            {
                if (load_props.symbol_address(id) == 0ULL)
                    load_props.add_symbol(id,symbol_address);
                else if (symbol_address != load_props.symbol_address(id))
                    return "There is a symbol with more that one definition (the symbol lies on two addresses).";
            }
        }

        for (uint32_t  i = 0U; i < num_address_table_entries; ++i)
            if (load_props.symbol_address(this_binary_name,start_ordinal_index + i) == 0ULL)
            {
                address  symbol_address =
                        fixed_address + read_uint32_t(address_table_begin + 4ULL * i,load_props.sections_table());
                if (symbol_address >= export_table_begin && symbol_address < export_table_end) // Is is a forwarded symbol?
                {
                    std::string  forward_symbol_name;
                    if (!read_zero_terminated_string(symbol_address,load_props.sections_table(),forward_symbol_name))
                        return "Cannot read a name of a forwarded symbol in the export table.";

                    std::string  binary_name,symbol_name;
                    uint32_t  ordinal;
                    std::string const error_message =
                            parse_forwarder(this_binary_name,forward_symbol_name,binary_name,ordinal,symbol_name);
                    if (!error_message.empty())
                        return error_message;
                    symbol_address = load_props.symbol_address({binary_name,ordinal,symbol_name});

                    if (symbol_address != 0ULL)
                        load_props.add_symbol({this_binary_name,start_ordinal_index + i,""},symbol_address);
                }
            }
    }

    // Now we read the import table in order to resolve relocations of imported symbols (defined in other binaries)
    // This may imply a load of dependent binaries.

    if (import_table_size > 0ULL)
    {
        if (import_table_address == 0ULL)
            return "The non-empty import table has wrong start address.";

        address const  import_table_begin = fixed_address + import_table_address;
        address const  import_table_end = import_table_begin + import_table_size;

        for (address  import_dir_ptr = fixed_address + import_table_address; true; import_dir_ptr += 20ULL)
        {
            if (import_dir_ptr + 20ULL > import_table_end)
            {
                LOAD_PE_WARNING("The import directory of the import table goes beyond the end of the whole import table");
                break;
            }

            address const  lookup_table_begin = fixed_address + read_uint32_t(import_dir_ptr + 0ULL,load_props.sections_table());
            uint32_t const  time_stamp = read_uint32_t(import_dir_ptr + 4ULL,load_props.sections_table());
            uint32_t const  first_forward_index = read_uint32_t(import_dir_ptr + 8ULL,load_props.sections_table());
            address const  dll_name_ptr = fixed_address + read_uint32_t(import_dir_ptr + 12ULL, load_props.sections_table());
            address const  address_table_begin = fixed_address + read_uint32_t(import_dir_ptr + 16ULL,load_props.sections_table());

            if (lookup_table_begin == fixed_address &&
                time_stamp == 0U &&
                first_forward_index == 0U &&
                dll_name_ptr == fixed_address &&
                address_table_begin == fixed_address) // This is how the end of the directory have to be recognised.
                break;

            std::string  dll_name;
            if (!read_zero_terminated_string(dll_name_ptr,load_props.sections_table(),dll_name))
                return "Cannot read a DLL name of an imported symbol in the import table.";

            // Here we have to find and load the DLL library

            std::string const lib_file_name = fileutl::parse_name_in_pathname(dll_name);
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
                        std::string const  err_message = detail::load_pe(lib_pathname,load_props);
                        if (!err_message.empty())
                        {
                            if (err_message == NOT_PE_FILE())
                                return "The referenced library '" + lib_pathname + "' is not an PE file.";
                            return err_message;
                        }
                    }
                    load_props.add_file_dependence(pe_props->path(),lib_pathname);
                    lib_loaded = true;
                    break;
                }
            }
            if (!lib_loaded)
            {
                load_props.add_skipped_file(lib_file_name);
                load_props.add_file_dependence(pe_props->path(),lib_file_name);
            }

            // Now we perform address bindings of import symbols.

            for (uint32_t  i = 0U; true; ++i)
            {
                if (!find_section(lookup_table_begin + i * 4ULL,load_props.sections_table()).operator bool())
                {
                    LOAD_PE_WARNING("Import lookup table of the import directory 0x" << std::hex << import_dir_ptr << " does not point to any of loaded sections.");
                    break;
                }

                uint32_t const  lookup_value = read_uint32_t(lookup_table_begin + i * 4ULL,load_props.sections_table());
                if (lookup_value == 0U)
                    break;

                bool const  do_import_via_ordinal = (lookup_value & 0x80000000U) != 0U;

                address  imported_symbol_adr = 0ULL;
                uint32_t  imported_symbol_ordinal = 0U;
                std::string  imported_symbol_name;
                {
                    if (do_import_via_ordinal)
                    {
                        imported_symbol_ordinal = lookup_value & 0xffffU;
                        imported_symbol_adr = load_props.symbol_address(lib_file_name,imported_symbol_ordinal);
                    }
                    else
                    {
                        address const  hint_name_table_element_ptr = fixed_address + lookup_value;
                        if (!read_zero_terminated_string(hint_name_table_element_ptr + 2ULL,load_props.sections_table(),
                                                         imported_symbol_name))
                            return "Cannot read an import symbol name in the hint/name table.";
                        if (imported_symbol_name.empty())
                            return "The imported symbol name from the hint/name table is empty string. There is no chance to find its address.";
                        imported_symbol_adr = load_props.symbol_address(lib_file_name,imported_symbol_name);
                    }
                }
                if (imported_symbol_adr > (address)std::numeric_limits<uint32_t>::max())
                    return "The address of an imported symbol cannot be stored in 32bits.";

                if (imported_symbol_adr != 0ULL)
                {
                    write_uint32_t(address_table_begin + i * 4ULL,(uint32_t)imported_symbol_adr,load_props.sections_table());
                    load_props.add_performed_relocation({
                            address_table_begin + i * 4ULL,
                            4ULL,
                            do_import_via_ordinal ? "Import via ordinal." : "Import via symbol name.",
                            imported_symbol_adr,
                            lookup_value,
                            pe_symbol_id{lib_file_name,imported_symbol_ordinal,imported_symbol_name}.full_name(),
                            "Address binding of the import symbol."
                            });
                }
                else
                    load_props.add_skipped_relocation({
                            address_table_begin + i * 4ULL,
                            4ULL,
                            do_import_via_ordinal ? "Import via ordinal." : "Import via symbol name.",
                            0ULL,
                            lookup_value,
                            pe_symbol_id{lib_file_name,imported_symbol_ordinal,imported_symbol_name}.full_name(),
                            "The address of the imported symbol is unknown."
                            });
            }
        }

    }

    // And now we store properties of special sections

    if (thread_local_storage_table_size > 0ULL)
        load_props.add_special_section(pe_props->path(),
                                       special_section_name::thread_local_storage(),
                                       special_section_properties_ptr{ new special_section_properties{
                                                fixed_address + thread_local_storage_table_address,
                                                fixed_address + thread_local_storage_table_address + thread_local_storage_table_size,
                                                "Thread local storage."
                                                }}
                                       );
    if (resource_table_size > 0ULL)
        load_props.add_special_section(pe_props->path(),
                                       special_section_name::resources(),
                                       special_section_properties_ptr{ new special_section_properties{
                                                fixed_address + resource_table_address,
                                                fixed_address + resource_table_address + resource_table_size,
                                                "Resources like icons, images, texts, etc."
                                                }}
                                       );
    if (load_config_table_size > 0ULL)
    {
        special_section_properties_ptr  section_props =
                special_section_properties_ptr( new special_section::load_configuration_structure{
                        fixed_address + load_config_table_address,
                        fixed_address + load_config_table_address + load_config_table_size
                        });
        {
            address const  ptr = fixed_address + load_config_table_address;
            address const  end = ptr + load_config_table_size;

            if (load_config_table_size < 64ULL)
                LOAD_PE_WARNING("The load config structure is too short.")
            else if (load_props.platform()->architecture() == architecture_t::X86_32 && load_config_table_size < 72ULL)
                LOAD_PE_WARNING("The load config structure has wrong size.")
            else if (!find_section(ptr,load_props.sections_table()).operator bool() ||
                     !find_section(end - 1ULL,load_props.sections_table()).operator bool())
                LOAD_PE_WARNING("The load config structure does not fit into the loaded sections.")
            else
            {
                std::time_t const  timestamp = (std::time_t)read_uint32_t(ptr + 4ULL,load_props.sections_table());
                uint16_t const  major_version = read_uint16_t(ptr + 8ULL,load_props.sections_table());
                uint16_t const  minor_version = read_uint16_t(ptr + 10ULL,load_props.sections_table());
                uint32_t const  global_clear_flags = read_uint32_t(ptr + 12ULL,load_props.sections_table());
                uint32_t const  global_set_flags = read_uint32_t(ptr + 16ULL,load_props.sections_table());
                uint32_t const  critical_section_timeout = read_uint32_t(ptr + 20ULL,load_props.sections_table());
                uint64_t const  decommit_block_thresold = read_uint32_t(ptr + 24ULL,load_props.sections_table());
                uint64_t const  decommit_free_thresold = read_uint32_t(ptr + 28ULL,load_props.sections_table());
                address const  lock_prefix_table =
                        load_props.platform()->architecture() == architecture_t::X86_32 ?
                                read_uint32_t(ptr + 32ULL,load_props.sections_table())
                                : 0ULL;
                uint64_t const  max_alloc_size = read_uint32_t(ptr + 36ULL,load_props.sections_table());
                uint64_t const  memory_thresold = read_uint32_t(ptr + 40ULL,load_props.sections_table());
                uint64_t const  process_affinity_mask = read_uint32_t(ptr + 44ULL,load_props.sections_table());
                uint32_t const  process_heap_flags = read_uint32_t(ptr + 48ULL,load_props.sections_table());
                uint16_t const  service_pack_version = read_uint16_t(ptr + 52ULL,load_props.sections_table());
                address const  security_cookie = read_uint32_t(ptr + 60ULL,load_props.sections_table());
                std::vector<address>  structured_exception_handlers;
                {
                    address const  seh_begin  = read_uint32_t(ptr + 64ULL,load_props.sections_table());
                    uint64_t const  seh_count  = read_uint64_t(ptr + 68ULL,load_props.sections_table());
                    for (address  seh_ptr = seh_begin; seh_ptr < seh_begin + seh_count * 4ULL; seh_ptr += 4ULL)
                        if (!find_section(seh_ptr,load_props.sections_table()).operator bool())
                            LOAD_PE_WARNING("A structured exception handler does not lies inside the loaded sections.")
                        else
                            structured_exception_handlers.push_back(
                                        fixed_address + read_uint32_t(seh_ptr,load_props.sections_table())
                                        );
                }

                section_props = special_section_properties_ptr( new special_section::load_configuration_structure{
                                        fixed_address + load_config_table_address,
                                        fixed_address + load_config_table_address + load_config_table_size,
                                        timestamp,
                                        major_version,
                                        minor_version,
                                        global_clear_flags,
                                        global_set_flags,
                                        critical_section_timeout,
                                        decommit_block_thresold,
                                        decommit_free_thresold,
                                        lock_prefix_table,
                                        max_alloc_size,
                                        memory_thresold,
                                        process_affinity_mask,
                                        process_heap_flags,
                                        service_pack_version,
                                        security_cookie,
                                        structured_exception_handlers
                                        });
            }
        }
        INVARIANT(section_props.operator bool());
        load_props.add_special_section(pe_props->path(),special_section_name::load_configuration_structure(),section_props);
    }

    (void)dll_characteristics;

    (void)bound_import_table_address;
    (void)bound_import_table_size;

    return "";
}


}}
