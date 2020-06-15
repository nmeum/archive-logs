#!/bin/sh
. "${0%/*}/common.sh"

echo "foobar" > current/file1
cp current/file1 current-expected/file1

run_test
