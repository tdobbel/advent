const std = @import("std");

const dxs = [8]isize{ -1, 0, 1, 1, 1, 0, -1, -1 };
const dys = [8]isize{ -1, -1, -1, 0, 1, 1, 1, 0 };

const Seats = struct {
    nx: usize,
    ny: usize,
    state0: []const u8,
    state: []u8,
    prev: []u8,

    pub fn init(
        allocator: std.mem.Allocator,
        state0: []const u8,
        nx: usize,
        ny: usize,
    ) !Seats {
        const state = try allocator.dupe(u8, state0);
        const prev = try allocator.alloc(u8, state0.len);
        return Seats{ .nx = nx, .ny = ny, .state0 = state0, .state = state, .prev = prev };
    }

    pub fn get_first_seat(self: *const Seats, x: usize, y: usize, dx: isize, dy: isize) ?u8 {
        var ix: isize = @as(isize, @intCast(x)) + dx;
        var iy: isize = @as(isize, @intCast(y)) + dy;
        const inx: isize = @intCast(self.nx);
        const iny: isize = @intCast(self.ny);
        while (ix >= 0 and iy >= 0 and ix < inx and iy < iny) {
            const index: usize = @intCast(iy * inx + ix);
            if (self.prev[index] != '.') return self.prev[index];
            ix += dx;
            iy += dy;
        }
        return null;
    }

    pub fn count_neighbours_v1(self: *const Seats, x: usize, y: usize) u32 {
        const ix: isize = @intCast(x);
        const iy: isize = @intCast(y);
        var n_neighbors: u32 = 0;
        for (0..8) |k| {
            const ixn = ix + dxs[k];
            const iyn = iy + dys[k];
            if (ixn < 0 or iyn < 0) continue;
            const xn: usize = @intCast(ixn);
            const yn: usize = @intCast(iyn);
            if (xn >= self.nx or yn >= self.ny) continue;
            if (self.prev[yn * self.nx + xn] == '#') {
                n_neighbors += 1;
            }
            if (n_neighbors == 4) {
                break;
            }
        }
        return n_neighbors;
    }

    pub fn count_neighbours_v2(self: *const Seats, x: usize, y: usize) u32 {
        var n_neighbors: u32 = 0;
        for (0..8) |k| {
            if (self.get_first_seat(x, y, dxs[k], dys[k])) |c| {
                if (c == '#') n_neighbors += 1;
            }
            if (n_neighbors == 5) break;
        }
        return n_neighbors;
    }

    pub fn next_gen(self: *Seats, v1: bool) u32 {
        @memcpy(self.prev, self.state);
        var delta: u32 = 0;
        var max_neighbors: u32 = 4;
        if (!v1) max_neighbors += 1;
        for (0..self.ny) |y| {
            for (0..self.nx) |x| {
                const index = y * self.nx + x;
                const seat = self.prev[index];
                if (seat == '.') continue;
                const n_neighbors = if (v1) self.count_neighbours_v1(x, y) else self.count_neighbours_v2(x, y);
                if (seat == 'L' and n_neighbors == 0) {
                    self.state[index] = '#';
                    delta += 1;
                } else if (seat == '#' and n_neighbors >= max_neighbors) {
                    self.state[index] = 'L';
                    delta += 1;
                }
            }
        }
        return delta;
    }

    pub fn reset_state(self: *Seats) void {
        @memcpy(self.state, self.state0);
    }

    pub fn print_state(self: *const Seats) void {
        for (0..self.ny) |y| {
            for (0..self.nx) |x| {
                std.debug.print("{c}", .{self.state[y * self.nx + x]});
            }
            std.debug.print("\n", .{});
        }
    }

    pub fn count_occupied(self: *const Seats) u32 {
        var count: u32 = 0;
        for (self.state) |c| {
            if (c == '#') count += 1;
        }
        return count;
    }
};

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

    var arena: std.heap.ArenaAllocator = .init(std.heap.c_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();

    var vec = std.array_list.Managed(u8).init(allocator);

    var nx: usize = undefined;
    var ny: usize = 0;
    while (try reader.interface.takeDelimiter('\n')) |line| {
        nx = line.len;
        for (line) |c| {
            try vec.append(c);
        }
        ny += 1;
    }
    const state = try vec.toOwnedSlice();
    var seats = try Seats.init(allocator, state, nx, ny);

    while (seats.next_gen(true) > 0) {}
    const part1 = seats.count_occupied();
    std.debug.print("Part 1: {}\n", .{part1});

    seats.reset_state();
    while(seats.next_gen(false) > 0) {}
    const part2 = seats.count_occupied();
    std.debug.print("Part 2: {}\n", .{part2});
}
