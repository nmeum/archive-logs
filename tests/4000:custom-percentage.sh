#!/bin/sh
. "${0%/*}/common.sh"

cat >current/file1 <<-EOF
	1
	2
	3
	4
	5
	6
	7
	8
	9
	10
EOF

cat >current-expected/file1 <<-EOF
	8
	9
	10
EOF

cat >archive-expected/file1 <<-EOF
	1
	2
	3
	4
	5
	6
	7
EOF

run_test -k 25
