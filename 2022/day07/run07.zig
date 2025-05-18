const std = @import("std");

const TOTAL_DISK_SPACE = 70_000_000;
const REQUIRED_SPACE = 30_000_000;

const allocator = std.heap.c_allocator;

const Directory = struct {
    name: []const u8,
    fsize: usize,
    parent: ?*Directory,
    children: std.StringHashMap(*Directory),

    pub fn create(name: []const u8, parent: ?*Directory) Directory {
        return Directory{
            .name = name,
            .fsize = 0,
            .parent = parent,
            .children = std.StringHashMap(*Directory).init(allocator),
        };
    }

    pub fn add_children(self: *Directory, name: []const u8) !void {
        const dir = try allocator.create(Directory);
        dir.* = Directory.create(name, self);
        try self.children.put(name, dir);
    }

    pub fn deinit(self: *Directory) void {
        var it = self.children.iterator();
        while (it.next()) |entry| {
            entry.value_ptr.*.deinit();
        }
        self.children.deinit();
        if (self.parent != null) {
            allocator.free(self.name);
            allocator.destroy(self);
        }
    }

    pub fn get_size(self: *Directory) usize {
        var size = self.fsize;
        var it = self.children.iterator();
        while (it.next()) |entry| {
            size += entry.value_ptr.*.get_size();
        }
        return size;
    }
};

pub fn split_line(line: []const u8) !std.ArrayList([]const u8) {
    var parts = std.ArrayList([]const u8).init(allocator);
    var it = std.mem.tokenizeAny(u8, line, " ");
    while (it.next()) |token| {
        try parts.append(token);
    }
    return parts;
}

pub fn get_total_size_below_limit(dir: *Directory, limit: usize) usize {
    var result: usize = 0;
    const dir_size = dir.get_size();
    if (dir_size < limit) {
        result += dir_size;
    }
    var it = dir.children.iterator();
    while (it.next()) |entry| {
        result += get_total_size_below_limit(entry.value_ptr.*, limit);
    }
    return result;
}

pub fn get_minimum_free(dir: *Directory, target_size: usize, result: *usize) void {
    const dir_size = dir.get_size();
    if (dir_size < target_size) return;
    if (dir_size < result.*) result.* = dir_size;
    var it = dir.children.iterator();
    while (it.next()) |entry| {
        get_minimum_free(entry.value_ptr.*, target_size, result);
    }
}

pub fn main() !void {
    if (std.os.argv.len != 2) {
        return error.InvalidArgument;
    }
    const cwd = std.fs.cwd();
    const file_name: [:0]const u8 = std.mem.span(std.os.argv[1]);
    const file = try cwd.openFile(file_name, .{});
    defer file.close();

    var buffer: [5096]u8 = undefined;
    var buf_reader = std.io.bufferedReader(file.reader());
    var in_stream = buf_reader.reader();

    var root = Directory.create("/", null);
    defer root.deinit();
    var current = &root;

    var first = true;
    while (try in_stream.readUntilDelimiterOrEof(&buffer, '\n')) |line| {
        if (first) {
            first = false;
            continue;
        }
        const split = try split_line(line);
        defer split.deinit();
        if (std.mem.eql(u8, split.items[0], "$")) {
            if (std.mem.eql(u8, split.items[1], "cd")) {
                if (std.mem.eql(u8, split.items[2], "..")) {
                    current = current.parent.?;
                } else {
                    current = current.children.get(split.items[2]).?;
                }
            } else if (std.mem.eql(u8, split.items[1], "ls")) {
                continue;
            } else {
                return error.InvalidCommand;
            }
        } else if (std.mem.eql(u8, split.items[0], "dir")) {
            const name = try allocator.dupe(u8, split.items[1]);
            errdefer allocator.free(name);
            try current.add_children(name);
        } else {
            const size = try std.fmt.parseInt(usize, split.items[0], 10);
            current.fsize += size;
        }
    }
    const part1 = get_total_size_below_limit(&root, 100_000);
    std.debug.print("Part 1: {}\n", .{part1});
    const free_space = TOTAL_DISK_SPACE - root.get_size();
    const target_size = REQUIRED_SPACE - free_space;
    var part2: usize = TOTAL_DISK_SPACE;
    get_minimum_free(&root, target_size, &part2);
    std.debug.print("Part 2: {}\n", .{part2});
}
