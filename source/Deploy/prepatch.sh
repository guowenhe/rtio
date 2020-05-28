
#rm -rf patches
#mkdir patches/ 



icegridadmin -u admin -p admin01 --Ice.Default.Locator="Grid-001/Locator:default -h localhost -p 12000" -e "server stop Facilities.IcePatch2"
icepatch2calc -V ./patches
echo "##################################################"
# echo "press ENTER key to patch ..."
# read
icegridadmin -u admin -p admin01 --Ice.Default.Locator="Grid-001/Locator:default -h localhost -p 12000" -e "application patch -f DMS"

#icegridadmin -u admin -p admin01 --Ice.Default.Locator="PayServerGrid/Locator:default -h localhost -p 4062" -e "server patch -f PayServer.Cashier-1"
echo "patching ..."

echo "##################################################"
echo "copy independent servers"
cp -rf independent/AccessServerTCP 		../../cluster/independent/
cp -f config/access-server-tcp.config 	../../cluster/independent/AccessServerTCP/

cp -rf independent/APISender 		../../cluster/independent/
cp -f config/api-sender.config 	../../cluster/independent/APISender/

cp -rf independent/APIManager 		../../cluster/independent/
cp -f config/api-manager.config 	../../cluster/independent/APIManager/