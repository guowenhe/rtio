<!-- TOC -->

- [环境部署](#环境部署)
  - [依赖环境及工具安装](#依赖环境及工具安装)
    - [编译等工具安装](#编译等工具安装)
    - [boost安装](#boost安装)
    - [zeroc-ice安装](#zeroc-ice安装)
    - [redis安装](#redis安装)
    - [nginx安装](#nginx安装)
  - [编译rtio服务](#编译rtio服务)
  - [服务启动](#服务启动)
    - [启动redis服务](#启动redis服务)
    - [启动nginx服务](#启动nginx服务)
    - [启动rtio服务](#启动rtio服务)

<!-- /TOC -->

# 环境部署
以下介绍centos8上如何部署rtio服务。注意未在纯净centos8上做安装测试，步骤可能有疏漏，后续验证。

## 依赖环境及工具安装

### 编译等工具安装

```
sudo dnf install git   
sudo dnf install gcc gdb gcc-c++  
sudo dnf install make cmake 
```
gcc version在8.3.1以上，cmake 2.8以上版本。
### boost安装
```
tar zxvf boost_1_71_0.tar.gz 
./bootstrap.sh --prefix=/home/wenhe/Study/libraries/boost171
./b2 install
```

### zeroc-ice安装
```
sudo dnf install https://zeroc.com/download/ice/3.7/el8/ice-repo-3.7.el8.noarch.rpm
sudo dnf install ice-all-runtime ice-all-devel
```

### redis安装 
```
tar zxvf redis-5.0.7.tar.gz
cd redis-5.0.7/
make PREFIX=/dir/to/install/ install
```
### nginx安装 
需要tcp代理和ssl相关功能，安装时需要引入相关模块。
```
sudo dnf install pcre-devel
./configure --with-stream  --with-stream_ssl_module --with-http_ssl_module --prefix=/dir/to/install/
make
make install
```

## 编译rtio服务
如果以下组件，没有安装在系统默认目录，需要设定以下环境变量，指向安装目录。
```
export set BOOST_HOME=/dir/to/boost
export set HI_REDIS=/dir/to/hiredis
```
编译rtio源码，并安装到预发布目录。
```
git clone https://github.com/guowenhe/rtio.git
cd rtio/
mkdir build && cd build/
cmake ../
make            # 可并行编译，比如4个job带参数“-j4”即可
make install    # 安装到预发布目录
```
安装成功，显示类似信息如下：
```
Install the project...
-- Install configuration: "DEBUG"
-- Installing: /home/wenhe/Work/test/rtio/source/Deploy/patches/APINotifier/server
-- Installing: /home/wenhe/Work/test/rtio/source/Deploy/independent/AccessServerTCP/server
-- Installing: /home/wenhe/Work/test/rtio/source/Deploy/independent/AccessServerTCP/client_async
-- Installing: /home/wenhe/Work/test/rtio/source/Deploy/independent/APISender/server
-- Installing: /home/wenhe/Work/test/rtio/source/Deploy/independent/APIManager/server
-- Installing: /home/wenhe/Work/test/rtio/source/Deploy/patches/DeviceHub/server
-- Installing: /home/wenhe/Work/test/rtio/source/Deploy/independent/DeviceHub/client_sync_b
-- Installing: /home/wenhe/Work/test/rtio/source/Deploy/patches/StatusServer/server
-- Installing: /home/wenhe/Work/test/rtio/source/Deploy/patches/MessageReporter/server
-- Installing: /home/wenhe/Work/test/rtio/source/Deploy/independent/MessageReporter/client_sync
-- Installing: /home/wenhe/Work/test/rtio/source/Deploy/patches/DeviceManager/server
-- Installing: /home/wenhe/Work/test/rtio/source/Deploy/independent/DeviceManager/client_sync

```

## 服务启动

### 启动redis服务
切换到redis安装目录下，启动好redis。
```
./redis-server &
```

### 启动nginx服务

切换到nginx安装目录，修改配置。

```
    server {
        listen     12012 ssl;
        ssl_protocols        TLSv1 TLSv1.1 TLSv1.2;
        ssl_ciphers          AES128-SHA:AES256-SHA:RC4-SHA:DES-CBC3-SHA:RC4-MD5;
        ssl_certificate      cert2/server.crt;
        ssl_certificate_key  cert2/server-prikey.key; 
        ssl_session_cache    shared:SSL:1m;
        ssl_session_timeout  5m;

        proxy_pass 127.0.0.1:8080;
    }
```
启动nginx。

```
./sbin/nginx
```



### 启动rtio服务
修改rtio配置“source/Deploy/config/DMS.xml”，指定“clusterpath”，即每个node相应的部署根目录。

```
<node name="node1">
    <variable name="clusterpath" value="/path/to/cluster/"/>
    <server-instance template="APINotifier"/>
    <server-instance template="DeviceHub"/>
    <server-instance template="DeviceManager"/>
    <server-instance template="MessageReporter"/>
    <server-instance template="MessageTrigger"/>
    <server-instance template="StatusServer"/>
</node>
```

给ice register 增加应用配置。
```
cd ../source/Deploy/ #切换到Deploy目录
icegridadmin --Ice.Config=../../cluster/config.client -e "application add ./config/Facilities.xml"
icegridadmin --Ice.Config=../../cluster/config.client -e "application add ./config/DMS.xml"
```
部署执行程序。
```
./prepatch.sh 
```
输出如下：
```
checksum: .gitignore
compress: APINotifier/server
checksum: APINotifier/server
compress: DeviceHub/server
checksum: DeviceHub/server
compress: DeviceManager/server
checksum: DeviceManager/server
compress: MessageReporter/server
checksum: MessageReporter/server
compress: StatusServer/server
checksum: StatusServer/server
##################################################
patching ...
##################################################
copy independent servers
```
运行非ice管理的程序（后续考虑自动启动）：
```
cd ../../cluster/independent/
cd AccessServerTCP
./server access-server-tcp.config &
cd ../APIManager/
./server api-manager.config &
cd ../APISender/
/server api-sender.config &
```
在“cluster/logs”中查看各个服务日志，有“server started”字样表示该服务已启动。


