#!/bin/sh
. "${0%/*}/common.sh"

cat >current/file1 <<-EOF
	3
	4
EOF

cat >current-expected/file1 <<-EOF
	4
EOF

cat >archive/file1 <<-EOF
	1
	2
EOF

cat >archive-expected/file1 <<-EOF
	1
	2
	3
EOF

run_test
