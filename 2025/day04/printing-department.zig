const std = @import("std");

const Department = struct {
    map: [][]u8,
    nx: usize,
    ny: usize,

    pub fn new(map: [][]u8) Department {
        const ny = map.len;
        const nx = map[0].len;
        return .{ .map = map, .nx = nx, .ny = ny };
    }

    pub fn is_accessible(self: *const Department, x: usize, y: usize) bool {
        var xmin: usize = 0;
        if (x > 0) {
            xmin = x - 1;
        }
        var xmax: usize = self.nx - 1;
        if (x < self.nx - 1) {
            xmax = x + 1;
        }
        var ymin: usize = 0;
        if (y > 0) {
            ymin = y - 1;
        }
        var ymax: usize = self.ny - 1;
        if (y < self.ny - 1) {
            ymax = y + 1;
        }
        var n: u8 = 0;
        for (ymin..ymax + 1) |iy| {
            for (xmin..xmax + 1) |ix| {
                if (ix == x and iy == y) {
                    continue;
                }
                if (self.map[iy][ix] == '@') {
                    n += 1;
                }
                if (n >= 4) {
                    return false;
                }
            }
        }
        return true;
    }

    pub fn get_accessible_blocks(self: *const Department, allocator: std.mem.Allocator) ![][2]usize {
        var positions = std.array_list.Managed([2]usize).init(allocator);
        defer positions.deinit();
        for (0..self.ny) |y| {
            for (0..self.nx) |x| {
                if (self.map[y][x] == '.') {
                    continue;
                }
                if (self.is_accessible(x, y)) {
                    try positions.append(.{ x, y });
                }
            }
        }
        return positions.toOwnedSlice();
    }

    pub fn remove_all_accessible(self: *Department, allocator: std.mem.Allocator) !usize {
        var n_removed: usize = 0;
        while (true) {
            const pos = try self.get_accessible_blocks(allocator);
            const n = pos.len;
            for (pos) |xy| {
                self.map[xy[1]][xy[0]] = '.';
            }
            allocator.free(pos);
            n_removed += n;
            if (n == 0) break;
        }
        return n_removed;
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

    const allocator = std.heap.c_allocator;
    var map = std.array_list.Managed([]u8).init(allocator);
    defer {
        for (map.items) |line| {
            allocator.free(line);
        }
        map.deinit();
    }

    while (try reader.interface.takeDelimiter('\n')) |line| {
        const line_dup = try allocator.dupe(u8, line);
        try map.append(line_dup);
    }
    var dept = Department.new(map.items);
    const access_pos = try dept.get_accessible_blocks(allocator);
    const part1 = access_pos.len;
    allocator.free(access_pos);
    const part2 = try dept.remove_all_accessible(allocator);
    std.debug.print("Part 1: {}\n", .{part1});
    std.debug.print("Part 2: {}\n", .{part2});
}
