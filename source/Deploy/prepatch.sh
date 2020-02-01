
#rm -rf patches
#mkdir patches/ 



icegridadmin -u admin -p admin01 --Ice.Default.Locator="VerijsGrid/Locator:default -h localhost -p 12000" -e "server stop Facilities.IcePatch2"
icepatch2calc -V ./patches
echo "##################################################"
# echo "press ENTER key to patch ..."
# read
icegridadmin -u admin -p admin01 --Ice.Default.Locator="VerijsGrid/Locator:default -h localhost -p 12000" -e "application patch -f DMS"

#icegridadmin -u admin -p admin01 --Ice.Default.Locator="PayServerGrid/Locator:default -h localhost -p 4062" -e "server patch -f PayServer.Cashier-1"
echo "patching ..."

echo "##################################################"
echo "copy independent servers"
cp -rf independent/AccessServer 		../../cluster/independent/
cp -f config/access-server.config 	../../cluster/independent/AccessServer/

cp -rf independent/APIPusher 		../../cluster/independent/
cp -f config/api-pusher.config 	../../cluster/independent/APIPusher/