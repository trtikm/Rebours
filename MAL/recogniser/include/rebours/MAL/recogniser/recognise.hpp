#ifndef REBOURS_MAL_RECOGNISER_RECOGNISE_HPP_INCLUDED
#   define REBOURS_MAL_RECOGNISER_RECOGNISE_HPP_INCLUDED

#   include <rebours/program/program.hpp>
#   include <functional>
#   include <memory>
#   include <cstdint>

namespace mal { namespace descriptor {
struct storage;
}}

namespace mal { namespace recogniser {
struct recognition_result;
}}

namespace mal { namespace recogniser { namespace detail {
struct recognition_data;
std::shared_ptr<recognition_data const>  get_implementation_details(recognition_result const&  result);
}}}

namespace mal { namespace recogniser {


struct recognition_result
{
    explicit recognition_result(std::shared_ptr<detail::recognition_data> const  data);

    /**
     * Returns a recognised Microcode program. The returned pointer may be invalid, if
     * no program was recognised.
     */
    std::shared_ptr<microcode::program>  program() const noexcept;

    /**
     * A text of the body of an annotation 'ASM.TEXT'.
     * The return value matters only if 'program()' returns a valid pointer.
     */
    std::string const&  asm_text() const noexcept;

    /**
     * A text of the body of an annotation 'ASM.BYTES'.
     * The return value matters only if 'program()' returns a valid pointer.
     */
    std::string const&  asm_bytes() const noexcept;

    /**
     * Returns a two's complement integer representing a return value from the first call to a valuation function ('reg' or 'mem')
     * yielding a negative return value. The method returns values:
     *      1  If 'reg' function returned the value -1 (i.e. the value of the accessed byte at 'address()' is ambiguous or unknown)
     *      2  If 'mem' function returned the value -1 (i.e. the value of the accessed byte at 'address()' is ambiguous or unknown)
     *      4  If 'mem' function returned the value -2 (i.e. the accessed byte at 'address()' does not have proper access rights (read or execute))
     *    254  If the recogniser cannot recognise an instruction from concrete bytes due to its incomplete implementation.
     *    255  If the recogniser recognised an instruction from concrete bytes, but it cannot express it as Microcode program due to its incomplete implementation.
     * The return value matters only if 'program()' returns the invalid pointer.
     */
    uint8_t  result() const noexcept;

    /**
     * Returns the address passed to the first call of a valuation function ('reg' or 'mem') yielding a negative return value.
     * The return value matters only if 'program()' returns the invalid pointer.
     */
    uint64_t  address() const noexcept;

    /**
     * Returns the rights passed to the first call of a valuation function ('reg' or 'mem') yielding a negative return value.
     * The method returns values:
     *      1  if read access rights was passed to the 'mem' function
     *      2  if execute access rights was passed to the 'mem' function
     * The return value matters only if 'program()' returns the invalid pointer and 'result()' returns 4.
     */
    uint8_t  rights() const noexcept;

    /**
     * It holds bytes representing the recognised instruction.
     * The return value matters only if 'program()' is a valid reference or if 'result()' holds the value 255.
     */
    std::vector<uint8_t> const&  buffer() const noexcept;


private:
    std::shared_ptr<detail::recognition_data>  m_data;
    friend std::shared_ptr<detail::recognition_data const>  detail::get_implementation_details(recognition_result const&  result);
};


using  reg_fn_type = std::function<int16_t(uint64_t)>;
using  mem_fn_type = std::function<int16_t(uint64_t,uint8_t)>;
using  recognise_callback_fn = std::function<recognition_result(uint64_t, reg_fn_type const&, mem_fn_type const&)>;


recognition_result  recognise(descriptor::storage const&  description, uint64_t const  start_address, reg_fn_type const&  reg_fn, mem_fn_type const&  mem_fn);


using  recognition_result_dump_fn = std::function<bool(recognition_result const&,   //!< Results containing info about instruction to be dumped
                                                       std::string const&           //!< Pathname of the output file the instruction will be dumped into.
                                                       )>;


}}

#endif
