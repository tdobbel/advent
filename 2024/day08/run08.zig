const std = @import("std");

const Position = struct {
    x: i32,
    y: i32,
};

const AntennaMap = std.AutoHashMap(u8, std.ArrayList(Position));
const Antinodes = std.AutoHashMap(Position, void);
const allocator = std.heap.c_allocator;

pub fn add_antinode(
    antinodes: *Antinodes,
    x: i32,
    y: i32,
    nx: i32,
    ny: i32,
) !bool {
    if (x < 0 or y < 0 or x >= nx or y >= ny) {
        return false;
    }
    try antinodes.put(Position{ .x = x, .y = y }, {});
    return true;
}

pub fn count_antinodes(antennas: *AntennaMap, nx: i32, ny: i32) !usize {
    var antinodes = Antinodes.init(allocator);
    defer antinodes.deinit();
    var it = antennas.iterator();
    while (it.next()) |entry| {
        const positions = entry.value_ptr.*;
        const n = positions.items.len;
        for (0..n - 1) |i| {
            const xa: i32 = positions.items[i].x;
            const ya: i32 = positions.items[i].y;
            for (i + 1..n) |j| {
                const xb: i32 = positions.items[j].x;
                const yb: i32 = positions.items[j].y;
                const dx = xb - xa;
                const dy = yb - ya;
                _ = try add_antinode(&antinodes, xb + dx, yb + dy, nx, ny);
                _ = try add_antinode(&antinodes, xa - dx, ya - dy, nx, ny);
            }
        }
    }
    return antinodes.count();
}

pub fn count_antinodes2(antennas: *AntennaMap, nx: i32, ny: i32) !usize {
    var antinodes = Antinodes.init(allocator);
    defer antinodes.deinit();
    var it = antennas.iterator();
    var x: i32 = 0;
    var y: i32 = 0;
    while (it.next()) |entry| {
        const positions = entry.value_ptr.*;
        const n = positions.items.len;
        for (0..n - 1) |i| {
            const xa: i32 = positions.items[i].x;
            const ya: i32 = positions.items[i].y;
            for (i + 1..n) |j| {
                const xb: i32 = positions.items[j].x;
                const yb: i32 = positions.items[j].y;
                const dx = xb - xa;
                const dy = yb - ya;
                x = xa;
                y = ya;
                while (try add_antinode(&antinodes, x, y, nx, ny)) {
                    x -= dx;
                    y -= dy;
                }
                x = xb;
                y = yb;
                while (try add_antinode(&antinodes, x, y, nx, ny)) {
                    x += dx;
                    y += dy;
                }
            }
        }
    }
    return antinodes.count();
}

pub fn main() !void {
    if (std.os.argv.len != 2) {
        return error.InvalidArgument;
    }
    const cwd = std.fs.cwd();
    const file_name: [:0]const u8 = std.mem.span(std.os.argv[1]);
    const file = try cwd.openFile(file_name, .{});
    defer file.close();

    var antennas = AntennaMap.init(allocator);
    defer antennas.deinit();
    var nx: usize = 0;
    var ny: i32 = 0;

    var buffer: [1024]u8 = undefined;
    var buf_reader = std.io.bufferedReader(file.reader());
    var in_stream = buf_reader.reader();

    while (try in_stream.readUntilDelimiterOrEof(&buffer, '\n')) |line| {
        nx = line.len;
        for (line, 0..) |c, x| {
            if (c == '.') continue;
            var v = try antennas.getOrPut(c);
            if (!v.found_existing) {
                v.value_ptr.* = std.ArrayList(Position).init(allocator);
            }
            try v.value_ptr.append(Position{ .x = @intCast(x), .y = ny });
        }
        ny += 1;
    }
    const part1 = try count_antinodes(&antennas, @intCast(nx), ny);
    std.debug.print("Part 1: {}\n", .{part1});
    const part2 = try count_antinodes2(&antennas, @intCast(nx), ny);
    std.debug.print("Part 2: {}\n", .{part2});

    var it = antennas.iterator();
    while (it.next()) |entry| {
        entry.value_ptr.deinit();
    }
}
