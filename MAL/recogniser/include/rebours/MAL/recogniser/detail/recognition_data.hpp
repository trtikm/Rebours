#ifndef REBOURS_MAL_RECOGNISER_DETAIL_RECOGNITION_DATA_HPP_INCLUDED
#   define REBOURS_MAL_RECOGNISER_DETAIL_RECOGNITION_DATA_HPP_INCLUDED

#   include <rebours/MAL/recogniser/recognise.hpp>
//#   include <unordered_map>
#   include <memory>
#   include <vector>
#   include <cstdint>

namespace mal { namespace recogniser { namespace detail {


struct recognition_data
{
    recognition_data(uint64_t const  start_address, reg_fn_type const&  reg_fn, mem_fn_type const&  mem_fn);
    virtual  ~recognition_data() {}

    virtual void  recognise(descriptor::storage const&  description) = 0;
    virtual bool  dump(std::string const&  dump_file_pathname) const = 0;

    uint64_t  start_address() const noexcept { return m_start_address; }
    reg_fn_type const&  reg_fn() const noexcept { return m_reg_fn; }
    mem_fn_type const&  mem_fn() const noexcept { return m_mem_fn; }

//    std::unordered_map<uint64_t,uint8_t>  reg_values() const noexcept { return m_reg_values; }
//    std::unordered_map<uint64_t,uint8_t>  mem_values() const noexcept { return m_mem_values; }

    std::shared_ptr<microcode::program>  program() const noexcept { return m_program; }
    std::string const&  asm_text() const noexcept { return m_asm_text; }
    std::string const&  asm_bytes() const noexcept { return m_asm_bytes; }
    uint8_t  error_result() const noexcept { return m_error_result; }
    uint64_t  error_address() const noexcept { return m_error_address; }
    uint8_t  error_rights() const noexcept { return m_error_rights; }
    std::vector<uint8_t> const&  buffer() const noexcept { return m_buffer; }

    void  set_asm_text(std::string const&  value) { m_asm_text = value; }
    void  set_asm_bytes(std::string const&  value) { m_asm_bytes = value; }
    void  set_error_result(uint8_t const  value) noexcept;
    void  set_error_address(uint64_t const  value) noexcept { m_error_address = value; }
    void  set_error_rights(uint8_t const  value) noexcept;
    std::vector<uint8_t>&  buffer() { return m_buffer; }

private:
    uint64_t  m_start_address;
    reg_fn_type  m_reg_fn;
    mem_fn_type  m_mem_fn;

//    std::unordered_map<uint64_t,uint8_t>  m_reg_values;
//    std::unordered_map<uint64_t,uint8_t>  m_mem_values;

    std::shared_ptr<microcode::program>  m_program;
    std::string  m_asm_text;
    std::string  m_asm_bytes;
    uint8_t  m_error_result;
    uint64_t  m_error_address;
    uint8_t  m_error_rights;
    std::vector<uint8_t>  m_buffer;
};


using  recognition_data_ptr = std::shared_ptr<recognition_data>;


}}}


#endif
