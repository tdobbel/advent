const std = @import("std");

pub fn play_turn(hm: *std.hash_map.AutoHashMap(u64, u64), num: *u64, turn: u64) !void {
    const entry = try hm.getOrPut(num.*);
    if (!entry.found_existing) {
        num.* = 0;
    } else {
        const last_turn = entry.value_ptr.*;
        num.* = turn - last_turn;
    }
    entry.value_ptr.* = turn;
}

pub fn main() !void {
    if (std.os.argv.len != 2) {
        return error.InvalidArgument;
    }
    const start_nums = std.mem.span(std.os.argv[1]);
    std.debug.print("{s}\n", .{start_nums});

    var arena: std.heap.ArenaAllocator = .init(std.heap.c_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();

    var hm = std.hash_map.AutoHashMap(u64, u64).init(allocator);

    var start: usize = 0;
    var num: u64 = undefined;
    var turn: u64 = 1;
    for (start_nums, 0..) |c, i| {
        if (c == ',') {
            num = try std.fmt.parseInt(u64, start_nums[start..i], 10);
            try hm.put(num, turn);
            turn += 1;
            start = i + 1;
        }
    }
    num = try std.fmt.parseInt(u64, start_nums[start..], 10);
    while (turn < 2020) : (turn += 1) {
        try play_turn(&hm, &num, turn);
    }
    std.debug.print("Part 1: {}\n", .{num});

    while (turn < 30000000) : (turn += 1) {
        try play_turn(&hm, &num, turn);
    }
    std.debug.print("Part 2: {}\n", .{num});
}
