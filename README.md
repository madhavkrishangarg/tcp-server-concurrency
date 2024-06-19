# TCP Server Concurrency

This repository contains several TCP server programs demonstrating different concurrency models in C. The servers handle up to 4000 concurrent connections and compute the factorial of a given number. The implementations include multiple processes, multiple threads, and non-blocking I/O multiplexing using `select()`, `poll()`, and `epoll`.

## Concurrency Models

1. **Multiple Processes (using fork)**
2. **Multiple Threads (using pthreads)**
3. **Non-blocking I/O Multiplexing**
   - `select()`
   - `poll()`
   - `epoll`

## Server Functionality

Each server program performs the following steps:
1. Accepts an incoming connection.
2. Reads the payload and casts it to a 64-bit unsigned integer, `n`.
3. Computes the factorial of `n` using the function `fact(n)`. If `n > 20`, it computes the factorial of 20.
4. Sends the factorial result back to the client.

## Experiments

For each server program, experiments are conducted with the number of concurrent client connections set to 500, 1000, and 3000. 
The throughputs are in the saved in the repository.
