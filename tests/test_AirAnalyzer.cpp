#include <gtest/gtest.h>
#include "analysis/AirAnalyzer.hpp"
#include "analysis/LinearTrendStrategy.hpp"
#include "analysis/MovingAverageTrendStrategy.hpp"
#include "utils/AppExceptions.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// Funkcja pomocnicza: tworzy Measurement z podanych wartości
// ─────────────────────────────────────────────────────────────────────────────
static Measurement makeMeasurement(const QList<double>& values,
                                   const QList<bool>& valid = {}) {
    Measurement m(1, "PM10");
    QDateTime base = QDateTime::fromString("2024-01-01 00:00:00",
                                           "yyyy-MM-dd hh:mm:ss");
    for (int i = 0; i < values.size(); ++i) {
        bool isValid = valid.isEmpty() ? true : valid[i];
        m.addDataPoint(DataPoint(base.addSecs(i * 3600), values[i], isValid));
    }
    return m;
}

// ─────────────────────────────────────────────────────────────────────────────
// Testy calculateAverage
// ─────────────────────────────────────────────────────────────────────────────
TEST(AirAnalyzerTest, AverageBasic) {
    AirAnalyzer analyzer;
    auto m = makeMeasurement({10.0, 20.0, 30.0});
    EXPECT_DOUBLE_EQ(analyzer.calculateAverage(m), 20.0);
}

TEST(AirAnalyzerTest, AverageEmpty) {
    AirAnalyzer analyzer;
    Measurement m(1, "PM10");
    EXPECT_DOUBLE_EQ(analyzer.calculateAverage(m), -1.0);
}

TEST(AirAnalyzerTest, AverageSkipsNullValues) {
    AirAnalyzer analyzer;
    // Pomiary: 10 (valid), 0 (null/invalid), 30 (valid) -> średnia = (10+30)/2 = 20
    auto m = makeMeasurement({10.0, 0.0, 30.0}, {true, false, true});
    EXPECT_DOUBLE_EQ(analyzer.calculateAverage(m), 20.0);
}

TEST(AirAnalyzerTest, AverageAllNull) {
    AirAnalyzer analyzer;
    auto m = makeMeasurement({0.0, 0.0, 0.0}, {false, false, false});
    EXPECT_DOUBLE_EQ(analyzer.calculateAverage(m), -1.0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Testy findExtremes
// ─────────────────────────────────────────────────────────────────────────────
TEST(AirAnalyzerTest, ExtremesBasic) {
    AirAnalyzer analyzer;
    auto m = makeMeasurement({15.0, 5.0, 45.0, 30.0});
    auto result = analyzer.findExtremes(m);
    EXPECT_DOUBLE_EQ(result.minValue, 5.0);
    EXPECT_DOUBLE_EQ(result.maxValue, 45.0);
}

TEST(AirAnalyzerTest, ExtremesThrowsOnEmpty) {
    AirAnalyzer analyzer;
    Measurement m(1, "PM10");
    EXPECT_THROW(analyzer.findExtremes(m), DataException);
}

TEST(AirAnalyzerTest, ExtremesSinglePoint) {
    AirAnalyzer analyzer;
    auto m = makeMeasurement({42.0});
    auto result = analyzer.findExtremes(m);
    EXPECT_DOUBLE_EQ(result.minValue, 42.0);
    EXPECT_DOUBLE_EQ(result.maxValue, 42.0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Testy calculateTrend – LinearTrendStrategy
// ─────────────────────────────────────────────────────────────────────────────
TEST(AirAnalyzerTest, LinearTrendRising) {
    AirAnalyzer analyzer;
    analyzer.setTrendStrategy(std::make_unique<LinearTrendStrategy>());
    auto m = makeMeasurement({10.0, 20.0, 30.0, 40.0, 50.0});
    auto result = analyzer.calculateTrend(m);
    EXPECT_GT(result.slope, 0.0);
    EXPECT_EQ(result.description, "Trend rosnący");
}

TEST(AirAnalyzerTest, LinearTrendFalling) {
    AirAnalyzer analyzer;
    analyzer.setTrendStrategy(std::make_unique<LinearTrendStrategy>());
    auto m = makeMeasurement({50.0, 40.0, 30.0, 20.0, 10.0});
    auto result = analyzer.calculateTrend(m);
    EXPECT_LT(result.slope, 0.0);
    EXPECT_EQ(result.description, "Trend malejący");
}

TEST(AirAnalyzerTest, LinearTrendStable) {
    AirAnalyzer analyzer;
    analyzer.setTrendStrategy(std::make_unique<LinearTrendStrategy>());
    auto m = makeMeasurement({20.1, 20.0, 19.9, 20.1, 20.0});
    auto result = analyzer.calculateTrend(m);
    EXPECT_EQ(result.description, "Trend stabilny");
}

// ─────────────────────────────────────────────────────────────────────────────
// Testy calculateTrend – MovingAverageTrendStrategy
// ─────────────────────────────────────────────────────────────────────────────
TEST(AirAnalyzerTest, MovingAverageTrendRising) {
    AirAnalyzer analyzer;
    analyzer.setTrendStrategy(std::make_unique<MovingAverageTrendStrategy>());
    auto m = makeMeasurement({5.0, 10.0, 15.0, 20.0, 25.0, 30.0});
    auto result = analyzer.calculateTrend(m);
    EXPECT_GT(result.slope, 0.0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Testy wzorca Strategy: podmiana strategii w locie
// ─────────────────────────────────────────────────────────────────────────────
TEST(AirAnalyzerTest, StrategySwitch) {
    AirAnalyzer analyzer;
    auto m = makeMeasurement({1.0, 2.0, 3.0, 4.0, 5.0});

    analyzer.setTrendStrategy(std::make_unique<LinearTrendStrategy>());
    auto r1 = analyzer.calculateTrend(m);

    analyzer.setTrendStrategy(std::make_unique<MovingAverageTrendStrategy>());
    auto r2 = analyzer.calculateTrend(m);

    // Obie strategie wykryją trend rosnący, ale mogą dać inne slope
    EXPECT_GT(r1.slope, 0.0);
    EXPECT_GT(r2.slope, 0.0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Testy calculateAverageInRange
// ─────────────────────────────────────────────────────────────────────────────
TEST(AirAnalyzerTest, AverageInRange) {
    AirAnalyzer analyzer;
    auto m = makeMeasurement({10.0, 20.0, 30.0, 40.0, 50.0});

    QDateTime from = QDateTime::fromString("2024-01-01 01:00:00", "yyyy-MM-dd hh:mm:ss");
    QDateTime to   = QDateTime::fromString("2024-01-01 03:00:00", "yyyy-MM-dd hh:mm:ss");

    // Zakres: indeksy 1,2,3 -> wartości 20, 30, 40 -> średnia = 30
    double avg = analyzer.calculateAverageInRange(m, from, to);
    EXPECT_DOUBLE_EQ(avg, 30.0);
}

TEST(AirAnalyzerTest, AverageInRangeEmpty) {
    AirAnalyzer analyzer;
    auto m = makeMeasurement({10.0, 20.0});
    QDateTime from = QDateTime::fromString("2025-01-01 00:00:00", "yyyy-MM-dd hh:mm:ss");
    QDateTime to   = QDateTime::fromString("2025-01-02 00:00:00", "yyyy-MM-dd hh:mm:ss");
    EXPECT_DOUBLE_EQ(analyzer.calculateAverageInRange(m, from, to), -1.0);
}
