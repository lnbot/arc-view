#!/bin/bash

# Send a message with by message key name
#
# Example:
# 	TARGET=10.1.1.2 send-appmsg-by-name.sh DigitalHour --uint %s=0

KEYNAME=$1
shift

if [[ -z "${KEYNAME}" ]]; then
  echo "No message key name given"
  exit 1
fi

DIR=$(dirname $0)
MSGKEYS="${DIR}/../build/src/message_keys.auto.c"
KEY="MESSAGE_KEY_${KEYNAME}"

if [[ ! -e "${MSGKEYS}" ]]; then
  echo "Can't find message keys file ${MSGKEYS}"
  exit 1
fi
MSGKEY_VAL=$(awk -F '[ ;]' "/${KEY}/"'{ print $(NF-1) }' "${MSGKEYS}")

if [[ -z "${MSGKEY_VAL}" ]]; then
  echo "Can't find value of '${MSGKEY_VAL}' in ${MSGKEYS}"
  exit 1
fi

processed_args=()
for arg in "$@"; do
  processed_args+=("${arg//%s/${MSGKEY_VAL}}")
done

"${DIR}/send-appmsg.sh" ${processed_args[@]}
