import os
from celery import Celery

#env=os.environ
#CELERY_BROKER_URL=env.get('CELERY_BROKER_URL','redis://localhost:6379'),
#CELERY_RESULT_BACKEND=env.get('CELERY_RESULT_BACKEND','redis://localhost:6379')

# def make_celery(app):
#     celery = Celery(app.import_name, broker=app.config['CELERY_BROKER_URL'])
#     celery.conf.update(app.config)
#     TaskBase = celery.Task
#     class ContextTask(TaskBase):
#         abstract = True
#         def __call__(self, *args, **kwargs):
#             with app.app_context():
#                 return TaskBase.__call__(self, *args, **kwargs)
#     celery.Task = ContextTask
#     return celery

celery = Celery('judgeApp',broker='amqp://admin:mypass@rabbit:5672',backend='redis://redis:6379',include=['judgeApp.tasks'])
