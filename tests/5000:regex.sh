#!/bin/sh
. "${0%/*}/common.sh"

cat >current/file1 <<-EOF
	fooooobaaarr
	foobar
EOF

cat >current/file2 <<-EOF
	test123
	barfoooooooo
EOF
cat >current/file3 <<-EOF
	1
	2
EOF
chmod -w current/file2 current/file3

cat >current-expected/file1 <<-EOF
	foobar
EOF

cat >archive-expected/file1 <<-EOF
	fooooobaaarr
EOF

archive_logs -e "file[2-3]"
rm current/file2 current/file3
check_dirs
