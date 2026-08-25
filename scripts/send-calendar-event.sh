#!/bin/bash

DIR="$(dirname $0)"
START_TIME_MIN=${1:-3}
END_TIME_MIN=${2:-10}

START_TIME=$("${DIR}/epochplusmin.sh" ${START_TIME_MIN})
END_TIME=$("${DIR}/epochplusmin.sh" ${END_TIME_MIN})

set -x
START_TIME_HEX=$("${DIR}"/printbytes.py ${START_TIME})
END_TIME_HEX=$("${DIR}"/printbytes.py ${END_TIME})
BYTES_STR="${START_TIME_HEX}${END_TIME_HEX}00000000"

"${DIR}/send-appmsg.sh" --bytes 112="${BYTES_STR}"
