const std = @import("std");

const ObjectType = enum {
    Box,
    Wall,
};

const ObjectBase = struct {
    type: ObjectType,
    x: i32,
    y: i32,
};

const Object = struct {
    type: ObjectType,
    x: i32,
    y: i32,
    width: usize,
};

const Robot = struct {
    x: i32,
    y: i32,
    allocator: std.mem.Allocator,

    pub fn move(self: *Robot, warehouse: [][]i32, objects: []Object, motion: u8) !void {

        var dx: i32 = 0;
        var dy: i32 = 0;
        switch (motion) {
            '^' => dy = -1,
            'v' => dy = 1,
            '<' => dx = -1,
            '>' => dx = 1,
            else => return error.InvalidData, // Invalid move
        }
        var x_next: usize = @intCast(self.x + dx);
        var y_next: usize = @intCast(self.y + dy);
        if (warehouse[y_next][x_next] < 0) {
            self.x += dx;
            self.y += dy;
            return;
        }
        // Find objects that need to be moved
        var toCheck = std.array_list.Managed(usize).init(self.allocator);
        defer toCheck.deinit();
        var queue = std.array_list.Managed(usize).init(self.allocator);
        defer queue.deinit();

        var index: usize = @intCast(warehouse[y_next][x_next]);
        try toCheck.append(index);

        while (toCheck.items.len > 0) {
            index = toCheck.orderedRemove(0);
            try queue.append(index);
            const object = objects[index];
            if (object.type == .Wall) {
                return; // Cannot move through walls
            }
            if (motion == '<' or motion == '>') {
                y_next = @intCast(object.y);
                if (motion == '>') {
                    x_next = @intCast(object.x);
                    x_next += object.width;
                } else {
                    x_next = @intCast(object.x - 1);
                }
                if (warehouse[y_next][x_next] >= 0) {
                    index = @intCast(warehouse[y_next][x_next]);
                    if (!std.mem.containsAtLeastScalar(usize, toCheck.items, 1, index)) {
                        try toCheck.append(index);
                    }
                }
            } else {
                y_next = @intCast(object.y + dy);
                x_next = @intCast(object.x);
                for (0..object.width) |_| {
                    if (warehouse[y_next][x_next] >= 0) {
                        index = @intCast(warehouse[y_next][x_next]);
                        if (!std.mem.containsAtLeastScalar(usize, toCheck.items, 1, index)) {
                            try toCheck.append(index);
                        }
                    }
                    x_next += 1;
                }
            }
        }
        // Move objects
        while (queue.items.len > 0) {
            index = queue.pop().?;
            var object = &objects[index];
            x_next = @intCast(object.x);
            y_next = @intCast(object.y);
            for (0..object.width) |_| {
                warehouse[y_next][x_next] = -1; // Clear old position
                x_next += 1;
            }
            object.x += dx;
            object.y += dy;
            x_next = @intCast(object.x);
            y_next = @intCast(object.y);
            for (0..object.width) |_| {
                warehouse[y_next][x_next] = @intCast(index); // Set new position
                x_next += 1;
            }
        }
        self.x += dx;
        self.y += dy;
    }
};

pub fn createObjects(allocator: std.mem.Allocator, base: []ObjectBase, width: usize) ![]Object {
    var objects = try allocator.alloc(Object, base.len);
    const iwidth: i32 = @intCast(width);
    for (base, 0..) |proto, i| {
        objects[i] = Object{
            .type = proto.type,
            .x = proto.x * iwidth,
            .y = proto.y,
            .width = width,
        };
    }
    return objects;
}

pub fn plotWarehouse(robot: Robot, warehouse: [][]i32, objects: []Object) void {
    for (0..warehouse.len) |y| {
        for (0..warehouse[y].len) |x| {
            if (x == robot.x and y == robot.y) {
                std.debug.print("@", .{});
                continue;
            }
            const index = warehouse[y][x];
            if (index < 0) {
                std.debug.print(".", .{});
            } else {
                const object = objects[@intCast(index)];
                switch (object.type) {
                    .Wall => std.debug.print("#", .{}),
                    .Box => {
                        if (object.width == 1) {
                            std.debug.print("O", .{});
                        } else if (x == object.x) {
                            std.debug.print("[", .{});
                        } else {
                            std.debug.print("]", .{});
                        }
                    },
                }
            }
        }
        std.debug.print("\n", .{});
    }
}

pub fn computeSummedGPS(objects: []Object) i32 {
    var sum: i32 = 0;
    for (objects) |object| {
        if (object.type == .Box) {
            sum += @intCast(object.x + 100 * object.y);
        }
    }
    return sum;
}

pub fn fillWarehouse(allocator: std.mem.Allocator, objects: []Object, nx: usize, ny: usize) ![][]i32 {
    var warehouse = try allocator.alloc([]i32, ny);
    for (0..ny) |i| {
        warehouse[i] = try allocator.alloc(i32, nx);
        @memset(warehouse[i], -1);
    }

    var x: usize = 0;
    var y: usize = 0;
    for (objects, 0..) |object, iobj| {
        x = @intCast(object.x);
        y = @intCast(object.y);
        for (0..object.width) |w| {
            if (y < ny and x < nx) {
                warehouse[y][x + w] = @intCast(iobj);
            }
        }
    }
    return warehouse;
}

pub fn freeWarehouse(allocator: std.mem.Allocator, warehouse: [][]i32) void {
    for (warehouse) |row| {
        allocator.free(row);
    }
    allocator.free(warehouse);
}

pub fn solvePuzzle(
    allocator: std.mem.Allocator,
    x_start: i32,
    y_start: i32,
    object_pos: []ObjectBase,
    nx: usize,
    ny: usize,
    motions: []u8,
    width: usize
) !i32 {
    const objects = try createObjects(allocator, object_pos, width);
    defer allocator.free(objects);
    const warehouse = try fillWarehouse(allocator, objects, nx * width, ny);
    defer freeWarehouse(allocator, warehouse);
    const iwidth: i32 = @intCast(width);
    var robot = Robot{ .x = x_start * iwidth, .y = y_start, .allocator = allocator };
    for (motions) |motion| {
        try robot.move(warehouse, objects, motion);
    }
    // plotWarehouse(robot, warehouse, objects);
    return computeSummedGPS(objects);
}

pub fn main() !void {
    if (std.os.argv.len != 2) {
        return error.InvalidArgument;
    }
    const cwd = std.fs.cwd();
    const file_name: [:0]const u8 = std.mem.span(std.os.argv[1]);
    const file = try cwd.openFile(file_name, .{});
    defer file.close();

    const allocator = std.heap.c_allocator;

    var buffer: [1024]u8 = undefined;
    var moves = std.array_list.Managed(u8).init(allocator);
    defer moves.deinit();
    var object_pos = std.array_list.Managed(ObjectBase).init(allocator);
    defer object_pos.deinit();

    var reader = file.reader(&buffer);

    var parse_warehouse: bool = true;
    var ny: usize = 0;
    var nx: usize = 0;
    var x_start: i32 = undefined;
    var y_start: i32 = undefined;
    while (reader.interface.takeDelimiterExclusive('\n')) |line| {
        if (line.len == 0) {
            parse_warehouse = false;
            continue;
        }
        if (parse_warehouse) {
            nx = line.len;
            for (line, 0..) |c, x| {
                switch (c) {
                    '.' => continue,
                    '@' => {
                        x_start = @intCast(x);
                        y_start = @intCast(ny);
                    },
                    '#' => {
                        const wall = ObjectBase{ .type = .Wall, .x = @intCast(x), .y = @intCast(ny) };
                        try object_pos.append(wall);
                    },
                    'O' => {
                        const box = ObjectBase{ .type = .Box, .x = @intCast(x), .y = @intCast(ny) };
                        try object_pos.append(box);
                    },
                    else => {
                        std.debug.print("Unknown character: {}\n", .{c});
                        return error.InvalidData;
                    },
                }
            }
            ny += 1;
        } else {
            try moves.appendSlice(line);
        }
    } else |err| if (err != error.EndOfStream) return err;

    const part1 = try solvePuzzle(allocator, x_start, y_start, object_pos.items, nx, ny, moves.items, 1);
    std.debug.print("Part 1: {}\n", .{part1});
    const part2 = try solvePuzzle(allocator, x_start, y_start, object_pos.items, nx, ny, moves.items, 2);
    std.debug.print("Part 2: {}\n", .{part2});
}
