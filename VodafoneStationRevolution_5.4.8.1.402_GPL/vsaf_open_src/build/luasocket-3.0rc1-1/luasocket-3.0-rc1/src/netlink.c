/*=========================================================================*\
* Netlink object 
* LuaSocket toolkit
*
* RCS ID: $Id
\*=========================================================================*/
#include <string.h> 
#include <bits/sockaddr.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "lua.h"
#include "lauxlib.h"

#include "auxiliar.h"
#include "socket.h"
#include "inet.h"
#include "options.h"
#include "netlink.h"

/* min and max macros */
#ifndef MIN
#define MIN(x, y) ((x) < (y) ? x : y)
#endif 
#ifndef MAX
#define MAX(x, y) ((x) > (y) ? x : y)
#endif 

/*=========================================================================*\
* Internal function prototypes
\*=========================================================================*/
static int global_create(lua_State *L);
static int meth_bind(lua_State *L);
static int meth_receive(lua_State *L);
static int meth_close(lua_State *L);
static int meth_settimeout(lua_State *L);
static int meth_getfd(lua_State *L);
static int meth_setfd(lua_State *L);
static int meth_dirty(lua_State *L);

/* netlink object methods */
static luaL_Reg netlink[] = {
    {"__gc",        meth_close},
    {"__tostring",  auxiliar_tostring},
    {"bind",        meth_bind},
    {"close",       meth_close},
    {"dirty",       meth_dirty},
    {"getfd",       meth_getfd},
    {"receive",     meth_receive},
    {"setfd",       meth_setfd},
    {"settimeout",  meth_settimeout},
    {NULL,          NULL}
};

/* functions in library namespace */
static luaL_Reg func[] = {
    {"netlink", global_create},
    {NULL, NULL}
};

typedef struct {
    char *name;
    int value;
} lua_consts_t;

#define CONST_VAL(val) { #val, val }
lua_consts_t netlink_consts[] =
{
    /* Netlink families */
    CONST_VAL(NETLINK_ROUTE),
    CONST_VAL(NETLINK_USERSOCK),
    CONST_VAL(NETLINK_FIREWALL),
    CONST_VAL(NETLINK_INET_DIAG),
    CONST_VAL(NETLINK_NFLOG),
    CONST_VAL(NETLINK_XFRM),
    CONST_VAL(NETLINK_SELINUX),
    CONST_VAL(NETLINK_ISCSI),
    CONST_VAL(NETLINK_AUDIT),
    CONST_VAL(NETLINK_FIB_LOOKUP),
    CONST_VAL(NETLINK_CONNECTOR),
    CONST_VAL(NETLINK_NETFILTER),
    CONST_VAL(NETLINK_IP6_FW),
    CONST_VAL(NETLINK_DNRTMSG),
    CONST_VAL(NETLINK_KOBJECT_UEVENT),
    CONST_VAL(NETLINK_GENERIC),
    /* Netlink route groups */
    CONST_VAL(RTMGRP_IPV4_IFADDR),
    CONST_VAL(RTMGRP_IPV4_MROUTE),
    CONST_VAL(RTMGRP_IPV4_ROUTE),
    CONST_VAL(RTMGRP_IPV4_RULE),
    CONST_VAL(RTMGRP_IPV6_IFADDR),
    CONST_VAL(RTMGRP_IPV6_MROUTE),
    CONST_VAL(RTMGRP_IPV6_ROUTE),
    CONST_VAL(RTMGRP_IPV6_IFINFO),
    /* Netlink route message types */
    CONST_VAL(RTM_NEWLINK),
    CONST_VAL(RTM_DELLINK),
    CONST_VAL(RTM_GETLINK),
    CONST_VAL(RTM_NEWADDR),
    CONST_VAL(RTM_DELADDR),
    CONST_VAL(RTM_GETADDR),
    CONST_VAL(RTM_NEWROUTE),
    CONST_VAL(RTM_DELROUTE),
    CONST_VAL(RTM_GETROUTE),
    CONST_VAL(RTM_NEWNEIGH),
    CONST_VAL(RTM_DELNEIGH),
    CONST_VAL(RTM_GETNEIGH),
    CONST_VAL(RTM_NEWRULE),
    CONST_VAL(RTM_DELRULE),
    CONST_VAL(RTM_GETRULE),
    CONST_VAL(RTM_NEWQDISC),
    CONST_VAL(RTM_DELQDISC),
    CONST_VAL(RTM_GETQDISC),
    CONST_VAL(RTM_NEWTCLASS),
    CONST_VAL(RTM_DELTCLASS),
    CONST_VAL(RTM_GETTCLASS),
    CONST_VAL(RTM_NEWTFILTER),
    CONST_VAL(RTM_DELTFILTER),
    CONST_VAL(RTM_GETTFILTER),
    { NULL, -1 }
};

/*-------------------------------------------------------------------------*\
* Initializes module
\*-------------------------------------------------------------------------*/
int netlink_open(lua_State *L)
{
    int i;
    /* create classes */
    auxiliar_newclass(L, "netlink{connected}", netlink);
    auxiliar_newclass(L, "netlink{unconnected}", netlink);
    /* create class groups */
    auxiliar_add2group(L, "netlink{connected}",   "netlink{any}");
    auxiliar_add2group(L, "netlink{unconnected}", "netlink{any}");
    /* define constants */
    for (i = 0; netlink_consts[i].name; i++)
    {
	lua_pushstring(L, netlink_consts[i].name);
	lua_pushnumber(L, netlink_consts[i].value);
	lua_rawset(L,-3);
    }
    /* define library functions */
    luaL_openlib(L, NULL, func, 0); 
    return 0;
}

/*=========================================================================*\
* Lua methods
\*=========================================================================*/
const char *netlink_strerror(int err) {
    /* a 'closed' error on an unconnected means the target address was not
     * accepted by the transport layer */
    if (err == IO_CLOSED) return "refused";
    else return socket_strerror(err);
}

/*-------------------------------------------------------------------------*\
* Binds an object to an address 
\*-------------------------------------------------------------------------*/
static int meth_bind(lua_State *L)
{
    p_netlink netlink = (p_netlink) auxiliar_checkclass(L,
        "netlink{unconnected}", 1);
    unsigned long groups = (unsigned long)luaL_optnumber(L, 2, -1);
    struct sockaddr_nl nls = {
        .nl_family = AF_NETLINK,
        .nl_pid = getpid(),
        .nl_groups = groups,
    };
    int err = socket_bind(&netlink->sock, (void *)&nls,
        sizeof(struct sockaddr_nl));

    if (err != IO_DONE) {
        socket_destroy(&netlink->sock);
        lua_pushnil(L);
        lua_pushstring(L, socket_strerror(err));
        return 2;
    }
    auxiliar_setclass(L, "netlink{connected}", 1);
    lua_pushnumber(L, 1);
    return 1;
}

/*-------------------------------------------------------------------------*\
* Receives data from a netlink socket
\*-------------------------------------------------------------------------*/
static int meth_receive(lua_State *L) {
    p_netlink netlink = (p_netlink) auxiliar_checkclass(L, "netlink{connected}", 1);
    char buffer[NETLINK_DATAGRAMSIZE];
    size_t got, count = (size_t) luaL_optnumber(L, 2, sizeof(buffer));
    int err, i;
    p_timeout tm = &netlink->tm;
    count = MIN(count, sizeof(buffer));
    timeout_markstart(tm);
    err = socket_recv(&netlink->sock, buffer, count, &got, tm);
    if (err != IO_DONE) {
        lua_pushnil(L);
        lua_pushstring(L, netlink_strerror(err));
        return 2;
    }
    for (i = 0; i < got - 1; i++)
        buffer[i] = buffer[i] ? : '\n';
    lua_pushlstring(L, buffer, got);
    return 1;
}

/*-------------------------------------------------------------------------*\
* Select support methods
\*-------------------------------------------------------------------------*/
static int meth_getfd(lua_State *L) {
    p_netlink netlink = (p_netlink) auxiliar_checkgroup(L, "netlink{any}", 1);
    lua_pushnumber(L, (int) netlink->sock);
    return 1;
}

/* this is very dangerous, but can be handy for those that are brave enough */
static int meth_setfd(lua_State *L) {
    p_netlink netlink = (p_netlink) auxiliar_checkgroup(L, "netlink{any}", 1);
    netlink->sock = (t_socket) luaL_checknumber(L, 2);
    return 0;
}

static int meth_dirty(lua_State *L) {
    p_netlink netlink = (p_netlink) auxiliar_checkgroup(L, "netlink{any}", 1);
    (void) netlink;
    lua_pushboolean(L, 0);
    return 1;
}

/*-------------------------------------------------------------------------*\
* Just call tm methods
\*-------------------------------------------------------------------------*/
static int meth_settimeout(lua_State *L) {
    p_netlink netlink = (p_netlink) auxiliar_checkgroup(L, "netlink{any}", 1);
    return timeout_meth_settimeout(L, &netlink->tm);
}

/*-------------------------------------------------------------------------*\
* Closes socket used by object 
\*-------------------------------------------------------------------------*/
static int meth_close(lua_State *L) {
    p_netlink netlink = (p_netlink) auxiliar_checkgroup(L, "netlink{any}", 1);
    socket_destroy(&netlink->sock);
    lua_pushnumber(L, 1);
    return 1;
}

/*=========================================================================*\
* Library functions
\*=========================================================================*/
/*-------------------------------------------------------------------------*\
* Creates a master netlink object 
\*-------------------------------------------------------------------------*/
static int global_create(lua_State *L) {
    t_socket sock;
    int nl_family = luaL_checknumber(L, 1);
    const char *err = socket_strerror(socket_create(&sock, PF_NETLINK, SOCK_DGRAM,
        nl_family));
    /* try to allocate a system socket */
    if (!err) { 
        /* allocate tcp object */
        p_netlink netlink = (p_netlink) lua_newuserdata(L, sizeof(t_netlink));
        auxiliar_setclass(L, "netlink{unconnected}", -1);
        /* initialize remaining structure fields */
        socket_setnonblocking(&sock);
        netlink->sock = sock;
        timeout_init(&netlink->tm, -1, -1);
        return 1;
    } else {
        lua_pushnil(L);
        lua_pushstring(L, err);
        return 2;
    }
}
