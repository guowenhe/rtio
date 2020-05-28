#include <Ice/Ice.h>
#include <vector>
#include <RtioLog.h>
#include "RtioExcept.h"
#include "ServerGlobal.h"
#include "APINotifierI.h"

using namespace DMS;

class Server: public Ice::Application
{
public:

    virtual int run(int argc, char*[]) override
    {
        try
        {
            std::cout << "server start "  << "@" << IceUtil::Time::now().toDateTime() << std::endl;
            if(argc != 1)
            {
                throw Rtio::Except<int>(EXIT_FAILURE, Rtio_where() + "arguments error");
            }
            if(ServerGlobal::getInstance()->init(communicator()))
            {
                throw Rtio::Except<int>(EXIT_FAILURE,  Rtio_where() + "ServerGlobal communicator init error");
            }

            auto properties = communicator()->getProperties();
            int iocThreadNum= properties->getPropertyAsIntWithDefault("IocThreadNum", 2);

            // create io_context.
            asio::io_context ioc;
            auto guard = asio::make_work_guard(ioc);
            std::vector<std::thread> v;
            v.reserve(iocThreadNum);
            for (auto i = iocThreadNum; i > 0; --i)
            {
                v.emplace_back([&ioc]
                {
                    ioc.run();
                });
            }

            // active ice servant
            auto adapterName = properties->getProperty("AdapterName");
            auto adapter = communicator()->createObjectAdapter(adapterName);
            adapter->add(std::make_shared<APINotifierI>(ioc), Ice::stringToIdentity(properties->getProperty("Identity")));
            adapter->activate();

            // set generator server index
            int serverIndex= properties->getPropertyAsInt("serverIndex");
            Util::IdGenerator::getInstance()->setIndex(serverIndex);
            log2I(communicator(), "server started");
            communicator()->waitForShutdown();

            // stop the io_context.
            guard.reset();
            log2I(communicator(), "guard reseted");
            for(auto& t : v)
            {
                t.join();
            }
            log2I(communicator(), "ioc threads joined");
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

