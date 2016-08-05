#include <rebours/MAL/loader/detail/abi_loaders.hpp>
#include <rebours/MAL/loader/special_sections.hpp>
#include <rebours/MAL/loader/assumptions.hpp>
#include <rebours/MAL/loader/invariants.hpp>
#include <rebours/MAL/loader/msgstream.hpp>
#include <unordered_map>
#include <iostream>

namespace loader { namespace detail { namespace ld_linux_x86_64_so_2 { namespace {


std::string  perform_relocations(
        uint64_t const  relocations_table_address,
        uint64_t const  relocations_table_size,
        uint64_t const  pltgot_address,
        file_props_ptr const elf_props,
        load_props_elf& load_props
        )
{
    address const  fixed_pltgot_address = load_props.fixed_address_for(elf_props->path(),pltgot_address);
    (void)fixed_pltgot_address; // Currently we do not use this value.

    for (address  reloc_ptr = load_props.fixed_address_for(elf_props->path(),relocations_table_address),
                  reloc_end = reloc_ptr + relocations_table_size;
            reloc_ptr < reloc_end;
            )
    {
        uint64_t const  offset = read_uint64_t(reloc_ptr,load_props.sections_table());
        reloc_ptr += sizeof(uint64_t);

        uint64_t const  info = read_uint64_t(reloc_ptr,load_props.sections_table());
        reloc_ptr += sizeof(uint64_t);

        uint64_t const  addend = read_uint64_t(reloc_ptr,load_props.sections_table());
        reloc_ptr += sizeof(uint64_t);

        address const  relocation_address = load_props.fixed_address_for(elf_props->path(),offset);
        address const  base_address = load_props.fixed_base_address_for(elf_props->path());
        uint32_t const  symbol_index = (uint32_t)(info >> 32U);
        uint32_t const  action_type = (uint32_t)info;

        std::string const  symbol_file = elf_props->path();//load_props.compute_definition_file_from_symbol_index(symbol_index);
        bool const has_symbol = load_props.has_symbol(elf_props->path(),symbol_index);//!symbol_file.empty();
        std::string const  symbol_id = has_symbol ? load_props.symbol_id(symbol_file,symbol_index) : "" ;
        std::string const  symbol_type = has_symbol ? load_props.symbol_type(symbol_file,symbol_index) : "" ;
        uint64_t const  symbol_value = has_symbol ? load_props.symbol_value(symbol_file,symbol_index) : 0ULL ;

        //Meanings of 'calculation' symbols bellow:
        //    A    Represents the addend used to compute the value of the relocatable field.
        //    B    Represents the base address at which a shared object has been loaded into memory
        //         during execution. Generally, a shared object is built with a 0 base virtual
        //         address, but the execution address will be different.
        //    G    Represents the offset into the global offset table at which the relocation entry’s
        //         symbol will reside during execution.
        //    GOT  Represents the address of the global offset table.
        //    L    Represents the place (section offset or address) of the Procedure Linkage Table
        //         entry for a symbol.
        //    P    Represents the place (section offset or address) of the storage unit being relocated
        //         (computed using r_offset).
        //    S    Represents the value of the symbol whose index resides in the relocation entry.
        //    Z    Represents the size of the symbol whose index resides in the relocation entry.
        switch (action_type)
        {
                 //    Name                Value  Field   Calculation
                 //    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        case  0U: //    R_X86_64_NONE         0   none     none
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_NONE []' is NOT IMPLEMENTED YET."});
            break;
        case  1U: //    R_X86_64_64           1   word64   S + A
            {
                address  symbol_address;
                std::string  error_message =
                        load_props.compute_symbol_address(symbol_file,symbol_index,symbol_address);
                if (!error_message.empty())
                {
                    load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                                       msgstream() << std::hex << "R_X86_64_64 [S{" << symbol_index << "}="
                                                                   << symbol_address << " + A=" << addend << "]: " << error_message
                                                                   << " {file: " << symbol_file << "}"});
                    break;
                }
                error_message = write_address(relocation_address,symbol_address + addend, load_props.sections_table());
                if (!error_message.empty())
                {
                    load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                                       msgstream() << std::hex << "R_X86_64_64 [S{" << symbol_index << "}="
                                                                   << symbol_address << " + A=" << addend << "]: "
                                                                      "The relocation address writes outside loaded sections. "
                                                                   << " {file: " << symbol_file << "}"});
                    break;
                }
                load_props.add_performed_relocation({relocation_address,sizeof(uint64_t),symbol_type,symbol_address + addend,0ULL,symbol_id,
                                                     msgstream() << std::hex << "R_X86_64_64 [S{" << symbol_index << "}="
                                                                 << symbol_address << " + A=" << addend << "] "
                                                                 << " {file: " << symbol_file << "}"});
            }
            break;
        case  2U: //    R_X86_64_PC32         2   word32   S + A - P
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_PC32 [S + A - P]' is NOT IMPLEMENTED YET."});
            break;
        case  3U: //    R_X86_64_GOT32        3   word32   G + A
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_GOT32 [G + A]' is NOT IMPLEMENTED YET."});
            break;
        case  4U: //    R_X86_64_PLT32        4   word32   L + A - P
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_PLT32 [L + A - P]' is NOT IMPLEMENTED YET."});
            break;
        case  5U: //    R_X86_64_COPY         5   none     none             (i.e. Copy symbol at runtime)
        case  6U: //    R_X86_64_GLOB_DAT     6   word64   S                (i.e. Create GOT entry)
        case  7U: //    R_X86_64_JUMP_SLOT    7   word64   S                (i.e. Create PLT entry)
            {
                address  symbol_address;
                std::string  error_message =
                        load_props.compute_symbol_address(symbol_file,symbol_index,symbol_address);

                std::string const  description_base =
                        (action_type == 5U) ? "R_X86_64_COPY [none]" :
                        (action_type == 6U) ? msgstream() << std::hex << "R_X86_64_GLOB_DAT [S{" << symbol_index << "}=" << symbol_address << "]" << msgstream::end() :
                        (action_type == 7U) ? msgstream() << std::hex << "R_X86_64_JUMP_SLOT [S{" << symbol_index << "}=" << symbol_address << "]" << msgstream::end() :
                                              "" ;
                if (!error_message.empty())
                {
                    load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                                       msgstream() << description_base << ": " <<  error_message << " {file: " << symbol_file << "}"});
                    break;
                }
                error_message = write_address(relocation_address,symbol_address,load_props.sections_table());
                if (!error_message.empty())
                {
                    load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                                       msgstream() << description_base << ": The relocation address writes outside loaded sections. "
                                                                   << " {file: " << symbol_file << "}"});
                    break;
                }
                load_props.add_performed_relocation({relocation_address,sizeof(uint64_t),symbol_type,symbol_address,0ULL,symbol_id,
                                                     msgstream() << description_base << " {file: " << symbol_file << "}"});
            }
            break;
        case  8U: //    R_X86_64_RELATIVE     8   word64   B + A            (i.e. Adjust by program base)
            {
                std::string  error_message = write_address(relocation_address,base_address + addend,
                                                           load_props.sections_table());
                if (!error_message.empty())
                {
                    load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,"",
                                                       msgstream() << std::hex << "R_X86_64_RELATIVE [B=" << base_address << " + A=" << addend << "]: "
                                                                                  "The relocation address writes outside loaded sections. "
                                                                               << " {file: " << symbol_file << "}"});
                    break;
                }
                load_props.add_performed_relocation({relocation_address,sizeof(uint64_t),symbol_type,base_address + addend,0ULL,
                                                     "",msgstream() << std::hex << "R_X86_64_RELATIVE [B=" << base_address << " + A=" << addend << "] "
                                                                                << " {file: " << symbol_file << "}"});
            }
            break;
        case  9U: //    R_X86_64_GOTPCREL     9   word32   G + GOT + A - P
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_GOTPCREL [G + GOT + A - P]' is NOT IMPLEMENTED YET."});
            break;
        case 10U: //    R_X86_64_32          10   word32   S + A
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_32 [S + A]' is NOT IMPLEMENTED YET."});
            break;
        case 11U: //    R_X86_64_32S         11   word32   S + A
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_32S [S + A]' is NOT IMPLEMENTED YET."});
            break;
        case 12U: //    R_X86_64_16          12   word16   S + A
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_16 [S + A]' is NOT IMPLEMENTED YET."});
            break;
        case 13U: //    R_X86_64_PC16        13   word16   S + A - P
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_PC16 [S + A - P]' is NOT IMPLEMENTED YET."});
            break;
        case 14U: //    R_X86_64_8           14   word8    S + A
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_8 [S + A]' is NOT IMPLEMENTED YET."});
            break;
        case 15U: //    R_X86_64_PC8         15   word8    S + A - P
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_PC8 [S + A - P]' is NOT IMPLEMENTED YET."});
            break;
        case 16U: //    R_X86_64_DTPMOD64    16   word64
            {
                uint64_t  module_id;
                {
                    auto const  it = load_props.files_table()->find(symbol_file);
                    INVARIANT(it != load_props.files_table()->end());
                    module_id = it->second->id();
                }
                std::string const  error_message = write_address(relocation_address,module_id,load_props.sections_table());
                if (!error_message.empty())
                {
                    load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                                       msgstream() << "R_X86_64_DTPMOD64 {id=" << module_id << "} : The relocation address "
                                                                   << relocation_address << "writes outside loaded sections. "
                                                                   << " {file: " << symbol_file << "}"});
                    break;
                }
                load_props.add_performed_relocation({relocation_address,sizeof(uint64_t),symbol_type,module_id,0ULL,
                                                     "",msgstream() << std::hex << "R_X86_64_DTPMOD64 {id=" << module_id << "}"
                                                                                << " {file: " << symbol_file << "}"});
            }
            break;
        case 17U: //    R_X86_64_DTPOFF64    17   word64
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_DTPOFF64 []' is NOT IMPLEMENTED YET."});
            break;
        case 18U: //    R_X86_64_TPOFF64     18   word64
            {
                special_section::elf_tls_ptr  tls = load_props.tls(has_symbol ? symbol_file : elf_props->path());
                if (!tls.operator bool())
                {
                    load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                                       msgstream() << "R_X86_64_TPOFF64 {A=" << addend << "} The file '" << symbol_file
                                                                   << "' does not contain TLS section."});
                    break;
                }
                uint64_t const  value_to_write = (has_symbol ? : symbol_value) + addend - (tls->end_address() - tls->start_address());
                std::string const  error_message = write_address(relocation_address,value_to_write,load_props.sections_table());
                if (!error_message.empty())
                {
                    load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                                       msgstream() << "R_X86_64_TPOFF64 {A=" << addend << "} The relocation address "
                                                                   << relocation_address << "writes outside loaded sections. "
                                                                   << " {file: " << symbol_file << "}"});
                    break;
                }
                load_props.add_performed_relocation({relocation_address,sizeof(uint64_t),symbol_type,value_to_write,0ULL,
                                                     "",msgstream() << std::hex << "R_X86_64_TPOFF64 {A=" << addend << "} "
                                                                                << " {file: " << symbol_file << "}"});
            }
            break;
        case 19U: //    R_X86_64_TLSGD       19   word32
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_TLSGD []' is NOT IMPLEMENTED YET."});
            break;
        case 20U: //    R_X86_64_TLSLD       20   word32
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_TLSLD []' is NOT IMPLEMENTED YET."});
            break;
        case 21U: //    R_X86_64_DTPOFF32    21   word32
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_DTPOFF32 []' is NOT IMPLEMENTED YET."});
            break;
        case 22U: //    R_X86_64_GOTTPOFF    22   word32
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_GOTTPOFF []' is NOT IMPLEMENTED YET."});
            break;
        case 23U: //    R_X86_64_TPOFF32     23   word32
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_TPOFF32 []' is NOT IMPLEMENTED YET."});
            break;
        case 24U: //    R_X86_64_PC64        24   word64   S + A - P
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_PC64 [S + A - P]' is NOT IMPLEMENTED YET."});
            break;
        case 25U: //    R_X86_64_GOTOFF64    25   word64   S + A - GOT
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_GOTOFF64 [S + A - GOT]' is NOT IMPLEMENTED YET."});
            break;
        case 26U: //    R_X86_64_GOTPC32     26   word32   GOT + A - P
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_GOTPC32 [GOT + A - P]' is NOT IMPLEMENTED YET."});
            break;
        case 27U: //    R_X86_64_GOT64       27   word64   G + A
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_GOT64 [G + A]' is NOT IMPLEMENTED YET."});
            break;
        case 28U: //    R_X86_64_GOTPCREL64  28   word64   G + GOT + A - P
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_GOTPCREL64 [G + GOT + A - P]' is NOT IMPLEMENTED YET."});
            break;
        case 29U: //    R_X86_64_GOTPC64     29   word64   GOT + A - P
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_GOTPC64 [GOT + A - P]' is NOT IMPLEMENTED YET."});
            break;
        case 30U: //    R_X86_64_GOTPLT64    30   word64   G + A
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_GOTPLT64 [G + A]' is NOT IMPLEMENTED YET."});
            break;
        case 31U: //    R_X86_64_PLTOFF64    31   word64   L + A - GOT
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_PLTOFF64 [L + A - GOT]' is NOT IMPLEMENTED YET."});
            break;
        case 32U: //    R_X86_64_SIZE32      32   word32   Z + A
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_SIZE32 [Z + A]' is NOT IMPLEMENTED YET."});
            break;
        case 33U: //    R_X86_64_SIZE64      33   word64   Z + A
            load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,
                                               "Relocation type 'R_X86_64_SIZE64 [Z + A]' is NOT IMPLEMENTED YET."});
            break;
        default:
            {
                std::stringstream  sstr;
                sstr << "Unknown relocation type '" << action_type << "'. " << " {file: " << symbol_file << "}";
                load_props.add_skipped_relocation({relocation_address,sizeof(uint64_t),symbol_type,0ULL,0ULL,symbol_id,sstr.str()});
            }
            break;
        }
    }
    return "";
}


}}}}

namespace loader { namespace detail { namespace ld_linux_x86_64_so_2 {


address  choose_address_for_section(section_file_header_ptr const  file_header,
                                    loader::sections_table const&  sections_table)
{
    uint64_t const mem_page_size = 0x1000ULL;

    address const  prefered_start_address = 0x7f4000000000ULL + file_header->virtual_address();
    address  page_start = prefered_start_address < file_header->in_memory_align() ? file_header->in_memory_align() :
                                                                                    prefered_start_address ;
    ASSUMPTION(page_start % file_header->in_memory_align() == file_header->offset() % file_header->in_memory_align());

    if (sections_table.empty())
        return page_start;

    while (true)
    {
        auto const  it = sections_table.upper_bound(page_start);
        if (it == sections_table.cend())
        {
            section_ptr const  ptr = std::prev(it)->second;
            address  page_aligned_end =
                    ptr->end_address() + (ptr->end_address() % mem_page_size == 0ULL ? 0ULL : mem_page_size - ptr->end_address() % mem_page_size);
            if (page_aligned_end <= page_start)
                return page_start;
        }
        page_start += file_header->in_memory_align();
    }
}


void  split_section_by_pltgot(section_ptr const  raw_section,
                              address const  pltgot_address,
                              std::vector<section_ptr>&  output_sections)
{
    if (pltgot_address == 0ULL ||
            pltgot_address <= raw_section->section_file_header()->virtual_address() ||
            pltgot_address >= raw_section->section_file_header()->virtual_address() +
                              raw_section->section_file_header()->size_in_memory() - 1ULL ||
            raw_section->has_execute_access() ||
            !raw_section->has_read_access() ||
            !raw_section->has_write_access())
    {
        output_sections.push_back(raw_section);
        return;
    }
    ASSUMPTION(raw_section->section_file_header()->virtual_address() <= raw_section->start_address());
    address const  fixed_pltgot_address =
            raw_section->start_address() + (pltgot_address - raw_section->section_file_header()->virtual_address());
    output_sections.push_back(
            section_ptr{
                    new section{raw_section->start_address(),
                                fixed_pltgot_address,
                                section_content_ptr(
                                        new section_content{
                                                raw_section->content()->cbegin(),
                                                raw_section->content()->cbegin() +
                                                        fixed_pltgot_address - raw_section->start_address()
                                                }
                                        ),
                                true,
                                false,
                                false,
                                raw_section->is_in_big_endian(),
                                raw_section->has_const_endian(),
                                raw_section->file_props(),
                                section_file_header_ptr(
                                        new section_file_header{
                                                raw_section->section_file_header()->offset(),
                                                raw_section->section_file_header()->virtual_address(),
                                                fixed_pltgot_address - raw_section->start_address(),
                                                fixed_pltgot_address - raw_section->start_address(),
                                                raw_section->section_file_header()->in_file_align(),
                                                raw_section->section_file_header()->in_memory_align()
                                                }
                                        )
                                }
                    }
            );
    output_sections.push_back(
            section_ptr{
                    new section{fixed_pltgot_address,
                                raw_section->end_address(),
                                section_content_ptr(
                                        new section_content(
                                                raw_section->content()->cbegin() +
                                                        fixed_pltgot_address - raw_section->start_address(),
                                                raw_section->content()->cend()
                                                )
                                        ),
                                true,
                                true,
                                false,
                                raw_section->is_in_big_endian(),
                                raw_section->has_const_endian(),
                                raw_section->file_props(),
                                section_file_header_ptr(
                                        new section_file_header{
                                                raw_section->section_file_header()->offset() +
                                                        fixed_pltgot_address - raw_section->start_address(),
                                                raw_section->section_file_header()->virtual_address() +
                                                        fixed_pltgot_address - raw_section->start_address(),
                                                raw_section->section_file_header()->size_in_file() -
                                                        (fixed_pltgot_address - raw_section->start_address()),
                                                raw_section->section_file_header()->size_in_memory() -
                                                        (fixed_pltgot_address - raw_section->start_address()),
                                                raw_section->section_file_header()->in_file_align(),
                                                raw_section->section_file_header()->in_memory_align()
                                                }
                                        )
                                }
                    }
            );
}


std::string  special_symbol_hash_table_parser(
        uint64_t const  symbol_hash_table_address,
        uint64_t const  symbol_hash_table_type,
        uint64_t&  symbol_table_begin_index,
        uint64_t&  symbol_table_end_index,
        std::unordered_set<uint64_t>& symbol_table_indices,
        file_props_ptr const elf_props,
        load_props_elf& load_props
        )
{
    if (symbol_hash_table_type != 0x6ffffef5ULL) // Not the DT_GNU_HASH table?
        return "Unknown hash table type.";

    address const  fixed_symbol_hash_table_address = load_props.fixed_address_for(elf_props->path(),symbol_hash_table_address);

    uint32_t const  num_buckets  = read_uint32_t(fixed_symbol_hash_table_address,load_props.sections_table());
    uint32_t const  symbols_base  = read_uint32_t(fixed_symbol_hash_table_address + 4ULL,load_props.sections_table());
    uint32_t const  num_mask_words  = read_uint32_t(fixed_symbol_hash_table_address + 8ULL,load_props.sections_table());
    address const  buckets_begin  = fixed_symbol_hash_table_address + 16ULL + num_mask_words * 8ULL;
    address const  buckets_end  = buckets_begin + num_buckets * 4ULL;
    address const  chains_begin  = buckets_end;

    symbol_table_begin_index = symbols_base;

    if (num_buckets == 0U)
    {
        symbol_table_end_index = symbols_base + 1ULL;
        return "";
    }

    symbol_table_end_index = 0ULL;
    for (address  bucket_ptr = buckets_begin; bucket_ptr != buckets_end; bucket_ptr += 4U)
    {
        uint32_t const  symbol_index = read_uint32_t(bucket_ptr,load_props.sections_table());
        if (symbol_index == 0U)
            continue;
        if (symbol_index < symbols_base)
            return "A non-zero bucket in the GNU HASH symbol table is smaller than the symbol-base index.";
        if (symbol_table_end_index >= symbol_index)
            return "Bucket non-zero values in the GNU HASH symbol table must be increasing.";
        symbol_table_end_index = symbol_index;
        symbol_table_indices.insert(symbol_table_end_index);
        for (address  chain_ptr = chains_begin + (symbol_table_end_index - symbols_base) * 4ULL;
                (read_uint32_t(chain_ptr,load_props.sections_table()) & 1U) == 0U;
                chain_ptr += 4ULL)
        {
            ++symbol_table_end_index;
            symbol_table_indices.insert(symbol_table_end_index);
        }

    }

    if (symbol_table_end_index == 0ULL)
        symbol_table_end_index = symbols_base;

    ++symbol_table_end_index;

    return "";
}


std::string  perform_relocations(
        uint64_t const  reladyn_relocations_table_address,
        uint64_t const  reladyn_relocations_table_size,
        uint64_t const  reladyn_relocations_table_entry_size,
        uint64_t const  relaplt_relocations_table_address,
        uint64_t const  relaplt_relocations_table_size,
        uint64_t const  relaplt_relocations_table_type,
        uint64_t const  pltgot_address,
        file_props_ptr const  elf_props,
        load_props_elf&  load_props
        )
{
    if (reladyn_relocations_table_address != 0ULL && reladyn_relocations_table_size > 0ULL)
    {
        if (reladyn_relocations_table_entry_size != 3ULL * sizeof(uint64_t))
            return "Wrong size of entries in '.rela.dyn' RELA relocation table.";
        std::string  error_message =
                perform_relocations(reladyn_relocations_table_address,reladyn_relocations_table_size,
                                    pltgot_address,elf_props,load_props);
        if (!error_message.empty())
            return error_message;
    }
    if (relaplt_relocations_table_address != 0ULL && relaplt_relocations_table_size > 0ULL)
    {
        if (relaplt_relocations_table_type != 0ULL && relaplt_relocations_table_type != 7ULL)
            return "The x86-64bit ELF binary does not contain '.rela.plt' RELA relocation table type.";
        std::string  error_message =
                perform_relocations(relaplt_relocations_table_address,relaplt_relocations_table_size,
                                    pltgot_address,elf_props,load_props);
        if (!error_message.empty())
            return error_message;
    }
    return "";
}


}}}
