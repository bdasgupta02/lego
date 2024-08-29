#include "lego/lego.hpp"

#include <iostream>

// This example is a simple ping-pong component, where one component
// pings the other, and we can get the total number of pings made at the end.
// The fun part is that you can simply choose whichever pong component you want.


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

// The first pong component, which just pongs when its pinged
template<typename NodeBase>
struct PongOnce : NodeBase
{
    using NodeBase::NodeBase;
    using PingCountType = NodeBase::Traits::PingCountType;

    void handle(tag::Ping)
    {
        auto* handler = this->getHandler();
        handler->invoke(tag::Pong{});
    }
};


// The second pong component, which sends back the specified number of pongs
template<typename NodeBase>
struct PongNTimes : NodeBase
{
    using NodeBase::NodeBase;
    using PingCountType = NodeBase::Traits::PingCountType;

    void handle(tag::Ping, PingCountType numPongs)
    {
        auto* handler = this->getHandler();
        for (PingCountType i = PingCountType{}; i < numPongs; ++i)
            handler->invoke(tag::Pong{});
    }
};


// Type traits can be injected into the graph,
// if not, pass an empty struct
struct Traits
{
    using PingCountType = int;
};


// This is how the app is assembled using the Router
// There are two variations that use shared dependencies

// clang-format off
using PingPongOnce = lego::Router<
    Traits,
    lego::NodeList<
        PingComponent,
        PongOnce
    >
>;

using PingPongNTimes = lego::Router<
    Traits,
    lego::NodeList<
        PingComponent,
        PongNTimes
    >
>;
// clang-format on

int main()
{
    {
        PingPongOnce app;

        auto* handler = app.getHandler();
        handler->invoke(tag::Ping{});

        auto pings = handler->retrieve(tag::GetPings{});
        std::cout << pings << std::endl; // prints 1
    }

    {
        PingPongNTimes app;

        auto* handler = app.getHandler();
        handler->invoke(tag::Ping{}, 50);

        auto pings = handler->retrieve(tag::GetPings{});
        std::cout << pings << std::endl; // prints 50
    }
}
