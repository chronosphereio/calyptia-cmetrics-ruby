#!/bin/bash

set -exu

export DEBIAN_FRONTEND=noninteractive

if [ -f /etc/lsb-release ]; then
    . /etc/lsb-release
    distribution=$DISTRIB_ID
    version=${DISTRIB_RELEASE%%.*}
    codename=${DISTRIB_CODENAME%%.*}
else
    distribution="Debian"
    version=$(cat /etc/debian_version | cut -d'.' -f1)
fi

apt update

case "$distribution" in
    "Ubuntu")
        case "$codename" in
            "bionic")
                apt install -V -y wget gpg
                wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null && \
                echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ bionic main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null && \
                apt update
                ;;
            *)
                ;;
        esac
        ;;
esac

apt install -V -y lsb-release

apt install -V -y ruby-dev git build-essential pkg-config cmake
cd /cmetrics-ruby && \
    gem install bundler --no-document && \
    bundle install && \
    bundle exec rake
