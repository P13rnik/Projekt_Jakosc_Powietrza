#include <gtest/gtest.h>
#include "core/Sensor.hpp"
#include "core/Measurement.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// Testy klasy Sensor
// ─────────────────────────────────────────────────────────────────────────────

TEST(SensorTest, ConstructorSetsAllFields) {
    Sensor s(101, 5, "pył zawieszony PM10", "PM10", "PM10");
    EXPECT_EQ(s.id(),           101);
    EXPECT_EQ(s.stationId(),    5);
    EXPECT_EQ(s.paramName(),    "pył zawieszony PM10");
    EXPECT_EQ(s.paramCode(),    "PM10");
    EXPECT_EQ(s.paramFormula(), "PM10");
}

TEST(SensorTest, DefaultConstructorZeroFields) {
    Sensor s;
    EXPECT_EQ(s.id(),        0);
    EXPECT_EQ(s.stationId(), 0);
    EXPECT_TRUE(s.paramName().isEmpty());
    EXPECT_TRUE(s.paramCode().isEmpty());
}

TEST(SensorTest, DifferentParamFormulas) {
    Sensor no2(200, 10, "dwutlenek azotu", "NO2", "NO2");
    EXPECT_EQ(no2.paramFormula(), "NO2");

    Sensor o3(201, 10, "ozon", "O3", "O3");
    EXPECT_EQ(o3.paramFormula(), "O3");

    Sensor c6h6(202, 10, "benzen", "C6H6", "C6H6");
    EXPECT_EQ(c6h6.paramFormula(), "C6H6");
}

// ─────────────────────────────────────────────────────────────────────────────
// Testy klasy Measurement – obsługa DataPoint i validPoints()
// ─────────────────────────────────────────────────────────────────────────────

TEST(MeasurementTest, EmptyMeasurement) {
    Measurement m(1, "PM10");
    EXPECT_TRUE(m.isEmpty());
    EXPECT_EQ(m.dataPoints().size(), 0);
    EXPECT_EQ(m.validPoints().size(), 0);
}

TEST(MeasurementTest, ConstructorFields) {
    Measurement m(42, "NO2");
    EXPECT_EQ(m.sensorId(),  42);
    EXPECT_EQ(m.paramCode(), "NO2");
}

TEST(MeasurementTest, AddSingleValidPoint) {
    Measurement m(1, "PM10");
    QDateTime ts = QDateTime::fromString("2024-01-01 10:00:00",
                                         "yyyy-MM-dd hh:mm:ss");
    m.addDataPoint(DataPoint(ts, 35.5, true));
    EXPECT_FALSE(m.isEmpty());
    EXPECT_EQ(m.dataPoints().size(), 1);
    EXPECT_EQ(m.validPoints().size(), 1);
    EXPECT_DOUBLE_EQ(m.dataPoints().first().value, 35.5);
}

TEST(MeasurementTest, ValidPointsFiltersNulls) {
    Measurement m(1, "PM10");
    QDateTime base = QDateTime::fromString("2024-01-01 00:00:00",
                                           "yyyy-MM-dd hh:mm:ss");
    m.addDataPoint(DataPoint(base,                10.0, true));
    m.addDataPoint(DataPoint(base.addSecs(3600),   0.0, false)); // null
    m.addDataPoint(DataPoint(base.addSecs(7200),  20.0, true));
    m.addDataPoint(DataPoint(base.addSecs(10800),  0.0, false)); // null
    m.addDataPoint(DataPoint(base.addSecs(14400), 30.0, true));

    EXPECT_EQ(m.dataPoints().size(), 5);
    EXPECT_EQ(m.validPoints().size(), 3);
}

TEST(MeasurementTest, AllNullsGivesEmptyValidPoints) {
    Measurement m(1, "PM2.5");
    QDateTime ts = QDateTime::currentDateTime();
    m.addDataPoint(DataPoint(ts, 0.0, false));
    m.addDataPoint(DataPoint(ts.addSecs(3600), 0.0, false));
    EXPECT_EQ(m.dataPoints().size(), 2);
    EXPECT_EQ(m.validPoints().size(), 0);
}

TEST(MeasurementTest, DataPointIsValidFlag) {
    DataPoint valid(QDateTime::currentDateTime(), 12.5, true);
    DataPoint invalid(QDateTime::currentDateTime(), 0.0, false);
    EXPECT_TRUE(valid.isValid);
    EXPECT_FALSE(invalid.isValid);
}

TEST(MeasurementTest, LargeMeasurementAllValid) {
    Measurement m(99, "SO2");
    QDateTime base = QDateTime::fromString("2024-01-01 00:00:00",
                                           "yyyy-MM-dd hh:mm:ss");
    // Symulacja 7 dni pomiarów co godzinę = 168 punktów
    for (int i = 0; i < 168; ++i)
        m.addDataPoint(DataPoint(base.addSecs(i * 3600), 5.0 + i * 0.1, true));

    EXPECT_EQ(m.dataPoints().size(), 168);
    EXPECT_EQ(m.validPoints().size(), 168);
}

TEST(MeasurementTest, MixedValidityPreservesOrder) {
    Measurement m(1, "PM10");
    QDateTime base = QDateTime::fromString("2024-06-01 00:00:00",
                                           "yyyy-MM-dd hh:mm:ss");
    // Wzorzec: valid, null, null, valid, valid
    QList<bool> validity = {true, false, false, true, true};
    QList<double> values = {10.0, 0.0, 0.0, 20.0, 30.0};

    for (int i = 0; i < 5; ++i)
        m.addDataPoint(DataPoint(base.addSecs(i * 3600), values[i], validity[i]));

    auto valid = m.validPoints();
    ASSERT_EQ(valid.size(), 3);
    EXPECT_DOUBLE_EQ(valid[0].value, 10.0);
    EXPECT_DOUBLE_EQ(valid[1].value, 20.0);
    EXPECT_DOUBLE_EQ(valid[2].value, 30.0);

    // Sprawdź że kolejność czasowa jest zachowana
    EXPECT_LT(valid[0].timestamp, valid[1].timestamp);
    EXPECT_LT(valid[1].timestamp, valid[2].timestamp);
}

// ─────────────────────────────────────────────────────────────────────────────
// Testy DataPoint
// ─────────────────────────────────────────────────────────────────────────────

TEST(DataPointTest, DefaultValidIsTrue) {
    QDateTime ts = QDateTime::currentDateTime();
    DataPoint dp(ts, 42.0);
    EXPECT_TRUE(dp.isValid);
    EXPECT_DOUBLE_EQ(dp.value, 42.0);
    EXPECT_EQ(dp.timestamp, ts);
}

TEST(DataPointTest, ExplicitInvalid) {
    QDateTime ts = QDateTime::currentDateTime();
    DataPoint dp(ts, 0.0, false);
    EXPECT_FALSE(dp.isValid);
}


