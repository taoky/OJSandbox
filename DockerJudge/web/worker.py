import os
from celery import Celery

#env=os.environ
#CELERY_BROKER_URL=env.get('CELERY_BROKER_URL','redis://localhost:6379'),
#CELERY_RESULT_BACKEND=env.get('CELERY_RESULT_BACKEND','redis://localhost:6379')


celery = Celery('judgeApp',broker='amqp://admin:mypass@rabbit:5672',backend='rpc://',include=['judgeApp.tasks'])
