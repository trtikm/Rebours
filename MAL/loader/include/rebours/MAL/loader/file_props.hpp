#ifndef REBOURS_MAL_LOADER_FILE_PROPS_HPP_INCLUDED
#   define REBOURS_MAL_LOADER_FILE_PROPS_HPP_INCLUDED

#   include <cstdint>
#   include <map>
#   include <string>
#   include <set>
#   include <memory>

namespace loader { namespace format {


/**
 * Here are string constnats identifying particular types of binary files (their format).
 */

std::string  ELF();
std::string  PE();
std::string  MACH();


}}

namespace loader { namespace file_properties {


/**
 * These string constants identify *kinds* of properties which may appear in a binary
 * file. One string constant may be valid for several formats of binary files. Typically,
 * different types of binary files use different subsets of these constants, but these
 * subsets may overlap.
 *
 * These kinds are used as keys into a dictionary storing strings representing concrete
 * values the these kinds. See 'file_property_map' defined bellow.
 */

/**
 * Possible values for this kind of property are string constants listed in the namespace loader::file_types bellow.
 */
std::string  file_type();

/**
 * Possible values for this kind of property are string constants listed in the namespace loader::abi_loaders bellow.
 * This property is valid only for ELF files.
 */
std::string  abi_loader();

/**
 * Possible values for these kinds are string constants. Their strucure is dependent on a format of the binary file.
 * These properties are valid only for MACH-O files.
 */
std::string  os_version();
std::string  os_major_version();
std::string  os_minor_version();

/**
 * Possible values for this kind are string constants listed in the namespace loader::subsystems bellow.
 * This property is valid only for PE files.
 */
std::string  subsytem();

/**
 * Possible values for these kinds are string constants. Their strucure is dependent on a format of the binary file.
 * These properties are valid only for PE files.
 */
std::string  subsytem_major_version();
std::string  subsytem_minor_version();

/**
 * Possible values for these kinds are string representaition of natural numbers denoting number of bytes for that property.
 * These properties are valid only for PE files.
 */
std::string  max_stack_size();
std::string  max_local_heap_size();


}}

namespace loader { namespace file_types {


/**
 * These string constants represent possible values for the property kind 'file_type' in the namespace loader::file_properties
 */

std::string  executable();
std::string  library();


}}

namespace loader { namespace abi_loaders {


/**
 * These string constants represent possible values for the property kind 'abi_loader' in the namespace loader::file_properties
 */

std::string  NONE();
std::string  ld_linux_x86_64_so_2();    // The standard Linux ABI loader 64 bit
std::string  ld_linux_x86_32_so_2();    // The standard Linux ABI loader 32 bit
std::string  ld_linux_uClibc_so_0();    // ABI loader for embedded systems Linux 64bit (uses uClib instead of glibc)
std::string  windows();
std::string  darwin();


}}

namespace loader { namespace subsystems {


/**
 * These string constants represent possible values for the property kind 'subsytem' in the namespace loader::file_properties
 */

std::string  native();
std::string  GUI();
std::string  CUI();
std::string  posix_CUI();
std::string  windows_ce();
std::string  EFI();


}}

namespace loader {


/**
 * This dictionary is supposed to store properties retrieved from a loaded binary file. Keys
 * into this dictionary are string constants defined in the namespace loader::file_properties and
 * values are strings. See the namespace loader::file_properties for more info about both keys
 * and their possible values.
 */
typedef std::map<std::string,std::string>  file_property_map;
typedef std::shared_ptr<file_property_map const>  file_property_map_ptr;


/**
 * It stores all information about a loaded binary file, which is absolutelly necessary to even
 * succesfully read the file (like its format type, endianness, address mode 32/64bit) and
 * also important ABI-specific information for an underlying system (like abi-loaders, used
 * subsystems, OS version, maximal sizes of stack and heap).
 *
 * All data are read-only.
 */
struct file_props
{
    file_props(std::string const&  path,    //!< A full disc path-name of a binary file
               std::string const&  format,  //!< A string constant from the namespace loader::format
               bool const  is_in_big_endian,
               uint8_t const  num_address_bits, //!< It is an address mode. Typically, 32 or 64.
               uint32_t const  id,    //!< A non-zero unique identifiers amongst all loaded files.
               file_property_map_ptr const  file_property_map = {});
    std::string const&  path() const { return m_path; }
    std::string const&  format() const { return m_format; }
    bool  is_in_big_endian() const { return m_is_in_big_endian; }
    uint8_t  num_address_bits() const { return m_num_address_bits; }
    uint32_t  id() const { return m_id; }
    file_property_map_ptr  property_map() const { return m_file_property_map; }
    bool  has_property(std::string const&  property_name) const;
    bool  has_property_value(std::string const&  property_name, std::string const&  value_name) const;
    std::string const&  property(std::string const&  property_name) const;

private:
    std::string  m_path;
    std::string  m_format;
    bool  m_is_in_big_endian;
    uint8_t  m_num_address_bits;
    uint32_t  m_id;
    file_property_map_ptr  m_file_property_map;
};

typedef std::shared_ptr<file_props const>  file_props_ptr;


/**
 * It is a dictionary whose keys are disc path-names of loaded binary files and
 * values are descriptions of those files (i.e. instances of the structure defined above).
 * Note that more that one file may be loaded for a given binary file, because the
 * binary may reference dynamic libraries which should be thus loaded as well.
 */
typedef std::map<std::string,file_props_ptr> files_table;
typedef std::shared_ptr<files_table const>  files_table_ptr;


/**
 * It is a set of those path-names of binary files which were NOT loaded together with
 * a given binary file although the binary depends on them. There are two reasons for
 * a binary file to be skipped during the load: either its name is listed in a given
 * 'ignore list' or it lies outside all given 'search directories'. See the header file
 * Loader/load.hpp for more details.
 */
typedef std::set<std::string>  skipped_files;
typedef std::shared_ptr<skipped_files const>  skipped_files_ptr;


}

#endif
