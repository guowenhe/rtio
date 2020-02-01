#pragma once

module DMS
{
    class MessageAReq
    {
        int sn;
        string deviceId;
        string message;
    }
    class MessageAResp
    {
        int sn;
        int code = -1;
    }
    
    class MessageBReq
    {
        int sn;
        string deviceId;
        string message;
    }
    class MessageBResp
    {
        int sn;
        int asCode = -1; // access server return code
        int code = -1;
    } 
    
    interface AccessServer
    {
        ["amd"] void dispatch(MessageAReq req, out MessageAResp resp);
    }
  
    interface MessageHubA
    {
        int addAccessServer(AccessServer* server);
        // report message, call by access server
        ["amd"] void report(MessageAReq req, out MessageAResp resp);
    }
    interface MessageHubB
    {
    	// dispatch message to client, call by message router
        ["amd"] void dispatch(MessageBReq req, out MessageBResp resp);        
    }
} //DMS

