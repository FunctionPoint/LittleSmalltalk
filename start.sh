#!/bin/bash
# Start the REPL image bin/lst.img (build.sh builds it): it reads its
# commands from standard input. Always works on the folder this script is in.
# Runs bin/lst (bin/lst.exe on Windows), which the build of lst copies there.

set -e
cd "$(dirname "$0")"

lst=bin/lst
if [ ! -x "$lst" ] && [ ! -x "$lst.exe" ]; then
	echo "$lst not found: build lst first (see README.md)" >&2
	exit 1
fi
if [ ! -f bin/lst.img ]; then
	echo "bin/lst.img not found: run build.sh first" >&2
	exit 1
fi

exec "$lst" bin/lst.img "$@"
