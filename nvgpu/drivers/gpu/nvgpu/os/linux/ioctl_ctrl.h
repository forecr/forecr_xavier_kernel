/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef __NVGPU_IOCTL_CTRL_H__
#define __NVGPU_IOCTL_CTRL_H__

struct nvgpu_tsg;

struct gk20a_ctrl_priv {
	struct device *dev;
	struct gk20a *g;
	struct nvgpu_clk_session *clk_session;
	struct nvgpu_cdev *cdev;
#ifdef CONFIG_NVGPU_TSG_SHARING
	u64 device_instance_id;
	u64 tsg_share_token;
	struct nvgpu_list_node tsg_share_tokens_list;
	struct nvgpu_mutex tokens_lock;
#endif

	/**
	 * This ref is initialized during gk20a_ctrl_dev_open.
	 * This is ref_get whenever a TSG is opened for a device.
	 * This is ref_put whenever a TSG is released from a device.
	 */
	struct nvgpu_ref refcount;

	struct nvgpu_list_node list;
	struct {
		struct vm_area_struct *vma;
		bool vma_mapped;
	} usermode_vma;
};

void nvgpu_ioctl_ctrl_release(struct nvgpu_ref *ref);

int gk20a_ctrl_dev_open(struct inode *inode, struct file *filp);
int gk20a_ctrl_dev_release(struct inode *inode, struct file *filp);
long gk20a_ctrl_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
int gk20a_ctrl_dev_mmap(struct file *filp, struct vm_area_struct *vma);

void nvgpu_hide_usermode_for_poweroff(struct gk20a *g);
void nvgpu_restore_usermode_for_poweron(struct gk20a *g);

#ifdef CONFIG_NVGPU_TSG_SHARING
u64 nvgpu_gpu_get_device_instance_id(struct gk20a_ctrl_priv *priv);
int nvgpu_gpu_get_share_token(struct gk20a *g,
			      u64 source_device_instance_id,
			      u64 target_device_instance_id,
			      struct nvgpu_tsg *tsg,
			      u64 *token);
int nvgpu_gpu_revoke_share_token(struct gk20a *g,
				 u64 source_device_instance_id,
				 u64 target_device_instance_id,
				 u64 token,
				 struct nvgpu_tsg *tsg);
int nvgpu_gpu_tsg_revoke_share_tokens(struct gk20a *g,
				      u64 source_device_instance_id,
				      struct nvgpu_tsg *tsg,
				      u32 *out_count);
struct nvgpu_tsg *nvgpu_gpu_open_tsg_with_share_token(struct gk20a *g,
				 u64 source_device_instance_id,
				 u64 target_device_instance_id,
				 u64 token);
void nvgpu_gpu_encode_share_token(struct gk20a *g,
				  u64 source_device_instance_id,
				  u64 token, size_t share_token_sz,
				  u8 *share_token);
void nvgpu_gpu_decode_share_token(struct gk20a *g,
				  u8 *share_token,
				  size_t share_token_sz,
				  u64 *source_device_instance_id, u64 *token);
#endif

#endif
