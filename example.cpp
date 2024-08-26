#include "lego/lego.hpp"

#include <iostream>

// These tags are used to wire and dispatch functions
// between components
namespace tag {
struct Ping
{};
struct Pong
{};
struct GetPings
{};
} // namespace tag

// These are the actual components of the app
template<typename NodeBase>
struct PingComponent : NodeBase
{
    using NodeBase::NodeBase;
    using PingCountType = NodeBase::Traits::PingCountType;

    void handle(tag::Pong) { ++pings; }
    PingCountType handle(tag::GetPings) { return pings; }

    PingCountType pings = 0;
};

template<typename NodeBase>
struct PongComponent : NodeBase
{
    using NodeBase::NodeBase;
    using PingCountType = NodeBase::Traits::PingCountType;

    void handle(tag::Ping)
    {
        auto* handler = this->getHandler();
        handler->invoke(tag::Pong{});
    }
};

// Type traits can be injected into the app,
// if not, pass an empty struct
struct Traits
{
    using PingCountType = int;
};

// This is how the app is assembled using the Router
// clang-format off
using App = lego::Router<
    Traits,
    lego::NodeList<
        PingComponent,
        PongComponent
    >
>;
// clang-format on

int main()
{
    App app;

    auto* handler = app.getHandler();
    for (auto i = 0u; i < 5; ++i)
        handler->invoke(tag::Ping{});

    auto pings = handler->retrieve(tag::GetPings{});
    std::cout << pings << std::endl;
}
