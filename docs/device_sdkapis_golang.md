# golang device sdk

## golang接口

注册请求处理函数。参数中"uri"为URI字符串形式的摘要，采用CRC32计算，可通过项目里的uri_to_crc.py计算。

```golang

func (s *DeviceSession) RegisterGetHandler(uri uint32, handler func(req []byte) ([]byte, error)) error
func (s *DeviceSession) RegisterPostHandler(uri uint32, handler func(req []byte) ([]byte, error)) error 
func (s *DeviceSession) RegisterObGetHandler(uri uint32, handler func(ctx context.Context, req []byte) (<-chan []byte, error)) error 
```

向云端服务发出请求。

```golang
func (s *DeviceSession) Get(uri uint32, Req []byte, timeout time.Duration) ([]byte, error)
func (s *DeviceSession) Post(uri uint32, Req []byte, timeout time.Duration) ([]byte, error)
```
