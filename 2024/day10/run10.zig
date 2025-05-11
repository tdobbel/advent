const std = @import("std");

const allocator = std.heap.c_allocator;
const OutsideError = error{OutOfBounds};
const Direction = enum {
    Up,
    Down,
    Left,
    Right,
};

pub fn free_topo(topo: *std.ArrayList([]u8)) void {
    for (topo.items) |row| {
        allocator.free(row);
    }
    topo.deinit();
}

pub fn checked_sub(a: usize, b: usize) !usize {
    if (a < b) return OutsideError.OutOfBounds;
    return a - b;
}

pub fn get_sum(hm: *std.AutoHashMap(usize, usize)) usize {
    var sum: usize = 0;
    var it = hm.iterator();
    while (it.next()) |entry| {
        sum += entry.value_ptr.*;
    }
    return sum;
}

pub fn next_pos(x: usize, y: usize, dir: Direction, nx: usize, ny: usize) ![2]usize {
    var x_new = x;
    var y_new = y;
    switch (dir) {
        Direction.Up => y_new = try checked_sub(y, 1),
        Direction.Down => {
            if (y == ny - 1) return OutsideError.OutOfBounds;
            y_new += 1;
        },
        Direction.Left => x_new = try checked_sub(x, 1),
        Direction.Right => {
            if (x == nx - 1) return OutsideError.OutOfBounds;
            x_new += 1;
        },
    }
    return [2]usize{ x_new, y_new };
}

pub fn count_trails(topo: *std.ArrayList([]u8), index: usize, trails: *std.AutoHashMap(usize, usize)) !void {
    const nx = topo.items[0].len;
    const ny = topo.items.len;
    const x = index % nx;
    const y = index / nx;
    const value = topo.items[y][x];
    if (value == 9) {
        const entry = try trails.getOrPut(index);
        if (!entry.found_existing) {
            entry.value_ptr.* = 0;
        }
        entry.value_ptr.* += 1;
        return;
    }
    const directions = [_]Direction{
        Direction.Up,
        Direction.Down,
        Direction.Left,
        Direction.Right,
    };
    for (directions) |dir| {
        const xy_new = next_pos(x, y, dir, nx, ny) catch |err| {
            if (err == OutsideError.OutOfBounds) continue else return err;
        };
        const x_new = xy_new[0];
        const y_new = xy_new[1];
        if (topo.items[y_new][x_new] == value + 1) {
            try count_trails(topo, y_new * nx + x_new, trails);
        }
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

    var buffer: [1024]u8 = undefined;
    var buf_reader = std.io.bufferedReader(file.reader());
    var in_stream = buf_reader.reader();
    var topo = std.ArrayList([]u8).init(allocator);
    defer free_topo(&topo);
    var start_positions = std.ArrayList(usize).init(allocator);
    defer start_positions.deinit();
    while (try in_stream.readUntilDelimiterOrEof(&buffer, '\n')) |line| {
        var row: []u8 = try allocator.alloc(u8, line.len);
        for (line, 0..) |c, i| {
            row[i] = c - '0';
            if (row[i] == 0) {
                const index = topo.items.len * row.len + i;
                try start_positions.append(index);
            }
        }
        try topo.append(row);
    }

    var part1: usize = 0;
    var part2: usize = 0;
    for (start_positions.items) |start| {
        var trails = std.AutoHashMap(usize, usize).init(allocator);
        defer trails.deinit();
        try count_trails(&topo, start, &trails);
        part1 += trails.count();
        part2 += get_sum(&trails);
    }
    std.debug.print("Part 1: {}\n", .{part1});
    std.debug.print("Part 2: {}\n", .{part2});
}
