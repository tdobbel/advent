const std = @import("std");

const Present = struct {
    x: usize,
    y: usize,
    area: u8,
    shape: [3][3]u2,

    pub fn rotate(self: *Present) void {
        var rotated: [3][3]u2 = undefined;
        for (0..3) |i| {
            const u = @as(i8, @intCast(i)) - 1;
            const jr = 1 + u;
            for (0..3) |j| {
                const v = @as(i8, @intCast(j)) - 1;
                const ir = 1 - v;
                rotated[@intCast(ir)][@intCast(jr)] = self.shape[i][j];
            }
        }
        self.shape = rotated;
    }

    pub fn show(self: *const Present) void {
        for (self.shape) |row| {
            for (row) |v| {
                if (v == 0) {
                    std.debug.print(".", .{});
                } else {
                    std.debug.print("#", .{});
                }
            }
            std.debug.print("\n", .{});
        }
    }

    pub fn new(pattern: [3][3]u8) Present {
        var area: u8 = 0;
        var shape: [3][3]u2 = undefined;
        for (pattern, 0..) |row, i| {
            for (row, 0..) |c, j| {
                if (c == '#') {
                    shape[i][j] = 1;
                    area += 1;
                } else {
                    shape[i][j] = 0;
                }
            }
        }
        return Present{ .x = 0, .y = 0, .area = area, .shape = shape };
    }
};

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

    var n_shape: u3 = 6;
    var shape: [3][3]u8 = undefined;
    const allocator = std.heap.c_allocator;
    var presents = std.array_list.Managed(Present).init(allocator);
    defer presents.deinit();

    var part1: u16 = 0;
    var ny: u16 = undefined;
    var nx: u16 = undefined;
    var i: usize = 0;

    while (try reader.interface.takeDelimiter('\n')) |line| {
        if (line.len == 0 or line[line.len - 1] == ':') {
            continue;
        }
        if (n_shape > 0) {
            for (0..3) |j| {
                shape[i][j] = line[j];
            }
            i += 1;
            if (i == 3) {
                i = 0;
                n_shape -= 1;
                try presents.append(Present.new(shape));
            }
        } else {
            i = 0;
            nx = 0;
            while (line[i] != 'x') : (i += 1) {
                nx = nx * 10 + line[i] - '0';
            }
            ny = 0;
            i += 1;
            while (line[i] != ':') : (i += 1) {
                ny = ny * 10 + line[i] - '0';
            }
            var it = std.mem.tokenizeAny(u8, line[i + 2 ..], " ");
            var j: usize = 0;
            var queue: [6]u16 = undefined;
            var total_area: u16 = 0;
            while (it.next()) |slice| : (j += 1) {
                queue[j] = try std.fmt.parseInt(u16, slice, 10);
                total_area += presents.items[j].area * queue[j];
            }
            if (total_area <= nx * ny) {
                part1 += 1;
            }
        }
    }

    std.debug.print("Part 1: {}\n", .{part1});
}
