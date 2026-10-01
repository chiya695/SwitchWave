FROM devkitpro/devkita64:20260215

SHELL ["/bin/bash", "-c"]

ARG GIMP_VERSION=2

# Build tools + GIMP (for BCn texture conversion)
RUN if [ "$GIMP_VERSION" = "3" ]; then \
        echo 'deb http://deb.debian.org/debian trixie main' > /etc/apt/sources.list.d/trixie.list \
        && printf 'Package: *\nPin: release n=trixie\nPin-Priority: 100\n' > /etc/apt/preferences.d/trixie; \
    fi \
    && apt-get update && apt-get install -y --no-install-recommends \
    autoconf automake libtool \
    bison flex \
    python3-pip ninja-build \
    wget ca-certificates \
    && if [ "$GIMP_VERSION" = "3" ]; then \
        apt-get install -y -t trixie --no-install-recommends gimp python3-mako; \
    else \
        apt-get install -y --no-install-recommends gimp python3-mako; \
    fi \
    && pip3 install --break-system-packages meson \
    && rm -rf /var/lib/apt/lists/*

ENV DEVKITPRO=/opt/devkitpro
ENV PORTLIBS_PREFIX=${DEVKITPRO}/portlibs/switch

RUN wget -q https://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.18.tar.gz -O /tmp/libiconv.tar.gz \
    && echo '3b08f5f4f9b4eb82f151a7040bfd6fe6c6fb922efe4b1659c66ea933276965e8  /tmp/libiconv.tar.gz' | sha256sum -c - \
    && tar xzf /tmp/libiconv.tar.gz -C /tmp \
    && cd /tmp/libiconv-1.18 \
    && source ${DEVKITPRO}/switchvars.sh \
    && ./configure --prefix="${PORTLIBS_PREFIX}" --host=aarch64-none-elf \
        --disable-shared --enable-static --disable-nls \
    && make lib/localcharset.h \
    && make -C lib -j2 && make -C lib install \
    && install -Dm644 include/iconv.h.inst "${PORTLIBS_PREFIX}/include/iconv.h" \
    && rm -rf /tmp/libiconv-1.18 /tmp/libiconv.tar.gz

# Build libusbhsfs (GPL)
RUN git clone --depth 1 -b dev https://github.com/DarkMatterCore/libusbhsfs.git /tmp/libusbhsfs \
    && cd /tmp/libusbhsfs \
    && source ${DEVKITPRO}/switchvars.sh \
    && make BUILD_TYPE=gpl install \
    && rm -rf /tmp/libusbhsfs

# Build libsmb2
RUN git clone --depth 1 https://github.com/sahlberg/libsmb2.git /tmp/libsmb2 \
    && cd /tmp/libsmb2 \
    && make -f Makefile.platform switch_install \
    && rm -rf /tmp/libsmb2

# Build libnfs v5.0.2
COPY misc/libnfs/switch.patch /tmp/libnfs-switch.patch
RUN wget -qO- https://github.com/sahlberg/libnfs/archive/refs/tags/libnfs-5.0.2.tar.gz | tar xz -C /tmp \
    && cd /tmp/libnfs-libnfs-5.0.2 \
    && patch -Np1 -i /tmp/libnfs-switch.patch \
    && source ${DEVKITPRO}/switchvars.sh \
    && ./bootstrap \
    && ./configure --prefix="${PORTLIBS_PREFIX}" --host=aarch64-none-elf \
        --disable-shared --enable-static --disable-werror --disable-utils --disable-examples \
    && make && make install \
    && rm -rf /tmp/libnfs-libnfs-5.0.2 /tmp/libnfs-switch.patch

WORKDIR /mnt
