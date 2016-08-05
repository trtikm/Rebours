#include <rebours/MAL/loader/file_props.hpp>
#include <rebours/MAL/loader/assumptions.hpp>

namespace loader { namespace format {


std::string  ELF() { return "ELF"; }
std::string  PE() { return "PE"; }
std::string  MACH() { return "Mach-O"; }


}}

namespace loader { namespace file_properties {


std::string  file_type() { return "File type"; }

std::string  abi_loader() { return "ABI Loader"; }

std::string  os_version() { return "OS version"; }
std::string  os_major_version() { return "OS major"; }
std::string  os_minor_version() { return "OS minor"; }

std::string  subsytem() { return "OS subsystem"; }
std::string  subsytem_major_version() { return "Subsystem major"; }
std::string  subsytem_minor_version() { return "Subsystem minor"; }

std::string  max_stack_size() { return "Max stack size"; }
std::string  max_local_heap_size() { return "Max local heap size"; }


}}

namespace loader { namespace file_types {


std::string  executable() { return "Executable"; }
std::string  library() { return "Library"; }


}}

namespace loader { namespace abi_loaders {


std::string  NONE() { return "NONE"; }
std::string  ld_linux_x86_64_so_2() { return "/lib64/ld-linux-x86-64.so.2"; }
std::string  ld_linux_x86_32_so_2() { return "/lib/ld-linux.so.2"; }
std::string  ld_linux_uClibc_so_0() { return "/lib/ld-uClibc.so.0"; }
std::string  windows() { return "Windows"; }
std::string  darwin() { return "/usr/lib/dyld"; }


}}

namespace loader { namespace subsystems {


std::string  native() { return "Driver/Native"; }
std::string  GUI() { return "GUI"; }
std::string  CUI() { return "CUI"; }
std::string  posix_CUI() { return "Posix CUI"; }
std::string  windows_ce() { return "Windows CE"; }
std::string  EFI() { return "EFI"; }


}}

namespace loader {


file_props::file_props(std::string const&  path,
                       std::string const&  format,
                       bool const  is_in_big_endian,
                       uint8_t const  num_address_bits,
                       uint32_t const  id,
                       file_property_map_ptr const  file_property_map)
    : m_path(path)
    , m_format(format)
    , m_is_in_big_endian(is_in_big_endian)
    , m_num_address_bits(num_address_bits)
    , m_id(id)
    , m_file_property_map(!file_property_map.operator bool() ? file_property_map_ptr(new loader::file_property_map{}) :
                                                               file_property_map)
{
    ASSUMPTION(m_id != 0U);
}

bool  file_props::has_property(std::string const&  property_name) const
{
    return m_file_property_map->count(property_name) != 0ULL;
}

bool  file_props::has_property_value(std::string const&  property_name, std::string const&  value_name) const
{
    return has_property(property_name) && property(property_name) == value_name;
}

std::string const&  file_props::property(std::string const&  property_name) const
{
    ASSUMPTION(has_property(property_name));
    return m_file_property_map->at(property_name);
}

}
