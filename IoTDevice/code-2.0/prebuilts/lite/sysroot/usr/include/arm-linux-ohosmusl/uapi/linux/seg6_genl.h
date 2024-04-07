/*
 * This header was generated from the Linux kernel headers by update_headers.py,
 * to provide necessary information from kernel to userspace, such as constants,
 * structures, and macros, and thus, contains no copyrightable information.
 */
#ifndef _UAPI_LINUX_SEG6_GENL_H
#define _UAPI_LINUX_SEG6_GENL_H
#define SEG6_GENL_NAME		"SEG6"
#define SEG6_GENL_VERSION	0x1
enum {
	SEG6_ATTR_UNSPEC,
	SEG6_ATTR_DST,
	SEG6_ATTR_DSTLEN,
	SEG6_ATTR_HMACKEYID,
	SEG6_ATTR_SECRET,
	SEG6_ATTR_SECRETLEN,
	SEG6_ATTR_ALGID,
	SEG6_ATTR_HMACINFO,
	__SEG6_ATTR_MAX,
};
#define SEG6_ATTR_MAX (__SEG6_ATTR_MAX - 1)
enum {
	SEG6_CMD_UNSPEC,
	SEG6_CMD_SETHMAC,
	SEG6_CMD_DUMPHMAC,
	SEG6_CMD_SET_TUNSRC,
	SEG6_CMD_GET_TUNSRC,
	__SEG6_CMD_MAX,
};
#define SEG6_CMD_MAX (__SEG6_CMD_MAX - 1)
#endif
