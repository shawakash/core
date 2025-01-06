# TCP Design Implementation & System Monitor

A client-server application that provides real-time system information over TCP/IP. The server monitors system resources and processes, while clients can request this information by connecting to the server.

## Features

- Real-time system monitoring
- Process information including CPU and memory usage
- Top N processes sorting by CPU usage
- Cross-platform support (Linux-specific features available)
- Configurable number of processes to display
- Clean and formatted output

## System Information Provided

- CPU Usage
- Memory Usage (Total, Used, Free)
- Disk Usage (Total, Used, Free)
- Process Details:
  - PID
  - Process Name
  - CPU Usage
  - Memory Usage
  - Process State
  - User/Owner
  - Command Line

## Prerequisites

- Linux Operating System
- G++ Compiler
- Make

## Building the Project

1. Clone the repo.

2. Build both server and client:
```bash
make
```

This will create two executables:
- `server`: The monitoring server
- `client`: The client application

## Usage

### Starting the Server

```bash
./server <port> [ip]
```
- `port`: Port number to listen on
- `ip`: (Optional) IP address to bind to. If not specified, binds to all interfaces.

Example:
```bash
./server 1234              # Listen on all interfaces
./server 1234 127.0.0.1    # Listen only on localhost
```

### Using the Client

```bash
./client <port> <ip>
```
- `port`: Server's port number
- `ip`: Server's IP address

Example:
```bash
./client 1234 127.0.0.1
```

### Client Commands

Send a number to get system information with that many top processes:
```
5     # Shows top 5 processes
10    # Shows top 10 processes
20    # Shows top 20 processes
```

## Project Structure

```
core/tcp/
├── Makefile
├── Makefile.client
├── Makefile.server
├── client.cpp
├── server.cpp
├── systeminfo.h
├── tcpacceptor.cpp
├── tcpacceptor.h
├── tcpconnector.cpp
├── tcpconnector.h
├── tcpstream.cpp
└── tcpstream.h
```

## Technical Details

### TCP Implementation
- Uses POSIX sockets for network communication
- Implements a stream-based protocol
- Handles multiple client connections
- Provides clean connection handling and resource cleanup

### System Monitoring
- Uses `/proc` filesystem for Linux system information
- Real-time process monitoring
- Efficient sorting and filtering of process information
- Formatted output for better readability
