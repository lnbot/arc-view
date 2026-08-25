#!/bin/bash

# Sends an alarm pin 1 minute in the future (by default) to the currently
# running emulator (by default).
# Set environment variable TARGET to an ip address to hit a real device,
# and first argument to change the pin time.
#
# Example:
# 	TARGET=10.1.1.2 send-alarm-pin.sh 10
# 	TARGET=chalk send-alarm-pin.sh 10

TIME_MIN=${1:-2}
DIR=$(dirname $0)

"${DIR}/send-appmsg.sh" --uint 113=$("${DIR}/epochplusmin.sh" ${TIME_MIN})
