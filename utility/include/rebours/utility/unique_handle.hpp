#ifndef REBOURS_BITVECTORS_DETAIL_UNIQUE_HANDLES_HPP_INCLUDED
#   define REBOURS_BITVECTORS_DETAIL_UNIQUE_HANDLES_HPP_INCLUDED

#   include <cstdint>

/**
 * Each instances of this type possesses a unique number. More precisely, it is
 * impossible for two different instances of this type to exist at the same time
 * and to store the same number. Construction of instances is thread-safe.
 *
 * The type is usefull to distinguish some global (shared) data. For instance,
 * generation of temporary files.
 */
struct unique_handle
{
    unique_handle();
    ~unique_handle();
    unique_handle& operator=(unique_handle const& ) = delete;
    unique_handle(unique_handle const& ) = delete;

    uint32_t value() const noexcept { return m_id; }
    operator uint32_t() const noexcept { return value(); }

private:
    uint32_t  m_id;
};


#endif
