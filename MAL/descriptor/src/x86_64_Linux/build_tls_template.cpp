#include <rebours/MAL/descriptor/storage.hpp>
#include <rebours/utility/assumptions.hpp>
#include <rebours/utility/invariants.hpp>
#include <rebours/utility/endian.hpp>
#include <rebours/utility/config.hpp>
#include <vector>
#include <utility>
#include <algorithm>
#include <iterator>
#if PLATFORM() == PLATFORM_WINDOWS()
#   include <rebours/utility/large_types.hpp>
#   pragma warning(disable : 4200)
#endif


namespace {


uint64_t  to_little_endian(uint64_t  value)
{
    if (!is_this_little_endian_machine())
    {
        uint8_t*  b = reinterpret_cast<uint8_t*>(&value);
        std::reverse(b,b + sizeof(value));
    }
    return value;
}

template<typename T>
void  write_to_buffer(T const  value, std::vector<uint8_t>&  buffer, uint64_t  offset)
{
    ASSUMPTION(buffer.size() >= offset + sizeof(value));
    for (uint8_t const*  b = reinterpret_cast<uint8_t const*>(&value), * const e = b + sizeof(value); b != e; ++b, ++offset)
        buffer.at(offset) = *b;
}


}

namespace tcb_glibc { namespace {

namespace detail {

typedef struct {
  int i[4];
} __128bits;

typedef union dtv
{
  size_t counter;
  struct
  {
    void *val;
    bool is_static;
  } pointer;
} dtv_t;

typedef struct
{
  void *tcb;        /* Pointer to the TCB.  Not necessarily the thread descriptor used by libpthread.  */
  dtv_t *dtv;
  void *self;       /* Pointer to the thread descriptor.  */
  int multiple_threads;
  int gscope_flag;
  uintptr_t sysinfo;
  uintptr_t stack_guard;
  uintptr_t pointer_guard;
  unsigned long int vgetcpu_cache[2];
# ifndef __ASSUME_PRIVATE_FUTEX
  int private_futex;
# else
  int __glibc_reserved1;
# endif
  int rtld_must_xmm_save;
  /* Reservation of some values for the TM ABI.  */
  void *__private_tm[4];
  /* GCC split stack support.  */
  void *__private_ss;
  long int __glibc_reserved2;
  /* Have space for the post-AVX register size.  */
#if PLATFORM() == PLATFORM_LINUX()
  __128bits rtld_savespace_sse[8][4] __attribute__ ((aligned (32)));
#elif PLATFORM() == PLATFORM_WINDOWS()
  uint128_t rtld_savespace_sse[8][4];
#elif PLATFORM() == PLATFORM_WINDOWS()
#   error "Support of Linux TLS section is not implemented on MacOS build of Rebours."
#else
#   error "Unsupported platform"
#endif

  void *__padding[8];
} tcbhead_t;


typedef struct {} list_t;
struct robust_list_head {};
struct _pthread_cleanup_buffer;
struct pthread_unwind_buf;
typedef struct {} td_eventbuf_t;
struct pthread_key_data;
struct __res_state {};

struct pthread
{
  union
  {
    /* This overlaps the TCB as used for TLS without threads (see tls.h).  */
    tcbhead_t header;

    /* This extra padding has no special purpose, and this structure layout
       is private and subject to change without affecting the official ABI.
       We just have it here in case it might be convenient for some
       implementation-specific instrumentation hack or suchlike.  */
    void *__padding[24];
  };

  /* This descriptor's link on the `stack_used' or `__stack_user' list.  */
  list_t list;

  /* Thread ID - which is also a 'is this thread descriptor (and
     therefore stack) used' flag.  */
#if PLATFORM() == PLATFORM_LINUX()
  pid_t tid;
#elif PLATFORM() == PLATFORM_WINDOWS()
  uint64_t tid;
#elif PLATFORM() == PLATFORM_WINDOWS()
#   error "Support of Linux TLS section is not implemented on MacOS build of Rebours."
#else
#   error "Unsupported platform"
#endif

  /* Process ID - thread group ID in kernel speak.  */
#if PLATFORM() == PLATFORM_LINUX()
  pid_t pid;
#elif PLATFORM() == PLATFORM_WINDOWS()
  uint64_t pid;
#elif PLATFORM() == PLATFORM_WINDOWS()
#   error "Support of Linux TLS section is not implemented on MacOS build of Rebours."
#else
#   error "Unsupported platform"
#endif

  /* List of robust mutexes the thread is holding.  */
  void *robust_prev;
  struct robust_list_head robust_head;

  /* List of cleanup buffers.  */
  struct _pthread_cleanup_buffer *cleanup;

  /* Unwind information.  */
  struct pthread_unwind_buf *cleanup_jmp_buf;

  /* Flags determining processing of cancellation.  */
  int cancelhandling;
  /* Bit set if cancellation is disabled.  */

  /* Flags.  Including those copied from the thread attribute.  */
  int flags;

  /* We allocate one block of references here.  This should be enough
     to avoid allocating any memory dynamically for most applications.  */
  struct pthread_key_data
  {
    /* Sequence number.  We use uintptr_t to not require padding on
       32- and 64-bit machines.  On 64-bit machines it helps to avoid
       wrapping, too.  */
    uintptr_t seq;

    /* Data pointer.  */
    void *data;
  } specific_1stblock[2];

  /* Two-level array for the thread-specific data.  */
  struct pthread_key_data *specific[2];

  /* Flag which is set when specific data is set.  */
  bool specific_used;

  /* True if events must be reported.  */
  bool report_events;

  /* True if the user provided the stack.  */
  bool user_stack;

  /* True if thread must stop at startup time.  */
  bool stopped_start;

  /* The parent's cancel handling at the time of the pthread_create
     call.  This might be needed to undo the effects of a cancellation.  */
  int parent_cancelhandling;

  /* Lock to synchronize access to the descriptor.  */
  int lock;

  /* Lock for synchronizing setxid calls.  */
  int setxid_futex;

  /* If the thread waits to join another one the ID of the latter is
     stored here.

     In case a thread is detached this field contains a pointer of the
     TCB if the thread itself.  This is something which cannot happen
     in normal operation.  */
  struct pthread *joinid;
  /* Check whether a thread is detached.  */

  /* The result of the thread function.  */
  void *result;

  /* Scheduling parameters for the new thread.  */
#if PLATFORM() == PLATFORM_WINDOWS()
  struct sched_param {
      int sched_priority;
  } schedparam;
#else
  struct sched_param schedparam;
#endif
  int schedpolicy;

  /* Start position of the code to be executed and the argument passed
     to the function.  */
  void *(*start_routine) (void *);
  void *arg;

  /* Debug state.  */
  td_eventbuf_t eventbuf;
  /* Next descriptor with a pending event.  */
  struct pthread *nextevent;

  /* If nonzero pointer to area allocated for the stack and its
     size.  */
  void *stackblock;
  size_t stackblock_size;
  /* Size of the included guard area.  */
  size_t guardsize;
  /* This is what the user specified and what we will report.  */
  size_t reported_guardsize;

  /* Thread Priority Protection data.  */
  struct priority_protection_data *tpp;

  /* Resolver state.  */
  struct __res_state res;

  /* This member must be last.  */
  char end_padding[];
};

}

inline constexpr uint64_t  get_tcb_size() noexcept { return sizeof(struct detail::pthread); }
inline constexpr uint64_t  get_offset_tcb() noexcept { return 0ULL; }
inline constexpr uint64_t  get_offset_dvt() noexcept { return 8ULL; }
inline constexpr uint64_t  get_offset_self() noexcept { return 16ULL; }


}}




namespace mal { namespace descriptor { namespace detail { namespace x86_64_Linux {


namespace tcb = tcb_glibc;

void  build_tls_template(std::vector<uint8_t>&  content, uint64_t&  offset, std::vector<uint64_t>&  relocations, file_descriptor_ptr const  descriptor)
{
    ASSUMPTION(descriptor->files_table()->size() > 0ULL);

    std::vector< std::pair<bool,std::vector<uint8_t> > >  data(descriptor->files_table()->size() + 1ULL,{false,{}});
    {
        for (auto const&  file_sections : *descriptor->special_sections())
            for (auto const  name_props : file_sections.second)
                if (name_props.first == loader::special_section_name::thread_local_storage())
                    if (loader::special_section::elf_tls_ptr const  tls = std::dynamic_pointer_cast<loader::special_section::elf_tls const>(name_props.second))
                    {
                        INVARIANT(tls->end_address() - tls->start_address() > 0ULL);
                        INVARIANT(descriptor->files_table()->count(file_sections.first) != 0ULL);

                        uint32_t const  module_id = descriptor->files_table()->at(file_sections.first)->id();
                        INVARIANT(module_id  > 0U && module_id < data.size());
                        INVARIANT(data.at(module_id).second.empty());

                        data.at(module_id).first = tls->use_static_scheme();

                        uint64_t  begin = tls->start_address();
                        loader::sections_table::const_iterator  section_begin = descriptor->sections_table()->lower_bound(begin);
                        if (section_begin == descriptor->sections_table()->cend() || section_begin->first > begin)
                        {
                            section_begin = section_begin == descriptor->sections_table()->cbegin() ? section_begin : std::prev(section_begin,1);
                            INVARIANT(!(section_begin == descriptor->sections_table()->cend() ||
                                        section_begin->first > begin ||
                                        section_begin->second->end_address() <= begin));
                        }
                        uint64_t const  end = tls->image_end_address();
                        loader::sections_table::const_iterator const  section_end = descriptor->sections_table()->lower_bound(end);
                        INVARIANT(section_begin != section_end);
                        for ( ; section_begin != section_end && begin < end; ++section_begin)
                        {
                            INVARIANT(section_begin->first <= begin);
                            INVARIANT(section_begin->second->end_address() > begin);

                            uint64_t const  copy_end = std::min(section_begin->second->end_address(),end);
                            std::copy(section_begin->second->content()->data() + (begin - section_begin->first),
                                      section_begin->second->content()->data() + (copy_end - section_begin->first),
                                      std::back_inserter(data.at(module_id).second)
                                      );
                            begin = copy_end;
                        }
                        INVARIANT(begin == end);
                        INVARIANT(data.at(module_id).second.size() == tls->image_end_address() - tls->start_address());

                        for (uint64_t  i = tls->image_end_address(); i < tls->end_address(); ++i)
                            data.at(module_id).second.push_back(0U);
                        while (data.at(module_id).second.size() % tls->alignment() != 0ULL)
                            data.at(module_id).second.push_back(0U);
                    }
    }

    static_assert(sizeof(tcb_glibc::detail::dtv_t::pointer) == 2ULL * sizeof(uint64_t),
                  "We assume each TCB element is 16 bytes long (a pair '64_bit_address,is_static_boolean').");

    std::vector<uint64_t>  offsets(2ULL * data.size(),to_little_endian(0ULL));  // TODO: the array should be initialised by ABI-specific
                                                                                //       'invalid pointer' values (not by 0ULL).
    uint64_t  tcb_begin = 0ULL;
    uint64_t  tcb_end = 0ULL;
    uint64_t  offsets_end_offset = 0ULL;
    uint64_t  content_end_offset = 0ULL;
    {
        uint64_t  shift = 0ULL;

        for (uint64_t  module_id = 1ULL; module_id < data.size(); ++module_id)
            if (!data.at(module_id).second.empty() && data.at(module_id).first)
            {
                offsets.at(2ULL * module_id) = shift;
                offsets.at(2ULL * module_id + 1ULL) = 1ULL;
                shift += data.at(module_id).second.size();
            }

        tcb_begin = shift;
        tcb_end = tcb_begin + tcb::get_tcb_size();

        shift += tcb_end - tcb_begin;
        shift += offsets.size() * sizeof(uint64_t);
        shift += 512ULL;

        offsets_end_offset = shift;

        for (uint64_t  module_id = 1ULL; module_id < data.size(); ++module_id)
            if (!data.at(module_id).second.empty() && !data.at(module_id).first)
            {
                offsets.at(2ULL * module_id) = shift;
                offsets.at(2ULL * module_id + 1ULL) = 0ULL;
                shift += data.at(module_id).second.size();
            }

        content_end_offset = shift;
    }

    for (uint64_t  module_id = 1ULL; module_id < data.size(); ++module_id)
        if (!data.at(module_id).second.empty() && data.at(module_id).first)
        {
            INVARIANT(offsets.at(2ULL * module_id) == content.size());
            std::copy(data.at(module_id).second.data(),
                      data.at(module_id).second.data() + data.at(module_id).second.size(),
                      std::back_inserter(content)
                      );
        }

    INVARIANT(tcb_begin == content.size());

    offset = tcb_begin;

    for (uint64_t  off = tcb_begin; off != tcb_end; ++off)
        content.push_back(0U);
    write_to_buffer(to_little_endian(tcb_begin),content,tcb_begin + tcb::get_offset_tcb());
    write_to_buffer(to_little_endian(tcb_end),content,tcb_begin + tcb::get_offset_dvt());
    write_to_buffer(to_little_endian(tcb_begin),content,tcb_begin + tcb::get_offset_self());

    INVARIANT(tcb_end == content.size());

    for (uint64_t  index = 0ULL; index < offsets.size(); ++index)
    {
        uint64_t const  value = to_little_endian(offsets.at(index));
        for (uint8_t const*  b = reinterpret_cast<uint8_t const*>(&value), * const e = b + sizeof(value); b != e; ++b)
            content.push_back(*b);
    }
    for (uint64_t  i = 0ULL; i != 512ULL; ++i)
        content.push_back(0ULL);

    INVARIANT(offsets_end_offset == content.size());

    for (uint64_t  module_id = 1ULL; module_id < data.size(); ++module_id)
        if (!data.at(module_id).second.empty() && !data.at(module_id).first)
        {
            INVARIANT(offsets.at(2ULL * module_id) == content.size());
            std::copy(data.at(module_id).second.data(),
                      data.at(module_id).second.data() + data.at(module_id).second.size(),
                      std::back_inserter(content)
                      );
        }

    INVARIANT(content_end_offset == content.size());

    relocations.push_back(tcb_begin + tcb::get_offset_tcb());
    relocations.push_back(tcb_begin + tcb::get_offset_dvt());
    relocations.push_back(tcb_begin + tcb::get_offset_self());
    for (uint64_t  module_id = 1ULL; module_id < data.size(); ++module_id)
        if (!data.at(module_id).second.empty())
        {
            relocations.push_back(tcb_end + module_id * 2ULL * sizeof(uint64_t));
            INVARIANT(relocations.back() >= tcb_end && relocations.back() < offsets_end_offset);
        }
}


}}}}
