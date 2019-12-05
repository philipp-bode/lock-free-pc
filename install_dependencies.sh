#!/bin/bash

if [[ "$OSTYPE" == "linux-gnu" ]]; then
    if [[ -f /etc/lsb-release ]];then
        apt-get update -qq
        apt-get install -qq libgsl0-dev libarmadillo-dev libboost-all-dev
    elif [[ -f /etc/centos-release ]] || [[ -f /etc/redhat-release ]]; then
        yum -y install blas-devel lapack-devel boost-devel armadillo-devel gsl-devel
    fi

elif [[ "$OSTYPE" == "darwin"* ]]; then
    brew install gsl armadillo boost
fi
