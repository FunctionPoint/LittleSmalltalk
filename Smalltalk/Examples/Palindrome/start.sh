#!/bin/bash
# Start the image lst.img of the Palindrome example (build.sh builds it): it runs
# Main>>start, then lst exits. Always works on the folder this script is in.
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

exec "$lst" lst.img "$@"
