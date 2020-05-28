module DMS
{
    class NotifyReq
    {
        int sn;
        string deviceId;
        string messageId;
        string message;
    }
    class NotifyResp
    {
        int sn;
        int code = -1;
        string message;
    } 
    
    interface APINotifier
    {
        ["amd"] void notify(NotifyReq req, out NotifyResp resp);        
    }

} //DMS