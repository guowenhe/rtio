# RTIO压测

- [RTIO压测](#rtio压测)
  - [环境](#环境)
    - [服务端环境](#服务端环境)
    - [客户端环境](#客户端环境)
    - [系统设置](#系统设置)
  - [压测过程](#压测过程)
  - [压测总结](#压测总结)
  - [附录](#附录)
    - [60万连接测试](#60万连接测试)
    - [84万连接测试](#84万连接测试)

## 环境

### 服务端环境

运行RTIO服务端程序，1台主机配置如下：

- CPU：8C16T （8核16线程）
- RAM：48GB （DDR3, 服务端实际用量小于20GB）
- SSD：128GB
- OS：Ubuntu 20.04.5 LTS

### 客户端环境

用于模拟设备端，3台虚拟机配置如下：

- CPU：8C （8虚拟核心）
- RAM：16G
- OS：Debian 5.10.179-3 (2023-07-27)
- VirtualIP： 7 （7个虚拟IP）
- HDD：20GB

### 系统设置

服务端和所有客户端机器，调整系统参数如下：

```sh
sudo sysctl -w fs.file-max="9999999"
sudo sysctl -w fs.nr_open="9999999"
sudo sysctl -w net.core.netdev_max_backlog="4096"
sudo sysctl -w net.core.rmem_max="16777216"
sudo sysctl -w net.core.somaxconn="65535"
sudo sysctl -w net.core.wmem_max="16777216"
sudo sysctl -w net.ipv4.ip_local_port_range="1025       65535"
sudo sysctl -w net.ipv4.tcp_fin_timeout="30"
sudo sysctl -w net.ipv4.tcp_max_syn_backlog="20480"
sudo sysctl -w net.ipv4.tcp_max_tw_buckets="400000"
sudo sysctl -w net.ipv4.tcp_no_metrics_save="1"
sudo sysctl -w net.ipv4.tcp_syn_retries="2"
sudo sysctl -w net.ipv4.tcp_synack_retries="2"
sudo sysctl -w net.ipv4.tcp_tw_reuse="1"
sudo sysctl -w net.nf_conntrack_max="9999999" # 默认debian没有该选项，注释掉即可
sudo sysctl -w vm.min_free_kbytes="65536"
sudo sysctl -w vm.overcommit_memory="1"
ulimit -n 9999999
```

环境变量设置如下：

```sh
export RTIO_LOG_LEVEL=error
export RTIO_LOG_JSON=true
```

> 2023年10月26日 备注：rtio不再使用环境变量设置日志级别，可通过`./rtio -h` 查看帮助。
  
## 压测过程

编译RTIO代码后，生成测试工具如下。

```sh
cd rtio
$ tree out/
.
├── deviceaccess
│   ├── access_devicegen
│   └── access_tcp_stress
├── rtio
└── useraccess
    └── access_user_stress
```

客户端机器生成所需要的设备文件，如下每个文件60000个设备。

``` sh
cd rtio
./access_devicegen -file 161.devices -num 60000
ls
161.devices 
```

服务端机器运行服务程序：

```sh
nohup ./rtio &
```

> 2023年10月26日 备注：后续版本使用`./rtio -log.format=json  -log.level=error`运行RTIO设置正确日志等级（未进行压测验证）。

客户端机器分批运行压测程序，每台机器启动多个进程并绑定虚拟IP：

```sh
nohup ./access_tcp_stress -local 192.168.50.161 -server 192.168.50.222:17017 -file 161.devices &
nohup ./access_tcp_stress -local 192.168.50.162 -server 192.168.50.222:17017 -file 162.devices &
# ...
nohup ./access_tcp_stress -local 192.168.50.167 -server 192.168.50.222:17017 -file 167.devices &

```

另两台客户端机器：

```sh
nohup ./access_tcp_stress -local 192.168.50.171 -server 192.168.50.222:17017 -file 171.devices &
nohup ./access_tcp_stress -local 192.168.50.172 -server 192.168.50.222:17017 -file 172.devices &
# ...
nohup ./access_tcp_stress -local 192.168.50.175 -server 192.168.50.222:17017 -file 175.devices &
```

```sh
nohup ./access_tcp_stress -local 192.168.50.181 -server 192.168.50.222:17017 -file 181.devices &
nohup ./access_tcp_stress -local 192.168.50.182 -server 192.168.50.222:17017 -file 182.devices &
# ...
nohup ./access_tcp_stress -local 192.168.50.185 -server 192.168.50.222:17017 -file 185.devices &
```

客户端逐渐增加到17个进程，共102万连接。服务端输出日志如下(日志级别调成ERROR，程序通过ERROR打印出连接信息)。

```json
{"level":"error","sessionnum":0,"caller":"devicetcp.go:92","time":1692947736411}
{"level":"error","sessionnum":106,"caller":"devicetcp.go:92","time":1692947746402}
{"level":"error","sessionnum":16886,"caller":"devicetcp.go:92","time":1692947756402}
{"level":"error","sessionnum":33722,"caller":"devicetcp.go:92","time":1692947766402}
{"level":"error","sessionnum":49921,"caller":"devicetcp.go:92","time":1692947776402}
{"level":"error","sessionnum":60000,"caller":"devicetcp.go:92","time":1692947786403}
{"level":"error","sessionnum":60000,"caller":"devicetcp.go:92","time":1692947796402}   
...
{"level":"error","sessionnum":993370,"caller":"devicetcp.go:92","time":1692948759552}
{"level":"error","sessionnum":997351,"caller":"devicetcp.go:92","time":1692948766402}
{"level":"error","sessionnum":1004053,"caller":"devicetcp.go:92","time":1692948776402}
{"level":"error","sessionnum":1011125,"caller":"devicetcp.go:92","time":1692948786402}
{"level":"error","sessionnum":1017758,"caller":"devicetcp.go:92","time":1692948796402}
{"level":"error","sessionnum":1019999,"caller":"devicetcp.go:92","time":1692948810523}
{"level":"error","sessionnum":1019999,"caller":"devicetcp.go:92","time":1692948817287}
{"level":"error","sessionnum":1019999,"caller":"devicetcp.go:92","time":1692948826402}
{"level":"error","sessionnum":1019999,"caller":"devicetcp.go:92","time":1692948836402}
{"level":"error","sessionnum":1019999,"caller":"devicetcp.go:92","time":1692948846407}
{"level":"error","sessionnum":1019999,"caller":"devicetcp.go:92","time":1692948861275}
{"level":"error","sessionnum":1020000,"caller":"devicetcp.go:92","time":1692948866402}
{"level":"error","sessionnum":1020000,"caller":"devicetcp.go:92","time":1692948876403}
{"level":"error","sessionnum":1020000,"caller":"devicetcp.go:92","time":1692948886402}
{"level":"error","sessionnum":1020000,"caller":"devicetcp.go:92","time":1692948897318}
{"level":"error","sessionnum":1020000,"caller":"devicetcp.go:92","time":1692948906425}
{"level":"error","sessionnum":1020000,"caller":"devicetcp.go:92","time":1692948917672}
{"level":"error","sessionnum":1020000,"caller":"devicetcp.go:92","time":1692948926402}
{"level":"error","sessionnum":1020000,"caller":"devicetcp.go:92","time":1692948936402}
{"level":"error","sessionnum":1020000,"caller":"devicetcp.go:92","time":1692948946402}
{"level":"error","sessionnum":1020000,"caller":"devicetcp.go:92","time":1692948956402}
{"level":"error","sessionnum":1020000,"caller":"devicetcp.go:92","time":1692948968720}
...
{"level":"error","sessionnum":1020000,"caller":"devicetcp.go:92","time":1692952176402}
{"level":"error","sessionnum":1020000,"caller":"devicetcp.go:92","time":1692952186402}
```

客户端运行用户端压测程序，对设备逐个调用（GET到设备并返回数据），验证连接的有效性。

```sh
./access_user_stress -server 192.168.50.222:17317 -file 175.devices
```

用户调用175.devices关键日志如下：

```json
{"level":"info","num":60000,"caller":"access_user_stress.go:88","time":1692949861802,"message":"load devices"}
{"level":"error","devicenum":60000,"success":59990,"timeout":10,"caller":"access_user_stress.go:139","time":1692950419758,"message":"result"}
```

用户端超时设定为5秒，分析如下。

- 设备端保持102万连接
- 用户端调用设备5秒内返回响应的连接数占比：59990/60000 = 99.98%
- 用户端调用设备端平均响应时间：(((1692950419758-1692949861802)/1000)-10*5)/59970 = 0.0085秒
- 每个客户端心跳为60秒，整个测试持续时常(1692952186402-1692948866402)/1000 = 3320秒 = 55分钟

## 压测总结

以下为102万设备连接时的各项指标：

- RTIO单机可承受102万连接，持续测试55分钟，中间有个别设备掉线（小于5个）重连成功，稳定维持在102万连接
- 用户端调用设备5秒内返回响应的连接数占比99.98%
- 用户端调用设备端平均响应时间0.0085秒
- 服务端CPU通常4-5个，偶尔会达到15-16个（8核心16虚拟CPU，这里指的虚拟CPU个数）
- 服务端内存用量18-19GB
  
服务端连接速度可达到6000连接每秒（随连接数量上升，连接速度会下降，这里未作详细测试）。另外，连接数降低增加系统稳定性，比如84万连接，5秒内返回响应的连接数占比接近100%，响应时间也会快一倍，参考附录。

## 附录

### 60万连接测试

客户端逐渐增加到10个进程，共60万连接。服务端输出日志如下。

```json
{"level":"error","sessionnum":0,"caller":"devicetcp.go:92","time":1692963904159}
{"level":"error","sessionnum":14696,"caller":"devicetcp.go:92","time":1692963914150}
{"level":"error","sessionnum":65806,"caller":"devicetcp.go:92","time":1692963924150}
{"level":"error","sessionnum":126520,"caller":"devicetcp.go:92","time":1692963934150}
{"level":"error","sessionnum":196896,"caller":"devicetcp.go:92","time":1692963944150}
{"level":"error","sessionnum":249527,"caller":"devicetcp.go:92","time":1692963954150}
{"level":"error","sessionnum":298010,"caller":"devicetcp.go:92","time":1692963964150}
{"level":"error","sessionnum":346448,"caller":"devicetcp.go:92","time":1692963974150}
{"level":"error","sessionnum":389044,"caller":"devicetcp.go:92","time":1692963984150}
{"level":"error","sessionnum":432448,"caller":"devicetcp.go:92","time":1692963994150}
{"level":"error","sessionnum":472721,"caller":"devicetcp.go:92","time":1692964004150}
{"level":"error","sessionnum":506892,"caller":"devicetcp.go:92","time":1692964014153}
{"level":"error","sessionnum":538486,"caller":"devicetcp.go:92","time":1692964024684}
{"level":"error","sessionnum":568146,"caller":"devicetcp.go:92","time":1692964034150}
{"level":"error","sessionnum":581551,"caller":"devicetcp.go:92","time":1692964044150}
{"level":"error","sessionnum":595624,"caller":"devicetcp.go:92","time":1692964054150}
{"level":"error","sessionnum":600000,"caller":"devicetcp.go:92","time":1692964064150}
{"level":"error","sessionnum":600000,"caller":"devicetcp.go:92","time":1692964074150}
```

用户调用181.devices关键日志如下：

```json
{"level":"info","num":60000,"caller":"access_user_stress.go:88","time":1692964243348,"message":"load devices"}
{"level":"error","devicenum":60000,"success":60000,"timeout":0,"caller":"access_user_stress.go:139","time":1692964390077,"message":"result"}
```

用户端超时设定为5秒，分析如下。

- 设备端保持60万连接
- 用户端调用设备5秒内返回响应的连接数占比：100%
- 用户端调用设备端平均响应时间：((1692964390077-1692964243348)/1000)/60000 = 0.0024秒
- CPU稳定2-3个
- RAM占用11-12GB

### 84万连接测试

客户端逐渐增加到14个进程，共84万连接。服务端输出日志如下。

```json
{"level":"error","sessionnum":780000,"caller":"devicetcp.go:92","time":1692966004150}
{"level":"error","sessionnum":790564,"caller":"devicetcp.go:92","time":1692966014150}
{"level":"error","sessionnum":802392,"caller":"devicetcp.go:92","time":1692966024150}
{"level":"error","sessionnum":814546,"caller":"devicetcp.go:92","time":1692966034150}
{"level":"error","sessionnum":825021,"caller":"devicetcp.go:92","time":1692966044151}
{"level":"error","sessionnum":836961,"caller":"devicetcp.go:92","time":1692966054150}
{"level":"error","sessionnum":840000,"caller":"devicetcp.go:92","time":1692966064150}
{"level":"error","sessionnum":840000,"caller":"devicetcp.go:92","time":1692966074150}
```

用户调用182.devices关键日志如下：

```json
{"level":"info","num":60000,"caller":"access_user_stress.go:88","time":1692966310145,"message":"load devices"}
{"level":"error","devicenum":60000,"success":59999,"timeout":1,"caller":"access_user_stress.go:139","time":1692966552905,"message":"result"}
```

用户端超时设定为5秒，分析如下。

- 设备端保持84万连接
- 用户端调用设备5秒内返回响应的连接数占比：59999/60000 = 99.998%
- 用户端调用设备端平均响应时间：((1692966552905-1692966310145)/1000)/60000 = 0.0040秒
- CPU稳定3-4个，偶尔会到12个
- RAM占用15-16GB
