#include "server_c.h"
#include <memory>

void Server::set_port(int port){
    this->port = port;
}

void Server::clear_buffer() {
    for (size_t i = 0; i < buffer_size; ++i)buffer[i] = 0;
}

void Server::open_socket() {
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) syserr("socket");
}

void Server::bind_socket() {
    server_address.sin_family = AF_INET; // IPv4
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // listening on all interfaces
    server_address.sin_port = htons(port); // listening on port PORT_NUM
    if (bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) syserr("bind");
}

void Server::listen_on_socket() {
    if (listen(sock, queue_lenght) < 0) syserr("listen");
}

void Server::accept_client() {
    client_address_len = sizeof(client_address);
    msg_sock = accept(sock, (struct sockaddr *) &client_address, &client_address_len);
    if (msg_sock < 0) syserr("accept");
}

void Server::configure_telnet() {
    //telnet magic ;)
    unsigned char conf_telnet[3] = {255, 253, 34};
    int len = 3;
    if (write(msg_sock, conf_telnet, len) != len)syserr("Telnet config");
    len = 7;
    unsigned char conf_telnet2[7] = {255, 250, 34, 1, 0, 255, 240};
    if (write(msg_sock, conf_telnet2, len) != len)syserr("Telnet config");
    unsigned char conf_telnet25[3] = {255, 251, 1};
    len = 3;
    if (write(msg_sock, conf_telnet25, len) != len)syserr("Telnet config");
    unsigned char conf_telnet3[8] = "\e[?25l\r";
    len = 8;
    if (write(msg_sock, conf_telnet3, len) != len)syserr("Telnet config");
}

void Server::close_connection() {
    printf("Ending connection\n");
    chosen_option = "";
    if(!is_connected){
	printf("Strange...\nIt is already disconnected\n");
	return;
    }
    unsigned char conf_telnet3[7] = "\e[?25h";
    len = 7;
    if (write(msg_sock, conf_telnet3, len) != len)fprintf(stderr,"Problem with connection\n");
    if (close(msg_sock) < 0) fprintf(stderr,"Connection already closed\n");
    is_connected = false;
}

void Server::read_to_buffer() {
    clear_buffer();
    len = read(msg_sock, buffer, sizeof(buffer));
    if (len < 0) syserr("reading from client socket");
    for (int i = 0; i < len; ++i) {
        buffer_l.push_back(buffer[i]);
    }
}

void Server::write_to_client() {
    snd_len = write(msg_sock, buffer, len);
    if (snd_len != len) syserr("writing to client socket");
}

//Creating interface for the task
std::shared_ptr<User_interface> give_interface() {
    std::shared_ptr<User_interface> user_interface_A_ptr = std::make_shared<User_interface>("Opcja A", "A");
    std::shared_ptr<User_interface> user_interface_B1_ptr = std::make_shared<User_interface>("Opcja B1", "B1");
    std::shared_ptr<User_interface> user_interface_B2_ptr = std::make_shared<User_interface>("Opcja B2", "B2");
    std::shared_ptr<User_interface> user_interface_B_wstecz_ptr = std::make_shared<User_interface>("Wstecz");
    std::vector<std::shared_ptr<User_interface>> b;
    b.push_back(user_interface_B1_ptr);
    b.push_back(user_interface_B2_ptr);
    b.push_back(user_interface_B_wstecz_ptr);
    std::shared_ptr<User_interface> user_interface_B_ptr = std::make_shared<User_interface>("Opcja B", b);
    std::shared_ptr<User_interface> user_interface_menu_wstecz_ptr = std::make_shared<User_interface>("Koniec");
    std::vector<std::shared_ptr<User_interface>> menu;
    menu.push_back(user_interface_A_ptr);
    menu.push_back(user_interface_B_ptr);
    menu.push_back(user_interface_menu_wstecz_ptr);
    std::shared_ptr<User_interface> user_interface_menu_ptr = std::make_shared<User_interface>("Menu", menu);
    user_interface_B_wstecz_ptr->set_parent(user_interface_menu_ptr);
    return user_interface_menu_ptr;
}


void Server::run() {
    for (;;) {
        if(clean_user_interface != nullptr)clean_user_interface->destroy();
        user_interface = give_interface();
        clean_user_interface = user_interface;
        accept_client();
        printf("Accepting client connections on port %hu\n", ntohs(server_address.sin_port));
        configure_telnet();
        read_to_buffer();
        buffer_l.clear();
        is_connected = true;
	chosen_option = "";
	write_message_to_client(user_interface->give_message());
        do {
            read_to_buffer();
            action_e a_e = analize_input();
            user_interface = user_interface->act(a_e, user_interface, chosen_option);
            if (user_interface == nullptr) {
                close_connection();
                len = 0;
            }
            if (len > 0)write_message_to_client(user_interface->give_message());
        } while (len > 0);
	is_connected = false;
        printf("Connection terminated\n");
    }
}

void Server::close_server(){
    close_connection();
    if(clean_user_interface != nullptr)clean_user_interface->destroy();
    close(msg_sock);
}

void Server::write_message_to_client(std::string message) {
    clear_buffer();
    size_t i;
    for (i = 0; i < message.size(); ++i) buffer[i] = message[i];
    buffer[i] = '\n';
    ++i;
    size_t j = 0;
    if(chosen_option.compare("") != 0){
	for (j = 0; j < chosen_option.size(); ++j) buffer[i+j] = chosen_option[j];
    	buffer[i+j] = '\n';
    	++j;
    }
    len = i + j;
    write_to_client();
}


action_e Server::analize_input() {
    char b;
    int state = 0;
    std::vector<char>v;
    while (1) {
        if (buffer_l.size() == 0)
	{
		for(char c : v)buffer_l.push_back(c);
		return NONE;
	}
        b = buffer_l.front();
        v.push_back(b);
	buffer_l.pop_front();
	if(state == 0){
        	switch(b)
		{
		case '\n':
			return ENTER;
		case '\r':
			state = 1;
			break;
		case '\033':
			state = 2;
		default:
			break;
		}
		continue;
	}

	if(state == 1){
		if(b == '\0')return ENTER;
		state = 0;
		continue;
	}

	if(state == 2){
		if(b == '[') state = 3;
		else state = 0;
		continue;
	}
	
	if(state == 3)
	{
		if(b == 'A') return ARROW_UP;
		if(b == 'B') return ARROW_DOWN;
		state = 0;
	}
    }
    return NONE;
}


