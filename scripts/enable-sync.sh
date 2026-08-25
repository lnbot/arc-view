#!/bin/bash

# Force the watchface to enable to disable alarm/calendar/timer sync
#
# Example:
# 	TARGET=10.1.1.2 enable-sync.sh 1
# 	TARGET=chalk enable-sync.sh 0

ENABLE=${1:-1}
DIR=$(dirname $0)
KEY="EnableAlarmCalendarSync"

"${DIR}/send-appmsg-by-name.sh" "${KEY}" --int %s=1
