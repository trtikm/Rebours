#include <rebours/MAL/loader/detail/load_pe.hpp>
#include <rebours/MAL/loader/detail/set_file_property.hpp>
#include <rebours/MAL/loader/file_utils.hpp>
#include <rebours/MAL/loader/to_string.hpp>
#include <rebours/MAL/loader/assumptions.hpp>
#include <rebours/MAL/loader/invariants.hpp>
#include <fstream>
#include <algorithm>

namespace loader { namespace detail {


std::pair<file_props_ptr,coff_standard_header_info>
load_pe_file_props(std::string const&  pe_file, std::ifstream&  pe, load_props_pe&  load_props, std::string& error_message)
{
    if (file_size(pe_file) < 2ULL)
    {
        error_message = NOT_PE_FILE();
        return {};
    }

    // We read magic number in order to reconse the file type (is it PE or not).

    {
        char magic[2U];
        pe.read(magic,2U);

        if (magic[0] != 'M' ||
            magic[1] != 'Z' )
        {
            error_message = NOT_PE_FILE();
            return {};
        }
    }

    // Well, it is a PE file. Let us load it!

    bool const is_file_in_big_endian = false; // PE file is stored in little-endian by default.
    bool const  is_it_load_of_root_binary = load_props.root_file() == pe_file;

    uint32_t  number_of_sections = 0U;
    uint32_t  optional_header_offset = 0U;
    uint32_t  size_of_optional_header = 0U;
    bool  is_executable_file = false;
    bool  is_dynamic_library = false;
    bool  supports_64bit_addressing = false;
    bool  is_32bit_machine = false;

    // First we have to locate and jump to the COFF header

    {
        pe.seekg(0x3cULL); // Skip most of the DOS header (go to pointer to COFF header)

        uint32_t const  pe_header_offset = ::read_bytes_to_uint32_t(pe,4U,is_file_in_big_endian);
        if (pe_header_offset + 0x17ULL >= file_size(pe_file))
        {
            error_message = "The COFF header passes the end of the file.";
            return {};
        }

        pe.seekg(pe_header_offset); // Jump to the begin of the COFF header.
    }

    // Now we read the COFF header

    {
        uint32_t const  pe_signature = ::read_bytes_to_uint32_t(pe,4U,is_file_in_big_endian);
        if (pe_signature != 0x4550U) // ASCII "PE\0\0"
        {
            error_message = "Wrong PE signature.";
            return {};
        }

        architecture_t arch;
        uint32_t const  arch_id = ::read_bytes_to_uint32_t(pe,2U,is_file_in_big_endian);
        switch (arch_id)
        {
        case 0x8664U:
            arch = architecture_t::X86_64; break;
        case 0x14cU:
            arch = architecture_t::X86_32; break;
        default:
            error_message = "Unknown architecture type.";
            return {};
        }

	platform_ptr const  file_platform{ new platform(arch, abi_t::WINDOWS) };
        if (is_it_load_of_root_binary)
            load_props.set_platform( file_platform );
        else
        {
            ASSUMPTION(load_props.platform().operator bool());
        }

        number_of_sections = ::read_bytes_to_uint32_t(pe,2U,is_file_in_big_endian);

        skip_bytes(pe,12U); // We skip the time-stamp and two deprecated fields.

        size_of_optional_header = ::read_bytes_to_uint32_t(pe,2U,is_file_in_big_endian);

        uint32_t const  characteristics = ::read_bytes_to_uint32_t(pe,2U,is_file_in_big_endian);

        is_executable_file = (characteristics & 0x2U) != 0U;
        is_dynamic_library = (characteristics & 0x2000U) != 0U;
        if (!is_executable_file && !is_dynamic_library)
        {
            error_message = "The PE file does not represent executable nor dynamic library.";
            return {};
        }

        supports_64bit_addressing = (characteristics & 0x20U) != 0U;
        is_32bit_machine = (characteristics & 0x100U) != 0U;
        if (supports_64bit_addressing && is_32bit_machine)
            LOAD_PE_WARNING_IN(pe_file,"The PE file is inconsistent. It indicates a 32-bit machine with 64-bit addressing.");

        optional_header_offset = (uint32_t)pe.tellg();
        if (optional_header_offset + size_of_optional_header > file_size(pe_file))
        {
            error_message = "The optional header goes beyond the end of file.";
            return {};
        }
    }

    uint8_t  num_address_bits = 0U;
    uint32_t  size_of_code = 0U;
    uint32_t  size_of_initialised_data = 0U;
    uint32_t  size_of_uninitialised_data = 0U;
    uint64_t  address_of_entry_point = 0ULL;
    uint64_t  base_of_code = 0ULL;
    uint64_t  base_of_data = 0ULL;

    // Now we read the optional header (although it is called optional, it must always be present)

    {
        uint32_t const  magic = ::read_bytes_to_uint32_t(pe,2U,is_file_in_big_endian);
        switch (magic)
        {
        case 0x10bU: // PE32
            num_address_bits = 32U; break;
        case 0x20bU: // PE32+ (64-bit addressing)
            num_address_bits = 64U; break;
        default:
            error_message = "Unknown PE image type (neither 32- nor 64-bits PE).";
            return {};
        }
        if (num_address_bits == 64U && !supports_64bit_addressing)
        {
            error_message = "The file specifies 64-bits address mode without support of 64-bit addressing.";
            return {};
        }

        skip_bytes(pe,2ULL); // We do not use neither major nor minor linker version.

        size_of_code = ::read_bytes_to_uint32_t(pe,4U,is_file_in_big_endian);
        size_of_initialised_data = ::read_bytes_to_uint32_t(pe,4U,is_file_in_big_endian);
        size_of_uninitialised_data = ::read_bytes_to_uint32_t(pe,4U,is_file_in_big_endian);
        address_of_entry_point = ::read_bytes_to_uint64_t(pe,4U,is_file_in_big_endian);
        base_of_code = ::read_bytes_to_uint64_t(pe,4U,is_file_in_big_endian);

        if (num_address_bits == 32U) // The 'base_of_data' filed is missing in 64-bit mode.
            base_of_data = ::read_bytes_to_uint64_t(pe,4U,is_file_in_big_endian);

        if (address_of_entry_point < base_of_code || address_of_entry_point >= base_of_code + size_of_code)
        {
            error_message = "The entry point lies outside the code section.";
            return {};
        }
    }

    static uint32_t file_id_generator = 0U; // TODO: This is not an ideal implementation: move it to load_props.
    file_props_ptr const  pe_props { new file_props{
            normalise_path(absolute_path(pe_file)),
            format::PE(),
            is_file_in_big_endian,
            num_address_bits,
            ++file_id_generator
            }};
    set_file_property(pe_props->property_map(), file_properties::file_type(),
                      is_dynamic_library ? file_types::library() : file_types::executable());

    load_props.add_file_props(pe_props);

    return { pe_props,
             {
                number_of_sections,
                optional_header_offset,
                size_of_optional_header,
                size_of_code,
                size_of_initialised_data,
                size_of_uninitialised_data,
                address_of_entry_point,
                base_of_code,
                base_of_data
             }
           };
}

std::string  load_pe(std::string const& pe_file, load_props_pe&  load_props)
{
    ASSUMPTION( file_exists(pe_file) );
    ASSUMPTION( load_props.files_table()->count(pe_file) == 0U );

    std::ifstream  pe{pe_file,std::ifstream::binary};
    std::string  error_message;
    std::pair<file_props_ptr,detail::coff_standard_header_info> file_info =
            load_pe_file_props(pe_file,pe,load_props,error_message);
    if (!error_message.empty())
        return error_message == NOT_PE_FILE() ? std::string("The loaded file '") + pe_file + "' is not a PE file." :
                                                 error_message ;
    return file_info.first->num_address_bits() == 32U ?
                detail::load_pe_32bit(pe,file_info.first,file_info.second,load_props) :
                detail::load_pe_64bit(pe,file_info.first,file_info.second,load_props) ;
}

symbol_table_ptr  build_visible_symbols_table(load_props_pe const&  load_props)
{
    (void)load_props;
    // TODO!
    return symbol_table_ptr();
}


}}
