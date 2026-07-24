#pragma once

#include "RequestHandler.h"

class TokenHandler : public RequestHandler
{
public:
    explicit TokenHandler(FDAClient* client);

    void getTokens();
};   