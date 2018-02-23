FROM python:3.6

ADD requirements.txt /judgeApp/requirements.txt
WORKDIR /judgeApp

RUN mkdir -p /root/.config/pip && echo "[global]\nindex-url = https://mirrors.ustc.edu.cn/pypi/web/simple\nformat = columns" > /root/.config/pip/pip.conf \
&& pip install -r requirements.txt

ADD . /judgeApp

EXPOSE 5001

ENTRYPOINT ["python"]
CMD ["app.py"]
