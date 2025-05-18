const std = @import("std");

pub fn contains(comptime T: anytype, array: []const T, value: T) bool {
    for (array) |item| {
        if (item == value) return true;
    }
    return false;
}

pub fn find_start_of_packet(line: []const u8, size: usize) ?usize {
    for (0..line.len-size + 1) |start| {
        var found = true;
        for (start + 1..start + size) |stop| {
            if (contains(u8, line[start..stop], line[stop])) {
                found = false;
                break;
            }
        }
        if (found) return start + size;
    }
    return null;
}

pub fn main() !void {
    if (std.os.argv.len != 2) {
        return error.InvalidArgument;
    }
    const cwd = std.fs.cwd();
    const file_name: [:0]const u8 = std.mem.span(std.os.argv[1]);
    const file = try cwd.openFile(file_name, .{});
    defer file.close();

    var content: [5096]u8 = undefined;
    const bytes_read = try std.fs.File.read(file, &content);
    const part1 = find_start_of_packet(content[0..bytes_read - 1], 4).?;
    std.debug.print("Part 1: {}\n", .{part1});
    const part2 = find_start_of_packet(content[0..bytes_read - 1], 14).?;
    std.debug.print("Part 2: {}\n", .{part2});
}
