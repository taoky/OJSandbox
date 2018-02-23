import subprocess
from celery import Celery
app = Celery('tasks',broker='amqp://admin:mypass@rabbit:5672',backend='rpc://')

@app.task(name="judgeApp.judge")
def judge(runProg):
    cp = subprocess.run(["/judgeApp/safeJudger", "-i", "/dev/null", "-o", "/dev/null", "--", runProg], stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    return (cp.stdout.split("\n"), cp.stderr.split("\n"))
