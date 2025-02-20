/*
 *  NSA Security-Enhanced Linux (SELinux) security module
 *
 *  This file contains the SELinux security data structures for kernel objects.
 *
 *  Author(s):  Stephen Smalley, <sds@tycho.nsa.gov>
 *		Chris Vance, <cvance@nai.com>
 *		Wayne Salamon, <wsalamon@nai.com>
 *		James Morris <jmorris@redhat.com>
 *
 *  Copyright (C) 2001,2002 Networks Associates Technology, Inc.
 *  Copyright (C) 2003 Red Hat, Inc., James Morris <jmorris@redhat.com>
 *  Copyright (C) 2016 Mellanox Technologies
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2,
 *	as published by the Free Software Foundation.
 */
#ifndef _SELINUX_OBJSEC_H_
#define _SELINUX_OBJSEC_H_

#include <linux/list.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/binfmts.h>
#include <linux/in.h>
#include <linux/spinlock.h>
#include <net/net_namespace.h>
#include "flask.h"
#include "avc.h"

struct task_security_struct {
	u32 osid;		/* SID prior to last execve */
	u32 sid;		/* current SID */
	u32 exec_sid;		/* exec SID */
	u32 create_sid;		/* fscreate SID */
	u32 keycreate_sid;	/* keycreate SID */
	u32 sockcreate_sid;	/* fscreate SID */
    u64 flags;
};

#define TSEC_FLAG_DENY_EXECMEM (1 << 0)
#define TSEC_FLAG_DENY_EXECMOD (1 << 1)
#define TSEC_FLAG_DENY_EXECUTE_APPDOMAIN_TMPFS (1 << 2)
#define TSEC_FLAG_DENY_EXECUTE_APP_DATA_FILE (1 << 3)
#define TSEC_FLAG_DENY_EXECUTE_NO_TRANS_APP_DATA_FILE (1 << 4)
#define TSEC_FLAG_DENY_EXECUTE_ASHMEM_DEVICE (1 << 5)
#define TSEC_FLAG_DENY_EXECUTE_ASHMEM_LIBCUTILS_DEVICE (1 << 6)
#define TSEC_FLAG_DENY_EXECUTE_PRIVAPP_DATA_FILE (1 << 7)
#define TSEC_FLAG_DENY_PROCESS_PTRACE (1 << 8)

#define TSEC_ALL_DENY_EXECUTE_FLAGS (\
	TSEC_FLAG_DENY_EXECUTE_APPDOMAIN_TMPFS | \
	TSEC_FLAG_DENY_EXECUTE_APP_DATA_FILE | \
	TSEC_FLAG_DENY_EXECUTE_NO_TRANS_APP_DATA_FILE | \
	TSEC_FLAG_DENY_EXECUTE_ASHMEM_DEVICE | \
	TSEC_FLAG_DENY_EXECUTE_ASHMEM_LIBCUTILS_DEVICE | \
	TSEC_FLAG_DENY_EXECUTE_PRIVAPP_DATA_FILE | \
0)

#define TSEC_ALL_FLAGS (\
	TSEC_FLAG_DENY_EXECMEM | \
	TSEC_FLAG_DENY_EXECMOD | \
	TSEC_ALL_DENY_EXECUTE_FLAGS | \
    TSEC_FLAG_DENY_PROCESS_PTRACE | \
0)

/*
 * get the subjective security ID of the current task
 */
static inline u32 current_sid(void)
{
	const struct task_security_struct *tsec = current_security();

	return tsec->sid;
}

enum label_initialized {
	LABEL_INVALID,		/* invalid or not initialized */
	LABEL_INITIALIZED,	/* initialized */
	LABEL_PENDING
};

struct inode_security_struct {
	struct inode *inode;	/* back pointer to inode object */
	union {
		struct list_head list;	/* list of inode_security_struct */
		struct rcu_head rcu;	/* for freeing the inode_security_struct */
	};
	u32 task_sid;		/* SID of creating task */
	u32 sid;		/* SID of this object */
	u16 sclass;		/* security class of this object */
	unsigned char initialized;	/* initialization flag */
	spinlock_t lock;
};

struct file_security_struct {
	u32 sid;		/* SID of open file description */
	u32 fown_sid;		/* SID of file owner (for SIGIO) */
	u32 isid;		/* SID of inode at the time of file open */
	u32 pseqno;		/* Policy seqno at the time of file open */
};

struct superblock_security_struct {
	struct super_block *sb;		/* back pointer to sb object */
	u32 sid;			/* SID of file system superblock */
	u32 def_sid;			/* default SID for labeling */
	u32 mntpoint_sid;		/* SECURITY_FS_USE_MNTPOINT context for files */
	unsigned short behavior;	/* labeling behavior */
	unsigned short flags;		/* which mount options were specified */
	struct mutex lock;
	struct list_head isec_head;
	spinlock_t isec_lock;
};

struct msg_security_struct {
	u32 sid;	/* SID of message */
};

struct ipc_security_struct {
	u16 sclass;	/* security class of this object */
	u32 sid;	/* SID of IPC resource */
};

struct netif_security_struct {
	struct net *ns;			/* network namespace */
	int ifindex;			/* device index */
	u32 sid;			/* SID for this interface */
};

struct netnode_security_struct {
	union {
		__be32 ipv4;		/* IPv4 node address */
		struct in6_addr ipv6;	/* IPv6 node address */
	} addr;
	u32 sid;			/* SID for this node */
	u16 family;			/* address family */
};

struct netport_security_struct {
	u32 sid;			/* SID for this node */
	u16 port;			/* port number */
	u8 protocol;			/* transport protocol */
};

struct sk_security_struct {
#ifdef CONFIG_NETLABEL
	enum {				/* NetLabel state */
		NLBL_UNSET = 0,
		NLBL_REQUIRE,
		NLBL_LABELED,
		NLBL_REQSKB,
		NLBL_CONNLABELED,
	} nlbl_state;
	struct netlbl_lsm_secattr *nlbl_secattr; /* NetLabel sec attributes */
#endif
	u32 sid;			/* SID of this object */
	u32 peer_sid;			/* SID of peer */
	u16 sclass;			/* sock security class */
	enum {				/* SCTP association state */
		SCTP_ASSOC_UNSET = 0,
		SCTP_ASSOC_SET,
	} sctp_assoc_state;
};

struct tun_security_struct {
	u32 sid;			/* SID for the tun device sockets */
};

struct key_security_struct {
	u32 sid;	/* SID of key */
};

struct ib_security_struct {
	u32 sid;        /* SID of the queue pair or MAD agent */
};

struct pkey_security_struct {
	u64	subnet_prefix; /* Port subnet prefix */
	u16	pkey;	/* PKey number */
	u32	sid;	/* SID of pkey */
};

struct bpf_security_struct {
	u32 sid;  /* SID of bpf obj creator */
};

struct perf_event_security_struct {
	u32 sid;  /* SID of perf_event obj creator */
};

#endif /* _SELINUX_OBJSEC_H_ */
