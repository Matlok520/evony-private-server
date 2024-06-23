#include <iostream>
#include <array>
#include <algorithm>
#include <cmath>

struct researchStats {
    int iron_working;
    int medicine;
    int compass;
    int archery;
    int horseback_riding;
};

struct heroStat {
    int intel;
};

struct troopStat {
    int life;
    int attack;
    int defense;
    int speed;
    int range;
};

struct combatTroops {
    int count;
    int location;
    int typeId;
    troopStat* stat;
    bool isAttacker;
    int effectiveTroops;
};

struct battleResult {
    int winner;
    int wallHitpoints;
};

struct attacker {
    researchStats research;
    heroStat hero;
    float attack_modifier;
    float defence_modifier;
    float life_modifier;
    int troops[12];
    int fortifications[5];
};

struct defender {
    researchStats research;
    heroStat hero;
    float attack_modifier;
    float defence_modifier;
    float life_modifier;
    int troops[12];
    int fortifications[5];
    int wallLevel;
    int trapKillPower;
};

class CombatSimulator {
private:
    static const std::array<int, 5> meleeTroopTypes;
    static const std::array<int, 3> rangedTroopTypes;
    static const std::array<int, 3> mechTroopTypes;
    static const std::array<int, 2> mountedTroopTypes;
    static const std::array<int, 6> groundTroopTypes;
    static const bool troopTypes[12];
    static const int movementOrder[12];
    static const int nonRangedTroops[9];
    static const troopStat baseStats[12];
    static const troopStat baseFortificationStats[5];
    static const float damageModifiers[12][12];

    static void modifyStats(troopStat* base, researchStats res, heroStat hero, float atk_modifier, float def_modifier, float life_modifier);
    static int compareSpeed(const combatTroops& x, const combatTroops& y);
    static float calculateDamage(int attackerType, int defenderType, float rawDamage);
    static void applyDamage(attacker& atk, defender& def, battleResult& result, int atkType, int defType, int atkCount, int defCount);

public:
    static void fight(attacker& atk, defender& def, battleResult& br);
    static void simulateBattle(attacker& atk, defender& def, battleResult& result);
    static void simulateRounds(attacker& atk, defender& def, battleResult& result, int numRounds);
    static void applyResearchAndHeroStats(attacker& atk, defender& def);
    static void printTroopStats(const attacker& atk, const defender& def);
};

// Static member initialization
const std::array<int, 5> CombatSimulator::meleeTroopTypes = { 0, 1, 2, 3, 4 };
const std::array<int, 3> CombatSimulator::rangedTroopTypes = { 5, 9, 11 };
const std::array<int, 3> CombatSimulator::mechTroopTypes = { 9, 10, 11 };
const std::array<int, 2> CombatSimulator::mountedTroopTypes = { 7, 8 };
const std::array<int, 6> CombatSimulator::groundTroopTypes = { 0, 1, 2, 3, 4, 5 };
const bool CombatSimulator::troopTypes[12] = { true, true, true, true, true, false, true, true, true, false, true, false };
const int CombatSimulator::movementOrder[12] = { 2, 7, 8, 3, 4, 1, 0, 6, 10, 5, 9, 11 };
const int CombatSimulator::nonRangedTroops[9] = { 7, 8, 3, 4, 1, 0, 6, 2, 10 };
const troopStat CombatSimulator::baseStats[12] = {
    { 100, 5, 50, 180, 10 },     // worker
    { 200, 50, 50, 200, 20 },    // warrior
    { 100, 20, 50, 3000, 20 },   // scout
    { 300, 150, 150, 300, 50 },  // pike
    { 350, 100, 250, 275, 30 },  // swords
    { 250, 120, 50, 250, 1200 }, // archer
    { 700, 10, 60, 150, 10 },    // transporter
    { 500, 250, 180, 1000, 100 },// cavalry
    { 1000, 350, 350, 750, 80 }, // cataphract
    { 320, 450, 160, 100, 1400 },// ballista
    { 5000, 250, 160, 120, 600 },// ram
    { 480, 600, 200, 80, 1500 }  // catapult
};
const troopStat CombatSimulator::baseFortificationStats[5] = {
    { 0, 0, 0, 0, 5000 },         // trap
    { 0, 0, 0, 0, 5000 },         // abatis
    { 2000, 300, 360, 0, 1300 },  // archer tower
    { 0, 500, 0, 0, 1300 },       // rolling logs
    { 0, 800, 0, 0, 5000 }        // trebuchet
};
const float CombatSimulator::damageModifiers[12][12] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // worker
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // warrior
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // scout
    { 0, 0, 0, 0, 1.8, 1.8, 0, 0, 0, 0, 0, 0 },// pike
    { 0, 0, 0, 0, 0, 0, 1.1, 0, 0, 0, 0, 0 },  // swords
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // archer
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // transporter
    { 0, 0, 0, 0, 0, 0, 0, 1.2, 1.2, 0, 0, 0 },// cavalry
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1.2, 0, 0 },  // cataphract
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // ballista
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // ram
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }     // catapult
};

void CombatSimulator::modifyStats(troopStat* base, researchStats res, heroStat hero, float atk_modifier, float def_modifier, float life_modifier) {
    for (int i = 0; i < 12; ++i) {
        base[i].life = (int)((float)base[i].life * life_modifier);
        base[i].attack = (int)((float)base[i].attack * atk_modifier);
        base[i].defense = (int)((float)base[i].defense * def_modifier);
        base[i].speed = (int)((float)base[i].speed * 100);
        base[i].range = (int)((float)base[i].range * 100);
    }
}

int CombatSimulator::compareSpeed(const combatTroops& x, const combatTroops& y) {
    return x.stat->speed > y.stat->speed;
}

float CombatSimulator::calculateDamage(int attackerType, int defenderType, float rawDamage) {
    return rawDamage * damageModifiers[attackerType][defenderType];
}

void CombatSimulator::applyDamage(attacker& atk, defender& def, battleResult& result, int atkType, int defType, int atkCount, int defCount) {
    float dmg = (float)atkType * 50 * 1 * 1;
    float defend_damagem = calculateDamage(atkType, defType, dmg);
    def.wallLevel = 0;
    result.wallHitpoints = (int) (def.wallHitpoints - defend_damagem);
}

void CombatSimulator::fight(attacker& atk, defender& def, battleResult& br) {
    applyDamage(atk,def,br,7,3,1000,2000);
}

void CombatSimulator::simulateBattle(attacker& atk, defender& def, battleResult& result) {
    battleResult res;
    fight(atk, def, res);
    std::cout << "Damage taken: " << res.wallHitpoints << std::endl;
}

void CombatSimulator::simulateRounds(attacker& atk, defender& def, battleResult& result, int numRounds) {
    for (int i = 0; i < numRounds; ++i) {
        simulateBattle(atk, def, result);
    }
}

void CombatSimulator::applyResearchAndHeroStats(attacker& atk, defender& def) {
    researchStats a = atk.research;
    researchStats b = def.research;
    heroStat x = atk.hero;
    heroStat y = def.hero;
    float atkmodifier = 1;
    float defmodifier = 1;
    float lifemodifier = 1;
    modifyStats(atk.troops, a, x, atkmodifier, defmodifier, lifemodifier);
    modifyStats(def.troops, b, y, atkmodifier, defmodifier, lifemodifier);
}

void CombatSimulator::printTroopStats(const attacker& atk, const defender& def) {
    std::cout << "Attacker Troop Stats:\n";
    for (int i = 0; i < 12; ++i) {
        std::cout << "Troop Type " << i << ":\n";
        std::cout << "  Life: " << atk.troops[i] << "\n";
        std::cout << "  Attack: " << atk.troops[i] << "\n";
        std::cout << "  Defense: " << atk.troops[i] << "\n";
        std::cout << "  Speed: " << atk.troops[i] << "\n";
        std::cout << "  Range: " << atk.troops[i] << "\n";
    }

    std::cout << "Defender Troop Stats:\n";
    for (int i = 0; i < 12; ++i) {
        std::cout << "Troop Type " << i << ":\n";
        std::cout << "  Life: " << def.troops[i] << "\n";
        std::cout << "  Attack: " << def.troops[i] << "\n";
        std::cout << "  Defense: " << def.troops[i] << "\n";
        std::cout << "  Speed: " << def.troops[i] << "\n";
        std::cout << "  Range: " << def.troops[i] << "\n";
    }
}

int main() {
    attacker atk;
    defender def;
    battleResult result;

    // Initialize attacker and defender (research, hero stats, troops, fortifications, etc.)

    // Simulate battle
    CombatSimulator::applyResearchAndHeroStats(atk, def);
    CombatSimulator::printTroopStats(atk, def);
    CombatSimulator::simulateRounds(atk, def, result, 5);

    return 0;
}
