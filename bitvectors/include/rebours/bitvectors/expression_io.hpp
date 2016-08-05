#ifndef REBOURS_BITVECTORS_EXPRESSION_IO_HPP_INCLUDED
#   define REBOURS_BITVECTORS_EXPRESSION_IO_HPP_INCLUDED

#   include <rebours/bitvectors/expression.hpp>
#   include <rebours/bitvectors/assumptions.hpp>
#   include <unordered_map>
#   include <iosfwd>
#   include <string>
#   include <vector>
#   include <cstdint>
#   include <stdexcept>

namespace bv {


void  save_in_smtlib2_format(std::ostream&  ostr, expression const  e);

inline std::ostream&  operator <<(std::ostream&  ostr, expression const  e) { save_in_smtlib2_format(ostr,e); return ostr; }

template<typename T>
inline std::ostream&  operator <<(std::ostream&  ostr, typed_expression<T> const  e)
{
    return ostr << e.operator expression();
}


expression  load_in_smtlib2_format(std::istream&  istr, std::string&  error);

std::istream&  operator >>(std::istream&  istr, expression&  e);


}

namespace bv { namespace detail {


std::unordered_map<std::string,std::string> const&  operators_outside_QF_UFBV();
std::unordered_map<std::string,std::string> const&  operators_to_QF_UFBV_names();


struct QF_UFBV_operator_props
{
    QF_UFBV_operator_props(std::string const&  name,
                           std::vector<uint64_t> const&  num_bits_of_parameters);

    std::string const&  name() const noexcept { return m_name; }
    uint64_t  num_parameters() const { return m_num_bits_of_parameters.size(); }
    uint64_t  num_bits_of_parameter(uint64_t const  param_index) const { return m_num_bits_of_parameters.at(param_index); }

    static std::size_t  hasher(QF_UFBV_operator_props const&  props);
    static bool  equal(QF_UFBV_operator_props const&  p1, QF_UFBV_operator_props const&  props2);

private:
    std::string  m_name;
    std::vector<uint64_t>  m_num_bits_of_parameters;
};


std::unordered_map<QF_UFBV_operator_props,
                   bv::symbol,
                   decltype(&bv::detail::QF_UFBV_operator_props::hasher),
                   decltype(&bv::detail::QF_UFBV_operator_props::equal)> const&
QF_UFBV_names_to_symbols();

std::unordered_map<std::string,std::string> const&  inverted_operators_outside_QF_UFBV();


struct smtlib2_token
{
    smtlib2_token(std::string const&  token, uint32_t const  line, uint32_t const  column)
        : m_token(token)
        , m_line(line)
        , m_column(column)
    {}

    std::string const&  name() const noexcept { return m_token; }
    uint32_t  line() const noexcept { return m_line; }
    uint32_t  column() const noexcept { return m_column; }
private:
    std::string  m_token;
    uint32_t  m_line;
    uint32_t  m_column;
};

std::string  to_string(smtlib2_token const&  token);
inline std::ostream&  operator <<(std::ostream&  ostr, smtlib2_token const&  token) { return ostr << to_string(token); }

void  tokenise_smtlib2_stream(std::istream&  istr, std::vector<smtlib2_token>&  output);


struct smtlib2_ast_node
{
    smtlib2_ast_node(smtlib2_token const&  token, std::vector<smtlib2_ast_node> const&  children)
        : m_token(token)
        , m_children(children)
    {
        ASSUMPTION(token.name().empty() || children.empty());
    }

    smtlib2_token const&  token() const noexcept { return m_token; }
    std::vector<smtlib2_ast_node> const&  children() const noexcept { return m_children; }
private:
    smtlib2_token  m_token;
    std::vector<smtlib2_ast_node>  m_children;
};

std::string  to_string(smtlib2_ast_node const&  node);
inline std::ostream&  operator <<(std::ostream&  ostr, smtlib2_ast_node const&  node) { return ostr << to_string(node); }

std::vector<uint64_t>  to_ret_numbits(std::vector<expression> const&  parameters);

std::string  tokenised_smtlib2_stream_to_ast(std::vector<smtlib2_token> const&  tokens,
                                             std::vector<smtlib2_ast_node>&  output);


uint64_t  compute_numbits_from_smtlib2_type_ast(smtlib2_ast_node const&  ast, std::string&  error);

expression  build_expression_from_smtlib2_ast(detail::smtlib2_ast_node const&  ast,
                                              std::unordered_map<std::string,symbol> const&  ufunctions,
                                              std::string&  error);


}}

#endif
