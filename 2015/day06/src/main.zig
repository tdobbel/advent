const std = @import("std");
const day06 = @import("day06");
const c = @import("c");

const Operation = enum { turn_on, turn_off, toggle };

const Instruction = struct { op: Operation, xa: usize, ya: usize, xb: usize, yb: usize };

const Regex = struct {
    inner: *c.regex_t,

    fn init(pattern: [:0]const u8) !Regex {
        const inner = c.alloc_regex_t().?;
        if (0 != c.regcomp(inner, pattern, c.REG_NEWLINE | c.REG_EXTENDED)) {
            return error.RegexCompilation;
        }

        return .{
            .inner = inner,
        };
    }

    fn deinit(self: *Regex) void {
        c.free_regex_t(self.inner);
    }

    fn match(self: *Regex, input: [:0]const u8) !Instruction {
        const match_size = 6;
        var pmatch: [match_size]c.regmatch_t = undefined;
        if (c.regexec(self.inner, input, match_size, &pmatch, 0) != 0) return error.NoMatch;
        const op_slice = get_group(pmatch[1], input);
        var op: Operation = undefined;
        if (std.mem.eql(u8, op_slice, "turn off")) {
            op = .turn_off;
        } else if (std.mem.eql(u8, op_slice, "turn on")) {
            op = .turn_on;
        } else {
            op = .toggle;
        }
        const xa = try std.fmt.parseUnsigned(usize, get_group(pmatch[2], input), 10);
        const ya = try std.fmt.parseUnsigned(usize, get_group(pmatch[3], input), 10);
        const xb = try std.fmt.parseUnsigned(usize, get_group(pmatch[4], input), 10);
        const yb = try std.fmt.parseUnsigned(usize, get_group(pmatch[5], input), 10);
        return Instruction{ .op = op, .xa = xa, .xb = xb, .ya = ya, .yb = yb };
    }
};

pub fn get_group(pmatch: c.regmatch_t, input: []const u8) []const u8 {
    const start: usize = @intCast(pmatch.rm_so);
    const end: usize = @intCast(pmatch.rm_eo);
    return input[start..end];
}

pub fn main(init: std.process.Init) !void {

    const args = try init.minimal.args.toSlice(init.arena.allocator());
    if (args.len < 2) return error.MissingInputFile;

    var regex = try Regex.init("(turn on|toggle|turn off) ([0-9]+),([0-9]+) through ([0-9]+),([0-9]+)");
    defer regex.deinit();

    const file = try std.Io.Dir.cwd().openFile(init.io, args[1], .{ .mode = .read_only });
    defer file.close(init.io);

    var grid: [1000][1000]u1 = undefined;
    var grid2: [1000][1000]u32 = undefined;
    for (0..1000) |i| {
        @memset(grid[i][0..], 0);
        @memset(grid2[i][0..], 0);
    }
    var buf: [256]u8 = undefined;
    var c_string: [257:0]u8 = undefined;
    @memset(c_string[0..], 0);
    var reader = file.reader(init.io, &buf);
    while (try reader.interface.takeDelimiter('\n')) |line| {
        @memcpy(c_string[0..line.len], line);
        c_string[line.len] = 0;
        const cmd = try regex.match(&c_string);
        for (cmd.ya..cmd.yb + 1) |y| {
            for (cmd.xa..cmd.xb + 1) |x| {
                switch (cmd.op) {
                    .toggle => {
                        grid[y][x] ^= 1;
                        grid2[y][x] += 2;
                    },
                    .turn_off => {
                        grid[y][x] = 0;
                        if (grid2[y][x] > 0) grid2[y][x] -= 1;
                    },
                    .turn_on => {
                        grid[y][x] = 1;
                        grid2[y][x] += 1;
                    },
                }
            }
        }
    }

    var part1: u32 = 0;
    var part2: u64 = 0;
    for (0..1000) |y| {
        for (0..1000) |x| {
            part1 += @intCast(grid[y][x]);
            part2 += @intCast(grid2[y][x]);
        }
    }

    var writer = std.Io.File.stdout().writer(init.io, &buf);
    try writer.interface.print("Part 1: {}\n", .{part1});
    try writer.interface.print("Part 2: {}\n", .{part2});
    try writer.flush();
}
