#include <natexe/argparser.hpp>
#include <rebours/utility/file_utils.hpp>
#include <rebours/utility/assumptions.hpp>
#include <rebours/utility/invariants.hpp>
#include <rebours/utility/msgstream.hpp>
#include <rebours/program/program.hpp>
#include <rebours/program/assembly.hpp>
#include <rebours/MAL/loader/load.hpp>
#include <rebours/MAL/reloader/reload.hpp>
#include <rebours/MAL/descriptor/storage.hpp>
#include <rebours/MAL/descriptor/dump.hpp>
#include <rebours/MAL/prologue/builder.hpp>
#include <rebours/MAL/recogniser/recognise.hpp>
#include <rebours/MAL/recogniser/dump.hpp>
#include <rebours/analysis/native_execution/run.hpp>
#include <rebours/MAL/encoder/encode.hpp>
#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>


static void log_error(std::string const&  error_message)
{
    std::cout << "ERROR: " << error_message << "\n";
    std::ofstream  ofile("natexe_LOG.txt", std::ios_base::app );
    ofile << error_message << "\n";
}

static void save_crash_report(std::string const& crash_message)
{
    std::cout << "ERROR: " << crash_message << "\n";
    std::ofstream  ofile("natexe_CRASH.txt", std::ios_base::app );
    ofile << crash_message << "\n";
}

static void  save_log_file(std::string const&  analysis_log_root_dir, std::string const&  error_message)
{
    struct local
    {
        static std::string  compute_relative_pathname(std::string const&  pathname)
        {
            std::string const  dir =
                    fileutl::get_relative_path(
                                fileutl::absolute_path(fileutl::parse_path_in_pathname(pathname)),
                                fileutl::absolute_path(fileutl::parse_path_in_pathname(argparser::path_and_name_of_the_output_log_file()))
                                );
            std::string const  name = fileutl::parse_name_in_pathname(pathname);
            return fileutl::concatenate_file_paths(dir,name);
        }
    };

    std::fstream  ostr(argparser::path_and_name_of_the_output_log_file(), std::ios_base::out);
    if (!ostr.is_open())
    {
        log_error(msgstream() << "Cannot open the output log file '" << argparser::path_and_name_of_the_output_log_file() << "'.");
        return;
    }

    ostr << "<!DOCTYPE html>\n";
    ostr << "<html>\n";
    ostr << "<head>\n";
    ostr << "<style>\n";
    ostr << "table, th, td {\n";
    ostr << "    border: 1px solid black;\n";
    ostr << "    border-collapse: collapse;\n";
    ostr << "}\n";
    ostr << "th, td {\n";
    ostr << "    padding: 5px;\n";
    ostr << "}\n";
    ostr << "h1, h2, h3, h4, p, a, table, ul { font-family: \"Liberation serif\", serif; }\n";
    ostr << "p, a, table, ul { font-size: 12pt; }\n";
    ostr << "h4 { font-size: 12pt; }\n";
    ostr << "h3 { font-size: 14pt; }\n";
    ostr << "h2 { font-size: 18pt; }\n";
    ostr << "h1 { font-size: 24pt; }\n";
    ostr << "tt { font-family: \"Liberation Mono\", monospace; }\n";
    ostr << "tt { font-size: 10pt; }\n";
    ostr << "body {\n";
    ostr << "    background-color: white;\n";
    ostr << "    color: black;\n";
    ostr << "}\n";
    ostr << "</style>\n";
    ostr << "</head>\n";
    ostr << "<body>\n";
    ostr << "<h1>Overview of results</h1>\n";
    ostr << "<p>The analysis has terminated with the message: " << error_message << "</p>\n";
    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Property</th>\n";
    ostr << "    <th>Value</th>\n";
    ostr << "  </tr>\n";
    if (argparser::do_save_encoded_program())
    {
        ostr << "  <tr>\n";
        ostr << "    <td>Disassembled program</td>\n";
        ostr << "    <td><a href=\"./" << local::compute_relative_pathname(argparser::save_path_and_name_of_an_encoded_program()) << "\">here</a></td>\n";
        ostr << "  </tr>\n";
    }
    if (argparser::do_save_recovered_program())
    {
        ostr << "  <tr>\n";
        ostr << "    <td>Recovered Microcode program</td>\n";
        ostr << "    <td><a href=\"./" << local::compute_relative_pathname(argparser::save_path_and_name_of_a_recovered_program()) << "\">here</a></td>\n";
        ostr << "  </tr>\n";
    }
    if (argparser::do_save_prologue_program())
    {
        ostr << "  <tr>\n";
        ostr << "    <td>Generated Prologue program</td>\n";
        ostr << "    <td><a href=\"./" << local::compute_relative_pathname(argparser::save_path_and_name_of_a_prologue_program()) << "\">here</a></td>\n";
        ostr << "  </tr>\n";
    }
    if (!argparser::use_loader())
    {
        ostr << "  <tr>\n";
        ostr << "    <td>Loaded initial Microcode program</td>\n";
        ostr << "    <td><a href=\"./" << local::compute_relative_pathname(argparser::path_and_name_of_a_microcode_program()) << "\">here</a></td>\n";
        ostr << "  </tr>\n";
        ostr << "  <tr>\n";
        ostr << "    <td>Loaded Prologue program</td>\n";
        ostr << "    <td><a href=\"./" << local::compute_relative_pathname(argparser::path_and_name_of_a_prologue_program()) << "\">here</a></td>\n";
        ostr << "  </tr>\n";
    }
    if (argparser::do_save_descriptor())
    {
        ostr << "  <tr>\n";
        ostr << "    <td>Content of MAL/descriptor</td>\n";
        ostr << "    <td><a href=\"./" << local::compute_relative_pathname(argparser::save_path_and_name_of_a_descriptor_start_file()) << "\">here</a></td>\n";
        ostr << "  </tr>\n";
    }
    if (!analysis_log_root_dir.empty() && fileutl::file_exists(fileutl::concatenate_file_paths(analysis_log_root_dir,"start.html")))
    {
        ostr << "  <tr>\n";
        ostr << "    <td>Log info from the analysis</td>\n";
        ostr << "    <td><a href=\"./" << local::compute_relative_pathname(fileutl::concatenate_file_paths(analysis_log_root_dir,"start.html")) << "\">here</a></td>\n";
        ostr << "  </tr>\n";
    }
    ostr << "</table>\n";
    ostr << "</body>\n";
    ostr << "</html>\n";
}

int main(int argc, char* argv[])
{
    try
    {
        std::string  error_msg = argparser::parse_arguments(argc,argv);
        if (!error_msg.empty())
        {
            log_error(error_msg);
            return -1;
        }

        if (argparser::do_print_help())
        {
            std::cout << argparser::help_text() << "\n";
            return 0;
        }

        if (argparser::do_print_version())
        {
            std::cout << argparser::version_text() << "\n";
            return 0;
        }

        std::string  analysis_log_root_dir =
                argparser::path_and_name_of_the_output_log_file().empty() ?
                        argparser::path_and_name_of_the_output_log_file() :
                        fileutl::concatenate_file_paths(
                            fileutl::normalise_path(fileutl::parse_path_in_pathname(argparser::path_and_name_of_the_output_log_file())),
                            "log_from_analysis"
                            );
        fileutl::create_directory(analysis_log_root_dir);
        analysis_log_root_dir = fileutl::absolute_path(analysis_log_root_dir);

        std::string  error_message;

        if (argparser::use_loader())
        {
            mal::descriptor::file_descriptor_ptr   file_descriptor =
                    loader::load(argparser::path_and_name_of_a_binary_file_to_be_loaded(),
                                 argparser::ignored_dynamic_link_files(),
                                 argparser::search_directories_for_dynamic_link_files(),
                                 error_msg);
            if (!error_msg.empty())
            {
                log_error(error_msg);
                return -2;
            }
            INVARIANT(file_descriptor.operator bool());

            mal::descriptor::storage  descriptor{file_descriptor};

            if (argparser::do_save_descriptor())
                mal::descriptor::dump_html(descriptor,argparser::save_path_and_name_of_a_descriptor_start_file(),argparser::do_save_sections_of_descriptor());

            std::pair<std::unique_ptr<microcode::program>,std::unique_ptr<microcode::annotations> > const  prologue = mal::prologue::build(descriptor);
            INVARIANT(prologue.first.operator bool());

            std::string const&  executable_pathname =  descriptor.file_descriptor()->dependencies_of_loaded_files()->root_file();

            std::map<uint64_t,uint64_t>  important_code;
            for (auto const& address_section : *descriptor.file_descriptor()->sections_table())
                if (address_section.second->file_props()->path() == descriptor.file_descriptor()->dependencies_of_loaded_files()->root_file()
                        && address_section.second->has_execute_access())
                    important_code.insert({address_section.first,address_section.second->end_address()});

            uint64_t const  entry_point = descriptor.file_descriptor()->entry_point();

            descriptor.drop_file_info();

            if (argparser::do_save_prologue_program())
            {
                std::fstream  prologue_file(argparser::save_path_and_name_of_a_prologue_program(), std::ios_base::out);
                if (!prologue_file.is_open())
                {
                    log_error(msgstream() << "Cannot open the output prologue assembly file '" << argparser::save_path_and_name_of_a_prologue_program() << "'.");
                    return -3;
                }

                microcode::save_program_as_assembly_text(prologue_file,*prologue.first,&*prologue.second);
            }

            std::unique_ptr<microcode::program> const  program = microcode::create_initial_program(fileutl::parse_name_in_pathname(executable_pathname),"MAIN");
            std::unique_ptr<microcode::annotations> const  annotations = microcode::create_initial_annotations();
            microcode::append({ {program->start_component().entry(), { {"PROGRAM.NAME",program->name()} } } }, *annotations);

            std::vector<uint8_t>  default_stack_init_data;
            mal::descriptor::linearise(descriptor.default_stack_init_data(),default_stack_init_data);

            error_message =
                    analysis::natexe::run(*prologue.first,
                                          *program,
                                          *annotations,
                                          descriptor.heap_start(),
                                          descriptor.heap_end(),
                                          descriptor.start_address_of_temporaries(),
                                          important_code,
                                          { { program->start_component().entry(), entry_point } },
                                          default_stack_init_data,
                                          std::bind(&mal::recogniser::recognise,std::cref(descriptor),std::placeholders::_1,std::placeholders::_2,std::placeholders::_3),
                                          argparser::timeout_in_seconds(),
                                          analysis_log_root_dir,
                                          true,
                                          std::bind(&mal::recogniser::dump_details_of_recognised_instruction,std::placeholders::_1,std::placeholders::_2),
                                          descriptor.ranges_to_registers()
                                          );

            if (argparser::do_save_recovered_program())
            {
                std::fstream  recovered_program_file(argparser::save_path_and_name_of_a_recovered_program(), std::ios_base::out);
                if (!recovered_program_file.is_open())
                {
                    log_error(msgstream() << "Cannot open the output recovered program file '" << argparser::save_path_and_name_of_a_recovered_program() << "'.");
                    return -4;
                }

                microcode::save_program_as_assembly_text(recovered_program_file,*program,&*annotations);
            }

            if (argparser::do_save_encoded_program())
            {
                std::fstream  encoded_program_file(argparser::save_path_and_name_of_an_encoded_program(), std::ios_base::out);
                if (!encoded_program_file.is_open())
                {
                    log_error(msgstream() << "Cannot open the output encoded program file '" << argparser::save_path_and_name_of_an_encoded_program() << "'.");
                    return -5;
                }

                file_descriptor = mal::reloader::reload(*prologue.first,*prologue.second,true,error_msg) ;
                if (!error_msg.empty())
                {
                    log_error(error_msg);
                    return -6;
                }
                INVARIANT(file_descriptor.operator bool());

                mal::encoder::encode(encoded_program_file,mal::descriptor::storage{file_descriptor},*program,&*annotations);
            }
        }
        else
        {
            std::fstream  prologue_file(argparser::path_and_name_of_a_prologue_program(), std::ios_base::in);
            if (!prologue_file.is_open())
            {
                log_error(msgstream() << "Cannot open the prologue assembly file '" << argparser::path_and_name_of_a_prologue_program() << "'.");
                return -2;
            }

            std::pair<std::unique_ptr<microcode::program>,std::unique_ptr<microcode::annotations> > const  prologue =
                    microcode::create_program_from_assembly_text(prologue_file,error_msg);

            prologue_file.close();

            if (!error_msg.empty())
            {
                log_error(error_msg);
                return -3;
            }
            INVARIANT(prologue.first.operator bool());

            mal::descriptor::file_descriptor_ptr  file_descriptor = mal::reloader::reload(*prologue.first,*prologue.second,false,error_msg);

            if (!error_msg.empty())
            {
                log_error(error_msg);
                return -4;
            }
            INVARIANT(file_descriptor.operator bool());

            mal::descriptor::storage const  descriptor{file_descriptor};

            if (argparser::do_save_descriptor())
                mal::descriptor::dump_html(descriptor,argparser::save_path_and_name_of_a_descriptor_start_file(),argparser::do_save_sections_of_descriptor());

            std::string const&  executable_pathname =  descriptor.file_descriptor()->dependencies_of_loaded_files()->root_file();

            std::map<uint64_t,uint64_t>  important_code;
            for (auto const& address_section : *descriptor.file_descriptor()->sections_table())
                if (address_section.second->file_props()->path() == executable_pathname && address_section.second->has_execute_access())
                    important_code.insert({address_section.first,address_section.second->end_address()});

            std::fstream  recovered_program_file(argparser::path_and_name_of_a_microcode_program(), std::ios_base::in);
            if (!recovered_program_file.is_open())
            {
                log_error(msgstream() << "Cannot open the assembly file of the recovered program '" << argparser::path_and_name_of_a_microcode_program() << "'.");
                return -5;
            }

            std::pair<std::unique_ptr<microcode::program>,std::unique_ptr<microcode::annotations> > const  recovered_program =
                    microcode::create_program_from_assembly_text(recovered_program_file,error_msg);

            recovered_program_file.close();

            if (!error_msg.empty())
            {
                log_error(error_msg);
                return -6;
            }
            INVARIANT(recovered_program.first.operator bool());

            recovered_program.first->name() = fileutl::parse_name_in_pathname(executable_pathname);

            std::vector<uint8_t>  default_stack_init_data;
            mal::descriptor::linearise(descriptor.default_stack_init_data(),default_stack_init_data);

            error_message =
                    analysis::natexe::run(*prologue.first,
                                          *recovered_program.first,
                                          *recovered_program.second,
                                          descriptor.heap_start(),
                                          descriptor.heap_end(),
                                          descriptor.start_address_of_temporaries(),
                                          important_code,
                                          {}, // TODO: here should be passed all unexplored exits
                                          default_stack_init_data,
                                          std::bind(&mal::recogniser::recognise,std::cref(descriptor),std::placeholders::_1,std::placeholders::_2,std::placeholders::_3),
                                          argparser::timeout_in_seconds(),
                                          analysis_log_root_dir,
                                          true,
                                          std::bind(&mal::recogniser::dump_details_of_recognised_instruction,std::placeholders::_1,std::placeholders::_2),
                                          descriptor.ranges_to_registers()
                                          );

            if (argparser::do_save_recovered_program())
            {
                std::fstream  extended_recovered_program_file(argparser::save_path_and_name_of_a_recovered_program(), std::ios_base::out);
                if (!extended_recovered_program_file.is_open())
                {
                    log_error(msgstream() << "Cannot open the output extended recovered program file '" << argparser::save_path_and_name_of_a_recovered_program() << "'.");
                    return -4;
                }

                microcode::save_program_as_assembly_text(extended_recovered_program_file,*recovered_program.first,&*recovered_program.second);
            }

            if (argparser::do_save_encoded_program())
            {
                std::fstream  encoded_program_file(argparser::save_path_and_name_of_an_encoded_program(), std::ios_base::out);
                if (!encoded_program_file.is_open())
                {
                    log_error(msgstream() << "Cannot open the output encoded program file '" << argparser::save_path_and_name_of_an_encoded_program() << "'.");
                    return -5;
                }

                file_descriptor = mal::reloader::reload(*prologue.first,*prologue.second,true,error_msg) ;
                if (!error_msg.empty())
                {
                    log_error(error_msg);
                    return -6;
                }
                INVARIANT(file_descriptor.operator bool());

                mal::encoder::encode(encoded_program_file,mal::descriptor::storage{file_descriptor},*recovered_program.first,&*recovered_program.second);
            }
        }

        save_log_file(analysis_log_root_dir,error_message);
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
