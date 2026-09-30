const std = @import("std");

pub fn count_arrangements(cntr: *std.AutoHashMap(u32, u32), remaining: u32, capacities: []u32, used: []u1, pos: usize) !void {
    if (remaining == 0) {
        var n_used: u32 = 0;
        for (used) |bit| {
            n_used += @intCast(bit);
        }
        const item = try cntr.getOrPut(n_used);
        if (!item.found_existing) item.value_ptr.* = 0;
        item.value_ptr.* += 1;
        return;
    }
    if (pos == capacities.len) return;
    try count_arrangements(cntr, remaining, capacities, used, pos + 1);
    const capa = capacities[pos];
    if (capa > remaining) return;
    used[pos] = 1;
    try count_arrangements(cntr, remaining - capa, capacities, used, pos + 1);
    used[pos] = 0;
}

pub fn main(init: std.process.Init) !void {
    const argv = try init.minimal.args.toSlice(init.arena.allocator());
    if (argv.len < 2) {
        return error.MissingInputFile;
    }

    const file = try std.Io.Dir.cwd().openFile(init.io, argv[1], .{ .mode = .read_only });
    defer file.close(init.io);

    var buf: [256]u8 = undefined;
    var reader = file.reader(init.io, &buf);

    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();

    var containers: std.ArrayList(u32) = .empty;

    while (try reader.interface.takeDelimiter('\n')) |line| {
        const capacity = try std.fmt.parseUnsigned(u32, line, 10);
        try containers.append(allocator, capacity);
    }

    var cntr = std.AutoHashMap(u32, u32).init(allocator);
    const used = try allocator.alloc(u1, containers.items.len);
    @memset(used, 0);

    try count_arrangements(&cntr, 150, containers.items, used, 0);
    var part1: u32 = 0;
    var iter = cntr.iterator();
    var n_min: u32 = @intCast(used.len);
    var part2: u32 = 0;

    while (iter.next()) |item| {
        part1 += item.value_ptr.*;
        const n_used = item.key_ptr.*;
        if (n_used < n_min) {
            n_min = n_used;
            part2 = item.value_ptr.*;
        }
    }

    var writer = std.Io.File.stdout().writer(init.io, &buf);
    try writer.interface.print("Part 1: {}\nPart 2: {}\n", .{ part1, part2 });
    try writer.interface.flush();
}
