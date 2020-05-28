module DMS
{
    class ReportReq
    {
        int sn;
        string deviceId;
        int messageId;	
        string message;
    }
    class ReportResp
    {
        int sn;
        int code = -1;
    } 
    

    interface MessageReporter
    {
        ["amd"] void report(ReportReq req, out ReportResp resp);        
    }

} //DMS