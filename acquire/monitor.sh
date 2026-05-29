#!/usr/bin/env bash

DIR=${1:-./}

watch -n 1 "ls -lht $(realpath "${DIR}")"

