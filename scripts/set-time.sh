#!/bin/bash

# Force the watchface to display a certain time
#
# Example:
# 	TARGET=10.1.1.2 set-time.sh +60
# 	TARGET=chalk set-time.sh 15:00

TIME="$1"
DIR=$(dirname $0)
KEY="TestSetTime"
EPOCH_SEC=0

if [[ "${TIME}" =~ [0-9]{1,2}:[0-9]{1,2} ]]; then
  EPOCH_SEC="$(date --date ${TIME} +%s)"
elif [[ "${TIME}" =~ [+-][0-9]+ ]]; then
  EPOCH_SEC="$(($(date +%s) + ${TIME} * 60))"
else
  echo "* Unsetting clock"
fi

"${DIR}/send-appmsg-by-name.sh" "${KEY}" --int %s=${EPOCH_SEC}
