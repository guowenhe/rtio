
if [ "$1" == "start" ]; then
    icegridregistry --Ice.Config=config.master &
    icegridregistry --Ice.Config=config.replica1 &
    icegridregistry --Ice.Config=config.replica2 &
    icegridnode --Ice.Config=config.node1 &
    icegridnode --Ice.Config=config.node2 &
fi

if [ "$1" == "stop" ]; then
    killall icegridregistry icegridnode
fi

sleep 0.5
echo processes:
ps -ux|grep icegridregistry|grep -v grep
ps -ux|grep icegridnode|grep -v grep
