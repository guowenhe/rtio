module DMS
{
    class ReportReq
    {
        int sn;
        string deviceId;
        string message;
    }
    class ReportResp
    {
        int sn;
        int code = -1;
    } 
    
    class SendReq
    {
        int sn;
        string deviceId;
        string message;
    }
    class SendResp
    {
        int sn;
        int deviceCode = -1; // device return code
        int code = -1;
    } 

    interface MessageTriggerA
    {
        ["amd"] void report(ReportReq req, out ReportResp resp);        
    }
    interface MessageTriggerB
    {
        ["amd"] void send(SendReq req, out SendResp resp);        
    }

} //DMS