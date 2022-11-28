#!/bin/bash
# Install Python 3.7.15
wget https://www.python.org/ftp/python/3.7.15/Python-3.7.15.tgz
tar -xvf Python-3.7.15.tgz
cd Python-3.7.15
./configure --enable-optimizations
make
make install
cd ~
# ESP idf dependencies
apt-get install -y git wget flex bison gperf python3 python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0
git clone --recursive https://github.com/gregjesl/esp-idf.git
cd esp-idf
./install.sh esp32
. ./export.sh