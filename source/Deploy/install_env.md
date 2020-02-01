# centos8 vmware

## boost
    tar zxvf boost_1_71_0.tar.gz 
    ./bootstrap.sh --prefix=/home/wenhe/Study/libraries/boost171
    ./b2 install

## zeroc-ice  
    sudo dnf install https://zeroc.com/download/ice/3.7/el8/ice-repo-3.7.el8.noarch.rpm
    sudo dnf install ice-all-runtime ice-all-devel







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
     



 

     