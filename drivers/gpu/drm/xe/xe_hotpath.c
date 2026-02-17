// SPDX-License-Identifier: MIT
/*
 * Copyright © 2026 Intel Corporation
 */

#include <linux/preempt.h>
#include <linux/xarray.h>

#include "xe_assert.h"
#include "xe_hotpath.h"

static DEFINE_XARRAY(xe_hotpath_tasks);

static bool xe_in_hotpath(struct xe_device *xe)
{
	return in_task() && xa_load(&xe_hotpath_tasks, (unsigned long)current);
}

/* Private, do not call */
void __xe_hotpath_enter(void *)
{
	if (!in_task())
		return;

	xa_store(&xe_hotpath_tasks, (unsigned long)current, (void *)1,
		 GFP_KERNEL);
}

/* Private, do not call */
void __xe_hotpath_exit(void *)
{
	if (!in_task())
		return;

	xa_erase(&xe_hotpath_tasks, (unsigned long)current);
}

void xe_hotpath_assert(struct xe_device *xe)
{
	xe_assert(xe, !xe_in_hotpath(xe));
}
