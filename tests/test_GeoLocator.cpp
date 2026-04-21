#include <gtest/gtest.h>
#include "analysis/GeoLocator.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// Testy wzoru haversine
// ─────────────────────────────────────────────────────────────────────────────

TEST(GeoLocatorTest, SamePointDistanceIsZero) {
    double dist = GeoLocator::haversineDistance(52.0, 21.0, 52.0, 21.0);
    EXPECT_NEAR(dist, 0.0, 0.001);
}

TEST(GeoLocatorTest, WarsawKrakow) {
    // Warszawa: 52.2297° N, 21.0122° E
    // Kraków:   50.0647° N, 19.9450° E
    // Rzeczywista odległość: ~252 km
    double dist = GeoLocator::haversineDistance(52.2297, 21.0122, 50.0647, 19.9450);
    EXPECT_NEAR(dist, 252.0, 5.0); // tolerancja ±5 km
}

TEST(GeoLocatorTest, PoznanWroclaw) {
    // Poznań: 52.4069° N, 16.9299° E
    // Wrocław: 51.1079° N, 17.0385° E
    // Rzeczywista odległość: ~148 km
    double dist = GeoLocator::haversineDistance(52.4069, 16.9299, 51.1079, 17.0385);
    EXPECT_NEAR(dist, 148.0, 5.0);
}

TEST(GeoLocatorTest, IsSymmetric) {
    double d1 = GeoLocator::haversineDistance(52.0, 21.0, 50.0, 19.0);
    double d2 = GeoLocator::haversineDistance(50.0, 19.0, 52.0, 21.0);
    EXPECT_NEAR(d1, d2, 0.0001);
}

TEST(GeoLocatorTest, ShortDistanceAccuracy) {
    // ~1 km: różnica ~0.009° szerokości ≈ 1 km
    double dist = GeoLocator::haversineDistance(52.000, 21.000, 52.009, 21.000);
    EXPECT_NEAR(dist, 1.0, 0.1);
}

// ─────────────────────────────────────────────────────────────────────────────
// Testy findNearest
// ─────────────────────────────────────────────────────────────────────────────

static QList<Station> makeSampleStations() {
    QList<Station> s;
    s.append(Station(1, "Stacja A", "Poznań",   52.4069, 16.9299)); // ~0 km od Poznania
    s.append(Station(2, "Stacja B", "Wrocław",  51.1079, 17.0385)); // ~148 km
    s.append(Station(3, "Stacja C", "Warszawa", 52.2297, 21.0122)); // ~280 km
    s.append(Station(4, "Stacja D", "Gdańsk",   54.3520, 18.6466)); // ~248 km
    return s;
}

TEST(GeoLocatorTest, FindNearestReturnsAllWhenNoLimit) {
    auto stations = makeSampleStations();
    auto result = GeoLocator::findNearest(stations, 52.4069, 16.9299, 0.0);
    ASSERT_EQ(result.size(), 4);
    // Pierwsza powinna być Stacja A (0 km)
    EXPECT_EQ(result.first().id(), 1);
}

TEST(GeoLocatorTest, FindNearestSortedByDistance) {
    auto stations = makeSampleStations();
    auto result = GeoLocator::findNearest(stations, 52.4069, 16.9299, 0.0);
    // Odległości powinny rosnąć
    for (int i = 1; i < result.size(); ++i) {
        double d1 = GeoLocator::haversineDistance(52.4069, 16.9299,
                                                   result[i-1].latitude(),
                                                   result[i-1].longitude());
        double d2 = GeoLocator::haversineDistance(52.4069, 16.9299,
                                                   result[i].latitude(),
                                                   result[i].longitude());
        EXPECT_LE(d1, d2);
    }
}

TEST(GeoLocatorTest, FindNearestWithRadiusFilter) {
    auto stations = makeSampleStations();
    // Promień 200 km od Poznania: powinna być tylko A (0 km) i B (~148 km)
    auto result = GeoLocator::findNearest(stations, 52.4069, 16.9299, 200.0);
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].id(), 1); // Poznań
    EXPECT_EQ(result[1].id(), 2); // Wrocław
}

TEST(GeoLocatorTest, FindNearestEmptyInputReturnsEmpty) {
    QList<Station> empty;
    auto result = GeoLocator::findNearest(empty, 52.0, 21.0, 100.0);
    EXPECT_TRUE(result.isEmpty());
}

TEST(GeoLocatorTest, FindNearestNothingInRadius) {
    auto stations = makeSampleStations();
    // Promień 10 km od środka Polski – żadna ze stacji nie jest tak blisko
    auto result = GeoLocator::findNearest(stations, 52.1, 19.5, 10.0);
    EXPECT_TRUE(result.isEmpty());
}

