#pragma once

module DMS
{
    enum ClientStatus
    {
        UNKNOWN = 0,
        ONLINE,
        OFFLINE, 
    }

    class SetStatusReq
    {
        int sn;
        string deviceId;
        ClientStatus status;
        string accessProxy;
    }
    class SetStatusResp
    {
        int sn;
        int code = -1;
    }
    
    class QueryStatusReq
    {
        int sn;
        string deviceId;
    }
    class QueryStatusResp
    {
        int sn;
        int code = -1;
        ClientStatus status;
        string accessProxy;
    }
    
    interface StatusServer
    {
        ["amd"] void setStatus(SetStatusReq req, out SetStatusResp resp);
        ["amd"] void queryStatus(QueryStatusReq req, out QueryStatusResp resp);
    }
    
} //DMS

