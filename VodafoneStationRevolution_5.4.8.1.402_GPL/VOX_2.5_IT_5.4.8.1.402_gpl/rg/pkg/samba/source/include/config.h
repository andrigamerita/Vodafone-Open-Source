/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* XXX Undocumented */
#define REPLACE_GETPASS 1
#define WITH_SMBPASSWD_SAM 1
#define HAVE_KERNEL_SHARE_MODES 1
#define SEEKDIR_RETURNS_VOID 1
#define BROKEN_NISPLUS_INCLUDE_FILES 1
#define HAVE_EXPLICIT_LARGEFILE_SUPPORT 1
#define SYSCONF_SC_NGROUPS_MAX 1
#define PUTUTLINE_RETURNS_UTMP 1
#define COMPILER_SUPPORTS_LL 1
#define HAVE_IMMEDIATE_STRUCTURES 1
#define WITH_SYSLOG 1
#define HAVE_STRUCT_TIMESPEC 1
#define HAVE_SETEUID 1
#define HAVE_SETEGID 1
#define HAVE_STRTOLL 1
#define HAVE_STRTOULL 1
#define HAVE_FNMATCH_H 1
#define HAVE_COMPARISON_FN_T 1
#define HAVE_MKDIR_MODE 1

#ifdef CONFIG_RG_FILESERVER_ACLS
#define HAVE_ACL_LIBACL_H 1
#endif

#undef USE_SETEUID
#undef USE_SETRESUID
#undef USE_SETUIDX
#define USE_SETREUID 1

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE 1
#endif

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <libc_config.h>

#ifdef CONFIG_RG_FILESERVER_ACLS
#define HAVE_POSIX_ACLS 1
#define HAVE_SYS_ACL_H 1
#else
#define HAVE_NO_ACLS 1
#endif

/* XXX Buggy code in lib/fsusage.c
 * in GLIBC both STAT_STATFS2_BSIZE and STAT_STATVFS, STAT_STATVFS64 are
 * available 
 */
#if defined(STAT_STATFS2_BSIZE) && \
    (defined(STAT_STATVFS) || defined(STAT_STATVFS64))
#undef STAT_STATFS2_BSIZE
#endif

/* Default display charset name */
#define DEFAULT_DISPLAY_CHARSET "UTF8"

/* Default dos charset name */
#define DEFAULT_DOS_CHARSET "CP850"

/* Default unix charset name */
#define DEFAULT_UNIX_CHARSET "UTF8"

/* String list of builtin modules */
#define STRING_STATIC_MODULES ""

/* The size of a block */
#define STAT_ST_BLOCKSIZE 512

/* Decl of Static init functions */
#define static_decl_auth extern NTSTATUS auth_sam_init(void); extern NTSTATUS auth_unix_init(void); extern NTSTATUS auth_winbind_init(void); extern NTSTATUS auth_server_init(void); extern NTSTATUS auth_domain_init(void); extern NTSTATUS auth_builtin_init(void);

/* Decl of Static init functions */
#define static_decl_charset  extern NTSTATUS charset_CP850_init(void); extern NTSTATUS charset_CP437_init(void);

/* Decl of Static init functions */
#define static_decl_idmap extern NTSTATUS idmap_tdb_init(void); extern NTSTATUS idmap_passdb_init(void); extern NTSTATUS idmap_nss_init(void);

/* Decl of Static init functions */
#define static_decl_nss_info extern NTSTATUS nss_info_template_init(void);

/* Decl of Static init functions */
#define static_decl_pdb extern NTSTATUS pdb_smbpasswd_init(void); extern NTSTATUS pdb_tdbsam_init(void);

/* Decl of Static init functions */
#define static_decl_rpc extern NTSTATUS rpc_lsa_init(void); extern NTSTATUS rpc_reg_init(void); extern NTSTATUS rpc_lsa_ds_init(void); extern NTSTATUS rpc_wkssvc_init(void); extern NTSTATUS rpc_svcctl_init(void); extern NTSTATUS rpc_ntsvcs_init(void); extern NTSTATUS rpc_net_init(void); extern NTSTATUS rpc_netdfs_init(void); extern NTSTATUS rpc_srv_init(void); extern NTSTATUS rpc_spoolss_init(void); extern NTSTATUS rpc_eventlog_init(void); extern NTSTATUS rpc_samr_init(void); extern NTSTATUS rpc_echo_init(void);

/* Decl of Static init functions */
#define static_decl_vfs extern NTSTATUS vfs_default_init(void); extern NTSTATUS vfs_notify_init(void);

/* Static init functions */
#define static_init_auth {  auth_sam_init();  auth_unix_init();  auth_winbind_init();  auth_server_init();  auth_domain_init();  auth_builtin_init();}

/* Static init functions */
#define static_init_charset {  charset_CP850_init();  charset_CP437_init();}

/* Static init functions */
#define static_init_idmap {  idmap_tdb_init();  idmap_passdb_init();  idmap_nss_init();}

/* Static init functions */
#define static_init_nss_info {  nss_info_template_init();}

/* Static init functions */
#define static_init_pdb {  pdb_smbpasswd_init();  pdb_tdbsam_init();}

/* Static init functions */
#define static_init_rpc {  rpc_lsa_init();  rpc_reg_init();  rpc_lsa_ds_init();  rpc_wkssvc_init();  rpc_svcctl_init();  rpc_ntsvcs_init();  rpc_net_init();  rpc_netdfs_init();  rpc_srv_init();  rpc_spoolss_init();  rpc_eventlog_init();  rpc_samr_init();  rpc_echo_init();}

/* Static init functions */
#define static_init_vfs {  vfs_default_init();  vfs_notify_init();}

