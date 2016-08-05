#include <rebours/bitvectors/detail/unique_handles.hpp>
#include <rebours/bitvectors/invariants.hpp>
#include <mutex>
#include <set>

namespace bv { namespace detail { namespace {

std::mutex  mutex_for_handles;
std::set<uint32_t>  handles;

}}}

namespace bv { namespace detail {


unique_handle::unique_handle()
    : m_id{0U}
{
    std::lock_guard<std::mutex> const  lock(mutex_for_handles);
    auto const it = handles.crbegin();
    m_id = it == handles.crend() ? 0U : *it + 1U;
    INVARIANT(handles.count(m_id) == 0ULL);
    handles.insert(m_id);
}

unique_handle::~unique_handle()
{
    std::lock_guard<std::mutex> const  lock(mutex_for_handles);
    handles.erase(m_id);
}


}}
