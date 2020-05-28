/*
 * ClientAsyncA.cpp
 *
 *  Created on: Dec 23, 2019
 *      Author: wenhe
 */


#include <Ice/Ice.h>
#include "DeviceHub.h"
#include "RemoteCode.h"

using namespace std;

class AccessServerI : public DMS::AccessServer
{
public:
    virtual void dispatchAsync(::std::shared_ptr<DMS::MessageAReq> req,
            ::std::function<void(const ::std::shared_ptr<DMS::MessageAResp>& resp)> response,
            ::std::function<void(::std::exception_ptr)> exception, const ::Ice::Current& current) override
    {
        cout << req->sn << "," << req->message  << endl;
        auto resp = make_shared<DMS::MessageAResp>();
        resp->sn = req->sn;
        resp->code = RC::codeToInt(RC::Code::SUCCESS);

        response(resp);
    }


};


int main(int argc, char* argv[])
{
    cout << "client start ..." << endl;
    if(argc > 1)
    {
        cerr << argv[0] << ": too many arguments" << endl;
        return -1;
    }

    try
    {
        Ice::CommunicatorHolder ich(argc, argv, "/home/wenhe/ServerCluster/config.client");
        auto communicator = ich.communicator();
        if(!communicator)
        {
            cerr << "communicator error" << endl;
            return -1;
        }

        auto server = Ice::checkedCast<DMS::DeviceHubAPrx>(communicator->stringToProxy("DMS.DeviceHubA"));
        if(!server)
        {
            cerr << "invalid server proxy" << endl;
            return -1;
        }

        auto adapter = communicator->createObjectAdapter("AccessServer");

        auto prx = adapter->addWithUUID(make_shared<AccessServerI>());

        auto accessServerPrx = Ice::uncheckedCast<DMS::AccessServerPrx>(prx);
        adapter->activate();

        server->addAccessServer(accessServerPrx);
        cout << "identity[" << Ice::identityToString(accessServerPrx->ice_getIdentity()) << "]" << endl;
        cout << "prx[" << communicator->proxyToString(accessServerPrx) << "]" << endl;

        std::function<void(::std::shared_ptr<::DMS::MessageAResp>)> responseHandler = [](std::shared_ptr<::DMS::MessageAResp> resp)
        {
            cout <<"responseHandler:"<< resp->sn << "," << resp->code << endl;
        };
        std::function<void(::std::exception_ptr)> exceptionHandler = [](std::exception_ptr ex)
        {
            cout << "exceptionHandler" << endl;
            try
            {
                rethrow_exception(ex);
            }
            catch(const std::exception& ex)
            {
                cout << "exceptionHandler=" << ex.what() << endl;
            }
        };

        for(int i = 0; i < 500; ++i)
        {
            auto req = make_shared<DMS::MessageAReq>();
//            auto resp = make_shared<DMS::MessageAResp>();
            req->sn = time(NULL);
            req->message = std::to_string(i) + "async message";
            cout <<"reportAsync" << endl;
            server->reportAsync(req, responseHandler, exceptionHandler);
            cout <<"wait for reportAsync respone" << endl;
            sleep(10);
        }
    }
    catch(const Ice::ObjectNotExistException& ex)
    {
        cerr << argv[0] << ":ObjectNotExistException: " << ex.what() << endl;
    }
    catch(const std::exception& ex)
    {
        cerr << argv[0] << ":exception: " << ex.what() << endl;
        return -1;
    }

    return 0;
}



