#!/bin/sh
export LD_LIBRARY_PATH=/lib:/usr/lib:/usr/local/classpath/lib/classpath
./jemcc-static -classpath .:/usr/local/classpath/share/classpath/glibj.zip $*
