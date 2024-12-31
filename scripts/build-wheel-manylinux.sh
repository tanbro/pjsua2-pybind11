#!/usr/bin/env bash

# Python build script for the docker-compose service of many_linux.
# DO NOT run it alone.

set -eu

export PIP_DISABLE_PIP_VERSION_CHECK=1
export PIP_ROOT_USER_ACTION=ignore
export PIP_NO_WARN_SCRIPT_LOCATION=1

PYTHON_LIST=(python3.8 python3.9 python3.10 python3.11 python3.12 python3.13)

for PYTHON in ${PYTHON_LIST[@]}
do
    echo
    echo "---------------------------------------------------------------"
    echo "Begin of ${PYTHON}"
    echo "---------------------------------------------------------------"
    echo

    ${PYTHON} -m build -w

    echo
    echo "---------------------------------------------------------------"
    echo "End of ${PYTHON}"
    echo "---------------------------------------------------------------"
    echo
done

auditwheel repair dist/*-linux_*.whl
