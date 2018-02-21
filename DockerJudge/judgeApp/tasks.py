from judgeApp.celery import app
import subprocess

@app.task
def judge(runProg):
    cp = subprocess.run(["/judgeApp/judgeApp/safeJudger", "-i", "/dev/null", "-o", "/dev/null", "--allow-multi-process", "--", runProg], stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    return (cp.stdout.split("\n"), cp.stderr.split("\n"))
