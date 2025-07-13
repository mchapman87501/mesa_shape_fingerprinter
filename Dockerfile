FROM ubuntu:25.04

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    git gcc python3 python3-pip cmake cmake-data ninja-build \
    lcov doxygen \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir /source
VOLUME /source

# There is also a separate volume mounted at /source/build.  Why 
# doesn't it need to be declared here?
RUN mkdir /source/build
VOLUME /source/build


RUN useradd -u 10000 mchapman
USER mchapman
WORKDIR /home/mchapman

ENTRYPOINT ["/bin/bash"]
CMD ["/source/scripts/docker/in_container/build_release.sh"]