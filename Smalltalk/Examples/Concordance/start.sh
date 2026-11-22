#!/bin/bash
# Start the image lst.img of the Concordance example (build.sh builds it): it runs
# Main>>start (it reads text from standard input), then lst exits. Always works on the folder this script is in.
# Runs bin/lst (bin/lst.exe on Windows), which the build of lst copies there.

set -e
cd "$(dirname "$0")"

lst=../../../bin/lst
if [ ! -x "$lst" ] && [ ! -x "$lst.exe" ]; then
	echo "$lst not found: build lst first (see README.md)" >&2
	exit 1
fi
if [ ! -f lst.img ]; then
	echo "lst.img not found: run build.sh first" >&2
	exit 1
fi

# the text: standard input, or Input.txt when that is not a pipe or a file
if [ -t 0 ]; then
	exec "$lst" lst.img "$@" < Input.txt
fi
exec "$lst" lst.img "$@"
