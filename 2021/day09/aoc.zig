const std = @import("std");

const Puzzle = struct {
    grid: []u8,
    nx: usize,
    ny: usize,

    pub fn get(self: *const Puzzle, x: usize, y: usize) u8 {
        return self.grid[y * self.nx + x];
    }

    pub fn neighbors(self: *const Puzzle, x: usize, y: usize) [4]?[2]usize {
        var out: [4]?[2]usize = .{ null, null, null, null };
        const shiftx: [4]isize = .{ 0, 1, 0, -1 };
        const shifty: [4]isize = .{ -1, 0, 1, 0 };
        const ix: isize = @intCast(x);
        const iy: isize = @intCast(y);
        for (shiftx, shifty, 0..) |dx, dy, i| {
            const ixn = ix + dx;
            const iyn = iy + dy;
            if (ixn < 0 or iyn < 0) continue;
            const xn: usize = @intCast(ixn);
            const yn: usize = @intCast(iyn);
            if (xn >= self.nx or yn >= self.ny) continue;
            out[i] = .{ xn, yn };
        }
        return out;
    }
};

pub fn count_basin(puzzle: Puzzle, x: usize, y: usize, visited: []bool, size: *usize) void {
    size.* += 1;
    visited[y * puzzle.nx + x] = true;
    const level = puzzle.get(x, y);
    for (puzzle.neighbors(x, y)) |pos| {
        const xy = pos orelse continue;
        const other = puzzle.get(xy[0], xy[1]);
        if (!visited[xy[1] * puzzle.nx + xy[0]] and other > level and other < 9) {
            count_basin(puzzle, xy[0], xy[1], visited, size);
        }
    }
}

pub fn solve_part1(puzzle: Puzzle, visited: []bool, basins: *std.array_list.Managed(usize)) !u32 {
    var cntr: u32 = 0;
    for (0..puzzle.ny) |y| {
        for (0..puzzle.nx) |x| {
            const level = puzzle.get(x, y);
            var low_point: bool = true;
            for (puzzle.neighbors(x, y)) |pos| {
                const xy = pos orelse continue;
                if (puzzle.get(xy[0], xy[1]) <= level) {
                    low_point = false;
                    break;
                }
            }
            if (low_point) {
                @memset(visited, false);
                var basin_size: usize = 0;
                count_basin(puzzle, x, y, visited, &basin_size);
                try basins.append(basin_size);
                cntr += @intCast(level + 1);
            }
        }
    }
    return cntr;
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

    var arena = std.heap.ArenaAllocator.init(std.heap.c_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();

    var grid_array = std.array_list.Managed(u8).init(allocator);
    while (try reader.interface.takeDelimiter('\n')) |line| {
        nx = line.len;
        for (line) |v| {
            try grid_array.append(v - '0');
        }
        ny += 1;
    }
    const puzzle = Puzzle{ .grid = try grid_array.toOwnedSlice(), .nx = nx, .ny = ny };
    const visited = try allocator.alloc(bool, nx * ny);
    var basins_array: std.array_list.Managed(usize) = .init(allocator);
    const part1 = try solve_part1(puzzle, visited, &basins_array);
    std.debug.print("Part 1: {}\n", .{part1});

    const basins = try basins_array.toOwnedSlice();
    std.mem.sort(usize, basins, {}, comptime std.sort.desc(usize));

    var part2: usize = 1;
    for (0..3) |i| {
        part2 *= basins[i];
    }
    std.debug.print("Part 2: {}\n", .{part2});
}
