#!/bin/bash
if [ "$(whoami)" != "root" ] ; then
	echo "Please run as root"
	echo "Quitting ..."
	exit 1
fi

function apt_install_pkg {
	REQUIRED_PKG=$1
	PKG_OK=$(dpkg-query -W --showformat='${Status}\n' $REQUIRED_PKG|grep "install ok installed")
	echo "Checking for $REQUIRED_PKG: $PKG_OK"
	if [ "" = "$PKG_OK" ]; then
		echo ""
		echo "$REQUIRED_PKG not found. Setting it up..."
		sudo apt-get --yes install $REQUIRED_PKG 

		PKG_OK=$(dpkg-query -W --showformat='${Status}\n' $REQUIRED_PKG|grep "install ok installed")
		echo ""
		echo "Checking for $REQUIRED_PKG: $PKG_OK"

		if [ "" = "$PKG_OK" ]; then
			echo ""
			echo "$REQUIRED_PKG not installed. Please try again later"
			exit 1
		fi
	fi
}

L4T_RELEASE_PACKAGE=Jetson_Linux_R38.5.0_aarch64.tbz2
L4T_RELEASE_URL="https://developer.nvidia.com/downloads/igx/v2.0/Jetson_Linux_R38.5.0_aarch64.tbz2"
SAMPLE_FS_PACKAGE=Tegra_Linux_Sample-Root-Filesystem_R38.5.0_aarch64.tbz2
SAMPLE_FS_URL="https://developer.nvidia.com/downloads/igx/v2.0/Tegra_Linux_Sample-Root-Filesystem_R38.5.0_aarch64.tbz2"

L4T_DIR=/home/$SUDO_USER/nvidia/nvidia_sdk

if [ $# -eq 1 ]; then
	L4T_DIR=$1
fi
if [ $# -gt 1 ]; then
	echo "Please type the destination path"
	echo "Please run as:"
	echo "sudo $0 <FULL_PATH>"
	echo "Quitting ..."
	exit 1
fi

if [ -d "$L4T_DIR" ]; then
	echo "$L4T_DIR folder exists"
else
	echo "$L4T_DIR folder does not exist"
	echo "Quitting ..."
	exit 1
fi

apt_install_pkg 'wget'
apt_install_pkg 'pv'

echo "*** Downloading the required source files..."
L4T_DIR=$L4T_DIR/IGX-SW_2.0_Linux_JETSON_TARGETS

if [ -d "$L4T_DIR" ]; then
	echo "$L4T_DIR folder exists"
else
	sudo -u $SUDO_USER mkdir $L4T_DIR
fi

if [ -f "$L4T_DIR/$L4T_RELEASE_PACKAGE" ]; then
	echo "$L4T_RELEASE_PACKAGE exists"
else
	sudo -u $SUDO_USER wget -O $L4T_RELEASE_PACKAGE $L4T_RELEASE_URL
	mv $L4T_RELEASE_PACKAGE $L4T_DIR/
fi

if [ -f "$L4T_DIR/$SAMPLE_FS_PACKAGE" ]; then
	echo "$SAMPLE_FS_PACKAGE exists"
else
	sudo -u $SUDO_USER wget -O $SAMPLE_FS_PACKAGE $SAMPLE_FS_URL
	mv $SAMPLE_FS_PACKAGE $L4T_DIR/
fi

echo "*** Extracting them..."

if [ -d "$L4T_DIR/Linux_for_Tegra" ]; then
	echo "$L4T_DIR/Linux_for_Tegra folder exists"
	echo "Quitting ..."
	exit 1
else
	pv $L4T_DIR/$L4T_RELEASE_PACKAGE | sudo -u $SUDO_USER tar -jx -C $L4T_DIR
	pv $L4T_DIR/$SAMPLE_FS_PACKAGE | tar -jx -C $L4T_DIR/Linux_for_Tegra/rootfs/
	cd $L4T_DIR/Linux_for_Tegra
	./tools/l4t_flash_prerequisites.sh
	./apply_binaries.sh --openrm
fi

echo "*** Done."
