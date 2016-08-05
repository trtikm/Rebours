#include <rebours/MAL/loader/detail/abi_loaders.hpp>
#include <rebours/MAL/loader/assumptions.hpp>
#include <unordered_map>
#include <iostream>

namespace loader { namespace detail { namespace ld_linux_x86_32_so_2 { namespace {


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
        uint32_t const  offset = read_uint32_t(reloc_ptr,load_props.sections_table());
        reloc_ptr += sizeof(uint32_t);

        uint32_t const  info = read_uint32_t(reloc_ptr,load_props.sections_table());
        reloc_ptr += sizeof(uint32_t);

        address const  relocation_address = load_props.fixed_address_for(elf_props->path(),offset);
        address const  base_address = load_props.fixed_base_address_for(elf_props->path());
        uint32_t const  symbol_index = (info >> 8U);
        uint32_t const  action_type = (uint8_t)info;

        uint32_t const  addend = read_uint32_t(relocation_address,load_props.sections_table());

        bool const has_symbol = load_props.has_symbol(elf_props->path(),symbol_index);
        std::string const  symbol_id = has_symbol ? load_props.symbol_id(elf_props->path(),symbol_index) : "" ;
        std::string const  symbol_type = has_symbol ? load_props.symbol_type(elf_props->path(),symbol_index) :
                                                      relocation_type::UNKNOWN() ;

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
        case  0U: //    R_386_NONE         0   none     none
            return "Relocation type 0 is NOT IMPLEMENTED YET.";
        case  1U: //    R_386_32           1   word32   S + A
            return "Relocation type 1 is NOT IMPLEMENTED YET.";
        case  2U: //    R_386_PC32         2   word32   S + A - P
            return "Relocation type 2 is NOT IMPLEMENTED YET.";
        case  3U: //    R_386_GOT32        3   word32   G + A
            return "Relocation type 3 is NOT IMPLEMENTED YET.";
        case  4U: //    R_386_PLT32        4   word32   L + A - P
            return "Relocation type 4 is NOT IMPLEMENTED YET.";
        case  5U: //    R_386_COPY         5   none     none             (i.e. Copy symbol at runtime)
        case  6U: //    R_386_GLOB_DAT     6   word32   S                (i.e. Create GOT entry)
        case  7U: //    R_386_JUMP_SLOT    7   word32   S                (i.e. Create PLT entry)
            {
                std::string const  description_base =
                        (action_type == 5U) ? "R_386_COPY [none]" :
                        (action_type == 6U) ? "R_386_GLOB_DAT [S]" :
                        (action_type == 7U) ? "R_386_JUMP_SLOT [S]" :
                                              "" ;
                address  symbol_address;
                std::string  error_message =
                        load_props.compute_symbol_address(elf_props->path(),symbol_index,symbol_address);
                if (!error_message.empty())
                {
                    load_props.add_skipped_relocation({relocation_address,sizeof(uint32_t),symbol_type,0ULL,0ULL,symbol_id,
                                                       description_base + ": " + error_message});
                    break;
                }
                error_message = write_uint32_t(relocation_address,symbol_address,load_props.sections_table());
                if (!error_message.empty())
                {
                    load_props.add_skipped_relocation({relocation_address,sizeof(uint32_t),symbol_type,0ULL,0ULL,symbol_id,
                                                       description_base + ": The relocation address writes outside loaded sections."});
                    break;
                }
                load_props.add_performed_relocation({relocation_address,sizeof(uint32_t),symbol_type,symbol_address,0ULL,symbol_id,
                                                     description_base});
            }
            break;
        case  8U: //    R_386_RELATIVE     8   word32   B + A            (i.e. Adjust by program base)
            {
                std::string  error_message = write_uint32_t(relocation_address,base_address + addend,
                                                            load_props.sections_table());
                if (!error_message.empty())
                {
                    load_props.add_skipped_relocation({relocation_address,sizeof(uint32_t),symbol_type,0ULL,0ULL,"",
                                                       "R_386_RELATIVE [B + A]: "
                                                       "The relocation address writes outside loaded sections."});
                    break;
                }
                load_props.add_performed_relocation({relocation_address,sizeof(uint32_t),symbol_type,base_address + addend,0ULL,
                                                     "","R_386_RELATIVE [B + A]"});
            }
            break;
        case  9U: //    R_386_GOTOFF       9   word32   S + A - GOT
            return "Relocation type 9 is NOT IMPLEMENTED YET.";
        case 10U: //    R_386_GOTPC       10   word32   GOT + A - P
            return "Relocation type 10 is NOT IMPLEMENTED YET.";
        case 11U: //    R_386_32PLT       11   word32   L + A
            return "Relocation type 11 is NOT IMPLEMENTED YET.";
        case 20U: //    R_386_16          20   word16   S + A
            return "Relocation type 20 is NOT IMPLEMENTED YET.";
        case 21U: //    R_386_PC16        21   word16   S + A - P
            return "Relocation type 21 is NOT IMPLEMENTED YET.";
        case 22U: //    R_386_8           22   word8    S + A
            return "Relocation type 22 is NOT IMPLEMENTED YET.";
        case 23U: //    R_386_PC8         23   word8    S + A - P
            return "Relocation type 23 is NOT IMPLEMENTED YET.";
        case 38U: //    R_386_SIZE32      38   word32   Z + A
            return "Relocation type 38 is NOT IMPLEMENTED YET.";
        default:
            return "Unknown relocation type.";
        }
    }
    return "";
}


}}}}

namespace loader { namespace detail { namespace ld_linux_x86_32_so_2 {


address  choose_address_for_section(section_file_header_ptr const  file_header,
                                    loader::sections_table const&  sections_table)
{
    address const  prefered_start_address = 0xf7400000ULL + file_header->virtual_address();
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
            if (ptr->end_address() <= page_start)
                return page_start;
        }
        page_start += file_header->in_memory_align();
    }
}


void  split_section_by_pltgot(section_ptr const  raw_section,
                              address const  pltgot_address,
                              std::vector<section_ptr>&  output_sections)
{
    ld_linux_x86_64_so_2::split_section_by_pltgot(raw_section,pltgot_address,output_sections);
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
    address const  buckets_begin  = fixed_symbol_hash_table_address + 16ULL + num_mask_words * 4ULL;
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
        uint64_t const  reldyn_relocations_table_address,
        uint64_t const  reldyn_relocations_table_size,
        uint64_t const  reldyn_relocations_table_entry_size,
        uint64_t const  relplt_relocations_table_address,
        uint64_t const  relplt_relocations_table_size,
        uint64_t const  relplt_relocations_table_type,
        uint64_t const  pltgot_address,
        file_props_ptr const  elf_props,
        load_props_elf&  load_props
        )
{
    if (reldyn_relocations_table_address != 0ULL && reldyn_relocations_table_size > 0ULL)
    {
        if (reldyn_relocations_table_entry_size != 8ULL)
            return "Wrong size of entries in '.rel.dyn' REL relocation table.";
        std::string  error_message =
                perform_relocations(reldyn_relocations_table_address,reldyn_relocations_table_size,
                                    pltgot_address,elf_props,load_props);
        if (!error_message.empty())
            return error_message;
    }
    if (relplt_relocations_table_address != 0ULL && relplt_relocations_table_size > 0ULL)
    {
        if (relplt_relocations_table_type != 0ULL && relplt_relocations_table_type != 17ULL)
            return "The x86-32bit ELF binary does not contain '.rel.plt' REL relocation table type.";
        std::string  error_message =
                perform_relocations(relplt_relocations_table_address,relplt_relocations_table_size,
                                    pltgot_address,elf_props,load_props);
        if (!error_message.empty())
            return error_message;
    }
    return "";
}


}}}
