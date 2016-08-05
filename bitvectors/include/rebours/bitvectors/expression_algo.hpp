#ifndef REBOURS_BITVECTORS_EXPRESSION_ALGO_HPP_INCLUDED
#   define REBOURS_BITVECTORS_EXPRESSION_ALGO_HPP_INCLUDED

#   include <rebours/bitvectors/expression.hpp>
#   include <functional>
#   include <unordered_set>
#   include <vector>

namespace bv {


bool  find_symbols(expression const  e, std::function<bool(symbol)> const  selector,
                   std::unordered_set<symbol,symbol::hash>&  output);

bool  find_unintepreted_symbols(expression const  e, std::unordered_set<symbol,symbol::hash>&  output);


expression  to_conjunction(std::vector<expression> const&  conjuncts);


}

#endif
