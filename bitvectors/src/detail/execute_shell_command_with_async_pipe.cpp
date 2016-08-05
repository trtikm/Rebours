#include <rebours/bitvectors/detail/execute_shell_command_with_async_pipe.hpp>
#include <memory>
#include <cstdint>
#   if defined(WIN32)
#       error "NOT IMPLEMENTED YET!"
#   elif defined(__linux__) || defined(__APPLE__)
#       include <cstdio>
#       include <signal.h>
#       include <fcntl.h>
#       include <cstdlib>
#       include <cctype>
#       include <unistd.h>
#   else
#       error "Unsuported platform."
#   endif
#include <iostream>

namespace bv { namespace detail {


#   if defined(WIN32)
#       error "NOT IMPLEMENTED YET!"
#   elif defined(__linux__) || defined(__APPLE__)

//pid_t  popen2(char* const* command, int *infp, int *outfp)
//{
//    uint32_t const READ = 0U;
//    uint32_t const WRITE = 1U;

//    int p_stdin[2], p_stdout[2];
//    pid_t pid;

//    if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0)
//        return -1;

//    pid = fork();

//    if (pid < 0)
//        return pid;
//    else if (pid == 0)
//    {
//        close(p_stdin[WRITE]);
//        dup2(p_stdin[READ], READ);
//        close(p_stdout[READ]);
//        dup2(p_stdout[WRITE], WRITE);

//        execvp(*command, command);
//        perror("execvp");
//        exit(1);
//    }

//    if (infp == NULL)
//        close(p_stdin[WRITE]);
//    else
//        *infp = p_stdin[WRITE];

//    if (outfp == NULL)
//        close(p_stdout[READ]);
//    else
//        *outfp = p_stdout[READ];

//    return pid;
//}

//bool  exec_sat_query(char* const*  shell_command, uint32_t const  timeout_milliseconds, std::stringstream&  ostr)
//{
//    std::chrono::system_clock::time_point const  start_time = std::chrono::high_resolution_clock::now();
//    int  fno;
//    ::pid_t const  process_id = popen2(shell_command,&fno,nullptr);
//    if (process_id <= 0)
//        return false;
//    if (fno <= 0)
//    {
//        ::kill(process_id,SIGKILL);
//        return false;
//    }
//    int const  flags = ::fcntl(fno, F_GETFL, 0);
//    ::fcntl(fno,F_SETFL,flags | O_NONBLOCK);
//    while (true)
//    {
//        static char  buff[512];
//        ssize_t num_read = read(fno,buff,sizeof(buff));
//        if (num_read == -1 && errno == EAGAIN)
//        {
//            // no data yet => do nothing
//        }
//        else if (num_read > 0)
//        {
//            for (int i = 0; i < num_read; ++i)
//                ostr << buff[i];
//        }
//        else
//        {
//            std::cout << "Waiting 5 seconds till to kill of the process: " << process_id  << "\n";
//            ::sleep(5);
//            std::cout << "Killing the process: " << process_id  << "\n";
//            if (::kill(process_id,SIGKILL))
//            {
//                std::cout << "Killing SECCESSFULL.\n";
//                int iii = 0;
//            }
//            else
//                std::cout << "Killing FAILED.\n";
//            ::close(fno);
//            return true;
//        }

//        std::chrono::system_clock::time_point const  current_time = std::chrono::high_resolution_clock::now();
//        double const duration = std::chrono::duration<double, std::milli>(current_time - start_time).count();
//        if (duration >= double(timeout_milliseconds))
//        {
//            ::kill(process_id,SIGKILL);
//            ::close(fno);
//            return false;
//        }
//    }
//}


//bool  execute_shell_command_with_async_pipe(std::string const& shell_command, uint32_t const  timeout_in_milliseconds,
//                                            std::function<bool()> const&  interrupt, std::stringstream&  ostr)
//{
//    std::chrono::system_clock::time_point const  start_time = std::chrono::high_resolution_clock::now();
//    std::stringstream  process_id_ostr;
//    ::pid_t  process_id = -1;
//    std::FILE* const  pipe = ::popen((std::string("echo PID $$ && ") + shell_command).c_str(), "r");
//    if (pipe == nullptr)
//        return false;
//    int const  fno = ::fileno(pipe);
//    int const  flags = ::fcntl(fno, F_GETFL, 0);
//    ::fcntl(fno,F_SETFL,flags | O_NONBLOCK);
//    while (true)
//    {
//        static char  buff[512];
//        ssize_t num_read = read(fno,buff,sizeof(buff));
//        if (num_read == -1 && errno == EAGAIN)
//        {
//            // no data yet => do nothing
//        }
//        else if (num_read > 0)
//        {
//            int i = 0;
//            if (process_id == -1)
//            {
//                for ( ; i < num_read && buff[i] != '\n'; ++i)
//                    if (std::isdigit(buff[i]))
//                        process_id_ostr << buff[i];
//                process_id = std::atoi(process_id_ostr.str().c_str());
//                if (process_id <= 0)
//                    throw std::runtime_error("[sat_engine_z3] exec_sat_query : cannot parse id of the executed process.");
//                ++i;
//            }
//            for ( ; i < num_read; ++i)
//                ostr << buff[i];
//        }
//        else
//        {
//            ::pclose(pipe);
//            return true;
//        }

//        std::chrono::system_clock::time_point const  current_time = std::chrono::high_resolution_clock::now();
//        double const duration = std::chrono::duration<double, std::milli>(current_time - start_time).count();
//        if (duration >= double(timeout_in_milliseconds))
//        {
//            ::kill(process_id,SIGKILL);
//            ::pclose(pipe);
//            if (process_id <= 0)
//                throw std::runtime_error("[sat_engine_z3] exec_sat_query : cannot kill the executed process.");
//            return false;
//        }
//    }
//}

bool  execute_shell_command_with_async_pipe(std::string const& shell_command, std::function<bool()> const&  interrupt,
                                            std::stringstream&  ostr)
{
    std::stringstream  process_id_ostr;
    ::pid_t  process_id = -1;
    std::FILE* const  pipe = ::popen((std::string("echo PID $$ && ") + shell_command).c_str(), "r");
    if (pipe == nullptr)
        return false;
    int const  fno = ::fileno(pipe);
    int const  flags = ::fcntl(fno, F_GETFL, 0);
    ::fcntl(fno,F_SETFL,flags | O_NONBLOCK);
    while (true)
    {
        static char  buff[512];
        ssize_t num_read = read(fno,buff,sizeof(buff));
        if (num_read == -1 && errno == EAGAIN)
        {
            // no data yet => do nothing
        }
        else if (num_read > 0)
        {
            int i = 0;
            if (process_id == -1)
            {
                for ( ; i < num_read && buff[i] != '\n'; ++i)
                    if (std::isdigit(buff[i]))
                        process_id_ostr << buff[i];
                process_id = std::atoi(process_id_ostr.str().c_str());
                if (process_id <= 0)
                    throw std::runtime_error("execute_shell_command_with_async_pipe : cannot parse id of the executed process.");
                ++i;
            }
            for ( ; i < num_read; ++i)
                ostr << buff[i];
        }
        else
        {
            ::pclose(pipe);
            return true;
        }

        if (interrupt())
        {
            if (process_id > 0)
            {
                ::kill(process_id,SIGKILL);
                ::pclose(pipe);
            }
            return false;
        }
    }
}

#   else
#       error "Unsuported platform."
#   endif


}}
