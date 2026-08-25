#!/bin/bash

SCRIPT_TAG="App message"
DIR="$(dirname $0)"
source "${DIR}/_init_target"
APP_UUID=${UUID:-c4b040f4-ea4c-481c-8050-355006f5804d}

set -x
pebble send-app-message ${TARGET_ARG} --app-uuid "${APP_UUID}" $@

set +x
echo "* Exit code: $?"
