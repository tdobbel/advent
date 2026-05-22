const std = @import("std");

const Operator = enum { Sum, Prod };
const Number = std.hash_map.AutoHashMap(u64, u64);
const PrimeError = error{NotDivisibleError};

pub fn prime_decomposition(allocator: std.mem.Allocator, num: u64) !Number {
    std.debug.print("Prime decomposition started for {}\n", .{num});
    var x = num;
    var div: u64 = 2;
    var out: Number = .init(allocator);
    while (x > 1 and div * div <= num) : (div += 1) {
        if (x % div != 0) continue;
        x /= div;
        var p: u64 = 1;
        while (x % div == 0) {
            x /= div;
            p += 1;
        }
        try out.put(div, p);
    }
    if (x > 1) {
        try out.put(x, 1);
    }
    std.debug.print("Prime decomposition ended\n", .{});
    return out;
}

pub fn prime_product(num: *Number, factor: Number) !void {
    var iter = factor.iterator();
    while (iter.next()) |entry| {
        const key = entry.key_ptr.*;
        const value = entry.value_ptr.*;
        if (num.contains(key)) {
            try num.put(key, value + num.get(key).?);
        } else {
            try num.put(key, value);
        }
    }
}

pub fn prime_divide(num: *Number, divider: Number) !void {
    var iter = divider.iterator();
    while (iter.next()) |div_entry| {
        const prime_factor = div_entry.key_ptr.*;
        const p1 = div_entry.value_ptr.*;
        const p2 = num.get(prime_factor) orelse return PrimeError.NotDivisibleError;
        if (p2 < p1) {
            return PrimeError.NotDivisibleError;
        } else if (p2 == p1) {
            _ = num.remove(prime_factor);
        } else {
            try num.put(prime_factor, p2 - p1);
        }
    }
}

pub fn prime_sum(allocator: std.mem.Allocator, num: *Number, other: Number) !void {
    var gcd: Number = .init(allocator);
    var iter = other.iterator();
    while (iter.next()) |entry| {
        const key = entry.key_ptr.*;
        const v1 = entry.value_ptr.*;
        const v2 = num.get(key) orelse continue;
        try gcd.put(key, @min(v1, v2));
    }
    var other2 = try other.clone();
    try prime_divide(num, gcd);
    try prime_divide(&other2, gcd);
    const n = to_number(num) + to_number(&other2);
    num.deinit();
    other2.deinit();
    other2 = try prime_decomposition(allocator, n);
    try prime_product(&gcd, other2);
    other2.deinit();
    num.* = gcd;
}

pub fn to_number(num: *const Number) u64 {
    var res: u64 = 1;
    var iter = num.iterator();
    while (iter.next()) |entry| {
        const prime = entry.key_ptr.*;
        const exp = entry.value_ptr.*;
        res *= std.math.pow(u64, prime, exp);
    }
    return res;
}

pub fn divide_by_three(allocator: std.mem.Allocator, num: *Number) !void {
    const n = to_number(num) / 3;
    const new_num = try prime_decomposition(allocator, n);
    num.deinit();
    num.* = new_num;
}

const MonkeyOperation = struct {
    operator: Operator,
    rhs: ?Number,

    pub fn apply(self: *const MonkeyOperation, allocator: std.mem.Allocator, lhs: *Number) !void {
        const rhs = self.rhs orelse lhs.*;
        return switch (self.operator) {
            .Sum => try prime_sum(allocator, lhs, rhs),
            .Prod => try prime_product(lhs, rhs),
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
    worry_levels: std.array_list.Managed(Number) = undefined,
    divider: u64 = undefined,
    monkey_true: usize = undefined,
    monkey_false: usize = undefined,
    operation: MonkeyOperation = undefined,
    counter: u32 = 0,

    pub fn apply_rule(self: *const Monkey, x: Number) usize {
        return if (x.contains(self.divider)) self.monkey_true else self.monkey_false;
    }

    pub fn parse_operator(self: *Monkey, allocator: std.mem.Allocator, line: []const u8) !void {
        const i = std.mem.indexOf(u8, line, "old").?;
        const op: Operator = if (line[i + 4] == '+') .Sum else .Prod;
        const rhs_str = line[(i + 6)..];
        var rhs: ?Number = null;
        if (!std.mem.eql(u8, rhs_str, "old")) {
            const n = try std.fmt.parseInt(u64, rhs_str, 10);
            rhs = try prime_decomposition(allocator, n);
        }
        self.operation = MonkeyOperation{ .operator = op, .rhs = rhs };
    }

    pub fn parse_worry_levels(self: *Monkey, allocator: std.mem.Allocator, line: []const u8) !void {
        var i: usize = 0;
        while (line[i] != ':') : (i += 1) {}
        const nums: []const u8 = line[(i + 2)..];
        self.worry_levels = std.array_list.Managed(Number).init(allocator);
        var split_iter = std.mem.splitSequence(u8, nums, ", ");
        while (split_iter.next()) |item| {
            const v = try std.fmt.parseInt(u64, item, 10);
            try self.worry_levels.append(try prime_decomposition(allocator, v));
        }
    }

    pub fn clone(self: *const Monkey, allocator: std.mem.Allocator) !Monkey {
        var monkey: Monkey = Monkey{ .divider = self.divider, .monkey_true = self.monkey_true, .monkey_false = self.monkey_false, .operation = self.operation, .counter = 0 };
        monkey.worry_levels = std.array_list.Managed(Number).init(allocator);
        for (self.worry_levels.items) |item| {
            try monkey.worry_levels.append(try item.clone());
        }
        return monkey;
    }

    pub fn parse_line(self: *Monkey, allocator: std.mem.Allocator, flag: u8, line: []const u8) !void {
        try switch (flag) {
            0 => return,
            1 => self.parse_worry_levels(allocator, line),
            2 => self.parse_operator(allocator, line),
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

pub fn print_prime(num: Number) void {
    var iter = num.iterator();
    std.debug.print("(", .{});
    while (iter.next()) |entry| {
        std.debug.print("{} => {}, ", .{ entry.key_ptr.*, entry.value_ptr.* });
    }
    std.debug.print(")\n", .{});
}

pub fn next_round(allocator: std.mem.Allocator, monkeys: []Monkey, div_by_3: bool) !void {
    for (0..monkeys.len) |i| {
        std.debug.print("Monkey {}\n", .{i});
        var monkey = &monkeys[i];
        while (monkey.worry_levels.pop()) |item| {
            var wlvl = item;
            std.debug.print("  Worry level is ", .{});
            print_prime(wlvl);
            switch (monkey.operation.operator) {
                .Sum => std.debug.print("  sum with", .{}),
                .Prod => std.debug.print("  product with", .{}),
            }
            if (monkey.operation.rhs) |num| {
                print_prime(num);
            } else {
                std.debug.print("self\n", .{});
            }
            try monkey.operation.apply(allocator, &wlvl);
            if (div_by_3) {
                try divide_by_three(allocator, &wlvl);
            }
            const indx = monkey.apply_rule(wlvl);
            try monkeys[indx].worry_levels.append(wlvl);
            monkey.counter += 1;
        }
    }
}

pub fn monkey_business(allocator: std.mem.Allocator, monkeys: []const Monkey) !u32 {
    var n_inspected: []u32 = try allocator.alloc(u32, monkeys.len);
    for (monkeys, 0..) |m, i| {
        n_inspected[i] = m.counter;
    }
    std.mem.sort(u32, n_inspected, {}, std.sort.desc(u32));
    const v: u32 = n_inspected[0] * n_inspected[1];
    allocator.free(n_inspected);
    return v;
}

pub fn print_monkeys(monkeys: []const Monkey) void {
    for (monkeys, 0..) |monkey, i| {
        std.debug.print("Monkey {}\n",.{ i, monkey.worry_levels.items });
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

    var arena = std.heap.ArenaAllocator.init(std.heap.c_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();

    var cntr: u8 = 0;
    var monkey = Monkey{};
    var monkey_list = std.array_list.Managed(Monkey).init(allocator);
    while (try reader.interface.takeDelimiter('\n')) |line| {
        if (line.len == 0) {
            cntr = 0;
            continue;
        }
        try monkey.parse_line(allocator, cntr, line);
        cntr += 1;
        if (cntr == 6) try monkey_list.append(monkey);
    }

    const monkeys: []Monkey = try monkey_list.toOwnedSlice();
    var monkeys2 = try allocator.alloc(Monkey, monkeys.len);
    for (monkeys, 0..) |m, i| {
        monkeys2[i] = try m.clone(allocator);
    }
    for (0..20) |_| {
        try next_round(allocator, monkeys, true);
    }
    std.debug.print("Part 1: {}\n", .{try monkey_business(allocator, monkeys)});


    for (0..20) |_| {
        try next_round(allocator, monkeys2, false);
    }
    // std.debug.print("Part 2: {}", .{try monkey_business(allocator, monkeys2)});
}
