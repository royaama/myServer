A multi-threaded web server implemented in C, utilizing request queues to handle multiple client requests efficiently.
 Features:
- Thread Management: Uses a queue-based worker thread model.
- Request Handling: Processes incoming HTTP requests concurrently.
- Synchronization Mechanisms: Implements mutexes and condition variables to manage resource access.
- Custom Scheduling Policies: Supports different queuing mechanisms for handling requests.

Installation & Usage
Compiling:
make
 
 Running the Server:
./server <port> <thread_pool_size> <queue_size> <scheduling_policy>
Example:
./server 8080 4 10 block

Project Structure:

|-- server.c       # Main server logic
|-- request.c      # Handles HTTP requests
|-- Queue.c        # Queue implementation for managing tasks
|-- segel.c        # Utility functions for networking
|-- Makefile       # Compilation script


## Author
Developed by Roaia Mahajna. 
