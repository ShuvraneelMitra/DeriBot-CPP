# **DERIBOT** : A simple order management and execution tool in C++

DeriBot is an order management application which works through the command line interface. It uses a websocket client to connect to the [Deribit TESTNET website](https://test.deribit.com) and manage the user portfolio from there.

<p align="center">
  <img src="https://i.imgur.com/CgVxwNf.png" alt="Message" width="500px">
</p>

## Build instructions:

### Prerequisites
Ensure your system has a C++ compiler such as g++ installed.

The project uses CMake, so make sure that you have CMake installed:
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
`$ git clone https://github.com/ShuvraneelMitra/DeriBot-CPP.git`
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

## Using the project via Command-Line:

### Basic commands
The application doubles up as a normal websocket client in addition to being able to access the Deribit API:
1. Type `help` to see all the supported commands all at once.
2. `quit` closes all existing websocket connections and exits the program.
3. `show <id>` gets the metadata of the connection with the given id.
4. `send <id> msg` sends the message `msg` to the connection with the given id.
5. `show_messages <id>` allows you to view all the messages exchanged between your client and the server on the mentioned connection.

### Accessing the Deribit API using DeriBot
1. Create an account on the Deribit website and generate a set of API keys which contains a Client ID and a Client Secret.
2. Run the DeriBot program and enter the command 
   `connect wss://test.deribit.com/ws/api/v2` to connect to the Deribit testnet websocket server. You should get a message similar to:
    `Created connection with id 0`.
    **NOTE:** A more compact alternative `DERIBIT connect` has now been added with the same behaviour.
3. Enter the command `show 0` to ensure that the connection has been correctly created. In case you are making multiple connections, replace the `0` with the connection id. A proper connection should return the following:
   ```
   > URI: wss://test.deribit.com/ws/api/v2
   > Status: Open
   > Remote Server: nginx/1.25.5
   > Error/close reason: N/A
   > Messages Processed: (0)
   ```
4. To use the full functionality of the Deribit API, we need to authenticate using our user credentials. Enter the command 
   `DERIBIT authorize <connection_id> <client_id> <client_secret> -r(optional)`
   for authentication. You can find the values here:
   ![](https://i.imgur.com/poRb5xD.png)
   This returns a JSON object which contains an "Access Key" which can either be saved in the bot or can be manually input everytime. This key will be used to access the functionality of the API. <br>
   You can use a flag `-r` at the end of the command if you want the application to store your access_token automatically for the entire duration of the session. If this flag is absent, you will be prompted to enter the access_token everytime you use a `private` API method (i.e. one which requires connecting to your account).<br> 
   Note that this authentication needs to be carried out in every session of the DeriBot since the application sets the default scope of authorization to "connection-level".

### Trading commands:
1. `DERIBIT <id> buy <instrument> <transaction_name>` sends a buy order on the connection with id `<id>` for the instrument `<instrument>`. The program further prompts the user to enter certain information about the trade, as well as the access token for the session, to complete placing the order.
   <br>
   **Amount or contracts**: if you want to specify the amount, say x, then enter `amount x`. If you want to specify the amount in number of contracts bought, enter `contracts x`.<br>
   **Order Type**: Must be one of `limit`, `stop_limit`, `take_limit`, `market`, `stop_market`, `take_market`, `market_limit`, and `trailing_stop`.<br>
   **Time in force value**: Must be one of `good_til_cancelled`, `good_til_day`, `fill_or_kill`, or `immediate_or_cancel`.

2. `DERIBIT <id> sell <instrument> <transaction_name>` sends a sell order on the connection with id `<id>` for the instrument `<instrument>`. This has the same behaviour as `buy` except it sends a sell order.
**NOTE** If you want to further customise your trades, you can always whip up your own JSON object based on the Deribit API and use the `websocket_endpoint.send()` command.

3. `DERIBIT <id> get_open_orders` gets all the open orders on the account of this connection. The command can be further specialised as `DERIBIT get_open_orders <currency>`, `DERIBIT get_open_orders <instrument>` and `DERIBIT get_open_orders <currency> <label>`.
**NOTE** To use this command you will have to modify your API key scope to have `trade:read` or `read_write`.

4. `DERIBIT <id> modify <order_id>` allows you to modify the order with the given order ID. The order ID can be gotten either by noting it down from the received message while placing the order, or later can be retrieved using the `get_open_orders` command.
5. `DERIBIT <id> cancel <order_id>` allows you to cancel a specific order while `DERIBIT <id> cancel_all <options>` allows you to cancel all orders with the given specification of instrument or currency. Specifying no options in `cancel_all` cancels all possible open orders.
