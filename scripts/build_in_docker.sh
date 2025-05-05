#!/bin/bash

# Copyright (c) 2019 Rumen G.Bogdanovski
# All rights reserved.
#
# You can use this software under the terms of 'INDIGO Astronomy
# open-source license'
# (see https://github.com/indigo-astronomy/indigo/blob/master/LICENSE.md).
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHORS 'AS IS' AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

echo FROM $1 >Dockerfile
cat >>Dockerfile <<EOF
LABEL maintainer="rumenastro@gmail.com"
RUN apt-get -y update && apt-get -y install wget unzip build-essential autoconf autotools-dev libtool cmake libudev-dev libavahi-compat-libdnssd-dev libusb-1.0-0-dev libcurl4-gnutls-dev libz-dev git curl bsdmainutils qtbase5-dev qtmultimedia5-dev devscripts cdbs apt-transport-https
RUN apt-get -y remove systemd; exit 0
RUN echo 'deb [trusted=yes] https://indigo-astronomy.github.io/indigo_ppa/ppa indigo main' >>/etc/apt/sources.list
RUN apt-get update
RUN apt-get -y install indigo
COPY indigo-control-panel-$2.tar.gz .
RUN tar -zxf indigo-control-panel-$2.tar.gz
RUN rm indigo-control-panel-$2.tar.gz
WORKDIR indigo-control-panel-$2
RUN qmake indigo_tools.pro
RUN scripts/builddeb.sh $2
EOF
docker build -t icp .
docker create --name icp icp
docker cp icp:/indigo-control-panel_$2_$3.deb .
docker container rm icp
docker image rm icp
rm Dockerfile
