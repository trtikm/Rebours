#ifndef REBOURS_MAL_LOADER_PLATFORM_HPP
#   define REBOURS_MAL_LOADER_PLATFORM_HPP

#include <memory>
#include <string>

namespace loader {
  /* CPU architectures */
  enum class architecture_t {
    ARM,
    ARM64,
    MIPS,
    POWERPC,
    SPARC,
    SPARC64,
    X86_32,
    X86_64,
    UNKNOWN_ARCH
  };

  /* Application Binary Interfaces (ABI) */
  enum class abi_t {
    DARWIN,
    LINUX,
    OPENBSD,
    SOLARIS,
    WINDOWS,
    UNIX,
    UNKNOWN_ABI
  };

  /* Store the CPU architecture and the ABI */
  class platform {
  private:
    architecture_t m_architecture;
    abi_t m_abi;
  public:
    platform(architecture_t const&, abi_t const&);
    architecture_t const& architecture() const { return m_architecture; }
    abi_t const& abi() const { return m_abi; }
  };

  typedef std::shared_ptr<platform const> platform_ptr;
}

/* Operators on 'platform' struct */
bool operator==(loader::platform const&, loader::platform const&);
inline bool operator!=(loader::platform const& l, loader::platform const& r)
{
  return !(l == r);
}

/* Output operators for architecture and abi */
std::ostream& operator<<(std::ostream&, loader::architecture_t);
std::ostream& operator<<(std::ostream&, loader::abi_t);

#endif /* LOADER_PLATFORM_HPP */
