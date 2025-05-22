const std = @import("std");
const Order = std.math.Order;

const allocator = std.heap.c_allocator;

const Direction = enum { Up, Down, Left, Right };

pub fn turn_left(direction: Direction) Direction {
    return switch (direction) {
        .Up => .Left,
        .Down => .Right,
        .Left => .Down,
        .Right => .Up,
    };
}

pub fn turn_right(direction: Direction) Direction {
    return switch (direction) {
        .Up => .Right,
        .Down => .Left,
        .Left => .Up,
        .Right => .Down,
    };
}

const State = struct {
    x: usize,
    y: usize,
    direction: Direction,
    counter: usize,
};

const Record = std.AutoHashMap(State, void);

const Crucible = struct {
    x: usize,
    y: usize,
    direction: Direction,
    counter: usize,
    total: usize,

    pub fn as_state(self: *const Crucible) State {
        return State {
            .x = self.x,
            .y = self.y,
            .direction = self.direction,
            .counter = self.counter
        };
    }
};

pub fn abs_diff(x: usize, y: usize) usize {
    return if (x > y) x - y else y - x;
}

const Grid = struct {
    nx: usize,
    ny: usize,
    blocks: std.ArrayList([]u8),

    pub fn free(self: *const Grid) void {
        for (self.blocks.items) |row| {
            allocator.free(row);
        }
        self.blocks.deinit();
    }

    pub fn distance_to_end(self: *const Grid, x: usize, y: usize) usize {
        return abs_diff(x, self.nx - 1) + abs_diff(y, self.ny - 1);
    }

    pub fn next_position(self: *const Grid, x: usize, y: usize, direction: Direction) ?[2]usize {
        var new_x: usize = x;
        var new_y: usize = y;
        switch (direction) {
            .Up => {
                if (y == 0) return null;
                new_y -= 1;
            },
            .Right => {
                if (x == self.nx - 1) return null;
                new_x += 1;
            },
            .Down => {
                if (y == self.ny - 1) return null;
                new_y += 1;
            },
            .Left => {
                if (x == 0) return null;
                new_x -= 1;
            },
        }
        return .{ new_x, new_y };
    }
};

pub fn lessThan(grid: *const Grid, a: Crucible, b: Crucible) Order {
    const distA = grid.distance_to_end(a.x, a.y);
    const scoreA = a.total + distA;
    const distB = grid.distance_to_end(b.x, b.y);
    const scoreB = b.total + distB;
    if (scoreA > scoreB) return Order.gt;
    if (scoreA < scoreB) return Order.lt;
    return Order.eq;
}

pub fn find_shortest_path(grid: *const Grid, min_block: usize, max_block: usize) !?usize {
    var queue = std.PriorityQueue(Crucible, *const Grid, lessThan).init(allocator, grid);
    defer queue.deinit();
    const start = Crucible{
        .x = 0,
        .y = 0,
        .direction = Direction.Right,
        .counter = 0,
        .total = 0,
    };
    try queue.add(start);
    var visited = Record.init(allocator);
    defer visited.deinit();
    try visited.put(start.as_state(), {});
    var iiter: usize = 0;
    while (queue.count() > 0) : (iiter += 1) {
        const current = queue.remove();
        if (grid.distance_to_end(current.x, current.y) == 0) {
            return current.total;
        }
        const directions = [_]Direction{
            current.direction,
            turn_left(current.direction),
            turn_right(current.direction),
        };
        for (directions) |dir| {
            if (dir != current.direction and current.counter > 0 and current.counter < min_block) {
                continue;
            }
            const counter = if (dir == current.direction) current.counter + 1 else 1;
            if (counter > max_block) {
                continue;
            }
            const xy = grid.next_position(current.x, current.y, dir);
            if (xy == null) {
                continue;
            }
            const x = xy.?[0];
            const y = xy.?[1];
            const heat_loss: usize = @intCast(grid.blocks.items[y][x]);
            const new_entry = Crucible{
                .x = x,
                .y = y,
                .direction = dir,
                .counter = counter,
                .total = current.total + heat_loss,
            };
            const new_state = new_entry.as_state();
            if (visited.contains(new_state)) {
                continue;
            }
            try visited.put(new_state, {});
            try queue.add(new_entry);
        }
    }
    return null;
}

pub fn main() !void {
    if (std.os.argv.len != 2) {
        return error.InvalidArgument;
    }
    const cwd = std.fs.cwd();
    const file_name: [:0]const u8 = std.mem.span(std.os.argv[1]);
    const file = try cwd.openFile(file_name, .{});
    defer file.close();

    var buffer: [5096]u8 = undefined;
    var buf_reader = std.io.bufferedReader(file.reader());
    var in_stream = buf_reader.reader();

    var blocks = std.ArrayList([]u8).init(allocator);

    var nx: usize = undefined;
    var ny: usize = 0;
    while (try in_stream.readUntilDelimiterOrEof(&buffer, '\n')) |line| {
        nx = line.len;
        var row: []u8 = try allocator.alloc(u8, nx);
        for (line, 0..) |c, i| {
            row[i] = c - '0';
        }
        try blocks.append(row);
        ny += 1;
    }
    const grid = Grid{ .nx = nx, .ny = ny, .blocks = blocks };
    defer grid.free();

    const part1 = try find_shortest_path(&grid, 1, 3);
    std.debug.print("Part 1: {}\n", .{part1.?});
    const part2 = try find_shortest_path(&grid, 4, 10);
    std.debug.print("Part 2: {}\n", .{part2.?});
}
