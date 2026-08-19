const std = @import("std");

pub fn main(init: std.process.Init) !void {
    const argv = try init.minimal.args.toSlice(init.arena.allocator());
    if (argv.len < 2) {
        return error.MissingInputFile;
    }
    var buf: [256]u8 = undefined;
    var hash: [16]u8 = undefined;
    var i: usize = 0;

    var part1: ?usize = null;
    var part2: ?usize = null;

    while (part1 == null or part2 == null) : (i += 1) {
        const key = try std.fmt.bufPrint(&buf, "{s}{}", .{ argv[1], i });
        std.crypto.hash.Md5.hash(key, &hash, .{});
        const out = std.fmt.bytesToHex(hash, .lower);
        if (std.mem.eql(u8, out[0..5], "00000") and part1 == null) {
            part1 = i;
        }
        if (std.mem.eql(u8, out[0..6], "000000") and part2 == null) {
            part2 = i;
        }
    }

    var outbuf: [256]u8 = undefined;
    var writer = std.Io.File.stdout().writer(init.io, &outbuf);
    try writer.interface.print("Part 1: {}\n", .{part1.?});
    try writer.interface.print("Part 2: {}\n", .{part2.?});
    try writer.flush();
}
