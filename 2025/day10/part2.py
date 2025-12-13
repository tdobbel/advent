import sys
import re
import numpy as np
from scipy.optimize import milp, LinearConstraint

part2 = 0

with open(sys.argv[1], "r") as fp:
    while line := fp.readline():
        line = line.strip()
        buttons = []
        for b in re.findall(r"\(([0-9,]+)\)", line):
            button = tuple(map(int, b.split(",")))
            buttons.append(button)
        target = re.findall(r"{([0-9,]+)}", line)
        b = np.array(list(map(int, target[0].split(","))))
        m, n = b.size, len(buttons)
        c = np.ones(n)
        integr = np.ones(n, dtype=np.int32)
        a = np.zeros((m, n))
        for j, button in enumerate(buttons):
            for i in button:
                a[i, j] = 1
        res = milp(c=c, constraints=LinearConstraint(a, b, b), integrality=integr)
        part2 += int(np.sum(res.x))

print(part2)
