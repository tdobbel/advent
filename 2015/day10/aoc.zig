const std = @import("std");

const bufsize: usize = 5000000;

pub fn next_gen(word: []u2, wsize: *usize, next: []u2) void {
    var i: usize = 0;
    var k: usize = 0;
    while (i < wsize.*) {
        var n: u2 = 1;
        var j: usize = i + 1;
        while (j < wsize.* and word[i] == word[j]) : (j += 1) {
            n += 1;
        }
        next[k + 0] = n;
        next[k + 1] = word[i];
        k += 2;
        i = j;
    }
    @memcpy(word[0..k], next[0..k]);
    wsize.* = k;
}

pub fn main(init: std.process.Init) !void {
    var buffer: [bufsize]u2 = undefined;
    var next: [bufsize]u2 = undefined;
    const start: [10]u2 = [_]u2{ 3, 1, 1, 3, 3, 2, 2, 1, 1, 3 };
    var len = start.len;
    @memcpy(buffer[0..start.len], &start);
    for (0..40) |_| {
        next_gen(&buffer, &len, &next);
    }
    var buf: [256]u8 = undefined;
    var writer = std.Io.File.stdout().writer(init.io, &buf);
    try writer.interface.print("Part 1: {}\n", .{len});
    for (0..10) |_| {
        next_gen(&buffer, &len, &next);
    }
    try writer.interface.print("Part 2: {}\n", .{len});
    try writer.flush();
}
