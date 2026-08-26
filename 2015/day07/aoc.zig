const std = @import("std");

const Puzzle = struct {
    allocator: std.mem.Allocator,
    state: std.StringHashMap(u16),
    ops: [][]const u8,
    executed: []bool,

    pub fn init(allocator: std.mem.Allocator, ops: [][]const u8) !Puzzle {
        const executed = try allocator.alloc(bool, ops.len);
        @memset(executed, false);
        return Puzzle{ .allocator = allocator, .state = std.StringHashMap(u16).init(allocator), .ops = ops, .executed = executed };
    }

    pub fn parse_number(self: *Puzzle, str: []const u8) ?u16 {
        if (std.fmt.parseUnsigned(u16, str, 10)) |value| {
            return value;
        } else |_| {
            return self.state.get(str);
        }
    }

    pub fn apply_op(self: *Puzzle, op: []const u8) !bool {
        var parts: [5][]const u8 = undefined;
        var it = std.mem.splitScalar(u8, op, ' ');
        var n: usize = 0;
        while (it.next()) |p| : (n += 1) {
            parts[n] = p;
        }
        const dst = parts[n - 1];
        switch (n) {
            3 => {
                const value = self.parse_number(parts[0]) orelse return false;
                try self.state.put(dst, value);
                return true;
            },
            4 => {
                const value = self.parse_number(parts[1]) orelse return false;
                try self.state.put(dst, ~value);
                return true;
            },
            5 => {
                const x = self.parse_number(parts[0]) orelse return false;
                const kw = parts[1];
                if (std.mem.eql(u8, kw, "LSHIFT")) {
                    const n_shift: u4 = try std.fmt.parseUnsigned(u4, parts[2], 10);
                    try self.state.put(dst, x << n_shift);
                    return true;
                }
                if (std.mem.eql(u8, kw, "RSHIFT")) {
                    const n_shift: u4 = try std.fmt.parseUnsigned(u4, parts[2], 10);
                    try self.state.put(dst, x >> n_shift);
                    return true;
                }
                const y = self.parse_number(parts[2]) orelse return false;
                if (std.mem.eql(u8, kw, "OR")) {
                    try self.state.put(dst, x | y);
                    return true;
                }
                if (std.mem.eql(u8, kw, "AND")) {
                    try self.state.put(dst, x & y);
                    return true;
                }
                unreachable;
            },
            else => unreachable,
        }
    }
    pub fn solve(self: *Puzzle) !void {
        const n_ops = self.ops.len;
        var n_exec: usize = 0;
        for (self.executed) |ok| {
            if (ok) n_exec += 1;
        }
        while (n_exec < n_ops) {
            const prev = n_exec;
            for (self.ops, 0..) |op, i| {
                if (self.executed[i]) continue;
                if (try self.apply_op(op)) {
                    self.executed[i] = true;
                    n_exec += 1;
                }
            }
            if (n_exec == prev) return error.CouilleDansLePotage;
        }
    }
};

pub fn main(init: std.process.Init) !void {
    const args = try init.minimal.args.toSlice(init.arena.allocator());
    if (args.len < 2) return error.MissingInputFile;
    const file = try std.Io.Dir.cwd().openFile(init.io, args[1], .{ .mode = .read_only });
    defer file.close(init.io);

    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    const allocator = arena.allocator();

    const stat = try file.stat(init.io);
    const content: []u8 = try std.posix.mmap(null, @intCast(stat.size - 1), .{ .READ = true }, .{ .TYPE = .SHARED }, file.handle, 0);

    var lines: std.ArrayList([]const u8) = .empty;
    defer lines.deinit(allocator);
    var it = std.mem.splitScalar(u8, content, '\n');
    while (it.next()) |line| {
        try lines.append(allocator, line);
    }
    var puzzle = try Puzzle.init(allocator, lines.items);
    try puzzle.solve();

    var buf: [256]u8 = undefined;
    var writer = std.Io.File.stdout().writer(init.io, &buf);
    try writer.interface.print("Part 1: {}\n", .{puzzle.state.get("a").?});
    try writer.flush();
}
