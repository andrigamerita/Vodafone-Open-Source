#
# $Id: net.japan.sh,v 1.1.1.1 2003/02/19 11:46:31 sergey Exp $
#
if [ -n "$UML_private_CTL" ]
then
    net_eth0="eth0=daemon,10:00:00:ab:cd:02,unix,$UML_private_CTL,$UML_private_DATA";
else
    net_eth0="eth0=mcast,10:00:00:ab:cd:02,239.192.0.2,40800"
fi

net="$net_eth0"

