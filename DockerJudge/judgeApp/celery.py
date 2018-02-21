from celery import Celery
app = Celery('judgeApp',broker='amqp://admin:mypass@rabbit:5672',backend='rpc://',include=['judgeApp.tasks'])
