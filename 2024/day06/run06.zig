const std = @import("std");

const allocator = std.heap.c_allocator;

const Direction = enum(usize) {
    Up,
    Right,
    Down,
    Left,
};

pub fn turnRight(heading: Direction) Direction {
    switch (heading) {
        Direction.Up => return Direction.Right,
        Direction.Right => return Direction.Down,
        Direction.Down => return Direction.Left,
        Direction.Left => return Direction.Up,
    }
}

pub fn nextPosition(x: usize, y: usize, heading: Direction) struct { i32, i32 } {
    const ix: i32 = @intCast(x);
    const iy: i32 = @intCast(y);
    switch (heading) {
        Direction.Up => return .{ ix, iy - 1 },
        Direction.Right => return .{ ix + 1, iy },
        Direction.Down => return .{ ix, iy + 1 },
        Direction.Left => return .{ ix - 1, iy },
    }
}

pub fn moveGuard(x0: usize, y0: usize, obstacles: std.ArrayList([]bool), visited: []bool, history: [4][]bool) !bool {
    const ny = obstacles.items.len;
    const nx = obstacles.items[0].len;
    var x = x0;
    var y = y0;
    var x_next: usize = undefined;
    var y_next: usize = undefined;
    var heading = Direction.Up;
    while (true) {
        visited[y * nx + x] = true;
        const xy_next: [2]i32 = nextPosition(x, y, heading);
        if (xy_next[0] < 0 or xy_next[0] >= nx or xy_next[1] < 0 or xy_next[1] >= ny) {
            return false;
        }
        x_next = @intCast(xy_next[0]);
        y_next = @intCast(xy_next[1]);
        if (obstacles.items[y_next][x_next]) {
            heading = turnRight(heading);
        } else {
            x = x_next;
            y = y_next;
        }
        const d = @intFromEnum(heading);
        if (history[d][y * nx + x]) {
            return true;
        }
        history[d][y * nx + x] = true;
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

    var obstacles = std.ArrayList([]bool).init(allocator);
    defer obstacles.deinit();

    var buffer: [1024]u8 = undefined;
    var buf_reader = std.io.bufferedReader(file.reader());
    var in_stream = buf_reader.reader();

    var ny: usize = 0;
    var startX: usize = undefined;
    var startY: usize = undefined;
    while (try in_stream.readUntilDelimiterOrEof(&buffer, '\n')) |line| {
        var row: []bool = try allocator.alloc(bool, line.len);
        errdefer allocator.free(row);
        for (line, 0..) |c, i| {
            row[i] = c == '#';
            if (c == '^') {
                startX = i;
                startY = ny;
            }
        }
        try obstacles.append(row);
        ny += 1;
    }

    var history: [4][]bool = undefined;
    for (0..4) |i| {
        history[i] = try allocator.alloc(bool, ny * ny);
        errdefer allocator.free(history[i]);
        @memset(history[i], false);
    }

    const nx = obstacles.items[0].len;
    const visited: []bool = try allocator.alloc(bool, ny * nx);
    defer allocator.free(visited);
    @memset(visited, false);

    var part1: usize = 0;
    _ = try moveGuard(startX, startY, obstacles, visited, history);
    for (visited) |v| {
        if (v) {
            part1 += 1;
        }
    }
    std.debug.print("Part 1: {}\n", .{part1});
    var extra_walls: []usize = try allocator.alloc(usize, part1 - 1);
    defer allocator.free(extra_walls);

    var iw: usize = 0;
    for (visited, 0..) |v, ip| {
        if (v and ip != startY * nx + startX) {
            extra_walls[iw] = ip;
            iw += 1;
        } 
    }
    var part2: usize = 0;
    for (extra_walls) |pixel| {
        const x = pixel % nx;
        const y = pixel / ny;
        for (0..4) |d| {
            @memset(history[d], false);
        }
        obstacles.items[y][x] = true;
        if (try moveGuard(startX, startY, obstacles, visited, history)) {
            part2 += 1;
        }
        obstacles.items[y][x] = false;
    }
    std.debug.print("Part 2: {}\n", .{part2});

    for (history) |h| {
        allocator.free(h);
    }
    for (obstacles.items) |row| {
        allocator.free(row);
    }
}
