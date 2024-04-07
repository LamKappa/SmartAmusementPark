/*
 * This header was generated from the Linux kernel headers by update_headers.py,
 * to provide necessary information from kernel to userspace, such as constants,
 * structures, and macros, and thus, contains no copyrightable information.
 */
#ifndef _UAPI_LINUX_MEMFD_H
#define _UAPI_LINUX_MEMFD_H
#include <asm-generic/hugetlb_encode.h>
#define MFD_CLOEXEC		0x0001U
#define MFD_ALLOW_SEALING	0x0002U
#define MFD_HUGETLB		0x0004U
#define MFD_HUGE_SHIFT	HUGETLB_FLAG_ENCODE_SHIFT
#define MFD_HUGE_MASK	HUGETLB_FLAG_ENCODE_MASK
#define MFD_HUGE_64KB	HUGETLB_FLAG_ENCODE_64KB
#define MFD_HUGE_512KB	HUGETLB_FLAG_ENCODE_512KB
#define MFD_HUGE_1MB	HUGETLB_FLAG_ENCODE_1MB
#define MFD_HUGE_2MB	HUGETLB_FLAG_ENCODE_2MB
#define MFD_HUGE_8MB	HUGETLB_FLAG_ENCODE_8MB
#define MFD_HUGE_16MB	HUGETLB_FLAG_ENCODE_16MB
#define MFD_HUGE_32MB	HUGETLB_FLAG_ENCODE_32MB
#define MFD_HUGE_256MB	HUGETLB_FLAG_ENCODE_256MB
#define MFD_HUGE_512MB	HUGETLB_FLAG_ENCODE_512MB
#define MFD_HUGE_1GB	HUGETLB_FLAG_ENCODE_1GB
#define MFD_HUGE_2GB	HUGETLB_FLAG_ENCODE_2GB
#define MFD_HUGE_16GB	HUGETLB_FLAG_ENCODE_16GB
#endif
