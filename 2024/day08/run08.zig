const std = @import("std");

const AntennaMap = std.AutoHashMap(u8, std.ArrayList(usize));
const allocator = std.heap.page_allocator;

pub fn validIndex(x: i32, y: i32, nx: usize, ny: usize, index: *usize) bool {
    const inx: i32 = @intCast(nx);
    const iny: i32 = @intCast(ny);
    if (x < 0 or y < 0 or x >= inx or y >= iny) {
        return false;
    }
    index.* = @intCast(x + y * inx);
    return true;
}

pub fn antinodes1(antennas: *AntennaMap, nx: usize, ny: usize) !usize {
    var antinodes = std.AutoHashMap(usize, void).init(allocator);
    defer antinodes.deinit();
    var it = antennas.iterator();
    var index: usize = 0;
    while (it.next()) |entry| {
        const nodes = entry.value_ptr.*;
        const n = nodes.items.len;
        for (0..n - 1) |i| {
            const xa: i32 = @intCast(nodes.items[i] % nx);
            const ya: i32 = @intCast(nodes.items[i] / nx);
            for (i + 1..n) |j| {
                const xb: i32 = @intCast(nodes.items[j] % nx);
                const yb: i32 = @intCast(nodes.items[j] / nx);
                const dx = xb - xa;
                const dy = yb - ya;
                if (validIndex(xa - dx, ya - dy, nx, ny, &index)) try antinodes.put(index, {});
                if (validIndex(xb + dx, yb + dy, nx, ny, &index)) try antinodes.put(index, {});
            }
        }
    }
    return antinodes.count();
}

pub fn antinodes2(antennas: *AntennaMap, nx: usize, ny: usize) !usize {
    var antinodes = std.AutoHashMap(usize, void).init(allocator);
    defer antinodes.deinit();
    var it = antennas.iterator();
    var index: usize = 0;
    while (it.next()) |entry| {
        const nodes = entry.value_ptr.*;
        const n = nodes.items.len;
        for (0..n - 1) |i| {
            const xa: i32 = @intCast(nodes.items[i] % nx);
            const ya: i32 = @intCast(nodes.items[i] / nx);
            for (i + 1..n) |j| {
                const xb: i32 = @intCast(nodes.items[j] % nx);
                const yb: i32 = @intCast(nodes.items[j] / nx);
                var dx = xb - xa;
                var dy = yb - ya;
                for (0..2) |k| {
                    if (k == 1) {
                        dx = -dx;
                        dy = -dy;
                    }
                    var x = xa;
                    var y = ya;
                    while (validIndex(x, y, nx, ny, &index)) {
                        try antinodes.put(index, {});
                        x += dx;
                        y += dy;
                    }
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
    // var part2: u32 = 0;
    var nx: usize = 0;
    var ny: usize = 0;

    var buffer: [1024]u8 = undefined;
    var buf_reader = std.io.bufferedReader(file.reader());
    var in_stream = buf_reader.reader();
    while (try in_stream.readUntilDelimiterOrEof(&buffer, '\n')) |line| {
        if (line.len == 0) continue;
        nx = line.len;
        for (line, 0..) |c, i| {
            if (c == '.') continue;
            const index = nx * ny + i;
            var v = try antennas.getOrPut(c);
            if (!v.found_existing) {
                v.value_ptr.* = std.ArrayList(usize).init(allocator);
            }
            try v.value_ptr.append(index);
        }
        ny += 1;
    }
    const part1 = try antinodes1(&antennas, @intCast(nx), @intCast(ny));
    std.debug.print("Part 1: {}\n", .{part1});
    const part2 = try antinodes2(&antennas, @intCast(nx), @intCast(ny));
    std.debug.print("Part 2: {}\n", .{part2});
}
