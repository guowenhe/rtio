/*
 * ServerGlobal.h
 *
 *  Created on: Jan 7, 2020
 *      Author: wenhe
 */

#ifndef SERVERGLOBAL_H_
#define SERVERGLOBAL_H_

#include <Ice/Ice.h>

class ServerGlobal
{
private:
    ServerGlobal(){}

public:
    static ServerGlobal* getInstance()
    {
        static ServerGlobal instance;
        return &instance;
    }

    int init(std::shared_ptr<Ice::Communicator> communicator)
    {
        if(nullptr == communicator)
        {
            std::cerr << "ServerGlobal communicator init error" << std::endl;
            return -1;
        }
        _communicator = communicator;
        return 0;
    }
    std::shared_ptr<Ice::Communicator> getCommunicator()
    {
        return _communicator;
    }
private:
    std::shared_ptr<Ice::Communicator> _communicator;
};



#endif /* SERVERGLOBAL_H_ */
