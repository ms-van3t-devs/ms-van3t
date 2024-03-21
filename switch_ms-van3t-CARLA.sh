#!/bin/bash

is_anaconda_installed() {
    # Check if 'conda' command is available
    if command -v conda &> /dev/null; then
        return 0 # Anaconda is installed
    else
        return 1 # Anaconda is not installed
    fi
}

ns_3_dir=$(pwd)

mode_file="src/automotive/aux-files/current-mode.txt" 

if [ ! -f "$mode_file" ]; then
    echo "Error: File '$mode_file' not found."
    exit 1
fi
mode=$(head -n 1 "$mode_file")

if [ "$mode" = "base" ]; then
	read -p "Current mode is 'base', do you wish to switch to ms-van3t-CARLA? (WARNING: GPS-TC and Emulation capabilities are disabled in this mode)."

	config_file="CARLA-OpenCDA.conf"

	carla_found=false
	opencda_found=false



	if [ -f "$config_file" ]; then
	    
	    # If the file exists, check for CARLA_HOME and OpenCDA_HOME
	    if grep -q "CARLA_HOME" "$config_file"; then
	        carla_found=true
		echo "CARLA installation found."
	    fi
	    if grep -q "OpenCDA_HOME" "$config_file"; then
	        opencda_found=true
		echo "OpenCDA installation found."
	    fi
	fi


	if [ "$carla_found" = false ]; then
	    read -p "CARLA installation not found in CARLA-OpenCDA.conf. Press Enter to install CARLA or Ctrl+C to cancel."

	    lsb_release -d | grep Ubuntu > /dev/null
	    if [ $? -eq 0 ]; then
			version=$(lsb_release -d | cut -d " " -f2 | cut -d "." -f1)
			subversion=$(lsb_release -d | cut -d " " -f2 | cut -d "." -f2)
			
			if [ $version -gt 22 -o \( $version -ge 22 -a $subversion -ge 04 \) ]; then
				echo "WARNING: Detected Ubuntu >= 22.04 - CARLA only officially supports Ubuntu 18.04 and Ubuntu 20.04 "
				sudo apt install libomp5
			fi
		else
			echo "Error: Detected a distribution different than Ubuntu"
			exit 1
		fi

		sudo apt install -y software-properties-common build-essential cmake debhelper git wget curl xdg-user-dirs xserver-xorg libvulkan1 libsdl2-2.0-0 libsm6 libgl1-mesa-glx libomp5 pip unzip libjpeg8 libtiff5 software-properties-common nano fontconfig

		ns_3_dir=$(pwd)
		wget https://carla-releases.s3.eu-west-3.amazonaws.com/Linux/CARLA_0.9.12.tar.gz 
		wget https://carla-releases.s3.eu-west-3.amazonaws.com/Linux/AdditionalMaps_0.9.12.tar.gz
		mkdir CARLA_0.9.12
		tar -xzf CARLA_0.9.12.tar.gz -C CARLA_0.9.12/
		cp AdditionalMaps_0.9.12.tar.gz CARLA_0.9.12/Import/
		rm CARLA_0.9.12.tar.gz
		rm AdditionalMaps_0.9.12.tar.gz
		cd CARLA_0.9.12/Import/
		tar -xzf AdditionalMaps_0.9.12.tar.gz
		cd ..
		./ImportAssets.sh
		carla_dir=$(pwd)
		cd .. 

		echo "CARLA_HOME=$carla_dir" >> "$config_file"
		echo "CARLA_0.9.12 installation completed."
	fi


	if [ "$opencda_found" = false ]; then
	    read -p "OpenCDA installation not found in CARLA-OpenCDA.conf! Press Enter to install OpenCDA or Ctrl+C to cancel."
	    echo "Enter 'conda' to install OpenCDA with conda (HIGHLY RECOMMENDED) or 'pip' to install OpenCDA with pip:"
	    read -r install_method 

		ns_3_dir=$(pwd)
		sudo apt install -y software-properties-common build-essential cmake debhelper git wget curl xdg-user-dirs xserver-xorg libvulkan1 libsdl2-2.0-0 libsm6 libgl1-mesa-glx libomp5 pip unzip libjpeg8 libtiff5 software-properties-common nano fontconfig

		# Add Python PPA and install Python 3.7 and other dependencies
		sudo add-apt-repository ppa:deadsnakes/ppa
		sudo apt-get install -y python3.7 ffmpeg libsm6 libxext6 python3-pip python3.7-distutils python3-apt python3-dev protobuf-compiler libprotobuf-dev libgrpc++-dev libzmq5-dev libglfw3-dev python-dev libpython3.7-dev libpython3-dev libyaml-cpp-dev

		git clone https://github.com/carlosrisma/OpenCDA.git
		cd OpenCDA

	    case $install_method in
	            conda)
	                echo "Installing OpenCDA with conda..."
	                
	                if is_anaconda_installed; then
	    				echo "Anaconda is already installed."
					else
					    echo "Installing Anaconda..."
					    wget https://repo.anaconda.com/archive/Anaconda3-2022.05-Linux-x86_64.sh -O anaconda.sh
					    bash anaconda.sh -b
					    source ~/anaconda3/etc/profile.d/conda.sh
					    rm anaconda.sh
					fi

					conda env create -f environment.yml

					# Get the path of the interpreter
					env_name="msvan3t_carla" 
					interpreter_path=$(conda env list | grep "^$env_name\s" | awk '{print $2 "/bin/python3"}')

					export CARLA_HOME="$ns_3_dir/CARLA_0.9.12"
					export CARLA_VERSION=0.9.12 

					# Initialize Conda for the script's shell
					eval "$(conda shell.bash hook)"

					. setup.sh

					conda activate msvan3t_carla

					# Installing following packages one by one for reliability 
					conda install cudatoolkit=11.1 -c pytorch -c conda-forge
					conda install pytorch==1.8.0 -c pytorch -c conda-forge
					conda install torchvision -c pytorch -c conda-forge
					conda install torchaudio -c pytorch -c conda-forge

					pip install carla==0.9.12
					pip install lxml
					pip install cycler
					pip install pandas
					pip install open3d
					pip install filterpy
					pip install psutil

					cd ..

					echo "OpenCDA_HOME=$ns_3_dir/OpenCDA" >> "$config_file"
					echo "Python_Interpreter=$interpreter_path" >> "$config_file"
					echo "OpenCDA installation completed."	

	                ;;
	            pip)
	                echo "Installing OpenCDA with pip..."
	                python3.7 -m pip install -r requirements.txt

					python3.7 -m pip install carla==0.9.12
					python3.7 -m pip install coloredlogs omegaconf 
					python3.7 -m pip install 'gitpython>=3.1.30'
					python3.7 -m pip install 'Pillow>=10.0.1'
					python3.7 -m pip install 'ultralytics>=8.0.147'
					python3.7 -m pip install torch==1.13.1+cu117 torchvision==0.14.1+cu117 torchtext==0.14.1 torchaudio==0.13.1 torchdata==0.5.1 --extra-index-url https://download.pytorch.org/whl/cu117
					python3.7 -m pip install open3d
					python3.7 -m pip install grpcio-tools grpcio conan==1.52.0
					python3.7 -m pip install pyzmq==25.0.2 zmq
					python3.7 -m pip install filterpy

					export CARLA_HOME="$ns_3_dir/CARLA_0.9.12"
					export CARLA_VERSION=0.9.12 
					# FROM OpenCDA setup.sh
					CARLA_EGG_FILE=${CARLA_HOME}/PythonAPI/carla/dist/carla-"${CARLA_VERSION}"-py3.7-linux-x86_64.egg
					if [ ! -f "$CARLA_EGG_FILE" ]; then
					    echo "Error: $CARLA_EGG_FILE can not be found. Please make sure you are using python3.7 and carla 0.9.11. "
					    return 0
					fi

					CACHE=${PWD}/cache
					if [ ! -d "$CACHE" ]; then
					  echo "creating cache folder for carla PythonAPI egg file"
					  mkdir -p "$CACHE"
					fi

					echo "copying egg file to cache folder"
					cp  $CARLA_EGG_FILE $CACHE

					echo "unzip egg file"
					unzip "${CACHE}"/carla-"${CARLA_VERSION}"-py3.7-linux-x86_64.egg -d "${CACHE}"/carla-"${CARLA_VERSION}"-py3.7-linux-x86_64

					echo "copy setup file to egg folder"
					SETUP_PY=${PWD}/scripts/setup.py
					cp "$SETUP_PY"  "${CACHE}"/carla-"${CARLA_VERSION}"-py3.7-linux-x86_64/

					python3.7 -m pip install -e ${CACHE}/carla-"${CARLA_VERSION}"-py3.7-linux-x86_64
					
					cd ..

					echo "OpenCDA_HOME=$ns_3_dir/OpenCDA" >> "$config_file"
					echo "Python_Interpreter=python3.7" >> "$config_file"
					echo "OpenCDA installation completed."

	                ;;
	            *)
	                echo "Invalid input. Installation aborted."
	                exit 1
	                ;;
	        esac
	fi

	# Switch CMakeLists.txt and PRRsup
	ns_3_dir=$(pwd)
	cd src/automotive/
	cp aux-files/CMakeLists-CARLA.txt CMakeLists.txt
	cp aux-files/CMakeLists-examples-CARLA.txt examples/CMakeLists.txt
	cp aux-files/Measurements-CARLA/* model/Measurements/

	rm aux-files/current-mode.txt
	echo "CARLA" >>  aux-files/current-mode.txt
	cd "$ns_3_dir"
	cd src/carla/proto/
	./buildProto.sh
	cd "$ns_3_dir"
	echo "Succesfully switched to ms-van3t-CARLA!"
fi 


if [ "$mode" = "CARLA" ]; then
	read -p "Current mode is 'CARLA', do you wish to switch to base ms-van3t? (WARNING: CARLA-OpenCDA capabilities are disabled in this mode)."

	# Switch back to 'base'

	ns_3_dir=$(pwd)
	cd src/automotive/
	cp aux-files/CMakeLists-base.txt CMakeLists.txt
	cp aux-files/CMakeLists-examples-base.txt examples/CMakeLists.txt
	cp aux-files/Measurements-base/* model/Measurements/

	rm aux-files/current-mode.txt
	echo "base" >>  aux-files/current-mode.txt
	cd "$ns_3_dir"
	echo "Succesfully switched to base ms-van3t!"
fi





