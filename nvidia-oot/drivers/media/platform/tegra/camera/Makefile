# SPDX-License-Identifier: GPL-2.0-only
# SPDX-FileCopyrightText: Copyright (c) 2022-2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

ifdef CONFIG_MEDIA_SUPPORT
ifndef CONFIG_TEGRA_SYSTEM_TYPE_ACK
LINUXINCLUDE += -I$(srctree.nvidia-oot)/drivers/video/tegra/camera
LINUXINCLUDE += -I$(srctree.nvidia-oot)/drivers/media/platform/tegra
LINUXINCLUDE += -DCONFIG_TEGRA_HOST1X
LINUXINCLUDE += -Werror

ifneq ($(NV_BUILD_SYSTEM_TYPE),embedded-linux)
LINUXINCLUDE += -DNV_IS_L4T
tegra-camera-objs := regmap_util.o
tegra-camera-objs += camera_common.o
tegra-camera-objs += sensor_common.o
tegra-camera-objs += camera_version_utils.o
tegra-camera-objs += tegracam_v4l2.o
tegra-camera-objs += tegracam_core.o
tegra-camera-objs += tegracam_ctrls.o
tegra-camera-objs += tegracam_utils.o
tegra-camera-objs += vi/vi5_fops.o
tegra-camera-objs += vi/mc_common.o
tegra-camera-objs += vi/graph.o
tegra-camera-objs += vi/channel.o
tegra-camera-objs += vi/core.o
tegra-camera-objs += csi/csi.o
tegra-camera-objs += csi/deskew.o
tegra-camera-objs += nvcsi/csi5_fops.o
endif

tegra-camera-objs += nvcamera_log.o
tegra-camera-objs += fusa-capture/capture-vi.o
tegra-camera-objs += fusa-capture/capture-common.o
tegra-camera-objs += fusa-capture/capture-vi-channel.o
obj-m += tegra-camera.o
obj-m += tests/

tegra-capture-isp-objs += fusa-capture/capture-isp-channel.o
tegra-capture-isp-objs += fusa-capture/capture-isp.o
obj-m += tegra-capture-isp.o

tegra-capture-coe-objs += coe/rtcpu-coe.o
obj-m += tegra-capture-coe.o
endif
endif
