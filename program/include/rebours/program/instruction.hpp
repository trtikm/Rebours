#ifndef REBOURS_PROGRAM_MICROCODE_INSTRUCTION_HPP_INCLUDED
#   define REBOURS_PROGRAM_MICROCODE_INSTRUCTION_HPP_INCLUDED

#   include <rebours/program/large_types.hpp>
#   include <rebours/program/endian.hpp>
#   include <rebours/program/assumptions.hpp>
#   include <rebours/program/invariants.hpp>
#   include <rebours/program/endian.hpp>
#   include <vector>
#   include <array>
#   include <iterator>
#   include <tuple>
#   include <algorithm>
#   include <type_traits>
#   include <cstdint>
#   ifdef DEBUG
#       include <string>
#   endif

namespace microcode { namespace detail {
struct instruction;
bool  instruction_equal(instruction const&  I1, instruction const&  I2);
bool  instruction_less_than(instruction const&  I1, instruction const&  I2);
std::size_t  instruction_hash(instruction const&  I);
}}

namespace microcode {


enum struct GID : uint8_t
{
    GUARDS                          =   0,
    SETANDCOPY                      =   2,
    INDIRECTCOPY                    =   4,
    DATATRANSFER                    =   7,
    TYPECASTING                     =  19,
    MEMORYMANAGEMENT                =  26,
    CONCURRENCY                     =  31,
    MODULARITY                      =  33,
    HAVOC                           =  34,
    INTEGERARITHMETICS              =  36,
    ZEROTEST                        =  46,
    BITOPERATIONS                   =  47,
    FLOATINGPOINTCOMPARISONS        =  58,
    FLOATINGPOINTARITHMETICS        =  60,
    INTERRUPTS                      =  68,
    INPUTOUTPUT                     =  74,
    MISCELLANEOUS                   =  89,
};

inline constexpr uint8_t  num(GID const  gid) noexcept { return static_cast<uint8_t>(gid); }

using  SID = uint8_t;

enum struct GIK : uint8_t
{
    // GIK symbolic name                                     GID                                + SID     args            Assembly text

    GUARDS__REG_EQUAL_TO_ZERO                              = num(GID::GUARDS)                   +   0, // n,a             REG[a]{n} =={n} 0{n}
    GUARDS__REG_NOT_EQUAL_TO_ZERO                          = num(GID::GUARDS)                   +   1, // n,a             REG[a]{n} !={n} 0{n}

    SETANDCOPY__REG_ASGN_NUMBER                            = num(GID::SETANDCOPY)               +   0, // n,a,v           REG[a]{n} := v{n}
    SETANDCOPY__REG_ASGN_REG                               = num(GID::SETANDCOPY)               +   1, // n,a,a'          REG[a]{n} := REG[a']{n}

    INDIRECTCOPY__REG_ASGN_REG_REG                         = num(GID::INDIRECTCOPY)             +   0, // n,n',a,a'       REG[a]{n} := REG[REG[a']{n'}]{n}
    INDIRECTCOPY__REG_REG_ASGN_REG                         = num(GID::INDIRECTCOPY)             +   2, // n,n',a,a'       REG[REG[a]{n'}]{n} := REG[a']{n}

    DATATRANSFER__REG_ASGN_DEREF_ADDRESS                   = num(GID::DATATRANSFER)             +   0, // n,a,a'          REG[a]{n} := * a'{8}
    DATATRANSFER__REG_ASGN_DEREF_INV_ADDRESS               = num(GID::DATATRANSFER)             +   1, // n,a,a'          REG[a]{n} := *' a'{8}
    DATATRANSFER__REG_ASGN_DEREF_REG                       = num(GID::DATATRANSFER)             +   2, // n,a,a'          REG[a]{n} := * REG[a']{8}
    DATATRANSFER__REG_ASGN_DEREF_INV_REG                   = num(GID::DATATRANSFER)             +   3, // n,a,a'          REG[a]{n} := *' REG[a']{8}
    DATATRANSFER__DEREF_REG_ASGN_REG                       = num(GID::DATATRANSFER)             +   6, // n,a,a'          * REG[a]{8} := REG[a']{n}
    DATATRANSFER__DEREF_INV_REG_ASGN_REG                   = num(GID::DATATRANSFER)             +   7, // n,a,a'          *' REG[a]{8} := REG[a']{n}
    DATATRANSFER__DEREF_ADDRESS_ASGN_DATA                  = num(GID::DATATRANSFER)             +   8, // a,v1,...,vN     * a{8} := [ v1,...,vN ]{N}
    DATATRANSFER__DEREF_REG_ASGN_DATA                      = num(GID::DATATRANSFER)             +   9, // a,v1,...,vN     * REG[a]{8} := [ v1,...,vN ]{N}
    DATATRANSFER__DEREF_REG_ASGN_NUMBER                    = num(GID::DATATRANSFER)             +   10,// n,a,v           * REG[a]{8} := v{n}
    DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER                = num(GID::DATATRANSFER)             +   11,// n,a,v           *' REG[a]{8} := v{n}

    TYPECASTING__REG_ASGN_ZERO_EXTEND_REG                  = num(GID::TYPECASTING)              +   0, // n,a,a'          REG[a]{2n} := #z REG[a']{n}
    TYPECASTING__REG_ASGN_SIGN_EXTEND_REG                  = num(GID::TYPECASTING)              +   1, // n,a,a'          REG[a]{2n} := #s REG[a']{n}

    MEMORYMANAGEMENT__REG_ASGN_MEM_STATIC                  = num(GID::MEMORYMANAGEMENT)         +   0, // a,a',v1,v8      REG[a]{8} := MEM STATIC v1{1}, v8{8}, a'{8}
    MEMORYMANAGEMENT__REG_ASGN_MEM_ALLOC                   = num(GID::MEMORYMANAGEMENT)         +   1, // a0,a1,a2,a3     REG[a0]{8} := MEM ALLOC REG[a1]{1}, REG[a2]{8}, REG[a3]{8}
    MEMORYMANAGEMENT__REG_ASGN_MEM_FREE                    = num(GID::MEMORYMANAGEMENT)         +   2, // a0,a1,a2        REG[a0]{1} := MEM FREE REG[a1]{8}, REG[a2]{8}
    MEMORYMANAGEMENT__REG_ASGN_MEM_VALID_REG_NUMBER_NUMBER = num(GID::MEMORYMANAGEMENT)         +   3, // a,a',v1,v8      REG[a]{1} := MEM VALID? REG[a']{8}, v1{8}, v8{8}

    CONCURRENCY__REG_ASGN_THREAD                           = num(GID::CONCURRENCY)              +   0, // a               REG[a]{1} := THREAD

    MODULARITY__CALL                                       = num(GID::MODULARITY)               +   0, // node_id         CALL node_id

    HAVOC__REG_ASGN_HAVOC                                  = num(GID::HAVOC)                    +   0, // v8,a            REG[a]{v8} := HAVOC
    HAVOC__REG_REG_ASGN_HAVOC                              = num(GID::HAVOC)                    +   1, // n,v8,a          REG[REG[a]{n}]{v8} := HAVOC

    INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER           = num(GID::INTEGERARITHMETICS)       +   0, // n,a,a',v        REG[a]{n} := REG[a']{n} +{n} v{n}
    INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG              = num(GID::INTEGERARITHMETICS)       +   1, // n,a0,a1,a2      REG[a0]{n} := REG[a1]{n} +{n} REG[a2]{n}
    INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER          = num(GID::INTEGERARITHMETICS)       +   2, // n,a,a',v        REG[a]{n} := REG[a']{n} *{n} v{n}
    INTEGERARITHMETICS__REG_ASGN_REG_DIVIDE_REG            = num(GID::INTEGERARITHMETICS)       +   6, // n,a0,a1,a2      REG[a0]{n} := REG[a1]{n} /{n} REG[a2]{n}
    INTEGERARITHMETICS__REG_ASGN_REG_MODULO_REG            = num(GID::INTEGERARITHMETICS)       +   9, // n,a0,a1,a2      REG[a0]{n} := REG[a1]{n} %{n} REG[a2]{n}

    ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO                   = num(GID::ZEROTEST)                 +   0, // n,a,a'          REG[a]{1} := REG[a']{n} =={n} 0{n}

    BITOPERATIONS__REG_ASGN_REG_AND_NUMBER                 = num(GID::BITOPERATIONS)            +   0, // n,a,a',v        REG[a]{n} := REG[a']{n} &{n} v{n}
    BITOPERATIONS__REG_ASGN_REG_AND_REG                    = num(GID::BITOPERATIONS)            +   1, // n,a0,a1,a2      REG[a0]{n} := REG[a1]{n} &{n} REG[a2]{n}
    BITOPERATIONS__REG_ASGN_REG_OR_NUMBER                  = num(GID::BITOPERATIONS)            +   2, // n,a,a',v        REG[a]{n} := REG[a']{n} |{n} v{n}
    BITOPERATIONS__REG_ASGN_REG_OR_REG                     = num(GID::BITOPERATIONS)            +   3, // n,a0,a1,a2      REG[a0]{n} := REG[a1]{n} |{n} REG[a2]{n}
    BITOPERATIONS__REG_ASGN_NOT_REG                        = num(GID::BITOPERATIONS)            +   4, // n,a,a'          REG[a]{n} := !{n} REG[a']{n}
    BITOPERATIONS__REG_ASGN_REG_XOR_NUMBER                 = num(GID::BITOPERATIONS)            +   5, // n,a0,a1,v       REG[a0]{n} := REG[a1]{n} XOR{n} v{n}
    BITOPERATIONS__REG_ASGN_REG_XOR_REG                    = num(GID::BITOPERATIONS)            +   6, // n,a0,a1,a2      REG[a0]{n} := REG[a1]{n} XOR{n} REG[a2]{n}
    BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER              = num(GID::BITOPERATIONS)            +   7, // n,a0,a1,v       REG[a0]{n} := REG[a1]{n} >>{n} v{n}
    BITOPERATIONS__REG_ASGN_REG_RSHIFT_REG                 = num(GID::BITOPERATIONS)            +   8, // n,a0,a1,a2      REG[a0]{n} := REG[a1]{n} >>{n} REG[a2]{n}
    BITOPERATIONS__REG_ASGN_REG_LSHIFT_NUMBER              = num(GID::BITOPERATIONS)            +   9, // n,a0,a1,v       REG[a0]{n} := REG[a1]{n} <<{n} v{n}
    BITOPERATIONS__REG_ASGN_REG_LSHIFT_REG                 = num(GID::BITOPERATIONS)            +  10, // n,a0,a1,a2      REG[a0]{n} := REG[a1]{n} <<{n} REG[a2]{n}

    FLOATINGPOINTCOMPARISONS__REG_ASGN_REG_EQUAL_TO_ZERO   = num(GID::FLOATINGPOINTCOMPARISONS) +   0, // a,a'            REG[a]{10} := REG[a']{10} =={10} 0{10}

    FLOATINGPOINTARITHMETICS__REG_ASGN_REG_PLUS_NUMBER     = num(GID::FLOATINGPOINTARITHMETICS) +   0, // a,a',f          REG[a]{10} := REG[a']{10} +{10} f{10}

    INTERRUPTS__REG_ASGN_INTERRUPT_SET_AT_FIXED_NUMBER     = num(GID::INTERRUPTS)               +   0, // n,a,a',v        REG[a]{8} := INTERRUPT SET v{n}, REG[a']{8}
    INTERRUPTS__REG_ASGN_INTERRUPT_GET_AT_FIXED_NUMBER     = num(GID::INTERRUPTS)               +   4, // n,a,v           REG[a]{8} := INTERRUPT GET v{n}

    INPUTOUTPUT__REG_ASGN_STREAM_OPEN_REG                  = num(GID::INPUTOUTPUT)              +   0, // a,a',v          REG[a]{8} := STREAM OPEN v{1}, REG[a']{8}
    INPUTOUTPUT__REG_ASGN_STREAM_OPEN_NUMBER               = num(GID::INPUTOUTPUT)              +   1, // a,v,v'          REG[a]{8} := STREAM OPEN v{1}, v'{8}
    INPUTOUTPUT__REG_ASGN_STREAM_READ_NUMBER               = num(GID::INPUTOUTPUT)              +   8, // a,v'            REG[a]{1} := STREAM READ v{8}
    INPUTOUTPUT__REG_ASGN_STREAM_WRITE_NUMBER_REG          = num(GID::INPUTOUTPUT)              +  10, // a0,v,a1         REG[a0]{1} := STREAM WRITE v{8}, REG[a1]{1}
    INPUTOUTPUT__REG_ASGN_STREAM_WRITE_REG_REG             = num(GID::INPUTOUTPUT)              +  11, // a0,a1,a2        REG[a0]{1} := STREAM WRITE REG[a1]{8}, REG[a2]{1}

    MISCELLANEOUS__REG_ASGN_PARITY_REG                     = num(GID::MISCELLANEOUS)            +   0, // n,a,a'          REG[a]{1} := PARITY REG[a']{n}
    MISCELLANEOUS__NOP                                     = num(GID::MISCELLANEOUS)            +   1, //                 NOP
    MISCELLANEOUS__STOP                                    = num(GID::MISCELLANEOUS)            +   2, //                 STOP

    NUM_GIKs
};

inline constexpr uint8_t  num(GIK const  gik) noexcept { return static_cast<uint8_t>(gik); }


struct instruction;


instruction  create_GUARDS__REG_EQUAL_TO_ZERO(uint8_t const  n, uint64_t const  a);
instruction  create_GUARDS__REG_NOT_EQUAL_TO_ZERO(uint8_t const  n, uint64_t const  a);

instruction  create_SETANDCOPY__REG_ASGN_NUMBER(uint8_t const  n, uint64_t const  a, uint64_t const  v);
instruction  create_SETANDCOPY__REG_ASGN_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1);

instruction  create_INDIRECTCOPY__REG_ASGN_REG_REG(uint8_t const  n0, uint8_t const  n1, uint64_t const  a0, uint64_t const  a1);
instruction  create_INDIRECTCOPY__REG_REG_ASGN_REG(uint8_t const  n0, uint8_t const  n1, uint64_t const  a0, uint64_t const  a1);

instruction  create_DATATRANSFER__REG_ASGN_DEREF_ADDRESS(uint8_t const  n, uint64_t const  a0, uint64_t const  a1);
instruction  create_DATATRANSFER__REG_ASGN_DEREF_INV_ADDRESS(uint8_t const  n, uint64_t const  a0, uint64_t const  a1);
instruction  create_DATATRANSFER__REG_ASGN_DEREF_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1);
instruction  create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1);
instruction  create_DATATRANSFER__DEREF_REG_ASGN_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1);
instruction  create_DATATRANSFER__DEREF_INV_REG_ASGN_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1);
instruction  create_DATATRANSFER__DEREF_ADDRESS_ASGN_DATA(uint64_t const  a, std::vector<uint8_t> const&  data);
instruction  create_DATATRANSFER__DEREF_REG_ASGN_DATA(uint64_t const  a, std::vector<uint8_t> const&  data);
instruction  create_DATATRANSFER__DEREF_REG_ASGN_NUMBER(uint8_t const  n, uint64_t const  a, uint64_t const  v);
instruction  create_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(uint8_t const  n, uint64_t const  a, uint64_t const  v);

instruction  create_TYPECASTING__REG_ASGN_ZERO_EXTEND_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1);
instruction  create_TYPECASTING__REG_ASGN_SIGN_EXTEND_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1);

instruction  create_MEMORYMANAGEMENT__REG_ASGN_MEM_STATIC(uint64_t const  a0, uint64_t const  a1, uint8_t const  v0, uint64_t const  v1);
instruction  create_MEMORYMANAGEMENT__REG_ASGN_MEM_STATIC(uint64_t const  a0, uint64_t const  a1,
                                                          bool const  readable, bool const  writable, bool const  executable,
                                                          bool const  use_big_endian, bool const  has_mutable_endian,
                                                          uint64_t const  v);
instruction  create_MEMORYMANAGEMENT__REG_ASGN_MEM_ALLOC(uint64_t const  a0, uint64_t const  a1, uint64_t const  a2, uint64_t const  a3);
instruction  create_MEMORYMANAGEMENT__REG_ASGN_MEM_FREE(uint64_t const  a0, uint64_t const  a1, uint64_t const  a2);
instruction  create_MEMORYMANAGEMENT__REG_ASGN_MEM_VALID_REG_NUMBER_NUMBER(uint64_t const  a0, uint64_t const  a1, uint8_t const  v0, uint64_t const  v1);

instruction  create_CONCURRENCY__REG_ASGN_THREAD(uint64_t const  a);

instruction  create_MODULARITY__CALL(uint64_t const  u);

instruction  create_HAVOC__REG_ASGN_HAVOC(uint64_t const  v, uint64_t const  a);
instruction  create_HAVOC__REG_REG_ASGN_HAVOC(uint8_t const  n, uint64_t const  v, uint64_t const  a);

instruction  create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  v);
instruction  create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint128_t const  v);
instruction  create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2);
instruction  create_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  v);
instruction  create_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint128_t const  v);
instruction  create_INTEGERARITHMETICS__REG_ASGN_REG_DIVIDE_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2);
instruction  create_INTEGERARITHMETICS__REG_ASGN_REG_MODULO_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2);

instruction  create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(uint8_t const  n, uint64_t const  a0, uint64_t const  a1);

instruction  create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  v);
instruction  create_BITOPERATIONS__REG_ASGN_REG_AND_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2);
instruction  create_BITOPERATIONS__REG_ASGN_REG_OR_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  v);
instruction  create_BITOPERATIONS__REG_ASGN_REG_OR_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2);
instruction  create_BITOPERATIONS__REG_ASGN_NOT_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1);
instruction  create_BITOPERATIONS__REG_ASGN_REG_XOR_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  v);
instruction  create_BITOPERATIONS__REG_ASGN_REG_XOR_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2);
instruction  create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint8_t const  v);
instruction  create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2);
instruction  create_BITOPERATIONS__REG_ASGN_REG_LSHIFT_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint8_t const  v);
instruction  create_BITOPERATIONS__REG_ASGN_REG_LSHIFT_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2);

instruction  create_FLOATINGPOINTCOMPARISONS__REG_ASGN_REG_EQUAL_TO_ZERO(uint64_t const  a0, uint64_t const  a1);

instruction  create_FLOATINGPOINTARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(uint64_t const  a0, uint64_t const  a1, float80_t const&  f);

instruction  create_INTERRUPTS__REG_ASGN_INTERRUPT_SET_AT_FIXED_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  v);
instruction  create_INTERRUPTS__REG_ASGN_INTERRUPT_GET_AT_FIXED_NUMBER(uint8_t const  n, uint64_t const  a, uint64_t const  v);

instruction  create_INPUTOUTPUT__REG_ASGN_STREAM_OPEN_REG(uint64_t const  a0, uint64_t const  a1, uint8_t const  v);
instruction  create_INPUTOUTPUT__REG_ASGN_STREAM_OPEN_NUMBER(uint64_t const  a, uint8_t const  v0, uint64_t const  v1);
instruction  create_INPUTOUTPUT__REG_ASGN_STREAM_READ_NUMBER(uint64_t const  a, uint64_t const  v);
instruction  create_INPUTOUTPUT__REG_ASGN_STREAM_WRITE_NUMBER_REG(uint64_t const  a0, uint64_t const  v, uint64_t const  a1);
instruction  create_INPUTOUTPUT__REG_ASGN_STREAM_WRITE_REG_REG(uint64_t const  a0, uint64_t const  a1, uint64_t const  a2);

instruction  create_MISCELLANEOUS__REG_ASGN_PARITY_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1);
instruction  create_MISCELLANEOUS__NOP();
instruction  create_MISCELLANEOUS__STOP();


/**
 * It returns a pair (GID,SID) representing a decomposition of a given GIK = GID + SID.
 */
std::pair<GID,SID>  decompose(microcode::GIK const  GIK);

uint8_t  num_instructions();
uint8_t  num_instruction_groups();
uint8_t  num_instruction_subgroups(microcode::GID const  GID);


struct instruction
{
    instruction();
    explicit instruction(detail::instruction const*  ptr_impl);

    detail::instruction const*  operator->() const noexcept { return m_value; }
    operator bool () const noexcept { return m_value != nullptr; }

    microcode::GIK  GIK() const noexcept;
    microcode::GID  GID() const { return decompose(GIK()).first; }
    microcode::SID  SID() const { return decompose(GIK()).second; }

    uint64_t  num_arguments() const noexcept;
    uint8_t  argument_size(uint64_t const  argument_index) const;

    template<typename return_type>
    return_type  argument(uint64_t const  argument_index) const { return get_argument(argument_index,(return_type const*)nullptr); }

    uint8_t const*  argument_begin(uint64_t const  argument_index) const;

    struct hash { std::size_t  operator()(instruction const  I) const { return detail::instruction_hash(*I.operator ->()); } };

private:

    template<typename return_type>
    return_type  get_argument(uint64_t const  index, return_type const* const = nullptr) const;

    uint128_t  get_argument(uint64_t const  index, uint128_t const* const = nullptr) const;
    float80_t  get_argument(uint64_t const  index, float80_t const* const = nullptr) const;


    detail::instruction const*  m_value;

#   ifdef DEBUG
    std::string  m_dbg_asmtext;
#   endif
};


inline bool  operator ==(instruction const  I0, instruction const  I1) noexcept { return I0.operator ->() == I1.operator ->(); }
inline bool  operator !=(instruction const  I0, instruction const  I1) noexcept { return !(I0 == I1); }


template<typename return_type>
return_type  instruction::get_argument(uint64_t const  index, return_type const* const) const
{
    static_assert(std::is_integral<return_type>::value && std::is_unsigned<return_type>::value,"The argument value must be an unsigned integer.");
    ASSUMPTION(index < num_arguments());
    uint8_t const  size = argument_size(index);
    uint8_t const* const  begin = argument_begin(index);
    return_type retval(0U);
    ASSUMPTION(size <= sizeof(retval));
    if (is_this_little_endian_machine())
        std::copy(begin,begin+size,reinterpret_cast<uint8_t*>(&retval));
    else
        std::copy(begin,begin+size,reinterpret_cast<uint8_t*>(&retval) + (sizeof(retval) - size));
    return retval;
}


}

#endif
