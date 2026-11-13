const std = @import("std");

var GLOBAL_MOD: u64 = 1;

const Operator = enum { Sum, Prod };

const MonkeyOperation = struct {
    operator: Operator,
    rhs: ?u64,

    pub fn from_desc(line: []const u8) !MonkeyOperation {
        const i = std.mem.indexOf(u8, line, "old").?;
        const op: Operator = if (line[i + 4] == '+') .Sum else .Prod;
        const rhs_str = line[(i + 6)..];
        const rhs = if (std.mem.eql(u8, rhs_str, "old")) null else try std.fmt.parseInt(u64, rhs_str, 10);
        return MonkeyOperation{ .operator = op, .rhs = rhs };
    }

    pub fn apply(self: *const MonkeyOperation, old: u64) u64 {
        const rhs = self.rhs orelse old;
        return switch (self.operator) {
            .Sum => old + rhs,
            .Prod => old * rhs,
        };
    }
};

const MonkeyTest = struct {
    divider: u64 = undefined,
    monkey_true: usize = undefined,
    monkey_false: usize = undefined,

    pub fn apply(self: *const MonkeyTest, x: u64) usize {
        return if (x % self.divider == 0) self.monkey_true else self.monkey_false;
    }

    pub fn from_desc(desc: [3][]const u8) MonkeyTest {
        const divider = parse_last_number(desc[0]);
        GLOBAL_MOD *= divider;
        const monkey_true = parse_last_number(desc[1]);
        const monkey_false = parse_last_number(desc[2]);
        return MonkeyTest{
            .divider = divider,
            .monkey_true = @intCast(monkey_true),
            .monkey_false = @intCast(monkey_false),
        };
    }
};

pub fn parse_last_number(line: []const u8) u64 {
    var factor: u64 = 1;
    var i: usize = line.len - 1;
    var v: u64 = 0;
    while (line[i] != ' ') : (i -= 1) {
        v += factor * @as(u64, line[i] - '0');
        factor *= 10;
    }
    return v;
}

const Monkey = struct {
    alloc: std.mem.Allocator,
    worry_levels: std.ArrayList(u64),
    monkey_test: MonkeyTest,
    operation: MonkeyOperation,
    counter: u64 = 0,

    pub fn apply_rule(self: *const Monkey, x: u64) usize {
        return self.monkey_test.apply(x);
    }

    pub fn apply_operation(self: *const Monkey, old: u64) u64 {
        return self.operation.apply(old);
    }

    pub fn add_worry_level(self: *Monkey, wlev: u64) !void {
        try self.worry_levels.append(self.alloc, wlev);
    }

    pub fn from_desc(gpa: std.mem.Allocator, lines: [6][]const u8) !Monkey {
        var worry_levels: std.ArrayList(u64) = .empty;
        const line = lines[1];
        const start = std.mem.indexOf(u8, line, ": ").?;
        const nums: []const u8 = line[(start + 2)..];
        var split_iter = std.mem.splitSequence(u8, nums, ", ");
        while (split_iter.next()) |item| {
            const v = try std.fmt.parseInt(u64, item, 10);
            try worry_levels.append(gpa, v);
        }
        const op = try MonkeyOperation.from_desc(lines[2]);
        const mtest = MonkeyTest.from_desc(lines[3..6].*);
        return Monkey{
            .alloc = gpa,
            .worry_levels = worry_levels,
            .monkey_test = mtest,
            .operation = op,
        };
    }

    pub fn clone(self: *const Monkey) !Monkey {
        var worry_copy: std.ArrayList(u64) = .empty;
        for (self.worry_levels.items) |item| {
            try worry_copy.append(self.alloc, item);
        }
        return Monkey{
            .alloc = self.alloc,
            .worry_levels = worry_copy,
            .monkey_test = self.monkey_test,
            .operation = self.operation,
        };
    }
};

pub fn next_round(monkeys: []Monkey, divide_by_three: bool) !void {
    for (0..monkeys.len) |i| {
        var monkey = &monkeys[i];
        while (monkey.worry_levels.pop()) |old| {
            var worry = monkey.apply_operation(old);
            if (divide_by_three) {
                worry /= 3;
            } else {
                worry = worry % GLOBAL_MOD;
            }
            const indx = monkey.apply_rule(worry);
            try monkeys[indx].add_worry_level(worry);
            monkey.counter += 1;
        }
    }
}

pub fn monkey_business(allocator: std.mem.Allocator, monkeys: []const Monkey) !u64 {
    var n_inspected: []u64 = try allocator.alloc(u64, monkeys.len);
    for (monkeys, 0..) |m, i| {
        n_inspected[i] = m.counter;
    }
    std.mem.sort(u64, n_inspected, {}, std.sort.desc(u64));
    const v: u64 = n_inspected[0] * n_inspected[1];
    allocator.free(n_inspected);
    return v;
}

pub fn print_monkeys(monkeys: []const Monkey) void {
    for (monkeys, 0..) |monkey, i| {
        std.debug.print("Monkey {}: {any}\n", .{ i, monkey.worry_levels.items });
    }
}

pub fn main(init: std.process.Init) !void {
    const args = try init.minimal.args.toSlice(init.arena.allocator());
    if (args.len < 2) {
        return error.MissingInputFile;
    }

    // Allocator
    var arena = std.heap.ArenaAllocator.init(std.heap.c_allocator);
    defer arena.deinit();
    const gpa = arena.allocator();

    // Read lines
    const file = try std.Io.Dir.cwd().openFile(init.io, args[1], .{ .mode = .read_only });
    defer file.close(init.io);

    const stats = try file.stat(init.io);
    // size - 1 to ignore last new line character
    const content: []u8 = try std.posix.mmap(
        null,
        @intCast(stats.size - 1),
        .{ .READ = true },
        .{ .TYPE = .SHARED },
        file.handle,
        0,
    );

    var lines: std.ArrayList([]const u8) = .empty;
    var line_iter = std.mem.splitScalar(u8, content, '\n');
    while (line_iter.next()) |line| {
        try lines.append(gpa, line);
    }

    var monkey_list: std.ArrayList(Monkey) = .empty;

    var ptr: usize = 0;
    while (ptr < lines.items.len) : (ptr += 7) {
        const monkey = try Monkey.from_desc(gpa, lines.items[ptr..][0..6].*);
        try monkey_list.append(gpa, monkey);
    }

    const monkeys: []Monkey = try monkey_list.toOwnedSlice(gpa);
    var monkeys2 = try gpa.alloc(Monkey, monkeys.len);
    for (monkeys, 0..) |m, i| {
        monkeys2[i] = try m.clone();
    }

    var buf: [256]u8 = undefined;
    var writer = std.Io.File.stdout().writer(init.io, &buf);

    for (0..20) |_| {
        try next_round(monkeys, true);
    }
    try writer.interface.print("Part 1: {}\n", .{try monkey_business(gpa, monkeys)});

    for (0..10000) |_| {
        try next_round(monkeys2, false);
    }
    try writer.interface.print("Part 2: {}\n", .{try monkey_business(gpa, monkeys2)});
    try writer.flush();
}
