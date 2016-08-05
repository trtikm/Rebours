#include <rebours/MAL/descriptor/storage.hpp>
#include <rebours/MAL/descriptor/msgstream.hpp>
#include <rebours/MAL/descriptor/assumptions.hpp>
#include <rebours/MAL/descriptor/invariants.hpp>
#include <rebours/MAL/descriptor/endian.hpp>
#include <algorithm>
#include <utility>

namespace mal { namespace descriptor { namespace detail {


namespace x86_64_Linux {
uint64_t  build_register_maps(std::unordered_map<std::string,address_range> const*&  registers_to_ranges, std::map<address_range,std::string> const*&  ranges_to_registers);
void  build_stack_sections(std::vector<stack_section>&  sections, uint64_t const  desred_size, uint64_t const  begin_address);
void  build_stack_init_data(std::string const&  root_file, std::vector<std::string>&  cmd_line, std::vector<environment_variable_type>&  env_vars);
void  build_tls_template(std::vector<uint8_t>&  content, uint64_t&  offset, std::vector<uint64_t>&  relocations, file_descriptor_ptr const  descriptor);
}


std::map<address_range,std::string>  invert_registers_to_ranges(std::unordered_map<std::string,address_range> const&  registers_to_ranges)
{
    std::map<address_range,std::string> result;
    for (auto const&  register_range : registers_to_ranges)
        result.insert({register_range.second,register_range.first});
    return std::move(result);
}


uint64_t  compute_stack_size(file_descriptor_ptr const  fdesc)
{
    std::string const  root_file = fdesc->dependencies_of_loaded_files()->root_file();
    loader::file_property_map const&  fprops = *fdesc->files_table()->at(root_file)->property_map();
    auto const  it = fprops.find(loader::file_properties::max_stack_size());
    if (it != fprops.cend())
        return atoi(it->second.c_str());
    return 0ULL;
}


void  linearise(std::vector<uint8_t>&  output, uint32_t  value)
{
    uint8_t*  b = reinterpret_cast<uint8_t*>(&value);
    uint8_t* const  e = b + sizeof(value);
    if (is_this_little_endian_machine())
        std::reverse(b,e);
    for ( ; b != e; ++b)
        output.push_back(*b);
}

void  linearise(std::vector<uint8_t>&  output, std::string const&  value)
{
    for (uint8_t const  c : value)
        output.push_back(c);
    output.push_back('\0');
}


std::pair<uint64_t, //!< begin address
          uint64_t  //!< end address
          > compute_heap_boundary(loader::sections_table  const&  sections, std::string const&  root_file)
{
    uint64_t const  memory_page_alignment = 0x1000ULL;

    uint64_t heap_start = 0ULL;
    uint64_t heap_end = 0xffffffffffffffffULL;
    for (auto const& adr_sec : sections)
    {
        loader::file_props_ptr const  fprops = adr_sec.second->file_props();
        if (fprops->path() == root_file && fprops->has_property_value(loader::file_properties::file_type(),loader::file_types::executable()))
        {
            uint64_t  adr = adr_sec.second->end_address();
            if (adr % memory_page_alignment != 0ULL)
                adr += memory_page_alignment - (adr % memory_page_alignment);
            heap_start = std::max(heap_start,adr);
        }
        else
        {
            uint64_t  adr = adr_sec.first;
            if (adr % memory_page_alignment != 0ULL)
                adr -= adr % memory_page_alignment;
            heap_end = std::min(heap_end,adr);
        }
    }

    return {heap_start,heap_end};
}


}}}

namespace mal { namespace descriptor {


void  linearise(stack_init_data const&  sid, std::vector<uint8_t>&  output)
{
    detail::linearise(output,sid.first.size());
    for (std::string const& s : sid.first)
        detail::linearise(output,s);

    detail::linearise(output,sid.second.size());
    for (std::pair<std::string,std::string> const& name_value : sid.second)
    {
        detail::linearise(output,name_value.first);
        detail::linearise(output,name_value.second);
    }
}


stack_section::stack_section(
        uint64_t const  start_address,
        uint64_t const  end_address,
        bool const  has_read_access,
        bool const  has_write_access,
        bool const  has_execute_access,
        bool const  is_in_big_endian,
        bool const  has_const_endian)
    : m_start_address(start_address)
    , m_end_address(end_address)
    , m_has_read_access(has_read_access)
    , m_has_write_access(has_write_access)
    , m_has_execute_access(has_execute_access)
    , m_is_in_big_endian(is_in_big_endian)
    , m_has_const_endian(has_const_endian)
{
    ASSUMPTION(m_start_address < m_end_address);
}


storage::storage(file_descriptor_ptr const  descriptor)
    : m_processor(descriptor->platform()->architecture())
    , m_system(descriptor->platform()->abi())
    , m_registers_to_ranges()
    , m_ranges_to_registers()
    , m_start_address_of_temporaries()
    , m_file_descriptor(descriptor)
    , m_stack_sections()
    , m_stack_data()
    , m_tls_template_content()
    , m_tls_template_offset(0ULL)
    , m_tls_template_relocations()
    , m_heap_start(detail::compute_heap_boundary(*descriptor->sections_table(),descriptor->dependencies_of_loaded_files()->root_file()).first)
    , m_heap_end(detail::compute_heap_boundary(*descriptor->sections_table(),descriptor->dependencies_of_loaded_files()->root_file()).second)
{
    if (m_processor == descriptor::processor_architecture::X86_64 &&
        (m_system == descriptor::operating_system::LINUX ||
         m_system == descriptor::operating_system::UNIX))
    {
        m_start_address_of_temporaries = detail::x86_64_Linux::build_register_maps(m_registers_to_ranges,m_ranges_to_registers);

        detail::x86_64_Linux::build_stack_sections(m_stack_sections,detail::compute_stack_size(m_file_descriptor),std::next(m_file_descriptor->sections_table()->cend(),-1)->second->end_address());
        detail::x86_64_Linux::build_stack_init_data(m_file_descriptor->dependencies_of_loaded_files()->root_file(),m_stack_data.first,m_stack_data.second);
        detail::x86_64_Linux::build_tls_template(m_tls_template_content,m_tls_template_offset,m_tls_template_relocations,m_file_descriptor);
    }
    else
        UNREACHABLE();
}


}}
