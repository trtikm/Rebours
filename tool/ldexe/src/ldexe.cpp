#include <rebours/MAL/loader/load.hpp>
#include <rebours/MAL/loader/dump.hpp>
#include <rebours/utility/file_utils.hpp>
#include <rebours/utility/assumptions.hpp>
#include <rebours/utility/invariants.hpp>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <fstream>


static std::string const  KWD_HELP{ "--help" };
static std::string const  KWD_BINARY{ "--binary" };
static std::string const  KWD_DUMP_TO{ "--dump" };
static std::string const  KWD_IGNORE{ "--ignore" };
static std::string const  KWD_SEARCH_IN{ "--search" };
static std::set<std::string> const  KEYWORDS{ KWD_HELP, KWD_BINARY, KWD_DUMP_TO, KWD_IGNORE, KWD_SEARCH_IN };

static std::string  parse_argument(std::string const&  input, std::map< std::string,std::vector<std::string> >&  output)
{
    if (input == KWD_HELP)
    {
        output.insert({KWD_HELP,{}});
        return "";
    }

    for (auto const& raw_kwd : KEYWORDS)
    {
        std::string const  kwd = raw_kwd + "=";
        if (kwd != KWD_HELP && input.size() > kwd.size() && input.substr(0ULL,kwd.size()) == kwd)
        {
            output[raw_kwd].push_back( input.substr(kwd.size(),input.size()) );
            return "";
        }
    }

    return std::string("Unknown command: ") + input;
}

static void load_and_dump_binary(std::string const&  file_pathname,
                                 std::vector<std::string> const&  ignored_libs,
                                 std::vector<std::string> const&  search_dirs,
                                 std::string const&  output_dir)
{
    std::cout << "Loading: "
              << (fileutl::file_exists(file_pathname) ? fileutl::absolute_path(file_pathname) : file_pathname)
              << "\n";
    std::string  error_message;
    loader::descriptor_ptr const  bfile_descriptor =
            loader::load(file_pathname,ignored_libs,search_dirs,error_message);
    if (!error_message.empty())
    {
        INVARIANT(!bfile_descriptor.operator bool());
        std::cout << "ERROR: " << error_message << std::endl;
    }
    else
    {
        INVARIANT(bfile_descriptor.operator bool());
        INVARIANT(error_message.empty());

        fileutl::create_directory(output_dir);
        std::string const  full_output_dir = fileutl::absolute_path(output_dir);
        std::string const  dump_file =
                fileutl::normalise_path(
                    fileutl::concatenate_file_paths(
                        full_output_dir,
                        fileutl::parse_name_in_pathname(file_pathname) + ".html"
                        )
                    );
        std::cout << "Dumping: " << dump_file << "\n";
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
    try
    {
        std::map< std::string,std::vector<std::string> >  args;
        for (int i = 1; i < argc; ++i)
        {
            std::string const  error_message = parse_argument(argv[i],args);
            if (!error_message.empty())
            {
                std::cout << "ERROR: " << error_message << "\n";
                return 0;
            }
        }

        if (args.empty() || args.count(KWD_HELP) != 0ULL)
        {
            std::cout << "\nIt allows you to see an initial content of a virtual memory when\n"
                         "a specified executable file is loaded from a disc (together with\n"
                         "dependant shared libraries, if any) with relocations performed.\n\n"
                         "Use the following commad line options to define your query:\n\n"
                      << KWD_HELP << "\n"
                      << "        Prints this help message.\n\n"
                      << KWD_BINARY << "=[<path>/]<name>\n"
                      << "        Defines a disc path <path> and name <name> of an executable\n"
                         "        file to be loaded into a virtual addess space in order to see\n"
                         "        the initial content of the memory. You can specify this option\n"
                         "        more than once. It has the same effect as running this program\n"
                         "        for each of the options separatelly. The [<path>/]<name> can be\n"
                         "        enclosed in quotation marks, e.g. when it contains spaces. You\n"
                         "        must specify at least one " << KWD_BINARY << " option.\n\n"
                      << KWD_DUMP_TO << "=<directory>\n"
                      << "        Defines an output directory into which all information about the\n"
                         "        initial state of a loaded executable will be saved. The information\n"
                         "        is stored in several HTML files. The root HTML file has the same\n"
                         "        name as the loaded executable with the extension '.html' appended.\n"
                         "        You must specify one option " << KWD_DUMP_TO << " for each options " << KWD_BINARY << "\n"
                         "        specified. Order of these options in the command line defines the\n"
                         "        unique matching of the options. If no option " << KWD_DUMP_TO << " is specified,\n"
                         "        then for each option " << KWD_BINARY << "=[<path>/]<name> there is automatically\n"
                         "        generated a corresponding option " << KWD_DUMP_TO << "=./reload_dump/<path>/<name>.\n"
                         "        Finally, if a dump directory <directory> does not exists, then it is\n"
                         "        automaticaly created. If it already exists, its content is merged\n"
                         "        with newly created files s.t. colliding files are overwritten without\n"
                         "        any notification.\n\n"
                      << KWD_IGNORE << "=[<path>/]<name>\n"
                      << "        Specifies a shared library, which should not be loaded together with\n"
                         "        an executable specified in any of " << KWD_BINARY << " options. You can specify\n"
                         "        this option more than once, to ignore several libraries. You do not\n"
                         "        have to specify this option, if no library should be ignored.\n\n"
                      << KWD_SEARCH_IN << "=<directory>\n"
                      << "        Defines a directory in which the program will look for shared\n"
                         "        libraries on which a loaded executable depends on. You can specify\n"
                         "        this option more than once, to provide more search directories.\n"
                         "        The directories are seached in the same order as is the order of\n"
                         "        these options in the command line. If no option " << KWD_SEARCH_IN << " is\n"
                         "        specified, then for each option " << KWD_BINARY << "=[<path>/]<name> there is\n"
                         "        used a single search dir <path> for the load of that executable.\n"
                         "        Otherwise, all specified options " << KWD_SEARCH_IN << " are shared amongst\n"
                         "        all loaded executables (specified in " << KWD_BINARY << " options).\n\n"
                      ;
            return 0;
        }

        if (args.count(KWD_BINARY) == 0ULL)
        {
            std::cout << "ERROR: No binary file to load was specified.\n";
            return 0;
        }
        if (args.count(KWD_DUMP_TO) == 0ULL)
        {
            std::string  common_preffix = fileutl::parse_path_in_pathname(args.at(KWD_BINARY).front());
            for (auto const& pathname : args.at(KWD_BINARY))
                common_preffix = fileutl::get_common_preffix(pathname,common_preffix);
            for (auto const& pathname : args.at(KWD_BINARY))
                args[KWD_DUMP_TO].push_back(
                        fileutl::concatenate_file_paths("./reload_dump",fileutl::get_relative_path(pathname,common_preffix)));
        }
        if (args.at(KWD_DUMP_TO).size() != args.at(KWD_BINARY).size())
        {
            std::cout << "ERROR: The count of binaries does not equal to count of dump directories.\n";
            return 0;
        }
        if (args.count(KWD_IGNORE) == 0ULL)
            args.insert({KWD_IGNORE,{}});

        std::cout << "\n";
        for (uint64_t i = 0ULL; i < args.at(KWD_BINARY).size(); ++i)
        {
            load_and_dump_binary(
                    args.at(KWD_BINARY).at(i),
                    args.at(KWD_IGNORE),
                    args.count(KWD_SEARCH_IN) == 0ULL ?
                            std::vector<std::string>{ fileutl::parse_path_in_pathname(args.at(KWD_BINARY).at(i)) } :
                            args.at(KWD_SEARCH_IN),
                    args.at(KWD_DUMP_TO).at(i)
                    );
            std::cout << "\n";
        }
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
