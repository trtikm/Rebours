#include <rebours/MAL/descriptor/dump.hpp>
#include <rebours/MAL/descriptor/assumptions.hpp>
#include <rebours/MAL/descriptor/invariants.hpp>
#include <rebours/MAL/descriptor/msgstream.hpp>
#include <rebours/MAL/loader/dump.hpp>
#include <rebours/MAL/loader/file_utils.hpp>
#include <fstream>
#include <iomanip>
#include <map>

#include <algorithm>
//#include <cctype>
//#include <cstdlib>
//#include <iomanip>
//#include <set>
//#include <sstream>

namespace {


struct  html_prolog_epilog
{
    html_prolog_epilog();
    ~html_prolog_epilog();
};

void dump_html_prolog(std::ostream&  ostr)
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

void dump_html_epilog(std::ostream&  ostr)
{
    ostr << "</body>\n";
    ostr << "</html>\n";
}

}


namespace mal { namespace descriptor { namespace detail {


std::string const  registers_mapping_relative_pathname = "./registers_mapping.html";
std::string const  tls_relative_path = "./TLS";
std::string const  file_data_relative_pathname = "./file_descriptor/start.html";


void dump_tls_template_content(std::vector<uint8_t> const&  data, std::string const&  output_filename)
{
    std::fstream  ostr(output_filename, std::ios_base::out);
    ASSUMPTION(ostr.is_open());

    dump_html_prolog(ostr);

    ostr << "<h2>Content of TLS template</h2>\n";
    ostr << "<p>All number here are hexadecimal. The number of bytes in the array is: " << std::hex << data.size() << "</p>\n";
    ostr << "<pre>\n";

    uint8_t const*  ptr = data.data();
    uint8_t const* const  end = ptr + data.size();
    uint64_t  adr = 0ULL;
    do
    {
        std::string  text;
        ostr << std::hex << std::setw(12) << std::setfill('0') << adr << "   ";
        int i = 0;
        for ( ; i < 16 && ptr != end; ++i, ++ptr)
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
        if (i <= 8)
            ostr << ' ';
        for ( ; i < 16; ++i)
            ostr << "   ";
        ostr << "    " << text << '\n';
        adr += 16ULL;
    }
    while (ptr != end);

    ostr << '\n';
    ostr << "</pre>\n";

    dump_html_epilog(ostr);
}

void dump_tls_template_relocations(std::vector<uint64_t> const&  data, std::string const&  output_filename)
{
    std::fstream  ostr(output_filename, std::ios_base::out);
    ASSUMPTION(ostr.is_open());

    dump_html_prolog(ostr);

    ostr << "<p>\nA list of offsets into the array of TLS template bytes where\n"
            "relocations have to be performed after copying the bytes into some\n"
            "allocated block of memory. If the block starts at an address 'base',\n"
            "'offset' is any number listed in the table below, and 'value' is an\n"
            "8-byte unsigned integer stored in the array of TLS template bytes at\n"
            "the offset 'offset', then these 8 bytes must be rewritten by 8-byte\n"
            "unsigned integer 'base'+'value' in the array (at the offset 'offset').\n"
            "All numbers in the table below are hexadecimal.\n</p>\n";
    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Offset</th>\n";
    ostr << "  </tr>\n";

    for (uint64_t const  offset : data)
    {
        ostr << "  <tr>\n";
        ostr << "    <td>" << std::hex << offset <<  "</td>\n";
        ostr << "  </tr>\n";
    }

    ostr << "</table>\n";

    dump_html_epilog(ostr);
}

void dump_stack_sections(std::vector<stack_section> const&  data, std::string const&  output_filename)
{
    std::fstream  ostr(output_filename, std::ios_base::out);
    ASSUMPTION(ostr.is_open());

    dump_html_prolog(ostr);

    ostr << "<h2>Listing of stack sections</h2>\n";
    ostr << "<p>All numbers are hexadecimal.</p>\n";
    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Start address</th>\n";
    ostr << "    <th>End address</th>\n";
    ostr << "    <th>Num bytes</th>\n";
    ostr << "    <th>Readable</th>\n";
    ostr << "    <th>Writable</th>\n";
    ostr << "    <th>Executable</th>\n";
    ostr << "    <th>Uses big endian</th>\n";
    ostr << "    <th>Const endian</th>\n";
    ostr << "  </tr>\n";

    for (stack_section const&  sec : data)
    {
        ostr << "  <tr>\n";

        ostr << "    <td>";
        ostr << std::hex << sec.start_address();
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << std::hex << sec.end_address();
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << std::hex << sec.end_address() - sec.start_address();
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << (sec.has_read_access() ? "Yes" : "No");
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << (sec.has_write_access() ? "Yes" : "No");
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << (sec.has_execute_access() ? "Yes" : "No");
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << (sec.is_in_big_endian() ? "Yes" : "No");
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << (sec.has_const_endian() ? "Yes" : "No");
        ostr << "</td>\n";

        ostr << "  </tr>\n";
    }

    ostr << "</table>\n";

    dump_html_epilog(ostr);
}

void dump_default_stack_init_data(stack_init_data const&  data, std::string const&  output_filename)
{
    std::fstream  ostr(output_filename, std::ios_base::out);
    ASSUMPTION(ostr.is_open());

    dump_html_prolog(ostr);

    ostr << "<h2>Default stack init data</h2>\n";
    ostr << "<p>All numbers are hexadecimal.</p>\n";

    if (!data.first.empty())
    {
        ostr << "<h3>Command line arguments</h3>\n";
        ostr << "<table>\n";
        ostr << "  <tr>\n";
        ostr << "    <th>Index</th>\n";
        ostr << "    <th>Value</th>\n";
        ostr << "  </tr>\n";
        for (uint64_t  i = 0ULL; i < data.first.size(); ++i)
        {
            ostr << "  <tr>\n";
            ostr << "    <td>" << std::hex << i << "</td>\n";
            ostr << "    <td>" << data.first.at(i) << "</td>\n";
            ostr << "  </tr>\n";
        }
        ostr << "</table>\n";
    }

    if (!data.first.empty())
    {
        ostr << "<h3>Environment variables.</h3>\n";
        ostr << "<table>\n";
        ostr << "  <tr>\n";
        ostr << "    <th>Index</th>\n";
        ostr << "    <th>Name</th>\n";
        ostr << "    <th>Value</th>\n";
        ostr << "  </tr>\n";
        for (uint64_t  i = 0ULL; i < data.second.size(); ++i)
        {
            ostr << "  <tr>\n";
            ostr << "    <td>" << std::hex << i << "</td>\n";
            ostr << "    <td>" << data.second.at(i).first << "</td>\n";
            ostr << "    <td>" << data.second.at(i).second << "</td>\n";
            ostr << "  </tr>\n";
        }
        ostr << "</table>\n";
    }

    dump_html_epilog(ostr);
}


void  dump_registers_mapping(std::unordered_map<std::string,address_range> const&  mapping, std::string const&  root_dir)
{
    std::fstream  ostr(concatenate_file_paths(root_dir,registers_mapping_relative_pathname), std::ios_base::out);
    ASSUMPTION(ostr.is_open());

    dump_html_prolog(ostr);

    ostr << "<h2>MAL/descriptor: Registers mapping to ranges in REG memory pool</h2>\n";
    ostr << "<p>\nThe mapping is presented in the following table. Rows are sorted by start addresses\n"
            "of mapped registers. All numbers are hexadecimal.\n</p>\n";
    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Name</th>\n";
    ostr << "    <th>Begin</th>\n";
    ostr << "    <th>End</th>\n";
    ostr << "    <th>Bytes</th>\n";
    ostr << "  </tr>\n";
    std::multimap<uint64_t,std::pair<std::string,uint64_t> > sorted;
    for (auto const& name_range : mapping)
        sorted.insert({name_range.second.first,{name_range.first,name_range.second.second}});
    for (auto const& value : sorted)
    {
        ostr << "  <tr>\n";
        ostr << "    <td>" << value.second.first << "</td>\n";
        ostr << "    <td>" << std::hex << value.first <<  "</td>\n";
        ostr << "    <td>" << std::hex << value.second.second <<  "</td>\n";
        ostr << "    <td>" << std::hex << value.second.second - value.first <<  "</td>\n";
        ostr << "  </tr>\n";
    }
//    std::vector<std::string> keys;
//    for (auto const& name_range : mapping)
//        keys.push_back(name_range.first);
//    std::sort(keys.begin(),keys.end());
//    for (auto const& key : keys)
//    {
//        ostr << "  <tr>\n";
//        ostr << "    <td>" << key << "</td>\n";
//        ostr << "    <td>" << std::hex << mapping.at(key).first <<  "</td>\n";
//        ostr << "    <td>" << std::hex << mapping.at(key).second <<  "</td>\n";
//        ostr << "  </tr>\n";
//    }
    ostr << "</table>\n";
    ostr << "<p></p>\n";

    dump_html_epilog(ostr);
}


void  dump_html(storage const&  data, std::string const&  output_file_pathname)
{
    std::fstream  ostr(output_file_pathname, std::ios_base::out);
    ASSUMPTION(ostr.is_open());

    dump_html_prolog(ostr);

    ostr << "<h2>Dump of MAL/descriptor</h2>\n";
    ostr << "<p>All data are presented in this table. All numbers are hexadecimal.</p>\n";
    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Property</th>\n";
    ostr << "    <th>Value</th>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Processor architecture</td>\n";
    ostr << "    <td>" << (msgstream() << data.processor() << msgstream::end()) <<  "</td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Operating system</td>\n";
    ostr << "    <td>" << (msgstream() << data.system() << msgstream::end()) <<  "</td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Start address of temporaries</td>\n";
    ostr << "    <td>" << std::hex << data.start_address_of_temporaries() <<  "</td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Mapping of registers</td>\n";
    ostr << "    <td><a href=\"" << registers_mapping_relative_pathname << "\">here</a></td>\n";
    ostr << "  </tr>\n";
    if (data.has_tls_template())
    {
        ostr << "  <tr>\n";
        ostr << "    <td>TLS entry offset</td>\n";
        ostr << "    <td>" << data.tls_template_offset() << "</td>\n";
        ostr << "  </tr>\n";

        std::string const  abs_path = concatenate_file_paths(parse_path_in_pathname(output_file_pathname),tls_relative_path);
        create_directory(abs_path);

        {
            std::string const  filename = "tls_template_relocations.html";
            dump_tls_template_relocations(data.tls_template_relocations(),concatenate_file_paths(abs_path,filename));
            ostr << "  <tr>\n";
            ostr << "    <td>TLS template relocations</td>\n";
            ostr << "    <td><a href=\"" << concatenate_file_paths(tls_relative_path,filename) << "\">here</a></td>\n";
            ostr << "  </tr>\n";
        }

        {
            std::string const  filename = "tls_template_content.html";
            dump_tls_template_content(data.tls_template_content(),concatenate_file_paths(abs_path,filename));
            ostr << "  <tr>\n";
            ostr << "    <td>TLS template bytes</td>\n";
            ostr << "    <td><a href=\"" << concatenate_file_paths(tls_relative_path,filename) << "\">here</a></td>\n";
            ostr << "  </tr>\n";
        }
    }
    ostr << "  <tr>\n";
    ostr << "    <td>Heap start address</td>\n";
    ostr << "    <td>" << std::hex << data.heap_start() <<  "</td>\n";
    ostr << "  </tr>\n";
    {
        std::string const  abs_path = parse_path_in_pathname(output_file_pathname);
        std::string const  filename = "stack_sections.html";
        dump_stack_sections(data.stack_sections(),concatenate_file_paths(abs_path,filename));
        ostr << "  <tr>\n";
        ostr << "    <td>List of stack sections</td>\n";
        ostr << "    <td><a href=\"./" << filename << "\">here</a></td>\n";
        ostr << "  </tr>\n";
    }
    {
        std::string const  abs_path = parse_path_in_pathname(output_file_pathname);
        std::string const  filename = "stack_initial_data.html";
        dump_default_stack_init_data(data.default_stack_init_data(),concatenate_file_paths(abs_path,filename));
        ostr << "  <tr>\n";
        ostr << "    <td>Default stack init data</td>\n";
        ostr << "    <td><a href=\"./" << filename << "\">here</a></td>\n";
        ostr << "  </tr>\n";
    }
    ostr << "  <tr>\n";
    ostr << "    <td>Executable internals</td>\n";
    if (data.file_descriptor().operator bool())
        ostr << "    <td><a href=\"" << file_data_relative_pathname << "\">here</a></td>\n";
    else
        ostr << "    <td>Not available.</td>\n";
    ostr << "  </tr>\n";
    ostr << "</table>\n";
    ostr << "<p></p>\n";

    dump_html_epilog(ostr);
}


}}}


namespace mal { namespace descriptor {


void  dump_html(storage const&  data,
                std::string const&  output_file_pathname,
                bool const  dump_also_contents_of_sections
                )
{
    std::string const  root_dir = parse_path_in_pathname(output_file_pathname);
    create_directory(parse_path_in_pathname(output_file_pathname));
    if (data.file_descriptor().operator bool())
    {
        std::string const  file_descriptor_root_file = concatenate_file_paths(root_dir,detail::file_data_relative_pathname);
        loader::dump_html(data.file_descriptor(),file_descriptor_root_file,dump_also_contents_of_sections);
    }
    detail::dump_registers_mapping(data.registers_to_ranges(),root_dir);
    detail::dump_html(data,output_file_pathname);
}

void  dump_html(storage const&  data,
                std::string const&  output_file_pathname,
                std::vector<loader::address> const&  start_addresses_of_sections_whose_content_should_be_dumped
                )
{
    std::string const  root_dir = parse_path_in_pathname(output_file_pathname);
    create_directory(parse_path_in_pathname(output_file_pathname));
    if (data.file_descriptor().operator bool())
    {
        std::string const  file_descriptor_root_file = concatenate_file_paths(root_dir,detail::file_data_relative_pathname);
        loader::dump_html(data.file_descriptor(),file_descriptor_root_file,start_addresses_of_sections_whose_content_should_be_dumped);
    }
    detail::dump_registers_mapping(data.registers_to_ranges(),root_dir);
    detail::dump_html(data,output_file_pathname);
}


}}
