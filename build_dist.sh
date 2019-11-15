#!/bin/bash
set -e -x

# Install system requirements
yum -y install cmake3
ln -sfn /usr/bin/cmake3 /usr/bin/cmake

# Compile wheels
for PYBIN in /opt/python/cp3[6-9]*/bin; do
    # "${PYBIN}/pip" install -r requirements.txt
    "${PYBIN}/pip" wheel . -w wheelhouse/
done

PLAT="$(cut -d'/' -f3 <<<"$IMAGE")"

# Bundle external shared libraries into the wheels
for whl in wheelhouse/*.whl; do
    auditwheel repair "$whl" --plat $PLAT -w wheelhouse/
done
