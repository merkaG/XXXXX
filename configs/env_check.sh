#  
#   @file: run an environment build check
#
#

#!/bin/env -e bash

# as in the official buildroot documentation these are trhe official needed tools
# we need to check them first

declare array packages
# TODO: find how to check that a given pakcage is installed or not 


    which
    sed
    make (version 3.81 or any later)
    binutils
    build-essential (only for Debian based systems)
    gcc (version 4.8 or any later)
    g++ (version 4.8 or any later)
    bash
    patch
    gzip
    bzip2
    perl (version 5.8.7 or any later)
    tar
    cpio
    unzip
    rsync
    file (must be in /usr/bin/file)
    bc 
    wget

# checkout buildroot


