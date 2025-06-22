/* SPDX-License-Identifier: MIT */
/*
 * Copyright © 2025 Intel Corporation
 */

#ifndef _XE_USERPTR_H_
#define _XE_USERPTR_H_

#include <linux/dma-fence.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/scatterlist.h>
#include <linux/spinlock.h>

#include <drm/drm_gpusvm.h>

struct xe_vm;
struct xe_vma;
struct xe_userptr_vma;

/** struct xe_userptr_vm - User pointer VM level state */
struct xe_userptr_vm {
	/**
	 * @userptr.repin_list: list of VMAs which are user pointers,
	 * and needs repinning. Protected by @lock.
	 */
	struct list_head repin_list;
	/**
	 * @userptr.invalidated_lock: Protects the
	 * @userptr.invalidated list.
	 */
	spinlock_t invalidated_lock;
	/**
	 * @userptr.invalidated: List of invalidated userptrs, not yet
	 * picked
	 * up for revalidation. Protected from access with the
	 * @invalidated_lock. Removing items from the list
	 * additionally requires @lock in write mode, and adding
	 * items to the list requires either the @svm.gpusvm.notifier_lock in
	 * write mode, OR @lock in write mode.
	 */
	struct list_head invalidated;
};

/** struct xe_userptr - User pointer */
struct xe_userptr {
	/** @invalidate_link: Link for the vm::userptr.invalidated list */
	struct list_head invalidate_link;
	/** @userptr: link into VM repin list if userptr. */
	struct list_head repin_link;
	/**
	 * @pages: gpusvm pages for this user pointer.
	 */
	struct drm_gpusvm_pages pages;
	/**
	 * @notifier: MMU notifier for user pointer (invalidation call back)
	 */
	struct mmu_interval_notifier notifier;

	union {
		/** @destroy_cb: callback to destroy usertpr when unbind job is done */
		struct dma_fence_cb destroy_cb;
		/** @destroy_work: worker to destroy this usertpr */
		struct work_struct destroy_work;
	};

	/**
	 * @initial_bind: user pointer has been bound at least once.
	 * write: vm->svm.gpusvm.notifier_lock in read mode and vm->resv held.
	 * read: vm->svm.gpusvm.notifier_lock in write mode or vm->resv held.
	 */
	bool initial_bind;
#if IS_ENABLED(CONFIG_DRM_XE_USERPTR_INVAL_INJECT)
	u32 divisor;
#endif
};

#if IS_ENABLED(CONFIG_DRM_GPUSVM)
int xe_userptr_setup(struct xe_userptr_vma *uvma, unsigned long start,
		     unsigned long range);
void xe_userptr_destroy(struct xe_userptr_vma *uvma, struct dma_fence *fence);

int xe_vm_userptr_pin(struct xe_vm *vm);
int __xe_vm_userptr_needs_repin(struct xe_vm *vm);
int xe_vm_userptr_check_repin(struct xe_vm *vm);
int xe_vma_userptr_pin_pages(struct xe_userptr_vma *uvma);
int xe_vma_userptr_check_repin(struct xe_userptr_vma *uvma);
#else
static inline int xe_userptr_setup(struct xe_userptr_vma *uvma,
				   unsigned long start, unsigned long range)
{
	return -ENODEV;
}

static inline void xe_userptr_destroy(struct xe_userptr_vma *uvma,
				      struct dma_fence *fence) {}

static inline int xe_vm_userptr_pin(struct xe_vm *vm) { return 0; }
static inline int __xe_vm_userptr_needs_repin(struct xe_vm *vm) { return 0; }
static inline int xe_vm_userptr_check_repin(struct xe_vm *vm) { return 0; }
static inline int xe_vma_userptr_pin_pages(struct xe_userptr_vma *uvma) { return -ENODEV; }
static inline int xe_vma_userptr_check_repin(struct xe_userptr_vma *uvma) { return -ENODEV; };
#endif

#if IS_ENABLED(CONFIG_DRM_XE_USERPTR_INVAL_INJECT)
void xe_vma_userptr_force_invalidate(struct xe_userptr_vma *uvma);
#else
static inline void xe_vma_userptr_force_invalidate(struct xe_userptr_vma *uvma)
{
}
#endif
#endif
