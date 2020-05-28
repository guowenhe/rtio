// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// **********************************************************************
#include <Ice/Ice.h>
#include <vector>
#include <RtioLog.h>
#include "RtioExcept.h"
#include "DeviceHubI.h"

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

            auto properties = communicator()->getProperties();

            auto adapterName = properties->getProperty("AdapterName");

            auto adapter = communicator()->createObjectAdapter(adapterName);
            adapter->add(std::make_shared<DeviceHubAI>(), Ice::stringToIdentity(properties->getProperty("IdentityA")));
            adapter->add(std::make_shared<DeviceHubBI>(), Ice::stringToIdentity(properties->getProperty("IdentityB")));
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

