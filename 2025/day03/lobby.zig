const std = @import("std");

pub fn argmax(bank: []u8) usize {
    var imax: usize = 0;
    var vmax: u8 = 1;
    for (bank, 0..) |v, i| {
        if (v > vmax) {
            imax = i;
            vmax = v;
        }
    }
    return imax;
}

pub fn max_joltage(bank: []u8, n_battery: usize, joltage: u64) u64 {
    if (n_battery == 0) return joltage;
    const n = bank.len - n_battery + 1;
    const imax = argmax(bank[0..n]);
    const new_joltage = 10 * joltage + bank[imax] - '0';
    return max_joltage(bank[imax + 1 ..], n_battery - 1, new_joltage);
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

    var part1: u64 = 0;
    var part2: u64 = 0;

    while (try reader.interface.takeDelimiter('\n')) |line| {
        part1 += max_joltage(line, 2, 0);
        part2 += max_joltage(line, 12, 0);
    }

    std.debug.print("Part 1: {}\n", .{part1});
    std.debug.print("Part 2: {}\n", .{part2});
}
