from math import factorial

def combinations(x):
    return factorial(x) / (factorial(x-2) * 2)

with open("factorial.txt", "w") as f:
    for i in range(2, 2000):
        f.write("%d %d\n" % (i, combinations(i)))
