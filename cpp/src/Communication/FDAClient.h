#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

class FDAClient
{
public:
    FDAClient();
    ~FDAClient();

    bool connect();
    void disconnect();
    bool isConnected();
    bool sendRequest(const std::string& request, std::string& response);

private:
    SOCKET socket;
    bool connected;
};