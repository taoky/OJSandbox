import os
from flask import Flask
from flask import url_for
from flask import request
from flask import render_template
from worker import celery
from util import getAllProblems
from celery.result import AsyncResult
import celery.states as states
import redis

env = os.environ
app = Flask(__name__)
r = redis.StrictRedis(host="redis", port=6379, db=0)
# app.config.update(
#     CELERY_BROKER_URL="amqp://admin:mypass@rabbit:5672",
#     CELERY_RESULT_BACKEND="redis://localhost:6379",
#     CELERY_INCLUDE=['judgeApp.tasks']
# )
# celery = make_celery(app)

def result_parse(res):
    template_result = {}
    if res.state == "PENDING":
        # return "Queuing..."
        status = "Queuing"
    elif res.state == "JUDGING":
        # return "Judging..."
        status = "Judging"
    else:
        # return str(res.result)
        status = "Finished"
        template_result = res.result
        if not isinstance(template_result, dict):
            template_result = {"result": "AC", "time": 0, "mem": 0}
    return status, template_result

@app.route("/submit", methods=["POST", "GET"])
def submit():
    if request.method == "GET":
        return render_template("submit.html", problems=getAllProblems())
    else:
        param = {"problem": request.form["problem"],
            "language": request.form["language"],
            "code": request.form["code"]}
        return task_add(param)

@app.route('/task_add/<string:param>') # for debug only
def task_add(param):
    task = celery.send_task('judgeApp.judge', args=[param], kwargs={})
    return "<a href='{url}'>check status of {id} </a>".format(id=task.id,
                url=url_for('check_task',id=task.id,_external=True))

@app.route('/check/<string:id>')
def check_task(id):
    res = celery.AsyncResult(id)
    status, template_result = result_parse(res)
    # template_result = {} # to the template
    # if res.state == "PENDING":
    #     # return "Queuing..."
    #     status = "Queuing"
    # elif res.state == "JUDGING":
    #     # return "Judging..."
    #     status = "Judging"
    # else:
    #     # return str(res.result)
    #     status = "Finished"
    #     template_result = res.result
    return render_template("check_task.html", id=id, status=status, result=template_result)
    

@app.route('/')
def index():
    return "Flask is working now."

@app.route('/status')
def status():
    records = []
    for key in r.scan_iter("celery-task-meta-*"):
        id = key.decode("ascii")[len("celery-task-meta-"):]
        id_url = url_for('check_task', id=id, _external=True)
        id_res = celery.AsyncResult(id)
        status, template_result = result_parse(id_res)
        if status == "Finished":
            result = template_result["result"]
        else:
            result = ""
        records.append({"id": id, "id_url": id_url, "status": status, "result": result})
    return render_template("status.html", records=records)

if __name__ == '__main__':
    app.run(debug=env.get('DEBUG', True),
            port=int(env.get('PORT', 5001)),
            host=env.get('HOST', '0.0.0.0')
    )
