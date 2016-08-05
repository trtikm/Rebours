#include <natexe/argparser.hpp>
#include <natexe/assumptions.hpp>
#include <natexe/invariants.hpp>
#include <natexe/development.hpp>
#include <natexe/msgstream.hpp>
#include <unordered_map>
#include <unordered_set>
#include <cstdlib>

namespace argparser { namespace {


std::string const  KWD_HELP{ "--help" };
std::string const  KWD_VERSION{ "--version" };
std::string const  KWD_BINARY{ "--binary" };
std::string const  KWD_IGNORE{ "--ignore" };
std::string const  KWD_SEARCH{ "--search" };
std::string const  KWD_PROGRAM{ "--program" };
std::string const  KWD_SAVE_DESCRIPTOR{ "--save-descriptor" };
std::string const  KWD_SAVE_PROLOGUE{ "--save-prologue" };
std::string const  KWD_SAVE_MICROCODE{ "--save-program" };
std::string const  KWD_SAVE_DISASSEMBLY{ "--save-disassembly" };
std::string const  KWD_TIMEOUT{ "--timeout" };
std::string const  KWD_LOGFILE{ "--log-file" };
std::unordered_set<std::string> const  KEYWORDS{
        KWD_HELP,
        KWD_VERSION,
        KWD_BINARY,
        KWD_IGNORE,
        KWD_SEARCH,
        KWD_PROGRAM,
        KWD_SAVE_DESCRIPTOR,
        KWD_SAVE_PROLOGUE,
        KWD_SAVE_MICROCODE,
        KWD_SAVE_DISASSEMBLY,
        KWD_TIMEOUT,
        KWD_LOGFILE,
};
std::unordered_map< std::string,std::vector<std::string> >  args;


std::string const  OPT_DESCRIPTOR_NO_SECTIONS{ "--no-section-contents" };


std::string const  HELP_TEXT =
        msgstream() << "It performs a native execution of a Microcode program recognised from a given\n"
                       "executable file. The executable file can passed directly or it can alternatively\n"
                       "be represented by a Prologue program coupled posibly with a Microcode program\n"
                       "reresenting a fraction of already recovered code in the executable.\n\n"
                       "Use the following commad line options to define your query:\n\n"
                    << KWD_HELP << "\n"
                    << "        Prints this help message.\n\n"
                    << KWD_BINARY << "=[<path>/]<name>\n"
                    << "        Defines a disc path <path> and name <name> of an executable file to be\n"
                       "        converted to a Microcode program and executed. The [<path>/]<name> can\n"
                       "        be enclosed in quotation marks, e.g. when it contains spaces.\n\n"
                    << KWD_IGNORE << "=[<path1>/]<name1>, ..., [<pathN>/]<nameN>\n"
                    << "        Specifies list of shared libraries, which should not be loaded together\n"
                       "        with an executable specified in any of " << KWD_BINARY << " options.\n"
                       "        Individual path-names can be enclosed in quotation marks. You do not\n"
                       "        have to specify this option, if no library should be ignored.\n\n"
                       "        This option can be used only with the option " << KWD_BINARY << ".\n\n"
                    << KWD_SEARCH << "=<directory1>, ..., <directoryN>\n"
                    << "        Defines a list of directories in which the program will look for shared\n"
                       "        libraries on which a loaded executable depends on. The directories are\n"
                       "        seached in the order specified here. This option can be used only with\n"
                       "        the option " << KWD_BINARY << ".\n\n"
                    << KWD_PROGRAM << "= [<path1>/]<name1>, [<path>/]<name>\n"
                    << "        It is a path-name of a file containing a Prologue program representing\n"
                       "        an executable file to be analysed. The second path-name references to\n"
                       "        a Microcode program representing a already recovered part of the\n"
                       "        executable file. This option cannot be mixed with the option " << KWD_BINARY << ".\n\n"
                    << KWD_SAVE_DESCRIPTOR << "= [<path>/]<name> [, " << OPT_DESCRIPTOR_NO_SECTIONS << "]\n"
                    << "        It is a a path-name of a start HTML file into which the data stored in\n"
                       "        the descriptor will be saved to. Since several other files are generated\n"
                       "        into the directory of the start file, we recommend to specify the start\n"
                       "        file into a separate directory. The path-name of the start file can be\n"
                       "        followed by the text '" << OPT_DESCRIPTOR_NO_SECTIONS << "'. If specified, then contents\n"
                       "        of section of the captured executable file won't be saved.\n\n"
                    << KWD_SAVE_PROLOGUE << "= [<path>/]<name>\n"
                    << "        It is a a path-name of a file where the Prologue program will be stored.\n"
                       "        This option cannot be mixed with the option " << KWD_PROGRAM << ".\n\n"
                    << KWD_SAVE_MICROCODE << "= [<path>/]<name>\n"
                    << "        It is a a path-name of a file where a recovered Microcode program will.\n"
                       "        be stored.\n\n"
                    << KWD_SAVE_DISASSEMBLY << "= [<path>/]<name>\n"
                    << "        It is a a path-name of a file where a resulting disassebly program will.\n"
                       "        be stored. This program is dependent on CPU architecture and OS of the\n"
                       "        executable file.\n\n"
                    << KWD_TIMEOUT << "= <positive-integer>\n"
                    << "        It is a wall-clock duration in seconds reserved for the analysis\n"
                       "        performing the native execution. Durationso of other actions performed\n"
                       "        by the tool are not included. If not specified, the default timeout 60s\n"
                       "        is used.\n\n"
                    << KWD_LOGFILE << "= [<path>/]<name>\n"
                    << "        It is a a path-name of a start HTML file containing a logged info about\n"
                       "        all results from the tool. It serves for user friendly browing through\n"
                       "        all results. If it is not specified, then if is automatically set to the\n"
                       "        directory './log/start.html' relative to the current directory.\n\n"
                    ;
std::string const  VERSION_TEXT = "0.1";


bool  has_white_chars(std::string const&  str)
{
    return str.find_first_of(' ') != std::string::npos || str.find_first_of('\t') != std::string::npos;
}

void  build_argument_lines(int argc, char* argv[], std::vector<std::string>&  lines)
{
    for (int  i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
            lines.push_back("");
        if (has_white_chars(argv[i]))
        {
            lines.back().append("\"");
            lines.back().append(argv[i]);
            lines.back().append("\"");
        }
        else
            lines.back().append(argv[i]);
    }
}

void  skip_whites(std::string const&  input, int&  cursor)
{
    for ( ; cursor < (int)input.size(); ++cursor)
        if (input.at(cursor) != ' ' && input.at(cursor) != '\t')
            break;
}

int  read_till(std::string const&  input, int  cursor, std::unordered_set<char> const&  terminals)
{
    for ( ; cursor < (int)input.size(); ++cursor)
        if (terminals.count(input.at(cursor)) == 1ULL)
            break;
    return cursor;
}

std::string  tokenise(std::string const&  input, std::vector<std::string>&  tokens)
{
    for (int  cursor = 0; true; )
    {
        skip_whites(input,cursor);
        if (cursor == (int)input.size())
            break;
        if (input.at(cursor) == '-')
        {
            int const  end = read_till(input,cursor,{' ','\t','='});
            tokens.push_back(input.substr(cursor,end - cursor));
            cursor = end;
        }
        else if (input.at(cursor) == '=')
        {
            tokens.push_back("=");
            ++cursor;
        }
        else if (input.at(cursor) == ',')
        {
            tokens.push_back(",");
            ++cursor;
        }
        else if (input.at(cursor) == '"')
        {
            ++cursor;
            int const  end = read_till(input,cursor,{'"'});
            if (end == (int)input.size())
                return msgstream() << "Missing a matching '\"' for the symbol '\"' at the index " << cursor << ".";
            tokens.push_back(input.substr(cursor,end - cursor));
            cursor = end + 1;
        }
        else
        {
            int const  end = read_till(input,cursor,{' ','\t'});
            tokens.push_back(input.substr(cursor,end - cursor));
            cursor = end;
        }
    }
    if (tokens.empty())
        return "No takens were recognised in the parameter's text.";

    if (KEYWORDS.count(tokens.front()) == 0ULL)
        return msgstream() << "Unknown keyword '" << tokens.front() << "'.";

    if (tokens.size() > 1ULL)
    {
        if (tokens.at(1) != "=")
            return msgstream() << "Missing '=' after the keyword '" << tokens.front() << "'.";

        if ((tokens.size() - 2) % 2 == 0)
            return "Unexpected separator ',' after the last value.";

        for (int  i = 2; i < (int)tokens.size(); ++i)
            if (i % 2 == 1 && tokens.at(i) != ",")
                return msgstream() << "Missing separator ',' after the value number " << i - 2 << ".";
    }

    return "";
}

std::string  parse_argument(std::string const&  input, std::string&  kwd, std::vector<std::string>&  params)
{
    std::vector<std::string>  tokens;
    std::string  error_msg = tokenise(input,tokens);
    if (!error_msg.empty())
        return error_msg;
    INVARIANT(!tokens.empty());
    kwd = tokens.front();
    for (int  i = 2; i < (int)tokens.size(); i += 2)
        params.push_back(tokens.at(i));
    return error_msg;
}

bool  is_uint(std::string const&  text)
{
    for (auto c : text)
        if (!std::isdigit(c))
            return false;
    return true;
}

void  add_default_args()
{
    if (args.empty())
        args.insert({KWD_HELP,{}});
    else if (args.count(KWD_HELP) == 0ULL && args.count(KWD_VERSION) == 0ULL)
    {
        if (args.count(KWD_TIMEOUT) == 0ULL)
            args.insert({KWD_TIMEOUT,{"60"}});
        if (args.count(KWD_BINARY) != 0ULL && args.count(KWD_IGNORE) == 0ULL)
            args.insert({KWD_IGNORE,{}});
        if (args.count(KWD_BINARY) != 0ULL && args.count(KWD_SEARCH) == 0ULL)
            args.insert({KWD_SEARCH,{}});
        if (args.count(KWD_LOGFILE) == 0ULL)
            args.insert({KWD_LOGFILE,{"./log/start.html"}});
    }
}

std::string  check_argument_consistency(std::string const&  kwd, std::vector<std::string> const&  params)
{
    if ((kwd == KWD_HELP || kwd == KWD_VERSION) && !params.empty())
        return msgstream() << "The parameter '" << kwd << "' accepts no arguments.";
    else if ((kwd == KWD_BINARY ||
              kwd == KWD_SAVE_PROLOGUE ||
              kwd == KWD_SAVE_MICROCODE ||
              kwd == KWD_SAVE_DISASSEMBLY ||
              kwd == KWD_LOGFILE) && params.size() != 1ULL)
        return msgstream() << "The parameter '" << kwd << "' accepts 1 argument.";
    else if (kwd == KWD_PROGRAM && params.size() != 2ULL)
        return msgstream() << "The parameter '" << kwd << "' accepts 2 arguments.";
    else if (kwd == KWD_SAVE_DESCRIPTOR)
    {
        if (params.size() != 1ULL && params.size() != 2ULL)
            return msgstream() << "The parameter '" << kwd << "' accepts 1 or 2 arguments.";
        if (params.size() == 2ULL && params.at(1) != OPT_DESCRIPTOR_NO_SECTIONS)
            return msgstream() << "The second value of the parameter '" << kwd << "' must be the text '" << OPT_DESCRIPTOR_NO_SECTIONS << "'.";
    }
    else if (kwd == KWD_TIMEOUT)
    {
        if (params.size() != 1ULL)
            return msgstream() << "The parameter '" << kwd << "' accepts 1 argument.";
        if (!is_uint(params.front()))
            return msgstream() << "The value '" << params.front() << "' of the parameter '" << kwd << "' is not an unsigned integer.";
        if (std::atoi(params.front().c_str()) == 0)
            return msgstream() << "The timeout cannot be 0.";
    }
    return "";
}

std::string  check_consistency()
{
    if (args.count(KWD_HELP) != 0ULL || args.count(KWD_VERSION) != 0ULL)
    {
        if (args.size() != 1ULL)
            return msgstream() << "Options '" << KWD_HELP << "' and '" << KWD_VERSION << "' cannot be mixed with other options.";
        return "";
    }

    if (args.count(KWD_BINARY) == 0ULL && args.count(KWD_PROGRAM) == 0ULL)
        return msgstream() << "Exactly one of the options '" << KWD_BINARY << "' and '" << KWD_PROGRAM << "' must be specified.";

    if (args.count(KWD_BINARY) != 0ULL && args.count(KWD_PROGRAM) != 0ULL)
        return msgstream() << "Options '" << KWD_BINARY << "' and '" << KWD_PROGRAM << "' cannot be mixed.";

    if (args.count(KWD_PROGRAM) != 0ULL && args.count(KWD_IGNORE) != 0ULL)
        return msgstream() << "Options '" << KWD_PROGRAM << "' and '" << KWD_IGNORE << "' cannot be mixed.";
    if (args.count(KWD_PROGRAM) != 0ULL && args.count(KWD_SEARCH) != 0ULL)
        return msgstream() << "Options '" << KWD_PROGRAM << "' and '" << KWD_SEARCH << "' cannot be mixed.";
    if (args.count(KWD_PROGRAM) != 0ULL && args.count(KWD_SAVE_PROLOGUE) != 0ULL)
        return msgstream() << "Options '" << KWD_PROGRAM << "' and '" << KWD_SAVE_PROLOGUE << "' cannot be mixed.";

    return "";
}


}}

namespace argparser {


std::string  parse_arguments(int argc, char* argv[])
{
    std::vector<std::string>  lines;
    build_argument_lines(argc,argv,lines);

    std::string  error_msg;
    for (unsigned int  i = 0U; i < lines.size(); ++i)
    {
        std::string  kwd;
        std::vector<std::string>  params;
        error_msg = parse_argument(lines.at(i),kwd,params);
        if (!error_msg.empty())
            return msgstream() << "In argument number " << i << ": " << error_msg;
        error_msg = check_argument_consistency(kwd,params);
        if (!error_msg.empty())
            return msgstream() << "In argument number " << i << ": " << error_msg;
        if (KEYWORDS.count(kwd) != 1ULL)
            return msgstream() << "In argument number " << i << ": " << "Unknown argument type '" << kwd << ".";
        if (args.count(kwd) != 0ULL)
            return msgstream() << "In argument number " << i << ": " << "The argument '" << kwd << "' is specified more than once.";
        args.insert({kwd,params});
    }
    add_default_args();
    return check_consistency();
}

bool  do_print_help()
{
    return args.count(KWD_HELP) != 0ULL;
}

std::string const&  help_text()
{
    ASSUMPTION(do_print_help());
    return HELP_TEXT;
}

bool  do_print_version()
{
    return args.count(KWD_VERSION) != 0ULL;
}

std::string const&  version_text()
{
    ASSUMPTION(do_print_version());
    return VERSION_TEXT;
}

bool  use_loader()
{
    return args.count(KWD_BINARY) != 0ULL;
}

std::string const&  path_and_name_of_a_binary_file_to_be_loaded()
{
    ASSUMPTION(use_loader());
    return args.at(KWD_BINARY).front();
}

std::vector<std::string> const&  ignored_dynamic_link_files()
{
    ASSUMPTION(use_loader());
    return args.at(KWD_IGNORE);
}

std::vector<std::string> const&  search_directories_for_dynamic_link_files()
{
    ASSUMPTION(use_loader());
    return args.at(KWD_SEARCH);
}

std::string const&  path_and_name_of_a_prologue_program()
{
    ASSUMPTION(!use_loader());
    return args.at(KWD_PROGRAM).at(0);
}

std::string const&  path_and_name_of_a_microcode_program()
{
    ASSUMPTION(!use_loader());
    return args.at(KWD_PROGRAM).at(1);
}

bool  do_save_descriptor()
{
    return args.count(KWD_SAVE_DESCRIPTOR) != 0ULL;
}

bool  do_save_sections_of_descriptor()
{
    ASSUMPTION(do_save_descriptor());
    return args.at(KWD_SAVE_DESCRIPTOR).size() == 1ULL;
}

std::string const&  save_path_and_name_of_a_descriptor_start_file()
{
    ASSUMPTION(do_save_descriptor());
    return args.at(KWD_SAVE_DESCRIPTOR).at(0ULL);
}

bool  do_save_prologue_program()
{
    return args.count(KWD_SAVE_PROLOGUE) != 0ULL;
}

std::string const&  save_path_and_name_of_a_prologue_program()
{
    ASSUMPTION(do_save_prologue_program() || use_loader());
    return args.at(KWD_SAVE_PROLOGUE).front();
}

bool  do_save_recovered_program()
{
    return args.count(KWD_SAVE_MICROCODE) != 0ULL;
}

std::string const&  save_path_and_name_of_a_recovered_program()
{
    ASSUMPTION(do_save_recovered_program());
    return args.at(KWD_SAVE_MICROCODE).front();
}

bool  do_save_encoded_program()
{
    return args.count(KWD_SAVE_DISASSEMBLY) != 0ULL;
}

std::string const&  save_path_and_name_of_an_encoded_program()
{
    ASSUMPTION(do_save_encoded_program());
    return args.at(KWD_SAVE_DISASSEMBLY).front();
}

uint32_t  timeout_in_seconds()
{
    ASSUMPTION(args.count(KWD_TIMEOUT) != 0ULL);
    return std::atoi(args.at(KWD_TIMEOUT).front().c_str());
}

std::string const&  path_and_name_of_the_output_log_file()
{
    return args.at(KWD_LOGFILE).front();
}


}
