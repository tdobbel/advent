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
    var x: u32 = 0;
    var y: u32 = 0;
    var aim: u32 = 0;

    while (try reader.interface.takeDelimiter('\n')) |line| {
        var i: usize = 0; 
        while (line[i] != ' ') {
            i += 1;
        }
        const n = try std.fmt.parseInt(u32, line[i+1..], 10);
        if (std.mem.eql(u8, line[0..i], "forward")) {
            x += n;
            y += aim * n;
        } else if (std.mem.eql(u8, line[0..i], "up")) {
            aim -= n;
        } else if (std.mem.eql(u8, line[0..i], "down")) {
            aim += n;
        } else {
            unreachable;
        }
    }

    std.debug.print("Part 1: {}\n", .{x * aim});
    std.debug.print("Part 2: {}\n", .{x * y});
}
