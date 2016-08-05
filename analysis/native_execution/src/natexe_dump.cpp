#include <rebours/analysis/native_execution/dump.hpp>
#include <rebours/analysis/native_execution/file_utils.hpp>
#include <rebours/analysis/native_execution/execution_properties.hpp>
#include <rebours/analysis/native_execution/assumptions.hpp>
#include <rebours/analysis/native_execution/invariants.hpp>
#include <rebours/analysis/native_execution/msgstream.hpp>
#include <rebours/analysis/native_execution/development.hpp>
#include <rebours/program/assembly.hpp>
#include <map>
#include <set>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <tuple>

namespace analysis { namespace natexe { namespace detail {

static std::vector<std::string>  g_root_directory;
static std::vector< std::pair<std::string,  //!< A brief description of the dump
                              std::string>  //!< A path-name of the root file of the dump
                    >  g_dump_files;


static std::string  compute_relative_pathname(std::string const&  pathname, std::string const&  dir)
{
    return fileutl::get_relative_path(pathname,dir);
//    return fileutl::get_relative_path(fileutl::absolute_path(pathname),fileutl::absolute_path(dir));
//    std::string const  dir =
//            fileutl::get_relative_path(fileutl::absolute_path(pathname),fileutl::absolute_path(dir));
//    std::string const  name = fileutl::parse_name_in_pathname(pathname);
//    return fileutl::concatenate_file_paths(dir,name);
}

void  dump_html_prefix(std::ostream&  ostr)
{
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
}

void  dump_html_suffix(std::ostream&  ostr)
{
    ostr << "</body>\n";
    ostr << "</html>\n";
}


}}}

namespace analysis { namespace natexe {


bool  dump_enabled()
{
    return !detail::g_root_directory.empty();
}

std::string  dump_root_directory()
{
    if (!dump_enabled())
        return "";
    return detail::g_root_directory.front();
}

std::string  dump_directory()
{
    if (!dump_enabled())
        return "";
    return detail::g_root_directory.back();
}

void  dump_push_directory(std::string const&  new_dir)
{
    detail::g_root_directory.push_back(new_dir);
    fileutl::create_directory(new_dir);
}

void  dump_pop_directory()
{
    detail::g_root_directory.pop_back();
}


dump_push_guard::dump_push_guard(std::string const&  new_dir, bool const  allow_to_enable_dumping)
    : m_dir(new_dir)
{
    if ((allow_to_enable_dumping || dump_enabled()) && !dir().empty())
        dump_push_directory(new_dir);
}

dump_push_guard::~dump_push_guard()
{
    if (dump_enabled() && !dir().empty())
    {
        ASSUMPTION(dump_directory() == dir());
        dump_pop_directory();
    }
}


void  dump_add_file(std::string const&  description, std::string const&  pathname)
{
    if (!dump_enabled())
        return;
    if (!pathname.empty())
        detail::g_dump_files.push_back({description,pathname});
}

void  dump_create_root_file(std::string const&  error_message, std::string const&  dump_subdir)
{
    if (!dump_enabled())
        return;

    std::string const  root_dir = fileutl::concatenate_file_paths(dump_root_directory(),dump_subdir);
    std::string const  pathname = fileutl::concatenate_file_paths(root_dir,"start.html");

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump root file '" << pathname << "'.";
        return;
    }

    detail::dump_html_prefix(ostr);

    ostr << "<h1>Overview of generated log files</h1>\n";
    if (error_message.empty())
        ostr << "<p></p>\n";
    else
        ostr << "<p>The analysis has terminated with the following error message: " << error_message << "</p>\n";
    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Dump description</th>\n";
    ostr << "    <th>Link</th>\n";
    ostr << "  </tr>\n";

    for (auto const&  desc_file : detail::g_dump_files)
    {
        ostr << "  <tr>\n";
        ostr << "    <td>" << desc_file.first << "</td>\n";
        ostr << "    <td><a href=\"./" << detail::compute_relative_pathname(desc_file.second,root_dir) << "\">here</a></td>\n";
        ostr << "  </tr>\n";
    }

    ostr << "</table>\n";

    detail::dump_html_suffix(ostr);
}


}}

namespace analysis { namespace natexe {


std::string  dump_memory_allocations(memory_allocations const&  alloc, std::string const&  filename)
{
    std::string const  pathname = fileutl::concatenate_file_paths(dump_directory(),filename);

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump file of memory allocations '" << pathname << "'.";
        return pathname;
    }

    detail::dump_html_prefix(ostr);

    if (alloc.empty())
        ostr << "<p>No memory is allocated.</p>\n";
    else
    {
        ostr << "<p>All numbers are hexadecimal.</p>\n";
        ostr << "<table>\n";
        ostr << "  <tr>\n";
        ostr << "    <th>Start address</th>\n";
        ostr << "    <th>Num bytes</th>\n";
        ostr << "    <th>Readable</th>\n";
        ostr << "    <th>Writable</th>\n";
        ostr << "    <th>Executable</th>\n";
        ostr << "    <th>Big endian?</th>\n";
        ostr << "    <th>Mutable endian?</th>\n";
        ostr << "  </tr>\n";
        for (auto const&  adr_info : alloc)
        {
            ostr << "  <tr>\n";
            ostr << "    <td>" << std::hex << std::setw(12) << std::setfill('0') << adr_info.first << "</td>\n";
            ostr << "    <td>" << std::hex << adr_info.second.num_bytes() << "</td>\n";
            ostr << "    <td>" << std::boolalpha << adr_info.second.readable() << "</td>\n";
            ostr << "    <td>" << std::boolalpha << adr_info.second.writable() << "</td>\n";
            ostr << "    <td>" << std::boolalpha << adr_info.second.executable() << "</td>\n";
            ostr << "    <td>" << std::boolalpha << adr_info.second.is_in_big_endian() << "</td>\n";
            ostr << "    <td>" << std::boolalpha << adr_info.second.has_mutable_endian() << "</td>\n";
            ostr << "  </tr>\n";
        }
        ostr << "</table>\n";
    }

    detail::dump_html_suffix(ostr);

    return pathname;
}

std::string  dump_memory_content(memory_content const&  content, std::string const&  filename, memory_allocations const* const  alloc)
{
    (void)alloc;

    std::string const  pathname = fileutl::concatenate_file_paths(dump_directory(),filename);

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump file of memory content '" << pathname << "'.";
        return pathname;
    }

    detail::dump_html_prefix(ostr);

    ostr << "<h2>Listing of memory content</h2>\n";
    ostr << "<p>Number of accessed memory pages: " << content.size() << "</p>\n";
    ostr << "<p>Only accessed memory pages are listed. All number are hexadecimal.</p>\n";
    if (!content.empty())
    {
        ostr << "<table>\n";
        ostr << "  <caption>List of pages</caption>\n";
        ostr << "  <tr>\n";
        ostr << "    <th>Page</th>\n";
        ostr << "    <th>Content</th>\n";
        ostr << "  </tr>\n";
        for (auto const&  adr_page : content)
        {
            ostr << "  <tr>\n";
            ostr << "    <td>["
                 << std::hex << std::setw(12) << std::setfill('0') << adr_page.first
                 << ","
                 << std::hex << std::setw(12) << std::setfill('0') << adr_page.first + adr_page.second.size()
                 << ")</td>\n";
            ostr << "    <td><a href=\"./" << filename << "#page_"
                 << std::hex << std::setw(12) << std::setfill('0') << adr_page.first
                 << "\">here</a></td>\n";
            ostr << "  </tr>\n";
        }
        ostr << "</table>\n";
        ostr << "<pre>\n\n</pre>\n";
        for (auto const&  adr_page : content)
        {
            INVARIANT(adr_page.second.size() % 16ULL == 0ULL);

            byte const*  ptr = adr_page.second.data();
            byte const* const  end = ptr + adr_page.second.size();
            address  adr = adr_page.first;

            ostr << "<h3 id=page_"
                 << std::hex << std::setw(12) << std::setfill('0') << adr
                 << ">Page ["
                 << std::hex << std::setw(12) << std::setfill('0') << adr
                 << ","
                 << std::hex << std::setw(12) << std::setfill('0') << adr + adr_page.second.size()
                 << ")</h3>\n";

            ostr << "<pre>\n";
            for ( ; ptr != end; adr += 16ULL)
            {
                std::string  text;
                ostr << std::hex << std::setw(12) << std::setfill('0') << adr << "   ";
                for (int i = 0; i < 16; ++i, ++ptr)
                {
                    if (i == 8)
                    {
                        ostr << ' ';
                        text.push_back(' ');
                    }
                    ostr << ' ' << std::hex << std::setw(2) << std::setfill('0') << (uint32_t)*ptr;
                    if (*ptr == '<')
                        text.append("&lt;");
                    else if (std::isprint(*ptr))
                        text.push_back(*ptr);
                    else
                        text.push_back('.');
                }
                ostr << "    " << text << '\n';
            }
            ostr << '\n';
            ostr << "</pre>\n";
        }
    }

    detail::dump_html_suffix(ostr);

    return pathname;
}

std::string  dump_content_of_registers(memory_content const&  content,
                                       std::map<std::pair<uint64_t,uint64_t>,std::string> const&  ranges_to_registers,
                                       std::string const&  filename)
{
    std::string const  pathname = fileutl::concatenate_file_paths(dump_directory(),filename);

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump file of content of registers '" << pathname << "'.";
        return pathname;
    }

    detail::dump_html_prefix(ostr);

    ostr << "<h2>Contents of CPU registers</h2>\n";
    ostr << "<p>All number are hexadecimal.</p>\n";
    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Register</th>\n";
    ostr << "    <th>Value</th>\n";
    ostr << "    <th>Address</th>\n";
    ostr << "    <th>Size</th>\n";
    ostr << "  </tr>\n";

    for (auto const& elem : ranges_to_registers)
    {
        uint64_t const  adr_begin = elem.first.first;
        uint64_t const  adr_end = elem.first.second;
        uint64_t const  reg_size = adr_end - adr_begin;
        std::string const&  reg_name = elem.second;

        ostr << "  <tr>\n";
        ostr << "    <td>" << reg_name << "</td>\n";
        ostr << "    <td>" << std::hex << std::setw(2ULL * reg_size) << std::setfill('0');
        if (reg_size <= sizeof(uint64_t))
            ostr << memory_read<uint64_t>(content,adr_begin,reg_size,true);
        else
            ostr << memory_read<uint128_t>(content,adr_begin,true);
        ostr << "</td>\n";
        ostr << "    <td>" << std::hex << adr_begin <<  "</td>\n";
        ostr << "    <td>" << std::hex << reg_size <<  "</td>\n";
        ostr << "  </tr>\n";
    }

    ostr << "</table>\n";
    detail::dump_html_suffix(ostr);

    return pathname;
}

std::string  dump_stream_content(stream_content const&  content, std::string const&  filename, stream_allocations const* const  alloc)
{
    (void)alloc;

    std::string const  pathname = fileutl::concatenate_file_paths(dump_directory(),filename);

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump file of stream content '" << pathname << "'.";
        return pathname;
    }

    detail::dump_html_prefix(ostr);

    ostr << "<h2>Listing of stream's content</h2>\n";
    ostr << "<p>Number of accessed pages: " << content.size() << "</p>\n";
    ostr << "<p>Only accessed pages are listed. All number are hexadecimal.</p>\n";
    ostr << "<table>\n";
    ostr << "  <caption>List of pages</caption>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Page</th>\n";
    ostr << "    <th>Content</th>\n";
    ostr << "  </tr>\n";
    for (auto const&  index_page : content)
    {
        ostr << "  <tr>\n";
        ostr << "    <td>["
             << std::hex << std::setw(8) << std::setfill('0') << index_page.first
             << ","
             << std::hex << std::setw(8) << std::setfill('0') << index_page.first + index_page.second.size()
             << ")</td>\n";
        ostr << "    <td><a href=\"./" << filename << "#page_"
             << std::hex << std::setw(8) << std::setfill('0') << index_page.first
             << "\">here</a></td>\n";
        ostr << "  </tr>\n";
    }
    ostr << "</table>\n";
    ostr << "<pre>\n\n</pre>\n";
    for (auto const&  index_page : content)
    {
        INVARIANT(index_page.second.size() % 16ULL == 0ULL);

        byte const*  ptr = index_page.second.data();
        byte const* const  end = ptr + index_page.second.size();
        address  adr = index_page.first;

        ostr << "<h3 id=page_"
             << std::hex << std::setw(8) << std::setfill('0') << adr
             << ">Page ["
             << std::hex << std::setw(8) << std::setfill('0') << adr
             << ","
             << std::hex << std::setw(8) << std::setfill('0') << adr + index_page.second.size()
             << ")</h3>\n";

        ostr << "<pre>\n";
        for ( ; ptr != end; adr += 16ULL)
        {
            std::string  text;
            ostr << std::hex << std::setw(8) << std::setfill('0') << adr << "   ";
            for (int i = 0; i < 16; ++i, ++ptr)
            {
                if (i == 8)
                {
                    ostr << ' ';
                    text.push_back(' ');
                }
                ostr << ' ' << std::hex << std::setw(2) << std::setfill('0') << (uint32_t)*ptr;
                if (std::isprint(*ptr))
                    text.push_back(*ptr);
                else
                    text.push_back('.');
            }
            ostr << "    " << text << '\n';
        }
        ostr << '\n';
        ostr << "</pre>\n";
    }

    detail::dump_html_suffix(ostr);

    return pathname;
}

std::string  dump_streams(stream_allocations const&  alloc, contents_of_streams const&  contents, std::string const&  filename)
{
    std::string const  pathname = fileutl::concatenate_file_paths(dump_directory(),filename);

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump file of stream allocations '" << pathname << "'.";
        return pathname;
    }

    detail::dump_html_prefix(ostr);

    ostr << "<h2>Dump of streams</h2>\n";
    if (alloc.empty())
        ostr << "<p>No stream is allocated.</p>\n";
    else
    {
        ASSUMPTION(
            [](stream_allocations const&  alloc, contents_of_streams const&  contents) {
                for (auto const& id_pages : contents)
                    if (alloc.count(id_pages.first) == 0ULL)
                        return false;
                return true;
            }(alloc,contents)
            );
        ostr << "<p>All numbers are hexadecimal.</p>\n";
        ostr << "<table>\n";
        ostr << "  <tr>\n";
        ostr << "    <th>Stream ID</th>\n";
        ostr << "    <th>Is open</th>\n";
        ostr << "    <th>Readable</th>\n";
        ostr << "    <th>Writable</th>\n";
        ostr << "    <th>Seekable</th>\n";
        ostr << "    <th>Size</th>\n";
        ostr << "    <th>Cursor position</th>\n";
        ostr << "    <th>Content</th>\n";
        ostr << "  </tr>\n";
        uint64_t  counter = 0ULL;
        for (auto const&  id_info : alloc)
        {
            ostr << "  <tr>\n";
            ostr << "    <td>" << std::hex << id_info.first << "</td>\n";
            ostr << "    <td>" << std::boolalpha << id_info.second.is_open() << "</td>\n";
            ostr << "    <td>" << std::boolalpha << id_info.second.readable() << "</td>\n";
            ostr << "    <td>" << std::boolalpha << id_info.second.writable() << "</td>\n";
            ostr << "    <td>" << std::boolalpha << id_info.second.seekable() << "</td>\n";
            ostr << "    <td>" << std::boolalpha << id_info.second.size() << "</td>\n";
            ostr << "    <td>" << std::boolalpha << id_info.second.cursor() << "</td>\n";
            if (contents.count(id_info.first) != 0ULL)
                ostr << "    <td><a href=\"./"
                     << detail::compute_relative_pathname(dump_stream_content(contents.at(id_info.first),
                                                                              msgstream() << "stream_content_" << counter << ".html",
                                                                              &alloc),
                                                          dump_directory())
                     << "\">here</a></td>\n";
            else
                ostr << "    <td>EMPTY</td>\n";

            ostr << "  </tr>\n";
            ++counter;
        }
        ostr << "</table>\n";
    }

    detail::dump_html_suffix(ostr);

    return pathname;
}

std::string  dump_interrupts(interrupts_vector_table const&  ivt, std::string const&  filename)
{
    std::string const  pathname = fileutl::concatenate_file_paths(dump_directory(),filename);

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump file of interrupts vector table '" << pathname << "'.";
        return pathname;
    }

    detail::dump_html_prefix(ostr);

    ostr << "<h2>Interrupts vector table</h2>\n";
    ostr << "<p>Number of initialised elements: " << ivt.size() << "</p>\n";
    ostr << "<p>Only initialised elements are listed. All number are hexadecimal.</p>\n";
    if (!ivt.empty())
    {
        ostr << "<table>\n";
        ostr << "  <tr>\n";
        ostr << "    <th>Index</th>\n";
        ostr << "    <th>Address</th>\n";
        ostr << "  </tr>\n";
        std::map<index,address> const  sorted(ivt.cbegin(),ivt.cend());
        for (auto const&  index_address : sorted)
        {
            ostr << "  <tr>\n";
            ostr << "    <td>" << index_address.first << "</td>\n";
            ostr << "    <td>" << index_address.second << "</td>\n";
            ostr << "  </tr>\n";
        }
        ostr << "</table>\n";
    }

    detail::dump_html_suffix(ostr);

    return pathname;
}

//std::string  dump_map_from_instructions_to_edges(instructions_to_edges_map const&  i2e, std::string const&  filename)
//{
//    std::string const  pathname = fileutl::concatenate_file_paths(dump_directory(),filename);

//    std::fstream  ostr(pathname, std::ios_base::out);
//    if (!ostr.is_open())
//    {
//        std::cerr << "natexe: Cannot open the dump file of interrupts vector table '" << pathname << "'.";
//        return pathname;
//    }

//    detail::dump_html_prefix(ostr);

//    ostr << "<h2>Map from instructions to edges</h2>\n";
//    ostr << "<p>Numbers in columns 'GIK' and 'Edges' are decimal.</p>\n";
//    ostr << "<table>\n";
//    ostr << "  <tr>\n";
//    ostr << "    <th>GIK</th>\n";
//    ostr << "    <th>Assembly</th>\n";
//    ostr << "    <th style=\"width:430px\">Edges</th>\n";
//    ostr << "  </tr>\n";

//    std::multimap<microcode::GIK,microcode::instruction>  sorted;
//    for (auto const& i_e : i2e)
//        sorted.insert({i_e.first.GIK(),i_e.first});
//    for (auto const& g_i : sorted)
//    {
//        ostr << "  <tr>\n";
//        ostr << "    <td>" << (uint32_t)g_i.first << "</td>\n";
//        ostr << "    <td>" << microcode::assembly_text(g_i.second) << "</td>\n";
//        ostr << "    <td>";
//        std::set<edge_id>  sorted_edges;
//        for (edge_id const& id : i2e.at(g_i.second))
//            sorted_edges.insert(id);
//        for (edge_id const&  id : sorted_edges)
//            ostr << "{" << id.first << "," << id.second << "}, ";
//        ostr << "    </td>\n";
//        ostr << "  </tr>\n";
//    }

//    ostr << "</table>\n";

//    detail::dump_html_suffix(ostr);

//    return pathname;
//}

//std::string  dump_probe_properties(
//        probe_properties const&  pprops,
//        microcode::program const&  P
//        )
//{
//    (void)P;
//    std::string const pathname = fileutl::concatenate_file_paths(dump_directory(),"start.html");

//    std::fstream  ostr(pathname, std::ios_base::out);
//    if (!ostr.is_open())
//    {
//        std::cerr << "natexe: Cannot open the dump file for probe properties '" << pathname << "'.";
//        return pathname;
//    }

//    detail::dump_html_prefix(ostr);

//    ostr << "<h2>Probe properties</h2>\n";
//    ostr << "<p></p>\n";
//    ostr << "<table>\n";
//    ostr << "  <tr>\n";
//    ostr << "    <th>Property</th>\n";
//    ostr << "    <th>Value</th>\n";
//    ostr << "  </tr>\n";
//    ostr << "  <tr>\n";
//    ostr << "    <td>Map from instructions to edges</td>\n";
//    ostr << "    <td><a href=\"./"
//         << detail::compute_relative_pathname(dump_map_from_instructions_to_edges(pprops.instructions_to_edges(),msgstream() << "instructions_to_edges.html"),dump_directory())
//         << "\">here</a></td>\n";
//    ostr << "  </tr>\n";

//    ostr << "</table>\n";

//    detail::dump_html_suffix(ostr);

//    return pathname;
//}


std::string  dump_control_switches(
        std::unordered_map<node_id,switch_info> const&  switches,
        important_code_ranges const&  important_code,
        std::string const&  filename)
{
    std::string const  pathname = fileutl::concatenate_file_paths(dump_directory(),filename);

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump file of control switches '" << pathname << "'.";
        return pathname;
    }

    detail::dump_html_prefix(ostr);

    std::vector<std::string> const  titles = { "Primary control switches", "Secondary control switches", };
    for (int i = 0; i < 2; ++i)
    {
        ostr << "<h2>" << titles.at(i) << "</h2>\n";

        std::set<node_id>  sorted;
        for (auto const& u_i : switches)
            for (auto const& a_u : u_i.second.cases())
                if ((i == 0 && is_inside_important_code(a_u.first,important_code)) ||
                    (i != 0 && !is_inside_important_code(a_u.first,important_code)) )
                {
                    sorted.insert(u_i.first);
                    break;
                }

        if (sorted.empty())
            ostr << "<p>No control switch has been found.</p>\n";
        else
        {
            ostr << "<p>Numbers representing first components in 'Cases' are hexadecimal and all other numbers are adecimal.</p>\n";
            ostr << "<table>\n";
            ostr << "  <tr>\n";
            ostr << "    <th>Head</th>\n";
            ostr << "    <th>Default</th>\n";
            ostr << "    <th style=\"width:430px\">Cases (addres,node)</th>\n";
            ostr << "  </tr>\n";
            for (auto const u : sorted)
            {
                switch_info const&  info = switches.at(u);
                ostr << "  <tr>\n";
                ostr << "    <td>" << std::dec << info.head_node() << "</td>\n";
                ostr << "    <td>" << std::dec << info.default_node() << "</td>\n";
                ostr << "    <td>";
                for (auto const&  adr_node : info.cases())
                    ostr << "(" << std::hex << adr_node.first << "," << std::dec << adr_node.second << "), ";
                ostr << "    </td>\n";
                ostr << "  </tr>\n";
            }
            ostr << "</table>\n";
        }
    }

    detail::dump_html_suffix(ostr);

    return pathname;
}

std::string  dump_unexplored_exits(
        std::unordered_map<node_id,unexplored_info> const&  unexplored,
        important_code_ranges const&  important_code,
        std::string const&  filename)
{
    std::string const  pathname = fileutl::concatenate_file_paths(dump_directory(),filename);

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump file of unexplored exits '" << pathname << "'.";
        return pathname;
    }

    detail::dump_html_prefix(ostr);

    std::vector<std::string> const  titles = { "Primary unexplored exits", "Secondary unexplored exits", };
    for (int i = 0; i < 2; ++i)
    {
        ostr << "<h2>" << titles.at(i) << "</h2>\n";

        std::set<node_id>  sorted;
        for (auto const& u_i : unexplored)
            if ((i == 0 && is_inside_important_code(u_i.second.IP(),important_code)) ||
                (i != 0 && !is_inside_important_code(u_i.second.IP(),important_code)) )
                sorted.insert(u_i.first);

        if (sorted.empty())
            ostr << "<p>No unexplored exit has been found.</p>\n";
        else
        {
            ostr << "<p>Numbers in 'Node' are decimal. Numbers in 'IP' are hexadecimal.</p>\n";
            ostr << "<table>\n";
            ostr << "  <tr>\n";
            ostr << "    <th>Node</th>\n";
            ostr << "    <th>IP</th>\n";
            ostr << "  </tr>\n";

            for (auto const u : sorted)
            {
                unexplored_info const&  info = unexplored.at(u);
                (void)info;
                ostr << "  <tr>\n";
                ostr << "    <td>" << std::dec << info.node() << "</td>\n";
                ostr << "    <td>" << std::hex << info.IP() << "</td>\n";
                ostr << "  </tr>\n";
            }

            ostr << "</table>\n";
        }
    }

    detail::dump_html_suffix(ostr);

    return pathname;
}

std::string  dump_nodes_history_of_one_executed_thread(
        std::vector<node_id> const&  nodes,
        std::string const&  filename)
{
    std::string const  pathname = fileutl::concatenate_file_paths(dump_directory(),filename);

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump file of nodes history of an executed thread '" << pathname << "'.";
        return pathname;
    }

    detail::dump_html_prefix(ostr);

    ostr << "<h3>List of all nodes visited during execution of the thread.</h3>\n";
    ostr << "<p>The 'Counter' is an index into the list of 'Nodes'.</p>\n";
    ostr << "<pre>\nCounter      Nodes\n\n";
    index  i = 0ULL,j = 0ULL;
    for (node_id const&  id : nodes)
    {
        if (j % 10ULL == 0ULL)
        {
            ostr << (i != 0ULL ? "\n" : "") << std::setw(7) << std::setfill('0') << i << "      ";
            j = 0ULL;
        }

        ostr << id << ", ";

        ++j;
        ++i;
    }
    ostr << "</pre>\n";

    detail::dump_html_suffix(ostr);

    return pathname;
}

std::string  dump_node_histories_of_executed_threads(
        std::vector<nodes_history_of_threads> const&  node_histories,
        std::string const&  filename)
{
    std::string const  pathname = fileutl::concatenate_file_paths(dump_directory(),filename);

    std::string const  pure_filename = fileutl::remove_extension(filename);

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump file of node history of executed threads '" << pathname << "'.";
        return pathname;
    }

    detail::dump_html_prefix(ostr);

    ostr << "<h2>Node history of executed threads</h2>\n";
    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Execution</th>\n";
    ostr << "    <th>Thread</th>\n";
    ostr << "  </tr>\n";

    for (index  i = 0ULL; i < node_histories.size(); ++i)
    {
        std::set<thread_id>  sorted;
        for (auto const& tid_nodes : node_histories.at(i))
            sorted.insert(tid_nodes.first);
        if (!sorted.empty())
        {
            ostr << "  <tr>\n";
            ostr << "    <td>" << std::setw(6) << std::setfill('0') << i << "</td>\n";
            for (thread_id const tid : sorted)
            {
                ostr << "    <td><a href=\"./"
                     << detail::compute_relative_pathname(
                                dump_nodes_history_of_one_executed_thread(node_histories.at(i).at(tid),
                                                                          msgstream() << pure_filename << "_" << i << "_" << tid << ".html"),
                                dump_directory()
                                )
                     << "\">" << std::setw(3) << std::setfill('0') << tid << "</a></td>\n";
            }
            ostr << "  </tr>\n";
        }
    }

    ostr << "</table>\n";

    detail::dump_html_suffix(ostr);

    return pathname;
}

std::string  dump_branchings_of_one_executed_thread(
        std::vector<node_counter_type> const&  counters,
        std::vector<node_id> const&  nodes,
        std::string const&  filename)
{
    std::string const  pathname = fileutl::concatenate_file_paths(dump_directory(),filename);

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump file of branchings of an executed thread '" << pathname << "'.";
        return pathname;
    }

    detail::dump_html_prefix(ostr);

    ostr << "<h3>List of all branchings taken during execution of the thread.</h3>\n";
    ostr << "<pre>\n";
    index  j = 0ULL;
    for (node_counter_type const&  counter : counters)
    {
        ASSUMPTION(nodes.size() > counter + 1ULL);
        ostr << "{" << nodes.at(counter) << "," << nodes.at(counter + 1ULL) << "}, ";
        if (++j % 10ULL == 0ULL)
        {
            j = 0ULL;
            ostr << "\n";
        }
    }
    ostr << "</pre>\n";

    detail::dump_html_suffix(ostr);

    return pathname;
}

std::string  dump_branchings_of_executed_threads(
        std::vector<branchings_of_threads> const&  branchings,
        std::vector<nodes_history_of_threads> const&  node_histories,
        std::string const&  filename)
{
    ASSUMPTION(branchings.size() == node_histories.size());

    std::string const  pathname = fileutl::concatenate_file_paths(dump_directory(),filename);

    std::string const  pure_filename = fileutl::remove_extension(filename);

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump file of branchings of executed threads '" << pathname << "'.";
        return pathname;
    }

    detail::dump_html_prefix(ostr);

    ostr << "<h2>Branchings of executed threads</h2>\n";
    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Execution</th>\n";
    ostr << "    <th>Thread</th>\n";
    ostr << "  </tr>\n";

    for (index  i = 0ULL; i < branchings.size(); ++i)
    {
        std::set<thread_id>  sorted;
        for (auto const& tid_edges : branchings.at(i))
            sorted.insert(tid_edges.first);
        if (!sorted.empty())
        {
            ostr << "  <tr>\n";
            ostr << "    <td>" << std::setw(6) << std::setfill('0') << i << "</td>\n";
            for (thread_id const tid : sorted)
            {
                ostr << "    <td><a href=\"./"
                     << detail::compute_relative_pathname(
                                dump_branchings_of_one_executed_thread(branchings.at(i).at(tid),node_histories.at(i).at(tid),
                                                                       msgstream() << pure_filename << "_" << i << "_" << tid << ".html"),
                                dump_directory()
                                )
                     << "\">" << std::setw(3) << std::setfill('0') << tid << "</a></td>\n";
            }
            ostr << "  </tr>\n";
        }
    }

    ostr << "</table>\n";

    detail::dump_html_suffix(ostr);

    return pathname;
}

std::string  dump_visited_branchings(
        std::unordered_set<edge_id> const&  branchings,
        std::string const&  filename)
{
    std::string const  pathname = fileutl::concatenate_file_paths(dump_directory(),filename);

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump file of visited branchings '" << pathname << "'.";
        return pathname;
    }

    detail::dump_html_prefix(ostr);

    ostr << "<h3>List of all branchings visited in the recovered program.</h3>\n";
    ostr << "<pre>\n";
    std::set<edge_id>  sorted;
    for (auto const& eid : branchings)
        sorted.insert(eid);
    index  j = 0ULL;
    for (edge_id const&  eid : sorted)
    {
        ostr << "{" << eid.first << "," << eid.second << "}, ";
        if (++j % 10ULL == 0ULL)
        {
            j = 0ULL;
            ostr << "\n";
        }
    }
    ostr << "</pre>\n";

    detail::dump_html_suffix(ostr);

    return pathname;
}

std::string  dump_threads_of_performed_executions(
        recovery_properties const&  rprops,
        std::string const&  filename)
{
    std::string const  pathname = fileutl::concatenate_file_paths(dump_directory(),filename);

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump file of threads of performed executions '" << pathname << "'.";
        return pathname;
    }

    detail::dump_html_prefix(ostr);

    ostr << "<h2>Threads of performed executions</h2>\n";
    ostr << "<p>\n"
            "For each performed execution we provide a list of executed threads.\n"
            "All numbers are decimal.\n"
            "</p>\n"
            ;

    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Execution</th>\n";
    ostr << "    <th>#</th>\n";
    ostr << "    <th style=\"width:430px\">Threads</th>\n";
    ostr << "  </tr>\n";

    for (index  eid = 0ULL; eid < rprops.num_executions_performed(); ++eid)
    {
        std::vector<thread_id> const&  thds = rprops.threads_of_execution(eid);

        ostr << "  <tr>\n";
        ostr << "    <td>" << std::setw(6) << std::setfill('0') << std::dec << eid << "</td>\n";
        ostr << "    <td>" << thds.size() << "</td>\n";
        ostr << "    <td>\n";
        for (index  i = 0ULL; i < thds.size(); ++i)
            ostr << thds.at(i) << (i + 1ULL != thds.size() ? ", " : "");
        ostr << "</td>\n";
        ostr << "  </tr>\n";
    }

    ostr << "</table>\n";

    detail::dump_html_suffix(ostr);

    return pathname;
}

std::string  dump_interleaving_of_threads_of_one_execution(
        recovery_properties const&  rprops,
        execution_id const  eid,
        std::string const&  filename)
{
    std::string const  pathname = fileutl::concatenate_file_paths(dump_directory(),filename);

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump file of nodes history of an executed thread '" << pathname << "'.";
        return pathname;
    }

    detail::dump_html_prefix(ostr);

    ostr << "<h3>Interleaving of threads</h3>\n";
    ostr << "<p>\n"
            "The table bellow represents a map from execution steps (column 'Counter')\n"
            "to a theads operating at those steps (column 'Thread IDs'). Thread IDs\n"
            "starting with the symbol '*' represent begins of concurrency groups of\n"
            "threads (a group ends at the start of the next one). All numbers bellow are decimal."
            "</p>\n"
            "<pre>\nCounter      Thread IDs\n\n"
            ;
    std::vector<thread_id> const&  interleaving = rprops.interleaving_of_threads(eid);
    for (index  i = 0ULL, j = 0ULL; i < interleaving.size(); ++i, ++j)
    {
        if (j % 10ULL == 0ULL)
        {
            ostr << (i != 0ULL ? "\n" : "") << std::setw(7) << std::setfill('0') << i << "      ";
            j = 0ULL;
        }
        ostr << (rprops.is_begin_of_concurrent_group(eid,i) ? '*' : ' ') << interleaving.at(i) << ", ";
    }
    ostr << "</pre>\n";

    detail::dump_html_suffix(ostr);

    return pathname;
}

std::string  dump_interleaving_of_threads(
        recovery_properties const&  rprops,
        std::string const&  filename)
{
    std::string const  pathname = fileutl::concatenate_file_paths(dump_directory(),filename);

    std::string const  pure_filename = fileutl::remove_extension(filename);

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump file of interleaving of threads '" << pathname << "'.";
        return pathname;
    }

    detail::dump_html_prefix(ostr);

    ostr << "<h2>Interleaving of threads</h2>\n";
    ostr << "<p>\n"
            "For each performed execution we provide a link to the interleaving data.\n"
            "All numbers (execution IDs) are decimal.\n"
            "</p>\n"
            ;

    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th style=\"width:430px\">Executions</th>\n";
    ostr << "  </tr>\n";

    for (index  eid = 0ULL; eid < rprops.num_executions_performed(); ++eid)
    {
        ostr << "  <tr>\n";
        ostr << "    <td><a href=\"./"
             << detail::compute_relative_pathname(
                        dump_interleaving_of_threads_of_one_execution(rprops,eid,msgstream() << pure_filename << "_" << std::setw(6) << std::setfill('0') << std::dec << eid << ".html"),
                        dump_directory()
                        )
             << "\">" << std::setw(6) << std::setfill('0') << std::dec << eid << "</a></td>\n";
        ostr << "  </tr>\n";
    }

    ostr << "</table>\n";

    detail::dump_html_suffix(ostr);

    return pathname;
}


std::string  dump_input_impacts_of_one_executed_thread(
        std::vector<std::vector<input_impact_value> > const&  impacts,
        std::vector<node_id> const&  nodes_history,
        std::string const&  filename)
{
    std::string const  pathname = fileutl::concatenate_file_paths(dump_directory(),filename);

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump file of input impacts of an executed thread '" << pathname << "'.";
        return pathname;
    }

    detail::dump_html_prefix(ostr);

    ostr << "<h3>Input impacts of the executed thread.</h3>\n";
    ostr << "<p>\n"
            "The column 'Counter' shows indices to nodes history visited by the thread during its execution.\n"
            "'Index' holds indices into a list of input-dependent values all written at the same history node.\n"
            "In 'Storage' there can be eiter 'REG', 'MEM', or stream identifier.\n"
            "'Shift' shows shifts (offsets) from the begining of the storage.\n"
            "'Links' holds a list of pairs (Counter,Index) referencing some preceeding record in this table on which the current recond depends on.\n"
            "Numbers in the column 'Value' and 'Shift' are hexadecimal, all others are decimal.\n"
            "</p>\n"
            ;
//    ostr << "<pre>\n";
//    ostr << "</pre>\n";
    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Counter</th>\n";
    ostr << "    <th>Index</th>\n";
    ostr << "    <th>Node</th>\n";
    ostr << "    <th>Value</th>\n";
    ostr << "    <th>Storage</th>\n";
    ostr << "    <th>Shift</th>\n";
    ostr << "    <th style=\"width:430px\">Links</th>\n";
    ostr << "  </tr>\n";

    for (index  counter = 0ULL; counter < impacts.size(); ++counter)
        if (!impacts.at(counter).empty())
        {
            ostr << "  <tr>\n";
            ostr << "    <td rowspan=\"" << impacts.at(counter).size() << "\">" << std::dec << counter << "</td>\n";
            for (index  i = 0ULL; i < impacts.at(counter).size(); ++i)
            {
                if (i != 0ULL)
                    ostr << "  <tr>\n";

                ostr << "    <td>" << std::dec << i << "</td>\n";

                ASSUMPTION(counter < nodes_history.size());
                ostr << "    <td>" << std::dec << nodes_history.at(counter) << "</td>\n";

                input_impact_value const&  value = impacts.at(counter).at(i);

                ostr << "    <td>" << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)value.value() << "</td>\n";

                if (value.is_in_reg_pool())
                    ostr << "    <td>REG</td>\n";
                else if (value.is_in_mem_pool())
                    ostr << "    <td>MEM</td>\n";
                else
                    ostr << "    <td>" << value.stream() << "</td>\n";

                ostr << "    <td>" << std::hex << value.shift_from_begin() << "</td>\n";

                ostr << "    <td>";
                if (value.links().empty())
                    ostr << "NONE";
                else
                    for (input_impact_link const&  link : value.links())
                        ostr << "(" << std::dec << link.first << "," << std::dec << link.second << "), ";
                ostr << "    </td>\n";

                ostr << "  </tr>\n";
            }
        }

    ostr << "</table>\n";

    detail::dump_html_suffix(ostr);

    return pathname;
}

std::string  dump_input_impacts_of_executed_threads(
        std::vector<input_impacts_of_threads> const&  impacts,
        std::vector<nodes_history_of_threads> const&  node_histories,
        std::string const&  filename)
{
    std::string const  pathname = fileutl::concatenate_file_paths(dump_directory(),filename);

    std::string const  pure_filename = fileutl::remove_extension(filename);

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump file of input impacts of executed threads '" << pathname << "'.";
        return pathname;
    }

    detail::dump_html_prefix(ostr);

    ostr << "<h2>Input impacts of executed threads</h2>\n";
    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Execution</th>\n";
    ostr << "    <th>Thread</th>\n";
    ostr << "  </tr>\n";

    for (index  i = 0ULL; i < impacts.size(); ++i)
    {
        std::set<thread_id>  sorted;
        for (auto const& tid_edges : impacts.at(i))
            sorted.insert(tid_edges.first);
        if (!sorted.empty())
        {
            ostr << "  <tr>\n";
            ostr << "    <td>" << std::setw(6) << std::setfill('0') << i << "</td>\n";
            for (thread_id const tid : sorted)
            {
                ostr << "    <td><a href=\"./"
                     << detail::compute_relative_pathname(
                                dump_input_impacts_of_one_executed_thread(impacts.at(i).at(tid),node_histories.at(i).at(tid),
                                                                          msgstream() << pure_filename << "_" << i << "_" << tid << ".html"),
                                dump_directory()
                                )
                     << "\">" << std::setw(3) << std::setfill('0') << tid << "</a></td>\n";
            }
            ostr << "  </tr>\n";
        }
    }

    ostr << "</table>\n";

    detail::dump_html_suffix(ostr);

    return pathname;
}

std::string  dump_input_impact_links_of_one_executed_thread(
        std::vector<std::unordered_set<input_impact_link> > const&  links,
        std::vector<node_id> const&  nodes_history,
        std::string const&  filename)
{
    std::string const  pathname = fileutl::concatenate_file_paths(dump_directory(),filename);

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump file of input impact links of an executed thread '" << pathname << "'.";
        return pathname;
    }

    detail::dump_html_prefix(ostr);

    ostr << "<h3>Input impact links of the executed thread.</h3>\n";
    ostr << "<p>\n"
            "The column 'Counter' shows indices to nodes history visited by the thread during its execution.\n"
            "'Node' shows corresponding program nodes.\n"
            "The column 'Links' holds a list of pairs (Counter,Index) referencing record in the corresponding 'input impacts' table of the current thread.\n"
            "All numbers are decimal.\n"
            "</p>\n"
            ;
    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Counter</th>\n";
    ostr << "    <th>Node</th>\n";
    ostr << "    <th style=\"width:430px\">Links</th>\n";
    ostr << "  </tr>\n";

    for (index  counter = 0ULL; counter < links.size(); ++counter)
        if (!links.at(counter).empty())
        {
            ostr << "  <tr>\n";
            ostr << "    <td>" << std::dec << counter << "</td>\n";
            ASSUMPTION(counter < nodes_history.size());
            ostr << "    <td>" << std::dec << nodes_history.at(counter) << "</td>\n";

            ostr << "    <td>\n";
            std::set<input_impact_link>  sorted;
            for (auto const& cnt_idx : links.at(counter))
                sorted.insert(cnt_idx);
            for (input_impact_link const&  link : sorted)
                ostr << "(" << std::dec << link.first << "," << std::dec << link.second << "), ";
            ostr << "    </td>\n";

            ostr << "  </tr>\n";
        }

    ostr << "</table>\n";

    detail::dump_html_suffix(ostr);

    return pathname;
}

std::string  dump_input_impact_links_of_executed_threads(
        std::vector<input_impact_links_of_threads> const&  links,
        std::vector<nodes_history_of_threads> const&  node_histories,
        std::string const&  filename)
{
    std::string const  pathname = fileutl::concatenate_file_paths(dump_directory(),filename);

    std::string const  pure_filename = fileutl::remove_extension(filename);

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump file of input impact links of executed threads '" << pathname << "'.";
        return pathname;
    }

    detail::dump_html_prefix(ostr);

    ostr << "<h2>Input impact links of executed threads</h2>\n";
    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Execution</th>\n";
    ostr << "    <th>Thread</th>\n";
    ostr << "  </tr>\n";

    for (index  i = 0ULL; i < links.size(); ++i)
    {
        std::set<thread_id>  sorted;
        for (auto const& tid_edges : links.at(i))
            sorted.insert(tid_edges.first);
        if (!sorted.empty())
        {
            ostr << "  <tr>\n";
            ostr << "    <td>" << std::setw(6) << std::setfill('0') << i << "</td>\n";
            for (thread_id const tid : sorted)
            {
                ostr << "    <td><a href=\"./"
                     << detail::compute_relative_pathname(
                                dump_input_impact_links_of_one_executed_thread(links.at(i).at(tid),node_histories.at(i).at(tid),
                                                                               msgstream() << pure_filename << "_" << i << "_" << tid << ".html"),
                                dump_directory()
                                )
                     << "\">" << std::setw(3) << std::setfill('0') << tid << "</a></td>\n";
            }
            ostr << "  </tr>\n";
        }
    }

    ostr << "</table>\n";

    detail::dump_html_suffix(ostr);

    return pathname;
}

std::string  dump_recovery_properties(
        recovery_properties const&  rprops,
        microcode::program const&  P
        )
{
    if (!dump_enabled())
        return "";

    (void)P;
    std::string const pathname = fileutl::concatenate_file_paths(dump_directory(),"start.html");

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump file for recovery properties '" << pathname << "'.";
        return pathname;
    }

    detail::dump_html_prefix(ostr);

    ostr << "<h2>Recovery properties</h2>\n";
    ostr << "<p>All numbers are hexadecimal.</p>\n";
    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Property</th>\n";
    ostr << "    <th style=\"width:430px\">Value</th>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Heap begin</td>\n";
    ostr << "    <td>" << std::hex << rprops.heap_begin() << "</td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Heap end</td>\n";
    ostr << "    <td>" << std::hex << rprops.heap_end() << "</td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Temporaries begin</td>\n";
    ostr << "    <td>" << std::hex << rprops.temporaries_begin() << "</td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Ranges of important code</td>\n";
    ostr << "    <td>";
    for (auto const a_u : rprops.important_code())
        ostr << "[" << std::hex << a_u.first << "," << a_u.second << "), ";
    ostr << "    </td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Control switches</td>\n";
    ostr << "    <td><a href=\"./"
         << detail::compute_relative_pathname(dump_control_switches(rprops.switches(),rprops.important_code(),msgstream() << "control_switches.html"),dump_directory())
         << "\">here</a></td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Unexplored exits</td>\n";
    ostr << "    <td><a href=\"./"
         << detail::compute_relative_pathname(dump_unexplored_exits(rprops.unexplored(),rprops.important_code(),msgstream() << "unexplored_exits.html"),dump_directory())
         << "\">here</a></td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Visited branchings</td>\n";
    ostr << "    <td><a href=\"./"
         << detail::compute_relative_pathname(dump_visited_branchings(rprops.visited_branchings(),msgstream() << "visited_branchings.html"),dump_directory())
         << "\">here</a></td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Threads IDs of performed executions</td>\n";
    ostr << "    <td><a href=\"./"
         << detail::compute_relative_pathname(dump_threads_of_performed_executions(rprops,msgstream() << "threads_of_performed_executions.html"),dump_directory())
         << "\">here</a></td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Interleaving of threads in performed executions</td>\n";
    ostr << "    <td><a href=\"./"
         << detail::compute_relative_pathname(dump_interleaving_of_threads(rprops,msgstream() << "interleaving_of_threads.html"),dump_directory())
         << "\">here</a></td>\n";
    ostr << "  </tr>\n";



    ostr << "  <tr>\n";
    ostr << "    <td>Node histories of executed threads</td>\n";
    ostr << "    <td><a href=\"./"
         << detail::compute_relative_pathname(dump_node_histories_of_executed_threads(rprops.node_histories(),
                                                                                      msgstream() << "nodes_history_of_executed_threads.html"),dump_directory())
         << "\">here</a></td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Branchings of executed threads</td>\n";
    ostr << "    <td><a href=\"./"
         << detail::compute_relative_pathname(dump_branchings_of_executed_threads(rprops.branchings(),rprops.node_histories(),
                                                                                  msgstream() << "branchings_of_executed_threads.html"),dump_directory())
         << "\">here</a></td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Input impacts of executed threads</td>\n";
    ostr << "    <td><a href=\"./"
         << detail::compute_relative_pathname(dump_input_impacts_of_executed_threads(rprops.input_impacts(),rprops.node_histories(),
                                                                                     msgstream() << "input_impacts_of_executed_threads.html"),dump_directory())
         << "\">here</a></td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Input impact links of executed threads</td>\n";
    ostr << "    <td><a href=\"./"
         << detail::compute_relative_pathname(dump_input_impact_links_of_executed_threads(rprops.input_impact_links(),rprops.node_histories(),
                                                                                          msgstream() << "input_impact_links_of_executed_threads.html"),dump_directory())
         << "\">here</a></td>\n";
    ostr << "  </tr>\n";
    ostr << "</table>\n";

    detail::dump_html_suffix(ostr);

    return pathname;
}

std::string  dump_input_frontier_of_thread(
        input_frontier_of_thread const&  frontier,
        std::vector<node_id> const* const  nodes_history,
        std::string const&  filename)
{
    std::string const  pathname = fileutl::concatenate_file_paths(dump_directory(),filename);

    std::string const  pure_filename = fileutl::remove_extension(filename);

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump file of input frontier of an executed thread '" << pathname << "'.";
        return pathname;
    }

    detail::dump_html_prefix(ostr);

    ostr << "<h2>Input frontier of an executed thread</h2>\n";

    if (!frontier.reg_impacts().empty())
    {
        ostr << "<h3>REG pool impacts</h3>\n";
        ostr << "<p>\n"
                "Addresses are hexadecimal, other numbers are decimal."
                "</p>\n";
        ostr << "<table>\n";
        ostr << "  <tr>\n";
        ostr << "    <th>Address</th>\n";
        ostr << "    <th>Counter</th>\n";
        ostr << "    <th>Index</th>\n";
        if (nodes_history != nullptr)
            ostr << "    <th>Node</th>\n";
        ostr << "  </tr>\n";
        std::set<address>  sorted;
        for (auto const& adr_link : frontier.reg_impacts())
            sorted.insert(adr_link.first);
        for (auto const& adr : sorted)
        {
            ostr << "  <tr>\n";
            ostr << "    <td>" << std::setw(16) << std::setfill('0') << std::hex << adr << "</td>\n";
            input_impact_link const&  link = frontier.reg_impacts().at(adr);
            ostr << "    <td>" << std::dec << link.first << "</td>\n";
            ostr << "    <td>" << std::dec << link.second << "</td>\n";
            if (nodes_history != nullptr)
            {
                ASSUMPTION(nodes_history->size() > link.first);
                ostr << "    <td>" << std::dec << nodes_history->at(link.first) << "</td>\n";
            }
        }
        ostr << "</table>\n";
    }

    if (!frontier.mem_impacts().empty())
    {
        ostr << "<h3>MEM pool impacts</h3>\n";
        ostr << "<p>\n"
                "Addresses are hexadecimal, other numbers are decimal."
                "</p>\n";
        ostr << "<table>\n";
        ostr << "  <tr>\n";
        ostr << "    <th>Address</th>\n";
        ostr << "    <th>Counter</th>\n";
        ostr << "    <th>Index</th>\n";
        if (nodes_history != nullptr)
            ostr << "    <th>Node</th>\n";
        ostr << "  </tr>\n";
        std::set<address>  sorted;
        for (auto const& adr_link : frontier.mem_impacts())
            sorted.insert(adr_link.first);
        for (auto const& adr : sorted)
        {
            ostr << "  <tr>\n";
            ostr << "    <td>" << std::setw(16) << std::setfill('0') << std::hex << adr << "</td>\n";
            input_impact_link const&  link = frontier.mem_impacts().at(adr);
            ostr << "    <td>" << std::dec << link.first << "</td>\n";
            ostr << "    <td>" << std::dec << link.second << "</td>\n";
            if (nodes_history != nullptr)
            {
                ASSUMPTION(nodes_history->size() > link.first);
                ostr << "    <td>" << std::dec << nodes_history->at(link.first) << "</td>\n";
            }
        }
        ostr << "</table>\n";
    }
    if (!frontier.stream_impacts().empty())
    {
        ostr << "<h3>Stream impacts</h3>\n";
        ostr << "<p>\n"
                "Addresses are hexadecimal, other numbers are decimal."
                "</p>\n";
        ostr << "<table>\n";
        ostr << "  <tr>\n";
        ostr << "    <th>Stream</th>\n";
        ostr << "    <th>Shift</th>\n";
        ostr << "    <th>Counter</th>\n";
        ostr << "    <th>Index</th>\n";
        if (nodes_history != nullptr)
            ostr << "    <th>Node</th>\n";
        ostr << "  </tr>\n";
        std::set< std::pair<stream_id,address> >  sorted;
        for (auto const& id_adr__link : frontier.stream_impacts())
            sorted.insert(id_adr__link.first);
        for (auto const& id_adr : sorted)
        {
            ostr << "  <tr>\n";
            ostr << "    <td>" << id_adr.first << "</td>\n";
            ostr << "    <td>" << std::setw(16) << std::setfill('0') << std::hex << id_adr.second << "</td>\n";
            input_impact_link const&  link = frontier.stream_impacts().at(id_adr);
            ostr << "    <td>" << std::dec << link.first << "</td>\n";
            ostr << "    <td>" << std::dec << link.second << "</td>\n";
            if (nodes_history != nullptr)
            {
                ASSUMPTION(nodes_history->size() > link.first);
                ostr << "    <td>" << std::dec << nodes_history->at(link.first) << "</td>\n";
            }
        }
        ostr << "</table>\n";
    }

    detail::dump_html_suffix(ostr);

    return pathname;
}

std::string  dump_thread_properties(
        microcode::program const&  P,
        thread const&  thd,
        execution_properties const&  eprops,
        recovery_properties const&  rprops,
        std::map<std::pair<uint64_t,uint64_t>,std::string> const&  ranges_to_registers,
        std::string const&  filename)
{
    std::string const  pathname = fileutl::concatenate_file_paths(dump_directory(),filename);

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump file of thread properties '" << pathname << "'.";
        return pathname;
    }

    detail::dump_html_prefix(ostr);

    ostr << "<h2>Properties of the thread no. " << thd.id() << "</h2>\n";

    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Property</th>\n";
    ostr << "    <th>Value</th>\n";
    ostr << "  </tr>\n";

    ostr << "  <tr>\n";
    ostr << "    <td>Stack</td>\n";
    ostr << "    <td>[\n";
    for (index i = 0ULL; i < thd.stack().size(); ++i)
        ostr << "    "
             << std::dec << thd.stack().at(i)
             << "{" << P.component(microcode::find_component(P,thd.stack().at(i))).entry() << "}"
             << (i + 1ULL == thd.stack().size() ? "" : ",") << "\n";
    ostr << "    ]</td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Content of REG pool</td>\n";
    ostr << "    <td><a href=\"./"
         << detail::compute_relative_pathname(dump_memory_content(thd.reg(),msgstream() << "thread_" << std::dec << thd.id() << "_REG.html"),dump_directory())
         << "\">here</a></td>\n";
    ostr << "  </tr>\n";
    if (!ranges_to_registers.empty())
    {
        ostr << "  <tr>\n";
        ostr << "    <td>Content of registers</td>\n";
        ostr << "    <td><a href=\"./"
             << detail::compute_relative_pathname(dump_content_of_registers(thd.reg(),ranges_to_registers,
                                                                            msgstream() << "thread_" << std::dec << thd.id() << "_CPU_registers.html"),
                                                  dump_directory())
             << "\">here</a></td>\n";
        ostr << "  </tr>\n";
    }
    ostr << "  <tr>\n";
    ostr << "    <td>History of nodes</td>\n";
    ostr << "    <td><a href=\"./"
         << detail::compute_relative_pathname(dump_nodes_history_of_one_executed_thread(rprops.node_histories().at(eprops.get_execution_id()).at(thd.id()),
                                                                                        msgstream() << "nodes_history_of_thread_" << thd.id() << ".html"),dump_directory())
         << "\">here</a></td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Input frontier</td>\n";
    ostr << "    <td><a href=\"./"
         << detail::compute_relative_pathname(dump_input_frontier_of_thread(eprops.input_frontier(thd.id()),&rprops.node_histories().at(eprops.get_execution_id()).at(thd.id()),
                                                                            msgstream() << "input_frontier_of_thread_" << thd.id() << ".html"),dump_directory())
         << "\">here</a></td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Input impacts</td>\n";
    ostr << "    <td><a href=\"./"
         << detail::compute_relative_pathname(dump_input_impacts_of_one_executed_thread(rprops.input_impacts().at(eprops.get_execution_id()).at(thd.id()),
                                                                                        rprops.node_histories().at(eprops.get_execution_id()).at(thd.id()),
                                                                                        msgstream() << "input_impacts_of_thread_" << thd.id() << ".html"),dump_directory())
         << "\">here</a></td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Input impact links</td>\n";
    ostr << "    <td><a href=\"./"
         << detail::compute_relative_pathname(dump_input_impact_links_of_one_executed_thread(rprops.input_impact_links().at(eprops.get_execution_id()).at(thd.id()),
                                                                                             rprops.node_histories().at(eprops.get_execution_id()).at(thd.id()),
                                                                                             msgstream() << "input_impact_links_of_thread_" << thd.id() << ".html"),dump_directory())
         << "\">here</a></td>\n";
    ostr << "  </tr>\n";

    ostr << "</table>\n";

    detail::dump_html_suffix(ostr);

    return pathname;
}

std::string  dump_execution_state(
        microcode::program const&  P,
        execution_properties const&  eprops,
        recovery_properties const&  rprops,
        //probe_properties const* const  pprops,
        std::vector<thread> const&  thds,
        index const  fail_thread_index,
        std::string const&  error_message,
        std::string const&  dump_subdir,
        std::map<std::pair<uint64_t,uint64_t>,std::string> const&  ranges_to_registers
        )
{
    if (!dump_enabled())
        return "";

    ASSUMPTION(fail_thread_index == thds.size() || !error_message.empty());

    std::string const root_dir = fileutl::concatenate_file_paths(dump_directory(),dump_subdir);
    dump_push_guard const  root_pusher(root_dir);

    std::string const pathname = fileutl::concatenate_file_paths(root_dir,"start.html");

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump file for exexution state '" << pathname << "'.";
        return pathname;
    }

    detail::dump_html_prefix(ostr);

    ostr << "<h1>Log of execution state</h1>\n";
    ostr << "<p>All numbers are decimal. Thread's stack is printed from the bottom to the top\n"
            "(i.e. the right-most node is the top) and numbers in curly brackets are IDs of entry\n"
            "nodes of called components.</p>\n";
    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Property</th>\n";
    ostr << "    <th>Value</th>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Name of the executed program</td>\n";
    ostr << "    <td>" << P.name() << "</td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Execution ID</td>\n";
    ostr << "    <td>" << std::dec << eprops.get_execution_id() << "</td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Number of passed seconds</td>\n";
    ostr << "    <td>" << rprops.passed_milliseconds() / 1000.0 << "</td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Timeout in seconds</td>\n";
    ostr << "    <td>" << rprops.timeout_in_milliseconds() / 1000.0 << "</td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    if (fail_thread_index != thds.size())
        ostr << "    <td>Error message produced by thread #" << std::dec << thds.at(fail_thread_index).id() << "</td>\n";
    else
        ostr << "    <td>Produced error message</td>\n";
    ostr << "    <td>" << (error_message.empty() ? "NONE" : error_message) << "</td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Number of threads</td>\n";
    ostr << "    <td>" << thds.size() << "</td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Threads</td>\n";
    ostr << "    <td>\n";
    for (thread const&  thd : thds)
    {
        ostr << "    <a href=\"./"
             << detail::compute_relative_pathname(dump_thread_properties(P,thd,eprops,rprops,ranges_to_registers,
                                                                         msgstream() << "thread_" << std::dec << thd.id() << "_properties.html"),
                                                  root_dir)
             << "\">" << std::setw(3) << std::setfill('0') << std::dec << thd.id() << "</a>, \n";
    }
    ostr << "</td>\n";
    for (auto const&  id_reg : eprops.final_regs())
    {
        ostr << "  <tr>\n";
        ostr << "    <td>Final REG received from thread #" << std::dec <<  id_reg.first << "</td>\n";
        ostr << "    <td><a href=\"./"
             << detail::compute_relative_pathname(dump_memory_content(*id_reg.second,msgstream() << "final_REG_from_thread_" << std::dec << id_reg.first << ".html"),root_dir)
             << "\">here</a></td>\n";
        ostr << "  </tr>\n";
    }
    ostr << "  <tr>\n";
    ostr << "    <td>Memory allocations in MEM pool</td>\n";
    ostr << "    <td><a href=\"./"
         << detail::compute_relative_pathname(dump_memory_allocations(eprops.mem_allocations(),msgstream() << "mem_alloc.html"),root_dir)
         << "\">here</a></td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Content of MEM pool</td>\n";
    ostr << "    <td><a href=\"./"
         << detail::compute_relative_pathname(dump_memory_content(eprops.mem_content(),msgstream() << "mem_content.html",&eprops.mem_allocations()),root_dir)
         << "\">here</a></td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Streams</td>\n";
    ostr << "    <td><a href=\"./"
         << detail::compute_relative_pathname(dump_streams(eprops.stream_allocations(),eprops.contents_of_streams(),msgstream() << "streams.html"),root_dir)
         << "\">here</a></td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Interrupts vector table</td>\n";
    ostr << "    <td><a href=\"./"
         << detail::compute_relative_pathname(dump_interrupts(eprops.interrupts(),msgstream() << "interrupts.html"),root_dir)
         << "\">here</a></td>\n";
    ostr << "  </tr>\n";
//    if (pprops != nullptr)
//    {
//        dump_push_guard const  probe_pusher(fileutl::concatenate_file_paths(dump_directory(),"/probe_properties"));
//        ostr << "  <tr>\n";
//        ostr << "    <td>Probe properties</td>\n";
//        ostr << "    <td><a href=\"./"
//             << detail::compute_relative_pathname(dump_probe_properties(*pprops,P),root_dir)
//             << "\">here</a></td>\n";
//        ostr << "  </tr>\n";
//    }
    ostr << "</table>\n";

    detail::dump_html_suffix(ostr);

    return pathname;
}

std::string  dump_at_breakpoint(
        microcode::program const&  P,
        execution_properties const&  eprops,
        recovery_properties const&  rprops,
        //probe_properties const* const  pprops,
        thread const&  thd,
        node_id const  node,
        uint64_t const  hit_number,
        std::string const&  dump_subdir,
        std::map<std::pair<uint64_t,uint64_t>,std::string> const&  ranges_to_registers
        )
{
    if (!dump_enabled())
        return "";

    std::string const root_dir = fileutl::concatenate_file_paths(dump_directory(),dump_subdir);
    dump_push_guard const  root_pusher(root_dir);

    std::string const pathname = fileutl::concatenate_file_paths(root_dir,"start.html");

    std::fstream  ostr(pathname, std::ios_base::out);
    if (!ostr.is_open())
    {
        std::cerr << "natexe: Cannot open the dump file for exexution state '" << pathname << "'.";
        return pathname;
    }

    detail::dump_html_prefix(ostr);

    ostr << "<h1>Log of breakpoint</h1>\n";
    ostr << "<p>All numbers are decimal. Thread's stack is printed from the bottom to the top\n"
            "(i.e. the right-most node is the top) and numbers in curly brackets are IDs of entry\n"
            "nodes of called components.</p>\n";
    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Property</th>\n";
    ostr << "    <th>Value</th>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Name of the executed program</td>\n";
    ostr << "    <td>" << P.name() << "</td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Execution ID</td>\n";
    ostr << "    <td>" << std::dec << eprops.get_execution_id() << "</td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Thread id</td>\n";
    ostr << "    <td>" << std::dec <<  thd.id() << "</td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Breakpoint node</td>\n";
    ostr << "    <td>" << std::dec << node << "</td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>The number of hit of the node</td>\n";
    ostr << "    <td>" << std::dec << hit_number << "</td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Thread properties</td>\n";
    ostr << "    <td><a href=\"./"
         << detail::compute_relative_pathname(dump_thread_properties(P,thd,eprops,rprops,ranges_to_registers,
                                                                     msgstream() << "thread_" << std::dec << thd.id() << "_properties.html"),
                                              root_dir)
         << "\">here</a></td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Memory allocations in MEM pool</td>\n";
    ostr << "    <td><a href=\"./"
         << detail::compute_relative_pathname(dump_memory_allocations(eprops.mem_allocations(),msgstream() << "mem_alloc.html"),root_dir)
         << "\">here</a></td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Content of MEM pool</td>\n";
    ostr << "    <td><a href=\"./"
         << detail::compute_relative_pathname(dump_memory_content(eprops.mem_content(),msgstream() << "mem_content.html",&eprops.mem_allocations()),root_dir)
         << "\">here</a></td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Streams</td>\n";
    ostr << "    <td><a href=\"./"
         << detail::compute_relative_pathname(dump_streams(eprops.stream_allocations(),eprops.contents_of_streams(),msgstream() << "streams.html"),root_dir)
         << "\">here</a></td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Interrupts vector table</td>\n";
    ostr << "    <td><a href=\"./"
         << detail::compute_relative_pathname(dump_interrupts(eprops.interrupts(),msgstream() << "interrupts.html"),root_dir)
         << "\">here</a></td>\n";
    ostr << "  </tr>\n";
//    if (pprops != nullptr)
//    {
//        dump_push_guard const  probe_pusher(fileutl::concatenate_file_paths(dump_directory(),"/probe_properties"));
//        ostr << "  <tr>\n";
//        ostr << "    <td>Probe properties</td>\n";
//        ostr << "    <td><a href=\"./"
//             << detail::compute_relative_pathname(dump_probe_properties(*pprops,P),root_dir)
//             << "\">here</a></td>\n";
//        ostr << "  </tr>\n";
//    }
    ostr << "</table>\n";

    detail::dump_html_suffix(ostr);

    return pathname;
}


}}
