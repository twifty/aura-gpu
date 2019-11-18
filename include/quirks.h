/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _UAPI_INCLUDE_QUIRKS_H
#define _UAPI_INCLUDE_QUIRKS_H

#include <linux/version.h>

/*
    This file contains a few fixes for changes between kernels.
 */

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(5,1,18))
static inline void list_swap(struct list_head *entry1, struct list_head *entry2)
{
    struct list_head *pos = entry2->prev;

    list_del(entry2);
    list_replace(entry1, entry2);
    if (pos == entry1)
        pos = entry2;
    list_add(entry1, pos);
}
#endif

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(5,0,0))
#define is_user_memory(_ptr, _size) access_ok(VERIFY_READ, _ptr, _size)
#else
#define is_user_memory access_ok
#endif

#ifndef EXPORT_SYMBOL_NS_GPL
#define EXPORT_SYMBOL_NS_GPL(sym, ns) EXPORT_SYMBOL_GPL(sym)
#endif

#endif
