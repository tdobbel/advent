const std = @import("std");

const Operator = enum { Sum, Prod };

const MonkeyOperation = struct {
    operator: Operator,
    rhs: ?u32,

    pub fn apply(self: *const MonkeyOperation, old: u32) u32 {
        const rhs = self.rhs orelse old;
        return switch (self.operator) {
            .Sum => old + rhs,
            .Prod => old * rhs,
        };
    }
};

pub fn parse_last_number(line: []const u8) u32 {
    var factor: u32 = 1;
    var i: usize = line.len - 1;
    var v: u32 = 0;
    while (line[i] != ' ') : (i -= 1) {
        v += factor * @as(u32, line[i] - '0');
        factor *= 10;
    }
    return v;
}

const Monkey = struct {
    allocator: std.mem.Allocator,
    worry_levels: std.array_list.Managed(u32) = undefined,
    divider: u32 = undefined,
    monkey_true: usize = undefined,
    monkey_false: usize = undefined,
    operation: MonkeyOperation = undefined,
    counter: u32 = 0,

    pub fn apply_rule(self: *const Monkey, x: u32) usize {
        return if (x % self.divider == 0) self.monkey_true else self.monkey_false;
    }

    pub fn parse_operator(self: *Monkey, line: []const u8) !void {
        const i = std.mem.indexOf(u8, line, "old").?;
        const op: Operator = if (line[i + 4] == '+') .Sum else .Prod;
        const rhs_str = line[(i + 6)..];
        const rhs = if (std.mem.eql(u8, rhs_str, "old")) null else try std.fmt.parseInt(u32, rhs_str, 10);
        self.operation = MonkeyOperation{ .operator = op, .rhs = rhs };
    }

    pub fn parse_worry_levels(self: *Monkey, line: []const u8) !void {
        var i: usize = 0;
        while (line[i] != ':') : (i += 1) {}
        const nums: []const u8 = line[(i + 2)..];
        self.worry_levels = std.array_list.Managed(u32).init(self.allocator);
        var split_iter = std.mem.splitSequence(u8, nums, ", ");
        while (split_iter.next()) |item| {
            const v = try std.fmt.parseInt(u32, item, 10);
            try self.worry_levels.append(v);
        }
    }

    pub fn parse_line(self: *Monkey, flag: u8, line: []const u8) !void {
        try switch (flag) {
            0 => return,
            1 => self.parse_worry_levels(line),
            2 => self.parse_operator(line),
            3 => {
                self.divider = parse_last_number(line);
            },
            4 => {
                self.monkey_true = parse_last_number(line);
            },
            5 => {
                self.monkey_false = parse_last_number(line);
            },
            else => unreachable,
        };
    }
};

pub fn next_round(monkeys: []Monkey, divide_by_three: bool) !void {
    for (0..monkeys.len) |i| {
        var monkey = &monkeys[i];
        while (monkey.worry_levels.pop()) |old| {
            var w = monkey.operation.apply(old);
            if (divide_by_three) {
                w /= 3;
            }
            const indx = monkey.apply_rule(w);
            try monkeys[indx].worry_levels.append(w);
            monkey.counter += 1;
        }
    }
}

// pub fn print_monkeys(monkeys: []const Monkey) void {
//     for (monkeys, 0..) |monkey, i| {
//         std.debug.print("Monkey {}: {any}\n", .{ i, monkey.worry_levels.items });
//     }
// }

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

    var arena = std.heap.ArenaAllocator.init(std.heap.c_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();

    var cntr: u8 = 0;
    var monkey = Monkey{ .allocator = allocator };
    var monkey_list = std.array_list.Managed(Monkey).init(allocator);
    while (try reader.interface.takeDelimiter('\n')) |line| {
        if (line.len == 0) {
            cntr = 0;
            continue;
        }
        try monkey.parse_line(cntr, line);
        cntr += 1;
        if (cntr == 6) try monkey_list.append(monkey);
    }

    const monkeys: []Monkey = try monkey_list.toOwnedSlice();
    for (0..20) |_| {
        try next_round(monkeys, true);
    }
    var n_inspected: []u32 = try allocator.alloc(u32, monkeys.len);
    for (monkeys, 0..) |m, i| {
        n_inspected[i] = m.counter;
    }

    std.mem.sort(u32, n_inspected, {}, std.sort.desc(u32));
    std.debug.print("Part 1: {}", .{n_inspected[0] * n_inspected[1]});
}
