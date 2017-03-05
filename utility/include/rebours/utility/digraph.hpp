#ifndef DIGRAPH_HPP_INCLUDED
#   define DIGRAPH_HPP_INCLUDED

#   include <rebours/utility/assumptions.hpp>
#   include <unordered_map>
#   include <unordered_set>
#   include <vector>
#   include <algorithm>
#   include <type_traits>
#   include <cstdint>


namespace detail {


template<typename key_type__>
struct digraph_hash_func
{
    std::size_t  operator()(key_type__ const&  key) const noexcept { return std::hash<key_type__>()(key); }
};

template<typename key_type__>
struct digraph_hash_func< std::pair<key_type__,key_type__> >
{
    std::size_t  operator()(std::pair<key_type__,key_type__> const&  key) const noexcept
    {
        return std::hash<key_type__>()(key.first) + 1151ULL * std::hash<key_type__>()(key.second);
    }
};

template<typename key_type__, typename data_type__>
struct digraph_storage_selector
{
    using  key_type = typename std::remove_const<key_type__>::type;
    using  value_type = data_type__;
    using  value_type_reference = value_type&;
    using  container_type = std::unordered_map<key_type,value_type,digraph_hash_func<key_type__> >;
    using  container_value_type = std::pair<key_type,value_type>;

    static inline key_type  get_key(container_value_type const&  value) { return value.first; }
    static inline value_type  get_value(container_value_type const&  value) { return value.second; }
};

template<typename key_type__>
struct digraph_storage_selector<key_type__,void>
{
    using  key_type = key_type__;
    using  value_type = void;
    using  value_type_reference = value_type;
    using  container_type = std::unordered_set<key_type,digraph_hash_func<key_type__> >;
    using  container_value_type = key_type;

    static inline key_type  get_key(container_value_type const&  value) { return value; }
};


template<typename node_data_type__, typename edge_data_type__>
struct digraph
{
    using  node_id = uint64_t;
    using  node_data_type = typename detail::digraph_storage_selector<node_id,node_data_type__>::value_type;
    using  nodes_container_type = typename detail::digraph_storage_selector<node_id,node_data_type__>::container_type;
    using  nodes_container_value_type = typename detail::digraph_storage_selector<node_id,node_data_type__>::container_value_type;
    using  node_id_hasher_type = digraph_hash_func<node_id>;

    using  edge_id = std::pair<node_id,node_id>;
    using  edge_data_type = typename detail::digraph_storage_selector<edge_id,edge_data_type__>::value_type;
    using  edges_container_type = typename detail::digraph_storage_selector<edge_id,edge_data_type__>::container_type;
    using  edges_container_value_type = typename detail::digraph_storage_selector<edge_id,edge_data_type__>::container_value_type;
    using  edge_id_hasher_type = digraph_hash_func<edge_id>;

    digraph() = default;
    digraph(digraph&&) = default;

    virtual ~digraph() {}

    nodes_container_type const&  nodes() const noexcept { return m_nodes; }
    edges_container_type const&  edges() const noexcept { return m_edges; }

    std::vector<node_id> const&  successors(node_id const  n) const;
    std::vector<node_id> const&  predecessors(node_id const  n) const;

    virtual void  insert_nodes(std::vector<nodes_container_value_type> const&  nodes);
    virtual void  insert_nodes(typename nodes_container_type::const_iterator  begin, typename nodes_container_type::const_iterator const  end);
    virtual void  erase_nodes(std::vector<node_id> const&  nodes);

    virtual void  insert_edges(std::vector<edges_container_value_type> const&  edges);
    virtual void  insert_edges(typename edges_container_type::const_iterator  begin, typename edges_container_type::const_iterator const  end);
    virtual void  erase_edges(std::vector<edge_id> const&  edges);

private:
    digraph(digraph const& ) = delete;
    digraph& operator=(digraph const& ) = delete;

    std::unordered_map<node_id,std::vector<node_id> >  m_successors;
    std::unordered_map<node_id,std::vector<node_id> >  m_predecessors;
    nodes_container_type  m_nodes;
    edges_container_type  m_edges;
};


template<typename node_data_type__, typename edge_data_type__>
std::vector<typename digraph<node_data_type__,edge_data_type__>::node_id> const&
digraph<node_data_type__,edge_data_type__>::successors(node_id const  n) const
{
    auto const  it = m_successors.find(n);
    ASSUMPTION(it != m_successors.cend());
    return it->second;
}

template<typename node_data_type__, typename edge_data_type__>
std::vector<typename digraph<node_data_type__,edge_data_type__>::node_id> const&
digraph<node_data_type__,edge_data_type__>::predecessors(node_id const  n) const
{
    auto const  it = m_predecessors.find(n);
    ASSUMPTION(it != m_predecessors.cend());
    return it->second;
}

template<typename node_data_type__, typename edge_data_type__>
void  digraph<node_data_type__,edge_data_type__>::insert_nodes(std::vector<nodes_container_value_type> const&  nodes)
{
    for (auto const& n_d : nodes)
    {
        node_id const  id = detail::digraph_storage_selector<node_id,node_data_type>::get_key(n_d);

        {
            auto const  result = m_nodes.insert(n_d);
            ASSUMPTION(result.second);
        }
        {
            auto const  result = m_successors.insert({id,{}});
            ASSUMPTION(result.second);
        }
        {
            auto const  result = m_predecessors.insert({id,{}});
            ASSUMPTION(result.second);
        }
    }
}

template<typename node_data_type__, typename edge_data_type__>
void  digraph<node_data_type__,edge_data_type__>::insert_nodes(typename nodes_container_type::const_iterator  begin,
                                                               typename nodes_container_type::const_iterator const  end)
{
    for ( ; begin != end; ++begin)
    {
        node_id const  id = detail::digraph_storage_selector<node_id,node_data_type>::get_key(*begin);

        {
            auto const  result = m_nodes.insert(*begin);
            ASSUMPTION(result.second);
        }
        {
            auto const  result = m_successors.insert({id,{}});
            ASSUMPTION(result.second);
        }
        {
            auto const  result = m_predecessors.insert({id,{}});
            ASSUMPTION(result.second);
        }
    }
}

template<typename node_data_type__, typename edge_data_type__>
void  digraph<node_data_type__,edge_data_type__>::erase_nodes(std::vector<node_id> const&  nodes)
{
    for (auto const n : nodes)
    {
        std::vector<edge_id>  edges;
        auto const  sit = m_successors.find(n);
        ASSUMPTION(sit != m_successors.end());
        for (node_id const  m : sit->second)
            edges.push_back({n,m});
        erase_edges(edges);

        edges.clear();
        auto const  pit = m_predecessors.find(n);
        ASSUMPTION(pit != m_predecessors.end());
        for (node_id const  m : pit->second)
            edges.push_back({m,n});
        erase_edges(edges);

        m_nodes.erase(n);
        m_successors.erase(sit);
        m_predecessors.erase(pit);
    }
}

template<typename node_data_type__, typename edge_data_type__>
void  digraph<node_data_type__,edge_data_type__>::insert_edges(std::vector<edges_container_value_type> const&  edges)
{
    for (auto const& e : edges)
    {
        edge_id const  id = detail::digraph_storage_selector<edge_id,edge_data_type>::get_key(e);

        {
            auto const  result = m_edges.insert(e);
            ASSUMPTION(result.second);
        }
        {
            auto const  it = m_successors.find(id.first);
            ASSUMPTION(it != m_successors.end());
            ASSUMPTION(std::find(it->second.cbegin(),it->second.cend(),id.second) == it->second.cend());
            it->second.push_back(id.second);
        }
        {
            auto const  it = m_predecessors.find(id.second);
            ASSUMPTION(it != m_predecessors.end());
            ASSUMPTION(std::find(it->second.cbegin(),it->second.cend(),id.first) == it->second.cend());
            it->second.push_back(id.first);
        }
    }
}

template<typename node_data_type__, typename edge_data_type__>
void  digraph<node_data_type__,edge_data_type__>::insert_edges(typename edges_container_type::const_iterator  begin,
                                                               typename edges_container_type::const_iterator const  end)
{
    for ( ; begin != end; ++begin)
    {
        edge_id const  id = detail::digraph_storage_selector<edge_id,edge_data_type>::get_key(*begin);

        {
            auto const  result = m_edges.insert(*begin);
            ASSUMPTION(result.second);
        }
        {
            auto const  it = m_successors.find(id.first);
            ASSUMPTION(it != m_successors.end());
            ASSUMPTION(std::find(it->second.cbegin(),it->second.cend(),id.second) == it->second.cend());
            it->second.push_back(id.second);
        }
        {
            auto const  it = m_predecessors.find(id.second);
            ASSUMPTION(it != m_predecessors.end());
            ASSUMPTION(std::find(it->second.cbegin(),it->second.cend(),id.first) == it->second.cend());
            it->second.push_back(id.first);
        }
    }
}

template<typename node_data_type__, typename edge_data_type__>
void  digraph<node_data_type__,edge_data_type__>::erase_edges(std::vector<edge_id> const&  edges)
{
    for (auto const& e : edges)
    {
        {
            auto const  it = m_successors.find(e.first);
            ASSUMPTION(it != m_successors.end());

            auto const itr = std::remove(it->second.begin(),it->second.end(),e.second);
            ASSUMPTION(itr != it->second.end());
            it->second.erase(itr,it->second.end());
        }
        {
            auto const  it = m_predecessors.find(e.second);
            ASSUMPTION(it != m_predecessors.end());

            auto const itr = std::remove(it->second.begin(),it->second.end(),e.first);
            ASSUMPTION(itr != it->second.end());
            it->second.erase(itr,it->second.end());
        }

        m_edges.erase(e);
    }
}


}




template<typename node_data_type__, typename edge_data_type__>
struct digraph : public detail::digraph<node_data_type__,edge_data_type__>
{
    using  super = detail::digraph<node_data_type__,edge_data_type__>;

    typename super::node_data_type const&  data(typename super::node_id const  n) const;
    typename super::node_data_type&  data(typename super::node_id const  n);

    typename super::edge_data_type const&  data(typename super::edge_id const  e) const;
    typename super::edge_data_type&  data(typename super::edge_id const  e);
};

template<typename node_data_type__, typename edge_data_type__>
typename detail::digraph<node_data_type__,edge_data_type__>::node_data_type const&
digraph<node_data_type__,edge_data_type__>::data(typename super::node_id const  n) const
{
    auto const  it = this->nodes().find(n);
    ASSUMPTION(it !=  this->nodes().cend());
    return it->second;
}

template<typename node_data_type__, typename edge_data_type__>
typename detail::digraph<node_data_type__,edge_data_type__>::node_data_type&
digraph<node_data_type__,edge_data_type__>::data(typename super::node_id const  n)
{
    auto const  it = this->nodes().find(n);
    ASSUMPTION(it !=  this->nodes().end());
    return const_cast<typename super::node_data_type&>(it->second);
}

template<typename node_data_type__, typename edge_data_type__>
typename detail::digraph<node_data_type__,edge_data_type__>::edge_data_type const&
digraph<node_data_type__,edge_data_type__>::data(typename super::edge_id const  e) const
{
    auto const  it = this->edges().find(e);
    ASSUMPTION(it !=  this->edges().cend());
    return it->second;
}

template<typename node_data_type__, typename edge_data_type__>
typename detail::digraph<node_data_type__,edge_data_type__>::edge_data_type&
digraph<node_data_type__,edge_data_type__>::data(typename super::edge_id const  e)
{
    auto const  it = this->edges().find(e);
    ASSUMPTION(it !=  this->edges().cend());
    return const_cast<typename super::edge_data_type&>(it->second);
}






template<typename node_data_type__>
struct digraph<node_data_type__,void> : public detail::digraph<node_data_type__,void>
{
    using  super = detail::digraph<node_data_type__,void>;

    typename super::node_data_type const&  data(typename super::node_id const  n) const;
    typename super::node_data_type&  data(typename super::node_id const  n);
};

template<typename node_data_type__>
typename detail::digraph<node_data_type__,void>::node_data_type const&
digraph<node_data_type__,void>::data(typename super::node_id const  n) const
{
    auto const  it = this->nodes().find(n);
    ASSUMPTION(it !=  this->nodes().cend());
    return it->second;
}

template<typename node_data_type__>
typename detail::digraph<node_data_type__,void>::node_data_type&
digraph<node_data_type__,void>::data(typename super::node_id const  n)
{
    auto const  it = this->nodes().find(n);
    ASSUMPTION(it !=  this->nodes().end());
    return const_cast<typename super::node_data_type&>(it->second);
}







template<typename edge_data_type__>
struct digraph<void,edge_data_type__> :public  detail::digraph<void,edge_data_type__>
{
    using  super = detail::digraph<void,edge_data_type__>;

    typename super::edge_data_type const&  data(typename super::edge_id const  e) const;
    typename super::edge_data_type&  data(typename super::edge_id const  e);
};

template<typename edge_data_type__>
typename detail::digraph<void,edge_data_type__>::edge_data_type const&
digraph<void,edge_data_type__>::data(typename super::edge_id const  e) const
{
    auto const  it = this->edges().find(e);
    ASSUMPTION(it !=  this->edges().cend());
    return it->second;
}

template<typename edge_data_type__>
typename detail::digraph<void,edge_data_type__>::edge_data_type&
digraph<void,edge_data_type__>::data(typename super::edge_id const  e)
{
    auto const  it = this->edges().find(e);
    ASSUMPTION(it !=  this->edges().cend());
    return const_cast<typename super::edge_data_type&>(it->second);
}






template<>
struct digraph<void,void> : public detail::digraph<void,void>
{
    using  super = detail::digraph<void,void>;
};


#endif
