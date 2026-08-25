#!/bin/sh

# Takes the current date, chops off seconds, then adds a number of minutes to
# it and prints it as an epoch time (seconds)

TIME_MIN=${1:-1}
EPOCH=$(date --date $(date +%T | sed -e 's/[0-9][0-9]$/00/') +%s)
echo $((${EPOCH} + ($TIME_MIN * 60)))
