const std = @import("std");

pub fn total_fish(n_days: u32, timer: u32, cache: *std.AutoHashMap(u32, u64)) !u64 {
    if (n_days < timer + 1) return 1;
    if (n_days == timer + 1) return 2;
    var v1: u64 = undefined;
    const k1 = n_days - timer;
    if (cache.get(k1)) |value| {
        v1 = value;
    } else {
        v1 = try total_fish(k1, 7, cache);
        try cache.put(k1, v1);
    }
    var v2: u64 = undefined;
    const k2 = n_days - timer - 2;
    if (cache.get(k2)) |value| {
        v2 = value;
    } else {
        v2 = try total_fish(k2, 7, cache);
        try cache.put(k2, v2);
    }
    return v1 + v2;
}

pub fn main() !void {
    if (std.os.argv.len != 2) {
        return error.InvalidArgument;
    }
    const cwd = std.fs.cwd();
    const file_name: [:0]const u8 = std.mem.span(std.os.argv[1]);
    const file = try cwd.openFile(file_name, .{});
    defer file.close();

    var buffer: [4096]u8 = undefined;
    var reader = file.reader(&buffer);

    const allocator = std.heap.c_allocator;
    var timers = std.array_list.Managed(u32).init(allocator);
    defer timers.deinit();
    if (try reader.interface.takeDelimiter('\n')) |line| {
        var iter = std.mem.tokenizeAny(u8, line, ",");
        while (iter.next()) |elem| {
            const num = try std.fmt.parseInt(u32, elem, 10);
            try timers.append(num);
        }
    }

    var cache = std.AutoHashMap(u32, u64).init(allocator);
    defer cache.deinit();

    var part1: u64 = 0;
    var part2: u64 = 0;
    for (timers.items) |timer| {
        part1 += try total_fish(80, timer, &cache);
        part2 += try total_fish(256, timer, &cache);
    }

    std.debug.print("Part 1: {}\n", .{part1});
    std.debug.print("Part 2: {}\n", .{part2});
}
