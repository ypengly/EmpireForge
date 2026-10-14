#include "test_framework.hpp"

#include "Combat/CombatSystem.hpp"
#include "Military/Army.hpp"
#include "Military/UnitDatabase.hpp"

using namespace ef;

namespace {
UnitDatabase loadUnits() {
    UnitDatabase db;
    db.loadFromFile("data/units.json");
    return db;
}
}

EF_TEST(Army_AddRemoveMergeSplit) {
    Army a(1, "A", 0, TileCoord{ 0, 0 });
    a.addUnits("infantry", 10);
    a.addUnits("archer", 5);
    EF_ASSERT_EQ(a.totalUnitCount(), 15);

    EF_ASSERT_TRUE(a.removeUnits("infantry", 4));
    EF_ASSERT_EQ(a.totalUnitCount(), 11);
    EF_ASSERT_TRUE(!a.removeUnits("infantry", 100));

    Army b(2, "B", 0, TileCoord{ 0, 0 });
    b.addUnits("cavalry", 3);
    a.mergeFrom(b);
    EF_ASSERT_EQ(a.totalUnitCount(), 14);
    EF_ASSERT_EQ(b.totalUnitCount(), 0);

    Army split = a.split(3, { { "archer", 2 } });
    EF_ASSERT_EQ(split.totalUnitCount(), 2);
    EF_ASSERT_EQ(a.totalUnitCount(), 12);
}

EF_TEST(Combat_LargerArmyUsuallyWinsOnEqualTerrain) {
    UnitDatabase db = loadUnits();
    Army attacker(1, "Big", 0, TileCoord{ 0, 0 });
    attacker.addUnits("infantry", 50);
    Army defender(2, "Small", 1, TileCoord{ 1, 0 });
    defender.addUnits("infantry", 10);

    BattleReport report = CombatSystem::resolveBattle(attacker, defender, db, TerrainType::Plains);
    EF_ASSERT_TRUE(report.attackerWon);
}

EF_TEST(Combat_CavalryCountersArchers) {
    UnitDatabase db = loadUnits();

    // Equal unit counts, but cavalry counters archers per units.json, so the
    // cavalry side's effective strength should exceed its raw strength.
    Army cav(1, "Cav", 0, TileCoord{ 0, 0 });
    cav.addUnits("cavalry", 20);
    Army archers(2, "Archers", 1, TileCoord{ 1, 0 });
    archers.addUnits("archer", 20);

    double rawCav = cav.totalStrength(db);
    BattleReport report = CombatSystem::resolveBattle(cav, archers, db, TerrainType::Plains);
    EF_ASSERT_TRUE(report.attackerFinalStrength > rawCav);
}

EF_TEST(Combat_MountainTerrainBoostsDefender) {
    UnitDatabase db = loadUnits();

    Army attackerPlains(1, "A1", 0, TileCoord{ 0, 0 });
    attackerPlains.addUnits("infantry", 20);
    Army defenderPlains(2, "D1", 1, TileCoord{ 1, 0 });
    defenderPlains.addUnits("infantry", 20);
    BattleReport onPlains = CombatSystem::resolveBattle(attackerPlains, defenderPlains, db, TerrainType::Plains);

    Army attackerMountain(3, "A2", 0, TileCoord{ 0, 0 });
    attackerMountain.addUnits("infantry", 20);
    Army defenderMountain(4, "D2", 1, TileCoord{ 1, 0 });
    defenderMountain.addUnits("infantry", 20);
    BattleReport onMountain = CombatSystem::resolveBattle(attackerMountain, defenderMountain, db, TerrainType::Mountain);

    EF_ASSERT_TRUE(onMountain.defenderFinalStrength > onPlains.defenderFinalStrength);
}

EF_TEST(Combat_BothSidesTakeLosses) {
    UnitDatabase db = loadUnits();
    Army attacker(1, "A", 0, TileCoord{ 0, 0 });
    attacker.addUnits("infantry", 30);
    Army defender(2, "D", 1, TileCoord{ 1, 0 });
    defender.addUnits("infantry", 30);

    CombatSystem::resolveBattle(attacker, defender, db, TerrainType::Plains);
    EF_ASSERT_TRUE(attacker.totalUnitCount() < 30);
    EF_ASSERT_TRUE(defender.totalUnitCount() < 30);
}

EF_TEST(Commander_LevelsUpWithExperience) {
    Commander c;
    EF_ASSERT_EQ(c.level, 1);
    c.gainExperience(250.0);
    EF_ASSERT_TRUE(c.level >= 2);
}
