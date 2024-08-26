# Lego
A single-header lego-like static dependency injection wiring system for low-latency C++ applications.

### Requirements
- C++20

### Creating nodes
```cpp
// NodeBase is a lightweight struct to expose the handler to invoke functions of other nodes *magically*
template<typename NodeBase>
struct Stream : NodeBase 
{
    // The router calls the base constructor
    using NodeBase::NodeBase;

    // Get any type traits injected from outside 
    using PriceType = NodeBase::Traits::PriceType;

    // To receive function calls, just use handle(tag) functions 
    // These handlers support any number of universal ref arguments
    // Note: if multiple nodes have the same handler function signatures, 
    // they both will receive the function call only for void handlers
    void handle(tag::Stream::Start, &&...)
    {
        ...

        // To call other nodes:
        // 1. and retrieve() for functions that return something
        // 2. use invoke() for void handlers 

        // calls quoter's "Position" handle() function to get some value
        auto position = this->getHandler()->retrieve(tag::Quoter::Position{}); 

        // call's risk's "Evaluate" handle() function
        this->getHandler()->invoke(tag::Risk::Evaluate{}, position);
    }

    // Handlers can also return anything if needed
    Status handle(tag::Stream::Status, &&...) 
    {
        return status;
    }

    ...
};

```

### Construction of the graph
```cpp
// Finally, create a graph with a type traits and a node list
using Graph = Router<
    Traits,
    NodeList<
        Risk,
        Stream,
        Quoter,
        Logger,
        ...
    >
>;

// Construct the graph
Graph graph;

// Call any node handler functions from the outside to start the app,
// just like you would from within any node
auto* handler = graph.getHandler();
handler->invoke(tag::Stream::Start{});
```

### Quirks
- Each component does not need knowledge of other components, due to the tag dispatched functions
- There are checks to ensure that:
    - at least one handler is implemented for an invoke(tag) call,
    - and only one handler is implemented for a retrieve(tag) call
- Allows circular dependencies. With a few tweaks to the router this could be banned