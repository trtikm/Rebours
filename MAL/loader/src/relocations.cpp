#include <rebours/MAL/loader/relocations.hpp>

namespace loader { namespace relocation_type {


std::string  UNKNOWN() { return "UNKNOWN"; }
std::string  DATA() { return "DATA"; }
std::string  FUNCTION() { return "FUNCTION"; }
std::string  COMMON() { return "COMMON"; }
std::string  TLS() { return "TLS"; }


}}

namespace loader {


relocation::relocation(address const  start_address,
                       uint64_t const  num_bytes,
                       std::string const&  type,
                       uint64_t const  stored_value,
                       uint64_t const  original_value,
                       relocation_symbol_id const&  symbol_id,
                       std::string const&  description)
    : m_start_address(start_address)
    , m_num_bytes(num_bytes)
    , m_type(type)
    , m_stored_value(stored_value)
    , m_original_value(original_value)
    , m_symbol_id(symbol_id)
    , m_description(description)
{}


}
