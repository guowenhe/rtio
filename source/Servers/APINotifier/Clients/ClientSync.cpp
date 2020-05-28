
#include <Ice/Ice.h>
#include "APINotifier.h"
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

        auto server = Ice::checkedCast<DMS::APINotifierPrx>(communicator->stringToProxy("DMS.APINotifier"));
        if(!server)
        {
            cerr << "invalid server proxy" << endl;
            return -1;
        }


        for(int i = 0; i < 1; ++i)
        {

            auto req = make_shared<DMS::NotifyReq>();
            auto resp = make_shared<DMS::NotifyResp>();
            req->sn = time(NULL);
            req->message = "hello appservice!";
            req->deviceId = "deviceId-" + std::to_string(i);
            server->notify(req, resp);

            cout << resp->sn << "," << resp->code << endl;
            cout << "resp->message=" << resp->message << endl;
//            sleep(1);

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

