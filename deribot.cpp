#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <mutex>
#include <condition_variable>

#include "DeriBotConfig.h"
#include "deribit_api.h"
#include "utils.h"
#include "password.h"

#include <websocketpp/config/asio_client.hpp> 
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/context.hpp> 
#include <websocketpp/client.hpp> 

#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>

#include <fmt/color.h>

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef std::shared_ptr<boost::asio::ssl::context> context_ptr;

class connection_metadata {
    private:
        int m_id;
        websocketpp::connection_hdl m_hdl;
        std::string m_status;
        std::string m_uri;
        std::string m_server;
        std::string m_error_reason;
        std::vector<std::string> m_summaries;

    public:
        typedef websocketpp::lib::shared_ptr<connection_metadata> ptr;
        typedef std::shared_ptr<boost::asio::ssl::context> context_ptr;

        std::mutex mtx;
        std::condition_variable cv;
        std::vector<std::string> m_messages;
        bool MSG_PROCESSED;

        connection_metadata(int id, websocketpp::connection_hdl hdl, std::string uri):
        m_id(id),
        m_hdl(hdl),
        m_status("Connecting"),
        m_uri(uri),
        m_server("N/A"),
        m_messages({}),
        m_summaries({}),
        MSG_PROCESSED(false)
        {}

        int get_id(){return m_id;}
        websocketpp::connection_hdl get_hdl(){ return m_hdl;}
        std::string get_status(){return m_status;}
        void record_sent_message(std::string const &message) {
            m_messages.push_back("SENT: " + message);
        }
        
        void record_summary(std::string const &message, std::string const &sent){
            if (message == "") return;
            json parsed_msg = json::parse(message);
            std::string cmd = parsed_msg.contains("method")? parsed_msg["method"] : "received";
            std::map<std::string, std::string> summary;
            
            std::map<std::string, std::function<std::map<std::string, std::string>(json)>> action_map = 
            {
                {"public/auth", [](json parsed_msg){ 
                                                    std::map<std::string, std::string> summary;
                                                    summary["method"] = parsed_msg["method"];
                                                    summary["grant_type"] = parsed_msg["params"]["grant_type"];
                                                    return summary;
                                                 }
                },
                {"private/sell", [](json parsed_msg){
                                                    std::map<std::string, std::string> summary = {};
                                                    summary["id"] = std::to_string(parsed_msg["id"].get<int>());
                                                    summary["method"] = parsed_msg["method"];
                                                    summary["instrument_name"] = parsed_msg["params"]["instrument_name"];
                                                    if (parsed_msg["params"].contains("amount"))
                                                        summary["amount"] = std::to_string(parsed_msg["params"]["amount"].get<int>());
                                                    if (parsed_msg["params"].contains("contracts"))
                                                        summary["contracts"] = std::to_string(parsed_msg["params"]["contracts"].get<int>());
                                                    return summary;
                                                 }                   
                },
                {"private/buy", [](json parsed_msg){
                                                    std::map<std::string, std::string> summary = {};
                                                    summary["id"] = std::to_string(parsed_msg["id"].get<int>());
                                                    summary["method"] = parsed_msg["method"];
                                                    summary["instrument_name"] = parsed_msg["params"]["instrument_name"];
                                                    if (parsed_msg["params"].contains("amount"))
                                                        summary["amount"] = std::to_string(parsed_msg["params"]["amount"].get<int>());
                                                    if (parsed_msg["params"].contains("contracts"))
                                                        summary["contracts"] = std::to_string(parsed_msg["params"]["contracts"].get<int>());
                                                    return summary;
                                                 }                   
                },
                {"private/cancel", [](json parsed_msg){
                                                        std::map<std::string, std::string> summary = {};
                                                        summary["method"] = parsed_msg["method"];
                                                        summary["id"] = parsed_msg["order_id"];
                                                        return summary;
                                                      }
                },
                {"received", [](json parsed_msg){
                                                    std::map<std::string, std::string> summary = {};
                                                    if (parsed_msg.contains("result"))
                                                        summary = {{"result", parsed_msg["result"].dump()}};
                                                    else if (parsed_msg.contains("error"))
                                                        summary = {{"error message", parsed_msg["error"].dump()}};
                                                    return summary;
                                                 }
                }
            };
            
            auto find = action_map.find(cmd);
            if (find == action_map.end()) {
                summary["id"] = std::to_string(parsed_msg["id"].get<int>());
                if (sent == "SENT") summary["method"] = parsed_msg["method"];
            }
            else {
                summary = find->second(parsed_msg);
            }
            m_summaries.push_back(sent + " : \n" + utils::printmap(summary));
        }

        void on_open(client * c, websocketpp::connection_hdl hdl){
            m_status = "Open";

            client::connection_ptr con = c->get_con_from_hdl(hdl);
            m_server = con->get_response_header("Server");
        }

        void on_fail(client * c, websocketpp::connection_hdl hdl){
            m_status = "Failed";

            client::connection_ptr con = c->get_con_from_hdl(hdl);
            m_server = con->get_response_header("Server");
            m_error_reason = con->get_ec().message();
        }

        void on_close(client * c, websocketpp::connection_hdl hdl){
            m_status = "Closed";

            client::connection_ptr con = c->get_con_from_hdl(hdl);
            std::stringstream s;
            s << "Close code: " << con->get_remote_close_code() << "("
              << websocketpp::close::status::get_string(con->get_remote_close_code())
              << "), Close reason: " << con->get_remote_close_reason();
            
            m_error_reason = s.str();
        }

        void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg) {
            if (msg->get_opcode() == websocketpp::frame::opcode::text) {
                m_messages.push_back("RECEIVED: " + msg->get_payload());
                record_summary(msg->get_payload(), "RECEIVED");
            } else {
                m_messages.push_back("RECEIVED: " + websocketpp::utility::to_hex(msg->get_payload()));
                record_summary(websocketpp::utility::to_hex(msg->get_payload()), "RECEIVED");
            }

            char show_msg;
            utils::printcmd("Received message. Show message? Y/N ", 57, 255, 20);
            std::cin >> show_msg;
            if(show_msg == 'y' | show_msg == 'Y'){
                if (msg->get_payload()[0] == '{') {
                    std::cout << "Received message: " << utils::pretty(msg->get_payload()) << std::endl;
                }
                else{
                    std::cout << "Received message: " << msg->get_payload() << std::endl;
                }
            }

            if (AUTH_SENT) {
                Password::password().setAccessToken(json::parse(msg->get_payload())["result"]["access_token"]);
                utils::printcmd("Authorization successful!\n");
                AUTH_SENT = false;
            }
            MSG_PROCESSED = true;
            cv.notify_one();
        }
        
        friend std::ostream &operator<< (std::ostream &out, connection_metadata const &data);
};

std::ostream &operator<< (std::ostream &out, connection_metadata const &data){
    out << "> URI: " << data.m_uri << "\n"
        << "> Status: " << data.m_status << "\n"
        << "> Remote Server: " << (data.m_server.empty() ? "None Specified" : data.m_server) << "\n"
        << "> Error/close reason: " << (data.m_error_reason.empty() ? "N/A" : data.m_error_reason) << "\n"
        << "> Messages Processed: (" << data.m_messages.size() << ") \n";
 
        std::vector<std::string>::const_iterator it;
        for (it = data.m_summaries.begin(); it != data.m_summaries.end(); ++it) {
            out << *it << "\n";
        }
    return out;
}

context_ptr on_tls_init() {
            context_ptr context = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);

            try {
                context->set_options(boost::asio::ssl::context::default_workarounds |
                                boost::asio::ssl::context::no_sslv2 |
                                boost::asio::ssl::context::no_sslv3 |
                                boost::asio::ssl::context::single_dh_use);
            } catch (std::exception &e) {
                std::cout << "Error in context pointer: " << e.what() << std::endl;
            }
            return context;
}

class websocket_endpoint {
    private:
        typedef std::map<int,connection_metadata::ptr> con_list;

        client m_endpoint;
        websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;

        con_list m_connection_list;
        int m_next_id;

    public:
        websocket_endpoint(): m_next_id(0) {
            m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
            m_endpoint.clear_error_channels(websocketpp::log::elevel::all);

            m_endpoint.init_asio();
            m_endpoint.start_perpetual();

            m_thread.reset(new websocketpp::lib::thread(&client::run, &m_endpoint));
        }

        ~websocket_endpoint() {
            m_endpoint.stop_perpetual();
    
            for (con_list::const_iterator it = m_connection_list.begin(); it != m_connection_list.end(); ++it) {
                if (it->second->get_status() != "Open") {
                    continue;
                }
                
                std::cout << "> Closing connection " << it->second->get_id() << std::endl;
                
                websocketpp::lib::error_code ec;
                m_endpoint.close(it->second->get_hdl(), websocketpp::close::status::going_away, "", ec);
                if (ec) {
                    std::cout << "> Error closing connection " << it->second->get_id() << ": "  
                            << ec.message() << std::endl;
                }
            }
            
            m_thread->join();
        }

        int connect (std::string const &uri) {

            int new_id = m_next_id++;

            m_endpoint.set_tls_init_handler(websocketpp::lib::bind(
                                            &on_tls_init
                                            ));

            websocketpp::lib::error_code ec;
            client::connection_ptr con = m_endpoint.get_connection(uri, ec);

            if(ec){
                std::cout << "Connection initialization error: " << ec.message() << std::endl;
                return -1;
            }

            connection_metadata::ptr metadata_ptr(new connection_metadata(new_id, con->get_handle(), uri));
            m_connection_list[new_id] = metadata_ptr;

            con->set_open_handler(websocketpp::lib::bind(
                                  &connection_metadata::on_open,
                                  metadata_ptr,
                                  &m_endpoint,
                                  websocketpp::lib::placeholders::_1
                                  ));

            con->set_fail_handler(websocketpp::lib::bind(
                                  &connection_metadata::on_fail,
                                  metadata_ptr,
                                  &m_endpoint,
                                  websocketpp::lib::placeholders::_1
                                  ));
            con->set_close_handler(websocketpp::lib::bind(
                                   &connection_metadata::on_close,
                                   metadata_ptr,
                                   &m_endpoint,
                                   websocketpp::lib::placeholders::_1
                                  ));
            con->set_message_handler(websocketpp::lib::bind(
                                     &connection_metadata::on_message,
                                     metadata_ptr,
                                     websocketpp::lib::placeholders::_1,
                                     websocketpp::lib::placeholders::_2
                                    ));

            m_endpoint.connect(con);
    
            return new_id;
        }

        connection_metadata::ptr get_metadata(int id) const {
            con_list::const_iterator metadata_it = m_connection_list.find(id);
            if (metadata_it == m_connection_list.end()) {
                return connection_metadata::ptr();
            } else {
                return metadata_it->second;
            }
        }

        void close(int id, websocketpp::close::status::value code, std::string reason) {
            websocketpp::lib::error_code ec;

            con_list::iterator metadata_it = m_connection_list.find(id);
            if (metadata_it == m_connection_list.end()) {
                std::cout << "> No connection found with id " << id << std::endl;
                return;
            }

            m_endpoint.close(metadata_it->second->get_hdl(), code, "", ec);
            std::cout << "Connection closed with reason: " << reason << std::endl;
        }

        int send(int id, std::string message) {
            websocketpp::lib::error_code ec;
        
            con_list::iterator metadata_it = m_connection_list.find(id);

            if (metadata_it == m_connection_list.end()) {
                std::cout << "> No connection found with id " << id << std::endl;
                return -1;
            }
            
            m_endpoint.send(metadata_it->second->get_hdl(), message, websocketpp::frame::opcode::text, ec);
            if (ec) {
                std::cout << "> Error sending message: " << ec.message() << std::endl;
                return -1;
            }
            
            metadata_it->second->record_sent_message(message);
            metadata_it->second->record_summary(message, "SENT");
            return id;
        }
};

int main(){
    bool done = false;
    std::string input;
    websocket_endpoint endpoint;

    fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold,
            "-------------------------- DERIBOT VERSION {}.{} --------------------------\n", 
            DeriBot_VERSION_MAJOR, DeriBot_VERSION_MINOR);
    std::cout << "Type 'help' to check out all available commands\n" << std::endl;
              
    while(!done) {
        fmt::print(fg(fmt::color::cyan) | fmt::emphasis::italic,
            "Enter command: ");
        while (std::getline(std::cin, input) && input.empty()) {}   

        if(input == "quit" || input == "exit"){
            done = true;
        } 
        else if(input == "help"){
            std::cout << "\nCOMMAND LIST:\n"
            << "> help: Displays this help text\n"
            << "> quit / exit: exits the program\n"
            << "> connect <URI>: creates a connection with the given URI\n"
            << "> close <id> <code (optional)> <reason (optional)>: closes the connection with the given id with optionally specifiable exit_code and/or reason\n"
            << "> show <id>: Gets the metadata of the connection with the given id\n"
            << "> show_messages <id>: Gets the complete messages sent and received on the connection with the given id\n"
            << "> send <id> <message>: Sends the message to the specified connection\n\n"
            << "DERIBIT API COMMANDS\n"
            << "> DERIBIT connect: Creates a new connection to the Deribit testnet website\n"
            << "> DERIBIT <id> authorize <client_id> <client_secret>: sends the authorization message to retrieve the access token\n\tAn optional flag -r can be set to remember the access_token for the rest of the session\n"
            << "> DERIBIT <id> buy <instrument> <comments>: Sends a buy order via the connection with id <id> for the instrument specified\n"
            << "> DERIBIT <id> sell <instrument> <comments>: Sends a sell order via the connection with id <id> for the instrument specified\n"
            << "> DERIBIT <id> get_open_orders {options}: Gets all the open orders\n"
            << "> DERIBIT <id> modify <order_id>: Allows you to modify the price and amount of an active order with known order id"
            << "> DERIBIT <id> cancel <order_id>: Allows you to cancel a specific order"
            << std::endl;
        }
        else if (input.substr(0,7) == "connect") {
            int id = endpoint.connect(input.substr(8));
            if (id != -1) {
                std::cout << "> Created connection with id " << id << std::endl;
                std::cout << "> Status: " << endpoint.get_metadata(id)->get_status() << std::endl;
            }
        } 
        else if (input.substr(0,13) == "show_messages") {
            int id = atoi(input.substr(9).c_str());
 
            connection_metadata::ptr metadata = endpoint.get_metadata(id);
            std::vector<std::string>::const_iterator it;
            for (it = metadata->m_messages.begin(); it != metadata->m_messages.end(); ++it) {
                std::cout << *it << "\n\n";
            }
        }
        else if (input.substr(0,4) == "show") {
            int id = atoi(input.substr(5).c_str());
 
            connection_metadata::ptr metadata = endpoint.get_metadata(id);
            if (metadata) {
                std::cout << *metadata << std::endl;
            } else {
                std::cout << "> Unknown connection id " << id << std::endl;
            }
        }
        else if(input.substr(0, 5) == "close"){
            std::stringstream ss(input);

            std::string cmd;
            int id;
            int close_code = websocketpp::close::status::normal;
            std::string reason;

            ss >> cmd >> id >> close_code;
            std::getline(ss, reason);

            endpoint.close(id, close_code, reason);
        }
        else if (input.substr(0,4) == "send") {
            std::stringstream ss(input);
                
            std::string cmd;
            int id;
            std::string message = "";
            
            ss >> cmd >> id;
            std::getline(ss,message);
            
            endpoint.send(id, message);

            std::unique_lock<std::mutex> lock(endpoint.get_metadata(id)->mtx);
            endpoint.get_metadata(id)->cv.wait(lock, [&] { return endpoint.get_metadata(id)->MSG_PROCESSED;});
            endpoint.get_metadata(id)->MSG_PROCESSED = false;
        }
        else if (input == "DERIBIT connect") {
            std::string uri = "wss://test.deribit.com/ws/api/v2";
            int id = endpoint.connect(uri);
            if (id != -1) {
                std::cout << "> Created connection to DERIBIT TESTNET with id " << id << std::endl;
                std::cout << "> Status: " << endpoint.get_metadata(id)->get_status() << std::endl;
            }
        }
        else if (input.substr(0, 7) == "DERIBIT") {
            int id; 
            std::string cmd;

            std::stringstream ss(input);
            ss >> cmd >> id;
            
            std::string msg = deribit_api::process(input);
            if (msg != ""){
                int success = endpoint.send(id, msg);
                if (success >= 0){
                std::unique_lock<std::mutex> lock(endpoint.get_metadata(id)->mtx);
                endpoint.get_metadata(id)->cv.wait(lock, [&] { return endpoint.get_metadata(id)->MSG_PROCESSED;});
                endpoint.get_metadata(id)->MSG_PROCESSED = false;
                }
            }
        }
        else{
            std::cout << "Unrecognized command" << std::endl;
        }
    }
    return 0;
}
