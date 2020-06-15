#!/bin/sh
. "${0%/*}/common.sh"

touch current/file1
cp current/file1 current-expected/file1

run_test
