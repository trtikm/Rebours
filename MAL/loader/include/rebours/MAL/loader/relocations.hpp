#ifndef REBOURS_MAL_LOADER_RELOCATIONS_HPP_INCLUDED
#   define REBOURS_MAL_LOADER_RELOCATIONS_HPP_INCLUDED

#   include <rebours/MAL/loader/address.hpp>
#   include <rebours/MAL/loader/detail/std_pair_hash.hpp>
#   include <cstdint>
#   include <map>
#   include <unordered_map>
#   include <string>
#   include <memory>
#   include <tuple>


/**
 * A relocation is a process of updating pointers inside a loaded binary before it is executed.
 * The updates are necessary, if the binary is loaded to a different base address than expected
 * by the binary file on the disc.
 *
 * This module provides a history of all performed and also skipped relocations in all loaded
 * binary files. Note that a relocation can be skipped if the computation of a corrected pointer
 * fails. This may, for example, happen when pointer references a function of dynamic library
 * which was not loaded with the root binary file. We store performed and skipped relocations
 * separately. Mixing of performed and skipped relocations is not allowed.
 */


namespace loader { namespace relocation_type {


/**
 * Here are string constants denoting types of performed relocations. Although there can
 * be several ABI-specific types of relocations, we distinguish only two (the most important)
 * types: data and function relocation. Otherwise, a relocation type is unknown.
 */

std::string  UNKNOWN();
std::string  DATA();
std::string  FUNCTION();
std::string  COMMON();
std::string  TLS();


}}

namespace loader {


/**
 * There are several mechanisms accross different system how computation of relocated pointer
 * is performed. One important type is based on symbols. A symbol is a string associated with
 * a pointed object (like a global variable or a function). It means that a binary holds a table
 * of symbols and pointers to the corresponding objects. Relocations of all pointers to these
 * objects accross all loaded binaries are perfomed by looking into the table for symbols as keys.
 */
typedef std::string  relocation_symbol_id;


/**
 * It represents a single record in a history of all performed or skipped relocations.
 */
struct relocation
{
    relocation(address const  start_address,
                    //!< A pointer to the first byte of a memory block where the relocation was performed.
               uint64_t const  num_bytes,
                    //!< A size of the updated memory.
               std::string const&  type,
                    //!< A type of the performed relocation. Possible values are string constants defined
                    //!< in the namespace loader::relocation_type.
               uint64_t const  stored_value,
                    //!< It is the computed (relocated) address which was written to the memory addressed above.
               uint64_t const  original_value,
                    //!< It is the address which originaly appears in the memory addressed above.
                    //!< Note that this address is not correct, i.e. it does not points to the desired object.
               relocation_symbol_id const&  symbol_id,
                    //!< It is symbol (string) which was used in the computation of the relocated pointer.
                    //!< If the computation was NOT based on any symbol, than the string should be empty.
               std::string const&  description
                    //!< A textual description describing either the performed computation or any other
                    //!< information related to the relocation. Its only purpose is to provide a human user
                    //!< more details about this history record.
               );

    address  start_address() const { return m_start_address; }
    uint64_t  num_bytes() const { return m_num_bytes; }
    std::string const&  type() const { return m_type; }
    uint64_t  stored_value() const { return m_stored_value; }
    uint64_t  original_value() const { return m_original_value; }
    relocation_symbol_id const&  symbol_id() const { return m_symbol_id; }
    std::string const&  description() const { return m_description; }
private:
    address  m_start_address;
    uint64_t  m_num_bytes;
    std::string  m_type;
    uint64_t  m_stored_value;
    uint64_t  m_original_value;
    relocation_symbol_id  m_symbol_id;
    std::string  m_description;
};


/**
 * It represents a history of all relocations either performed or skipped in all loaded binary files.
 * Since individual relocations are always performed on disjoint memory blocks, an order in which they
 * are applied is not important. So, we sort them by their start address. It means that we hold them
 * in an std::map.
 */
typedef std::map<address,relocation>  relocations;


typedef std::shared_ptr<relocations const>  relocations_ptr;


typedef std::unordered_map<std::pair<std::string,               // binary file
                                     uint64_t                   // index into the symbol table
                                     >,
                           std::tuple<relocation_symbol_id,     // symbol name
                                      std::string,              // relocation type (DATA, FUNCTION, UNKNOWN)
                                      uint64_t,                 // symbol's value (e.g. its address)
                                      uint64_t                  // size of the symbol in memory
                                      > >
        symbol_table;


typedef std::shared_ptr<symbol_table const>  symbol_table_ptr;

}

#endif
