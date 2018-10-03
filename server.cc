#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <signal.h>
#include "err.h"
#include "server_c.h"
#include "user_interface.h"

using namespace std;

Server server;

void sig_handler(int signo)
{
    if (signo == SIGINT)
    {
	server.close_server();
    }
    exit(signo);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Wrong number of parameters\n");
        return 1;
    }
    if (signal(SIGINT, sig_handler) == SIG_ERR) syserr("can't catch SIGINT");
    int PORT_NUM = atoi(argv[1]);
    server.set_port(PORT_NUM);
    server.open_socket();
    server.bind_socket();
    server.listen_on_socket();
    server.run();
    return 0;
}
