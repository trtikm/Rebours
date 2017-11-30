#include <rebours/program/assembly.hpp>
#include <rebours/utility/assumptions.hpp>
#include <rebours/utility/invariants.hpp>
#include <algorithm>
#include <iterator>

namespace microcode {


void  append(annotations const&  src, annotations&  dst)
{
    for (auto const&  node_ans : src)
    {
        annotations::iterator const  nit = dst.find(node_ans.first);
        if (nit == dst.end())
            dst.insert(node_ans);
        else
            std::copy(node_ans.second.cbegin(),node_ans.second.cend(),std::back_inserter(nit->second));
    }
}

node_annotations::const_iterator  find(node_annotations const&  A, std::string const&  keyword)
{
    for (auto it = A.cbegin(); it != A.cend(); ++it)
        if (it->first == keyword)
            return it;
    return A.cend();
}

node_annotations::iterator  find(node_annotations&  A, std::string const&  keyword)
{
    for (auto it = A.begin(); it != A.end(); ++it)
        if (it->first == keyword)
            return it;
    return A.end();
}

std::string const*  find(annotations const&  A, program_component::node_id const  node, std::string const&  keyword)
{
    annotations::const_iterator const  nit = A.find(node);
    if (nit == A.end())
        return nullptr;
    node_annotations::const_iterator  ait = find(nit->second,keyword);
    return ait == nit->second.end() ? nullptr : &ait->second;
}

std::string*  find(annotations&  A, program_component::node_id const  node, std::string const&  keyword)
{
    annotations::iterator const  nit = A.find(node);
    if (nit == A.end())
        return nullptr;
    node_annotations::iterator  ait = find(nit->second,keyword);
    return ait == nit->second.end() ? nullptr : &ait->second;
}


std::unique_ptr<annotations>  create_initial_annotations()
{
    return std::unique_ptr<annotations>( new annotations );
}


}
