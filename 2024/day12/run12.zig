const std = @import("std");

const allocator = std.heap.c_allocator;

const Direction = enum {
    Up,
    Down,
    Left,
    Right,
};

pub fn free_array_list(comptime T: anytype, array: *std.ArrayList([]T)) void {
    for (array.items) |row| {
        allocator.free(row);
    }
    array.deinit();
}

const Fence = struct {
    x: [2]usize,
    y: [2]usize,

    pub fn is_prev(self: *Fence, other: *Fence) bool {
        return self.x[1] == other.x[0] and self.y[1] == other.y[0];
    }

    pub fn as_vec(self: *Fence) [2]i32 {
        const ixa: i32 = @intCast(self.x[0]);
        const ixb: i32 = @intCast(self.x[1]);
        const iya: i32 = @intCast(self.y[0]);
        const iyb: i32 = @intCast(self.y[1]);
        return .{ ixb - ixa, iyb - iya };
    }

    pub fn is_corner(self: *Fence, other: *Fence) bool {
        const vecA = self.as_vec();
        const vecB = other.as_vec();
        return vecA[0] != vecB[0] or vecA[1] != vecB[1];
    }
};

const Plots = struct {
    nx: usize,
    ny: usize,
    plots: std.ArrayList([]u8),
    visited: std.ArrayList([]bool),

    pub fn next_pos(self: *Plots, x: usize, y: usize, dir: Direction) ?[2]usize {
        var x_new = x;
        var y_new = y;
        switch (dir) {
            .Up => {
                if (y == 0) return null;
                y_new -= 1;
            },
            .Down => {
                if (y == self.ny - 1) return null;
                y_new += 1;
            },
            .Left => {
                if (x == 0) return null;
                x_new -= 1;
            },
            .Right => {
                if (x == self.nx - 1) return null;
                x_new += 1;
            },
        }
        return .{ x_new, y_new };
    }

    pub fn free(self: *Plots) void {
        free_array_list(u8, &self.plots);
        free_array_list(bool, &self.visited);
    }
};

pub fn get_fence(x: usize, y: usize, dir: Direction) Fence {
    var xs: [2]usize = undefined;
    var ys: [2]usize = undefined;
    switch (dir) {
        .Up => {
            xs = .{ x + 1, x };
            ys = .{ y, y };
        },
        .Left => {
            xs = .{ x, x };
            ys = .{ y, y + 1 };
        },
        .Down => {
            xs = .{ x, x + 1 };
            ys = .{ y + 1, y + 1 };
        },
        .Right => {
            xs = .{ x + 1, x + 1 };
            ys = .{ y + 1, y };
        },
    }
    return Fence{ .x = xs, .y = ys };
}

pub fn count_corners(fences: *std.ArrayList(Fence)) usize {
    var ifirst: usize = 0;
    var icurr: usize = 0;
    var count: usize = 0;
    while (icurr < fences.items.len) {
        const curr = &fences.items[icurr];
        if (curr.is_prev(&fences.items[ifirst])) {
            if (curr.is_corner(&fences.items[ifirst])) {
                count += 1;
            }
            ifirst = icurr + 1;
            icurr = ifirst;
            continue;
        }
        for (icurr + 1..fences.items.len) |i| {
            if (curr.is_prev(&fences.items[i])) {
                if (curr.is_corner(&fences.items[i])) {
                    count += 1;
                }
                std.mem.swap(Fence, &fences.items[icurr + 1], &fences.items[i]);
                icurr += 1;
                break;
            }
        }
    }
    return count;
}

pub fn compute_price(plots: *Plots, x: usize, y: usize, area: *usize, perimeter: *usize, fences: *std.ArrayList(Fence)) !void {
    plots.visited.items[y][x] = true;
    area.* += 1;
    const directions = [_]Direction{ .Up, .Down, .Left, .Right };
    for (directions) |dir| {
        const xy_new = plots.next_pos(x, y, dir);
        if (xy_new == null) {
            perimeter.* += 1;
            try fences.append(get_fence(x, y, dir));
            continue;
        }
        const x_ = xy_new.?[0];
        const y_ = xy_new.?[1];
        if (plots.plots.items[y_][x_] != plots.plots.items[y][x]) {
            try fences.append(get_fence(x, y, dir));
            perimeter.* += 1;
        } else if (!plots.visited.items[y_][x_]) {
            try compute_price(plots, x_, y_, area, perimeter, fences);
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
    var grid = std.ArrayList([]u8).init(allocator);
    var visited = std.ArrayList([]bool).init(allocator);
    while (try in_stream.readUntilDelimiterOrEof(&buffer, '\n')) |line| {
        const grid_row: []u8 = try allocator.dupe(u8, line[0..line.len]);
        try grid.append(grid_row);
        const visited_row: []bool = try allocator.alloc(bool, line.len);
        @memset(visited_row, false);
        try visited.append(visited_row);
    }
    var plots = Plots{ .nx = grid.items[0].len, .ny = grid.items.len, .plots = grid, .visited = visited };
    defer plots.free();
    var part1: usize = 0;
    var part2: usize = 0;
    for (0..plots.ny) |y| {
        for (0..plots.nx) |x| {
            if (plots.visited.items[y][x]) continue;
            var fences = std.ArrayList(Fence).init(allocator);
            defer fences.deinit();
            var area: usize = 0;
            var perimeter: usize = 0;
            try compute_price(&plots, x, y, &area, &perimeter, &fences);
            part1 += area * perimeter;
            part2 += count_corners(&fences) * area;
        }
    }
    std.debug.print("Part 1: {}\n", .{part1});
    std.debug.print("Part 2: {}\n", .{part2});
}
