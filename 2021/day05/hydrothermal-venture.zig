const std = @import("std");

const Segment = struct { x1: u16, y1: u16, x2: u16, y2: u16 };
const SegmentCounter = std.AutoHashMap([2]u16, u8);

pub fn parse_segment(line: []u8) !Segment {
    var numbers: [4]u16 = undefined;
    var iter = std.mem.tokenizeAny(u8, line, " ->,");
    var i: usize = 0;
    while (iter.next()) |elem| {
        numbers[i] = try std.fmt.parseInt(u16, elem, 10);
        i += 1;
    }
    return Segment{ .x1 = numbers[0], .y1 = numbers[1], .x2 = numbers[2], .y2 = numbers[3] };
}

pub fn add_segment(counter: *SegmentCounter, seg: Segment) !void {
    var dx: i16 = 0;
    var nx: u16 = 0;
    if (seg.x2 > seg.x1) {
        dx = 1;
        nx = seg.x2 - seg.x1;
    } else if (seg.x1 > seg.x2) {
        dx = -1;
        nx = seg.x1 - seg.x2;
    }
    var dy: i16 = 0;
    var ny: u16 = 0;
    if (seg.y2 > seg.y1) {
        dy = 1;
        ny = seg.y2 - seg.y1;
    } else if (seg.y1 > seg.y2) {
        dy = -1;
        ny = seg.y1 - seg.y2;
    }
    var n: u16 = nx;
    if (ny > n) {
        n = ny;
    }
    var x: i16 = @intCast(seg.x1);
    var y: i16 = @intCast(seg.y1);
    for (0..n + 1) |_| {
        const entry = try counter.getOrPut(.{ @intCast(x), @intCast(y) });
        if (!entry.found_existing) {
            entry.value_ptr.* = 0;
        }
        entry.value_ptr.* += 1;
        x += dx;
        y += dy;
    }
}

pub fn count_overlaps(counter: *SegmentCounter) u16 {
    var sum: u16 = 0;
    var value_iter = counter.valueIterator();
    while (value_iter.next()) |value_ptr| {
        const value = value_ptr.*;
        if (value > 1) {
            sum += 1;
        }
    }
    return sum;
}

pub fn main() !void {
    if (std.os.argv.len != 2) {
        return error.InvalidArgument;
    }
    const cwd = std.fs.cwd();
    const file_name: [:0]const u8 = std.mem.span(std.os.argv[1]);
    const file = try cwd.openFile(file_name, .{});
    defer file.close();

    var buffer: [4096]u8 = undefined;
    var reader = file.reader(&buffer);

    const allocator = std.heap.c_allocator;
    var counter1 = SegmentCounter.init(allocator);
    defer counter1.deinit();
    var counter2 = SegmentCounter.init(allocator);
    defer counter2.deinit();

    while (try reader.interface.takeDelimiter('\n')) |line| {
        const seg = try parse_segment(line);
        try add_segment(&counter2, seg);
        if (seg.x1 == seg.x2 or seg.y1 == seg.y2) {
            try add_segment(&counter1, seg);
        }
    }

    const part1 = count_overlaps(&counter1);
    const part2 = count_overlaps(&counter2);

    std.debug.print("Part 1: {}\n", .{part1});
    std.debug.print("Part 2: {}\n", .{part2});
}
