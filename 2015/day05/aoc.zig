const std = @import("std");

pub fn is_vowel(c: u8) bool {
    return c == 'a' or c == 'e' or c == 'i' or c == 'o' or c == 'u';
}

pub fn nice_string(string: []const u8, bad_strings: []const []const u8) bool {
    for (bad_strings) |bad| {
        if (std.mem.containsAtLeast(u8, string, 1, bad)) return false;
    }
    if (string.len < 2) return false;
    var has_pair = false;
    var n_vowel: usize = 0;
    for (string, 0..) |letter, i| {
        if (i < string.len - 1 and letter == string[i + 1]) {
            has_pair = true;
        }
        if (is_vowel(letter)) n_vowel += 1;
    }
    if (!has_pair or n_vowel < 3) return false;
    return true;
}

pub fn two_pairs(string: []const u8) bool {
    if (string.len < 4) return false;
    for (2..string.len - 1) |i| {
        const pair = string[i - 2 .. i];
        for (i..string.len - 1) |j| {
            if (pair[0] == string[j] and pair[1] == string[j + 1]) return true;
        }
    }
    return false;
}

pub fn two_letters(string: []const u8) bool {
    if (string.len < 3) return false;
    for (1..string.len - 1) |i| {
        if (string[i - 1] == string[i + 1]) return true;
    }
    return false;
}

pub fn nicer_string(string: []const u8) bool {
    return two_pairs(string) and two_letters(string);
}

pub fn main(init: std.process.Init) !void {
    const argv = try init.minimal.args.toSlice(init.arena.allocator());
    if (argv.len < 2) {
        return error.MissingInputFile;
    }

    const bad = [_][]const u8{ "ab", "cd", "pq", "xy" };

    const file = try std.Io.Dir.cwd().openFile(init.io, argv[1], .{ .mode = .read_only });
    defer file.close(init.io);
    var buf: [256]u8 = undefined;
    var reader = file.reader(init.io, &buf);

    var part1: usize = 0;
    var part2: usize = 0;
    while (try reader.interface.takeDelimiter('\n')) |line| {
        if (nice_string(line, &bad)) part1 += 1;
        if (nicer_string(line)) part2 += 1;
    }

    var writer = std.Io.File.stdout().writer(init.io, &buf);
    try writer.interface.print("Part 1: {}\n", .{part1});
    try writer.interface.print("Part 2: {}\n", .{part2});

    try writer.flush();
}
