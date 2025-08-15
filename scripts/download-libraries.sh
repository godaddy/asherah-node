#!/bin/bash

set -e  # Exit on any command failure

# Global Constants
CHECK_INTERVAL_SECONDS=$((5 * 60)) # 5 minutes
MAX_DOWNLOAD_RETRIES=3

# Function to check if a specific file download is necessary
function check_download_required {
  local file=$1
  local no_cache=$2
  local check_interval_seconds=$3
  local etag_file="${file}.etag"

  # If the file does not exist or the .etag file is missing, download is required
  if [[ ! -f "$file" ]]; then
    echo "${file} does not exist"
    return 0  # (download required)
  else
    echo "${file} exists"
  fi

  # If the file does not exist or the .etag file is missing, download is required
  if [[ ! -f "$etag_file" ]]; then
    echo "${etag_file} does not exist"
    return 0  # (download required)
  fi


  # If no-cache is passed, we ignore the modified date of the .etag files
  if [[ "$no_cache" == true ]]; then
    return 0  # (download required)
  fi

  # Check if the .etag file is older than the interval
  if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS uses 'stat -f'
    last_check=$(stat -f %m "$etag_file")
  else
    # Linux uses 'stat -c'
    last_check=$(stat -c %Y "$etag_file")
  fi

  current_time=$(date +%s)
  elapsed_time=$((current_time - last_check))

  if (( elapsed_time > check_interval_seconds )); then
    return 0  # (download required)
  fi

  return 1  # (download not required)
}

# Function to download a file
function download_file {
  local url=$1
  local file=$2
  local etag_file="${file}.etag"

  if ! curl -s -L --fail --etag-save "$etag_file" --etag-compare "$etag_file" -O "$url"; then
    echo "Failed to download $url" >&2
    exit 1
  fi

  # Explicitly touch the etag file to update its modification time only if successful
  touch "$etag_file"
}

# Function to verify checksums
function verify_checksums {
  local archive=$1
  local header=$2
  local warmup=$3
  local sums=$4

  # Determine the available SHA hashing utility
  if command -v sha256sum &> /dev/null; then
    sha_cmd="sha256sum"
  elif command -v shasum &> /dev/null; then
    sha_cmd="shasum -a 256"
  else
    echo "Error: Neither sha256sum nor shasum is available on this system." >&2
    exit 1 # Exit since this is fatal
  fi

  if [[ ! -f "${sums}" ]]; then
    echo "Error: Checksum file '${sums}' does not exist." >&2
    rm -f ./*.a ./*.h ./*.etag
    return 1
  fi

  # Filter the relevant checksums and verify they are not empty
  # Note: warmup library might not exist in older releases, so it's optional
  checksums=$(grep -e "${archive}" -e "${header}" "${sums}")
  if [[ -z "$checksums" ]]; then
    echo "Error: No matching checksums found for ${archive} or ${header} in ${sums}." >&2
    return 1
  fi
  
  # Try to add warmup checksum if it exists
  local warmup_checksum
  warmup_checksum=$(grep -e "${warmup}" "${sums}" 2>/dev/null || true)
  if [[ -n "$warmup_checksum" ]]; then
    checksums="${checksums}"$'\n'"${warmup_checksum}"
  fi

  echo "$checksums" > ./SHA256SUM  # Return value

  if ! $sha_cmd -c ./SHA256SUM; then
    echo 'SHA256 mismatch!' >&2
    rm -f ./*.a ./*.h
    return 1
  fi
}

# Function to copy library and header files
function copy_files {
  local archive=$1
  local header=$2

  if [[ ! -f "${archive}" ]]; then
    echo "Error: Source archive file '${archive}' does not exist." >&2
    exit 1
  fi

  if [[ ! -f "${header}" ]]; then
    echo "Error: Source header file '${header}' does not exist." >&2
    exit 1
  fi

  if ! cp -f "${archive}" libasherah.a; then
    echo "Error: Failed to copy archive file '${archive}' to 'libasherah.a'." >&2
    exit 1
  fi

  if ! cp -f "${header}" libasherah.h; then
    echo "Error: Failed to copy header file '${header}' to 'libasherah.h'." >&2
    exit 1
  fi
}

# Function to detect OS and CPU architecture
function detect_os_and_cpu {
  OS=$(uname)
  MACHINE=$(uname -m)

  #echo "Detected OS: ${OS}"
  #echo "Detected CPU architecture: ${MACHINE}"

  if [[ "${OS}" == 'Linux' ]]; then
    if [[ ${MACHINE} == 'x86_64' ]]; then
      #echo "Using Asherah libraries for Linux x86_64"
      ARCHIVE="libasherah-x64.a"
      HEADER="libasherah-x64-archive.h"
      WARMUP="go-warmup-linux-x64.so"
      SUMS="SHA256SUMS"
    elif [[ ${MACHINE} == 'aarch64' ]]; then
      #echo "Using Asherah libraries for Linux aarch64"
      ARCHIVE="libasherah-arm64.a"
      HEADER="libasherah-arm64-archive.h"
      WARMUP="go-warmup-linux-arm64.so"
      SUMS="SHA256SUMS"
    else
      #echo "Unsupported CPU architecture: ${MACHINE}" >&2
      exit 1
    fi
  elif [[ "${OS}" == 'Darwin' ]]; then
    if [[ ${MACHINE} == 'x86_64' ]]; then
      #echo "Using Asherah libraries for MacOS x86_64"
      ARCHIVE="libasherah-darwin-x64.a"
      HEADER="libasherah-darwin-x64-archive.h"
      WARMUP="go-warmup-darwin-x64.dylib"
      SUMS="SHA256SUMS-darwin"
    elif [[ ${MACHINE} == 'arm64' ]]; then
      #echo "Using Asherah libraries for MacOS arm64"
      ARCHIVE="libasherah-darwin-arm64.a"
      HEADER="libasherah-darwin-arm64-archive.h"
      WARMUP="go-warmup-darwin-arm64.dylib"
      SUMS="SHA256SUMS-darwin"
    else
      echo "Unsupported CPU architecture: ${MACHINE}" >&2
      exit 1
    fi
  else
    echo "Unsupported operating system: ${OS}" >&2
    exit 1
  fi

  echo "${ARCHIVE}" "${HEADER}" "${WARMUP}" "${SUMS}"  # Return value
}

# Parse script arguments
function parse_args {
  local no_cache=false
  while [[ $# -gt 0 ]]; do
    case "$1" in
      --no-cache)
        no_cache=true
        shift
        ;;
      *)
        echo "Unknown parameter: $1" >&2
        exit 1
        ;;
    esac
  done
  echo "${no_cache}"  # Return value
}

# Function to determine the interval message
function interval_message {
  local interval=$1
  if (( interval % 60 == 0 )); then
    echo "$((interval / 60)) minutes"  # Return value
  else
    echo "$interval seconds"  # Return value
  fi
}

# Main function
function main {
  echo "Downloading Asherah libraries"
  # shellcheck disable=SC1091
  source .asherah-version

  # Parse arguments
  local no_cache
  no_cache=$(parse_args "$@")

  # Detect OS and CPU architecture
  read -r archive header warmup sums < <(detect_os_and_cpu)
  echo "Archive: $archive"
  echo "Header: $header"
  echo "Warmup: $warmup"
  echo "Sums: $sums"
  echo "Version: $ASHERAH_VERSION"

  # Interpolate the URLs
  url_prefix="https://github.com/godaddy/asherah-cobhan/releases/download/${ASHERAH_VERSION}"
  file_names=("${archive}" "${header}" "${warmup}" "${sums}")
  file_urls=(
    "${url_prefix}/${archive}"
    "${url_prefix}/${header}"
    "${url_prefix}/${warmup}"
    "${url_prefix}/${sums}"
  )

  # Create the `lib` directory if it doesn't exist
  mkdir -p lib
  cd lib || exit 1

  local retries=0
  local checksums_verified=false
  while [[ $checksums_verified == false && $retries -lt $MAX_DOWNLOAD_RETRIES ]]; do
    # Per-file touch and download logic
    for i in "${!file_names[@]}"; do
      if check_download_required "${file_names[$i]}" "$no_cache" "$CHECK_INTERVAL_SECONDS"; then
        download_file "${file_urls[$i]}" "${file_names[$i]}"
      else
        interval_str=$(interval_message "$CHECK_INTERVAL_SECONDS")
        echo "${file_names[$i]} is up to date (checked within the last ${interval_str})"
      fi
    done

    # Verify checksums and copy files
    if verify_checksums "${archive}" "${header}" "${warmup}" "${sums}"; then
      copy_files "${archive}" "${header}"
      checksums_verified=true
    else
      echo "Verification failed, re-downloading files..."
      ((retries++))
      # Sleep for a bit before retrying to avoid hammering the server
      sleep 1
    fi
  done

  if [[ $checksums_verified == true ]]; then
    copy_files "${archive}" "${header}"
    echo "Asherah libraries downloaded successfully"
  else
    echo "Failed to download Asherah libraries after $retries retries."
    exit 1
  fi
}

# Execute the main function
main "$@"
