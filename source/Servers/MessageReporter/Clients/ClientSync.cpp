/*
 * ClientSyncA.cpp
 *
 *  Created on: Dec 19, 2019
 *      Author: wenhe
 */

#include <Ice/Ice.h>
#include "MessageReporter.h"

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

        auto server = Ice::checkedCast<DMS::MessageReporterPrx>(communicator->stringToProxy("DMS.MessageReporter"));
        if(!server)
        {
            cerr << "invalid server proxy" << endl;
            return -1;
        }


        for(int i = 11; i < 12; ++i)
        {

            auto req = make_shared<DMS::ReportReq>();
            auto resp = make_shared<DMS::ReportResp>();
            req->sn = time(NULL);
            req->message = "this a message" + std::to_string(i);
            req->deviceId = "DEVICEID0001";
            req->messageId = 100001 + i;
            server = server->ice_connectionCached(false);
            server = server->ice_locatorCacheTimeout(2);
            server->report(req, resp);
            cout << resp->sn << "," << resp->code << endl;
//            sleep(1);

        }

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



