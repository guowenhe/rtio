# centos8
    dnf update
    sudo dnf install java-1.8.0-openjdk-1:1.8.0.232.b09-2.el8_1.x86_64  

    sudo dnf install git   
    sudo dnf install gcc gdb gcc-c++  
    sudo dnf install make cmake 
    
## install vmware on centos8
    sudo dnf install epel-release
    sudo dnf install kernel-devel-`uname -r` kernel-headers-`uname -r`
    sudo ln -s /usr/include/linux/version.h /lib/modules/4.18.0-147.3.1.el8_1.x86_64/build/include/linux/version.h
    sudo dnf install elfutils-libelf-devel
    
## ntfs
	wget https://tuxera.com/opensource/ntfs-3g_ntfsprogs-2017.3.23.tgz
	./configure 
	make -j2
	sudo make install
	sudo ntfs-3g /dev/sdc1 ~/Backup/
	
	
## chrome
	sudo dnf install epel-release
	sudo dnf install libXScrnSaver*
	sudo dnf install libappindicator-gtk3
	sudo dnf install liberation-fonts
	sudo rpm -ivh google-chrome-stable_current_x86_64.rpm
## openoffice
	sudo rpm -ivh openoffice-*
	sudo rpm -ivh openoffice4.1.7-redhat-menus-4.1.7-9800.noarch.rpm
	sudo  ln -s libgdk_pixbuf-2.0.so.0 libgdk_pixbuf_xlib-2.0.so.0

## boost
    tar zxvf boost_1_71_0.tar.gz 
    ./bootstrap.sh --prefix=/home/wenhe/Study/libraries/boost171
    ./b2 install

## zeroc-ice  
    sudo dnf install https://zeroc.com/download/ice/3.7/el8/ice-repo-3.7.el8.noarch.rpm
    sudo dnf install ice-all-runtime ice-all-devel

## coredump
	lz4 -d /var/lib/systemd/coredump/core.APISender-serve.1000.282febd7c891469c8ecbf598615297ca.6153.1581841990000000.lz4 ./core.api





# centos7

## redis server 
    tar zxvf redis-5.0.7.tar.gz
    cd redis-5.0.7/
    make PREFIX=/home/wenhe/Study/redis/redis507 install
  
## redis client

    git clone https://github.com/redis/hiredis.git
    make PREFIX=/home/wenhe/Study/redis/redis507 install
    
## mysql

     wget https://dev.mysql.com/get/mysql80-community-release-el7-1.noarch.rpm
     sudo rpm -ivh mysql80-community-release-el7-1.noarch.rpm
     sudo yum install mysql-community-server.x86_64
     sudo yum install mysql-connector-c++ mysql-connector-c++-devel.x86_64
     sudo yum install mysql-workbench-community.x86_64
     



 

     