const std = @import("std");

const SpellName = enum(usize) {
    MagicMissile = 3,
    Drain = 4,
    Shield = 0,
    Poison = 1,
    Recharge = 2,
};

const Spell = struct { name: SpellName, cost: u32 };

pub fn safe_diff(a: u32, b: u32) u32 {
    if (b > a) return 0;
    return a - b;
}

const spell_book: [5]Spell = [_]Spell{
    Spell{ .name = .MagicMissile, .cost = 53 },
    Spell{ .name = .Drain, .cost = 73 },
    Spell{ .name = .Shield, .cost = 113 },
    Spell{ .name = .Poison, .cost = 173 },
    Spell{ .name = .Recharge, .cost = 229 },
};

const Player = struct {
    hp: u32,
    mana: u32,
    armor: u32 = 0,
    timers: [3]u32 = .{ 0, 0, 0 },

    pub fn cast(self: *Player, spell: Spell, boss: *Boss) !void {
        if (self.mana < spell.cost) {
            return error.NotEnoughMana;
        }
        self.mana -= spell.cost;
        switch (spell.name) {
            .MagicMissile => {
                boss.take_damage(4);
            },
            .Drain => {
                self.hp += 2;
                boss.take_damage(2);
            },
            .Shield => {
                const indx = @intFromEnum(SpellName.Shield);
                if (self.timers[indx] > 0) return error.SpellAlreadyActive;
                self.armor += 7;
                self.timers[indx] = 6;
            },
            .Poison => {
                const indx = @intFromEnum(SpellName.Poison);
                if (self.timers[indx] > 0) return error.SpellAlreadyActive;
                self.timers[indx] = 6;
            },
            .Recharge => {
                const indx = @intFromEnum(SpellName.Recharge);
                if (self.timers[indx] > 0) return error.SpellAlreadyActive;
                self.timers[indx] = 5;
            },
        }
    }

    pub fn apply_spells(self: *Player, boss: *Boss) void {
        if (self.timers[@intFromEnum(SpellName.Poison)] > 0) {
            boss.take_damage(3);
        }
        if (self.timers[@intFromEnum(SpellName.Recharge)] > 0) {
            self.mana += 101;
        }
    }

    pub fn take_damage(self: *Player, damage: u32) void {
        self.hp = safe_diff(self.hp, damage);
    }

    pub fn is_dead(self: *const Player) bool {
        return self.hp == 0;
    }

    pub fn decrement_timers(self: *Player) void {
        for (0..3) |i| {
            if (self.timers[i] == 0) continue;
            self.timers[i] -= 1;
            if (i == @intFromEnum(SpellName.Shield) and self.timers[i] == 0) {
                self.armor -= 7;
            }
        }
    }
};

const Boss = struct {
    hp: u32,
    damage: u32,

    pub fn take_damage(self: *Boss, damage: u32) void {
        self.hp = safe_diff(self.hp, damage);
    }

    pub fn attack(self: *const Boss, player: *Player) void {
        if (self.is_dead()) return;
        player.take_damage(safe_diff(self.damage, player.armor));
    }

    pub fn is_dead(self: *const Boss) bool {
        return self.hp == 0;
    }

    pub fn clone(self: *const Boss) Boss {
        return Boss{ .hp = self.hp, .damage = self.damage };
    }
};

pub fn main(init: std.process.Init) !void {
    const player = Player{ .hp = 50, .mana = 500 };
    const boss = Boss{ .hp = 58, .damage = 9 };

    var part1: u32 = 1000000;
    var part2: u32 = 1000000;

    var buf: [256]u8 = undefined;
    var writer = std.Io.File.stdout().writer(init.io, &buf);

    find_least_mana_required(&part1, 0, player, boss, true, false);
    try writer.interface.print("Part 1: {}\n", .{part1});
    find_least_mana_required(&part2, 0, player, boss, true, true);
    try writer.interface.print("Part 2: {}\n", .{part2});
    try writer.flush();
}

pub fn find_least_mana_required(
    min_mana: *u32,
    spent: u32,
    player_: Player,
    boss_: Boss,
    player_turn: bool,
    hard_mode_on: bool,
) void {
    var player = player_;
    var boss = boss_;
    if (hard_mode_on and player_turn) {
        player.take_damage(1);
        if (player.is_dead()) return;
    }
    player.apply_spells(&boss);
    player.decrement_timers();
    if (!player_turn) {
        boss.attack(&player);
        if (player.is_dead()) return;
        if (boss.is_dead()) {
            min_mana.* = @min(min_mana.*, spent);
            return;
        }
        find_least_mana_required(min_mana, spent, player, boss, true, hard_mode_on);
        return;
    }
    for (spell_book) |spell| {
        var player2 = player;
        var boss2 = boss;
        player2.cast(spell, &boss2) catch continue;
        const new_spent = spent + spell.cost;
        if (boss.is_dead()) {
            min_mana.* = @min(min_mana.*, new_spent);
            continue;
        }
        // We have exceeded the minimum value found so far
        if (new_spent > min_mana.*) continue;
        // No more mana, we are fucked
        if (player2.mana == 0 and player2.timers[@intFromEnum(SpellName.Recharge)] == 0) continue;
        find_least_mana_required(min_mana, new_spent, player2, boss2, false, hard_mode_on);
    }
}
