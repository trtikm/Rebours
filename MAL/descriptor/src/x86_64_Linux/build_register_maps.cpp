#include <rebours/MAL/descriptor/storage.hpp>
#include <rebours/MAL/descriptor/msgstream.hpp>
#include <rebours/MAL/descriptor/assumptions.hpp>
#include <rebours/MAL/descriptor/invariants.hpp>
#include <rebours/MAL/descriptor/development.hpp>

namespace mal { namespace descriptor { namespace detail {
std::map<address_range,std::string>  invert_registers_to_ranges(std::unordered_map<std::string,address_range> const&  registers_to_ranges);
}}}

namespace mal { namespace descriptor { namespace detail { namespace x86_64_Linux {


uint64_t  build_register_maps(std::unordered_map<std::string,address_range> const*&  registers_to_ranges, std::map<address_range,std::string> const*&  ranges_to_registers)
{
    static uint64_t constexpr registers_begin = 8ULL;
    static std::unordered_map<std::string,address_range> const  s_registers_to_ranges = {
        { "rip",    { 0ULL,  8ULL } },

        { "al",     { registers_begin + 7ULL, registers_begin + 8ULL } },
        { "ah",     { registers_begin + 6ULL, registers_begin + 7ULL } },
        { "ax",     { registers_begin + 6ULL, registers_begin + 8ULL } },
        { "eax",    { registers_begin + 4ULL, registers_begin + 8ULL } },
        { "rax",    { registers_begin + 0ULL, registers_begin + 8ULL } },

        { "bl",     { registers_begin + 15ULL, registers_begin + 16ULL } },
        { "bh",     { registers_begin + 14ULL, registers_begin + 15ULL } },
        { "bx",     { registers_begin + 14ULL, registers_begin + 16ULL } },
        { "ebx",    { registers_begin + 12ULL, registers_begin + 16ULL } },
        { "rbx",    { registers_begin + 8ULL,  registers_begin + 16ULL } },

        { "cl",     { registers_begin + 23ULL, registers_begin + 24ULL } },
        { "ch",     { registers_begin + 22ULL, registers_begin + 23ULL } },
        { "cx",     { registers_begin + 22ULL, registers_begin + 24ULL } },
        { "ecx",    { registers_begin + 20ULL, registers_begin + 24ULL } },
        { "rcx",    { registers_begin + 16ULL, registers_begin + 24ULL } },

        { "dl",     { registers_begin + 31ULL, registers_begin + 32ULL } },
        { "dh",     { registers_begin + 30ULL, registers_begin + 31ULL } },
        { "dx",     { registers_begin + 30ULL, registers_begin + 32ULL } },
        { "edx",    { registers_begin + 28ULL, registers_begin + 32ULL } },
        { "rdx",    { registers_begin + 24ULL, registers_begin + 32ULL } },

        { "sil",    { registers_begin + 39ULL, registers_begin + 40ULL } },
        { "si",     { registers_begin + 38ULL, registers_begin + 40ULL } },
        { "esi",    { registers_begin + 36ULL, registers_begin + 40ULL } },
        { "rsi",    { registers_begin + 32ULL, registers_begin + 40ULL } },

        { "dil",    { registers_begin + 47ULL, registers_begin + 48ULL } },
        { "di",     { registers_begin + 46ULL, registers_begin + 48ULL } },
        { "edi",    { registers_begin + 44ULL, registers_begin + 48ULL } },
        { "rdi",    { registers_begin + 40ULL, registers_begin + 48ULL } },

        { "bpl",    { registers_begin + 55ULL, registers_begin + 56ULL } },
        { "bp",     { registers_begin + 54ULL, registers_begin + 56ULL } },
        { "ebp",    { registers_begin + 52ULL, registers_begin + 56ULL } },
        { "rbp",    { registers_begin + 48ULL, registers_begin + 56ULL } },

        { "spl",    { registers_begin + 63ULL, registers_begin + 64ULL } },
        { "sp",     { registers_begin + 62ULL, registers_begin + 64ULL } },
        { "esp",    { registers_begin + 60ULL, registers_begin + 64ULL } },
        { "rsp",    { registers_begin + 56ULL, registers_begin + 64ULL } },

        { "r8b",    { registers_begin + 71ULL, registers_begin + 72ULL } },
        { "r8w",    { registers_begin + 70ULL, registers_begin + 72ULL } },
        { "r8d",    { registers_begin + 68ULL, registers_begin + 72ULL } },
        { "r8",     { registers_begin + 64ULL, registers_begin + 72ULL } },

        { "r9b",    { registers_begin + 79ULL, registers_begin + 80ULL } },
        { "r9w",    { registers_begin + 78ULL, registers_begin + 80ULL } },
        { "r9d",    { registers_begin + 76ULL, registers_begin + 80ULL } },
        { "r9",     { registers_begin + 72ULL, registers_begin + 80ULL } },

        { "r10b",   { registers_begin + 87ULL, registers_begin + 88ULL } },
        { "r10w",   { registers_begin + 86ULL, registers_begin + 88ULL } },
        { "r10d",   { registers_begin + 84ULL, registers_begin + 88ULL } },
        { "r10",    { registers_begin + 80ULL, registers_begin + 88ULL } },

        { "r11b",   { registers_begin + 95ULL, registers_begin + 96ULL } },
        { "r11w",   { registers_begin + 94ULL, registers_begin + 96ULL } },
        { "r11d",   { registers_begin + 92ULL, registers_begin + 96ULL } },
        { "r11",    { registers_begin + 88ULL, registers_begin + 96ULL } },

        { "r12b",   { registers_begin + 103ULL, registers_begin + 104ULL } },
        { "r12w",   { registers_begin + 102ULL, registers_begin + 104ULL } },
        { "r12d",   { registers_begin + 100ULL, registers_begin + 104ULL } },
        { "r12",    { registers_begin + 96ULL, registers_begin + 104ULL } },

        { "r13b",   { registers_begin + 111ULL, registers_begin + 112ULL } },
        { "r13w",   { registers_begin + 110ULL, registers_begin + 112ULL } },
        { "r13d",   { registers_begin + 108ULL, registers_begin + 112ULL } },
        { "r13",    { registers_begin + 104ULL, registers_begin + 112ULL } },

        { "r14b",   { registers_begin + 119ULL, registers_begin + 120ULL } },
        { "r14w",   { registers_begin + 118ULL, registers_begin + 120ULL } },
        { "r14d",   { registers_begin + 116ULL, registers_begin + 120ULL } },
        { "r14",    { registers_begin + 112ULL, registers_begin + 120ULL } },

        { "r15b",   { registers_begin + 127ULL, registers_begin + 128ULL } },
        { "r15w",   { registers_begin + 126ULL, registers_begin + 128ULL } },
        { "r15d",   { registers_begin + 124ULL, registers_begin + 128ULL } },
        { "r15",    { registers_begin + 120ULL, registers_begin + 128ULL } },

        { "cf",     { registers_begin + 128ULL, registers_begin + 129ULL } },
        { "pf",     { registers_begin + 129ULL, registers_begin + 130ULL } },
        { "af",     { registers_begin + 130ULL, registers_begin + 131ULL } },
        { "zf",     { registers_begin + 131ULL, registers_begin + 132ULL } },
        { "sf",     { registers_begin + 132ULL, registers_begin + 133ULL } },
        { "tf",     { registers_begin + 133ULL, registers_begin + 134ULL } },
        { "if",     { registers_begin + 134ULL, registers_begin + 135ULL } },
        { "df",     { registers_begin + 135ULL, registers_begin + 136ULL } },
        { "of",     { registers_begin + 136ULL, registers_begin + 137ULL } },
        { "iopl",   { registers_begin + 137ULL, registers_begin + 138ULL } },
        { "nt",     { registers_begin + 138ULL, registers_begin + 139ULL } },
        { "rf",     { registers_begin + 139ULL, registers_begin + 140ULL } },
        { "vm",     { registers_begin + 140ULL, registers_begin + 141ULL } },
        { "ac",     { registers_begin + 141ULL, registers_begin + 142ULL } },
        { "vif",    { registers_begin + 142ULL, registers_begin + 143ULL } },
        { "vip",    { registers_begin + 143ULL, registers_begin + 144ULL } },
        { "id",     { registers_begin + 144ULL, registers_begin + 145ULL } },

        { "fs",     { registers_begin + 0x98ULL, registers_begin + 0xa0ULL } },
        { "gs",     { registers_begin + 0xa0ULL, registers_begin + 0xa8ULL } },

        { "mm0",     { registers_begin + 0xb0ULL, registers_begin + 0xb8ULL } },
        { "mm1",     { registers_begin + 0xc0ULL, registers_begin + 0xc8ULL } },
        { "mm2",     { registers_begin + 0xd0ULL, registers_begin + 0xd8ULL } },
        { "mm3",     { registers_begin + 0xe0ULL, registers_begin + 0xe8ULL } },
        { "mm4",     { registers_begin + 0xf0ULL, registers_begin + 0xf8ULL } },
        { "mm5",     { registers_begin + 0x100ULL, registers_begin + 0x108ULL } },
        { "mm6",     { registers_begin + 0x110ULL, registers_begin + 0x118ULL } },
        { "mm7",     { registers_begin + 0x120ULL, registers_begin + 0x128ULL } },

        { "xmm0",     { registers_begin + 0x130ULL, registers_begin + 0x140ULL } },
        { "xmm1",     { registers_begin + 0x140ULL, registers_begin + 0x150ULL } },
        { "xmm2",     { registers_begin + 0x150ULL, registers_begin + 0x160ULL } },
        { "xmm3",     { registers_begin + 0x160ULL, registers_begin + 0x170ULL } },
        { "xmm4",     { registers_begin + 0x170ULL, registers_begin + 0x180ULL } },
        { "xmm5",     { registers_begin + 0x180ULL, registers_begin + 0x190ULL } },
        { "xmm6",     { registers_begin + 0x190ULL, registers_begin + 0x1a0ULL } },
        { "xmm7",     { registers_begin + 0x1a0ULL, registers_begin + 0x1b0ULL } },
        { "xmm8",     { registers_begin + 0x1b0ULL, registers_begin + 0x1c0ULL } },
        { "xmm9",     { registers_begin + 0x1c0ULL, registers_begin + 0x1d0ULL } },
        { "xmm10",    { registers_begin + 0x1d0ULL, registers_begin + 0x1e0ULL } },
        { "xmm11",    { registers_begin + 0x1e0ULL, registers_begin + 0x1f0ULL } },
        { "xmm12",    { registers_begin + 0x1f0ULL, registers_begin + 0x200ULL } },
        { "xmm13",    { registers_begin + 0x200ULL, registers_begin + 0x210ULL } },
        { "xmm14",    { registers_begin + 0x210ULL, registers_begin + 0x220ULL } },
        { "xmm15",    { registers_begin + 0x220ULL, registers_begin + 0x230ULL } },


    };
    static std::map<address_range,std::string> const  s_ranges_to_registers = invert_registers_to_ranges(s_registers_to_ranges);

    registers_to_ranges = &s_registers_to_ranges;
    ranges_to_registers = &s_ranges_to_registers;


    uint64_t const  registers_end = std::prev(s_ranges_to_registers.cend())->first.second;
    uint64_t const  temp_shift = 0x20ULL;
    uint64_t const  temp_align = 0x100ULL;
    uint64_t  temp_begin = registers_end + temp_shift;
    if ((temp_begin % temp_align) != 0ULL)
        temp_begin += temp_align - (temp_begin % temp_align);

    return temp_begin;
}


}}}}
