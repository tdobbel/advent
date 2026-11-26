import math
import sys

with open(sys.argv[1], "r") as fp:
    lines = fp.read().strip().split("\n")
graph = {}
for line in lines:
    parts = line.split(" -> ")
    graph[parts[0]] = parts[1].split(", ")
res = []
for m in graph["broadcaster"]:
    m2 = m
    bin = ""
    while True:
        # decode chains of flip flops as bits in an integer
        g = graph["%" + m2]

        # flip-flops that link to a conjunction are ones
        # everything else is a zero
        bin = ("1" if len(g) == 2 or "%" + g[0] not in graph else "0") + bin
        nextl = [next_ for next_ in graph["%" + m2] if "%" + next_ in graph]
        if len(nextl) == 0:
            break
        m2 = nextl[0]
    res += [int(bin, 2)]
# find least common multiple of integers
print(math.lcm(*res))
