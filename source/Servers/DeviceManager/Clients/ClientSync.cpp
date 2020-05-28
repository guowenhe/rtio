/*
 * ClientSyncA.cpp
 *
 *  Created on: Dec 19, 2019
 *      Author: wenhe
 */

#include <Ice/Ice.h>
#include "DeviceManager.h"

using namespace std;


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
        Ice::CommunicatorHolder ich(argc, argv, "/home/wenhe/Work/rtio-project/cluster/config.client");
        auto communicator = ich.communicator();
        if(!communicator)
        {
            cerr << "communicator error" << endl;
            return -1;
        }

        auto server = Ice::checkedCast<DMS::DeviceManagerPrx>(communicator->stringToProxy("DMS.DeviceManager"));
        if(!server)
        {
            cerr << "invalid server proxy" << endl;
            return -1;
        }


//        for(int i = 0; i < 1; ++i)
//        {
//            auto req = make_shared<DMS::CreateDeviceReq>();
//            auto resp = make_shared<DMS::CreateDeviceResp>();
//            req->sn = time(NULL);
//            req->nonce = "RD" + std::to_string(time(NULL));
//            server = server->ice_connectionCached(false);
//            server = server->ice_locatorCacheTimeout(2);
//            server->createDevice(req, resp);
//            cout << resp->sn << "," << resp->code << endl;
//            cout << resp->deviceId << " deviceKey[" << resp->deviceKey << "]"<<endl;
////            sleep(1);
//        }

        auto req = make_shared<DMS::AuthDeviceReq>();
        auto resp = make_shared<DMS::AuthDeviceResp>();
//        req->deviceId = "63eb0819f6654063bd3be8cc03f415bf";
        req->deviceKey = "yePKI+jFTjEuy3XtU9RQgpt7xDI=";
        req->sn = time(NULL);
        server = server->ice_connectionCached(false);
        server = server->ice_locatorCacheTimeout(2);
        server->authDevice(req, resp);
        cout << resp->sn << "," << resp->code << "," << (int)resp->authCode << endl;

//        sleep(120);

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



