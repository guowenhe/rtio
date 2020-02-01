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
    
    class PushReq
    {
        int sn;
        string deviceId;
        string message;
    }
    class PushResp
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
        ["amd"] void push(PushReq req, out PushResp resp);        
    }

} //DMS