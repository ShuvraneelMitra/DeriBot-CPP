# **DERIBOT** : A simple order management and execution tool in C++

DeriBot is a order management application which works through the command line interface. It uses a websocket client to connect to the [Deribit TESTNET website](https://test.deribit.com) and manage the user portfolio from there.

## Build instructions:

### Prerequisites
Ensure your system has a C++ compiler such as g++ installed.

The project is uses CMake, so make sure that you have CMake installed:
1. For Mac users: `brew install cmake`
2. For Linux users: `sudo apt install cmake` (or replace apt with your package manager, e.g., dnf for Fedora or yum for CentOS)
3. For Windows users: Download from the official [CMake downloads](https://cmake.org/download/) page.

Also install Boost and OpenSSL:
1. For Mac users: 
   ```
   brew install boost
   brew install openssl
   ```
2. For Linux users: 
   ```
   sudo apt install libboost-all-dev
   sudo apt install libssl-dev
   ```
3. For Windows users: Download the Boost installer/OpenSSL binaries from the official downloads pages and follow the installation instructions provided there.

Check all of these in your system (example for Ubuntu 20.04 provided):
```
$ g++ --version
$ cmake --version
$ dpkg -l | grep libboost
$ openssl version
```

### Build steps
Clone this repository to your system: 
`$ git clone https://github.com/ShuvraneelMitra/Order-Execution-Management-System-CPP.git`
1. Make a build directory inside the project folder and enter it:
   ```
   $ mkdir build
   $ cd build
   ```
2. Run CMake and build the project using the `--build .` option:
   ```
   $ cmake .. -Wno-dev
   $ cmake --build .
   ```
   This will download the config files of the [websocketpp library](https://github.com/zaphoyd/websocketpp) inside the build directory. Apart from websocketpp, the project also uses the [nlohmann json](https://github.com/nlohmann/json) library for handling the JSON-RPC procedure over websockets.

3. Run the application on the terminal: `$ ./DeriBot`
   Depending on the system the correct syntax could be `$ DeriBot`, `$ ./DeriBot` or `$ .\DeriBot`.
