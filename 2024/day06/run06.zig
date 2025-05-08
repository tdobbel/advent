const std = @import("std");

const PositionError = error{OutsideBounds};

const Direction = enum {
    Up,
    Down,
    Left,
    Right,
};

const Position = struct { x: usize, y: usize, direction: Direction };

pub fn turnRight(pos: *Position) void {
    switch (pos.direction) {
        Direction.Up => pos.direction = Direction.Right,
        Direction.Right => pos.direction = Direction.Down,
        Direction.Down => pos.direction = Direction.Left,
        Direction.Left => pos.direction = Direction.Up,
    }
}

pub fn substract(x: usize, y: usize) !usize {
    if (y > x) return error.OutsideBounds;
    return x - y;
}

pub fn nextPosition(pos: *Position, nx: usize, ny: usize) PositionError!struct { usize, usize } {
    var x = pos.x;
    var y = pos.y;
    switch (pos.direction) {
        Direction.Up => y = try substract(pos.y, 1),
        Direction.Down => y = pos.y + 1,
        Direction.Left => x = try substract(pos.x, 1),
        Direction.Right => x = pos.x + 1,
    }
    if (x >= nx or y >= ny) {
        return error.OutsideBounds;
    }
    return .{ x, y };
}

pub fn moveGuard(guard: *Position, obstacles: std.ArrayList([]bool), visited: []bool, nVisited: *usize) !bool {
    const ny = obstacles.items.len;
    const nx = obstacles.items[0].len;
    var history = std.AutoHashMap(Position, void).init(std.heap.page_allocator);
    defer history.deinit();
    nVisited.* = 0;
    while (true) {
        if (history.contains(guard.*)) return true;
        try history.put(guard.*, {});
        visited[guard.y * nx + guard.x] = true;
        const pnext: [2]usize = nextPosition(guard, nx, ny) catch |err| {
            if (err == PositionError.OutsideBounds) {
                for (visited) |v| {
                    if (v) nVisited.* += 1;
                }
                return false;
            }
            return err;
        };
        const x = pnext[0];
        const y = pnext[1];
        if (obstacles.items[y][x]) {
            turnRight(guard);
        } else {
            guard.x = x;
            guard.y = y;
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

    const allocator = std.heap.page_allocator;
    var obstacles = std.ArrayList([]bool).init(allocator);
    defer obstacles.deinit();

    var buffer: [1024]u8 = undefined;
    var buf_reader = std.io.bufferedReader(file.reader());
    var in_stream = buf_reader.reader();
    var ny: usize = 0;
    // var guard: Position = undefined;
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
    const nx = obstacles.items[0].len;
    const visited: []bool = try allocator.alloc(bool, ny * ny);
    defer allocator.free(visited);
    var part1: usize = 0;
    var guard = Position{ .x = startX, .y = startY, .direction = Direction.Up };
    _ = try moveGuard(&guard, obstacles, visited, &part1);
    std.debug.print("Part 1: {}\n", .{part1});

    const n = part1;
    const candidates: []usize = try allocator.alloc(usize, n * 2);
    defer allocator.free(candidates);
    {
        var i: usize = 0;
        for (visited, 0..) |v, pos| {
            if (!v) continue;
            candidates[2 * i + 0] = pos % nx;
            candidates[2 * i + 1] = pos / nx;
            i += 1;
        }
    }

    var part2: usize = 0;
    for (0..n) |i| {
        const x = candidates[2 * i + 0];
        const y = candidates[2 * i + 1];
        if (x == startX and y == startY) continue;
        @memset(visited, false);
        obstacles.items[y][x] = true;
        guard = Position{ .x = startX, .y = startY, .direction = Direction.Up };
        if (try moveGuard(&guard, obstacles, visited, &part1)) {
            part2 += 1;
        }
        obstacles.items[y][x] = false;
    }
    std.debug.print("Part 2: {}\n", .{part2});
}
