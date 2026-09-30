const std = @import("std");

const Sue = struct {
    children: ?u8 = null,
    cats: ?u8 = null,
    samoyeds: ?u8 = null,
    pomeranians: ?u8 = null,
    akitas: ?u8 = null,
    vizslas: ?u8 = null,
    goldfish: ?u8 = null,
    trees: ?u8 = null,
    cars: ?u8 = null,
    perfumes: ?u8 = null,

    pub fn equals_part1(self: *const Sue, other: Sue) bool {
        inline for (std.meta.fields(Sue)) |f| {
            const a = @field(self, f.name);
            const b = @field(other, f.name);
            if (a != null and b != null and a.? != b.?) return false;
        }
        return true;
    }

    pub fn equals_part2(self: *const Sue, other: Sue) bool {
        inline for (std.meta.fields(Sue)) |f| {
            const a = @field(self, f.name);
            const b = @field(other, f.name);
            if (a != null and b != null) {
                var ok: bool = false;
                if (std.mem.eql(u8, f.name, "cats") or std.mem.eql(u8, f.name, "trees")) {
                    ok = b.? > a.?;
                } else if (std.mem.eql(u8, f.name, "pomeranians") or std.mem.eql(u8, f.name, "goldfish")) {
                    ok = b.? < a.?;
                } else {
                    ok = a.? == b.?;
                }
                if (!ok) return false;
            }
        }
        return true;
    }

    pub fn set_field(self: *Sue, fname: []const u8, value: u8) !void {
        inline for (std.meta.fields(Sue)) |f| {
            if (std.mem.eql(u8, f.name, fname)) {
                @field(self, f.name) = value;
                return;
            }
        }
        return error.InvalidFieldName;
    }
};

pub fn main(init: std.process.Init) !void {
    const argv = try init.minimal.args.toSlice(init.arena.allocator());
    if (argv.len < 2) {
        return error.MissingInputFile;
    }
    const ref = Sue{
        .children = 3,
        .cats = 7,
        .samoyeds = 2,
        .pomeranians = 3,
        .akitas = 0,
        .vizslas = 0,
        .goldfish = 5,
        .trees = 3,
        .cars = 2,
        .perfumes = 1,
    };

    const file = try std.Io.Dir.cwd().openFile(init.io, argv[1], .{ .mode = .read_only });
    defer file.close(init.io);

    var buf: [256]u8 = undefined;
    var reader = file.reader(init.io, &buf);

    var part1: ?usize = null;
    var part2: ?usize = null;

    var indx: usize = 1;
    while (try reader.interface.takeDelimiter('\n')) |line| : (indx += 1) {
        var iter = std.mem.splitScalar(u8, line, ',');
        var sue = Sue{};
        while (iter.next()) |slice| {
            var stop: usize = slice.len;
            var start: usize = stop - 1;
            while (slice[start] != ' ') {
                start -= 1;
            }
            const v = try std.fmt.parseUnsigned(u8, slice[start + 1 ..], 10);
            stop = start - 1;
            start = stop - 1;
            while (slice[start] != ' ') {
                start -= 1;
            }
            const field_name = slice[start + 1 .. stop];
            try sue.set_field(field_name, v);
        }
        if (ref.equals_part1(sue)) part1 = indx;
        if (ref.equals_part2(sue)) part2 = indx;
        if (part1 != null and part2 != null) break;
    }

    buf = undefined;
    var writer = std.Io.File.stdout().writer(init.io, &buf);
    try writer.interface.print("Part 1: {}\n", .{part1.?});
    try writer.interface.print("Part 2: {}\n", .{part2.?});
    try writer.flush();
}
