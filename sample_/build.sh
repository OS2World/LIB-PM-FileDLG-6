#!/bin/sh
# build.sh -- build OPENTEST and SAVETEST on ArcaOS/OS2
# Output is tee'd to compile.log so it can be reviewed afterward.
make -f Makefile.os2 2>&1 | tee compile.log
