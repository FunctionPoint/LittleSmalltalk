#!/bin/bash
# Build the image lst.img of the Queen example: the system classes and the
# sources in this folder. Always works on the folder this script is in.
# Runs bin/lst (bin/lst.exe on Windows), which the build of lst copies there.

set -e
cd "$(dirname "$0")"

lst=../../../bin/lst
if [ ! -x "$lst" ] && [ ! -x "$lst.exe" ]; then
	echo "$lst not found: build lst first (see README.md)" >&2
	exit 1
fi

"$lst" -o lst.img ../../System .
