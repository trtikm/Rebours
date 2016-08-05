#ifndef REBOURS_MAL_LOADER_DETAIL_ABI_LOADERS_HPP_INCLUDED
#   define REBOURS_MAL_LOADER_DETAIL_ABI_LOADERS_HPP_INCLUDED

#   include <rebours/MAL/loader/detail/mutable_sections_table.hpp>
#   include <rebours/MAL/loader/detail/address_fixing.hpp>
#   include <rebours/MAL/loader/detail/load_props_elf.hpp>
#   include <rebours/MAL/loader/address.hpp>
#   include <functional>
#   include <unordered_set>
#   include <vector>

namespace loader { namespace detail {


typedef std::function<address(section_file_header_ptr,loader::sections_table const&)>
        abi_virtual_address_selector;

typedef std::function<void(section_ptr,address,std::vector<section_ptr>&)>
        abi_pltgot_section_split_func;

typedef std::function<std::string(uint64_t,uint64_t,                // Hash table data (address and type)
                                  uint64_t&,uint64_t&,              // Output start and end indices of the symbol table
                                  std::unordered_set<uint64_t>&,    // Output set of all adressable indices in the symbol table
                                  file_props_ptr,load_props_elf&
                                  )>
        abi_special_symbol_hash_table_parser;

typedef std::function<std::string(uint64_t,uint64_t,uint64_t,       // RELA relocation table data (i.e. .rela.dyn)
                                  uint64_t,uint64_t,uint64_t,       // JMPREL relocation table data (i.e. .rela.plt)
                                  uint64_t,                         // pltgot section data
                                  file_props_ptr,load_props_elf&)>
        abi_relocations_fn;


}}

namespace loader { namespace detail { namespace ld_linux_x86_64_so_2 {


address  choose_address_for_section(section_file_header_ptr const  file_header,
                                    loader::sections_table const&  sections_table);

void  split_section_by_pltgot(section_ptr const  raw_section,
                              address const  pltgot_address,
                              std::vector<section_ptr>&  output_sections);

std::string  special_symbol_hash_table_parser(
        uint64_t const  symbol_hash_table_address,
        uint64_t const  symbol_hash_table_type,
        uint64_t&  symbol_table_begin_index,
        uint64_t&  symbol_table_end_index,
        std::unordered_set<uint64_t>& symbol_table_indices,
        file_props_ptr const elf_props,
        load_props_elf& load_props
        );


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
        );


}}}

namespace loader { namespace detail { namespace ld_linux_x86_32_so_2 {


address  choose_address_for_section(section_file_header_ptr const  file_header,
                                    loader::sections_table const&  sections_table);

void  split_section_by_pltgot(section_ptr const  raw_section,
                              address const  pltgot_address,
                              std::vector<section_ptr>&  output_sections);

std::string  special_symbol_hash_table_parser(
        uint64_t const  symbol_hash_table_address,
        uint64_t const  symbol_hash_table_type,
        uint64_t&  symbol_table_begin_index,
        uint64_t&  symbol_table_end_index,
        std::unordered_set<uint64_t>& symbol_table_indices,
        file_props_ptr const elf_props,
        load_props_elf& load_props
        );


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
        );


}}}

#endif
