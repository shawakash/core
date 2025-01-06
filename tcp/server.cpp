#include <stdio.h>
#include <stdlib.h>
#include "systeminfo.h"
#include "tcpacceptor.h"

bool isNumber(const string& str) {
    if(str.empty()) return false;

    for(char const &c : str) {
        if(isdigit(c) == 0) return false;
    }
    return true;
}

int main(int argc, char** argv) {
    if (argc < 2 || argc > 4) {
        printf("usage: server <port> [<ip>]\n");
        exit(1);
    }

    TCPStream* stream = NULL;
    TCPAcceptor* acceptor = NULL;
    if (argc == 3) {
        acceptor = new TCPAcceptor(atoi(argv[1]), argv[2]);
    }
    else {
        acceptor = new TCPAcceptor(atoi(argv[1]));
    }
    if (acceptor->start() == 0) {
        while (1) {
            stream = acceptor->accept();
            if (stream != NULL) {
                size_t len;
                char line[256];
                while ((len = stream->receive(line, sizeof(line))) > 0) {
                    line[len] = '\0';
                    string command(line);
                    printf("received - %s\n", line);

                    if (isNumber(command)) {
                        int n = stoi(command);

                        string sysinfo = SystemInfo::getSystemStats(n);
                        stream->send(sysinfo.c_str(), sysinfo.length());
                    } else {
                        string error = "Sorry, haven't yet included this in our system. Use 'Get System Info' or 'Get System Info - n'";
                        stream->send(error.c_str(), error.length());
                    }
                }
                delete stream;
            }
        }
    }
    perror("Could not start the server");
    exit(-1);
}
