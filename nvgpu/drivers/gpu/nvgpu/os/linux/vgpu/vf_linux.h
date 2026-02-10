/*
  * Copyright (c) 2023, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NVGPU_VF_LINUX_H
#define NVGPU_VF_LINUX_H

struct pci_dev;
struct gk20a_platform;

#ifdef CONFIG_NVGPU_GR_VIRTUALIZATION
int vf_probe(struct pci_dev *pdev, struct gk20a_platform *platform);
#else
static inline int vf_probe(struct pci_dev *pdev,
		struct gk20a_platform *platform)
{
	return -ENOSYS;
}
#endif

#endif