const std = @import("std");

const allocator = std.heap.c_allocator;
const ArrayList = std.array_list.Managed;

const Towels = std.AutoHashMap(usize, ArrayList([]u8));

pub fn freeTowels(towels: *Towels) void {
    var it = towels.iterator();
    while (it.next()) |entry| {
        for (entry.value_ptr.items) |value| {
            allocator.free(value);
        }
        entry.value_ptr.deinit();
    }
    towels.deinit();
}

pub fn parseLine(towels: *Towels, line: []const u8) !void {
    var it = std.mem.tokenizeAny(u8, line, ", ");
    while (it.next()) |item| {
        var entry = try towels.getOrPut(item.len);
        if (!entry.found_existing) {
            entry.value_ptr.* = ArrayList([]u8).init(allocator);
        }
        try entry.value_ptr.append(try allocator.dupe(u8, item));
    }
}

pub fn countPossibilities(design: []const u8, towels: *Towels) !u64 {
    var counter: []u64 = try allocator.alloc(u64, design.len);
    defer allocator.free(counter);
    @memset(counter, 0);
    for (0..design.len) |i| {
        const stop = i + 1;
        var it = towels.iterator();
        while (it.next()) |entry| {
            const n = entry.key_ptr.*;
            if (n > stop) continue;
            const increment = if (n == stop) 1 else counter[i - n];
            const slice = design[stop - n .. stop];
            for (entry.value_ptr.items) |value| {
                if (std.mem.eql(u8, slice, value)) {
                    counter[i] += increment;
                    break;
                }
            }
        }
    }
    return counter[counter.len - 1];
}

pub fn main() !void {
    if (std.os.argv.len != 2) {
        return error.InvalidArgument;
    }
    const cwd = std.fs.cwd();
    const file_name: [:0]const u8 = std.mem.span(std.os.argv[1]);
    const file = try cwd.openFile(file_name, .{});
    defer file.close();

    var buffer: [5096]u8 = undefined;
    var reader = file.reader(&buffer);

    var towels = Towels.init(allocator);
    defer freeTowels(&towels);

    var first: bool = true;
    var part1: usize = 0;
    var part2: u64 = 0;
    while (reader.interface.takeDelimiterExclusive('\n')) |line| {
        if (first) {
            try parseLine(&towels, line);
            first = false;
            continue;
        }
        if (line.len == 0) continue;
        const n = try countPossibilities(line, &towels);
        if (n > 0) {
            part1 += 1;
            part2 += n;
        }
    } else |err| if (err != error.EndOfStream) {
        return err;
    }
    std.debug.print("Part 1: {}\n", .{part1});
    std.debug.print("Part 2: {}\n", .{part2});
}
