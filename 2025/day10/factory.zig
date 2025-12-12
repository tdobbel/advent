const std = @import("std");

const Requirements = std.array_list.Managed(u8);
const Machine = struct {
    diagram: u32,
    buttons: std.array_list.Managed(u32),
    requirements: Requirements,
    pub fn free(self: *Machine) void {
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

pub fn parse_button(word: []const u8, ndigit: u5) u32 {
    var res: u32 = 0;
    for (word) |w| {
        if (w == ',' or w == ')') {
            continue;
        }
        const n: u5 = @intCast(w - '0');
        res |= @as(u32, 1) << (ndigit - 1 - n);
    }
    return res;
}

pub fn parse_requirements(allocator: std.mem.Allocator, word: []const u8) !Requirements {
    var jolt = Requirements.init(allocator);
    var it = std.mem.tokenizeAny(u8, word[0 .. word.len - 1], ",");
    while (it.next()) |part| {
        const n = try std.fmt.parseInt(u8, part, 10);
        try jolt.append(n);
    }
    return jolt;
}

pub fn parse_line(allocator: std.mem.Allocator, line: []u8) !Machine {
    var it = std.mem.tokenizeAny(u8, line, " ");
    var diagram: u32 = undefined;
    var jolt: Requirements = undefined;
    var ndigit: u5 = 0;
    var buttons = std.array_list.Managed(u32).init(allocator);
    while (it.next()) |part| {
        switch (part[0]) {
            '[' => {
                diagram = parse_diagram(part[1..], &ndigit);
            },
            '(' => {
                try buttons.append(parse_button(part[1..], ndigit));
            },
            '{' => {
                jolt = try parse_requirements(allocator, part[1..]);
            },
            else => unreachable,
        }
    }
    return Machine{ .diagram = diagram, .buttons = buttons, .requirements = jolt };
}

pub fn find_min_combination(target: u32, buttons: []const u32, current: u32, cost: u8, best: *u8) void {
    if (current == target) {
        if (cost < best.*) {
            best.* = cost;
        }
        return;
    }
    if (cost > best.* or buttons.len == 0) return;
    find_min_combination(target, buttons[1..], current, cost, best);
    find_min_combination(target, buttons[1..], current ^ buttons[0], cost + 1, best);
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

    while (try reader.interface.takeDelimiter('\n')) |line| {
        var machine = try parse_line(allocator, line);
        var best: u8 = std.math.maxInt(u8);
        find_min_combination(machine.diagram, machine.buttons.items, 0, 0, &best);
        std.debug.print("{}\n", .{best});
        machine.free();
    }
}
