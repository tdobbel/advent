const std = @import("std");

const Cube = struct {
    xyz: [3]usize,
    neighbors: [6]?usize,

    pub fn manhattan_distance(self: *const Cube, other: *const Cube) usize {
        var d: usize = 0;
        for (0..3) |k| {
            d += if (self.xyz[k] > other.xyz[k]) self.xyz[k] - other.xyz[k] else other.xyz[k] - self.xyz[k];
        }
        return d;
    }
};

pub fn main(init: std.process.Init) !void {
    const argv = try init.minimal.args.toSlice(init.arena.allocator());
    if (argv.len < 2) {
        return error.MissingInputFile;
    }

    var file = try std.Io.Dir.cwd().openFile(init.io, argv[1], .{ .mode = .read_only });
    defer file.close(init.io);

    var arena = std.heap.ArenaAllocator.init(std.heap.c_allocator);
    defer arena.deinit();
    const gpa = arena.allocator();

    var buf: [64]u8 = undefined;
    var xyz: [3]usize = undefined;
    var reader = file.reader(init.io, &buf);

    var xmin: [3]usize = [_]usize{ std.math.maxInt(usize), std.math.maxInt(usize), std.math.maxInt(usize) };
    var xmax: [3]usize = [_]usize{ 0, 0, 0 };

    var cube_vec: std.ArrayList(Cube) = .empty;
    while (try reader.interface.takeDelimiter('\n')) |line| {
        var iter = std.mem.splitScalar(u8, line, ',');
        var i: usize = 0;
        while (iter.next()) |w| : (i += 1) {
            xyz[i] = try std.fmt.parseInt(usize, w, 10);
            xmin[i] = @min(xmin[i], xyz[i]);
            xmax[i] = @max(xmax[i], xyz[i]);
        }

        const cube = Cube{ .xyz = xyz, .neighbors = [_]?usize{ null, null, null, null, null, null } };
        try cube_vec.append(gpa, cube);
    }

    var nx: [3]usize = undefined;
    for (0..3) |k| {
        nx[k] = xmax[k] - xmin[k] + 3;
    }

    var grid: [][][]usize = try gpa.alloc([][]usize, nx[0]);
    for (0..nx[0]) |i| {
        grid[i] = try gpa.alloc([]usize, nx[1]);
        for (0..nx[1]) |j| {
            grid[i][j] = try gpa.alloc(usize, nx[2]);
            @memset(grid[i][j], 0);
        }
    }

    var writer = std.Io.File.stdout().writer(init.io, &buf);

    const cubes = try cube_vec.toOwnedSlice(gpa);
    // Make min pos start at 1 for each dimension
    for (0..cubes.len) |i| {
        for (0..3) |k| {
            cubes[i].xyz[k] = cubes[i].xyz[k] - xmin[k] + 1;
        }
        const ijk = cubes[i].xyz;
        grid[ijk[0]][ijk[1]][ijk[2]] = i + 2;
    }
    const part1 = solve_part1(cubes);
    try writer.interface.print("Part 1: {}\n", .{part1});

    find_exterior([_]usize{ 0, 0, 0 }, cubes, grid, nx);
    const part2 = part1 - count_null(cubes);
    try writer.interface.print("Part 2: {}\n", .{part2});

    try writer.flush();
}

pub fn count_null(cubes: []const Cube) usize {
    var total: usize = 0;
    for (cubes) |c| {
        for (c.neighbors) |n| {
            if (n == null) total += 1;
        }
    }
    return total;
}

pub fn solve_part1(cubes: []Cube) usize {
    // front back left right down top
    for (0..cubes.len - 1) |i| {
        var ci = &cubes[i];
        for (i + 1..cubes.len) |j| {
            var cj = &cubes[j];
            if (ci.manhattan_distance(cj) > 1) continue;
            for (0..3) |k| {
                if (ci.xyz[k] == cj.xyz[k] + 1) {
                    ci.neighbors[2 * k + 0] = j;
                    cj.neighbors[2 * k + 1] = i;
                    break;
                }
                if (cj.xyz[k] == ci.xyz[k] + 1) {
                    ci.neighbors[2 * k + 1] = j;
                    cj.neighbors[2 * k + 0] = i;
                    break;
                }
            }
        }
    }
    return count_null(cubes);
}

pub fn find_exterior(xyz: [3]usize, cubes: []Cube, grid: [][][]usize, nx: [3]usize) void {
    grid[xyz[0]][xyz[1]][xyz[2]] = 1;
    // back front right left top down
    var next: [6]?[3]usize = [_]?[3]usize{ null, null, null, null, null, null };
    for (0..3) |k| {
        if (xyz[k] < nx[k] - 1) {
            var ijk = xyz;
            ijk[k] += 1;
            next[2 * k + 0] = ijk;
        }
        if (xyz[k] > 0) {
            var ijk = xyz;
            ijk[k] -= 1;
            next[2 * k + 1] = ijk;
        }
    }
    for (0..6) |i| {
        const ijk = next[i] orelse continue;
        const index = grid[ijk[0]][ijk[1]][ijk[2]];
        if (index == 0) {
            find_exterior(ijk, cubes, grid, nx);
        } else if (index == 1) {
            continue;
        } else {
            var cube = &cubes[index - 2];
            cube.neighbors[i] = index - 2;
        }
    }
}
