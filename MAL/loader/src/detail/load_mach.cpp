#include <rebours/MAL/loader/detail/load_mach.hpp>
#include <rebours/MAL/loader/detail/set_file_property.hpp>
#include <rebours/utility/file_utils.hpp>
#include <rebours/utility/to_string.hpp>
#include <rebours/utility/endian.hpp>
#include <rebours/utility/assumptions.hpp>
#include <rebours/utility/invariants.hpp>
#include <fstream>
#include <algorithm>

namespace loader { namespace detail {

std::pair<file_props_ptr,mach_header_props>
load_mach_file_props(std::string const&  mach_file, std::ifstream&  mach,
                     load_props_mach&  load_props, std::string& error_message)
{
    if (fileutl::file_size(mach_file) < 4ULL)
    {
        error_message = NOT_MACH_FILE();
        return {};
    }

    bool const is_file_in_big_endian = false; // MACH file is stored in big-endian by default.
    bool const  is_it_load_of_root_binary = load_props.root_file() == mach_file;

    // We read magic number in order to reconse the file type (is it MACH or not).

    uint8_t  num_address_bits = 0U;
    {
        uint32_t const  magic = fileutl::read_bytes_to_int32_t(mach,4U,is_file_in_big_endian);
        if (magic == 0xfeedfaceU) // Mach-O 32 bit
            num_address_bits = 32U;
        else if (magic == 0xfeedfacfU)  // Mach-O 64 bit
            num_address_bits = 64U;
        else if (magic == 0xcafebabeU)  // FAT binary
        {
            error_message = "Mac OS fat binaries are NOT IMPLEMENTED YET!";
            return {};
        }
        else
        {
            error_message = NOT_MACH_FILE();
            return {};
        }
    }

    // Now we read the rest of the header

    uint32_t const  cpu_type = fileutl::read_bytes_to_int32_t(mach,4U,is_file_in_big_endian);

    //uint32_t const  cpu_subtype = read_bytes_to_int32_t(mach,4U,is_file_in_big_endian);
    fileutl::skip_bytes(mach,4U); // We ignore cpu-sub-type.

    if ((num_address_bits == 64U && cpu_type != 0x1000007) ||
        (num_address_bits == 32U && cpu_type != 0x7) )
    {
        error_message = "Unsupported processor architecture.";
        return {};
    }

    architecture_t const arch = num_address_bits == 32U ? architecture_t::X86_32 :
                                                          architecture_t::X86_64 ;
    abi_t const abi = abi_t::DARWIN;

    platform_ptr const  file_platform{ new platform(arch, abi) };

    uint32_t const  filetype = fileutl::read_bytes_to_int32_t(mach,4U,is_file_in_big_endian);

    if (filetype != 0x2U && filetype != 0x3U && filetype != 0x6U)
    {
        error_message = "The loaded Mach-O file is neither executable nor dynamic library.";
        return {};
    }

    bool const  is_dynamic_library = filetype == 0x3U || filetype == 0x6U;
    bool const  is_fixed_dynamic_library = filetype == 0x3U;

    uint32_t const  num_load_commnads = fileutl::read_bytes_to_int32_t(mach,4U,is_file_in_big_endian);

    if (num_load_commnads == 0U)
    {
        error_message = "There is no load command in the binary.";
        return {};
    }

    uint32_t const  total_num_commnad_bytes = fileutl::read_bytes_to_int32_t(mach,4U,is_file_in_big_endian);

    if (total_num_commnad_bytes < num_load_commnads * (num_address_bits / 8U))
    {
        error_message = "The list of load commands is too short.";
        return {};
    }

    fileutl::skip_bytes(mach,4ULL); // We ignore flags.

    if (num_address_bits == 64U)
        fileutl::skip_bytes(mach,4ULL); // We skip the reserved word.

    if ((uint64_t)mach.tellg() + (uint64_t)total_num_commnad_bytes >= fileutl::file_size(mach_file))
    {
        error_message = "The list of load commands goes beyond the end of the file.";
        return {};
    }

    // Next we create file properties data structure and we return results.

    static uint32_t file_id_generator = 0U;  // TODO: This is not an ideal implementation: move it to load_props.
    file_props_ptr const  mach_props { new file_props{
            fileutl::normalise_path(fileutl::absolute_path(mach_file)),
            format::MACH(),
            is_file_in_big_endian,
            num_address_bits,
            ++file_id_generator
            }};
    set_file_property(mach_props->property_map(), file_properties::file_type(),
                      is_dynamic_library ? file_types::library() : file_types::executable());

    load_props.add_file_props(mach_props);

    if (is_it_load_of_root_binary)
        load_props.set_platform( file_platform );
    if (*file_platform != *load_props.platform())
    {
        error_message = "Incompatible platforms of the executable and its shared library: " + mach_props->path();
        return {};
    }

    return {mach_props,std::make_tuple(num_load_commnads,total_num_commnad_bytes,is_fixed_dynamic_library)};
}


std::string  load_mach(std::string const& mach_file, load_props_mach&  load_props)
{
    ASSUMPTION( fileutl::file_exists(mach_file) );
    ASSUMPTION( load_props.files_table()->count(mach_file) == 0U );

    std::ifstream  mach{mach_file,std::ifstream::binary};
    std::string  error_message;
    auto const  mach_props = load_mach_file_props(mach_file,mach,load_props,error_message);
    if (!error_message.empty())
        return error_message == NOT_MACH_FILE() ? std::string("The loaded file '") + mach_file + "' is not a MACH file." :
                                                 error_message ;
    return mach_props.first->num_address_bits() == 32U ?
                detail::load_mach_32bit(mach,mach_props.first,
                                        std::get<0>(mach_props.second),std::get<1>(mach_props.second),
                                        std::get<2>(mach_props.second),
                                        load_props) :
                detail::load_mach_64bit(mach,mach_props.first,
                                        std::get<0>(mach_props.second),std::get<1>(mach_props.second),
                                        std::get<2>(mach_props.second),
                                        load_props) ;
}


symbol_table_ptr  build_visible_symbols_table(load_props_mach const&  load_props)
{
    (void)load_props;
    // TODO!
    return symbol_table_ptr();
}


}}
