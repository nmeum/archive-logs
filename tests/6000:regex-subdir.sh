#!/bin/sh
. "${0%/*}/common.sh"

cat >current/file1 <<-EOF
	fooooobaaarr
	foobar
EOF

mkdir current/ignored
cat >current/ignored/file2 <<-EOF
	test123
	barfoooooooo
EOF
cat >current/ignored/file3 <<-EOF
	1
	2
EOF
chmod -w current/ignored/*

cat >current-expected/file1 <<-EOF
	foobar
EOF

cat >archive-expected/file1 <<-EOF
	fooooobaaarr
EOF

archive_logs -e "ignored.*"
rm -r current/ignored
check_dirs
