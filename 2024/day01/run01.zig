const std = @import("std");

pub fn parseLine(line: []const u8, numbers: *[2]u32) !void {
    var it = std.mem.tokenizeAny(u8, line, " ");
    var i: u8 = 0;
    while (it.next()) | part | {
        numbers[i] = try std.fmt.parseInt(u32, part, 10);
        i += 1;
    }
}

pub fn getScore(array: []const u32, value: u32) u32 {
    var cntr: u32 = 0;
    for (array) |val| {
        if (val == value) {
            cntr += 1;
        }
    } 
    return cntr * value;
}

pub fn main() !void {
    if (std.os.argv.len != 2) {
        return error.InvalidArgument;
    }
    const cwd = std.fs.cwd();
    const file_name: [:0]const u8 = std.mem.span(std.os.argv[1]);
    const file  = try cwd.openFile(file_name, .{});
    defer file.close();
    var buffer: [1024]u8 = undefined;
    var reader = file.reader(&buffer);
    var numbers: [2]u32 = undefined;
    var arena = std.heap.ArenaAllocator.init(std.heap.c_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();
    const left = try  allocator.alloc(u32, 1024);
    const right = try allocator.alloc(u32, 1024);
    var cntr: u32 = 0;
    while (reader.interface.takeDelimiterExclusive('\n'))  |line| {
        try parseLine(line, &numbers);
        left[cntr] = numbers[0];
        right[cntr] = numbers[1];
        cntr += 1;
    } else |err| if (err != error.EndOfStream) return err;
    std.mem.sort(u32, left[0..cntr], {}, comptime std.sort.asc(u32));
    std.mem.sort(u32, right[0..cntr], {}, comptime std.sort.asc(u32));
    var part1: u32 = 0;
    var part2: u32 = 0;
    for (0..cntr) |i| {
        part1 += if (left[i] > right[i]) left[i] - right[i] else right[i] - left[i];
        part2 += getScore(right[0..cntr], left[i]);
    }
    std.debug.print("Part 1: {}\n", .{part1});
    std.debug.print("Part 2: {}\n", .{part2});
}

