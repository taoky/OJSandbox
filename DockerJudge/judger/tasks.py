import subprocess
from celery import Celery
import time, random # for debug
app = Celery('tasks',broker='amqp://admin:mypass@rabbit:5672',backend='redis://redis:6379')

@app.task(name="judgeApp.judge", bind=True)
def judge(self, runProg):
    self.update_state(state="JUDGING")
    # cp = subprocess.run(["/judgeApp/safeJudger", "-i", "/dev/null", "-o", "/dev/null", "--", runProg], stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    # return (cp.stdout.split("\n"), cp.stderr.split("\n"))
    time.sleep(5)
    #self.update_state(state="SUCCESS")
    return {"result": "AC", "time": random.randint(1, 100), "mem": random.randint(1, 200)}
