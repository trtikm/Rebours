#include <rebours/program/instruction.hpp>
#include <rebours/utility/assumptions.hpp>
#include <unordered_set>
#include <array>
#include <limits>
#include <mutex>
#ifdef DEBUG
#   include <rebours/program/assembly.hpp>
#endif

namespace microcode { namespace detail {


static std::array<uint8_t,17U> GIDs = {
    num(GID::GUARDS),
    num(GID::SETANDCOPY),
    num(GID::INDIRECTCOPY),
    num(GID::DATATRANSFER),
    num(GID::TYPECASTING),
    num(GID::MEMORYMANAGEMENT),
    num(GID::CONCURRENCY),
    num(GID::MODULARITY),
    num(GID::HAVOC),
    num(GID::INTEGERARITHMETICS),
    num(GID::ZEROTEST),
    num(GID::BITOPERATIONS),
    num(GID::FLOATINGPOINTCOMPARISONS),
    num(GID::FLOATINGPOINTARITHMETICS),
    num(GID::INTERRUPTS),
    num(GID::INPUTOUTPUT),
    num(GID::MISCELLANEOUS),
};


bool  is_instruction_with_data(microcode::GIK const  GIK);

void  test_cache();

struct instruction
{
    static instruction const*  create(std::vector<uint8_t> const&  separators, std::vector<uint8_t> const&  data);

    microcode::GIK  GIK() const noexcept { return static_cast<microcode::GIK>(m_data.at(0ULL)); }

    uint64_t  num_arguments() const noexcept { return m_separators.size() + (has_data_block() ? m_data.size() - m_separators.back() - 1ULL : 0ULL); }
    uint8_t  argument_size(uint64_t const  argument_index) const;
    uint8_t const*  argument_begin(uint64_t const  argument_index) const;

    std::vector<uint8_t> const&  separators() const noexcept { return m_separators; }
    std::vector<uint8_t> const&  data() const noexcept { return m_data; }

private:
    instruction(std::vector<uint8_t> const&  separators, std::vector<uint8_t> const&  data);

    instruction& operator=(instruction const&  other) = delete;

    bool  has_data_block() const noexcept { return is_instruction_with_data(GIK()); }

    using instructions_dictionary = std::unordered_set<instruction,decltype(&instruction_hash),decltype(&instruction_equal)>;
    static instructions_dictionary  s_instructions;
    static std::mutex  s_construction_mutex;
friend void  test_cache();

    std::vector<uint8_t>  m_separators;
    std::vector<uint8_t>  m_data;
};

instruction::instructions_dictionary  instruction::s_instructions(
        0,
        &instruction_hash,
        &instruction_equal
        );

std::mutex  instruction::s_construction_mutex;

instruction const*  instruction::create(std::vector<uint8_t> const&  separators, std::vector<uint8_t> const&  data)
{
    std::lock_guard<std::mutex> const  lock(s_construction_mutex);
    return &*s_instructions.insert({separators,data}).first;
}

instruction::instruction(std::vector<uint8_t> const&  separators, std::vector<uint8_t> const&  data)
    : m_separators(separators)
    , m_data(data)
{
    ASSUMPTION(!m_data.empty());
    ASSUMPTION(m_separators.size() <= 0xffULL);
    ASSUMPTION((m_data.size() == 1ULL && m_separators.empty()) ||
               (m_separators.front() == 1U && m_separators.back() < m_data.size() &&
                [](std::vector<uint8_t> const&  separators){
                    for (uint8_t  i = 1U; i < separators.size(); ++i)
                        if (separators.at(i) > separators.at(i-1U))
                            return false;
                    return true;
                    }(m_separators)
                ));
}

uint8_t  instruction::argument_size(uint64_t const  argument_index) const
{
    if (has_data_block() && argument_index + 1ULL >= m_separators.size())
    {
        ASSUMPTION(argument_index + 1ULL - m_separators.size() < m_data.size() - m_separators.back());
        return 1U;
    }
    else
    {
        ASSUMPTION(argument_index < m_separators.size());
        return argument_index + 1ULL < m_separators.size() ? m_separators.at(argument_index + 1ULL) - m_separators.at(argument_index) :
                                                             (uint8_t)m_data.size() - m_separators.at(argument_index);
    }
}

uint8_t const*  instruction::argument_begin(uint64_t const  argument_index) const
{
    if (has_data_block() && argument_index + 1ULL >= m_separators.size())
    {
        ASSUMPTION(argument_index + 1ULL - m_separators.size() < m_data.size() - m_separators.back());
        return m_data.data() + m_separators.back() + (argument_index + 1ULL - m_separators.size());
    }
    else
    {
        ASSUMPTION(argument_index < m_separators.size());
        return m_data.data() + m_separators.at(argument_index);
    }
}


std::size_t  instruction_hash(instruction const&  I)
{
    std::size_t  result = 0ULL;
    for (uint8_t i = 0ULL, n = (uint8_t)(I.data().size() < 32ULL ? I.data().size() : 32ULL); i < n; ++i)
        result += (i + 1ULL) * 101ULL * std::hash<uint64_t>()(I.data().at(i));
    return result;
}

bool  instruction_equal(instruction const&  I1, instruction const&  I2)
{
    if (I1.data().size() != I2.data().size() || I1.separators().size() != I2.separators().size())
        return false;
    for (uint64_t i = 0ULL; i < I1.data().size(); ++i)
        if (I1.data().at(i) != I2.data().at(i))
            return false;
    for (uint64_t i = 0ULL; i < I1.separators().size(); ++i)
        if (I1.separators().at(i) != I2.separators().at(i))
            return false;
    return true;
}

bool  instruction_less_than(instruction const&  I1, instruction const&  I2)
{
    if (I1.GIK() < I2.GIK())
        return true;
    if (I1.GIK() == I2.GIK())
    {
        if (I1.separators().size() < I2.separators().size())
            return true;
        if (I1.separators().size() == I2.separators().size())
        {
            if (I1.data().size() < I2.data().size())
                return true;
            if (I1.data().size() == I2.data().size())
            {
                bool ok = true;
                for (uint64_t i = 0ULL; i < I1.separators().size(); ++i)
                {
                    if (I1.separators().at(i) < I2.separators().at(i))
                        return true;
                    if (I1.separators().at(i) != I2.separators().at(i))
                    {
                        ok = false;
                        break;
                    }
                }
                if (ok)
                    for (uint64_t i = 0ULL; i < I1.data().size(); ++i)
                    {
                        if (I1.data().at(i) < I2.data().at(i))
                            return true;
                        if (I1.data().at(i) != I2.data().at(i))
                        {
                            ok = false;
                            break;
                        }
                    }
            }
        }
    }
    return false;
}

bool  is_instruction_with_data(microcode::GIK const  GIK)
{
    return GIK == microcode::GIK::DATATRANSFER__DEREF_ADDRESS_ASGN_DATA ||
           GIK == microcode::GIK::DATATRANSFER__DEREF_REG_ASGN_DATA ;
}


using  value_and_size = std::pair<uint64_t,uint8_t>;

struct builder
{
    explicit builder(microcode::GIK const  GIK)
        : m_separators()
        , m_data{ num(GIK) }
        , m_cursor(1U)
    {}

    builder&  store(uint8_t const*  begin, uint8_t const* const  end)
    {
        ASSUMPTION(end - begin != 0ULL);
        ASSUMPTION((uint64_t)m_cursor + (end - begin) <= 0xffULL);
        INVARIANT(m_cursor == m_data.size());
        m_separators.push_back(m_cursor);
        m_cursor += (uint8_t)(end - begin);
        for ( ; begin != end; ++begin)
            m_data.push_back(*begin);
        return *this;
    }

    template<typename argtype>
    builder& operator<<(argtype const arg)
    {
        static_assert(std::is_arithmetic<argtype>::value,"The argument must be a number.");
        return store(reinterpret_cast<uint8_t const*>(&arg), reinterpret_cast<uint8_t const*>(&arg) + sizeof(argtype));
    }

    builder& operator<<(value_and_size const&  arg)
    {
        switch (arg.second)
        {
        case 1U: ASSUMPTION((arg.first >> 8ULL) == 0ULL || ((~arg.first) >> 8ULL) == 0ULL); return operator <<(static_cast<uint8_t>(arg.first));
        case 2U: ASSUMPTION((arg.first >> 16ULL) == 0ULL || ((~arg.first) >> 16ULL) == 0ULL); return operator <<(static_cast<uint16_t>(arg.first));
        case 4U: ASSUMPTION((arg.first >> 32ULL) == 0ULL || ((~arg.first) >> 32ULL) == 0ULL); return operator <<(static_cast<uint32_t>(arg.first));
        case 8U: return operator <<(arg.first);
        default: UNREACHABLE();
        }
    }

    builder& operator<<(uint128_t const&  value)
    {
        return store(value.data(),value.data()+value.size());
    }

    builder& operator<<(float80_type const&  value)
    {
        return store(value.data(),value.data()+value.size());
    }

    builder& operator<<(std::vector<uint8_t> const&  data)
    {
        ASSUMPTION(!data.empty());
        ASSUMPTION(m_cursor != 0xffU);
        INVARIANT(m_cursor == m_data.size());
        m_separators.push_back(m_cursor++);
        for (uint8_t const  v : data)
            m_data.push_back(v);
        return *this;
    }

    operator microcode::instruction() const
    {
        return microcode::instruction(detail::instruction::create(m_separators,m_data));
    }

private:

    std::vector<uint8_t>  m_separators;
    std::vector<uint8_t>  m_data;
    uint8_t  m_cursor;
};


}}


namespace microcode {


std::pair<GID,SID>  decompose(microcode::GIK const  GIK)
{
    auto  it = std::lower_bound(detail::GIDs.cbegin(),detail::GIDs.cend(),num(GIK));
    if (it == detail::GIDs.cend())
        return std::make_pair(static_cast<GID>(detail::GIDs.back()),num(GIK) - detail::GIDs.back());
    if (*it > num(GIK))
    {
        INVARIANT(it != detail::GIDs.cbegin());
        it = std::prev(it,1);
    }
    INVARIANT(*it <= num(GIK));
    return std::make_pair(static_cast<GID>(*it),num(GIK) - *it);
}

uint8_t  num_instructions()
{
    return num(GIK::NUM_GIKs);
}

uint8_t  num_instruction_groups()
{
    return (uint8_t)detail::GIDs.size();
}

uint8_t  num_instruction_subgroups(microcode::GID const  GID)
{
    ASSUMPTION(std::binary_search(detail::GIDs.cbegin(),detail::GIDs.cend(),num(GID)));
    auto const  it = std::lower_bound(detail::GIDs.cbegin(),detail::GIDs.cend(),num(GID));
    auto const  next_it = std::next(it);
    if (next_it == detail::GIDs.cend())
        return num_instructions() - *it;
    else
        return *next_it - *it;
}

instruction::instruction()
    : m_value{nullptr}
#   ifdef DEBUG
    , m_dbg_asmtext()
#   endif
{
#   ifdef DEBUG
    m_dbg_asmtext = "<no-instruction>";
#   endif
}

instruction::instruction(detail::instruction const*  ptr_impl)
    : m_value{ptr_impl}
#   ifdef DEBUG
    , m_dbg_asmtext()
#   endif
{
#   ifdef DEBUG
    m_dbg_asmtext = assembly_text(*this);
#   endif
}

microcode::GIK  instruction::GIK() const noexcept
{
    return m_value->GIK();
}

uint64_t  instruction::num_arguments() const noexcept
{
    return m_value->num_arguments();
}

uint8_t  instruction::argument_size(uint64_t const  argument_index) const
{
    return m_value->argument_size(argument_index);
}

uint8_t const*  instruction::argument_begin(uint64_t const  argument_index) const
{
    return m_value->argument_begin(argument_index);
}


uint128_t  instruction::get_argument(uint64_t const  index, uint128_t const* const) const
{
    ASSUMPTION(index < num_arguments());
    uint8_t const  size = argument_size(index);
    uint8_t const* const  begin = argument_begin(index);
    uint128_t  retval(0ULL);
    ASSUMPTION(size <= retval.size());
    std::copy(begin,begin+size,retval.data());
    return std::move(retval);
}

float80_type  instruction::get_argument(uint64_t const  index, float80_type const* const) const
{
    ASSUMPTION(index < num_arguments());
    uint8_t const  size = argument_size(index);
    uint8_t const* const  begin = argument_begin(index);
    float80_type  retval;
    retval.fill(0U);
    ASSUMPTION(size <= retval.size());
    std::copy(begin,begin+size,retval.begin());
    return std::move(retval);
}


}

namespace microcode {


instruction  create_GUARDS__REG_EQUAL_TO_ZERO(uint8_t const  n, uint64_t const  a)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U || n == 16U);
    ASSUMPTION(a <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::GUARDS__REG_EQUAL_TO_ZERO);
    return bld << n << a;
}

instruction  create_GUARDS__REG_NOT_EQUAL_TO_ZERO(uint8_t const  n, uint64_t const  a)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U || n == 16U);
    ASSUMPTION(a <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO);
    return bld << n << a;
}


instruction  create_SETANDCOPY__REG_ASGN_NUMBER(uint8_t const  n, uint64_t const  a, uint64_t const  v)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U || n == 16U);
    ASSUMPTION(a <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::SETANDCOPY__REG_ASGN_NUMBER);
    return bld << n << a << detail::value_and_size(v,n);
}

instruction  create_SETANDCOPY__REG_ASGN_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U || n == 16U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::SETANDCOPY__REG_ASGN_REG);
    return bld << n << a0 << a1;
}


instruction  create_INDIRECTCOPY__REG_ASGN_REG_REG(uint8_t const  n0, uint8_t const  n1, uint64_t const  a0, uint64_t const  a1)
{
    ASSUMPTION(n0 == 1U || n0 == 2U || n0 == 4U || n0 == 8U || n0 == 16U);
    ASSUMPTION(n1 == 1U || n1 == 2U || n1 == 4U || n1 == 8U || n1 == 16U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n0);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n1);
    detail::builder bld(microcode::GIK::INDIRECTCOPY__REG_ASGN_REG_REG);
    return bld << n0 << n1 << a0 << a1;
}

instruction  create_INDIRECTCOPY__REG_REG_ASGN_REG(uint8_t const  n0, uint8_t const  n1, uint64_t const  a0, uint64_t const  a1)
{
    ASSUMPTION(n0 == 1U || n0 == 2U || n0 == 4U || n0 == 8U || n0 == 16U);
    ASSUMPTION(n1 == 1U || n1 == 2U || n1 == 4U || n1 == 8U || n1 == 16U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n1);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n0);
    detail::builder bld(microcode::GIK::INDIRECTCOPY__REG_REG_ASGN_REG);
    return bld << n0 << n1 << a0 << a1;
}


instruction  create_DATATRANSFER__REG_ASGN_DEREF_ADDRESS(uint8_t const  n, uint64_t const  a0, uint64_t const  a1)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - 8ULL);
    detail::builder bld(microcode::GIK::DATATRANSFER__REG_ASGN_DEREF_ADDRESS);
    return bld << n << a0 << a1;
}

instruction  create_DATATRANSFER__REG_ASGN_DEREF_INV_ADDRESS(uint8_t const  n, uint64_t const  a0, uint64_t const  a1)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - 8ULL);
    detail::builder bld(microcode::GIK::DATATRANSFER__REG_ASGN_DEREF_INV_ADDRESS);
    return bld << n << a0 << a1;
}

instruction  create_DATATRANSFER__REG_ASGN_DEREF_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - 8ULL);
    detail::builder bld(microcode::GIK::DATATRANSFER__REG_ASGN_DEREF_REG);
    return bld << n << a0 << a1;
}

instruction  create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - 8ULL);
    detail::builder bld(microcode::GIK::DATATRANSFER__REG_ASGN_DEREF_INV_REG);
    return bld << n << a0 << a1;
}

instruction  create_DATATRANSFER__DEREF_REG_ASGN_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - 8ULL);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::DATATRANSFER__DEREF_REG_ASGN_REG);
    return bld << n << a0 << a1;
}

instruction  create_DATATRANSFER__DEREF_INV_REG_ASGN_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - 8ULL);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::DATATRANSFER__DEREF_INV_REG_ASGN_REG);
    return bld << n << a0 << a1;
}

instruction  create_DATATRANSFER__DEREF_ADDRESS_ASGN_DATA(uint64_t const  a, std::vector<uint8_t> const&  data)
{
    ASSUMPTION(!data.empty());
    ASSUMPTION(a <= std::numeric_limits<uint64_t>::max() - data.size());
    detail::builder bld(microcode::GIK::DATATRANSFER__DEREF_ADDRESS_ASGN_DATA);
    return bld << a << data;
}

instruction  create_DATATRANSFER__DEREF_REG_ASGN_DATA(uint64_t const  a, std::vector<uint8_t> const&  data)
{
    ASSUMPTION(!data.empty());
    ASSUMPTION(a <= std::numeric_limits<uint64_t>::max() - data.size());
    detail::builder bld(microcode::GIK::DATATRANSFER__DEREF_REG_ASGN_DATA);
    return bld << a << data;
}

instruction  create_DATATRANSFER__DEREF_REG_ASGN_NUMBER(uint8_t const  n, uint64_t const  a, uint64_t const  v)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U);
    ASSUMPTION(a <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::DATATRANSFER__DEREF_REG_ASGN_NUMBER);
    return bld << n << a << detail::value_and_size(v,n);
}

instruction  create_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(uint8_t const  n, uint64_t const  a, uint64_t const  v)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U);
    ASSUMPTION(a <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER);
    return bld << n << a << detail::value_and_size(v,n);
}

instruction  create_TYPECASTING__REG_ASGN_ZERO_EXTEND_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - 2U*n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::TYPECASTING__REG_ASGN_ZERO_EXTEND_REG);
    return bld << n << a0 << a1;
}

instruction  create_TYPECASTING__REG_ASGN_SIGN_EXTEND_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - 2U*n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::TYPECASTING__REG_ASGN_SIGN_EXTEND_REG);
    return bld << n << a0 << a1;
}


instruction  create_MEMORYMANAGEMENT__REG_ASGN_MEM_STATIC(uint64_t const  a0, uint64_t const  a1, uint8_t const  v0, uint64_t const  v1)
{
    ASSUMPTION(v0 != 0U && v0 < 32U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - 8ULL);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - v1);
    detail::builder bld(microcode::GIK::MEMORYMANAGEMENT__REG_ASGN_MEM_STATIC);
    return bld << a0 << a1 << v0 << v1;
}

instruction  create_MEMORYMANAGEMENT__REG_ASGN_MEM_STATIC(uint64_t const  a0, uint64_t const  a1,
                                                          bool const  readable, bool const  writable, bool const  executable,
                                                          bool const  use_big_endian, bool const  has_mutable_endian,
                                                          uint64_t const  v)
{
    uint8_t const  access_rights =
            (readable ? 1 : 0) +
            (writable ? 2 : 0) +
            (executable ? 4 : 0) +
            (use_big_endian ? 8 : 0) +
            (has_mutable_endian ? 16 : 0)
            ;
    return create_MEMORYMANAGEMENT__REG_ASGN_MEM_STATIC(a0,a1,access_rights,v);
}

instruction  create_MEMORYMANAGEMENT__REG_ASGN_MEM_ALLOC(uint64_t const  a0, uint64_t const  a1, uint64_t const  a2, uint64_t const  a3)
{
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - 8ULL);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - 8ULL);
    ASSUMPTION(a2 <= std::numeric_limits<uint64_t>::max() - 8ULL);
    ASSUMPTION(a3 <= std::numeric_limits<uint64_t>::max() - 8ULL);
    detail::builder bld(microcode::GIK::MEMORYMANAGEMENT__REG_ASGN_MEM_ALLOC);
    return bld << a0 << a1 << a2 << a3;
}

instruction  create_MEMORYMANAGEMENT__REG_ASGN_MEM_FREE(uint64_t const  a0, uint64_t const  a1, uint64_t const  a2)
{
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - 8ULL);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - 8ULL);
    ASSUMPTION(a2 <= std::numeric_limits<uint64_t>::max() - 8ULL);
    detail::builder bld(microcode::GIK::MEMORYMANAGEMENT__REG_ASGN_MEM_FREE);
    return bld << a0 << a1 << a2;
}

instruction  create_MEMORYMANAGEMENT__REG_ASGN_MEM_VALID_REG_NUMBER_NUMBER(uint64_t const  a0, uint64_t const  a1, uint8_t const  v0, uint64_t const  v1)
{
    ASSUMPTION(v0 != 0U && v0 < 8U);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - 8ULL);
    detail::builder bld(microcode::GIK::MEMORYMANAGEMENT__REG_ASGN_MEM_VALID_REG_NUMBER_NUMBER);
    return bld << a0 << a1 << v0 << v1;
}


instruction  create_CONCURRENCY__REG_ASGN_THREAD(uint64_t const  a)
{
    detail::builder bld(microcode::GIK::CONCURRENCY__REG_ASGN_THREAD);
    return bld << a;
}


instruction  create_MODULARITY__CALL(uint64_t const  u)
{
    detail::builder bld(microcode::GIK::MODULARITY__CALL);
    return bld << u;
}


instruction  create_HAVOC__REG_ASGN_HAVOC(uint64_t const  v, uint64_t const  a)
{
    ASSUMPTION(v != 0U);
    ASSUMPTION(a <= std::numeric_limits<uint64_t>::max() - v);
    detail::builder bld(microcode::GIK::HAVOC__REG_ASGN_HAVOC);
    return bld << v << a;
}

instruction  create_HAVOC__REG_REG_ASGN_HAVOC(uint8_t const  n, uint64_t const  v, uint64_t const  a)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U);
    ASSUMPTION(v != 0U);
    ASSUMPTION(a <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::HAVOC__REG_REG_ASGN_HAVOC);
    return bld << n << v << a;
}


instruction  create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  v)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U || n == 16U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER);
    return bld << n << a0 << a1 << detail::value_and_size(v,n);
}

instruction  create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint128_t const  v)
{
    ASSUMPTION(n == 16U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER);
    return bld << n << a0 << a1 << v;
}

instruction  create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U || n == 16U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a2 <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG);
    return bld << n << a0 << a1 << a2;
}

instruction  create_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  v)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U || n == 16U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER);
    return bld << n << a0 << a1 << detail::value_and_size(v,n);
}

instruction  create_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint128_t const  v)
{
    ASSUMPTION(n == 16U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER);
    return bld << n << a0 << a1 << v;
}

instruction  create_INTEGERARITHMETICS__REG_ASGN_REG_DIVIDE_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U || n == 16U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a2 <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::INTEGERARITHMETICS__REG_ASGN_REG_DIVIDE_REG);
    return bld << n << a0 << a1 << a2;
}

instruction  create_INTEGERARITHMETICS__REG_ASGN_REG_MODULO_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U || n == 16U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a2 <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::INTEGERARITHMETICS__REG_ASGN_REG_MODULO_REG);
    return bld << n << a0 << a1 << a2;
}


instruction  create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(uint8_t const  n, uint64_t const  a0, uint64_t const  a1)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO);
    return bld << n << a0 << a1;
}


instruction  create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  v)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::BITOPERATIONS__REG_ASGN_REG_AND_NUMBER);
    return bld << n << a0 << a1 << detail::value_and_size(v,n);
}

instruction  create_BITOPERATIONS__REG_ASGN_REG_AND_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a2 <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::BITOPERATIONS__REG_ASGN_REG_AND_REG);
    return bld << n << a0 << a1 << a2;
}

instruction  create_BITOPERATIONS__REG_ASGN_REG_OR_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  v)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::BITOPERATIONS__REG_ASGN_REG_OR_NUMBER);
    return bld << n << a0 << a1 << detail::value_and_size(v,n);
}

instruction  create_BITOPERATIONS__REG_ASGN_REG_OR_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a2 <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::BITOPERATIONS__REG_ASGN_REG_OR_REG);
    return bld << n << a0 << a1 << a2;
}

instruction  create_BITOPERATIONS__REG_ASGN_NOT_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::BITOPERATIONS__REG_ASGN_NOT_REG);
    return bld << n << a0 << a1;
}

instruction  create_BITOPERATIONS__REG_ASGN_REG_XOR_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  v)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::BITOPERATIONS__REG_ASGN_REG_XOR_NUMBER);
    return bld << n << a0 << a1 << detail::value_and_size(v,n);
}

instruction  create_BITOPERATIONS__REG_ASGN_REG_XOR_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a2 <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::BITOPERATIONS__REG_ASGN_REG_XOR_REG);
    return bld << n << a0 << a1 << a2;
}

instruction  create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint8_t const  v)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER);
    return bld << n << a0 << a1 << v;
}

instruction  create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::BITOPERATIONS__REG_ASGN_REG_RSHIFT_REG);
    return bld << n << a0 << a1 << a2;
}

instruction  create_BITOPERATIONS__REG_ASGN_REG_LSHIFT_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint8_t const  v)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::BITOPERATIONS__REG_ASGN_REG_LSHIFT_NUMBER);
    return bld << n << a0 << a1 << v;
}

instruction  create_BITOPERATIONS__REG_ASGN_REG_LSHIFT_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - n);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::BITOPERATIONS__REG_ASGN_REG_LSHIFT_REG);
    return bld << n << a0 << a1 << a2;
}

instruction  create_FLOATINGPOINTCOMPARISONS__REG_ASGN_REG_EQUAL_TO_ZERO(uint64_t const  a0, uint64_t const  a1)
{
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - 10ULL);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - 10ULL);
    detail::builder bld(microcode::GIK::FLOATINGPOINTCOMPARISONS__REG_ASGN_REG_EQUAL_TO_ZERO);
    return bld << a0 << a1;
}


instruction  create_FLOATINGPOINTARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(uint64_t const  a0, uint64_t const  a1, float80_type const&  f)
{
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - 10ULL);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - 10ULL);
    detail::builder bld(microcode::GIK::FLOATINGPOINTARITHMETICS__REG_ASGN_REG_PLUS_NUMBER);
    return bld << a0 << a1 << f;
}


instruction  create_INTERRUPTS__REG_ASGN_INTERRUPT_SET_AT_FIXED_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  v)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - 8ULL);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - 8ULL);
    detail::builder bld(microcode::GIK::INTERRUPTS__REG_ASGN_INTERRUPT_SET_AT_FIXED_NUMBER);
    return bld << n << a0 << a1 << detail::value_and_size(v,n);
}

instruction  create_INTERRUPTS__REG_ASGN_INTERRUPT_GET_AT_FIXED_NUMBER(uint8_t const  n, uint64_t const  a, uint64_t const  v)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U);
    ASSUMPTION(a <= std::numeric_limits<uint64_t>::max() - 8ULL);
    detail::builder bld(microcode::GIK::INTERRUPTS__REG_ASGN_INTERRUPT_GET_AT_FIXED_NUMBER);
    return bld << n << a << detail::value_and_size(v,n);
}


instruction  create_INPUTOUTPUT__REG_ASGN_STREAM_OPEN_REG(uint64_t const  a0, uint64_t const  a1, uint8_t const  v)
{
    ASSUMPTION(v == 1U || v == 2U || v == 3U || v == 4U);
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - 8ULL);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - 8ULL);
    detail::builder bld(microcode::GIK::INPUTOUTPUT__REG_ASGN_STREAM_OPEN_REG);
    return bld << a0 << a1 << v;
}

instruction  create_INPUTOUTPUT__REG_ASGN_STREAM_OPEN_NUMBER(uint64_t const  a, uint8_t const  v0, uint64_t const  v1)
{
    ASSUMPTION((v0 == 1U && v1 == 1ULL) || (v0 == 2U && v1 == 2ULL) || (v0 == 2U && v1 == 3ULL) || (v0 == 2U && v1 == 4ULL));
    ASSUMPTION(a <= std::numeric_limits<uint64_t>::max() - 8ULL);
    detail::builder bld(microcode::GIK::INPUTOUTPUT__REG_ASGN_STREAM_OPEN_NUMBER);
    return bld << a << v0 << v1;
}

instruction  create_INPUTOUTPUT__REG_ASGN_STREAM_READ_NUMBER(uint64_t const  a, uint64_t const  v)
{
    ASSUMPTION(v != 0U);
    detail::builder bld(microcode::GIK::INPUTOUTPUT__REG_ASGN_STREAM_READ_NUMBER);
    return bld << a << v;
}

instruction  create_INPUTOUTPUT__REG_ASGN_STREAM_WRITE_NUMBER_REG(uint64_t const  a0, uint64_t const  v, uint64_t const  a1)
{
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - 8ULL);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - 8ULL);
    detail::builder bld(microcode::GIK::INPUTOUTPUT__REG_ASGN_STREAM_WRITE_NUMBER_REG);
    return bld << a0 << v << a1;
}

instruction  create_INPUTOUTPUT__REG_ASGN_STREAM_WRITE_REG_REG(uint64_t const  a0, uint64_t const  a1, uint64_t const  a2)
{
    ASSUMPTION(a0 <= std::numeric_limits<uint64_t>::max() - 8ULL);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - 8ULL);
    ASSUMPTION(a2 <= std::numeric_limits<uint64_t>::max() - 8ULL);
    detail::builder bld(microcode::GIK::INPUTOUTPUT__REG_ASGN_STREAM_WRITE_REG_REG);
    return bld << a0 << a1 << a2;
}


instruction  create_MISCELLANEOUS__REG_ASGN_PARITY_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1)
{
    ASSUMPTION(n == 1U || n == 2U || n == 4U || n == 8U || n == 16U);
    ASSUMPTION(a1 <= std::numeric_limits<uint64_t>::max() - n);
    detail::builder bld(microcode::GIK::MISCELLANEOUS__REG_ASGN_PARITY_REG);
    return bld << n << a0 << a1;
}

instruction  create_MISCELLANEOUS__NOP()
{
    detail::builder bld(microcode::GIK::MISCELLANEOUS__NOP);
    return bld;
}

instruction  create_MISCELLANEOUS__STOP()
{
    detail::builder bld(microcode::GIK::MISCELLANEOUS__STOP);
    return bld;
}


}
