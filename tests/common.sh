compare_dirs() {
	diffout="$(diff -r "${1}" "${2}")"
	if [ $? -ne 0 ]; then
		printf "FAIL: Directories differ.\n\n%s\n" "${diffout}"
		exit 1
	fi
}

run_test() {
	"${ARCHIVE_LOGS}" ./current ./archive
	if [ $? -ne 0 ]; then
		printf "FAIL: archive-logs failed\n"
		exit 1
	fi

	compare_dirs current current-expected
	compare_dirs archive archive-expected
}
