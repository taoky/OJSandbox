FROM debian:stretch

ARG OJSUSER=ojs

# configure debian repo & install language support for C, C++, Python2, Python3, Java
# & install curl, libsecccomp2 (required by backend)
# & init user (ojs)
RUN sed -i 's/deb.debian.org/mirrors.ustc.edu.cn/g' /etc/apt/sources.list \
&& sed -i 's|security.debian.org|mirrors.ustc.edu.cn/debian-security|g' /etc/apt/sources.list \
&& apt update \
&& apt -y install gcc g++ python python3 python3-pip openjdk-8-jdk-headless curl libseccomp2 libcap2 libcap2-bin\
&& rm -rf /var/lib/apt/lists/* \
&& useradd -s /usr/sbin/nologin -r -M -d /dev/null $OJSUSER \
&& echo -n "grant { };" > /etc/java.policy \
&& chmod 0444 /etc/java.policy

ADD requirements.txt /judgeApp/requirements.txt
WORKDIR /judgeApp

RUN mkdir -p /root/.config/pip && echo "[global]\nindex-url = https://mirrors.ustc.edu.cn/pypi/web/simple\nformat = columns" > /root/.config/pip/pip.conf \
&& pip3 install -r requirements.txt

ADD . /judgeApp
RUN chmod 0555 safeJudger && chmod 0555 /judgeApp \
&& setcap cap_kill,cap_setuid,cap_setgid,cap_sys_resource+ep safeJudger

ENTRYPOINT celery -A tasks worker --loglevel=info
