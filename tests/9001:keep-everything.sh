#!/bin/sh
. "${0%/*}/common.sh"

cat >current/file1 <<-EOF
	1
	2
	3
EOF

cat >current-expected/file1 <<-EOF
	1
	2
	3
EOF

run_test -k 100
