# rtio
RTIO - Run Time Input Output Service for IoT

![top](./doc/top.PNG)

RTIO（Run Time Input Output）Service是IoT云平台中的一个服务，可轻 松使用 Rest API控制和管理物联网设备。RTIO Service设计以实时性和易用 性为目标，比如下发消息（Rest API），传统 IoT云平台在http response仅仅 表示该消息是否发到云平台，但不确定设备是否接收到（设备需要再上报一条 消息作为确认）；而这里的 response 直接带回设备接收到消息后的响应 （return code） 。 

该项目采用 c/c++开发，更容易移植到边缘计算平台，比如树莓派或 beaglebone等嵌入式平台；使用zeroc ice为通信中间件，实现服务集群。 

目前该项目在开发中。  