#ifndef REBOURS_MAL_DESCRIPTOR_DUMP_HPP_INCLUDED
#   define REBOURS_MAL_DESCRIPTOR_DUMP_HPP_INCLUDED

#   include <rebours/MAL/descriptor/storage.hpp>
#   include <string>

namespace mal { namespace descriptor {

/**
 * The following two functions read a passed descriptor 'data' and create one or more HTML file
 * on the disc showing the content of the descriptor.
 */

void  dump_html(storage const&  data,
                std::string const&  output_file_pathname,
                        //!< A path-name of the root HTML file into which the descriptor will be
                        //!< dumped. Other generated HTML files are referenced from the root file
                        //!< and they all will be created in the same directory as the root file.
                        //!< If any of the created files already exists on the disc, then they
                        //!< will be overwritten without any notification.
                bool const  dump_also_contents_of_sections = false
                        //!< It allows for dump of content of either all or no section. A content of
                        //!< a section is an array of individual bytes of the section.
                );

void  dump_html(storage const&  data,
                std::string const&  output_file_pathname,
                        //!< A path-name of the root HTML file into which the descriptor will be
                        //!< dumped. Other generated HTML files are referenced from the root file
                        //!< and they all will be created in the same directory as the root file.
                        //!< If any of the created files already exists on the disc, then they
                        //!< will be overwritten without any notification.
                std::vector<loader::address> const&  start_addresses_of_sections_whose_content_should_be_dumped
                        //!< It allows for dump of content of selected sections. A content of
                        //!< a section is an array of individual bytes of the section.
                );


}}

#endif
