#include <rebours/bitvectors/expression_algo.hpp>

namespace bv {


bool  find_symbols(expression const  e, std::function<bool(symbol)> const  selector,
                   std::unordered_set<symbol,symbol::hash>&  output)
{
    if (selector(get_symbol(e)))
        output.insert(get_symbol(e));
    for (uint64_t  i = 0ULL; i < num_parameters(e); ++i)
        find_symbols(argument(e,i),selector,output);
    return !output.empty();
}

bool  find_unintepreted_symbols(expression const  e, std::unordered_set<symbol,symbol::hash>&  output)
{
    return find_symbols(e,std::not1(std::function<bool(symbol)>(&symbol_is_interpreted)),output);
}

expression  to_conjunction(std::vector<expression> const&  conjuncts)
{
    if (conjuncts.empty())
        return tt();
    expression  result = conjuncts.front();
    for (auto it = std::next(conjuncts.cbegin()); it != conjuncts.cend(); ++it)
        result = result && *it;
    return result;
}


}
