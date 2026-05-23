FROM stpos-multiboot
#ubuntu:20.04
LABEL authors="Tobias Madlberger"

RUN apt-get update
RUN apt install -y software-properties-common
RUN add-apt-repository -y ppa:rock-core/qt4
RUN apt-get update && apt-get install -y \
    build-essential  \
    rsync  \
    texinfo  \
    libncurses-dev  \
    whois  \
    unzip  \
    bc  \
    gcc=4:9.3.0-1ubuntu2 \
    qt4-linguist-tools \
    libssl-dev \
    git \
    wget \
    cpio \
    python2

RUN if [[ ! -f /usr/bin/python ]] ; then n -s /usr/bin/python2 /usr/bin/python ; else echo Python already linked ; fi

RUN wget https://www.libarchive.org/downloads/libarchive-3.3.1.tar.gz
RUN tar xzf libarchive-3.3.1.tar.gz
WORKDIR libarchive-3.3.1
RUN ./configure
RUN make
RUN make install

WORKDIR /home/dev/StpOs-Multiboot

CMD ["./build.sh"]