FROM debian:stretch

ARG OJSUSER=ojs

# configure debian repo & install language support for C, C++, Python2, Python3, Java
# & install curl, libsecccomp2 (required by backend)
# & init user (ojs)
RUN sed -i 's/deb.debian.org/mirrors.ustc.edu.cn/g' /etc/apt/sources.list \
&& sed -i 's|security.debian.org|mirrors.ustc.edu.cn/debian-security|g' /etc/apt/sources.list \
&& apt update \
&& apt -y install gcc g++ python python3 openjdk-8-jdk-headless curl libseccomp2 \
&& rm -rf /var/lib/apt/lists/* \
&& mkdir Backend \
&& curl -L https://github.com/taoky/OJSandbox/raw/master/Backend/init.sh > Backend/init.sh \
&& curl -L https://github.com/taoky/OJSandbox/raw/master/Backend/main > Backend/main \
&& chmod 0555 Backend/init.sh && chmod 0555 Backend/main && chmod 0555 Backend \
&& useradd -s /usr/sbin/nologin -r -M -d /dev/null $OJSUSER

WORKDIR Backend

# CMD unfinished 
# CMD ./init.sh
CMD /bin/bash