from judgeApp.tasks import judge
import time

if __name__ == '__main__':
    for _ in range(10):
        result = judge.delay("/bin/ls")
        print('Task finished?', result.ready())
        print('Task result:', result.result)
        time.sleep(5)
        print('Task finished"', result.ready())
        print('Task result:', result.result)

