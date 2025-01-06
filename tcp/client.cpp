#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <chrono>
#include "tcpconnector.h"

using namespace std;
using namespace std::chrono;

void measureLatency(TCPStream* stream, const string& message) {
    char line[1048576];
    int len;

    auto start = high_resolution_clock::now();

    // Send message
    ssize_t sent = stream->send(message.c_str(), message.size());
    if (sent <= 0) {
        printf("Error sending message\n");
        return;
    }
    printf("sent - %s\n", message.c_str());

    string response;
    while ((len = stream->receive(line, sizeof(line))) > 0) {
        line[len] = '\0';
        response += line;

        // Check if we've received the complete response
        if (len < sizeof(line)) break;
    }

    if (len < 0) {
        printf("Error receiving response\n");
        return;
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);

    printf("received - %s\n", line);
    printf("latency: %lld microseconds (%.3f ms)\n\n", duration.count(), duration.count() / 1000.0);
}


int main(int argc, char** argv) {
    if (argc != 3) {
        printf("usage: %s <port> <ip>\n", argv[0]);
        exit(1);
    }

    TCPConnector* connector = new TCPConnector();
    TCPStream* stream = connector->connect(atoi(argv[1]), argv[2]);

    if(stream) {
        measureLatency(stream, "10");
        delete stream;
    }

    delete connector;
    return 0;
}
