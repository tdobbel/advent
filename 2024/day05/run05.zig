const std = @import("std");

const parseError = error{ArrayTooSmall};

const orderMap = std.AutoArrayHashMap(u32, std.ArrayList(u32));

pub fn parseOrdering(line: []const u8) ![2]u32 {
    var it = std.mem.tokenizeAny(u8, line, "|");
    var i: usize = 0;
    var nums: [2]u32 = undefined;
    while (it.next()) |part| {
        nums[i] = try std.fmt.parseInt(u32, part, 10);
        i += 1;
    }
    return nums;
}

pub fn parseNumbers(line: []const u8, array: []u32, size: *usize) !void {
    var it = std.mem.tokenizeAny(u8, line, ",");
    size.* = 0;
    while (it.next()) |part| {
        const num = try std.fmt.parseInt(u32, part, 10);
        if (size.* >= array.len) return parseError.ArrayTooSmall;
        array[size.*] = num;
        size.* += 1;
    }
}

pub fn contains(array: []const u32, value: u32) bool {
    for (array) |item| {
        if (item == value) return true;
    }
    return false;
}

pub fn lessThan(largerThan: *orderMap, lhs: u32, rhs: u32) bool {
    const smaller = largerThan.get(rhs);
    if (smaller == null) return false;
    return contains(smaller.?.items, lhs);
}

pub fn isOrdered(ordering: *orderMap, array: []u32) bool {
    for (0..array.len - 1) |i| {
        const smaller = ordering.get(array[i]);
        if (smaller == null) continue;
        for (i + 1..array.len) |j| {
            if (contains(smaller.?.items, array[j])) return false;
        }
    }
    return true;
}

pub fn main() !void {
    if (std.os.argv.len != 2) {
        return error.InvalidArgument;
    }
    const cwd = std.fs.cwd();
    const file_name: [:0]const u8 = std.mem.span(std.os.argv[1]);
    const file = try cwd.openFile(file_name, .{});
    defer file.close();
    var buffer: [1024]u8 = undefined;
    var buf_reader = std.io.bufferedReader(file.reader());
    var in_stream = buf_reader.reader();
    const allocator = std.heap.c_allocator;
    var isLargerThan = orderMap.init(allocator);
    defer isLargerThan.deinit();
    var numbers: [64]u32 = undefined;
    var size: usize = 0;
    var part1: u32 = 0;
    var part2: u32 = 0;
    while (try in_stream.readUntilDelimiterOrEof(&buffer, '\n')) |line| {
        if (line.len == 0) continue;
        if (std.mem.count(u8, line, "|") == 1) {
            const nums = try parseOrdering(line);
            var v = try isLargerThan.getOrPut(nums[1]);
            if (!v.found_existing) {
                v.value_ptr.* = std.ArrayList(u32).init(allocator);
            }
            try v.value_ptr.append(nums[0]);
        } else {
            try parseNumbers(line, &numbers, &size);
            if (isOrdered(&isLargerThan, numbers[0..size])) {
                part1 += numbers[size / 2];
            } else {
                std.mem.sort(u32, numbers[0..size], &isLargerThan, lessThan);
                part2 += numbers[size / 2];
            }
        }
    }
    std.debug.print("Part 1: {}\n", .{part1});
    std.debug.print("Part 2: {}\n", .{part2});

    var it = isLargerThan.iterator();
    while (it.next()) |entry| {
        entry.value_ptr.deinit();
    }
}
