FROM gcr.io/oss-fuzz-base/base-builder
RUN apt-get update && apt-get install -y make autoconf automake libtool cmake \
                      pkg-config curl check
COPY . $SRC/simpleson
COPY .clusterfuzzlite/build.sh $SRC/build.sh
COPY .clusterfuzzlite/fuzzer.cpp $SRC/fuzzer.cpp
WORKDIR simpleson
