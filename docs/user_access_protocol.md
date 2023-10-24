# user access

## 协议

URL构成，HOST为rtio服务主机地址，DEVICE_ID为设备标识。

```text
http://$HOST/$DEVICE_ID/post_handler  # 向设备发送POST请求
http://$HOST/$DEVICE_ID/get_handler   # 向设备发送GET请求
http://$HOST/$DEVICE_ID/obget_handler # 向设备发送观察请求
```

请求参数

|参数 |类型   |最大长度|必选 | 描述|
|:---|:------|:-------|:---|:-----|
| uri|string |?  |是|设备内部的uri，设备上会绑定handler到该uri上|
| id |string |?  |是|请求标识，每个请求唯一，响应中该字段会与之匹配|
| data |base64-string |?  |否|data为byte数组（二进制），经过base64进行传输。如果发到QueryString中传输，需要经过urlencode后传输|

（? —— 待定义）

响应参数

|参数 |类型   |最大长度|必选 | 描述|
|:---|:------|:-------|:---|:-----|
| uri|string |?  |是|设备内部的uri，设备上会绑定handler到该uri上|
| id |string |?  |是|请求标识，每个请求唯一，响应中该字段会与之匹配|
| data |base64-string |?  |否|data为byte数组（二进制），经过base64进行传输。|

## 样例

```sh
$ curl -X POST "http://127.0.0.1:17317/cfa09baa-4913-4ad7-a936-2e26f9671b04/post_handler"  -d '{"uri":"/printer/action","id":12667,"data":"c3RhcnQ="}'
{"id":12667, "code":"CODE_OK", "data":"cHJpbnQgc3RhcnRlZA=="}

$ curl -X GET "http://127.0.0.1:17317/cfa09baa-4913-4ad7-a936-2e26f9671b04/obget_handler?uri=%2Fprinter%2Fstatus&id=12334&data=MTIzNDU%3D"
{"result":{"id":12334,"fid":0,"code":"CODE_CONTINUE","data":"cHJpbnRpbmcgMzMl"}}
{"result":{"id":12334,"fid":1,"code":"CODE_CONTINUE","data":"cHJpbnRpbmcgMzcl"}}
{"result":{"id":12334,"fid":2,"code":"CODE_CONTINUE","data":"cHJpbnRpbmcgNDAl"}}
...
{"result":{"id":12334,"fid":20,"code":"CODE_CONTINUE","data":"cHJpbnRpbmcgMTAwJQ=="}}
{"result":{"id":12334,"fid":21,"code":"CODE_TERMINATE","data":""}}

```

```sh
$ curl -X POST "http://127.0.0.1:17317/cfa09baa-4913-4ad7-a936-3e26f9671b09/post_handler" -d '{"uri":"/hello","id":12667,"data":"aGVsbG8="}'
{"id":12667, "code":"CODE_OK", "data":"d29ybGQ="}
```
