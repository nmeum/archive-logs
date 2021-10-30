#!/bin/sh
. "${0%/*}/common.sh"

cat >current/file1 <<-EOF
	1
	2
	3
	4
EOF

cat >archive/file1 <<-EOF
	0
EOF

touch current-expected/file1

cat >archive-expected/file1 <<-EOF
	0
	1
	2
	3
	4
EOF

run_test -k 0
