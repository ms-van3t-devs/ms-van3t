#!/bin/bash

NS3_VERSION=3.35

if [ $# -ne 0 -a $# -ne 1 ]; then
    echo "Too many arguments. You shall specify:"
    echo "	(Optional) 'install-dependencies' to attempt the installation of the ns-${NS3_VERSION} main dependencies"

    exit 1
fi

if [ "$EUID" -eq 0 ]; then
	echo "Please do NOT run this script as root."
	exit 1
fi

if [ -d "./ns-3-allinone" ]; then
    echo "Error: cannot proceed if there is a directory named ns-3-allinone is the current path."
    echo "Please rename it or move/launch the script from another directory."

    exit 1
fi

read -p "Press ENTER to continue with the installation..."

if [ ! -z $1 ]; then
	if [ $1 == "install-dependencies" ]; then
		echo "Installing some of the main dependencies for ns-${NS3_VERSION}..."
		sudo apt update
		if [ $? -ne 0 ]; then
			echo "Error: cannot run 'apt update'."
			exit 1
		fi

		sudo apt install gcc g++ python python3 python3-setuptools git mercurial qt5-default \
			openmpi-bin openmpi-common openmpi-doc libopenmpi-dev \
			autoconf cvs bzr unrar \
			gdb valgrind uncrustify \
			python3-sphinx dia gsl-bin libgsl-dev libgslcblas0 \
			tcpdump sqlite sqlite3 libsqlite3-dev \
			libxml2 libxml2-dev \
			libgtk2.0-0 libgtk2.0-dev
			
		# Detecting the current Ubuntu version to install the correct version of libgsl
		# This is done only on Ubuntu (i.e. only if the command "lsb_release" returns "Ubuntu" as distro)
		# If Ubuntu >= 20.10 is detected, libgsl25 is installed, otherwise libgsl23 is installed
		# If the distro is not detected to be Ubuntu, for the time being, the script attempts to install libgsl23
		# Please report any bug when using "install-dependencies": we will try to fix it as soon as possible!
		lsb_release -d | grep Ubuntu > /dev/null
		
		if [ $? -eq 0 ]; then
			version=$(lsb_release -d | cut -d " " -f2 | cut -d "." -f1)
			subversion=$(lsb_release -d | cut -d " " -f2 | cut -d "." -f2)
			
			if [ $version -gt 20 -o \( $version -ge 20 -a $subversion -ge 10 \) ]; then
				echo "Detected Ubuntu >= 20.10 - installing libgsl25"
				sudo apt install libgsl25
			else
				echo "Detected Ubuntu < 20.10 - installing libgsl23"
				sudo apt install libgsl23
			fi
		else
			echo "Detected a distribution different than Ubuntu - installing libgsl23"
			sudo apt install libgsl23
		fi

		if [ $? -ne 0 ]; then
			echo "Error: cannot install the dependencies."
			exit 1
		fi
	else
		echo "Error: the second optional argument can be equal to \"install-dependencies\" or left empty only."
		echo "\"$1\" is not a valid optional argument."
		exit 1
	fi
fi

echo "Downloading ns-${NS3_VERSION} from the official repository..."
sleep 1
set -v
git clone https://gitlab.com/cttc-lena/ns-3-dev.git
cd ns-3-dev/src/
git clone https://gitlab.com/cttc-lena/nr.git
cd nr
git checkout nr-v2x-dev
find . -type d -name .git -exec rm -rfv {} \;
find . -type f -name .gitignore -exec rm -rfv {} \;
find . -type f -name .gitlab-ci-clang.yml -exec rm -rfv {} \;
find . -type f -name .gitlab-ci-gcc.yml -exec rm -rfv {} \;
find . -type f -name .gitlab-ci.yml -exec rm -rfv {} \;
find . -type f -name .gitmodules -exec rm -rfv {} \;
cd ../..
git checkout v2x-lte-dev
set +v

echo "Removing git from vanilla ns-${NS3_VERSION}..."
sleep 1
set -v
find . -type d -name .git -exec rm -rfv {} \;
find . -type f -name .gitignore -exec rm -rfv {} \;
find . -type f -name .gitattributes -exec rm -rfv {} \;
set +v

echo "Installing the sandbox..."
sleep 1
set -v
cd ..
shopt -s extglob
shopt -s dotglob
cp -af ./!(ns-3-dev) ns-3-dev/
set +v

echo "Patching CMakeLists.txt to solve compatibility issue with C source files"
sleep 1
set -v
cd ns-3-dev
sed -i -E 's#^([[:blank:]]*)project\(NS3 CXX\)#\1project\(NS3 C CXX\)#' CMakeLists.txt
cd ..
set +v

echo "Moving the full installation to the current directory..."
sleep 1
set -v
rm -rfv AUTHORS .git .gitignore img LICENSE license_gplv2.txt README.md src switch_CAM_DENM_version.sh VERSION
set +v

echo "Installation completed. You will find a copy of this script in ./ns-3-dev."

rm $0
