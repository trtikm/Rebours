#include <rebours/MAL/loader/dump.hpp>
#include <rebours/MAL/loader/special_sections/elf_tls.hpp>
#include <rebours/MAL/loader/file_utils.hpp>
#include <rebours/MAL/loader/invariants.hpp>
#include <rebours/MAL/loader/assumptions.hpp>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <set>
#include <sstream>

namespace loader { namespace {

void  dump_html_warnings(std::ofstream&  ostr, descriptor_ptr const  data, file_props_ptr const  fprops)
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
    for (auto const& warning : data->warnings()->at(fprops->path()))
    {
        ostr << "<p>\n";
        ostr << warning;
        ostr << "</p>\n";
    }
    ostr << "</body>\n";
    ostr << "</html>\n";
}

void  dump_html_relocations(std::ofstream&  ostr, descriptor_ptr const  data)
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
    ostr << "<p>\n";
    ostr << "All numbers are hexadecimal. The 'Size' filed is in bytes. 'Symbol name' and 'Symbol type' may "
            "be irrelevant for some relocations (the fields are thus empty).";
    ostr << "</p>\n";
    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Start address</th>\n";
    ostr << "    <th>Size</th>\n";
    ostr << "    <th>Stored value</th>\n";
//            ostr << "    <th>Original value</th>\n";
    ostr << "    <th>Symbol name</th>\n";
    ostr << "    <th>Symbol type</th>\n";
    ostr << "    <th>Description</th>\n";
    ostr << "    <th>File path</th>\n";
    ostr << "  </tr>\n";
    for (auto const&  elem : *data->performed_relocations())
    {
        ostr << "  <tr>\n";

        relocation const&  reloc = elem.second;

        ostr << "    <td>";
        ostr << std::hex << reloc.start_address();
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << std::hex << reloc.num_bytes();
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << std::hex << reloc.stored_value();
        ostr << "</td>\n";

//                ostr << "    <td>";
//                ostr << std::hex << reloc.original_value();
//                ostr << "</td>\n";

        ostr << "    <td>";
        ostr << reloc.symbol_id();
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << reloc.type();
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << reloc.description();
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << find_section(reloc.start_address(),data->sections_table())->file_props()->path();
        ostr << "</td>\n";

        ostr << "  </tr>\n";
    }
    ostr << "</p>\n";
    ostr << "</table>\n";
    ostr << "<p></p>\n";
    ostr << "</body>\n";
    ostr << "</html>\n";
}

void  dump_html_skipped_relocations(std::ofstream&  ostr, descriptor_ptr const  data)
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
    ostr << "<p>\n";
    ostr << "All numbers are hexadecimal. The 'Size' filed is in bytes.\n"
            "Zero size means unknown size. Empty 'Symbol name' and 'Symbol type' may mean either the "
            "symbol is not relevant for the relocation or the symbol was not found.";
    ostr << "</p>\n";
    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Start address</th>\n";
    ostr << "    <th>Size</th>\n";
    ostr << "    <th>Symbol name</th>\n";
    ostr << "    <th>Symbol type</th>\n";
    ostr << "    <th>Description</th>\n";
    ostr << "    <th>File path</th>\n";
    ostr << "  </tr>\n";
    for (auto const&  elem : *data->skipped_relocations())
    {
        ostr << "  <tr>\n";

        relocation const&  reloc = elem.second;

        ostr << "    <td>";
        ostr << std::hex << reloc.start_address();
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << reloc.num_bytes();
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << reloc.symbol_id();
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << reloc.type();
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << reloc.description();
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << find_section(reloc.start_address(),data->sections_table())->file_props()->path();
        ostr << "</td>\n";

        ostr << "  </tr>\n";
    }
    ostr << "</table>\n";
    ostr << "<p></p>\n";
    ostr << "</body>\n";
    ostr << "</html>\n";
}

void  dump_html_per_section_relocations(std::string const&  output_file_pathname, descriptor_ptr const  data,
                                        std::vector<address> const&  start_addresses_of_sections_whose_content_should_be_dumped,
                                        std::map<address,uint64_t>&  num_performed_relocations_per_section)
{
    for (auto const&  elem : *data->sections_table())
    {
        if (std::find(start_addresses_of_sections_whose_content_should_be_dumped.cbegin(),
                      start_addresses_of_sections_whose_content_should_be_dumped.cend(),
                      elem.first) == start_addresses_of_sections_whose_content_should_be_dumped.cend())
            continue;

        section_ptr const  ptr = elem.second;

        num_performed_relocations_per_section[ptr->start_address()] = 0ULL;

        std::stringstream  filename;
        filename << "relocations_performed_"
                 << std::hex << ptr->start_address() << "_"
                 << std::hex << ptr->end_address() << ".html";

        std::string const  output_section_file_pathname =
                concatenate_file_paths(
                        parse_path_in_pathname(output_file_pathname),
                        filename.str()
                        );

        std::ofstream  ostr{output_section_file_pathname,std::ofstream::binary};

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
        ostr << "<p>\n";
        ostr << "All numbers are hexadecimal. The 'Size' filed is in bytes. 'Symbol name' and 'Symbol type' may "
                "be irrelevant for some relocations (fields are thus empty).";
        ostr << "</p>\n";
        ostr << "<table>\n";
        ostr << "  <tr>\n";
        ostr << "    <th>Start address</th>\n";
        ostr << "    <th>Size</th>\n";
        ostr << "    <th>Stored value</th>\n";
//            ostr << "    <th>Original value</th>\n";
        ostr << "    <th>Symbol name</th>\n";
        ostr << "    <th>Symbol type</th>\n";
        ostr << "    <th>Description</th>\n";
        ostr << "  </tr>\n";
        for (auto const&  elem : *data->performed_relocations())
        {
            ostr << "  <tr>\n";

            relocation const&  reloc = elem.second;
            if (reloc.start_address() >= ptr->end_address() ||
                reloc.start_address() + reloc.num_bytes() <= ptr->start_address() )
                continue;

            ++num_performed_relocations_per_section[ptr->start_address()];

            ostr << "    <td>";
            ostr << std::hex << reloc.start_address();
            ostr << "</td>\n";

            ostr << "    <td>";
            ostr << std::hex << reloc.num_bytes();
            ostr << "</td>\n";

            ostr << "    <td>";
            ostr << std::hex << reloc.stored_value();
            ostr << "</td>\n";

//                ostr << "    <td>";
//                ostr << std::hex << reloc.original_value();
//                ostr << "</td>\n";

            ostr << "    <td>";
            ostr << reloc.symbol_id();
            ostr << "</td>\n";

            ostr << "    <td>";
            ostr << reloc.type();
            ostr << "</td>\n";

            ostr << "    <td>";
            ostr << reloc.description();
            ostr << "</td>\n";

            ostr << "  </tr>\n";
        }
        ostr << "</table>\n";
        ostr << "<p></p>\n";
        ostr << "</body>\n";
        ostr << "</html>\n";
    }
}

void  dump_html_per_section_skipped_relocations(std::string const&  output_file_pathname, descriptor_ptr const  data,
                                                std::vector<address> const&  start_addresses_of_sections_whose_content_should_be_dumped,
                                                std::map<address,uint64_t>&  num_skipped_relocations_per_section)
{
    for (auto const&  elem : *data->sections_table())
    {
        if (std::find(start_addresses_of_sections_whose_content_should_be_dumped.cbegin(),
                      start_addresses_of_sections_whose_content_should_be_dumped.cend(),
                      elem.first) == start_addresses_of_sections_whose_content_should_be_dumped.cend())
            continue;

        section_ptr const  ptr = elem.second;

        num_skipped_relocations_per_section[ptr->start_address()] = 0ULL;

        std::stringstream  filename;
        filename << "relocations_skipped_"
                 << std::hex << ptr->start_address() << "_"
                 << std::hex << ptr->end_address() << ".html";

        std::string const  output_section_file_pathname =
                concatenate_file_paths(
                        parse_path_in_pathname(output_file_pathname),
                        filename.str()
                        );

        std::ofstream  ostr{output_section_file_pathname,std::ofstream::binary};

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
        ostr << "<p>\n";
        ostr << "All numbers are hexadecimal. The 'Size' filed is in bytes. "
                "Zero size means unknown size. Empty symbol name may mean either the "
                "symbol is not relevant for the relocation or it was not found.";
        ostr << "</p>\n";
        ostr << "<table>\n";
        ostr << "  <tr>\n";
        ostr << "    <th>Start address</th>\n";
        ostr << "    <th>Size</th>\n";
        ostr << "    <th>Symbol name</th>\n";
        ostr << "    <th>Symbol type</th>\n";
        ostr << "    <th>Description</th>\n";
        ostr << "  </tr>\n";
        for (auto const&  elem : *data->skipped_relocations())
        {
            ostr << "  <tr>\n";

            relocation const&  reloc = elem.second;
            if (reloc.start_address() >= ptr->end_address() ||
                reloc.start_address() + reloc.num_bytes() <= ptr->start_address() )
                continue;

            ++num_skipped_relocations_per_section[ptr->start_address()];

            ostr << "    <td>";
            ostr << std::hex << reloc.start_address();
            ostr << "</td>\n";

            ostr << "    <td>";
            ostr << reloc.num_bytes();
            ostr << "</td>\n";

            ostr << "    <td>";
            ostr << reloc.symbol_id();
            ostr << "</td>\n";

            ostr << "    <td>";
            ostr << reloc.type();
            ostr << "</td>\n";

            ostr << "    <td>";
            ostr << reloc.description();
            ostr << "</td>\n";

            ostr << "  </tr>\n";
        }
        ostr << "</table>\n";
        ostr << "<p></p>\n";
        ostr << "</body>\n";
        ostr << "</html>\n";
    }
}

void  dump_html_contents_of_sections(std::string const&  output_file_pathname, descriptor_ptr const  data,
                                     std::vector<address> const&  start_addresses_of_sections_whose_content_should_be_dumped)
{
    std::set<address>  relocated_addresses;
    for (auto const&  elem : *data->performed_relocations())
    {
        relocation const&  reloc = elem.second;
        for (uint64_t  shift = 0ULL; shift < reloc.num_bytes(); ++shift)
            relocated_addresses.insert(reloc.start_address() + shift);
    }

    for (auto const&  elem : *data->sections_table())
    {
        if (std::find(start_addresses_of_sections_whose_content_should_be_dumped.cbegin(),
                      start_addresses_of_sections_whose_content_should_be_dumped.cend(),
                      elem.first) == start_addresses_of_sections_whose_content_should_be_dumped.cend())
            continue;

        section_ptr const  ptr = elem.second;

        std::stringstream  filename;
        filename << "section_content_"
                 << std::hex << ptr->start_address() << "_"
                 << std::hex << ptr->end_address() << ".html";

        std::string const  output_section_file_pathname =
                concatenate_file_paths(
                        parse_path_in_pathname(output_file_pathname),
                        filename.str()
                        );

        std::ofstream  ostr{output_section_file_pathname,std::ofstream::binary};

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
        //ostr << "<h2>Section content</h2>\n";
        ostr << "<p>\n";
        ostr << "All number are hexadecimal. Relocated bytes are red. A row with\n"
                "a special address '...' (if actually present) stands for several rows\n"
                "with the same conntent up to the row below it with a regular address.";
        ostr << "</p>\n";
        ostr << "<table>\n";
        ostr << "  <tr>\n";
        ostr << "    <th>Address</th>\n";
        ostr << "    <th>00</th>\n";
        ostr << "    <th>01</th>\n";
        ostr << "    <th>02</th>\n";
        ostr << "    <th>03</th>\n";
        ostr << "    <th>04</th>\n";
        ostr << "    <th>05</th>\n";
        ostr << "    <th>06</th>\n";
        ostr << "    <th>07</th>\n";
        ostr << "    <th>08</th>\n";
        ostr << "    <th>09</th>\n";
        ostr << "    <th>0a</th>\n";
        ostr << "    <th>0b</th>\n";
        ostr << "    <th>0c</th>\n";
        ostr << "    <th>0d</th>\n";
        ostr << "    <th>0e</th>\n";
        ostr << "    <th>0f</th>\n";
        ostr << "    <th>ASCII</th>\n";

        section_file_header_ptr const  header = ptr->section_file_header();
        section_content_ptr const  content = elem.second->content();
        INVARIANT(header->size_in_memory() > 0ULL);
        uint64_t const  total_num_iterations =
                header->size_in_memory() + (header->size_in_memory() % 16ULL == 0ULL ?
                                                    0ULL :
                                                    16ULL - header->size_in_memory() % 16ULL);
        INVARIANT(total_num_iterations % 16ULL == 0ULL);
        std::string  ascii;
        for (uint64_t  shift = 0ULL; shift < total_num_iterations; ++shift)
        {
            if (shift % 16ULL == 0ULL)
            {
                if (shift > 0ULL)
                {
                    ostr << "    <td><tt>" << ascii << "</tt></td>\n";
                    ascii.clear();
                    ostr << "  </tr>\n";

                    if (shift > header->size_in_file() &&
                        shift + 16ULL < header->size_in_memory() &&
                        (relocated_addresses.empty() ||
                         shift > *std::prev(relocated_addresses.end())) )
                    {
                        ostr << "  <tr>\n";
                        ostr << "    <td>...</td>\n";

                        ostr << "    <td>00</td>\n";
                        ostr << "    <td>00</td>\n";
                        ostr << "    <td>00</td>\n";
                        ostr << "    <td>00</td>\n";

                        ostr << "    <td>00</td>\n";
                        ostr << "    <td>00</td>\n";
                        ostr << "    <td>00</td>\n";
                        ostr << "    <td>00</td>\n";

                        ostr << "    <td>00</td>\n";
                        ostr << "    <td>00</td>\n";
                        ostr << "    <td>00</td>\n";
                        ostr << "    <td>00</td>\n";

                        ostr << "    <td>00</td>\n";
                        ostr << "    <td>00</td>\n";
                        ostr << "    <td>00</td>\n";
                        ostr << "    <td>00</td>\n";

                        ostr << "    <td><tt></tt></td>\n"; // ASCII text

                        ostr << "  </tr>\n";
                        ostr << "  <tr>\n";
                        ostr << "    <td>";
                        ostr << std::hex << (ptr->end_address() - 1ULL);
                        ostr << "</td>\n";
                        ostr << "    <td>00</td>\n";
                        ostr << "    <td>" << "??" << "</td>\n";
                        ostr << "    <td>" << "??" << "</td>\n";
                        ostr << "    <td>" << "??" << "</td>\n";

                        ostr << "    <td>" << "??" << "</td>\n";
                        ostr << "    <td>" << "??" << "</td>\n";
                        ostr << "    <td>" << "??" << "</td>\n";
                        ostr << "    <td>" << "??" << "</td>\n";

                        ostr << "    <td>" << "??" << "</td>\n";
                        ostr << "    <td>" << "??" << "</td>\n";
                        ostr << "    <td>" << "??" << "</td>\n";
                        ostr << "    <td>" << "??" << "</td>\n";

                        ostr << "    <td>" << "??" << "</td>\n";
                        ostr << "    <td>" << "??" << "</td>\n";
                        ostr << "    <td>" << "??" << "</td>\n";
                        ostr << "    <td>" << "??" << "</td>\n";

                        ascii.clear();
                        break;
                    }
                }
                ostr << "  <tr>\n";
                ostr << "    <td>";
                ostr << std::hex << (ptr->start_address() + shift);
                ostr << "</td>\n";
            }
            ostr << "    <td>";
            if (shift < header->size_in_memory())
            {
                INVARIANT(shift < content->size());
                uint32_t const  value = (uint32_t)content->at(shift);
                bool const  highlight = relocated_addresses.count(ptr->start_address() + shift) != 0ULL;
                if (highlight)
                    ostr << "<font color=\"red\">";
                ostr << std::hex << std::setfill('0') << std::setw(2) << value;
                if (highlight)
                    ostr << "</font>";
                if (std::isprint(value) && value != (uint32_t)' ' && value != (uint32_t)'\t')
                {
                    if (value == (uint32_t)'<')
                        ascii.append("&lt;");
                    else if (value == (uint32_t)'>')
                        ascii.append("&gt;");
                    else
                        ascii.push_back(value);
                }
                else
                    ascii.append("&nbsp;");
            }
            else
            {
                ostr << "??";
                ascii.append("&nbsp;");
            }
            ostr << "</td>\n";
        }
        ostr << "    <td><tt>" << ascii << "</tt></td>\n";
        ostr << "  </tr>\n";
        ostr << "</table>\n";
        ostr << "<p></p>\n";
        ostr << "</body>\n";
        ostr << "</html>\n";
    }
}

void  dump_html_special_section_elf_tls(std::string const&  output_file_pathname, special_section::elf_tls_ptr const  data)
{
    ASSUMPTION(data.operator bool());

    std::ofstream  ostr{output_file_pathname,std::ofstream::binary};

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
    ostr << "<table>\n";
    ostr << "  <caption>Thread-local section info. All numbers are hexadecimal.</caption>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Property</th>\n";
    ostr << "    <th>Value</th>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Initialisation image start address</td>\n";
    ostr << "    <td>" << std::hex << data->start_address() << "</td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Initialisation image end address</td>\n";
    ostr << "    <td>" << std::hex << data->image_end_address() << "</td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Initialisation image size</td>\n";
    ostr << "    <td>" << std::hex << data->image_end_address() - data->start_address() << "</td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Template end address</td>\n";
    ostr << "    <td>" << std::hex << data->end_address() << "</td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>File offset</td>\n";
    ostr << "    <td>" << std::hex << data->file_offset() << "</td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Allignment in the memory</td>\n";
    ostr << "    <td>" << std::hex << data->alignment() << "</td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Flags</td>\n";
    ostr << "    <td>" << std::hex << data->flags() << "</td>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";
    ostr << "    <td>Use static TLS scheme?</td>\n";
    ostr << "    <td>" << std::boolalpha << data->use_static_scheme() << "</td>\n";
    ostr << "  </tr>\n";


    ostr << "</table>\n";
    ostr << "</body>\n";
    ostr << "</html>\n";
}


void  dump_html_special_section_load_configuration_structure(
        std::string const&  output_file_pathname,
        special_section::load_configuration_structure_ptr const  data)
{
    ASSUMPTION(data.operator bool());

    std::ofstream  ostr{output_file_pathname,std::ofstream::binary};

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
    ostr << "<p>\n";
    ostr << "All integer numeric values are hexadecimal.\n";
    ostr << "</p>\n";
    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Property</th>\n";
    ostr << "    <th>Value</th>\n";
    ostr << "  </tr>\n";

    ostr << "  <tr>\n";
    ostr << "    <td>Time stamp</td>\n";
    ostr << "    <td>";

    char buffer[64];
    ostr << std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::gmtime(&data->timestamp()));

    ostr << "</td>\n";
    ostr << "  </tr>\n";

    ostr << "  <tr>\n";
    ostr << "    <td>Major version</td>\n";
    ostr << "    <td>";
    ostr << std::hex << data->major_version();
    ostr << "</td>\n";
    ostr << "  </tr>\n";

    ostr << "  <tr>\n";
    ostr << "    <td>Minor version</td>\n";
    ostr << "    <td>";
    ostr << std::hex << data->minor_version();
    ostr << "</td>\n";
    ostr << "  </tr>\n";

    ostr << "  <tr>\n";
    ostr << "    <td>Global clear flags</td>\n";
    ostr << "    <td>";
    ostr << std::hex << data->global_clear_flags();
    ostr << "</td>\n";
    ostr << "  </tr>\n";

    ostr << "  <tr>\n";
    ostr << "    <td>Global set flags</td>\n";
    ostr << "    <td>";
    ostr << std::hex << data->global_set_flags();
    ostr << "</td>\n";
    ostr << "  </tr>\n";

    ostr << "  <tr>\n";
    ostr << "    <td>Critical section timeout</td>\n";
    ostr << "    <td>";
    ostr << std::hex << data->critical_section_timeout();
    ostr << "</td>\n";
    ostr << "  </tr>\n";

    ostr << "  <tr>\n";
    ostr << "    <td>Decommit block thresold</td>\n";
    ostr << "    <td>";
    ostr << std::hex << data->decommit_block_thresold();
    ostr << "</td>\n";
    ostr << "  </tr>\n";

    ostr << "  <tr>\n";
    ostr << "    <td>Decommit free thresold</td>\n";
    ostr << "    <td>";
    ostr << std::hex << data->decommit_free_thresold();
    ostr << "</td>\n";
    ostr << "  </tr>\n";

    ostr << "  <tr>\n";
    ostr << "    <td>Lock prefix table</td>\n";
    ostr << "    <td>";
    ostr << std::hex << data->lock_prefix_table();
    ostr << "</td>\n";
    ostr << "  </tr>\n";

    ostr << "  <tr>\n";
    ostr << "    <td>Max alloc size</td>\n";
    ostr << "    <td>";
    ostr << std::hex << data->max_alloc_size();
    ostr << "</td>\n";
    ostr << "  </tr>\n";

    ostr << "  <tr>\n";
    ostr << "    <td>Memory thresold</td>\n";
    ostr << "    <td>";
    ostr << std::hex << data->memory_thresold();
    ostr << "</td>\n";
    ostr << "  </tr>\n";

    ostr << "  <tr>\n";
    ostr << "    <td>Process affinity mask</td>\n";
    ostr << "    <td>";
    ostr << std::hex << data->process_affinity_mask();
    ostr << "</td>\n";
    ostr << "  </tr>\n";

    ostr << "  <tr>\n";
    ostr << "    <td>Process heap flags</td>\n";
    ostr << "    <td>";
    ostr << std::hex << data->process_heap_flags();
    ostr << "</td>\n";
    ostr << "  </tr>\n";

    ostr << "  <tr>\n";
    ostr << "    <td>Service pack version</td>\n";
    ostr << "    <td>";
    ostr << std::hex << data->service_pack_version();
    ostr << "</td>\n";
    ostr << "  </tr>\n";

    ostr << "  <tr>\n";
    ostr << "    <td>Security cookie</td>\n";
    ostr << "    <td>";
    ostr << std::hex << data->security_cookie();
    ostr << "</td>\n";
    ostr << "  </tr>\n";

    ostr << "  <tr>\n";
    ostr << "    <td>Strucutre exception handlers</td>\n";
    ostr << "    <td>";
    if (!data->structured_exception_handlers().empty())
    {
        bool  the_first = true;
        for (address const adr : data->structured_exception_handlers())
        {
            ostr << std::hex << (the_first ? "" : ", ") << adr;
            the_first = false;
        }
    }
    else
        ostr << "NONE";
    ostr << "</td>\n";
    ostr << "  </tr>\n";

    ostr << "</table>\n";
    ostr << "</body>\n";
    ostr << "</html>\n";
}

void  dump_html_special_section_exceptions(std::string const&  output_file_pathname,
                                           special_section::exceptions_ptr const  data)
{
    ASSUMPTION(data.operator bool());

    std::ofstream  ostr{output_file_pathname,std::ofstream::binary};

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
    ostr << "<p>\n";
    ostr << "All numbers are hexadecimal.\n";
    ostr << "</p>\n";

    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Start address</th>\n";
    ostr << "    <th>End address</th>\n";
    ostr << "    <th>Unwind info address</th>\n";
    ostr << "  </tr>\n";

    for (uint64_t  i = 0ULL; i < data->num_records(); ++i)
    {
        ostr << "  <tr>\n";

        ostr << "    <td>";
        ostr << std::hex << data->start_address(i);
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << std::hex << data->end_address(i);
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << std::hex << data->unwind_info_address(i);
        ostr << "</td>\n";

        ostr << "  </tr>\n";
    }
    ostr << "</table>\n";

    ostr << "</body>\n";
    ostr << "</html>\n";
}


void  dump_symbol_table(symbol_table const&  table, std::string const&  filename)
{
    std::ofstream  ostr{filename,std::ofstream::binary};

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

    ostr << "<table>\n";
    ostr << "  <caption>All numbers are hexadecimal.</caption>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>File</th>\n";
    ostr << "    <th>Index</th>\n";
    ostr << "    <th>Symbol ID</th>\n";
    ostr << "    <th>Reloc. type</th>\n";
    ostr << "    <th>Symbol value</th>\n";
    ostr << "    <th>Symbol size</th>\n";
    ostr << "  </tr>\n";

    std::set<symbol_table::key_type>  keys;
    for (auto const& elem : table)
        keys.insert(elem.first);
    for (auto const& key : keys)
    {
        auto const  it = table.find(key);
        INVARIANT(it != table.cend());
        ostr << "  <tr>\n";
        ostr << "    <td>" << it->first.first << "</td>\n";
        ostr << "    <td>" << std::hex << it->first.second << "</td>\n";
        ostr << "    <td>" << std::get<0>(it->second) << "</td>\n";
        ostr << "    <td>" << std::get<1>(it->second) << "</td>\n";
        ostr << "    <td>" << std::hex << std::get<2>(it->second) << "</td>\n";
        ostr << "    <td>" << std::hex << std::get<3>(it->second) << "</td>\n";
        ostr << "  </tr>\n";
    }

    ostr << "</table>\n";
    ostr << "</body>\n";
    ostr << "</html>\n";
}


}}

namespace loader {


void  dump_dot(dependencies_graph_ptr const graph, std::string const&  output_file_pathname)
{
    std::ofstream  ostr{output_file_pathname,std::ofstream::binary};
    ostr << "digraph loader_dependencies_graph {\n";
    ostr << "    graph [fontname=\"Liberation serif\""
         //<< ", fontsize=\"12px\""
         << ", ratio=0.5"
         << ", size=\"18,18\""
         << "];\n"
         ;
    ostr << "    node [shape=\"box\", fontname=\"Liberation serif\""
         //<< ", fontsize=\"12px\""
         << "];\n"
         ;
    ostr << "    edge [fontname=\"Liberation serif\""
         //<< ", fontsize=\"12px\""
         << "];\n"
         ;
    ostr << "    \"" << graph->root_file() << "\"[style=\"rounded\"];\n";
    for (auto const& elem : *graph->forward_dependencies())
    {
        std::string const&  src_file = elem.first;
        for (auto const& dst_file : elem.second)
            ostr << "    \"" << src_file << "\" -> \"" << dst_file << "\";\n";
    }
    ostr << "}\n";
}

  void  dump_html(descriptor_ptr const  data, std::string const&  output_file_pathname,
                bool const  dump_also_contents_of_sections)
{
    std::vector<address>  addresses;
    if (dump_also_contents_of_sections)
    {
        for (auto const& elem : *data->sections_table())
            addresses.push_back(elem.first);
    }
    dump_html(data,output_file_pathname,addresses);
}

void  dump_html(descriptor_ptr const  data, std::string const&  output_file_pathname,
                std::vector<address> const&  start_addresses_of_sections_whose_content_should_be_dumped)
{
    ASSUMPTION(!is_directory(output_file_pathname));

    create_directory(parse_path_in_pathname(output_file_pathname));

    std::ofstream  ostr{output_file_pathname,std::ofstream::binary};

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
    ostr << "<h1>Dump of file descriptor</h1>\n";

    ostr << "<h2>Platform</h2>\n";
    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Processor architecture</th>\n";
    ostr << "    <th>ABI name</th>\n";
    ostr << "  </tr>\n";
    ostr << "  <tr>\n";

    ostr << "    <td>";
    ostr << std::hex << data->platform()->architecture();
    ostr << "</td>\n";

    ostr << "    <td>";
    ostr << std::hex << data->platform()->abi();
    ostr << "</td>\n";

    ostr << "  </tr>\n";
    ostr << "</table>\n";

    ostr << "<h2>Loaded binaries</h2>\n";
    //ostr << "<table style=\"width:100%\">\n";
    ostr << "<p>\n";
    ostr << "All numbers are hexadecimal. Warnings (if any) represent those violation of a specification\n"
            "of a file format or OS ABI which do not prevent the operating system from a successful load and\n"
            "the start of process's execution.\n";
    ostr << "</p>\n";
    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>File path</th>\n";
    ostr << "    <th>Format</th>\n";
    ostr << "    <th>Uses big endian</th>\n";
    ostr << "    <th>Adress mode</th>\n";
    ostr << "    <th>ID</th>\n";

    std::set<std::string>  prop_names;
    for (auto const& elem : *data->files_table())
        for (auto const& prop : *elem.second->property_map())
            prop_names.insert(prop.first);
    for (auto const& prop : prop_names)
        ostr << "    <th>" << prop << "</th>\n";

    ostr << "    <th>Warnings</th>\n";
    ostr << "  </tr>\n";
    for (auto const& elem : *data->files_table())
    {
        file_props_ptr const  fprops = elem.second;

        ostr << "  <tr>\n";

        ostr << "    <td>";
        ostr << fprops->path();
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << fprops->format();
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << (fprops->is_in_big_endian() ? "Yes" : "No");
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << (uint32_t)fprops->num_address_bits();
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << fprops->id();
        ostr << "</td>\n";

        for (auto const& prop : prop_names)
        {
            ostr << "    <td>";
            auto const  it = fprops->property_map()->find(prop);
            ostr << (it != fprops->property_map()->cend() ? it->second : "NONE");
            ostr << "</td>\n";
        }

        ostr << "    <td>";

        if (data->warnings()->count(fprops->path()) > 0ULL)
        {
            std::string  output_name;
            {
                std::string  tmp{ parse_name_in_pathname(fprops->path()) +
                                  parse_path_in_pathname(fprops->path()) };
                std::replace(tmp.begin(),tmp.end(),'/','_');
                std::replace(tmp.begin(),tmp.end(),'.','_');
                output_name = std::string{"./warnings_"} + tmp + ".html";
            }
            ostr << " <a href=\"" << output_name << "\">"
                 << std::hex << data->warnings()->find(fprops->path())->second.size()
                 << "</a>";

            std::string const  output_pathname =
                    concatenate_file_paths(
                            parse_path_in_pathname(output_file_pathname),
                            output_name
                            );
            std::ofstream  ostr{output_pathname,std::ofstream::binary};

            dump_html_warnings(ostr,data,fprops);
        }
        else
            ostr << "0";
        ostr << "</td>\n";

        ostr << "  </tr>\n";
    }
    ostr << "</table>\n";

    ostr << "<h2>Skipped binaries</h2>\n";
    if (data->skipped_files()->empty())
        ostr << "<p>None.</p>\n";
    else
    {
        ostr << "<table>\n";
        ostr << "  <tr>\n";
        ostr << "    <th>File name</th>\n";
        ostr << "  </tr>\n";
        for (auto const& filename : *data->skipped_files())
        {
            ostr << "  <tr>\n";

            ostr << "    <td>";
            ostr << filename;
            ostr << "</td>\n";

            ostr << "  </tr>\n";
        }
        ostr << "</table>\n";
    }

    ostr << "<h2>dependencies of loaded files</h2>\n";
    ostr << "<p>\n";
    ostr << "The root binary file is drawn with the rounded box.\n";
    ostr << "</p>\n";
    {
        std::string const  dot_file = output_file_pathname + ".dep_loaded.dot";
        std::string const  extension = "svg";
        std::string const  img_file = output_file_pathname + ".dep_loaded." + extension;
        std::string const  filename = parse_name_in_pathname(output_file_pathname) + ".dep_loaded." + extension;
        std::string const  command = std::string("dot -T") + extension + " \"" + dot_file + "\" -o \"" + img_file + "\"";
        dump_dot(data->dependencies_of_loaded_files(),dot_file);
        auto ignore = std::system(command.c_str());
        (void)ignore;
        ostr << "<img src=\"./" << filename << "\" alt=\"" << filename << "\">\n";
    }

    if (!data->skipped_files()->empty())
    {
        ostr << "<h2>dependencies of all files</h2>\n";
        ostr << "<p>\n";
        ostr << "The root binary file is drawn with the rounded box.\n";
        ostr << "</p>\n";
        {
            std::string const  dot_file = output_file_pathname + ".dep_all.dot";
            std::string const  extension = "svg";
            std::string const  img_file = output_file_pathname + ".dep_all." + extension;
            std::string const  filename = parse_name_in_pathname(output_file_pathname) + ".dep_all." + extension;
            std::string const  command = std::string("dot -T") + extension + " \"" + dot_file + "\" -o \"" + img_file + "\"";
            dump_dot(data->dependencies_of_all_files(),dot_file);
            auto ignore = std::system(command.c_str());
            (void)ignore;
            ostr << "<img src=\"./" << filename << "\" alt=\"" << filename << "\">\n";
        }
    }

    ostr << "<h2>Access points</h2>\n";
    ostr << "<p>\n";
    ostr << "The entry point to the root binary: ";
    ostr << std::hex << data->entry_point();
    ostr << "</p>\n";
    std::set<std::string>  init_fini_files;
    for (auto const&  elem : *data->init_functions())
        init_fini_files.insert(elem.first);
    for (auto const&  elem : *data->fini_functions())
        init_fini_files.insert(elem.first);
    if (init_fini_files.empty())
        ostr << "<p>There are no initialisation nor termination functions in the loaded binaries.</p>\n";
    else
    {
        ostr << "<p>\n";
        ostr << "For each binary file both init and fini function addresses are presented in order\n";
        ostr << "in which they are supposed to be called. Use the dependancy graph above to choose\n";
        ostr << "an order in which functions of individual files are called (there can be several\n";
        ostr << "correct orders; leaves are taken first, the root is the last).\n";
        ostr << "The last init function of the root binary is the entry point presented above.\n";
        ostr << "All addresses in the table are hexadecimal.\n";
        ostr << "</p>\n";
        ostr << "<table>\n";
        ostr << "  <tr>\n";
        ostr << "    <th>File path</th>\n";
        ostr << "    <th>Init function addresses</th>\n";
        ostr << "    <th>Fini function addresses</th>\n";
        ostr << "  </tr>\n";
        for (auto const&  file : init_fini_files)
        {
            ostr << "  <tr>\n";

            ostr << "    <td>";
            ostr << file;
            ostr << "</td>\n";

            auto const  init_it = data->init_functions()->find(file);
            if (init_it != data->init_functions()->cend())
            {
                ostr << "    <td>";
                uint64_t  counter = 0ULL;
                for (auto const adr : init_it->second)
                    ostr << std::hex << adr << (++counter == init_it->second.size() ? "" : ", ");
                ostr << "</td>\n";
            }

            auto const  fini_it = data->fini_functions()->find(file);
            if (fini_it != data->fini_functions()->cend())
            {
                ostr << "    <td>";
                uint64_t  counter = 0ULL;
                for (auto const adr : fini_it->second)
                    ostr << std::hex << adr << (++counter == fini_it->second.size() ? "" : ", ");
                ostr << "</td>\n";
            }

            ostr << "  </tr>\n";
        }
        ostr << "</table>\n";
    }

    ostr << "<h2>Relocations</h2>\n";
    ostr << "<p>\n";
    ostr << "There were performed " << data->performed_relocations()->size()
         << " relocations in total (i.e. in all loaded binaries).\n";
    if (data->performed_relocations()->size() > 0ULL)
    {
        ostr << " The complete list can be found\n";
        ostr << "<a href=\"./relocations_performed.html\">here</a>.";
        std::string const  output_pathname =
                concatenate_file_paths(
                        parse_path_in_pathname(output_file_pathname),
                        "relocations_performed.html"
                        );
        std::ofstream  ostr{output_pathname,std::ofstream::binary};

        dump_html_relocations(ostr,data);
    }
    ostr << "<p>\n";
    ostr << "There were skipped " << data->skipped_relocations()->size()
         << " relocations in total (i.e. in all loaded binaries).\n";
    if (data->skipped_relocations()->size() > 0ULL)
    {
        ostr << "A typical reason for the skip is a reference to a binary,\n"
                " which was not loaded. The complete list can be found\n";
        ostr << "<a href=\"./relocations_skipped.html\">here</a>.";
        std::string const  output_pathname =
                concatenate_file_paths(
                        parse_path_in_pathname(output_file_pathname),
                        "relocations_skipped.html"
                        );
        std::ofstream  ostr{output_pathname,std::ofstream::binary};

        dump_html_skipped_relocations(ostr,data);
    }
    ostr << "</p>\n";
    ostr << "<p>\n";
    ostr << "Note that we also present relocations (both performed and skipped) per section basis\n"
            "<a href=\"./" << parse_name_in_pathname(output_file_pathname) << "#sections_list\">below</a>.";
    ostr << "</p>\n";

    if (data->visible_symbol_table().operator bool() && !data->visible_symbol_table()->empty())
    {
        std::string const  filename =
                concatenate_file_paths(
                        parse_path_in_pathname(output_file_pathname),
                        "visible_symbol_table.html"
                        );
        dump_symbol_table(*data->visible_symbol_table(),filename);
        ostr << "<p>\n";
        ostr << "Table of " << std::dec << data->visible_symbol_table()->size() << " symbols is located \n"
                "<a href=\"./visible_symbol_table.html\">here</a>.";
        ostr << "</p>\n";
    }

    if (data->hidden_symbol_table().operator bool() && !data->hidden_symbol_table()->empty())
    {
        std::string const  filename =
                concatenate_file_paths(
                        parse_path_in_pathname(output_file_pathname),
                        "hidden_symbol_table.html"
                        );
        dump_symbol_table(*data->hidden_symbol_table(),filename);
        ostr << "<p>\n";
        ostr << "Table of " << std::dec << data->hidden_symbol_table()->size() << " hidden symbols is located \n"
                "<a href=\"./hidden_symbol_table.html\">here</a>.";
        ostr << "</p>\n";
    }

    std::map<address,uint64_t>  num_performed_relocations_per_section;
    dump_html_per_section_relocations(output_file_pathname,data,start_addresses_of_sections_whose_content_should_be_dumped,
                                      num_performed_relocations_per_section);

    std::map<address,uint64_t>  num_skipped_relocations_per_section;
    dump_html_per_section_skipped_relocations(output_file_pathname,data,start_addresses_of_sections_whose_content_should_be_dumped,
                                              num_skipped_relocations_per_section);

    dump_html_contents_of_sections(output_file_pathname,data,start_addresses_of_sections_whose_content_should_be_dumped);

    ostr << "<h2 id=\"sections_list\">Sections</h2>\n";
    ostr << "<p>\n";
    ostr << "All numbers are hexadecimal.";
    ostr << "</p>\n";
    ostr << "<table>\n";
    ostr << "  <tr>\n";
    ostr << "    <th>Start address</th>\n";
    ostr << "    <th>End address</th>\n";
    ostr << "    <th>Readable</th>\n";
    ostr << "    <th>Writable</th>\n";
    ostr << "    <th>Executable</th>\n";
    ostr << "    <th>Uses big endian</th>\n";
    ostr << "    <th>Const endian</th>\n";
    ostr << "    <th>File path</th>\n";
    ostr << "    <th>Offset</th>\n";
    ostr << "    <th>Virtual address</th>\n";
    ostr << "    <th>File bytes</th>\n";
    ostr << "    <th>Memory bytes</th>\n";
    ostr << "    <th>In file alignment</th>\n";
    ostr << "    <th>In memory alignment</th>\n";
    ostr << "    <th>Content</th>\n";
    ostr << "    <th>Relocations performed</th>\n";
    ostr << "    <th>Relocations skipped</th>\n";
    ostr << "  </tr>\n";
    for (auto const&  elem : *data->sections_table())
    {
        ostr << "  <tr>\n";

        section_ptr const  ptr = elem.second;

        ostr << "    <td>";
        ostr << std::hex << ptr->start_address();
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << std::hex << ptr->end_address();
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << (ptr->has_read_access() ? "Yes" : "No");
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << (ptr->has_write_access() ? "Yes" : "No");
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << (ptr->has_execute_access() ? "Yes" : "No");
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << (ptr->is_in_big_endian() ? "Yes" : "No");
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << (ptr->has_const_endian() ? "Yes" : "No");
        ostr << "</td>\n";

        file_props_ptr const  fprops = ptr->file_props();

        ostr << "    <td>";
        ostr << fprops->path();
        ostr << "</td>\n";

        section_file_header_ptr const  header = ptr->section_file_header();

        ostr << "    <td>";
        ostr << std::hex << header->offset();
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << std::hex << header->virtual_address();
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << std::hex << header->size_in_file();
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << std::hex << header->size_in_memory();
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << std::hex << header->in_file_align();
        ostr << "</td>\n";

        ostr << "    <td>";
        ostr << std::hex << header->in_memory_align();
        ostr << "</td>\n";

        ostr << "    <td>";
        if (std::find(start_addresses_of_sections_whose_content_should_be_dumped.cbegin(),
                      start_addresses_of_sections_whose_content_should_be_dumped.cend(),
                      ptr->start_address()) !=
                start_addresses_of_sections_whose_content_should_be_dumped.cend())
            ostr << "<a href=\"./section_content_"
                 << std::hex << ptr->start_address() << "_"
                 << std::hex << ptr->end_address() << ".html\">"
                 << "here</a>";
        ostr << "</td>\n";

        ostr << "    <td>";
        if (num_performed_relocations_per_section[ptr->start_address()] > 0ULL)
             ostr << " <a href=\"./relocations_performed_"
                  << std::hex << ptr->start_address() << "_"
                  << std::hex << ptr->end_address() << ".html\">"
                  << num_performed_relocations_per_section[ptr->start_address()]
                  << "</a>";
        else
            ostr << "0";
        ostr << "</td>\n";

        ostr << "    <td>";
        if (num_skipped_relocations_per_section[ptr->start_address()] > 0ULL)
             ostr << " <a href=\"./relocations_skipped_"
                  << std::hex << ptr->start_address() << "_"
                  << std::hex << ptr->end_address() << ".html\">"
                  << num_skipped_relocations_per_section[ptr->start_address()]
                  << "</a>";
        else
            ostr << "0";
        ostr << "</td>\n";

        ostr << "  </tr>\n";
    }
    ostr << "</table>\n";

    if (!data->special_sections()->empty())
    {
        ostr << "<h2 id=\"special_sections\">Special sections</h2>\n";
        ostr << "<p>\n";
        ostr << "Here are sections, which are either ABI specific, or they\n"
                "might be useful later during execution of the process.\n"
                "All numbers are hexadecimal.\n";
        ostr << "</p>\n";
        ostr << "<table>\n";
        ostr << "  <tr>\n";
        ostr << "    <th>File name</th>\n";
        ostr << "    <th>Section name</th>\n";
        ostr << "    <th>Start address</th>\n";
        ostr << "    <th>End address</th>\n";
        ostr << "    <th>Size</th>\n";
        ostr << "    <th>Details</th>\n";
        ostr << "    <th>Description</th>\n";
        ostr << "  </tr>\n";
        for (auto const& felem : *data->special_sections())
            for (auto const& elem : felem.second)
            {
                ostr << "  <tr>\n";

                ostr << "    <td>";
                ostr << felem.first;
                ostr << "</td>\n";

                std::string const&  section_name = elem.first;
                special_section_properties_ptr const  props = elem.second;

                ostr << "    <td>";
                ostr << section_name;
                ostr << "</td>\n";

                ostr << "    <td>";
                ostr << std::hex << props->start_address();
                ostr << "</td>\n";

                ostr << "    <td>";
                ostr << std::hex << props->end_address();
                ostr << "</td>\n";

                ostr << "    <td>";
                ostr << std::hex << props->end_address() - props->start_address();
                ostr << "</td>\n";

                std::string  details_filename;
                {
                    std::stringstream  filename;
                    filename << "special_section_"
                             << std::hex << props->start_address() << "_"
                             << std::hex << props->end_address() << ".html";
                    details_filename = filename.str();

                    std::string const  output_section_file_pathname =
                            concatenate_file_paths(
                                    parse_path_in_pathname(output_file_pathname),
                                    details_filename
                                    );

                    if (section_name == special_section_name::thread_local_storage())
                    {
                        if (special_section::elf_tls_ptr const  p = std::dynamic_pointer_cast<special_section::elf_tls const>(props))
                            dump_html_special_section_elf_tls(output_section_file_pathname,p);
                        else
                            details_filename.clear();
                    }
                    else if (section_name == special_section_name::resources())
                        details_filename.clear();
                    else if (section_name == special_section_name::load_configuration_structure())
                        dump_html_special_section_load_configuration_structure(
                                output_section_file_pathname,
                                std::dynamic_pointer_cast<special_section::load_configuration_structure const>(props)
                                );
                    else if (section_name == special_section_name::exceptions())
                        dump_html_special_section_exceptions(
                                output_section_file_pathname,
                                std::dynamic_pointer_cast<special_section::exceptions const>(props)
                                );
                    else
                        details_filename.clear();
                }

                ostr << "    <td>";
                if (!details_filename.empty())
                    ostr << " <a href=\"./" << details_filename << "\">here</a>";
                ostr << "</td>\n";

                ostr << "    <td>";
                ostr << std::hex << props->description();
                ostr << "</td>\n";

                ostr << "  </tr>\n";
            }
        ostr << "</table>\n";
    }

    ostr << "</body>\n";
    ostr << "</html>\n";

}


}
