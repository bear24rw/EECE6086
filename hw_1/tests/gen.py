import os
import random
import itertools
from math import factorial
from multiprocessing import Pool


def gen_new(_):

    MAX_CELLS = 25000
    MAX_NETS = 1250000

    random.seed()

    def random_combination(iterable, r):
        "Random selection from itertools.combinations(iterable, r)"
        pool = tuple(iterable)
        n = len(pool)
        indices = sorted(random.sample(xrange(n), r))
        return tuple(pool[i] for i in indices)

    def num_combinations(x):
        return factorial(x) / (factorial(x-2) * 2)

    while True:

        num_cells = random.randint(1000, MAX_CELLS)
        num_nets = random.randint(1000, min(MAX_NETS, num_combinations(num_cells)))

        filename = "B_%d_%d" % (num_cells, num_nets)
        if os.path.exists(filename) or os.path.exists(filename+".tmp"):
            continue

        break

    print "Generating %d cells and %d nets" % (num_cells, num_nets)

    with open(filename + ".tmp", 'w') as f:

        f.write("%d\n" % num_cells)
        f.write("%d\n" % num_nets)

        for _ in xrange(num_nets):
            f.write("%d %d\n" % random_combination(xrange(1,num_cells), 2))

    os.rename(filename + ".tmp", filename)

Pool(processes=4).map(gen_new, xrange(1000))
