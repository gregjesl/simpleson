#!/bin/bash 
apt-get install -y git wget flex bison gperf python3 python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0
# Check the current version of python
python --version
python3 -m venv myenv
. ./myenv/bin/activate
type python
# Should be python v3.X
python --version
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32
. ./export.sh