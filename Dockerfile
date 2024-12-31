FROM quay.io/pypa/manylinux_2_28_x86_64 AS base

ARG PJPROJECT=pjproject-2.14.1
ARG PIP_INDEX_URL
ENV PIP_INDEX_URL=${PIP_INDEX_URL}

FROM base as build_pjproject
COPY downloads/ /downloads/
RUN tar -xvf /downloads/${PJPROJECT}.tar.gz -C /downloads && \
    cd /downloads/${PJPROJECT} && \
    ./configure --enable-shared && \
    make dep && make && make install && \
    cd && rm -fr /downloads

FROM base AS install_pjproject
COPY --from=build_pjproject /usr/local/ /usr/local/
RUN ldconfig
