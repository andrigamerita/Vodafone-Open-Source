#ifndef NETLINK_H
#define NETLINK_H
/*=========================================================================*\
* Netlink object
* LuaSocket toolkit
*
* The netlink.h module provides LuaSocket with support for Netlink protocol
* (AF_INET, SOCK_DGRAM).
*
* Two classes are defined: connected and unconnected. Netlink objects are
* originally unconnected. They can be "connected" to a given address 
* with a call to the setpeername function. The same function can be used to
* break the connection.
*
* RCS ID: $Id
\*=========================================================================*/
#include "lua.h"

#include "timeout.h"
#include "socket.h"

/* can't be larger than wsocket.c MAXCHUNK!!! */
#define NETLINK_DATAGRAMSIZE 8192

typedef struct t_netlink_ {
    t_socket sock;
    t_timeout tm;
} t_netlink;
typedef t_netlink *p_netlink;

int netlink_open(lua_State *L);

#endif /* NETLINK_H */
