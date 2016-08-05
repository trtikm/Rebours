#include <rebours/MAL/loader/platform.hpp>

#include <iostream>

std::ostream& operator<<(std::ostream& os, loader::architecture_t type)
{
  switch(type) {
  case loader::architecture_t::ARM          : os << "Arm"; break;
  case loader::architecture_t::ARM64        : os << "Arm64"; break;
  case loader::architecture_t::MIPS         : os << "MIPS"; break;
  case loader::architecture_t::POWERPC      : os << "PowerPC"; break;
  case loader::architecture_t::SPARC        : os << "Sparc"; break;
  case loader::architecture_t::SPARC64      : os << "Sparc64"; break;
  case loader::architecture_t::X86_32       : os << "x86-32"; break;
  case loader::architecture_t::X86_64       : os << "x86-64"; break;
  case loader::architecture_t::UNKNOWN_ARCH : os << "Unknown"; break;
  default                                   : os.setstate(std::ios_base::failbit);
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, loader::abi_t type)
{
  switch(type) {
  case loader::abi_t::DARWIN      : os << "Apple Darwin"; break;
  case loader::abi_t::LINUX       : os << "Linux"; break;
  case loader::abi_t::OPENBSD     : os << "OpenBSD"; break;
  case loader::abi_t::SOLARIS     : os << "Oracle Solaris"; break;
  case loader::abi_t::WINDOWS     : os << "MS-Windows"; break;
  case loader::abi_t::UNIX        : os << "UNIX"; break;
  case loader::abi_t::UNKNOWN_ABI : os << "Unknown"; break;
  default                         : os.setstate(std::ios_base::failbit);
  }
  return os;
}

loader::platform::platform(architecture_t const& arch, abi_t const& abi)
  : m_architecture(arch), m_abi(abi)
{}

bool operator==(loader::platform const& l, loader::platform const& r)
{
  return ((l.architecture() == r.architecture()) && (l.abi() == r.abi()));
}
