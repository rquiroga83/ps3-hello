# Usamos Debian 12 como base
FROM debian:bookworm

# Definimos el frontend como no interactivo
ARG DEBIAN_FRONTEND=noninteractive

# 1. Instalamos todas las dependencias necesarias
# Se han añadido 'xz-utils' para la descompresión y 'libtool-bin' para el script de verificación 
RUN apt-get update && apt-get install -y \
    autoconf automake bison flex gcc g++ make libtool libtool-bin patch \
    libelf-dev libncurses5-dev zlib1g-dev libgmp3-dev libssl-dev \
    python-is-python3 python-dev-is-python3 python3-setuptools \
    pkg-config texinfo wget git subversion curl bzip2 xz-utils clang \
    && apt-get clean

# 2. Configurar variables de entorno críticas
ENV PS3DEV=/usr/local/ps3dev
ENV PSL1GHT=${PS3DEV}
ENV PATH=${PATH}:${PS3DEV}/bin:${PS3DEV}/ppu/bin:${PS3DEV}/spu/bin

# 3. Clonar y construir el toolchain oficial
# Nota: Este proceso puede tardar un par de horas ya que compila GCC varias veces [3]
RUN git clone --depth 1 https://github.com/ps3dev/ps3toolchain.git /tmp/ps3toolchain \
    && cd /tmp/ps3toolchain \
    &&./toolchain.sh \
    && rm -rf /tmp/ps3toolchain

WORKDIR /src