const std = @import("std");

const Item = struct {
    cost: u32,
    damage: u32,
    armor: u32,
};

const the_boss = Character{ .health = 109, .damage = 8, .armor = 2 };

const weapons: [5]Item = [_]Item{
    Item{ .cost = 8, .damage = 4, .armor = 0 },
    Item{ .cost = 10, .damage = 5, .armor = 0 },
    Item{ .cost = 25, .damage = 6, .armor = 0 },
    Item{ .cost = 40, .damage = 7, .armor = 0 },
    Item{ .cost = 74, .damage = 8, .armor = 0 },
};

const armors: [6]Item = [_]Item{
    Item{ .cost = 0, .damage = 0, .armor = 0 },
    Item{ .cost = 13, .damage = 0, .armor = 1 },
    Item{ .cost = 31, .damage = 0, .armor = 2 },
    Item{ .cost = 53, .damage = 0, .armor = 3 },
    Item{ .cost = 75, .damage = 0, .armor = 4 },
    Item{ .cost = 102, .damage = 0, .armor = 5 },
};

const rings: [6]Item = [_]Item{
    Item{ .cost = 25, .damage = 1, .armor = 0 },
    Item{ .cost = 50, .damage = 2, .armor = 0 },
    Item{ .cost = 100, .damage = 3, .armor = 0 },
    Item{ .cost = 20, .damage = 0, .armor = 1 },
    Item{ .cost = 40, .damage = 0, .armor = 2 },
    Item{ .cost = 80, .damage = 0, .armor = 3 },
};

pub fn safe_sub(a: u32, b: u32) u32 {
    return a - @min(a, b);
}

const Character = struct {
    health: u32,
    damage: u32,
    armor: u32,

    pub fn wins_against(self: *const Character, boss: Character) bool {
        const boss_damage = safe_sub(boss.damage, self.armor);
        const player_damage = safe_sub(self.damage, boss.armor);
        if (player_damage == 0) return false;
        if (boss_damage == 0) return true;
        var n_turn_loss: u32 = self.health / boss_damage;
        if (n_turn_loss * boss_damage < self.health) n_turn_loss += 1;
        var n_turn_win: u32 = boss.health / player_damage;
        if (n_turn_win * player_damage < boss.health) n_turn_win += 1;
        return n_turn_win <= n_turn_loss;
    }
};

pub fn solve_puzzle(boss: Character) [2]u32 {
    var min_gold: u32 = 10000000;
    var max_gold: u32 = 0;
    for (weapons) |weapon| {
        for (armors) |armor| {
            const base_cost = weapon.cost + armor.cost;
            const base_player = Character{ .health = 100, .armor = armor.armor, .damage = weapon.damage };
            if (base_player.wins_against(boss)) {
                // No need to buy a ring
                min_gold = @min(min_gold, base_cost);
                continue;
            } else {
                max_gold = @max(max_gold, base_cost);
            }
            for (0..rings.len) |i| {
                const ring1 = rings[i];
                const cost_r1 = base_cost + ring1.cost;
                const player_r1 = Character{
                    .health = 100,
                    .damage = weapon.damage + ring1.damage,
                    .armor = armor.armor + ring1.armor,
                };
                if (player_r1.wins_against(boss)) {
                    // No need to buy a second ring
                    min_gold = @min(min_gold, cost_r1);
                    continue;
                } else {
                    max_gold = @max(max_gold, cost_r1);
                }
                for (i + 1..rings.len) |j| {
                    const ring2 = rings[j];
                    const cost_r2 = cost_r1 + ring2.cost;
                    const player_r2 = Character{
                        .health = 100,
                        .damage = player_r1.damage + ring2.damage,
                        .armor = player_r1.armor + ring2.armor,
                    };
                    if (player_r2.wins_against(boss)) {
                        min_gold = @min(cost_r2, min_gold);
                    } else {
                        max_gold = @max(cost_r2, max_gold);
                    }
                }
            }
        }
    }
    return .{ min_gold, max_gold };
}

pub fn main(init: std.process.Init) !void {
    const sol = solve_puzzle(the_boss);
    var buf: [256]u8 = undefined;
    var writer = std.Io.File.stdout().writer(init.io, &buf);
    try writer.interface.print("Part 1: {}\nPart 2: {}\n", .{ sol[0], sol[1] });
    try writer.flush();
}
