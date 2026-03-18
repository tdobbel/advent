const std = @import("std");

pub fn count_paths(allocator: std.mem.Allocator, joltages: []u32) !u64 {
    var ii: i32 = @intCast(joltages.len - 2);
    var totals = try allocator.alloc(u64, joltages.len);
    totals[joltages.len - 1] = 1;
    while (ii >= 0) : (ii -= 1) {
        const i: usize = @intCast(ii);
        var j: usize = i + 1;
        totals[i] = 0;
        while (j < joltages.len and joltages[j] - joltages[i] <= 3) : (j += 1) {
            totals[i] += totals[j];
        }
    }
    return totals[0];
}

pub fn main() !void {
    if (std.os.argv.len != 2) {
        return error.InvalidArgument;
    }
    const cwd = std.fs.cwd();
    const file_name: [:0]const u8 = std.mem.span(std.os.argv[1]);
    const file = try cwd.openFile(file_name, .{});
    defer file.close();

    var buffer: [128]u8 = undefined;
    var reader = file.reader(&buffer);

    var arena: std.heap.ArenaAllocator = .init(std.heap.c_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();

    var vec = std.array_list.Managed(u32).init(allocator);
    try vec.append(0);

    while (try reader.interface.takeDelimiter('\n')) |line| {
        const jolt = try std.fmt.parseInt(u32, line, 10);
        try vec.append(jolt);
    }
    const joltages = try vec.toOwnedSlice();
    std.mem.sort(u32, joltages, {}, std.sort.asc(u32));
    var d1: u32 = 0;
    var d3: u32 = 1;
    for (0..joltages.len - 1) |i| {
        const diff: u32 = joltages[i + 1] - joltages[i];
        if (diff == 1) {
            d1 += 1;
        } else if (diff == 3) {
            d3 += 1;
        }
    }

    std.debug.print("d1={}, d3={}\n", .{ d1, d3 });
    std.debug.print("Part 1: {}\n", .{d1 * d3});

    const part2 = try count_paths(allocator, joltages);
    std.debug.print("Part 2: {}\n", .{part2});
}
