const std = @import("std");

const GainMap = std.AutoHashMap([4]i32, usize);

pub fn mix(x: u64, y: u64) u64 {
    return x ^ y;
}

fn prune(x: u64) u64 {
    return x & ((1 << 24) - 1);
}

fn next_number(x: u64) u64 {
    var res: u64 = prune(mix(x << 6, x));
    res = prune(mix(res >> 5, res));
    res = prune(mix(res << 11, res));
    return res;
}

fn ones_digit(x: u64) usize {
    const res: usize = @intCast(x % 10);
    return res;
}

fn get_diff(x: usize, y: *usize) i32 {
    const x_: i32 = @intCast(x);
    const y_: i32 = @intCast(y.*);
    y.* = x;
    return x_ - y_;
}

pub fn main() !void {
    if (std.os.argv.len != 2) return error.InvalidArgument;

    const allocator = std.heap.c_allocator;

    const cwd = std.fs.cwd();
    const file_name: [:0]const u8 = std.mem.span(std.os.argv[1]);
    const file = try cwd.openFile(file_name, .{});
    defer file.close();

    var buffer: [64]u8 = undefined;
    var buf_reader = std.io.bufferedReader(file.reader());
    var in_stream = buf_reader.reader();

    var seq: [4]i32 = undefined;
    var prev: usize = undefined;
    var curr: usize = undefined;
    var part1: u64 = 0;
    var part2: usize = 0;
    var best_seq: [4]i32 = undefined;
    var gains = GainMap.init(allocator);
    defer gains.deinit();
    while (try in_stream.readUntilDelimiterOrEof(&buffer, '\n')) |line| {
        var secret = try std.fmt.parseInt(u64, line, 10);
        prev = ones_digit(secret);
        var i: usize = 0;
        var buyer = GainMap.init(allocator);
        defer buyer.deinit();
        while (i < 3) : (i += 1) {
            secret = next_number(secret);
            seq[i] = get_diff(ones_digit(secret), &prev);
        }
        while (i < 2000) : (i += 1) {
            secret = next_number(secret);
            curr = ones_digit(secret);
            seq[3] = get_diff(curr, &prev);
            if (!buyer.contains(seq)) {
                try buyer.put(seq, curr);
            }
            for (0..3) |j| {
                seq[j] = seq[j + 1];
            }
        }
        part1 += secret;
        var it = buyer.iterator();
        while (it.next()) |entry| {
            const gain_entry = try gains.getOrPut(entry.key_ptr.*);
            if (!gain_entry.found_existing) {
                gain_entry.value_ptr.* = entry.value_ptr.*;
            } else {
                gain_entry.value_ptr.* += entry.value_ptr.*;
            }
            if (gain_entry.value_ptr.* > part2) {
                part2 = gain_entry.value_ptr.*;
                best_seq = entry.key_ptr.*;
            }
        }
    }

    std.debug.print("Part 1: {d}\n", .{part1});
    std.debug.print("Part 2: {d} ({d}, {d}, {d}, {d})\n", .{ part2, best_seq[0], best_seq[1], best_seq[2], best_seq[3] });
}
