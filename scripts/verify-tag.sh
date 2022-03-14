#!/bin/bash
CURRENT_TAG=${1}
PACKAGE_VER=$(jq -r .version package.json)
echo "Current tag is ${CURRENT_TAG}"
echo "Current package.json version is ${PACKAGE_VER}"
if [[ "${CURRENT_TAG}" == "${PACKAGE_VER}" ]]; then
  exit 0
fi

echo "Tag / Version mismatch!"
exit 1
