#!/bin/bash

apt-get update
apt-get install -y jq

if [ -z "${1}" ]; then
  echo "set-version.sh <version>"
  exit 1
fi

VERSION=${1#v}

if [[ ${VERSION} =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  jq --arg version "${VERSION}" '.version = $version' package.json > updated-package.json
  mv updated-package.json package.json
  exit 0
fi

echo "Bad version: ${1}"
exit 1
