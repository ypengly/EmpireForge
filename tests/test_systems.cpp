#include <cstdio>

#include "test_framework.hpp"

#include "Diplomacy/DiplomacyManager.hpp"
#include "SaveSystem/SaveManager.hpp"
#include "Technology/TechnologyManager.hpp"

using namespace ef;

EF_TEST(Diplomacy_RelationsAreSymmetricAndClamped) {
    DiplomacyManager d;
    d.adjustRelation(1, 2, 150.0, "huge gift");
    EF_ASSERT_NEAR(d.relation(2, 1).value, 100.0, 0.001);
    d.adjustRelation(2, 1, -500.0, "insult");
    EF_ASSERT_NEAR(d.relation(1, 2).value, -100.0, 0.001);
}

EF_TEST(Diplomacy_WarAndAllianceStatus) {
    DiplomacyManager d;
    d.setStatus(0, 1, DiplomaticStatus::War, "declared");
    EF_ASSERT_TRUE(d.areAtWar(1, 0));
    d.setStatus(0, 1, DiplomaticStatus::Alliance, "peace treaty");
    EF_ASSERT_TRUE(d.areAllied(0, 1));
    EF_ASSERT_TRUE(!d.areAtWar(0, 1));
}

EF_TEST(Technology_RequiresPrerequisites) {
    TechnologyDatabase db;
    EF_ASSERT_TRUE(db.loadFromFile("data/technologies.json"));
    TechnologyManager mgr;
    Kingdom k(0, "T", true);

    EF_ASSERT_TRUE(mgr.isResearchable("improved_farming", k, db));
    EF_ASSERT_TRUE(!mgr.isResearchable("food_storage", k, db));

    k.researchTechnology("improved_farming");
    EF_ASSERT_TRUE(mgr.isResearchable("food_storage", k, db));
    EF_ASSERT_TRUE(!mgr.isResearchable("improved_farming", k, db)); // already known

    TechnologyEffects fx = mgr.computeCombinedEffects(k, db);
    EF_ASSERT_TRUE(fx.foodProductionMultiplier > 1.0);
}

EF_TEST(SaveManager_RoundTripPreservesKingdomState) {
    const std::string dbPath = "test_saves.db";
    std::remove(dbPath.c_str());

    std::vector<Kingdom> kingdoms;
    Kingdom player(0, "Hero Realm", true);
    player.setPopulation(42);
    player.setCapital({ 5, 6 });
    ResourceBundle res;
    res[ResourceType::Gold] = 777.0;
    player.resources().setAmounts(res);
    player.addBuilding(10, "farm", { 5, 7 }, true);
    Army& army = player.addArmy(11, "First Army", { 5, 6 });
    army.addUnits("infantry", 9);
    player.researchTechnology("improved_farming");
    kingdoms.push_back(std::move(player));

    DiplomacyManager diplomacy;
    diplomacy.setStatus(0, 1, DiplomaticStatus::War, "test");

    {
        SaveManager sm(dbPath);
        EF_ASSERT_TRUE(sm.isOpen());
        SaveableWorldState s;
        s.mapSeed = 1234; s.mapWidth = 60; s.mapHeight = 45; s.gameYear = 7;
        s.nextEntityId = 99; s.kingdoms = &kingdoms; s.diplomacy = &diplomacy;
        EF_ASSERT_TRUE(sm.saveToSlot(3, s));
        EF_ASSERT_EQ(static_cast<int>(sm.listSlots().size()), 1);
    }

    std::vector<Kingdom> loaded;
    DiplomacyManager loadedDip;
    SaveManager sm2(dbPath);
    SaveableWorldState out;
    out.kingdoms = &loaded; out.diplomacy = &loadedDip;
    EF_ASSERT_TRUE(sm2.loadFromSlot(3, out));

    EF_ASSERT_EQ(out.gameYear, 7);
    EF_ASSERT_EQ(static_cast<int>(out.mapSeed), 1234);
    EF_ASSERT_EQ(static_cast<int>(loaded.size()), 1);
    EF_ASSERT_EQ(loaded[0].population(), 42);
    EF_ASSERT_NEAR(loaded[0].resources().amounts()[ResourceType::Gold], 777.0, 0.001);
    EF_ASSERT_EQ(static_cast<int>(loaded[0].buildings().size()), 1);
    EF_ASSERT_EQ(static_cast<int>(loaded[0].armies().size()), 1);
    EF_ASSERT_TRUE(loaded[0].hasTechnology("improved_farming"));
    EF_ASSERT_TRUE(loadedDip.areAtWar(0, 1));

    std::remove(dbPath.c_str());
}
