# Warehouse Management System Simulation (C & POSIX Threads)
 
This project simulates a **real-time production line** in C, where items are generated, quality-checked, and logged asynchronously using three concurrent POSIX threads communicating through a shared circular buffer.
 
---
 
## System Overview
 
Three specialized threads run in parallel, coordinated through mutexes and condition variables:
 
- **Producer Thread** — Creates a new product every 2 seconds, assigning it an incremental ID and a random quality value (0–99), then inserts it into the shared circular buffer.
- **Quality Control Thread** — Extracts products from the buffer and checks their quality. Products scoring above 50 are passed to the log thread; the rest are immediately freed from memory.
- **Log Writer Thread** — Receives approved products and writes their ID and quality to `magazzino.log` in CSV format.
---
 
## Key Features
 
- **Circular Buffer** — Fixed-size FIFO buffer (`DIM_BUFFER = 10`) for thread-safe communication between producer and consumer.
- **Synchronization** — Uses `pthread_mutex_t` for exclusive buffer access and three `pthread_cond_t` condition variables (`notEmpty`, `notFull`, `validProduct`) to coordinate the three threads without busy-waiting.
- **Signal Handling** — Registers a `SIGINT` handler (CTRL+C) that sets a shared `stop` flag and broadcasts on all condition variables, ensuring every thread wakes up and exits cleanly.
- **Memory Management** — All products are dynamically allocated. Memory is freed as soon as a product is processed or rejected, with a final `freeBuffer()` cleanup on shutdown.
---
 
## Technical Requirements
 
- **Language**: C (C99 or later)
- **Libraries**: `pthread.h`, `stdio.h`, `stdlib.h`, `signal.h`, `unistd.h`, `time.h`
- **Compiler**: GCC
---
 
## File Structure
 
```
.
├── main.c          # Entry point: thread creation, signal handling, cleanup
├── production.c    # Thread logic: producer, quality control, log writer, buffer management
├── production.h    # Shared structs, constants, extern declarations, function prototypes
└── magazzino.log   # Generated output file (CSV format)
```
 
---
 
## How to Build and Run
 
```bash
gcc -o programma main.c production.c -lpthread
./programma
```
 
The program runs indefinitely. Press **CTRL+C** to stop it gracefully — all threads will finish their current operation, free their resources, and exit cleanly.
 
---
 
## Output
 
Products that pass quality control are written to `magazzino.log` in CSV format:
 
```
ID,QUALITY
0,87
3,62
5,91
...
```
 
---
 
## Architecture Diagram
 
```
┌─────────────────┐     notFull / notEmpty      ┌──────────────────────┐
│  Thread 1       │ ──────────────────────────▶ │  Thread 2            │
│  Producer       │    Circular Buffer           │  Quality Control     │
│  (every 2s)     │ ◀────────────────────────── │                      │
└─────────────────┘                              └──────────────────────┘
                                                          │
                                                 validProduct signal
                                                          │
                                                          ▼
                                                 ┌──────────────────────┐
                                                 │  Thread 3            │
                                                 │  Log Writer          │
                                                 │  → magazzino.log     │
                                                 └──────────────────────┘
```
