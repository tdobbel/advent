const std = @import("std");

const GameOfLife = struct {
    nx: usize,
    ny: usize,
    grid: []bool,
    buf: []bool,
    stuck_corners: bool,

    pub fn init(allocator: std.mem.Allocator, ny: usize, nx: usize, grid: []bool, stuck_corners: bool) !GameOfLife {
        const buf: []bool = try allocator.alloc(bool, ny * nx);
        if (stuck_corners) {
            grid[0] = true;
            grid[nx - 1] = true;
            grid[nx * (ny - 1)] = true;
            grid[nx * ny - 1] = true;
        }
        return GameOfLife{ .nx = nx, .ny = ny, .grid = grid, .buf = buf, .stuck_corners = stuck_corners };
    }

    // pub fn show(self: *const GameOfLife) void {
    //     for (0..self.ny) |y| {
    //         for (0..self.nx) |x| {
    //             const c: u8 = if (self.grid[y * self.nx + x]) '#' else '.';
    //             std.debug.print("{c}", .{c});
    //         }
    //         std.debug.print("\n", .{});
    //     }
    //     std.debug.print("\n", .{});
    // }

    pub fn n_active(self: *const GameOfLife) usize {
        var n: usize = 0;
        for (self.grid) |on| {
            if (on) n += 1;
        }
        return n;
    }

    pub fn next(self: *GameOfLife) void {
        for (0..self.ny) |cy| {
            const y_start = if (cy == 0) 0 else cy - 1;
            const y_stop = @min(self.ny - 1, cy + 1);
            for (0..self.nx) |cx| {
                const x_start = if (cx == 0) 0 else cx - 1;
                const x_stop = @min(cx + 1, self.nx - 1);
                var n_neighbor: u8 = 0;
                for (y_start..y_stop + 1) |y| {
                    for (x_start..x_stop + 1) |x| {
                        if (x == cx and y == cy) continue;
                        if (self.grid[y * self.nx + x]) n_neighbor += 1;
                    }
                }
                if (self.grid[cy * self.nx + cx]) {
                    // stays on if 2 or 3 neighbors are on
                    self.buf[cy * self.nx + cx] = n_neighbor == 2 or n_neighbor == 3;
                } else {
                    // turns on if 3 neighbors are on
                    self.buf[cy * self.nx + cx] = n_neighbor == 3;
                }
            }
        }
        if (self.stuck_corners) {
            self.buf[0] = true;
            self.buf[self.nx - 1] = true;
            self.buf[self.nx * (self.ny - 1)] = true;
            self.buf[self.nx * self.ny - 1] = true;
        }
        std.mem.swap([]bool, &self.grid, &self.buf);
    }
};

pub fn main(init: std.process.Init) !void {
    const argv = try init.minimal.args.toSlice(init.arena.allocator());
    if (argv.len < 2) {
        return error.MissingInputFile;
    }

    const file = try std.Io.Dir.cwd().openFile(init.io, argv[1], .{ .mode = .read_only });
    defer file.close(init.io);
    var buf: [1024]u8 = undefined;
    var reader = file.reader(init.io, &buf);

    var arena = std.heap.ArenaAllocator.init(std.heap.c_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();

    var nx: usize = undefined;
    var ny: usize = 0;

    var lights: std.ArrayList(bool) = .empty;

    while (try reader.interface.takeDelimiter('\n')) |line| : (ny += 1) {
        nx = line.len;
        for (line) |c| {
            try lights.append(allocator, c == '#');
        }
    }

    const cloned = try allocator.dupe(bool, lights.items);

    var g1 = try GameOfLife.init(allocator, ny, nx, lights.items, false);
    var g2 = try GameOfLife.init(allocator, ny, nx, cloned, true);

    for (0..100) |_| {
        g1.next();
        g2.next();
    }

    var writer = std.Io.File.stdout().writer(init.io, &buf);
    try writer.interface.print("Part 1: {}\n", .{g1.n_active()});
    try writer.interface.print("Part 2: {}\n", .{g2.n_active()});
    try writer.flush();
}
