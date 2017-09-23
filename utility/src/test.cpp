#include <rebours/utility/test.hpp>
#include <array>
#include <vector>
#include <algorithm>
#include <mutex>
#include <atomic>

namespace test_statistics {


void  print_test_statistical_data_to_log_and_standard_output(std::ostream& ostr, bool const  dump_messages)
{
    if (dump_messages)
        for (auto const& msg : get_messages())
            ostr << msg << std::endl << std::endl;

    uint32_t const  total_num_tests =
            num_tests_which_succeeded_without_exception() +
            num_tests_which_succeeded_by_exception() +
            num_tests_which_failed_without_exception() +
            num_tests_which_failed_by_exception()
            ;
    uint32_t const  total_num_succeeded_tests =
            num_tests_which_succeeded_without_exception() +
            num_tests_which_succeeded_by_exception()
            ;
    uint32_t const  total_num_failed_tests =
            num_tests_which_failed_without_exception() +
            num_tests_which_failed_by_exception()
            ;

    ostr << "RESULTS FROM TESTING:\n";
    ostr << "Total number of tests: " << total_num_tests << "\n";
    ostr << "Total number of successfull tests: " << total_num_succeeded_tests << "\n";
    ostr << "Total number of failed tests: " << total_num_failed_tests << "\n";

    if (total_num_failed_tests == 0)
        ostr << "The testing was SUCCESSFUL.\n";
    else
        ostr << "The testing has FAILED.\n";
}


}


namespace private_test_internal_implementation_details {

namespace {

std::mutex  g_print_progress_mutex;

}


void  print_next_test_progress_character()
{
    std::lock_guard<std::mutex> lock(g_print_progress_mutex);
    static std::array<uint8_t,4> char_set = { '|', '/', '-', '\\' };
    std::rotate(char_set.begin(),char_set.begin()+1,char_set.end());
    std::cout << char_set.at(0);
    std::cout.flush();
}

void  hide_test_progress_character()
{
    std::lock_guard<std::mutex> lock(g_print_progress_mutex);
    std::cout << '\b';
    std::cout.flush();
}


namespace {

std::atomic<uint32_t>  g_num_tests_which_succeeded_without_exception = 0U;
std::atomic<uint32_t>  g_num_tests_which_succeeded_by_exception = 0U;
std::atomic<uint32_t>  g_num_tests_which_failed_without_exception = 0U;
std::atomic<uint32_t>  g_num_tests_which_failed_by_exception = 0U;

}


uint32_t  num_tests_which_succeeded_without_exception()
{
    return g_num_tests_which_succeeded_without_exception;
}

uint32_t  num_tests_which_succeeded_by_exception()
{
    return g_num_tests_which_succeeded_by_exception;
}

uint32_t  num_tests_which_failed_without_exception()
{
    return g_num_tests_which_failed_without_exception;
}

uint32_t  num_tests_which_failed_by_exception()
{
    return g_num_tests_which_failed_by_exception;
}

void  increment_num_tests_which_succeeded_without_exception()
{
    ++g_num_tests_which_succeeded_without_exception;
}

void  increment_num_tests_which_succeeded_by_exception()
{
    ++g_num_tests_which_succeeded_by_exception;
}

void  increment_num_tests_which_failed_without_exception()
{
    ++g_num_tests_which_failed_without_exception;
}

void  increment_num_tests_which_failed_by_exception()
{
    ++g_num_tests_which_failed_by_exception;
}

void clear_couters_of_succeeded_tests()
{
    g_num_tests_which_succeeded_without_exception = 0U;
    g_num_tests_which_succeeded_by_exception = 0U;
}

void clear_couters_of_failed_tests()
{
    g_num_tests_which_failed_without_exception = 0U;
    g_num_tests_which_failed_by_exception = 0U;
}


namespace {

std::mutex  g_messages_mutex;
std::vector<std::string>  g_messages;

}


void  insert_message(std::string const&  msg)
{
    std::lock_guard<std::mutex> lock(g_messages_mutex);
    g_messages.push_back(msg);
}

std::vector<std::string>  get_messages()
{
    std::lock_guard<std::mutex> lock(g_messages_mutex);
    return g_messages;
}

void clear_messages()
{
    std::lock_guard<std::mutex> lock(g_messages_mutex);
    g_messages.clear();
}


void clear()
{
    clear_couters_of_succeeded_tests();
    clear_couters_of_failed_tests();
    clear_messages();
}


}
