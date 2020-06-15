#!/bin/sh
. "${0%/*}/common.sh"

for dir in current current-expected archive-expected; do
	mkdir -p "${dir}/subdir1" "${dir}/subdir2"
done

# Ensure that it dosen't error out on existing directories
mkdir -p archive/subdir2

cat >current/subdir1/file1 <<-EOF
	foo
	foobar
EOF

cat >current-expected/subdir1/file1 <<-EOF
	foobar
EOF

cat >archive-expected/subdir1/file1 <<-EOF
	foo
EOF

run_test
