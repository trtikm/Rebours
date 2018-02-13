#include <rebours/MAL/recogniser/detail/recognise_x86_64_Linux_syscall_syscall.hpp>
#include <rebours/MAL/recogniser/detail/register_info.hpp>
#include <ctime>
#include <cstddef>
#include <cstdint>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <unistd.h>

namespace osabi {


using  dev_t = uint64_t;          //!< ASSERT: sizeof(dev_t)==8
using  ino_t = uint64_t;          //!< ASSERT: sizeof(ino_t)==8
using  mode_t = uint32_t;         //!< ASSERT: sizeof(mode_t)==4
using  nlink_t = uint64_t;        //!< ASSERT: sizeof(nlink_t)==8
using  uid_t = uint32_t;          //!< ASSERT: sizeof(uid_t)==4
using  gid_t = uint32_t;          //!< ASSERT: sizeof(gid_t)==4
using  off_t = int64_t;           //!< ASSERT: sizeof(off_t)==8
using  blksize_t = int64_t;       //!< ASSERT: sizeof(blksize_t)==8
using  blkcnt_t = uint64_t;
using  syscall_slong_t = int64_t; //!< ASSERT: sizeof(syscall_slong_t)==8
using  time_t = int64_t;          //!< ASSERT: sizeof(time_t)==8

static_assert(sizeof(dev_t) == 8ULL,"It must match the size of the corresponding type used on Linux.");
static_assert(sizeof(ino_t) == 8ULL,"It must match the size of the corresponding type used on Linux.");
static_assert(sizeof(mode_t) == 4ULL,"It must match the size of the corresponding type used on Linux.");
static_assert(sizeof(nlink_t) == 8ULL,"It must match the size of the corresponding type used on Linux.");
static_assert(sizeof(uid_t) == 4ULL,"It must match the size of the corresponding type used on Linux.");
static_assert(sizeof(gid_t) == 4ULL,"It must match the size of the corresponding type used on Linux.");
static_assert(sizeof(off_t) == 8ULL,"It must match the size of the corresponding type used on Linux.");
static_assert(sizeof(blksize_t) == 8ULL,"It must match the size of the corresponding type used on Linux.");
static_assert(sizeof(syscall_slong_t) == 8ULL,"It must match the size of the corresponding type used on Linux.");
static_assert(sizeof(time_t) == 8ULL,"It must match the size of the corresponding type used on Linux.");

struct timespec     /* ASSERT: sizeof(struct stat)==0x10 */
{
    time_t tv_sec;		/* Seconds.  */
    syscall_slong_t tv_nsec;	/* Nanoseconds.  */
};

static_assert(sizeof(timespec) == 16ULL,"It must match the size of the corresponding type used on Linux.");

struct stat           /* ASSERT: sizeof(struct stat)==0x90 */
{
    dev_t st_dev;       /* Device.  */
    ino_t st_ino;       /* File serial number.	*/
    nlink_t st_nlink;   /* Link count.  */
    mode_t st_mode;     /* File mode.  */
    uid_t st_uid;       /* User ID of the file's owner.	*/
    gid_t st_gid;       /* Group ID of the file's group.*/
    int __pad0;           /* 8-bytes padding */
    dev_t st_rdev;      /* Device number, if device.  */
    off_t st_size;      /* Size of file, in bytes.  */
    blksize_t st_blksize;   /* Optimal block size for I/O.  */
    blkcnt_t st_blocks;     /* Number 512-byte blocks allocated. */
    struct timespec st_atim;      /* Time of last access.  */
    struct timespec st_mtim;      /* Time of last modification.  */
    struct timespec st_ctim;      /* Time of last status change.  */
    syscall_slong_t __glibc_reserved[3];
};

static_assert(sizeof(struct stat) == 0x90ULL,"It must match the size of the corresponding native structure used on Linux.");


uint64_t const stdin_fileno = 0ULL;     // ~ STDIN_FILENO   0   Standard input.
uint64_t const stdout_fileno = 1ULL;    // ~ STDOUT_FILENO  1   Standard output.
uint64_t const stderr_fileno = 2ULL;    // ~ STDERR_FILENO  2   Standard error output.


}

namespace mal { namespace recogniser { namespace detail { namespace x86_64_Linux { namespace syscall {


using  node_id = microcode::program_component::node_id;


ret_type  recognise_syscall_1_write(reg_fn_type const&  reg_fn, mem_fn_type const& , descriptor::storage const&  D,
                                    uint8_t const  num_bytes_of_instruction, microcode::program_component&  C)
{
    // ssize_t write(int fd, const void *buf, size_t count);
    // rdi ~ fd
    // rsi ~ buf
    // rdx ~ count
    // It writes up to 'count' bytes from the buffer pointed 'buf' to the file referred to by the file descriptor 'fd'.
    // On success, the number of bytes written is returned (zero indicates nothing was written).  It is not an error if
    // this number is smaller than the number of bytes requested; this may happen for example because the disk device
    // was filled. On error, -1 is returned, and errno is set appropriately.
    // For details see: http://man7.org/linux/man-pages/man2/write.2.html

    register_info const  rax_info = get_register_info("rax",D);
    register_info const  rsi_info = get_register_info("rsi",D);
    register_info const  rdx_info = get_register_info("rdx",D);

    uint64_t  stream_id;
    {
        ret_type  ret_state;
        uint64_t const  rdi = read_register<uint64_t>("rdi",D,reg_fn,ret_state);
        if (error_code(ret_state) != 0U)
            return ret_state;
        if (rdi == osabi::stdin_fileno) // Is that STDIN ?
            stream_id = 1ULL;
        else if (rdi == osabi::stdout_fileno) // Is that STDOUT ?
            stream_id = 2ULL;
        else if (rdi == osabi::stderr_fileno) // Is that STDERR ?
            stream_id = 3ULL;
        else
            stream_id = rdi;
    }

    ASSUMPTION(rsi_info.second == 8ULL && rdx_info.second == 8ULL);

    uint64_t  buf_ptr = D.start_address_of_temporaries();
    uint64_t  buf_end = buf_ptr + 8ULL;
    uint64_t  temp = buf_end + 8ULL;

    node_id  loop_head =
    C.insert_sequence(C.entry(),{
            microcode::create_SETANDCOPY__REG_ASGN_REG(
                    8ULL,
                    buf_ptr,
                    rsi_info.first
                    ),
            microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                    8ULL,
                    buf_end,
                    buf_ptr,
                    rdx_info.first
                    ),
            microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                    8ULL,
                    rax_info.first,
                    0ULL
                    ),
            });

    node_id  u =
    C.insert_sequence(loop_head,{
           microcode::create_BITOPERATIONS__REG_ASGN_REG_XOR_REG(
                    8ULL,
                    temp,
                    buf_ptr,
                    buf_end
                    )
            });

    node_id  end;
    std::tie(u,end) =
            C.insert_branching(microcode::GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO,8U,temp,u);

    u = C.insert_sequence(u,{
            microcode::create_DATATRANSFER__REG_ASGN_DEREF_REG(
                    1U,
                    temp,
                    buf_ptr
                    ),
            microcode::create_INPUTOUTPUT__REG_ASGN_STREAM_WRITE_NUMBER_REG(
                    temp,
                    stream_id,
                    temp
                    )
            });

    std::tie(u,end) =
            C.insert_branching(microcode::GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO,1U,temp,u,{0ULL,end});

    C.insert_sequence(u,{
            microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                    8ULL,
                    buf_ptr,
                    buf_ptr,
                    1ULL
                    ),
            microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                    8ULL,
                    rax_info.first,
                    rax_info.first,
                    1ULL
                    ),
            },loop_head);

    C.insert_sequence(end,{
            microcode::create_HAVOC__REG_ASGN_HAVOC(
                    temp + 8ULL - D.start_address_of_temporaries(),
                    D.start_address_of_temporaries()
                    ),
            microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                    8U,
                    0ULL,
                    0ULL,
                    num_bytes_of_instruction
                    )
            });

    return make_succes();
}

ret_type  recognise_syscall_5_fstat(reg_fn_type const&  reg_fn, mem_fn_type const& , descriptor::storage const&  D,
                                    uint8_t const  num_bytes_of_instruction, microcode::program_component&  C)
{
    // int fstat(int fd, struct stat *buf);
    // rdi ~ fd
    // rsi ~ buf
    // It fills in the 'stat' structure pointed to by 'buf' by data related to the file descriptor 'fd'.
    // On success, zero is returned. On error, -1 is returned, and errno is set appropriately.

    ret_type  ret_state;
    uint64_t const  rdi = read_register<uint64_t>("rdi",D,reg_fn,ret_state);
    if (error_code(ret_state) != 0U)
        return ret_state;

    if (rdi == osabi::stdout_fileno) // Is that STDOUT ?
    {
        register_info const  rsi_info = get_register_info("rsi",D);

        C.insert_sequence(C.entry(),{
               microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                       rsi_info.second,
                       D.start_address_of_temporaries(),
                       rsi_info.first,
                       offsetof(osabi::stat,st_dev)
                       ),
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(
                        sizeof(osabi::stat::st_dev),
                        D.start_address_of_temporaries(),
                        0xcULL
                        ),

                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        rsi_info.second,
                        D.start_address_of_temporaries(),
                        rsi_info.first,
                        offsetof(osabi::stat,st_ino)
                        ),
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(
                        sizeof(osabi::stat::st_ino),
                        D.start_address_of_temporaries(),
                        0x14ULL
                        ),

                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        rsi_info.second,
                        D.start_address_of_temporaries(),
                        rsi_info.first,
                        offsetof(osabi::stat,st_nlink)
                        ),
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(
                        sizeof(osabi::stat::st_nlink),
                        D.start_address_of_temporaries(),
                        0x1ULL
                        ),

                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        rsi_info.second,
                        D.start_address_of_temporaries(),
                        rsi_info.first,
                        offsetof(osabi::stat,st_mode)
                        ),
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(
                        sizeof(osabi::stat::st_mode),
                        D.start_address_of_temporaries(),
                        0x2190ULL
                        ),

                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        rsi_info.second,
                        D.start_address_of_temporaries(),
                        rsi_info.first,
                        offsetof(osabi::stat,st_uid)
                        ),
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(
                        sizeof(osabi::stat::st_uid),
                        D.start_address_of_temporaries(),
                        0x3e8ULL
                        ),

                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        rsi_info.second,
                        D.start_address_of_temporaries(),
                        rsi_info.first,
                        offsetof(osabi::stat,st_gid)
                        ),
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(
                        sizeof(osabi::stat::st_gid),
                        D.start_address_of_temporaries(),
                        0x5ULL
                        ),

                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        rsi_info.second,
                        D.start_address_of_temporaries(),
                        rsi_info.first,
                        offsetof(osabi::stat,__pad0)
                        ),
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(
                        sizeof(osabi::stat::__pad0),
                        D.start_address_of_temporaries(),
                        0x0ULL
                        ),

                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        rsi_info.second,
                        D.start_address_of_temporaries(),
                        rsi_info.first,
                        offsetof(osabi::stat,st_rdev)
                        ),
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(
                        sizeof(osabi::stat::st_rdev),
                        D.start_address_of_temporaries(),
                        0x8811ULL
                        ),

                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        rsi_info.second,
                        D.start_address_of_temporaries(),
                        rsi_info.first,
                        offsetof(osabi::stat,st_size)
                        ),
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(
                        sizeof(osabi::stat::st_size),
                        D.start_address_of_temporaries(),
                        0x0ULL
                        ),

                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        rsi_info.second,
                        D.start_address_of_temporaries(),
                        rsi_info.first,
                        offsetof(osabi::stat,st_blksize)
                        ),
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(
                        sizeof(osabi::stat::st_blksize),
                        D.start_address_of_temporaries(),
                        0x400ULL
                        ),

                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        rsi_info.second,
                        D.start_address_of_temporaries(),
                        rsi_info.first,
                        offsetof(osabi::stat,st_blocks)
                        ),
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(
                        sizeof(osabi::stat::st_blocks),
                        D.start_address_of_temporaries(),
                        0x0ULL
                        ),

                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        rsi_info.second,
                        D.start_address_of_temporaries(),
                        rsi_info.first,
                        offsetof(osabi::stat,st_atim) + offsetof(osabi::timespec,tv_sec)
                        ),
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(
                        sizeof(osabi::timespec::tv_sec),
                        D.start_address_of_temporaries(),
                        0x576d4b19ULL
                        ),

                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        rsi_info.second,
                        D.start_address_of_temporaries(),
                        rsi_info.first,
                        offsetof(osabi::stat,st_atim) + offsetof(osabi::timespec,tv_nsec)
                        ),
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(
                        sizeof(osabi::timespec::tv_nsec),
                        D.start_address_of_temporaries(),
                        0x524d88ULL
                        ),

                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        rsi_info.second,
                        D.start_address_of_temporaries(),
                        rsi_info.first,
                        offsetof(osabi::stat,st_mtim) + offsetof(osabi::timespec,tv_sec)
                        ),
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(
                        sizeof(osabi::timespec::tv_sec),
                        D.start_address_of_temporaries(),
                        0x576d4b19ULL
                        ),

                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        rsi_info.second,
                        D.start_address_of_temporaries(),
                        rsi_info.first,
                        offsetof(osabi::stat,st_mtim) + offsetof(osabi::timespec,tv_nsec)
                        ),
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(
                        sizeof(osabi::timespec::tv_nsec),
                        D.start_address_of_temporaries(),
                        0x524d88ULL
                        ),

                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        rsi_info.second,
                        D.start_address_of_temporaries(),
                        rsi_info.first,
                        offsetof(osabi::stat,st_ctim) + offsetof(osabi::timespec,tv_sec)
                        ),
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(
                        sizeof(osabi::timespec::tv_sec),
                        D.start_address_of_temporaries(),
                        0x576d4781ULL
                        ),

                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        rsi_info.second,
                        D.start_address_of_temporaries(),
                        rsi_info.first,
                        offsetof(osabi::stat,st_ctim) + offsetof(osabi::timespec,tv_nsec)
                        ),
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(
                        sizeof(osabi::timespec::tv_nsec),
                        D.start_address_of_temporaries(),
                        0x524d88ULL
                        ),

                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        rsi_info.second,
                        D.start_address_of_temporaries(),
                        rsi_info.first,
                        offsetof(osabi::stat,__glibc_reserved[0])
                        ),
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(
                        sizeof(osabi::stat::__glibc_reserved[0]),
                        D.start_address_of_temporaries(),
                        0x0ULL
                        ),

                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        rsi_info.second,
                        D.start_address_of_temporaries(),
                        rsi_info.first,
                        offsetof(osabi::stat,__glibc_reserved[1])
                        ),
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(
                        sizeof(osabi::stat::__glibc_reserved[1]),
                        D.start_address_of_temporaries(),
                        0x0ULL
                        ),

                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        rsi_info.second,
                        D.start_address_of_temporaries(),
                        rsi_info.first,
                        offsetof(osabi::stat,__glibc_reserved[2])
                        ),
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(
                        sizeof(osabi::stat::__glibc_reserved[2]),
                        D.start_address_of_temporaries(),
                        0x0ULL
                        ),

               microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        get_register_info("rax",D).second,
                        get_register_info("rax",D).first,
                        0x0ULL
                        ),

               microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        get_register_info("rf",D).second,
                        get_register_info("rf",D).first,
                        0x0ULL
                        ),
               microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        get_register_info("vm",D).second,
                        get_register_info("vm",D).first,
                        0x0ULL
                        ),

               microcode::create_HAVOC__REG_ASGN_HAVOC(
                        get_register_info("rcx",D).second,
                        get_register_info("rcx",D).first
                        ),
               microcode::create_HAVOC__REG_ASGN_HAVOC(
                        get_register_info("r11",D).second,
                        get_register_info("r11",D).first
                        ),

               microcode::create_HAVOC__REG_ASGN_HAVOC(
                        rsi_info.second,
                        D.start_address_of_temporaries()
                        ),

                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        num_bytes_of_instruction
                        )
                });

        return make_succes();
    }

    return make_failure();
}

ret_type  recognise_syscall_9_mmap(reg_fn_type const&  reg_fn, mem_fn_type const&  , descriptor::storage const&  D,
                                   uint8_t const  num_bytes_of_instruction, microcode::program_component&  C)
{
    // void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
    // rdi ~ addr (= 0)
    // rsi ~ length (= 0x1000)
    // rdx ~ prot (= 3)
    //            #define PROT_READ	0x1     /* Page can be read.  */
    //            #define PROT_WRITE	0x2     /* Page can be written.  */
    //            #define PROT_EXEC	0x4     /* Page can be executed.  */
    //            #define PROT_NONE	0x0     /* Page can not be accessed.  */
    // r10 ~ flags (= 0x22)
    //            #define MAP_SHARED	0x01    /* Share changes.  */
    //            #define MAP_PRIVATE	0x02    /* Changes are private.  */
    //            #define MAP_FIXED	0x10    /* Interpret addr exactly.  */
    //            #define MAP_FILE	0
    //            #define MAP_ANONYMOUS	0x20    /* Don't use a file.  */
    //            #define MAP_ANON	MAP_ANONYMOUS
    //            #define MAP_DENYWRITE	0x0800  /* ETXTBSY */
    //            #define MAP_FOOBAR	0x0800  /* ETXTBSY */
    // r8  ~ fd (= 0xffffffff)
    // r9  ~ offset (= 0)
    // On success, mmap() returns a pointer to the mapped area.  On error,
    // the value MAP_FAILED (that is, (void *) -1) is returned, and errno is
    // set to indicate the cause of the error.

    ret_type  ret_state;
    uint64_t const  r10 = read_register<uint64_t>("r10",D,reg_fn,ret_state);
    if (error_code(ret_state) != 0U)
        return ret_state;

    if (r10 == 0x22ULL) // flags == MAP_ANONYMOUS | MAP_PRIVATE
    {
        node_id  u = C.insert_sequence(C.entry(),{
                microcode::create_MEMORYMANAGEMENT__REG_ASGN_MEM_ALLOC(
                        get_register_info("rax",D).first,
                        get_register_info("dl",D).first,
                        get_register_info("rsi",D).first,
                        get_register_info("rdi",D).first
                        ),
                });

        node_id  v;
        std::tie(u,v) = C.insert_branching(microcode::GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO,get_register_info("rax",D).second,get_register_info("rax",D).first,u);

        C.insert_sequence(v,{
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        get_register_info("rax",D).second,
                        get_register_info("rax",D).first,
                        get_register_info("rax",D).first
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        num_bytes_of_instruction
                        )
                });

        uint64_t const  tmp_end = D.start_address_of_temporaries();
        uint64_t const  tmp_ptr = tmp_end + 8ULL;
        uint64_t const  tmp_xor = tmp_ptr + 8ULL;

        node_id const  loop_head = C.insert_sequence(u,{
              microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                      8U,
                      tmp_end,
                      get_register_info("rax",D).first,
                      get_register_info("rsi",D).first
                      ),
              microcode::create_SETANDCOPY__REG_ASGN_REG(
                      8U,
                      tmp_ptr,
                      get_register_info("rax",D).first
                      ),
              });
        u = C.insert_sequence(loop_head,{
              microcode::create_BITOPERATIONS__REG_ASGN_REG_XOR_REG(
                      8U,
                      tmp_xor,
                      tmp_ptr,
                      tmp_end
                      ),
              });

        std::tie(u,v) = C.insert_branching(microcode::GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO,8U,tmp_xor,u);

        C.insert_sequence(u,{
                microcode::create_DATATRANSFER__DEREF_REG_ASGN_NUMBER(
                        1U,
                        tmp_ptr,
                        0ULL
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        tmp_ptr,
                        tmp_ptr,
                        1ULL
                        )
               },loop_head);

        C.insert_sequence(v,{
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        tmp_xor + 8ULL - D.start_address_of_temporaries(),
                        D.start_address_of_temporaries()
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        num_bytes_of_instruction
                        )
               });

       return make_succes();
    }

    return make_failure();
}

ret_type  recognise_syscall_231_exit_group(reg_fn_type const&  , mem_fn_type const&  , descriptor::storage const&  ,
                                           uint8_t const  , microcode::program_component&  C)
{
    // void exit_group(int status);
    // rdi ~ status
    // This system call is equivalent to exit(2) except that it terminates not only the calling thread, but all threads in
    // the calling process's thread group.
    // This system call does not return.

    C.insert_sequence(C.entry(),{
            microcode::create_MISCELLANEOUS__STOP()
            });

    return make_succes();
}


}}}}}
