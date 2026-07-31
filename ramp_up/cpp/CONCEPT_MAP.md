# C++ Concept Map

Start with values and the build model: headers declare, translation units compile,
and the linker joins definitions. Value semantics make copying and aliasing explicit.
That leads to ownership and RAII, then to moves for transferring resources. Runtime
polymorphism adds controlled indirection; object layout determines the cost of that
indirection and of data traversal; concurrency finally requires explicit ownership,
visibility, and wake-up boundaries.

## Dependency chain

Build model and values → references and `const` → ownership and RAII → copies and
moves → runtime polymorphism → layout and locality → threads, atomics, and queues.

## Modules at a glance

| Module | Mental model | Application | Capstone | Downstream interview questions |
|---|---|---|---|---|
| [01](01_stl_containers/) | Values own their contents; algorithms operate over ranges. | Parse and organize sensor metadata. | Container and algorithm exercise suite. | [Sliding Window Maximum](../../questions/01_algorithms/001_sliding_window_maximum/) |
| [02](02_ownership_and_raii/) | One clear owner; scope ends resources. | Frame buffers, files, and locks clean up predictably. | Resource-owning value exercise suite. | [Ownership and Memory Layout](../../questions/05_systems/002_ownership_memory_layout_cpp/) |
| [03](03_move_semantics_rule_of_five/) | Copy preserves a value; move transfers a resource. | Hand off frames without duplicate buffer traffic. | Move-aware buffer exercise suite. | [Ring Buffer](../../questions/05_systems/004_ring_buffer_cpp/) |
| [04](04_virtual_functions_and_vtables/) | Virtual dispatch is an explicit interface boundary. | Swap sensor or backend implementations safely. | Polymorphic interface exercise suite. | [Thread Pool Inference Server](../../questions/05_systems/006_thread_pool_inference_server_cpp/) |
| [05](05_memory_layout_and_cache/) | Bytes, alignment, and traversal order shape throughput. | Keep image and point-cloud passes cache-friendly. | Layout and traversal exercise suite. | [Memory Pool and Allocation Benchmark](../../questions/05_systems/007_memory_pool_benchmark_cpp/) |
| [06](06_threads_atomics_queues/) | Shared state needs a synchronization and shutdown protocol. | Connect camera, inference, and publish stages. | Bounded queue exercise suite. | [Bounded Blocking Queue](../../questions/05_systems/003_bounded_blocking_queue_cpp/) |

## Concept tour

Before each implementation, answer these questions in your own words:

1. When is a local object destroyed?
2. Who owns the resource and who only borrows it?
3. Does this expression copy, move, or alias?
4. Which bytes are contiguous?
5. What synchronization boundary protects shared state?
6. What event wakes a blocked producer or consumer?

Return to the [track overview](README.md), or begin with [Module 01](01_stl_containers/).
