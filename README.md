# Lego
A very simple single-header static dependency injection wiring system for low-latency C++ applications.

### Requirements
C++20

### How is this dependency injection?
The graph system puts tag dispatch at its core to route components statically, making the concept of dependency injection implicit and dependent on how you use the library. The structure of the router implies that each node is dependent on the entire structure of the graph, as well as the tag dispatched API it uses. This can therefore be a powerful tool for dependency injection when swapping components around like legos, for instance using different variations of a component in different places or executables:
```cpp
using App1 = Router<
    Traits1, // whatever type traits you give it
    NodeList<
        CommonComponentX,
        CommonComponentY,
        UniqueComponentA // unique to this application (App1)
    >
>;

using App2 = Router<
    Traits2,
    NodeList<
        CommonComponentX,
        CommonComponentY,
        UniqueComponentB,
        UniqueComponentC
    >
>;
```

## Quick Start
### Creating nodes
These are the individual components of your app (or component graph). Other than the handlers and `NodeBase`, they just work like ordinary C++ structs or classes with your functionality within them. The bricks in this lego analogy.

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

        // calls trader component's "Position" handle() function to get some value
        auto position = this->getHandler()->retrieve(tag::Trader::Position{}); 

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
The graph is just all the components grouped and wired automatically together, so you don't need to worry about connecting the function calls manually.

```cpp
// Inject type traits into the graph, accessible by every node
Struct Traits
{
    using Price = Decimal<4u>;
};

// Finally, create a graph with type traits and a node list
using Graph = Router<
    Traits,
    NodeList<
        Risk,
        Stream,
        Trader,
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

### Example
Please find an example in the `example.cpp` file.
