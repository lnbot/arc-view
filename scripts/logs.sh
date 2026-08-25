#!/bin/bash

DIR="$(dirname $0)"
source "${DIR}/_init_target"

set -x
pebble logs ${TARGET_ARG}

