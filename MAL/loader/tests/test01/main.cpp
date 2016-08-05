#include "../data_path.hpp"
#include "../test.hpp"

#include <rebours/MAL/loader/dump.hpp>
#include <rebours/MAL/loader/file_utils.hpp>
#include <rebours/MAL/loader/load.hpp>

#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

void do_test(std::string const&  file_pathname,
             std::vector<std::string> const&  ignored_libs,
             std::vector<std::string> const&  search_dirs,
             std::string const&  output_dir)
{
    std::cout << "Loading: " << file_pathname << "\n";
    std::string  error_message;
    loader::descriptor_ptr const  bfile_descriptor =
            loader::load(file_pathname,ignored_libs,search_dirs,error_message);
    if (!error_message.empty())
    {
        TEST_SUCCESS(!bfile_descriptor.operator bool());
        std::cout << "ERROR: " << error_message << std::endl;
    }
    else
    {
        TEST_SUCCESS(bfile_descriptor.operator bool());
        TEST_SUCCESS(error_message.empty());

        create_directory(output_dir);
        std::string const  dump_file =
                normalise_path(concatenate_file_paths(output_dir,parse_name_in_pathname(file_pathname) + ".html"));
        std::cout << "-- dumping results to: " << dump_file << "\n";
        std::cout.flush();
        loader::dump_html(
                bfile_descriptor,
                dump_file,
                true
                );
    }
}


static void save_crash_report(std::string const& crash_message)
{
    std::cout << "ERROR: " << crash_message << "\n";
    std::ofstream  ofile("test01_CRASH.txt", std::ios_base::app );
    ofile << crash_message << "\n";
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    try
    {
        do_test(
            concatenate_file_paths(benchmarks_path(),"ImageMagick_darwin_x86_64/libMagickWand-6.Q16.2.dylib"),
            {},
            {concatenate_file_paths(benchmarks_path(),"ImageMagick_darwin_x86_64")},
            "./test01/ImageMagick_darwin_x86_64/libMagickWand-6.Q16.2.dylib"
            );
        do_test(
            concatenate_file_paths(benchmarks_path(),"ImageMagick_darwin_x86_64/animate"),
            {},
            {concatenate_file_paths(benchmarks_path(),"ImageMagick_darwin_x86_64")},
            "./test01/ImageMagick_darwin_x86_64/animate"
            );
        do_test(
            concatenate_file_paths(benchmarks_path(),"loadlibs_macosx_X86_64/loadlibs_Darwin_Release"),
            {},
            {concatenate_file_paths(benchmarks_path(),"loadlibs_macosx_X86_64")},
            "./test01/loadlibs_Darwin_Release (X86_64)"
            );
        do_test(
            concatenate_file_paths(benchmarks_path(),"loadlibs_macosx_X86_32/loadlibs_Darwin_Release"),
            {},
            {concatenate_file_paths(benchmarks_path(),"loadlibs_macosx_X86_32")},
            "./test01/loadlibs_Darwin_Release (X86_32)"
            );
        do_test(
            concatenate_file_paths(benchmarks_path(),"loadlibs_ubuntu_X86_32/loadlibs_Linux_Release"),
            {},
            {concatenate_file_paths(benchmarks_path(),"loadlibs_ubuntu_X86_32")},
            "./test01/loadlibs_Linux_Release (X86_32)"
            );
        do_test(
            concatenate_file_paths(benchmarks_path(),"loadlibs_windows_X86_64/loadlibs_Windows_Release.exe"),
            {},
            {concatenate_file_paths(benchmarks_path(),"loadlibs_windows_X86_64")},
            "./test01/loadlibs_Windows_Release.exe (X86_64)"
            );
        do_test(
            concatenate_file_paths(benchmarks_path(),"loadlibs_windows_X86_32/loadlibs_Windows_Release.exe"),
            {},
            {concatenate_file_paths(benchmarks_path(),"loadlibs_windows_X86_32")},
            "./test01/loadlibs_Windows_Release.exe (X86_32)"
            );
        do_test(
            concatenate_file_paths(benchmarks_path(),"malwaredb.malekal.com/windows/X86_64/69ca9a1113f95f9c08c9031ab4418fbf"),
            {},
            {concatenate_file_paths(benchmarks_path(),"malwaredb.malekal.com/windows/X86_64/69ca9a1113f95f9c08c9031ab4418fbf")},
            "./test01/malwaredb.malekal.com/windows/X86_64/69ca9a1113f95f9c08c9031ab4418fbf"
            );
        do_test(
            concatenate_file_paths(benchmarks_path(),"malwaredb.malekal.com/windows/X86_32/be0f735eb31f143c9f0fc5a070ea69df"),
            {},
            {concatenate_file_paths(benchmarks_path(),"malwaredb.malekal.com/windows/X86_32")},
            "./test01/malwaredb.malekal.com/windows/X86_32/be0f735eb31f143c9f0fc5a070ea69df"
            );
        do_test(
            concatenate_file_paths(benchmarks_path(),"loadlibs_ubuntu_X86_64/loadlibs_Linux_Release"),
            {},
            {concatenate_file_paths(benchmarks_path(),"loadlibs_ubuntu_X86_64")},
            "./test01/loadlibs_Linux_Release (X86_64)"
            );
        do_test(
            concatenate_file_paths(benchmarks_path(),"loadlibs_ubuntu_X86_64/libloadlibs_dynamic_lib_one_Linux_Release.so"),
            {},
            {concatenate_file_paths(benchmarks_path(),"loadlibs_ubuntu_X86_64")},
            "./test01/libloadlibs_dynamic_lib_one_Linux_Release (X86_64)"
            );
        do_test(
            concatenate_file_paths(benchmarks_path(),"loadlibs_ubuntu_X86_64/libloadlibs_dynamic_lib_seven_Linux_Release.so"),
            {},
            {concatenate_file_paths(benchmarks_path(),"loadlibs_ubuntu_X86_64")},
            "./test01/libloadlibs_dynamic_lib_seven_Linux_Release (X86_64)"
            );
        do_test(
            concatenate_file_paths(benchmarks_path(),"crackme_debian_X86_32"),
            {},
            {benchmarks_path()},
            "./test01/crackme_debian_X86_32"
            );
        do_test(
            concatenate_file_paths(benchmarks_path(),"crackme_debian_X86_64"),
            {},
            {benchmarks_path()},
            "./test01/crackme_debian_X86_64"
            );
        do_test(
            concatenate_file_paths(benchmarks_path(),"tiny32-00_debian_X86_32"),
            {},
            {benchmarks_path()},
            "./test01/tiny32-00_debian_X86_32"
            );
        do_test(
            concatenate_file_paths(benchmarks_path(),"tiny32-01_debian_X86_32"),
            {},
            {benchmarks_path()},
            "./test01/tiny32-01_debian_X86_32"
            );
        do_test(
            concatenate_file_paths(benchmarks_path(),"tiny64_debian_X86_64"),
            {},
            {benchmarks_path()},
            "./test01/tiny64_debian_X86_64"
            );
        do_test(
            concatenate_file_paths(benchmarks_path(),"tiny64-hello_debian_X86_64"),
            {},
            {benchmarks_path()},
            "./test01/tiny64-hello_debian_X86_64"
            );
    }
    catch(std::exception const& e)
    {
        try { save_crash_report(e.what()); } catch (...) {}
        return -1;
    }
    catch(...)
    {
        try { save_crash_report("Unknown exception was thrown."); } catch (...) {}
        return -2;
    }
    return 0;
}
