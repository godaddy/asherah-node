#!/bin/bash

#echo "Updating apt"
#sudo apt-get update -y

#echo "Installing qemu support"
#sudo apt-get install qemu binfmt-support qemu-user-static -y

#echo "Register qemu support with docker"
#docker run --rm --privileged multiarch/qemu-user-static --reset -p yes

echo "Testing ${TEST_TAG} emulation"
docker run --rm -v "$(pwd):/build" --platform "${TEST_PLATFORM}" --entrypoint /build/scripts/runs-on-emulated.sh "${TEST_TAG}/node:bookworm"
