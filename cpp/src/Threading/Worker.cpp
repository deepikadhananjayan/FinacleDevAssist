#include "Worker.h"
#include "../Utils/Logger.h"
#include "../Configuration/FDAConfig.h"
#include "../Process/JavaProcess.h"
#include "../Core/FDAApplication.h"
#include "../Communication/FDAClient.h"
#include "../Communication/Handlers/TokenHandler.h"
#include "../Communication/Handlers/ValidationHandler.h"
#include "../Communication/Handlers/CodeBeautifierHandler.h"
#include "../Communication/Handlers/FIExecutionHandler.h"

Worker::Worker()
{
    running = false;
    javaProcess = std::make_unique<JavaProcess>();
    client = std::make_unique<FDAClient>();
    tokenHandler = std::make_unique<TokenHandler>(client.get());
    validationHandler = std::make_unique<ValidationHandler>(client.get());
    codeBeautifierHandler = std::make_unique<CodeBeautifierHandler>(client.get());
    fiExecutionHandler = std::make_unique<FIExecutionHandler>(client.get());
}

Worker::~Worker() = default;

void Worker::start()
{
    running = true;
    workerThread = std::thread(&Worker::run, this);
}

void Worker::run()
{
    Logger::info("[WORKER] Thread started");

    while (running)
    {
        Task task = taskQueue.pop();

        switch (task.type)
        {
        case TaskType::START_JAVA:
        {
            Logger::info("[WORKER] START_JAVA task");

            if (!javaProcess->start())
            {
                Logger::error("[WORKER] Java start failed");
                FDAApplication::updateState(FDAApplicationState::NOT_INITIALIZED);
                break;
            }

            Logger::info("[WORKER] Java started successfully");

            Sleep(1000);

            Logger::info("[WORKER] Loading configuration");

            if (!FDAConfig::load())
            {
                Logger::error("[WORKER] Configuration load failed");
                FDAApplication::updateState(FDAApplicationState::NOT_INITIALIZED);
                break;
            }

            Logger::info("[WORKER] Configuration loaded");

            Logger::info("[WORKER] Connecting Java socket");

            if (!client->connect())
            {
                Logger::error("[WORKER] Socket connection failed");
                FDAApplication::updateState(FDAApplicationState::NOT_INITIALIZED);
                break;
            }

            Logger::info("[WORKER] Socket connected");

            submit({ TaskType::GET_KEYWORDS_AND_USERHOOKS });

            break;
        }

        case TaskType::GET_KEYWORDS_AND_USERHOOKS:
        {
            Logger::info("[WORKER] GET_KEYWORDS_AND_USERHOOKS task");

            try
            {
                tokenHandler->getTokens();
                FDAApplication::updateState(FDAApplicationState::READY);
            }
            catch (const std::exception& e)
            {
                Logger::error("[WORKER] Exception : " + std::string(e.what()));
            }

            break;
        }

        case TaskType::VALIDATE_SCRIPT:
        {
            Logger::info("[WORKER] VALIDATE_SCRIPT task");

            try
            {
                validationHandler->validateScript(std::get<std::string>(task.data));
            }
            catch (const std::exception& e)
            {
                Logger::error("[WORKER] Exception : " + std::string(e.what()));
            }

            break;
        }

        case TaskType::BEAUTIFY_CODE:
        {
            Logger::info("[WORKER] BEAUTIFY_CODE task");

            try
            {
                const BeautifyData& data = std::get<BeautifyData>(task.data);
                codeBeautifierHandler->beautify(data);
            }
            catch (const std::exception& e)
            {
                Logger::error("[WORKER] Exception : " + std::string(e.what()));
            }

            break;
        }

        case TaskType::GENERATE_MENU_SOURCE:
        {
            Logger::info("[WORKER] GENERATE_MENU_SOURCE task");

            try
            {
                // TODO
            }
            catch (const std::exception& e)
            {
                Logger::error("[WORKER] Exception : " + std::string(e.what()));
            }
                
            break;
        }

        case TaskType::EXECUTE_FI_REQUEST:
        {
            Logger::info("[WORKER] EXECUTE_FI_REQUEST task");
            
            try
            {
                const FIRequestData& data = std::get<FIRequestData>(task.data);
                fiExecutionHandler->executeRequest(data);
            }
            catch (const std::exception& e)
            {
                Logger::error("[WORKER] Exception : " + std::string(e.what()));
            }

            break;
        }

        case TaskType::STOP_JAVA:
        {
            Logger::info("[WORKER] STOP_JAVA task");

            if (client->isConnected())
            {
                std::string response;
                client->sendRequest("{\"type\":\"SHUTDOWN\"}", response);
            }

            Sleep(500);

            javaProcess->stop();
            client->disconnect();

            break;
        }
        }
    }

    Logger::info("[WORKER] Thread stopped");
}

void Worker::submit(Task task)
{
    taskQueue.push(task);
}

void Worker::stop()
{
    running = false;

    /*
        Wake worker thread because pop()
        can be waiting.
    */
    taskQueue.push({ TaskType::STOP_JAVA });

    if (workerThread.joinable())
    {
        workerThread.join();
    }

    FDAApplication::updateState(FDAApplicationState::NOT_INITIALIZED);
}