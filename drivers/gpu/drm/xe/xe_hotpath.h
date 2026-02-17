/* SPDX-License-Identifier: MIT */
/*
 * Copyright © 2026 Intel Corporation
 */

#ifndef _XE_HOTPATH_H_
#define _XE_HOTPATH_H_

#include <linux/types.h>

struct xe_device;

#ifdef CONFIG_DRM_XE_DEBUG

#include <linux/cleanup.h>

void __xe_hotpath_enter(void *);
void __xe_hotpath_exit(void *);
void xe_hotpath_assert(struct xe_device *xe);

DEFINE_GUARD(xe_hotpath, void *, __xe_hotpath_enter(_T), __xe_hotpath_exit(_T));
#define xe_hotpath_guard() scoped_guard(xe_hotpath, NULL)

#else
static inline void xe_hotpath_assert(struct xe_device *xe) { }
#define xe_hotpath_guard() do { } while (0);
#endif

#endif
