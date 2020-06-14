compare_dirs() {
	diffout="$(diff -ur "${1}" "${2}")"
	if [ $? -ne 0 ]; then
		printf "FAIL: Directories differ.\n\n%s\n" "${diffout}"
		exit 1
	fi
}

archive_logs() {
	"${ARCHIVE_LOGS}" "$@" ./current ./archive
	if [ $? -ne 0 ]; then
		printf "FAIL: archive-logs failed\n"
		exit 1
	fi
}

check_dirs() {
	compare_dirs current current-expected
	compare_dirs archive archive-expected
}

run_test() {
	archive_logs "$@"
	check_dirs
}
