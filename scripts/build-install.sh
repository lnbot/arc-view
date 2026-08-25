#!/bin/bash

# Builds and installs on a given target
# Set environment variable TARGET to an ip address to hit a real device.
# By default, installs to the current emulator.
#
# Example:
#       TARGET=10.1.1.2 build-install.sh
#       TARGET=chalk build-install.sh

SCRIPT_TAG="Install"
DIR="$(dirname $0)"
PROJECT_HOME="$(dirname $0)/.."
source "${DIR}/_init_target"
cd $PROJECT_HOME

set -x
pebble build && pebble install ${TARGET_ARG}

set +x
echo "* Exit code: $?"
