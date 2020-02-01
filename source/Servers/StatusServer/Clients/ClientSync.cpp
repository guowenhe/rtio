/*
 * ClientSync.cpp
 *
 *  Created on: Jan 8, 2020
 *      Author: wenhe
 */


#include <Ice/Ice.h>
#include "StatusServer.h"
using namespace std;


ostream& operator<<(ostream& out, DMS::ClientStatus status)
{
    switch(status)
    {
    case DMS::ClientStatus::UNKNOWN:
        out << "UNKNOWN";
        break;
    case DMS::ClientStatus::ONLINE:
        out << "ONLINE";
        break;
    case DMS::ClientStatus::OFFLINE:
        out << "OFFLINE";
        break;
    default:
        out << "no-desc(" << static_cast<int>(status) << ")";
    }

    return out;
}





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

        auto server = Ice::checkedCast<DMS::StatusServerPrx>(communicator->stringToProxy("DMS.StatusServer"));
        if(!server)
        {
            cerr << "invalid server proxy" << endl;
            return -1;
        }

//        for(int i = 0; i < 50; ++i)
//        {
//
//            auto req = make_shared<DMS::SetStatusReq>();
//            auto resp = make_shared<DMS::SetStatusResp>();
//            req->sn = time(NULL);
//            req->deviceId = "deviceId-" + std::to_string(i);
//            req->status = DMS::ClientStatus::ONLINE;
//            req->accessProxy = "accessProxy-" + std::to_string(i);
//            server->setStatus(req, resp);
//            cout << resp->sn << "," << resp->code << endl;
//            sleep(1);
//
//        }

        for(int i = 0; i < 50; ++i)
        {

            auto req = make_shared<DMS::QueryStatusReq>();
            auto resp = make_shared<DMS::QueryStatusResp>();
            req->sn = time(NULL);
            req->deviceId = "deviceId-" + std::to_string(i);
            server->queryStatus(req, resp);

            cout << resp->sn << "," << resp->code << "," << resp->status << "," << resp->accessProxy <<  endl;
            sleep(1);

        }

//        sleep(1);

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

