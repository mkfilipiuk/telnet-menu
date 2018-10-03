#ifndef SIK_SERVER
#define SIK_SERVER

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdio>
#include <string>
#include <unistd.h>
#include <memory>
#include <map>
#include <list>
#include "err.h"
#include "user_interface.h"

class Server {
private:
    static const int buffer_size = 10000;
    static const int queue_lenght = 5;
    int port;
    char buffer[buffer_size];
    char add_buf[3];
    std::string chosen_option = "";
    int sock, msg_sock;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    socklen_t client_address_len;
    ssize_t len, snd_len;
    std::shared_ptr<User_interface> user_interface, clean_user_interface = nullptr;
    bool is_connected = false;
    std::list<char> buffer_l;

    action_e analize_input();

    void accept_client();

    void configure_telnet();

    void close_connection();

    void read_to_buffer();

    void write_to_client();

    void clear_buffer();

public:
    Server(){}

    void set_port(int port);

    void open_socket();

    void bind_socket();

    void listen_on_socket();

    void run();

    void write_message_to_client(std::string message);

    void close_server();
};

#endif 
