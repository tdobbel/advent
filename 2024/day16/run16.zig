const std = @import("std");

const allocator = std.heap.c_allocator;
const ArrayList = std.array_list.Managed;

const Direction = enum {
    Up,
    Down,
    Left,
    Right,
};

pub fn free_array_list(comptime T: anytype, array: *ArrayList([]T)) void {
    for (array.items) |row| {
        allocator.free(row);
    }
    array.deinit();
}

pub fn create_boolean_2d_array(nx: usize, ny: usize) ![][]bool {
    var array = try allocator.alloc([]bool, nx);
    for (0..ny) |i| {
        array[i] = try allocator.alloc(bool, nx);
        @memset(array[i], false);
    }
    return array;
}

pub fn free_2d_array(comptime T: anytype, array: [][]T) void {
    for (array) |row| {
        allocator.free(row);
    }
    allocator.free(array);
}

pub fn turn_right(dir: Direction) Direction {
    return switch (dir) {
        .Up => .Right,
        .Right => .Down,
        .Down => .Left,
        .Left => .Up,
    };
}

pub fn turn_left(dir: Direction) Direction {
    return switch (dir) {
        .Up => .Left,
        .Left => .Down,
        .Down => .Right,
        .Right => .Up,
    };
}

const Position = struct {
    x: usize,
    y: usize,

    pub fn next(self: *const Position, dir: Direction) Position {
        return switch (dir) {
            .Up => Position{ .x = self.x, .y = self.y - 1 },
            .Down => Position{ .x = self.x, .y = self.y + 1 },
            .Left => Position{ .x = self.x - 1, .y = self.y },
            .Right => Position{ .x = self.x + 1, .y = self.y },
        };
    }
};

pub fn abs_diff(a: usize, b: usize) usize {
    return if (a > b) a - b else b - a;
}

pub fn direction_from_positions(a: Position, b: Position) Direction {
    if (a.x < b.x) return .Right;
    if (a.x > b.x) return .Left;
    if (a.y < b.y) return .Down;
    return .Up;
}

const Path = struct {
    score: usize,
    distance: usize,
    heading: Direction,
    positions: []Position,

    pub fn last_position(self: *const Path) Position {
        return self.positions[self.positions.len - 1];
    }

    pub fn new_positions(self: *const Path, extra_pos: Position) ![]Position {
        const n = self.positions.len;
        var new_pos = try allocator.alloc(Position, n + 1);
        for (self.positions, 0..) |pos, i| {
            new_pos[i] = pos;
        }
        new_pos[n] = extra_pos;
        return new_pos;
    }

    pub fn get_size(self: *const Path) usize {
        return self.positions.len;
    }

    pub fn get_cost(self: *const Path) usize {
        return self.score + self.distance;
    }
};

pub fn create_path(
    score: usize,
    distance: usize,
    direction: Direction,
    start_pos: Position,
) !Path {
    var positions = try allocator.alloc(Position, 1);
    positions[0] = start_pos;
    return Path{
        .score = score,
        .distance = distance,
        .heading = direction,
        .positions = positions,
    };
}

pub fn free_queue(queue: *ArrayList(Path)) void {
    for (queue.items) |path| {
        allocator.free(path.positions);
    }
    queue.deinit();
}

pub fn get_distance(a: Position, b: Position) usize {
    return abs_diff(a.x, b.x) + abs_diff(a.y, b.y);
}

pub fn find_shortest_path(start_pos: Position, end_pos: Position, heading: Direction, start_score: usize, is_wall: [][]bool, max_score: ?usize) !?Path {
    var queue = ArrayList(Path).init(allocator);
    defer free_queue(&queue);
    var visited = try create_boolean_2d_array(is_wall[0].len, is_wall.len);
    defer free_2d_array(bool, visited);
    try queue.append(try create_path(
        start_score,
        get_distance(start_pos, end_pos),
        heading,
        start_pos,
    ));

    while (queue.items.len > 0) {
        var min_index: usize = 0;
        var min_cost: usize = queue.items[0].get_cost();
        for (1..queue.items.len) |i| {
            const cost = queue.items[i].get_cost();
            if (cost < min_cost) {
                min_cost = cost;
                min_index = i;
            }
        }
        var shortest = queue.orderedRemove(min_index);
        const pos = shortest.last_position();
        if (std.meta.eql(pos, end_pos)) {
            return shortest;
        }
        defer allocator.free(shortest.positions);
        if (max_score != null and shortest.score > max_score.?) {
            return null;
        }
        const directions = [_]Direction{
            shortest.heading,
            turn_left(shortest.heading),
            turn_right(shortest.heading),
        };
        for (directions) |new_heading| {
            const next_pos = pos.next(new_heading);
            if (is_wall[next_pos.y][next_pos.x] or visited[next_pos.y][next_pos.x]) {
                continue;
            }
            visited[next_pos.y][next_pos.x] = true;
            var new_score = shortest.score + 1;
            if (new_heading != shortest.heading) new_score += 1000;
            const new_path = Path{
                .score = new_score,
                .distance = get_distance(next_pos, end_pos),
                .heading = new_heading,
                .positions = try shortest.new_positions(next_pos),
            };
            try queue.append(new_path);
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

    var buffer: [1024]u8 = undefined;
    var reader = file.reader(&buffer);

    var is_wall = ArrayList([]bool).init(allocator);
    var start_pos: Position = undefined;
    var end_pos: Position = undefined;
    var ny: usize = 0;
    while (reader.interface.takeDelimiterExclusive('\n')) |line| {
        var wall_row: []bool = try allocator.alloc(bool, line.len);
        for (line, 0..) |c, x| {
            wall_row[x] = c == '#';
            if (c == 'S') {
                start_pos = Position{ .x = x, .y = ny };
            } else if (c == 'E') {
                end_pos = Position{ .x = x, .y = ny };
            }
        }
        try is_wall.append(wall_row);
        ny += 1;
    } else |err| if (err != error.EndOfStream) {
        return err;
    }
    defer free_array_list(bool, &is_wall);


    const shortest_ = try find_shortest_path(start_pos, end_pos, .Right, 0, is_wall.items, null);
    const shortest = shortest_.?;
    defer allocator.free(shortest.positions);
    std.debug.print("Part 1: {}\n", .{shortest.score});

    const min_score = shortest.score;
    var start_score: usize = 0;
    var start = start_pos;
    var heading: Direction = .Right;
    const nx = is_wall.items[0].len;
    var is_shortest = try create_boolean_2d_array(nx, ny);
    defer free_2d_array(bool, is_shortest);
    for (shortest.positions) |p| {
        is_shortest[p.y][p.x] = true;
    }
    for (0..shortest.get_size() - 2) |i| {
        const pos = shortest.positions[i + 1];
        is_wall.items[pos.y][pos.x] = true;
        const path = try find_shortest_path(start, end_pos, heading, start_score, is_wall.items, min_score); 
        if (path != null) {
            for (path.?.positions) |p| {
                is_shortest[p.y][p.x] = true;
            }
        }
        is_wall.items[start.y][start.x] = false;
        start_score += 1;
        const new_dir = direction_from_positions(start, pos);
        if (new_dir != heading) {
            start_score += 1000;
            heading = new_dir;
        }
        start = pos;
    }
    var part2: usize = 0;
    for (0..ny) |y| {
        for (0..nx) |x| {
            part2 += if (is_shortest[y][x]) 1 else 0;
        }
    }
    std.debug.print("Part 2: {}\n", .{part2});
}
