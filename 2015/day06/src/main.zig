const std = @import("std");
const day06 = @import("day06");
const c = @import("c");

pub fn get_group(pmatch: c.regmatch_t, input: []const u8) []const u8 {
    const start: usize = @intCast(pmatch.rm_so);
    const end: usize = @intCast(pmatch.rm_eo);
    return input[start..end];
}

const Regex = struct {
    inner: *c.regex_t,

    fn init(pattern: [:0]const u8) !Regex {
        const inner = c.alloc_regex_t().?;
        if (0 != c.regcomp(inner, pattern, c.REG_NEWLINE | c.REG_EXTENDED)) {
            return error.compile;
        }

        return .{
            .inner = inner,
        };
    }

    fn deinit(self: *Regex) void {
        c.free_regex_t(self.inner);
    }

    fn matches(self: *Regex, input: [:0]const u8) void {
        const match_size = 6;
        var pmatch: [match_size]c.regmatch_t = undefined;
        if (c.regexec(self.inner, input, match_size, &pmatch, 0) != 0) return;
        std.debug.print("{s}\n", .{get_group(pmatch[1], input)});
    }
};

pub fn main(init: std.process.Init) !void {
    const args = try init.minimal.args.toSlice(init.arena.allocator());
    if (args.len < 2) return error.MissingInputFile;
    var regex = try Regex.init("(turn on|toggle|turn off) ([0-9]+),([0-9]+) through ([0-9]+),([0-9]+)");
    defer regex.deinit();
    regex.matches("turn on 0,0 through 999,999");
}
