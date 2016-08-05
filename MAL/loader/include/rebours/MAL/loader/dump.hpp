#ifndef REBOURS_MAL_LOADER_DUMP_HPP_INCLUDED
#   define REBOURS_MAL_LOADER_DUMP_HPP_INCLUDED

#   include <rebours/MAL/loader/descriptor.hpp>
#   include <string>

namespace loader {

/**
 * The following two functions read a passed descriptor 'data' and create one or more HTML file
 * on the disc showing the content of the descriptor.
 */

void  dump_html(descriptor_ptr const  data,
                        //!< A descriptor of loaded binaries whose content will be dumped.
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

void  dump_html(descriptor_ptr const  data,
                        //!< A descriptor of loaded binaries whose content will be dumped.
                std::string const&  output_file_pathname,
                        //!< A path-name of the root HTML file into which the descriptor will be
                        //!< dumped. Other generated HTML files are referenced from the root file
                        //!< and they all will be created in the same directory as the root file.
                        //!< If any of the created files already exists on the disc, then they
                        //!< will be overwritten without any notification.
                std::vector<address> const&  start_addresses_of_sections_whose_content_should_be_dumped
                        //!< It allows for dump of content of selected sections. A content of
                        //!< a section is an array of individual bytes of the section.
                );


/**
 * It creates Graphviz's .dot file on the disc whose content is the passed dependencies graph.
 */
void  dump_dot(dependencies_graph_ptr const  graph,
                       //!< A dependencies graph which will be dumped.
               std::string const&  output_file_pathname
                       //!< A path-name of Graphviz's .dot file into which the dependencies graph
                       //!< will be dumped. If the file already exists on the disc, then it will be
                       //!< overwritten without any notification.
               );


}

#endif
