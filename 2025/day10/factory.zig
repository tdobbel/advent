const std = @import("std");

const Requirements = std.array_list.Managed(u16);
const Machine = struct {
    ndigit: u5,
    diagram: u32,
    buttons: std.array_list.Managed([]u8),
    requirements: Requirements,
    pub fn free(self: *Machine) void {
        for (self.buttons.items) |v| {
            self.buttons.allocator.free(v);
        }
        self.buttons.deinit();
        self.requirements.deinit();
    }
};

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

pub fn parse_requirements(allocator: std.mem.Allocator, word: []const u8) !Requirements {
    var jolt = Requirements.init(allocator);
    var it = std.mem.tokenizeAny(u8, word[0 .. word.len - 1], ",");
    while (it.next()) |part| {
        const n = try std.fmt.parseInt(u16, part, 10);
        try jolt.append(n);
    }
    return jolt;
}

pub fn parse_line(allocator: std.mem.Allocator, line: []u8) !Machine {
    var it = std.mem.tokenizeAny(u8, line, " ");
    var diagram: u32 = undefined;
    var jolt: Requirements = undefined;
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
    return Machine{ .ndigit = ndigit, .diagram = diagram, .buttons = buttons, .requirements = jolt };
}

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

pub fn all_zeros(target: []u16) bool {
    for (target) |v| {
        if (v > 0) {
            return false;
        }
    }
    return true;
}

pub fn solve_part2(allocator: std.mem.Allocator, n: u16, best: *u16, buttons: []const []const u8, target: []u16) !void {
    if (all_zeros(target)) {
        if (n < best.*) {
            best.* = n;
        }
        return;
    }
    if (n > best.*) {
        return;
    }
    if (buttons.len == 0) return;
    var new_target = try allocator.alloc(u16, target.len);
    defer allocator.free(new_target);
    @memcpy(new_target, target);
    var nmax: u16 = std.math.maxInt(u16);
    for (buttons[0]) |i| {
        if (target[i] < nmax) {
            nmax = target[i];
        }
    }
    var start: u16 = 0;
    if (buttons.len == 1) {
        start = nmax;
    }
    var nmove: u16 = nmax + 1;
    while (nmove > start) {
        nmove -= 1;
        for (buttons[0]) |i| {
            new_target[i] = target[i] - nmove;
        }
        try solve_part2(allocator, n + nmove, best, buttons[1..], new_target);
    }
}

pub fn compare_buttons(_: void, a: []u8, b: []u8) bool {
    return a.len > b.len;
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
    var iline: usize = 0;

    while (try reader.interface.takeDelimiter('\n')) |line| {
        iline += 1;
        std.debug.print("line {}\n", .{iline});
        var machine = try parse_line(allocator, line);
        defer machine.free();
        var buttons: []u32 = try allocator.alloc(u32, machine.buttons.items.len);
        defer allocator.free(buttons);
        var sol1: u8 = std.math.maxInt(u8);
        std.mem.sort([]u8, machine.buttons.items, {}, compare_buttons);
        for (machine.buttons.items, 0..) |button, i| {
            buttons[i] = button_to_mask(button, machine.ndigit);
        }
        solve_part1(machine.diagram, buttons, 0, 0, &sol1);
        part1 += sol1;
        var sol2: u16 = std.math.maxInt(u16);
        try solve_part2(allocator, 0, &sol2, machine.buttons.items, machine.requirements.items);
        part2 += sol2;
    }
    std.debug.print("Part 1: {}\n", .{part1});
    std.debug.print("Part 2: {}\n", .{part2});
}
