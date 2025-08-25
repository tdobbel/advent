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

const Maze = struct {
    nx: usize,
    ny: usize,
    start: Position,
    end: Position,
    walls: *ArrayList([]bool),

    pub fn distance_to_end(self: *const Maze, pos: Position) usize {
        return abs_diff(pos.x, self.end.x) + abs_diff(pos.y, self.end.y);
    }
};

const Path = struct {
    score: usize,
    distance: usize,
    direction: Direction,
    positions: []Position,

    pub fn tail(self: *Path) Position {
        return self.positions[self.positions.len - 1];
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
        .direction = direction,
        .positions = positions,
    };
}

pub fn add_candidates(maze: *const Maze, path: *Path, visited: []bool, queue: *ArrayList(Path)) !void {
    const tail = path.tail();
    const directions = [_]Direction{
        path.direction,
        turn_left(path.direction),
        turn_right(path.direction),
    };
    const n = path.positions.len;
    for (directions) |dir| {
        const next_pos = tail.next(dir);
        const index = next_pos.y * maze.nx + next_pos.x;
        if (maze.walls.items[next_pos.y][next_pos.x] or visited[index]) {
            continue;
        }
        visited[index] = true;
        var new_score = path.score + 1;
        if (dir != path.direction) new_score += 1000;
        var positions = try allocator.alloc(Position, n + 1);
        for (path.positions, 0..) |pos, i| {
            positions[i] = pos;
        }
        positions[n] = next_pos;
        const new_path = Path{
            .score = new_score,
            .distance = maze.distance_to_end(next_pos),
            .direction = dir,
            .positions = positions,
        };
        try queue.append(new_path);
    }
}

pub fn free_queue(queue: *ArrayList(Path)) void {
    for (queue.items) |path| {
        allocator.free(path.positions);
    }
    queue.deinit();
}

pub fn solve_maze(maze: *const Maze, start_pos: Position, start_dir: Direction, start_score: usize, visited: []bool) !?Path {
    var queue = ArrayList(Path).init(allocator);
    defer free_queue(&queue);
    try queue.append(try create_path(
        start_score,
        maze.distance_to_end(start_pos),
        start_dir,
        start_pos,
    ));
    while (queue.items.len > 0) {
        var best = queue.pop().?;
        const pos = best.tail();
        if (std.meta.eql(pos, maze.end)) {
            return best;
        }
        try add_candidates(maze, &best, visited, &queue);
        defer allocator.free(best.positions);
        std.mem.sort(Path, queue.items, {}, struct {
            pub fn lessThan(_: void, a: Path, b: Path) bool {
                return (a.score + a.distance) > (b.score + b.distance);
            }
        }.lessThan);
    }
    return null;
}

pub fn solve_part1(maze: *const Maze) !?Path {
    const visited = try allocator.alloc(bool, maze.nx * maze.ny);
    defer allocator.free(visited);
    return solve_maze(maze, maze.start, Direction.Right, 0, visited);
}

pub fn add_positions(hash_set: *std.AutoHashMap(Position, void), path: *const Path) !void {
    for (path.positions) |p| {
        try hash_set.put(p, {});
    }
}

pub fn solve_part2(maze: *const Maze, ref: *const Path) !usize {
    const visited = try allocator.alloc(bool, maze.nx * maze.ny);
    defer allocator.free(visited);
    var start_pos = maze.start;
    var start_score: usize = 0;
    var start_dir = Direction.Right;

    var hash_set = std.AutoHashMap(Position, void).init(allocator);
    defer hash_set.deinit();
    try add_positions(&hash_set, ref);

    for (1..ref.positions.len - 1) |i| {
        for (0..i + 1) |j| {
            const pos = ref.positions[j];
            visited[pos.y * maze.nx + pos.x] = true;
        }
        const path = try solve_maze(maze, start_pos, start_dir, start_score, visited);
        if (path != null) {
            defer allocator.free(path.?.positions);
            if (path.?.score <= ref.score) {
                try add_positions(&hash_set, &path.?);
            }
        }
        @memset(visited, false);
        const next_pos = ref.positions[i];
        const directions = [_]Direction{
            start_dir,
            turn_left(start_dir),
            turn_right(start_dir),
        };
        for (directions) |dir| {
            const p = start_pos.next(dir);
            if (std.meta.eql(p, next_pos)) {
                start_pos = next_pos;
                start_score += if (dir == start_dir) 1 else 1001;
                start_dir = dir;
                break;
            }
        }
    }
    return hash_set.count();
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

    var walls = ArrayList([]bool).init(allocator);
    var maze = Maze{
        .nx = 0,
        .ny = 0,
        .start = undefined,
        .end = undefined,
        .walls = &walls,
    };
    while (reader.interface.takeDelimiterExclusive('\n')) |line| {
        var wall_row: []bool = try allocator.alloc(bool, line.len);
        maze.nx = line.len;
        for (line, 0..) |c, x| {
            wall_row[x] = c == '#';
            if (c == 'S') {
                maze.start = Position{ .x = x, .y = maze.ny };
            } else if (c == 'E') {
                maze.end = Position{ .x = x, .y = maze.ny };
            }
        }
        try maze.walls.append(wall_row);
        maze.ny += 1;
    } else |err| if (err != error.EndOfStream) {
        return err;
    }
    defer free_array_list(bool, &walls);
    const path = try solve_part1(&maze);
    defer allocator.free(path.?.positions);
    std.debug.print("Part 1: {}\n", .{path.?.score});
    const part2 = try solve_part2(&maze, &path.?);
    std.debug.print("Part 2: {}\n", .{part2});
}
