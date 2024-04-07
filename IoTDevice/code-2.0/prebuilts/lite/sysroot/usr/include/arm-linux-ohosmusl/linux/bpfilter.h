/*
 * This header was generated from the Linux kernel headers by update_headers.py,
 * to provide necessary information from kernel to userspace, such as constants,
 * structures, and macros, and thus, contains no copyrightable information.
 */
#ifndef _UAPI_LINUX_BPFILTER_H
#define _UAPI_LINUX_BPFILTER_H
#include <linux/if.h>
enum {
	BPFILTER_IPT_SO_SET_REPLACE = 64,
	BPFILTER_IPT_SO_SET_ADD_COUNTERS = 65,
	BPFILTER_IPT_SET_MAX,
};
enum {
	BPFILTER_IPT_SO_GET_INFO = 64,
	BPFILTER_IPT_SO_GET_ENTRIES = 65,
	BPFILTER_IPT_SO_GET_REVISION_MATCH = 66,
	BPFILTER_IPT_SO_GET_REVISION_TARGET = 67,
	BPFILTER_IPT_GET_MAX,
};
#endif
