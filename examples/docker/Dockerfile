# Create build container
FROM node:lts-slim AS build
# Upgrade to latest packages
RUN apt-get update && apt-get upgrade -y
# Install requirements for node-gyp to build
RUN apt-get install make gcc g++ curl python3 -y
# Need to have run npm install locally first for node_modules
COPY . /src/
WORKDIR /src
# Rebuild only the native / binary modules
RUN npm rebuild

# Create final container
FROM node:lts-slim AS final
# Copy the artifacts from the build container
COPY --from=build /src /src
WORKDIR /src
USER nobody
RUN node .
