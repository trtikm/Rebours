#ifndef REBOURS_MAL_DESCRIPTOR_STORAGE_HPP_INCLUDED
#   define REBOURS_MAL_DESCRIPTOR_STORAGE_HPP_INCLUDED

#   include <rebours/MAL/loader/descriptor.hpp>
#   include <string>
#   include <tuple>
#   include <unordered_map>
#   include <map>

namespace mal { namespace descriptor {


using  processor_architecture = loader::architecture_t;
using  operating_system = loader::abi_t;
using  file_descriptor = loader::descriptor;
using  file_descriptor_ptr = std::shared_ptr<file_descriptor const>;

using  address_range = std::pair< uint64_t,     // start address
                                  uint64_t      // end address
                                  >;

struct storage;


struct stack_section
{
    stack_section(uint64_t const  start_address,
                  uint64_t const  end_address,
                  bool const  has_read_access,
                  bool const  has_write_access,
                  bool const  has_execute_access,
                  bool const  is_in_big_endian,
                  bool const  has_const_endian);

    uint64_t  start_address() const { return m_start_address; }
    uint64_t  end_address() const { return m_end_address; }

    bool has_read_access() const { return m_has_read_access; }
    bool has_write_access() const { return m_has_write_access; }
    bool has_execute_access() const { return m_has_execute_access; }

    bool is_in_big_endian() const { return m_is_in_big_endian; }
    bool has_const_endian() const { return m_has_const_endian; }

private:

    uint64_t  m_start_address;
    uint64_t  m_end_address;
    bool  m_has_read_access;
    bool  m_has_write_access;
    bool  m_has_execute_access;
    bool  m_is_in_big_endian;
    bool  m_has_const_endian;
};


/**
 * An environment variable is a pair consisiting of a name of the variable and string representation of its value.
 */
using  environment_variable_type = std::pair< std::string,     //!< Name of the variable
                                              std::string      //!< Value of the variable
                                              >;

/**
 * It contains an input data for a Prologue program. The program reads these data from
 * the standard input stream #1 and copies them to proper places on the process stack.
 * Now we show how these data must be organised in (i.e. fed into) the stream #1 so that
 * the Prologue program can read them properly. Let 'sid' be an instance of 'stack_init_data'.
 * Then the stream #1 must start with these data:
 *
 * 4 bytes    // to_big_endian( (uint32_t)sid.first.size() )
 * N bytes    // All the strings in sid.first one by one, each string must be terminated by '\0'.
 * 4 bytes    // to_big_endian( (uint32_t)sid.second.size() )
 * M bytes    // All the strings in sid.second one by one, each string must be terminated by '\0'.
 */
using  stack_init_data = std::pair< std::vector<std::string>,   //!< Command line arguments
                                    std::vector<environment_variable_type> //!< Environment variables
                                    >;

/**
 * It converts the input stack init data into a sequence of bytes representing a content
 * of the standard input stream #1 for a Prologue program, as explained in the description
 * of the 'stack_init_data' above.
 */
void  linearise(stack_init_data const&  sid, std::vector<uint8_t>&  output);


struct storage
{
    explicit storage(file_descriptor_ptr const  descriptor);

    processor_architecture  processor() const noexcept { return m_processor; }
    operating_system  system() const noexcept { return m_system; }

    std::unordered_map<std::string,address_range> const&  registers_to_ranges() const noexcept { return *m_registers_to_ranges; }
    std::map<address_range,std::string> const&  ranges_to_registers() const noexcept { return *m_ranges_to_registers; }
    uint64_t  start_address_of_temporaries() const noexcept { return m_start_address_of_temporaries; }

    file_descriptor_ptr  file_descriptor() const noexcept { return m_file_descriptor; }

    /**
     * Sections are stored in the vector in the order according the their start addresses (from the smallest to the highest).
     */
    std::vector<stack_section> const&  stack_sections() const noexcept { return m_stack_sections; }
    stack_init_data const&  default_stack_init_data() const noexcept { return m_stack_data; }

    bool  has_tls_template() const noexcept { return !tls_template_content().empty(); }
    std::vector<uint8_t> const&  tls_template_content() const noexcept { return m_tls_template_content; }
    uint64_t  tls_template_offset() const noexcept { return m_tls_template_offset; }
    std::vector<uint64_t> const&  tls_template_relocations() const noexcept { return m_tls_template_relocations; }

    uint64_t  heap_start() const noexcept { return m_heap_start; }
    uint64_t  heap_end() const noexcept { return m_heap_end; }

    void  drop_file_info() { m_file_descriptor.reset(); }

private:
    processor_architecture  m_processor;
    operating_system  m_system;
    std::unordered_map<std::string,address_range> const*  m_registers_to_ranges;
    std::map<address_range,std::string> const*  m_ranges_to_registers;
    uint64_t  m_start_address_of_temporaries;
    file_descriptor_ptr  m_file_descriptor;
    std::vector<stack_section>  m_stack_sections;
    stack_init_data  m_stack_data;
    std::vector<uint8_t>  m_tls_template_content;
    uint64_t  m_tls_template_offset;
    std::vector<uint64_t>  m_tls_template_relocations;
    uint64_t  m_heap_start;
    uint64_t  m_heap_end;
};


}}


#endif
