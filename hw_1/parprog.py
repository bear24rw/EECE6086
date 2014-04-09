import sys
import copy
from pprint import pprint

"""
Read in the connections
"""
num_verts = int(sys.stdin.readline().strip())
num_edges = int(sys.stdin.readline().strip())

connections = {}

for pair in sys.stdin.readlines():
    n1, n2 = pair.strip().split()
    n1 = int(n1)
    n2 = int(n2)
    if not n1 in connections:
        connections[n1] = set()
    if not n2 in connections:
        connections[n2] = set()
    connections[n1].add(n2)
    connections[n2].add(n1)

print "Given verts %d, Found verts %d" % (num_verts, len(connections))
pprint(connections)

"""
Split the nodes into two groups
"""
a = set(connections.keys()[0::2])
b = set(connections.keys()[1::2])
print "--- GROUPS ---"
print a
print b

generation = 0

while True:

    generation += 1
    print "="*100
    print "Generation %d" % generation
    print "="*100

    A1 = copy.copy(a)
    B1 = copy.copy(b)

    """
    Compute cost of all nodes
    """
    D = {}
    for n1 in connections:
        D[n1] = 0
        for n2 in connections[n1]:
            if ((n1 in a) and (n2 in b)) or ((n1 in b) and (n2 in a)):
                D[n1] += 1
            else:
                D[n1] -= 1

    print "--- COSTS ---"
    pprint(D)
    print "-------------"

    marked = []
    gains = []

    for _ in xrange(len(connections)/2):

        """
        Find (a,b) that maximizes gain
        """
        max_cost = None
        max_a = None
        max_b = None

        for node_a in A1:
            for node_b in B1:

                if node_a in marked or node_b in marked: continue

                if node_b in connections[node_a]:
                    cost = D[node_a] + D[node_b] - 2
                else:
                    cost = D[node_a] + D[node_b]

                if cost > max_cost:
                    max_cost = cost
                    max_a = node_a
                    max_b = node_b

        print "Max cost (%s, %s) = %s" % (max_a, max_b, max_cost)
        gains.append((max_a, max_b, max_cost))

        """
        Swap the nodes in the temperary groups
        """
        A1.remove(max_a)
        B1.remove(max_b)
        A1.add(max_b)
        B1.add(max_a)

        """
        Mark node_a and node_b
        """
        marked.append(max_a)
        marked.append(max_b)

        """
        Update D values for A1 and B1
        """
        for n1 in connections:
            D[n1] = 0
            for n2 in connections[n1]:
                if ((n1 in A1) and (n2 in B1)) or ((n1 in B1) and (n2 in A1)):
                    D[n1] += 1
                else:
                    D[n1] -= 1

    """
    Find k which maximizes sum of gains
    """
    g_max = None
    k_max = None
    current_sum = 0
    k = 1
    for max_a, max_b, gain in gains:
        current_sum += gain
        if current_sum > g_max:
            g_max = current_sum
            k_max = k
        k += 1

    print "k_max: %d g_max: %d" % (k_max, g_max)

    if g_max <= 0:
        print "Done"
        break

    """
    Exchange up to k_max
    """
    for i in range(k_max):
        max_a, max_b, gain = gains[i]
        print "exchanging: %d %d" % (max_a, max_b)
        a.remove(max_a)
        b.remove(max_b)
        a.add(max_b)
        b.add(max_a)

cut_size = 0
for a_node in a:
    for edge in connections[a_node]:
        if edge in b:
            cut_size += 1

print "Cutsize: %d" % cut_size
print a
print b
