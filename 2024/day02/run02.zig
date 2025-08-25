const std = @import("std");

pub fn parseLine(line: []const u8, numbers: *[20]i32, size: *u32) !void {
    var it = std.mem.tokenizeAny(u8, line, " ");
    size.* = 0;
    while (it.next()) |part| {
        numbers[size.*] = try std.fmt.parseInt(i32, part, 10);
        size.* += 1;
    }
}

pub fn isSafe(array: []const i32) bool {
    const ascending = array[0] < array[1];
    var diff: i32 = 0;
    for (1..array.len) |i| {
        diff = array[i] - array[i - 1];
        if (!ascending) {
            diff *= -1;
        }
        if (diff < 1 or diff > 3) return false;
    }
    return true;
}

pub fn isSafeDampened(array: []const i32) bool {
    var subarr: [20]i32 = undefined;
    for (0..array.len) |i| {
        var k: u32 = 0;
        for (0..array.len) |j| {
            if (i == j) continue;
            subarr[k] = array[j];
            k += 1;
        }
        if (isSafe(subarr[0..array.len - 1])) return true;
    }
    return false;
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
    var reader = file.reader(&buffer);
    var numbers: [20]i32 = undefined;
    var size: u32 = 0;
    var part1: u32 = 0;
    var part2: u32 = 0;
    while (reader.interface.takeDelimiterExclusive('\n')) |line| {
        try parseLine(line, &numbers, &size);
        if (isSafe(numbers[0..size])) {
            part1 += 1;
            part2 += 1;
        } else if (isSafeDampened(numbers[0..size])) {
            part2 += 1;
        }
    } else |err| if (err != error.EndOfStream) return err;
    std.debug.print("Part 1: {}\n", .{part1});
    std.debug.print("Part 2: {}\n", .{part2});
}
