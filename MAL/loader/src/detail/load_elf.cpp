#include <rebours/MAL/loader/detail/load_elf.hpp>
#include <rebours/MAL/loader/detail/set_file_property.hpp>
#include <rebours/MAL/loader/file_utils.hpp>
#include <rebours/MAL/loader/assumptions.hpp>
#include <rebours/MAL/loader/invariants.hpp>
#include <fstream>
#include <algorithm>

namespace loader { namespace detail {


file_props_ptr  load_elf_file_props(std::string const&  elf_file, std::ifstream&  elf,
                                    load_props_elf&  load_props, std::string& error_message)
{
    if (file_size(elf_file) < 6ULL)
    {
        error_message = NOT_ELF_FILE();
        return file_props_ptr();
    }

    char magic[4U];
    elf.read(magic,4U);

    if (magic[0] != 0x7F ||
        magic[1] != 'E' ||
        magic[2] != 'L' ||
        magic[3] != 'F' )
    {
        error_message = NOT_ELF_FILE();
        return file_props_ptr();
    }

    elf.seekg(0x05ULL);
    uint8_t  endian_mode = ::read_byte(elf);
    if (endian_mode != 1 && endian_mode != 2)
    {
        elf.seekg(0x10ULL);
        uint32_t const  binary_type_little_endian = ::read_bytes_to_uint32_t(elf,2U,false);
        elf.seekg(0x10ULL);
        uint32_t const  binary_type_big_endian = ::read_bytes_to_uint32_t(elf,2U,true);
        bool const  little_cond = binary_type_little_endian == 2U || binary_type_little_endian == 3U;
        bool const  big_cond = binary_type_big_endian == 2U || binary_type_big_endian == 3U;
        if (little_cond && !big_cond)
        {
            endian_mode =  1U;
            LOAD_ELF_WARNING_IN(
                        elf_file,
                        "[Header offset 0x05] Unknown type of endian mode (neither little nor big). "
                        "The little endian was deduced form 2 bytes at the offset 0x10."
                        );
        }
        else if (!little_cond && big_cond)
        {
            endian_mode =  2U;
            LOAD_ELF_WARNING_IN(
                        elf_file,
                        "[Header offset 0x05] Unknown type of endian mode (neither little nor big). "
                        "The big endian was deduced form 2 bytes at the offset 0x10."
                        );
        }
        else
        {
            error_message = "[Header offset 0x05] Unknown type of endian mode (neither little nor big).";
            return file_props_ptr();
        }
    }
    bool const is_file_in_big_endian = endian_mode == 1 ? false : true;

    elf.seekg(0x04ULL);
    uint8_t  address_mode = ::read_byte(elf);
    if (address_mode != 1 && address_mode != 2)
    {
        elf.seekg(0x1CULL);
        uint32_t const  phd_offset_32 = ::read_bytes_to_uint32_t(elf,4U,is_file_in_big_endian);
        uint64_t const  phd_offset_64 = ::read_bytes_to_uint64_t(elf,8U,is_file_in_big_endian);
        bool const  cond32 = phd_offset_32 != 0U && phd_offset_32 < file_size(elf_file);
        bool const  cond64 = phd_offset_64 != 0U && phd_offset_64 < file_size(elf_file);
        if (cond32 && !cond64)
        {
            address_mode =  1U;
            LOAD_ELF_WARNING_IN(
                        elf_file,
                        "[Header offset 0x04] Unknown type for address mode (neither 32- nor 64-bit). "
                        "The 32-bit address mode was deduced form the 12 bytes at the offset 0x1c."
                        );
        }
        else if (!cond32 && cond64)
        {
            address_mode =  2U;
            LOAD_ELF_WARNING_IN(
                        elf_file,
                        "[Header offset 0x04] Unknown type for address mode (neither 32- nor 64-bit). "
                        "The 64-bit address mode was deduced form the 12 bytes at the offset 0x1c."
                        );
        }
        else
        {
            error_message = "[ELF][Header offset 0x04] Unknown type for address mode (neither 32- nor 64-bit).";
            return file_props_ptr();
        }
    }
    bool const file_uses_32_bit_addresses = address_mode == 1 ? true : false;

    elf.seekg(0x06ULL);

    file_props_ptr const  elf_props{ new file_props{
            normalise_path(absolute_path(elf_file)),
            format::ELF(),
            is_file_in_big_endian,
            (uint8_t)(file_uses_32_bit_addresses ? 32U : 64U),
            load_props.get_fresh_file_id()
            }};
    set_file_property(elf_props->property_map(), file_properties::abi_loader(), abi_loaders::NONE());

    return elf_props;
}


std::string  load_elf(std::string const& elf_file, load_props_elf&  load_props)
{
    ASSUMPTION( file_exists(elf_file) );
    ASSUMPTION( load_props.files_table()->count(elf_file) == 0U );

    std::ifstream  elf{elf_file,std::ifstream::binary};
    std::string  error_message;
    file_props_ptr const  elf_props = load_elf_file_props(elf_file,elf,load_props,error_message);
    if (!error_message.empty())
        return error_message == NOT_ELF_FILE() ? std::string("The loaded file '") + elf_file + "' is not an ELF file." :
                                                 error_message ;
    return elf_props->num_address_bits() == 32U ? detail::load_elf_32bit(elf,elf_props,load_props) :
                                                  detail::load_elf_64bit(elf,elf_props,load_props) ;
}


}}
