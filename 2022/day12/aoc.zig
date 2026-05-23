const std = @import("std");

const Puzzle = struct {
    grid: []u8,
    nx: usize,
    ny: usize,
    start_pos: [2]usize,
    end_pos: [2]usize,
};

pub fn solve_puzzle(puzzle: *const Puzzle, shortest: []usize, x: usize, y: usize, cost: usize) void {
    if (cost >= shortest[y * puzzle.nx + x]) return;
    shortest[y * puzzle.nx + x] = cost;
    if (x == puzzle.end_pos[0] and y == puzzle.end_pos[1]) {
        return;
    }
    const v = puzzle.grid[y * puzzle.nx + x];
    const ix: isize = @intCast(x);
    const iy: isize = @intCast(y);
    const shiftx: [4]isize = .{ 0, 1, 0, -1 };
    const shifty: [4]isize = .{ -1, 0, 1, 0 };
    const inx: isize = @intCast(puzzle.nx);
    const iny: isize = @intCast(puzzle.ny);
    for (shiftx, shifty) |dx, dy| {
        const x_ = ix + dx;
        const y_ = iy + dy;
        if (x_ < 0 or x_ >= inx or y_ < 0 or y_ >= iny) continue;
        const x_next: usize = @intCast(x_);
        const y_next: usize = @intCast(y_);
        const indx = puzzle.nx * y_next + x_next;
        const v_next = puzzle.grid[indx];
        if (v_next > v + 1) continue;
        solve_puzzle(puzzle, shortest, x_next, y_next, cost + 1);
    }
}

pub fn main(init: std.process.Init) !void {
    const args = try init.minimal.args.toSlice(init.arena.allocator());
    if (args.len != 2) {
        return error.InvalidArgument;
    }
    const io = init.io;
    const file = try std.Io.Dir.cwd().openFile(io, args[1], .{
        .mode = .read_only,
    });
    defer file.close(io);

    var buffer: [4096]u8 = undefined;
    var reader = file.reader(io, &buffer);

    var ny: usize = 0;
    var nx: usize = undefined;

    var start_x: usize = undefined;
    var start_y: usize = undefined;
    var end_x: usize = undefined;
    var end_y: usize = undefined;

    var arena = std.heap.ArenaAllocator.init(std.heap.c_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();

    var grid = std.array_list.Managed(u8).init(allocator);
    var candidates: std.array_list.Managed([2]usize) = .init(allocator);
    while (try reader.interface.takeDelimiter('\n')) |line| {
        nx = line.len;
        for (line, 0..) |c, x| {
            if (c == 'a') try candidates.append(.{ x, ny });
            if (c == 'S') {
                try grid.append('a');
                start_x = x;
                start_y = ny;
            } else if (c == 'E') {
                try grid.append('z');
                end_x = x;
                end_y = ny;
            } else {
                try grid.append(c);
            }
        }
        ny += 1;
    }
    var puzzle = Puzzle{ .grid = try grid.toOwnedSlice(), .nx = nx, .ny = ny, .start_pos = .{ start_x, start_y }, .end_pos = .{ end_x, end_y } };
    const shortest = try allocator.alloc(usize, nx * ny);
    @memset(shortest, nx * ny + 1);
    solve_puzzle(&puzzle, shortest, start_x, start_y, 0);

    const part1 = shortest[nx * end_y + end_x];
    std.debug.print("Part 1: {}\n", .{part1});

    var part2: usize = part1;
    for (candidates.items) |pos| {
        @memset(shortest, nx * ny + 1);
        solve_puzzle(&puzzle, shortest, pos[0], pos[1], 0);
        part2 = @min(part2, shortest[nx * end_y + end_x]);
    }
    std.debug.print("Part 2: {}\n", .{part2});
}
