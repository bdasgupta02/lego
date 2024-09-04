#pragma once

#include <memory>
#include <type_traits>
#include <utility>

namespace lego {

namespace concepts {
template<typename T>
concept Void = std::is_void_v<T>;

template<typename T>
concept NonVoid = !Void<T>;

template<typename Node, typename Tag, typename Router, typename... Args>
concept HasVoidHandler = requires(Node node, Tag tag, Router& router, Args&&... args) {
    { node.handle(tag, std::forward<Args>(args)...) } -> Void;
};

template<typename Node, typename Tag, typename Router, typename... Args>
concept HasReturnHandler = requires(Node node, Tag tag, Router& router, Args&&... args) {
    { node.handle(tag, std::forward<Args>(args)...) } -> NonVoid;
};
} // namespace concepts

// Maybe a good chance to use C++23's "deducing this"
template<typename Router>
struct RouterHandler
{
    // Use invoke() to dispatch a void call, and retrieve() to dispatch a non-void call
    // The receiver node(s) just need handle() functions to receive the dispatch calls
    // All handle() functions in the receiver nodes should begin with (tag, ...<args>)

    template<typename Tag, typename... Args>
    inline void invoke(Tag tag, Args&&... args)
    {
        static_cast<Router&>(*this).invokeImpl(tag, std::forward<Args>(args)...);
    }

    template<typename Tag, typename... Args>
    inline auto retrieve(Tag tag, Args&&... args)
    {
        return std::forward<decltype(static_cast<Router&>(*this).retrieveImpl(tag, std::forward<Args>(args)...))>(
            static_cast<Router&>(*this).retrieveImpl(tag, std::forward<Args>(args)...));
    }
};

template<typename _Traits, typename _Router>
struct NodeBase
{
    using Traits = _Traits;
    using Router = _Router;

    NodeBase(RouterHandler<Router>& handler)
        : handler{&handler}
    {}

    [[gnu::always_inline, gnu::hot]]
    inline RouterHandler<Router>* getHandler()
    {
        return handler;
    }

    RouterHandler<Router>* handler{nullptr};
};

template<template<typename> class... Nodes>
struct NodeList
{};

template<typename, typename>
struct Router;

template<template<typename> class Node, typename Traits, template<typename> class... Nodes>
Node<NodeBase<Traits, Router<Traits, NodeList<Nodes...>>>>& getNode(Router<Traits, NodeList<Nodes...>>& graph)
{
    static_assert(
        (std::is_same_v<
             Node<NodeBase<Traits, Router<Traits, NodeList<Nodes...>>>>,
             Nodes<NodeBase<Traits, Router<Traits, NodeList<Nodes...>>>>> ||
         ...),
        "Invalid node type");
    return static_cast<Node<NodeBase<Traits, Router<Traits, NodeList<Nodes...>>>>&>(graph);
}

template<typename Node, typename Traits, template<typename> class... Nodes>
Node& getNode(Router<Traits, NodeList<Nodes...>>& graph)
{
    static_assert(
        (std::is_same_v<Node, Nodes<NodeBase<Traits, Router<Traits, NodeList<Nodes...>>>>> || ...),
        "Invalid node type");
    return static_cast<Node&>(graph);
}

// Static dependency injection wiring router
// Nodes should be descending order in priority
// Statically checks if the called functions are implemented
// Can be simplified a lot more imo
// Note: retrieval functions can return lval or ptr
template<typename Traits, template<typename> class... Nodes>
struct Router<Traits, NodeList<Nodes...>>
    : public RouterHandler<Router<Traits, NodeList<Nodes...>>>
    , public Nodes<NodeBase<Traits, Router<Traits, NodeList<Nodes...>>>>...
{
    Router()
        : Nodes<NodeBase<Traits, Router>>(static_cast<RouterHandler<Router>&>(*this))...
    {}

    RouterHandler<Router>* getHandler() { return static_cast<RouterHandler<Router>*>(this); }

private:
    template<typename Tag, typename... Args>
    inline void invokeImpl(Tag tag, Args&&... args)
    {
        static_assert(
            (concepts::HasVoidHandler<Nodes<NodeBase<Traits, Router>>, Tag, Router, Args...> || ...),
            "At least one node should have this handler");

        (tryInvoke<Nodes>(tag, std::forward<Args>(args)...), ...);
    }

    template<typename Tag, typename... Args>
    inline auto retrieveImpl(Tag tag, Args&&... args)
    {
        static_assert(
            (concepts::HasReturnHandler<Nodes<NodeBase<Traits, Router>>, Tag, Router, Args...> + ...) == 1,
            "Exactly one node should have this handler");

        return std::forward<decltype(tryRetrieve<Nodes...>(tag, std::forward<Args>(args)...))>(
            tryRetrieve<Nodes...>(tag, std::forward<Args>(args)...));
    }

    template<template<typename> class Node, typename Tag, typename... Args>
    inline void tryInvoke(Tag tag, Args&&... args)
    {
        if constexpr (concepts::HasVoidHandler<Node<NodeBase<Traits, Router>>, Tag, Router, Args...>)
        {
            auto& node = getNode<Node>(*this);
            node.handle(tag, std::forward<Args>(args)...);
        }
    }

    template<template<typename> class FirstNode, template<typename> class... RestNodes, typename Tag, typename... Args>
    inline auto tryRetrieve(Tag tag, Args&&... args)
    {
        if constexpr (concepts::HasReturnHandler<FirstNode<NodeBase<Traits, Router>>, Tag, Router, Args...>)
        {
            auto& node = getNode<FirstNode>(*this);
            return std::forward<decltype(node.handle(tag, std::forward<Args>(args)...))>(
                node.handle(tag, std::forward<Args>(args)...));
        }
        else
        {
            static_assert(sizeof...(RestNodes) != 0, "No nodes have this handler");
            return std::forward<decltype(tryRetrieve<RestNodes...>(tag, std::forward<Args>(args)...))>(
                tryRetrieve<RestNodes...>(tag, std::forward<Args>(args)...));
        }
    }

    template<typename Tag, typename... Args>
    friend void RouterHandler<Router>::invoke(Tag tag, Args&&... args);

    template<typename Tag, typename... Args>
    friend auto RouterHandler<Router>::retrieve(Tag tag, Args&&... args);
};

}; // namespace lego
