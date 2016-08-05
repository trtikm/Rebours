#include <rebours/MAL/loader/load.hpp>
#include <rebours/MAL/loader/file_utils.hpp>
#include <rebours/MAL/loader/detail/load_elf.hpp>
#include <rebours/MAL/loader/detail/load_pe.hpp>
#include <rebours/MAL/loader/detail/load_mach.hpp>
#include <rebours/MAL/loader/assumptions.hpp>
#include <rebours/MAL/loader/invariants.hpp>
#include <algorithm>
#include <fstream>

namespace loader { namespace detail { namespace {


std::shared_ptr<file_dependencies_map>  build_loaded_files_dependencies(
        file_dependencies_map const&  orig_dependances,
        loader::skipped_files const&  skipped_files)
{
    std::shared_ptr<file_dependencies_map> const  result{new file_dependencies_map};
    for (auto const& elem : orig_dependances)
    {
        if (skipped_files.count(elem.first) != 0ULL)
            continue;
        auto& successors =
                result->insert(file_dependencies_map::value_type(elem.first,{})).first->second;
        for (auto const& filename : elem.second)
        {
            if (std::find(skipped_files.cbegin(),skipped_files.cend(),filename) != skipped_files.cend())
                continue;
            successors.push_back(filename);
        }
    }
    return result;
}


}}}

namespace loader {


descriptor_ptr  load(std::string const& path_and_name_of_a_binary_file_to_be_loaded,
                     std::vector<std::string> const&  ignored_dynamic_link_files,
                     std::vector<std::string> const&  search_directories_for_dynamic_link_files,
                     std::string& error_message)
{
    descriptor_ptr  bfile_descriptor = load_elf(path_and_name_of_a_binary_file_to_be_loaded,
                                                ignored_dynamic_link_files,
                                                search_directories_for_dynamic_link_files,
                                                error_message);
    if (bfile_descriptor.operator bool())
    {
        INVARIANT(error_message.empty());
        return bfile_descriptor;
    }
    else if (!error_message.empty())
        return bfile_descriptor;

    bfile_descriptor = load_pe(path_and_name_of_a_binary_file_to_be_loaded,
                               ignored_dynamic_link_files,
                               search_directories_for_dynamic_link_files,
                               error_message);
    if (bfile_descriptor.operator bool())
    {
        INVARIANT(error_message.empty());
        return bfile_descriptor;
    }
    else if (!error_message.empty())
        return bfile_descriptor;

    bfile_descriptor = load_mach(path_and_name_of_a_binary_file_to_be_loaded,
                                 ignored_dynamic_link_files,
                                 search_directories_for_dynamic_link_files,
                                 error_message);
    if (bfile_descriptor.operator bool())
    {
        INVARIANT(error_message.empty());
        return bfile_descriptor;
    }
    else if (!error_message.empty())
        return bfile_descriptor;

    INVARIANT(!bfile_descriptor.operator bool());
    INVARIANT(error_message.empty());

    error_message = "Unsupported file format.";
    return bfile_descriptor;
}


descriptor_ptr  load_elf(std::string const& elf_file,
                         std::vector<std::string> const&  ignored_dynamic_link_files,
                         std::vector<std::string> const&  search_directories_for_dynamic_link_files,
                         std::string& error_message)
{
    if (!file_exists(elf_file))
    {
        error_message = "The passed file does not exist.";
        return descriptor_ptr();
    }
    if (is_directory(elf_file))
    {
        error_message = "The passed path does not reference a file.";
        return descriptor_ptr();
    }

    ASSUMPTION( error_message.empty() );

    detail::load_props_elf  load_props{
            normalise_path(absolute_path(elf_file)),
            ignored_dynamic_link_files,
            search_directories_for_dynamic_link_files
            };

    std::ifstream  elf{elf_file,std::ifstream::binary};
    if (!elf.is_open())
    {
        error_message = "Cannot open the passed file.";
        return descriptor_ptr();
    }

    file_props_ptr const  elf_props = detail::load_elf_file_props(elf_file,elf,load_props,error_message);
    if (error_message == detail::NOT_ELF_FILE())
    {
        error_message.clear();
        return descriptor_ptr();
    }
    if (!error_message.empty())
        return descriptor_ptr();

    error_message = elf_props->num_address_bits() == 32U ? detail::load_elf_32bit(elf,elf_props,load_props) :
                                                           detail::load_elf_64bit(elf,elf_props,load_props) ;
    if (!error_message.empty())
        return descriptor_ptr();

    return descriptor_ptr(
                new descriptor{
                        load_props.platform(),
                        load_props.sections_table(),
                        load_props.special_sections(),
                        load_props.entry_point(),
                        load_props.files_table(),
                        load_props.skipped_files(),
                        dependencies_graph_ptr(
                                new dependencies_graph {
                                        load_props.root_file(),
                                        detail::build_loaded_files_dependencies(
                                                *load_props.file_forward_dependencies(),
                                                *load_props.skipped_files()),
                                        detail::build_loaded_files_dependencies(
                                                *load_props.file_backward_dependencies(),
                                                *load_props.skipped_files())
                                        }
                                ),
                        dependencies_graph_ptr(
                                new dependencies_graph {
                                        load_props.root_file(),
                                        load_props.file_forward_dependencies(),
                                        load_props.file_backward_dependencies()
                                        }
                                ),
                        load_props.init_functions(),
                        load_props.fini_functions(),
                        load_props.performed_relocations(),
                        load_props.skipped_relocations(),
                        load_props.visible_symbol_table(),
                        load_props.hidden_symbol_table(),
                        load_props.warnings()
                        }
                );
}

descriptor_ptr  load_pe(std::string const&  pe_file,
                        std::vector<std::string> const&  ignored_dynamic_link_files,
                        std::vector<std::string> const&  search_directories_for_dynamic_link_files,
                        std::string& error_message)
{
    if (!file_exists(pe_file))
    {
        error_message = "The passed file does not exist.";
        return descriptor_ptr();
    }
    if (is_directory(pe_file))
    {
        error_message = "The passed path does not reference a file.";
        return descriptor_ptr();
    }

    ASSUMPTION( error_message.empty() );

    detail::load_props_pe  load_props{
            normalise_path(absolute_path(pe_file)),
            ignored_dynamic_link_files,
            search_directories_for_dynamic_link_files
            };

    std::ifstream  pe{pe_file,std::ifstream::binary};
    if (!pe.is_open())
    {
        error_message = "Cannot open the passed file.";
        return descriptor_ptr();
    }

    std::pair<file_props_ptr,detail::coff_standard_header_info> file_info =
            detail::load_pe_file_props(load_props.root_file(),pe,load_props,error_message);
    if (error_message == detail::NOT_PE_FILE())
    {
        error_message.clear();
        return descriptor_ptr();
    }
    if (!error_message.empty())
        return descriptor_ptr();

    error_message = file_info.first->num_address_bits() == 32U ?
                detail::load_pe_32bit(pe,file_info.first,file_info.second,load_props) :
                detail::load_pe_64bit(pe,file_info.first,file_info.second,load_props) ;
    if (!error_message.empty())
        return descriptor_ptr();

    return descriptor_ptr(
                new descriptor{
                        load_props.platform(),
                        load_props.sections_table(),
                        load_props.special_sections(),
                        load_props.entry_point(),
                        load_props.files_table(),
                        load_props.skipped_files(),
                        dependencies_graph_ptr(
                                new dependencies_graph {
                                        load_props.root_file(),
                                        detail::build_loaded_files_dependencies(
                                                *load_props.file_forward_dependencies(),
                                                *load_props.skipped_files()),
                                        detail::build_loaded_files_dependencies(
                                                *load_props.file_backward_dependencies(),
                                                *load_props.skipped_files())
                                        }
                                ),
                        dependencies_graph_ptr(
                                new dependencies_graph {
                                        load_props.root_file(),
                                        load_props.file_forward_dependencies(),
                                        load_props.file_backward_dependencies()
                                        }
                                ),
                        load_props.init_functions(),
                        load_props.fini_functions(),
                        load_props.performed_relocations(),
                        load_props.skipped_relocations(),
                        detail::build_visible_symbols_table(load_props),
                        symbol_table_ptr(),
                        load_props.warnings()
                        }
                );
}


descriptor_ptr  load_mach(std::string const&  mach_file,
                          std::vector<std::string> const&  ignored_dynamic_link_files,
                          std::vector<std::string> const&  search_directories_for_dynamic_link_files,
                          std::string& error_message)
{
    if (!file_exists(mach_file))
    {
        error_message = "The passed file does not exist.";
        return descriptor_ptr();
    }
    if (is_directory(mach_file))
    {
        error_message = "The passed path does not reference a file.";
        return descriptor_ptr();
    }

    ASSUMPTION( error_message.empty() );

    detail::load_props_mach  load_props{
            normalise_path(absolute_path(mach_file)),
            ignored_dynamic_link_files,
            search_directories_for_dynamic_link_files
            };

    std::ifstream  mach{mach_file,std::ifstream::binary};
    if (!mach.is_open())
    {
        error_message = "Cannot open the passed file.";
        return descriptor_ptr();
    }

    auto const  mach_props =
            detail::load_mach_file_props(load_props.root_file(),mach,load_props,error_message);
    if (error_message == detail::NOT_MACH_FILE())
    {
        error_message.clear();
        return descriptor_ptr();
    }
    if (!error_message.empty())
        return descriptor_ptr();

    error_message = mach_props.first->num_address_bits() == 32U ?
                detail::load_mach_32bit(mach,mach_props.first,
                                        std::get<0>(mach_props.second),std::get<1>(mach_props.second),
                                        std::get<2>(mach_props.second),
                                        load_props) :
                detail::load_mach_64bit(mach,mach_props.first,
                                        std::get<0>(mach_props.second),std::get<1>(mach_props.second),
                                        std::get<2>(mach_props.second),
                                        load_props) ;
    if (!error_message.empty())
        return descriptor_ptr();

    return descriptor_ptr(
                new descriptor{
                        load_props.platform(),
                        load_props.sections_table(),
                        load_props.special_sections(),
                        load_props.entry_point(),
                        load_props.files_table(),
                        load_props.skipped_files(),
                        dependencies_graph_ptr(
                                new dependencies_graph {
                                        load_props.root_file(),
                                        detail::build_loaded_files_dependencies(
                                                *load_props.file_forward_dependencies(),
                                                *load_props.skipped_files()),
                                        detail::build_loaded_files_dependencies(
                                                *load_props.file_backward_dependencies(),
                                                *load_props.skipped_files())
                                        }
                                ),
                        dependencies_graph_ptr(
                                new dependencies_graph {
                                        load_props.root_file(),
                                        load_props.file_forward_dependencies(),
                                        load_props.file_backward_dependencies()
                                        }
                                ),
                        load_props.init_functions(),
                        load_props.fini_functions(),
                        load_props.performed_relocations(),
                        load_props.skipped_relocations(),
                        detail::build_visible_symbols_table(load_props),
                        symbol_table_ptr(),
                        load_props.warnings()
                        }
                );
}

}
