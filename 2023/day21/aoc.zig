const std = @import("std");

const Position = struct {
    x: usize,
    y: usize,
};

const Garden = struct {
    ny: usize,
    nx: usize,
    grid: []const u8,
};

const N_STEP = 64;

pub fn main(init: std.process.Init) !void {
    const argv = try init.minimal.args.toSlice(init.arena.allocator());
    if (argv.len < 2) {
        return error.MissingInputFile;
    }

    const file = try std.Io.Dir.cwd().openFile(init.io, argv[1], .{ .mode = .read_only });
    defer file.close(init.io);

    var buf: [4096]u8 = undefined;
    var reader = file.reader(init.io, &buf);

    var arena = std.heap.ArenaAllocator.init(std.heap.c_allocator);
    defer arena.deinit();

    const gpa = arena.allocator();
    var grid_vec: std.ArrayList(u8) = .empty;

    var ny: usize = 0;
    var nx: usize = undefined;
    var start: Position = undefined;
    while (try reader.interface.takeDelimiter('\n')) |line| : (ny += 1) {
        nx = line.len;
        for (line, 0..) |c, x| {
            if (c == 'S') {
                start.x = x;
                start.y = ny;
                try grid_vec.append(gpa, '.');
            } else {
                try grid_vec.append(gpa, c);
            }
        }
    }

    const garden = Garden{
        .ny = ny,
        .nx = nx,
        .grid = try grid_vec.toOwnedSlice(gpa),
    };

    var n_step = try gpa.alloc(usize, nx * ny);
    @memset(n_step, nx * ny);
    n_step[start.y * nx + start.x] = 0;
    compute_shortest(n_step, &garden, start);

    var part1: usize = 0;
    const mod2 = N_STEP % 2;
    for (n_step) |step| {
        if (step <= N_STEP and step % 2 == mod2) part1 += 1;
    }

    std.debug.print("Part 1: {}\n", .{part1});
}

pub fn compute_shortest(n_step: []usize, garden: *const Garden, pos: Position) void {
    var n: usize = 0;
    const prev = n_step[pos.y * garden.nx + pos.x];
    var next_pos: [4]Position = undefined;
    if (pos.y > 0) {
        next_pos[n] = Position{ .x = pos.x, .y = pos.y - 1 };
        n += 1;
    }
    if (pos.x < garden.nx - 1) {
        next_pos[n] = Position{ .x = pos.x + 1, .y = pos.y };
        n += 1;
    }
    if (pos.y < garden.ny - 1) {
        next_pos[n] = Position{ .x = pos.x, .y = pos.y + 1 };
        n += 1;
    }
    if (pos.x > 0) {
        next_pos[n] = Position{ .x = pos.x - 1, .y = pos.y };
        n += 1;
    }
    for (0..n) |i| {
        const idx = next_pos[i].y * garden.nx + next_pos[i].x;
        if (garden.grid[idx] == '#' or n_step[idx] <= prev + 1) {
            continue;
        }
        n_step[idx] = prev + 1;
        compute_shortest(n_step, garden, next_pos[i]);
    }
}
