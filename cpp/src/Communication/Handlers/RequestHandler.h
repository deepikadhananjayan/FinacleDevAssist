#pragma once

#include "../FDAClient.h"

class RequestHandler
{
protected:
    FDAClient* client;

public:
    explicit RequestHandler(FDAClient* client)
        : client(client)
    {
    }
};   