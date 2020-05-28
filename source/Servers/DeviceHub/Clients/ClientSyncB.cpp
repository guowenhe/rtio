/*
 * ClientB.cpp
 *
 *  Created on: Dec 19, 2019
 *      Author: wenhe
 */

#include <Ice/Ice.h>
#include "DeviceHub.h"

using namespace std;

int main(int argc, char* argv[])
{
    cout << "client start ..." << endl;
    if(argc > 2)
    {
        cerr << argv[0] << ": too many arguments" << endl;
        return -1;
    }

    try
    {
        Ice::CommunicatorHolder ich(argc, argv, "/home/wenhe/Work/rtio-project/cluster/config.client");
        auto communicator = ich.communicator();
        if(!communicator)
        {
            cerr << "communicator error" << endl;
            return -1;
        }

        auto server = Ice::checkedCast<DMS::DeviceHubBPrx>(communicator->stringToProxy("DMS.DeviceHubB"));
        if(!server)
        {
            cerr << "invalid server proxy" << endl;
            return -1;
        }

        for(int i = 0; i < 500; ++i)
        {

            auto req = make_shared<DMS::MessageBReq>();
            auto resp = make_shared<DMS::MessageBResp>();
            req->sn = time(NULL);
            req->deviceId = argv[1]; // "device001";
            req->message = std::to_string(i) + " message form b";
            server = server->ice_connectionCached(false);
            server = server->ice_locatorCacheTimeout(2);
//            server = server->ice_invocationTimeout(5000);
            server->dispatch(req, resp);
            cout << resp->sn << "," << resp->deviceCode <<","<< resp->code << endl;
            sleep(1);
        }

        sleep(120);

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
