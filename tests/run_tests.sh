#!/bin/sh

cd "${0%/*}"

export ARCHIVE_LOGS="${ARCHIVE_LOGS:-$(pwd)/../archive-logs}"
if [ ! -x "${ARCHIVE_LOGS}" ]; then
	printf "'%s' does not exist\n" "${ARCHIVE_LOGS}" 1>&2
	exit 1
fi

export TESTDIR="${TMPDIR:-/tmp}/archive-logs-tests"
trap "rm -rf '${TESTDIR}' 2>/dev/null" INT EXIT

scriptdir="$(pwd)"
for test in [0-9][0-9][0-9][0-9]:*; do
	mkdir -p "${TESTDIR}"
	cd "${TESTDIR}"
	mkdir -p current current-expected archive archive-expected

	name="${test##*/}"
	printf "Running test case '%s': " "${name}"

	"${scriptdir}/${test##*/}"
	if [ $? -ne 0 ]; then
		exit 1
	fi

	printf "OK\n"
	cd "${scriptdir}" && rm -rf "${TESTDIR}"
done

exit 0
