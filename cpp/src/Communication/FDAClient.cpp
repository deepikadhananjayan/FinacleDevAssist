#include "FDAClient.h"
#include "../Utils/Logger.h"
#include "../Configuration/FDAConfig.h"

FDAClient::FDAClient()
{
    socket = INVALID_SOCKET;
    connected = false;
}

FDAClient::~FDAClient()
{
    disconnect();
}

bool FDAClient::connect()
{
    if (connected)
        return true;

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (result != 0)
    {
        Logger::error("[CLIENT] WSAStartup failed");
        return false;
    }

    socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    DWORD timeout = 35000;

    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    if (socket == INVALID_SOCKET)
    {
        Logger::error("[CLIENT] Socket creation failed");
        WSACleanup();
        return false;
    }

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons((u_short)FDAConfig::getJavaPort());
    inet_pton(AF_INET, FDAConfig::getJavaHost().c_str(), &server.sin_addr);

    result = ::connect(socket, (sockaddr*)&server, sizeof(server));

    if (result == SOCKET_ERROR)
    {
        Logger::error("[CLIENT] Connection failed");
        closesocket(socket);
        socket = INVALID_SOCKET;
        WSACleanup();
        return false;
    }

    connected = true;
    Logger::info("[CLIENT] Connected to Java");

    return true;
}

bool FDAClient::isConnected()
{
    return connected;
}

bool FDAClient::sendRequest(const std::string& request, std::string& response)
{
    if (!connected)
    {
        Logger::error("[CLIENT] Not connected");
        return false;
    }

    Logger::debug("[CLIENT] Sending : " + request);

    std::string reqMsg = request + "\n";

    int sent = ::send(socket, reqMsg.c_str(), (int)reqMsg.length(), 0);

    if (sent == SOCKET_ERROR)
    {
        Logger::error("[CLIENT] Send failed");
        return false;
    }

    response.clear();
    char ch;

    while (true)
    {
        int received = ::recv(socket, &ch, 1, 0);

        if (received <= 0)
        {
            Logger::error("[CLIENT] Receive failed");
            return false;
        }

        if (ch == '\n')
            break;

        response += ch;
    }

    return true;
}

void FDAClient::disconnect()
{
    if (socket != INVALID_SOCKET)
    {
        closesocket(socket);
        socket = INVALID_SOCKET;
    }

    if (connected)
        WSACleanup();

    connected = false;
    Logger::info("[CLIENT] Disconnected");
}