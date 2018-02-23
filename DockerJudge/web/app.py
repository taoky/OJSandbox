import os
from flask import Flask
from flask import url_for
from worker import celery
from celery.result import AsyncResult
import celery.states as states

env=os.environ
app = Flask(__name__)

@app.route('/add/<string:param>')
def add(param):
    task = celery.send_task('judgeApp.judge', args=[param], kwargs={})
    return "<a href='{url}'>check status of {id} </a>".format(id=task.id,
                url=url_for('check_task',id=task.id,_external=True))

@app.route('/check/<string:id>')
def check_task(id):
    res = celery.AsyncResult(id)
    if res.state==states.PENDING:
        return res.state
    else:
        return str(res.result)
if __name__ == '__main__':
    app.run(debug=env.get('DEBUG', True),
            port=int(env.get('PORT', 5001)),
            host=env.get('HOST', '0.0.0.0')
    )

@app.route('/')
def index():
    return "Flask is working now."
