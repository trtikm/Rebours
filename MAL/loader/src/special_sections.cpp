#include <rebours/MAL/loader/special_sections.hpp>

namespace loader { namespace special_section_name {


std::string  thread_local_storage() { return ".tls"; }
std::string  thread_local_storage_initialisers() { return "thread_local_storage_initialisers"; }
std::string  resources() { return ".rsrc"; }
std::string  load_configuration_structure() { return "load_configuration_structure"; }
std::string  exceptions() { return ".pdata"; }


}}
