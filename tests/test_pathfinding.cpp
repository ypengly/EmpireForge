#include "test_framework.hpp"

#include "World/Pathfinding.hpp"
#include "World/TileMap.hpp"

using namespace ef;

namespace {
// Builds a blank all-plains map (generate() would add random terrain).
TileMap makeOpenMap(int w, int h) {
    TileMap map(w, h, 32);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) map.at({ x, y }).terrain = TerrainType::Plains;
    return map;
}
}

EF_TEST(Pathfinding_FindsStraightPathOnOpenGround) {
    TileMap map = makeOpenMap(10, 10);
    auto path = Pathfinding::findPath(map, { 0, 0 }, { 5, 0 });
    EF_ASSERT_TRUE(!path.empty());
    EF_ASSERT_TRUE(path.front() == (TileCoord{ 0, 0 }));
    EF_ASSERT_TRUE(path.back() == (TileCoord{ 5, 0 }));
}

EF_TEST(Pathfinding_RoutesAroundWaterBarrier) {
    TileMap map = makeOpenMap(10, 10);
    // Vertical river at x=5 with a single gap at y=9.
    for (int y = 0; y < 9; ++y) map.at({ 5, y }).terrain = TerrainType::River;

    auto path = Pathfinding::findPath(map, { 0, 0 }, { 9, 0 });
    EF_ASSERT_TRUE(!path.empty());
    for (const auto& c : path) EF_ASSERT_TRUE(map.at(c).isWalkable());
}

EF_TEST(Pathfinding_ReturnsEmptyWhenFullyBlocked) {
    TileMap map = makeOpenMap(10, 10);
    for (int y = 0; y < 10; ++y) map.at({ 5, y }).terrain = TerrainType::Water;

    auto path = Pathfinding::findPath(map, { 0, 0 }, { 9, 0 });
    EF_ASSERT_TRUE(path.empty());
}

EF_TEST(Pathfinding_RejectsUnwalkableGoalAndOutOfBounds) {
    TileMap map = makeOpenMap(10, 10);
    map.at({ 3, 3 }).terrain = TerrainType::Water;
    EF_ASSERT_TRUE(Pathfinding::findPath(map, { 0, 0 }, { 3, 3 }).empty());
    EF_ASSERT_TRUE(Pathfinding::findPath(map, { 0, 0 }, { 50, 50 }).empty());
}

EF_TEST(Pathfinding_PrefersRoadsOverForest) {
    TileMap map = makeOpenMap(10, 3);
    // Row 0 is forest, row 1 is road; path along the road should be chosen.
    for (int x = 0; x < 10; ++x) {
        map.at({ x, 0 }).terrain = TerrainType::Forest;
        map.at({ x, 1 }).terrain = TerrainType::Road;
        map.at({ x, 2 }).terrain = TerrainType::Forest;
    }
    auto path = Pathfinding::findPath(map, { 0, 1 }, { 9, 1 });
    EF_ASSERT_TRUE(!path.empty());
    for (const auto& c : path) EF_ASSERT_EQ(c.y, 1);
}
