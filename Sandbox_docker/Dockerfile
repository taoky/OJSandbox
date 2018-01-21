FROM debian

ARG username=sandbox
ARG userpasswd=sandbox

RUN sed -i 's/deb.debian.org/mirrors.ustc.edu.cn/g' /etc/apt/sources.list \
&& sed -i 's|security.debian.org|mirrors.ustc.edu.cn/debian-security|g' /etc/apt/sources.list \
&& apt-get update \
&& apt-get upgrade -y \
&& apt-get install -y openssh-server sudo build-essential \
&& echo AddressFamily inet >> /etc/ssh/sshd_config \
&& useradd -ms /bin/bash $username && (echo $username:$userpasswd | chpasswd) \
&& adduser $username sudo

CMD /usr/sbin/sshd -D
