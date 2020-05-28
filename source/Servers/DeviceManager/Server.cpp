// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// **********************************************************************
#include <Ice/Ice.h>
#include <vector>

#include "DeviceManagerI.hpp"
#include "RtioLog.h"
#include "RtioExcept.h"
#include "ServerGlobal.h"

using namespace DMS;

class Server: public Ice::Application
{
public:

    virtual int run(int argc, char*[]) override
    {
        try
        {

            std::cout << "server start " << "@" << IceUtil::Time::now().toDateTime() << std::endl;
            Ice::Error err(communicator()->getLogger());
            if(argc != 1)
            {
                throw Rtio::Except<int>(EXIT_FAILURE, Rtio_where() + "arguments error");
            }
            if(ServerGlobal::getInstance()->init(communicator()))
            {
                throw Rtio::Except<int>(EXIT_FAILURE,  Rtio_where() + "ServerGlobal communicator init error");
            }

            auto properties = communicator()->getProperties();

            auto config = std::make_shared<DeviceManagerConfig>();
            config->redisHost = properties->getProperty("RedisHost");
            config->redisPort = properties->getPropertyAsInt("RedisPort");
            const int workerNum = properties->getPropertyAsInt("WorkerNum");
            auto workers = std::make_shared<DeviceManagerWorkers>(workerNum);
            if(workers->start(config))
            {
                throw Rtio::Except<int>(EXIT_FAILURE, "workers start failed");
            }

            auto adapterName = properties->getProperty("AdapterName");

            auto adapter = communicator()->createObjectAdapter(adapterName);
            adapter->add(std::make_shared<DeviceManagerI>(workers), Ice::stringToIdentity(properties->getProperty("Identity")));
            adapter->activate();
            log2I(communicator(), "server started");
            communicator()->waitForShutdown();
        }
        catch(Rtio::Except<int>& ex)
        {
            log2E(communicator(), ex.what());
            return ex.code();
        }
        catch(std::exception& ex)
        {
            log2E(communicator(), ex.what());
            return EXIT_FAILURE;
        }
        log2I(communicator(), "server exit gracefully");
        return EXIT_SUCCESS;
    }
};

int main(int argc, char *argv[])
{
    Server app;
    return app.main(argc, argv);
}

