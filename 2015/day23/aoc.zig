const std = @import("std");

const Keyword = enum { hlf, tpl, inc, jmp, jie, jio };

const Instruction = struct {
    keyword: Keyword,
    register: ?usize,
    offset: ?isize,
};

pub fn main(init: std.process.Init) !void {
    const args = try init.minimal.args.toSlice(init.arena.allocator());
    if (args.len < 2) {
        return error.MissingInputFile;
    }

    var file = try std.Io.Dir.cwd().openFile(init.io, args[1], .{ .mode = .read_only });
    defer file.close(init.io);

    var buf: [256]u8 = undefined;
    var reader = file.reader(init.io, &buf);

    const gpa = std.heap.c_allocator;

    var instructions: std.ArrayList(Instruction) = .empty;
    defer instructions.deinit(gpa);

    while (try reader.interface.takeDelimiter('\n')) |line| {
        try instructions.append(gpa, try parse_instuction(line));
    }

    var writer = std.Io.File.stdout().writer(init.io, &buf);

    const part1 = run_program(instructions.items, 0);
    try writer.interface.print("Part 1: {}\n", .{part1});

    const part2 = run_program(instructions.items, 1);
    try writer.interface.print("Part 2: {}\n", .{part2});

    try writer.flush();
}

pub fn run_program(program: []const Instruction, a: i32) i32 {
    var registers: [2]i32 = .{ a, 0 };
    var ptr: isize = 0;
    while (ptr >= 0 and ptr < program.len) {
        const i = program[@as(usize, @intCast(ptr))];
        switch (i.keyword) {
            .hlf => {
                const r = i.register.?;
                registers[r] = @divTrunc(registers[r], 2);
                ptr += 1;
            },
            .tpl => {
                const r = i.register.?;
                registers[r] *= 3;
                ptr += 1;
            },
            .inc => {
                const r = i.register.?;
                registers[r] += 1;
                ptr += 1;
            },
            .jmp => {
                ptr += i.offset.?;
            },
            .jie => {
                const r = i.register.?;
                if (@mod(registers[r], 2) == 0) {
                    ptr += i.offset.?;
                } else {
                    ptr += 1;
                }
            },
            .jio => {
                const r = i.register.?;
                if (registers[r] == 1) {
                    ptr += i.offset.?;
                } else {
                    ptr += 1;
                }
            },
        }
    }
    return registers[1];
}

pub fn parse_keyword(kw: []const u8) !Keyword {
    if (std.mem.eql(u8, kw, "hlf")) return .hlf;
    if (std.mem.eql(u8, kw, "tpl")) return .tpl;
    if (std.mem.eql(u8, kw, "inc")) return .inc;
    if (std.mem.eql(u8, kw, "jmp")) return .jmp;
    if (std.mem.eql(u8, kw, "jie")) return .jie;
    if (std.mem.eql(u8, kw, "jio")) return .jio;
    return error.UnknownKeyword;
}

pub fn parse_instuction(line: []const u8) !Instruction {
    var parts: [3][]const u8 = undefined;
    var n: usize = 0;
    var iter = std.mem.splitScalar(u8, line, ' ');
    while (iter.next()) |w| : (n += 1) {
        parts[n] = w;
    }
    const kw = try parse_keyword(parts[0]);
    if (kw == .jmp) {
        const offset = try std.fmt.parseInt(isize, parts[1], 10);
        return Instruction{ .keyword = kw, .register = null, .offset = offset };
    }
    const register: usize = if (parts[1][0] == 'a') 0 else 1;
    var offset: ?isize = null;
    if (n == 3) {
        offset = try std.fmt.parseInt(isize, parts[2], 10);
    }
    return Instruction{
        .keyword = try parse_keyword(parts[0]),
        .register = register,
        .offset = offset,
    };
}
