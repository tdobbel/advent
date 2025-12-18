const std = @import("std");

const EPS = 5 * std.math.floatEps(f32);

pub fn parse_diagram(word: []const u8, ndigit: *u5) u32 {
    var res: u32 = 0;
    ndigit.* = 0;
    for (word[0 .. word.len - 1]) |w| {
        ndigit.* += 1;
        res <<= 1;
        if (w == '#') {
            res += 1;
        }
    }
    return res;
}

pub fn parse_button(allocator: std.mem.Allocator, word: []const u8) ![]u8 {
    var array = std.array_list.Managed(u8).init(allocator);
    for (word) |w| {
        if (w == ',' or w == ')') {
            continue;
        }
        try array.append(w - '0');
    }
    return array.toOwnedSlice();
}

pub fn button_to_mask(indices: []const u8, ndigit: u5) u32 {
    var res: u32 = 0;
    for (indices) |i| {
        const n: u5 = @intCast(i);
        res |= @as(u32, 1) << (ndigit - 1 - n);
    }
    return res;
}

pub fn parse_requirements(allocator: std.mem.Allocator, word: []const u8) ![]u16 {
    var jolt = std.array_list.Managed(u16).init(allocator);
    var it = std.mem.tokenizeAny(u8, word[0 .. word.len - 1], ",");
    while (it.next()) |part| {
        const n = try std.fmt.parseInt(u16, part, 10);
        try jolt.append(n);
    }
    return try jolt.toOwnedSlice();
}

const Machine = struct {
    ndigit: u5,
    diagram: u32,
    buttons: [][]u8,
    requirements: []u16,
    allocator: std.mem.Allocator,

    pub fn from_line(allocator: std.mem.Allocator, line: []u8) !Machine {
        var it = std.mem.tokenizeAny(u8, line, " ");
        var diagram: u32 = undefined;
        var jolt: []u16 = undefined;
        var ndigit: u5 = 0;
        var buttons = std.array_list.Managed([]u8).init(allocator);
        while (it.next()) |part| {
            switch (part[0]) {
                '[' => {
                    diagram = parse_diagram(part[1..], &ndigit);
                },
                '(' => {
                    try buttons.append(try parse_button(allocator, part[1..]));
                },
                '{' => {
                    jolt = try parse_requirements(allocator, part[1..]);
                },
                else => unreachable,
            }
        }
        return Machine{ .ndigit = ndigit, .diagram = diagram, .buttons = try buttons.toOwnedSlice(), .requirements = jolt, .allocator = allocator };
    }

    pub fn free(self: *Machine) void {
        for (self.buttons) |v| {
            self.allocator.free(v);
        }
        self.allocator.free(self.buttons);
        self.allocator.free(self.requirements);
    }

    pub fn constraint_matrix(self: *const Machine) ![][]f32 {
        const m = self.requirements.len;
        const n = self.buttons.len;
        var a = try self.allocator.alloc([]f32, m);
        for (0..m) |i| {
            a[i] = try self.allocator.alloc(f32, n);
        }
        for (self.buttons, 0..) |button, j| {
            for (button) |b| {
                const i: usize = @intCast(b);
                a[i][j] = 1.0;
            }
        }
        return a;
    }

    pub fn joltage_vector(self: *const Machine) ![]f32 {
        var b = try self.allocator.alloc(f32, self.requirements.len);
        for (self.requirements, 0..) |v, i| {
            b[i] = @as(f32, @floatFromInt(v));
        }
        return b;
    }

    pub fn check_solution(self: *Machine, x: []const f32, sol: *u32) !bool {
        var jolt = try self.allocator.alloc(u16, self.requirements.len);
        defer self.allocator.free(jolt);
        @memset(jolt, 0);
        for (self.buttons, 0..) |button, i| {
            const ix: i32 = @intFromFloat(@round(x[i]));
            if (ix < 0) return false;
            sol.* += @intCast(ix);
            for (button) |ib| {
                jolt[@intCast(ib)] += @intCast(ix);
            }
        }
        for (self.requirements, 0..) |target, i| {
            if (jolt[i] != target) return false;
        }
        return true;
    }
};

pub fn solve_part1(target: u32, buttons: []const u32, current: u32, cost: u8, best: *u8) void {
    if (current == target) {
        if (cost < best.*) {
            best.* = cost;
        }
        return;
    }
    if (cost > best.* or buttons.len == 0) return;
    solve_part1(target, buttons[1..], current, cost, best);
    solve_part1(target, buttons[1..], current ^ buttons[0], cost + 1, best);
}

pub fn show_matrix(comptime T: type, mat: [][]T) void {
    for (mat) |row| {
        for (row) |v| {
            std.debug.print(" {d:1.3} ", .{v});
        }
        std.debug.print("\n", .{});
    }
}

pub fn free_matrix(allocator: std.mem.Allocator, comptime T: type, mat: [][]T) void {
    for (mat) |row| {
        allocator.free(row);
    }
    allocator.free(mat);
}

pub fn square_matrix(allocator: std.mem.Allocator, comptime T: type, a: [][]T) ![][]T {
    const m = a.len;
    const n = a[0].len;
    var c = try allocator.alloc([]T, n);
    for (0..n) |i| {
        c[i] = try allocator.alloc(T, n);
        @memset(c[i], 0.0);
    }
    for (0..n) |i| {
        for (0..i + 1) |j| {
            for (0..m) |k| {
                const incr = a[k][i] * a[k][j];
                c[i][j] += incr;
                c[j][i] += incr;
            }
            if (i == j) {
                c[i][j] *= 0.5;
            }
        }
    }
    return c;
}

pub fn pre_mult(allocator: std.mem.Allocator, comptime T: type, b: []T, a: [][]T) ![]T {
    std.debug.assert(a.len == b.len);
    var c = try allocator.alloc(T, a[0].len);
    for (0..a[0].len) |i| {
        c[i] = 0;
        for (0..a.len) |j| {
            c[i] += a[j][i] * b[j];
        }
    }
    return c;
}

pub fn dot_product(comptime T: type, vec1: []T, vec2: []T) T {
    std.debug.assert(vec1.len == vec2.len);
    var scalar: T = 0;
    for (0..vec1.len) |i| {
        scalar += vec1[i] * vec2[i];
    }
    return scalar;
}

pub fn least_squares(allocator: std.mem.Allocator, a: [][]f32, b: []f32, atol: f32) ![]f32 {
    const aa = try square_matrix(allocator, f32, a);
    defer free_matrix(allocator, f32, aa);
    const ab = try pre_mult(allocator, f32, b, a);
    defer allocator.free(ab);
    var x = try allocator.alloc(f32, ab.len);

    var r = try allocator.alloc(f32, ab.len);
    defer allocator.free(r);
    var p = try allocator.alloc(f32, ab.len);
    defer allocator.free(p);
    var ap = try allocator.alloc(f32, ab.len);
    defer allocator.free(ap);

    @memcpy(x, ab);
    for (0..ab.len) |i| {
        r[i] = ab[i];
        for (0..ab.len) |j| {
            r[i] -= aa[i][j] * x[j];
        }
    }

    @memcpy(p, r);
    var r2: f32 = dot_product(f32, r, r);
    var r2new: f32 = undefined;
    var delta: f32 = 1000.0;
    while (delta > atol * atol) {
        for (0..ab.len) |i| {
            ap[i] = 0;
            for (0..ab.len) |j| {
                ap[i] += aa[i][j] * p[j];
            }
        }
        const alpha = r2 / dot_product(f32, p, ap);
        for (0..ab.len) |i| {
            x[i] += alpha * p[i];
        }
        // Modify conjugated gradient to make sure that solution is positive
        for (0..ab.len) |i| {
            if (x[i] < 0) x[i] = 0;
            r[i] = ab[i];
            for (0..ab.len) |j| {
                r[i] -= aa[i][j] * x[j];
            }
        }
        r2new = dot_product(f32, r, r);
        for (0..ab.len) |i| {
            p[i] = r[i] + r2new * p[i] / r2;
        }
        delta = (r2 - r2new) * (r2 - r2new);
        r2 = r2new;
    }
    // std.debug.print("residual: {}\n", .{r2});

    return x;
}

pub fn gaussian_elimination(a: [][]f32, b: []f32) void {
    const m = a.len;
    const n = a[0].len;
    var h: usize = 0;
    var k: usize = 0;
    while (h < m and k < n) {
        var imax: usize = h;
        for (h..m) |i| {
            if (@abs(a[i][k]) > @abs(a[imax][k])) {
                imax = i;
            }
        }
        if (std.math.approxEqAbs(f32, a[imax][k], 0, EPS)) {
            k += 1;
            continue;
        }

        std.mem.swap([]f32, &a[imax], &a[h]);
        std.mem.swap(f32, &b[h], &b[imax]);
        for (h + 1..m) |i| {
            const fac = a[i][k] / a[h][k];
            a[i][k] = 0;
            b[i] -= fac * b[h];
            for (k + 1..n) |j| {
                a[i][j] -= fac * a[h][j];
            }
        }
        h += 1;
        k += 1;
    }
}

const System = struct {
    allocator: std.mem.Allocator,
    a: [][]f32,
    b: []f32,
    free_var: []bool,
    upper_bounds: []usize,

    pub fn from_machine(machine: *const Machine) !System {
        const allocator = machine.allocator;
        const a = try machine.constraint_matrix();
        const b = try machine.joltage_vector();
        var upper = try allocator.alloc(usize, a[0].len);
        for (machine.buttons, 0..) |button, ivar| {
            var ub: usize = std.math.maxInt(usize);
            for (button) |ib| {
                const i: usize = @intCast(ib);
                if (machine.requirements[i] < ub) {
                    ub = machine.requirements[i];
                }
            }
            upper[ivar] = ub;
        }
        gaussian_elimination(a, b);
        var i: usize = 0;
        var j: usize = 0;

        var isfree = try allocator.alloc(bool, a[0].len);
        @memset(isfree, true);
        while (i < a.len and j < a[0].len) : (i += 1) {
            while (j < a[0].len and std.math.approxEqAbs(f32, a[i][j], 0, EPS)) {
                j += 1;
            }
            if (j < a[0].len) isfree[j] = false;
        }
        // show_matrix(f32, a);
        // std.debug.print("b={any}\n", .{b});
        // std.debug.print("free_var={any}\n", .{isfree});
        // std.debug.print("upper_bound={any}\n", .{upper});
        return System{ .allocator = allocator, .a = a, .b = b, .free_var = isfree, .upper_bounds = upper };
    }

    pub fn solve(self: *const System, x: []f32) void {
        var i: usize = 0;
        for (self.free_var) |skip| {
            if (!skip) i += 1;
        }
        var j: usize = self.a[0].len;
        while (i > 0) : (i -= 1) {
            j -= 1;
            while (self.free_var[j] and j > 0) {
                j -= 1;
            }
            var k: usize = self.a[0].len - 1;
            x[j] = self.b[i - 1];
            while (k > j) : (k -= 1) {
                if (self.free_var[k]) continue;
                x[j] -= x[k] * self.a[i - 1][k];
            }
            x[j] /= self.a[i - 1][j];
        }
    }

    pub fn free(self: *const System) void {
        free_matrix(self.allocator, f32, self.a);
        self.allocator.free(self.b);
        self.allocator.free(self.free_var);
        self.allocator.free(self.upper_bounds);
    }
};

pub fn solve_part2(machine: *Machine, sys: *System, col: usize, x: []f32, best: *u32) !void {
    var ivar = col;
    const nvar = sys.free_var.len;
    while (ivar < nvar and !sys.free_var[ivar]) {
        ivar += 1;
    }
    if (ivar == nvar) {
        sys.solve(x);
        var sol: u32 = 0;
        const valid_sol = try machine.check_solution(x, &sol);
        // std.debug.print("x={any} , valid={}\n", .{x, valid_sol});
        if (valid_sol and sol < best.*) {
            best.* = sol;
        }
        return;
    }
    const bold = try sys.allocator.alloc(f32, sys.b.len);
    defer sys.allocator.free(bold);
    @memcpy(bold, sys.b);
    for (0..sys.upper_bounds[ivar] + 1) |value| {
        x[ivar] = @floatFromInt(value);
        for (0..sys.a.len) |i| {
            sys.b[i] = bold[i] - sys.a[i][ivar] * x[ivar];
        }
        try solve_part2(machine, sys, ivar + 1, x, best);
    }
}

pub fn main() !void {
    if (std.os.argv.len != 2) {
        return error.InvalidArgument;
    }
    const cwd = std.fs.cwd();
    const file_name: [:0]const u8 = std.mem.span(std.os.argv[1]);
    const file = try cwd.openFile(file_name, .{});
    defer file.close();

    var buffer: [4096]u8 = undefined;
    var reader = file.reader(&buffer);

    const allocator = std.heap.c_allocator;
    var part1: u32 = 0;
    var part2: u32 = 0;

    while (try reader.interface.takeDelimiter('\n')) |line| {
        var machine = try Machine.from_line(allocator, line);
        defer machine.free();
        var buttons: []u32 = try allocator.alloc(u32, machine.buttons.len);
        defer allocator.free(buttons);
        var sol1: u8 = std.math.maxInt(u8);
        for (machine.buttons, 0..) |button, i| {
            buttons[i] = button_to_mask(button, machine.ndigit);
        }
        solve_part1(machine.diagram, buttons, 0, 0, &sol1);
        part1 += sol1;

        var sys = try System.from_machine(&machine);
        defer sys.free();

        var sol2: u32 = std.math.maxInt(u32);
        const x = try allocator.alloc(f32, sys.a[0].len);
        defer allocator.free(x);
        try solve_part2(&machine, &sys, 0, x, &sol2);
        // std.debug.print("-> sol2={}\n", .{sol2});
        part2 += sol2;
    }
    std.debug.print("Part 1: {}\n", .{part1});
    std.debug.print("Part 2: {}\n", .{part2});
}
