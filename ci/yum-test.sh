#!/bin/bash

set -exu

USE_SCL=0
USE_AMZN_EXT=0

distribution=$(cat /etc/system-release-cpe | awk '{print substr($0, index($1, "o"))}' | cut -d: -f2)
version=$(cat /etc/system-release-cpe | awk '{print substr($0, index($1, "o"))}' | cut -d: -f4)
USE_SCL=0
USE_AMZN_EXT=0
USE_DNF_MODULE=0

case ${distribution} in
  amazon)
    case ${version} in
      2)
        DNF=yum
        USE_AMZN_EXT=1
        ;;
    esac
    ;;
  centos)
    case ${version} in
      7)
        DNF=yum
        USE_SCL=1
        ;;
      8)
        DNF="dnf --enablerepo=powertools"
        USE_SCL=1
        ;;
    esac
    ;;
  rocky)
    case ${version} in
      8)
        DNF="dnf --enablerepo=powertools"
        USE_DNF_MODULE=1
        ;;
      9)
        DNF="dnf --enablerepo=crb"
        ;;
    esac
    ;;
  fedoraproject)
    case ${version} in
      33|34|35|36|37)
        DNF=yum
        ;;
    esac
    ;;
esac

${DNF} groupinstall -y "Development Tools"

if [ $USE_SCL -eq 1 ]; then
    ${DNF} install -y centos-release-scl && \
    ${DNF} install -y epel-release && \
    ${DNF} install -y \
    rh-ruby26-ruby-devel \
    rh-ruby26-rubygems \
    rh-ruby26-rubygem-rake \
    rpm-build \
    cmake3
elif [ $USE_DNF_MODULE -eq 1 ]; then
    dnf update -y && \
    dnf module install -y ruby:3.0 && \
    dnf install -y ruby-devel \
        rubygems \
        rpm-build \
        cmake
elif [ $USE_AMZN_EXT -eq 1 ]; then
    yum update -y && \
    yum install -y yum-utils && \
    yum-config-manager --enable epel && \
    amazon-linux-extras install -y ruby2.6 && \
    ${DNF} install -y ruby-devel \
           cmake3
else
    ${DNF} install -y ruby-devel \
           rubygems \
           rpm-build \
           cmake \
           libarchive
fi

if [ $USE_SCL -eq 1 ]; then
    # For unbound variable error
    export MANPATH=
    cd /cmetrics-ruby && source /opt/rh/rh-ruby26/enable && gem install bundler --no-document && bundle install && bundle exec rake
else
    if [ $USE_AMZN_EXT -eq 1 ]; then
        echo 'gem "io-console"' > /cmetrics-ruby/Gemfile.local
    fi
    cd /cmetrics-ruby && gem install bundler --no-document && bundle install && bundle exec rake
fi
