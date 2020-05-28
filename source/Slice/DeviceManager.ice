#pragma once

module DMS
{
	enum AuthCode
	{
		PASS = 1,
		FAIL = 2,
	}

    class CreateDeviceReq
    {
        int sn;
        string nonce;
    }
    class CreateDeviceResp
    {
        int sn;
        int code = -1;
        string deviceId;
        string deviceKey;
    }
    
    class AuthDeviceReq
    {
        int sn;
        string deviceId;
        string deviceKey;
    }
    class AuthDeviceResp
    {
        int sn;
        int code = -1;
        AuthCode authCode = FAIL;
    }
    
    interface DeviceManager
    {
        ["amd"] void createDevice(CreateDeviceReq req, out CreateDeviceResp resp);      
        ["amd"] void authDevice(AuthDeviceReq req, out AuthDeviceResp resp);      
    }
} //DMS

