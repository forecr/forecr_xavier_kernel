old-dtb := $(dtb-y)
old-dtbo := $(dtbo-y)
dtb-y :=
dtbo-y :=
makefile-path := platform/t19x/galen/kernel-dts

dtb-$(CONFIG_ARCH_TEGRA_19x_SOC) += tegra194-p2888-0001-p2822-0000.dtb
dtb-$(CONFIG_ARCH_TEGRA_19x_SOC) += tegra194-p2888-0001-p2822-0000-imx274-hdmi.dtb
dtb-$(CONFIG_ARCH_TEGRA_19x_SOC) += tegra194-p2888-0001-p2822-0000-imx185_v1.dtb
dtb-$(CONFIG_ARCH_TEGRA_19x_SOC) += tegra194-p2888-0001-p2822-0000-maxn.dtb
dtb-$(CONFIG_ARCH_TEGRA_19x_SOC) += tegra194-p2888-0001-e3366-1199.dtb
dtb-$(CONFIG_ARCH_TEGRA_19x_SOC) += tegra194-p2888-0006-e3366-1199.dtb
dtb-$(CONFIG_ARCH_TEGRA_19x_SOC) += tegra194-p2888-as-0006-p2822-0000.dtb
dtb-$(CONFIG_ARCH_TEGRA_19x_SOC) += tegra194-p2888-as-p3668-p2822-0000.dtb
dtbo-$(CONFIG_ARCH_TEGRA_19x_SOC) += tegra194-p2888-0001-p2822-0000-hdr40.dtbo
dtbo-$(CONFIG_ARCH_TEGRA_19x_SOC) += tegra194-p2888-0001-p2822-0000-adafruit-sph0645lm4h.dtbo
dtbo-$(CONFIG_ARCH_TEGRA_19x_SOC) += tegra194-p2888-0001-p2822-0000-adafruit-uda1334a.dtbo
dtbo-$(CONFIG_ARCH_TEGRA_19x_SOC) += tegra194-p2888-0001-p2822-0000-fe-pi-audio.dtbo
dtbo-$(CONFIG_ARCH_TEGRA_19x_SOC) += tegra194-p2888-0001-p2822-0000-respeaker-4-mic-array.dtbo
dtbo-$(CONFIG_ARCH_TEGRA_19x_SOC) += tegra194-p2888-0001-p2822-0000-respeaker-4-mic-lin-array.dtbo
dtbo-$(CONFIG_ARCH_TEGRA_19x_SOC) += tegra194-p2888-0001-p2822-0000-m2ke.dtbo
dtbo-$(CONFIG_ARCH_TEGRA_19x_SOC) += tegra194-p2888-0001-p2822-0000-csi.dtbo

ifneq ($(dtb-y),)
dtb-y := $(addprefix $(makefile-path)/,$(dtb-y))
endif
ifneq ($(dtbo-y),)
dtbo-y := $(addprefix $(makefile-path)/,$(dtbo-y))
endif

dtb-y += $(old-dtb)
dtbo-y += $(old-dtbo)
