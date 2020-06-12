#!/bin/sh
. "${0%/*}/common.sh"

for dir in current* archive*; do
	mkdir -p "${dir}/subdir"
done

cat >current/subdir/file1 <<-EOF
	foo
	foobar
EOF

cat >current-expected/subdir/file1 <<-EOF
	foobar
EOF

cat >archive-expected/subdir/file1 <<-EOF
	foo
EOF

run_test
