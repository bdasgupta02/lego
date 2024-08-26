# Lego
A single-header lego-like static dependency injection wiring system for low-latency C++ applications.

### Requirements
- C++20

### Quirks
- Each component does not need knowledge of other components, due to the tag dispatched functions.
- Allows circular dependencies. With a few tweaks to the router this could be banned.
