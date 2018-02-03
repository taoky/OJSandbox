FROM frolvlad/alpine-glibc

# configure alpine repo & install language support for C, C++, Python2, Python3, Java
# & install bash, curl, useradd (shadow) 
RUN sed -i 's/dl-cdn.alpinelinux.org/mirrors.ustc.edu.cn/g' /etc/apk/repositories \
&& apk update \
&& apk add --no-cache bash gcc g++ python2 python3 openjdk8 curl shadow\
&& mkdir Backend \
&& curl -L https://github.com/taoky/OJSandbox/raw/master/Backend/init.sh > Backend/init.sh \
&& curl -L https://github.com/taoky/OJSandbox/raw/master/Backend/main > Backend/main \
&& chmod +x Backend/init.sh && chmod +x Backend/main

WORKDIR Backend

# CMD unfinished 
CMD ./init.sh
