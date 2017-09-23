#ifndef UTILITY_TEST_HPP_INCLUDED
#   define UTILITY_TETS_HPP_INCLUDED

#   include <rebours/utility/fail_message.hpp>
#   include <rebours/utility/timeprof.hpp>
#   include <rebours/utility/timestamp.hpp>
#   include <rebours/utility/macro_to_string.hpp>
#   include <string>
#   include <iostream>
#   include <fstream>

#   define TEST_PROGRESS_SHOW() ::private_test_internal_implementation_details::print_next_test_progress_character()
#   define TEST_PROGRESS_HIDE() ::private_test_internal_implementation_details::hide_test_progress_character()
#   define TEST_PROGRESS_UPDATE() { TEST_PROGRESS_HIDE(); TEST_PROGRESS_SHOW(); }

#   define TEST_MESSAGE_TO_STREAM(MSG,OSTR) \
    do {\
        std::stringstream  sstr;\
        sstr << MSG;\
        ::private_test_internal_implementation_details::insert_message(sstr.str());\
        if (&(OSTR) == &std::cout)\
            TEST_PROGRESS_HIDE();\
        OSTR << sstr.str() << std::endl;\
        if (&(OSTR) == &std::cout)\
            TEST_PROGRESS_SHOW();\
    } while(false)

#   define TEST_MESSAGE(MSG) TEST_MESSAGE_TO_STREAM(MSG,std::cout)

#   define TEST_EVALUATION(COND,RESULT,LOGSUCCESS) \
    do {\
        try\
        {\
            if ((COND) == (RESULT))\
            {\
                ::private_test_internal_implementation_details::increment_num_tests_which_succeeded_without_exception();\
                if (LOGSUCCESS) { TEST_MESSAGE( FAIL_MSG("TEST SUCCEEDED : " STRINGIFY(COND)) ); }\
            }\
            else\
            {\
                ::private_test_internal_implementation_details::increment_num_tests_which_failed_without_exception();\
                TEST_MESSAGE( FAIL_MSG("TEST FAILED : " STRINGIFY(COND)) );\
            }\
        }\
        catch (...)\
        {\
            if (RESULT)\
            {\
                ::private_test_internal_implementation_details::increment_num_tests_which_failed_by_exception();\
                TEST_MESSAGE( FAIL_MSG("TEST FAILED (by throwing exception) : " STRINGIFY(COND)) );\
            }\
            else\
            {\
                ::private_test_internal_implementation_details::increment_num_tests_which_succeeded_by_exception();\
                if (LOGSUCCESS) { TEST_MESSAGE( FAIL_MSG("TEST SUCCEEDED (by throwing exception) : " STRINGIFY(COND)) ); }\
            }\
        }\
        TEST_PROGRESS_UPDATE();\
    } while(false)

#   define TEST_SUCCESS(COND) TEST_EVALUATION(COND,true,false)
#   define TEST_FAILURE(COND) TEST_EVALUATION(COND,false,false)

#   define TEST_SUCCESS_AND_PRINT(COND) TEST_EVALUATION(COND,true,true)
#   define TEST_FAILURE_AND_PRINT(COND) TEST_EVALUATION(COND,false,true)

#   define TEST_PRINT_STATISTICS() test_statistics::print_test_statistical_data_to_log_and_standard_output(std::cout,true)
#   define TEST_PRINT_STATISTICS_TO_STREAM(S) test_statistics::print_test_statistical_data_to_log_and_standard_output(S,true)

#   define TEST_PRINT_STATISTICS_SUMMARY() test_statistics::print_test_statistical_data_to_log_and_standard_output(std::cout,false)
#   define TEST_PRINT_STATISTICS_SUMMARY_TO_STREAM(S) test_statistics::print_test_statistical_data_to_log_and_standard_output(S,false)

#   define TEST_CLEAR_STATISTIC() test_statistics::clear()

#   define TEST_DEFINE_MAIN_FUNCTION_CALLING(UNIQUE_TEST_FN_NAME) \
    int main(int argc, char* argv[])\
    {\
        (void)argc; (void)argv;\
        struct local\
        {\
            static void save_crash_report(std::string const& crash_message, std::string const&  test_unique_name)\
            {\
                std::cout << "ERROR: " << crash_message << "\n";\
                std::ofstream  ofile( test_unique_name + "_CRASH.txt", std::ios_base::app );\
                ofile << crash_message << "\n";\
            }\
        };\
        try\
        {\
            TEST_PROGRESS_SHOW();\
            {\
                TMPROF_BLOCK();\
                (UNIQUE_TEST_FN_NAME)();\
            }\
            TEST_PROGRESS_HIDE();\
            TEST_PRINT_STATISTICS_SUMMARY();\
            {\
                std::string const  raw_filename(std::string(STRINGIFY(UNIQUE_TEST_FN_NAME)) + ".TESTLOG.txt");\
                std::ofstream  ofile( extend_file_path_name_by_timestamp(raw_filename) );\
                TEST_PRINT_STATISTICS_TO_STREAM(ofile);\
            }\
            TMPROF_PRINT_TO_FILE(std::string(STRINGIFY(UNIQUE_TEST_FN_NAME)) + ".TMPROF.html",true);\
        }\
        catch(std::exception const& e)\
        {\
            TEST_PROGRESS_HIDE();\
            try { local::save_crash_report(e.what(),STRINGIFY(UNIQUE_TEST_FN_NAME)); } catch (...) {}\
            return -1;\
        }\
        catch(...)\
        {\
            TEST_PROGRESS_HIDE();\
            try { local::save_crash_report("Unknown exception was thrown.",STRINGIFY(UNIQUE_TEST_FN_NAME)); }\
            catch (...) {}\
            return -2;\
        }\
        return 0;\
    }


namespace private_test_internal_implementation_details {
// PRIVATE STUFF IS IN THIS NAMESPACE! DO NOT USE ITS CONTENT DIRECTLY!
uint32_t  num_tests_which_succeeded_without_exception();
uint32_t  num_tests_which_succeeded_by_exception();
uint32_t  num_tests_which_failed_without_exception();
uint32_t  num_tests_which_failed_by_exception();
void  increment_num_tests_which_succeeded_without_exception();
void  increment_num_tests_which_succeeded_by_exception();
void  increment_num_tests_which_failed_without_exception();
void  increment_num_tests_which_failed_by_exception();
void  print_next_test_progress_character();
void  hide_test_progress_character();
void  insert_message(std::string const&  msg);
std::vector<std::string>  get_messages();
void  clear();
}

namespace test_statistics {


inline uint32_t  num_tests_which_succeeded_without_exception()
{ return private_test_internal_implementation_details::num_tests_which_succeeded_without_exception(); }

inline uint32_t  num_tests_which_succeeded_by_exception()
{ return private_test_internal_implementation_details::num_tests_which_succeeded_by_exception(); }

inline uint32_t  num_tests_which_failed_without_exception()
{ return private_test_internal_implementation_details::num_tests_which_failed_without_exception(); }

inline uint32_t  num_tests_which_failed_by_exception()
{ return private_test_internal_implementation_details::num_tests_which_failed_by_exception(); }

inline std::vector<std::string>  get_messages()
{ return private_test_internal_implementation_details::get_messages(); }

void  print_test_statistical_data_to_log_and_standard_output(std::ostream& ostr, bool const  dump_messages = true);

inline void  clear() { private_test_internal_implementation_details::clear(); }


}

#endif
