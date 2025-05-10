const std = @import("std");

const allocator = std.heap.c_allocator;

const Block = struct {
    size: u8,
    values: []usize,
    cntr: u8,
};

pub fn parse_blocks(content: []const u8) ![]Block {
    var blocks = try allocator.alloc(Block, content.len);
    for (content, 0..) |c, i| {
        const size: u8 = c - '0';
        const values = try allocator.alloc(usize, size);
        var cntr: u8 = 0;
        if (i % 2 == 0) {
            @memset(values, i / 2);
            cntr = size;
        }
        blocks[i] = Block{
            .size = size,
            .values = values,
            .cntr = cntr,
        };
    }
    return blocks;
}

pub fn free_blocks(disk: []Block) void {
    for (disk) |block| {
        allocator.free(block.values);
    }
    allocator.free(disk);
}

pub fn next_left(disk: []const Block, ileft: *usize) void {
    while (ileft.* < disk.len) {
        if (disk[ileft.*].cntr < disk[ileft.*].size) return;
        ileft.* += 2;
    }
}

pub fn next_right(disk: []const Block, iright: *usize) void {
    while (disk[iright.*].cntr == 0) {
        iright.* -= 2;
    }
}

pub fn check_sum(disk: []const Block) u64 {
    var sum: u64 = 0;
    var i: usize = 0;
    for (disk) |block| {
        for (0..block.cntr) |j| {
            sum += @intCast(block.values[j] * (i + j));
        }
        i += @intCast(block.size);
    }
    return sum;
}

pub fn transfer(left: *Block, right: *Block) void {
    left.values[left.cntr] = right.values[right.cntr - 1];
    right.cntr -= 1;
    left.cntr += 1;
}

pub fn compact1(disk: []Block) u64 {
    defer free_blocks(disk);
    var ileft: usize = 1;
    var iright: usize = disk.len - 1;
    if (disk.len % 2 == 0) iright -= 1;
    next_left(disk, &ileft);
    next_right(disk, &iright);
    while (ileft < iright) {
        const left = &disk[ileft];
        const right = &disk[iright];
        while (left.cntr < left.size and right.cntr > 0) {
            transfer(left, right);
        }
        next_left(disk, &ileft);
        next_right(disk, &iright);
    }
    return check_sum(disk);
}

pub fn compact2(disk: []Block) u64 {
    defer free_blocks(disk);
    var iright: usize = disk.len - 1;
    if (disk.len % 2 == 0) iright -= 1;
    while (true) {
        var ileft: usize = 1;
        while (ileft < iright) {
            const free = disk[ileft].size - disk[ileft].cntr;
            if (free >= disk[iright].size) {
                for (0..disk[iright].size) |_| {
                    transfer(&disk[ileft], &disk[iright]);
                }
                break;
            }
            ileft += 2;
        }
        if (iright == 0) break;
        iright -= 2;
    }
    return check_sum(disk);
}

pub fn main() !void {
    if (std.os.argv.len != 2) {
        return error.InvalidArgument;
    }
    const cwd = std.fs.cwd();
    const file_name: [:0]const u8 = std.mem.span(std.os.argv[1]);
    const file = try cwd.openFile(file_name, .{});
    defer file.close();

    var content: [20000]u8 = undefined;
    const bytes_read = try std.fs.File.read(file, &content);
    const part1 = compact1(try parse_blocks(content[0 .. bytes_read - 1]));
    const part2 = compact2(try parse_blocks(content[0 .. bytes_read - 1]));
    std.debug.print("Part 1: {}\n", .{part1});
    std.debug.print("Part 2: {}\n", .{part2});
}
