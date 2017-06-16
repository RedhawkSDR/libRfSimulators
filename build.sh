#!/bin/sh
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of libRfSimulators.
#
# libRfSimulators is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# libRfSimulators is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

if [ "$1" = "rpm" ]; then
    # A very simplistic RPM build scenario
    if [ -e libRfSimulators.spec ]; then
        mydir=`dirname $0`
        tmpdir=`mktemp -d`
        cp -r ${mydir} ${tmpdir}/libRfSimulators-1.1.1
        tar czf ${tmpdir}/libRfSimulators-1.1.1.tar.gz --exclude=".git" -C ${tmpdir} libRfSimulators-1.1.1
        git show ExampleFiles > ${tmpdir}/libFmRdsSimulatorExamples.tar.gz
        echo `ls ${tmpdir}`
        rpmbuild -ta ${tmpdir}/libRfSimulators-1.1.1.tar.gz
        echo "1"
        rm -rf $tmpdir
    else
        echo "Missing RPM spec file in" `pwd`
        exit 1
    fi
else
    ./reconf
    ./configure
    make -j
fi
