GCOV_PROFILE := y

ifneq (,$(filter $(CONFIG_VIDEO_ECAM),y m))
#	ccflags-y += -I$(srctree)/drivers/media/platform/tegra/
	ccflags-y += -Wno-unused-variable -I$(srctree)/drivers/media/platform/tegra/ -I$(srctree)/../nvidia/include/
	obj-$(CONFIG_VIDEO_ECAM_AR0234) += ar0234.o
endif

