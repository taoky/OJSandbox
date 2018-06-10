import random

f = open("random_num.txt", "w")

for i in range(0, 10000000):
    s = "%d " % (random.randint(0, i))
    f.write(s)
