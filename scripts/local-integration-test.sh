#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

if [ ! -f .asherah-version ]; then
  echo "Error: The script is meant to be run from the root directory of the asherah-node project" >&2
  exit 1
fi

# Check if docker is available, otherwise try podman
if command -v docker >/dev/null 2>&1; then
  CONTAINER_CMD=docker
elif command -v podman >/dev/null 2>&1; then
  CONTAINER_CMD=podman
else
  echo "Error: Neither docker nor podman command is available" >&2
  exit 1
fi

ORIG_DIR=$(pwd)

rm -rf "${ORIG_DIR}/integration/asherah"

function cleanup {
  rm -rf "${ORIG_DIR}/integration/asherah" || echo "Failed to delete asherah checkout"
  $CONTAINER_CMD kill "${MYSQL_CONTAINER_ID}" || echo "Failed to kill container"
}

trap cleanup INT
trap cleanup EXIT

export MYSQL_HOSTNAME=mysql
export TEST_DB_NAME=testdb
export TEST_DB_USER=root
export TEST_DB_PASSWORD=Password123
export TEST_DB_PORT=3306

MYSQL_CONTAINER_ID=$($CONTAINER_CMD run --rm -d \
  --platform linux/amd64 \
  -e MYSQL_HOSTNAME=${MYSQL_HOSTNAME} \
  -e MYSQL_DATABASE=${TEST_DB_NAME} \
  -e MYSQL_USERNAME=${TEST_DB_USER} \
  -e MYSQL_ROOT_PASSWORD=${TEST_DB_PASSWORD} \
  -p 127.0.0.1:${TEST_DB_PORT}:3306/tcp \
  --health-cmd "mysqladmin --protocol=tcp -u root -pPassword123 ping" \
  --health-interval 10s \
  --health-timeout 5s \
  --health-retries 10 \
  mysql:5.7)

if [ $? -ne 0 ]; then
   echo Docker failed to start container
   exit 1
fi

echo "Waiting for MySQL to come up"
while ! mysqladmin ping --protocol=tcp -u "${TEST_DB_USER}" -p"${TEST_DB_PASSWORD}" --silent 2>/dev/null; do
    sleep 1
done

echo "Create encryption_key table"
mysql --protocol=tcp -P "${TEST_DB_PORT}" -u "${TEST_DB_USER}" -p"${TEST_DB_PASSWORD}" -e "CREATE TABLE ${TEST_DB_NAME}.encryption_key (
          id             VARCHAR(255) NOT NULL,
          created        TIMESTAMP    NOT NULL DEFAULT CURRENT_TIMESTAMP,
          key_record     TEXT         NOT NULL,
          PRIMARY KEY (id, created),
          INDEX (created)
        );" 2>/dev/null

scripts/integration-test.sh
