const std = @import("std");

pub fn main() !void {
    if (std.os.argv.len != 2) {
        return error.InvalidArgument;
    }
    const cwd = std.fs.cwd();
    const file_name: [:0]const u8 = std.mem.span(std.os.argv[1]);
    const file = try cwd.openFile(file_name, .{});
    defer file.close();
    var buffer: [16]u8 = undefined;
    var reader = file.reader(&buffer);
    var iln: usize = 0;
    var part1: u32 = 0;
    var part2: u32 = 0;
    var prev: u32 = 0;
    var sum1: u32 = 0;
    var sum2: u32 = 0;
    var values = [3]u32{ 0, 0, 0 };
    while (try reader.interface.takeDelimiter('\n')) |line| {
        const depth = try std.fmt.parseInt(u32, line, 10);
        iln += 1;
        if (iln > 1 and depth > prev) {
            part1 += 1;
        }
        prev = depth;
        for (0..3) |i| {
            if (iln > i) {
                values[i] += depth;
            }
        }
        if (iln < 3) continue;
        sum1 = sum2;
        sum2 = values[0];
        for (0..2) |i| {
            values[i] = values[i + 1];
        }
        values[2] = 0;
        if (iln > 3 and sum2 > sum1) part2 += 1;
    }
    std.debug.print("Part 1: {}\n", .{part1});
    std.debug.print("Part 2: {}\n", .{part2});
}
