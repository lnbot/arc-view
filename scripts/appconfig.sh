#!/bin/bash

# Opens up app config.
# Set environment variable TARGET to an ip address to hit a real device.
# By default, installs to the current emulator.
#
# Example:
#       TARGET=10.1.1.2 appconfig.sh
#       TARGET=chalk appconfig.sh

SCRIPT_TAG="App config"
DIR="$(dirname $0)"
PROJECT_HOME="$(dirname $0)/.."
source "${DIR}/_init_target"
cd $PROJECT_HOME

set -x
pebble emu-app-config ${TARGET_ARG}

set +x
echo "* Exit code: $?"
