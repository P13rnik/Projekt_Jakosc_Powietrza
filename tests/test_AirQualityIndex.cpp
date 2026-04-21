#include <gtest/gtest.h>
#include "core/AirQualityIndex.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// Testy konstruktora i pól podstawowych
// ─────────────────────────────────────────────────────────────────────────────

TEST(AirQualityIndexTest, DefaultConstructorIsInvalid) {
    AirQualityIndex idx;
    EXPECT_FALSE(idx.isValid());
    EXPECT_EQ(idx.stationId(), 0);
}

TEST(AirQualityIndexTest, ParameterizedConstructorIsValid) {
    QDateTime dt = QDateTime::fromString("2024-06-01 12:00:00", "yyyy-MM-dd hh:mm:ss");
    AirQualityIndex idx(42, dt);
    EXPECT_TRUE(idx.isValid());
    EXPECT_EQ(idx.stationId(), 42);
    EXPECT_EQ(idx.calcDate(), dt);
}

// ─────────────────────────────────────────────────────────────────────────────
// Testy overallIndex – poziomy skali 0-5
// ─────────────────────────────────────────────────────────────────────────────

TEST(AirQualityIndexTest, OverallIndexDefaultId) {
    AirQualityIndex idx(1, QDateTime::currentDateTime());
    EXPECT_EQ(idx.overallIndex().id, -1); // Domyślnie: "Brak danych"
}

TEST(AirQualityIndexTest, SetOverallIndex) {
    AirQualityIndex idx(1, QDateTime::currentDateTime());
    idx.setOverallIndex(IndexLevel{2, "Umiarkowany"});
    EXPECT_EQ(idx.overallIndex().id, 2);
    EXPECT_EQ(idx.overallIndex().name, "Umiarkowany");
}

// ─────────────────────────────────────────────────────────────────────────────
// Testy kolorów poziomów indeksu
// ─────────────────────────────────────────────────────────────────────────────

TEST(AirQualityIndexTest, ColorLevel0IsGreen) {
    IndexLevel lvl{0, "Bardzo dobry"};
    EXPECT_EQ(lvl.color(), "#4CAF50");
}

TEST(AirQualityIndexTest, ColorLevel3IsOrange) {
    IndexLevel lvl{3, "Dostateczny"};
    EXPECT_EQ(lvl.color(), "#FF9800");
}

TEST(AirQualityIndexTest, ColorLevel5IsPurple) {
    IndexLevel lvl{5, "Bardzo zły"};
    EXPECT_EQ(lvl.color(), "#9C27B0");
}

TEST(AirQualityIndexTest, ColorUnknownIsGray) {
    IndexLevel lvl{99, "Nieznany"};
    EXPECT_EQ(lvl.color(), "#9E9E9E");
}

// ─────────────────────────────────────────────────────────────────────────────
// Testy indeksów cząstkowych per parametr
// ─────────────────────────────────────────────────────────────────────────────

TEST(AirQualityIndexTest, AddAndGetParamIndex) {
    AirQualityIndex idx(1, QDateTime::currentDateTime());
    idx.addParamIndex("PM10",   IndexLevel{1, "Dobry"});
    idx.addParamIndex("NO2",    IndexLevel{3, "Dostateczny"});
    idx.addParamIndex("PM2.5",  IndexLevel{0, "Bardzo dobry"});

    EXPECT_EQ(idx.paramIndex("PM10").id,   1);
    EXPECT_EQ(idx.paramIndex("NO2").id,    3);
    EXPECT_EQ(idx.paramIndex("PM2.5").id,  0);
}

TEST(AirQualityIndexTest, MissingParamIndexReturnsDefault) {
    AirQualityIndex idx(1, QDateTime::currentDateTime());
    auto result = idx.paramIndex("SO2"); // nie dodane
    EXPECT_EQ(result.id, -1);
    EXPECT_EQ(result.name, "Brak danych");
}

TEST(AirQualityIndexTest, OverwriteParamIndex) {
    AirQualityIndex idx(1, QDateTime::currentDateTime());
    idx.addParamIndex("O3", IndexLevel{1, "Dobry"});
    idx.addParamIndex("O3", IndexLevel{4, "Zły"});  // nadpisanie
    EXPECT_EQ(idx.paramIndex("O3").id, 4);
}

TEST(AirQualityIndexTest, AllSixStandardParams) {
    AirQualityIndex idx(5, QDateTime::currentDateTime());
    const QStringList codes = {"SO2", "NO2", "PM10", "PM2.5", "O3", "C6H6"};
    for (int i = 0; i < codes.size(); ++i)
        idx.addParamIndex(codes[i], IndexLevel{i % 6, "Poziom " + QString::number(i)});

    for (int i = 0; i < codes.size(); ++i)
        EXPECT_EQ(idx.paramIndex(codes[i]).id, i % 6) << codes[i].toStdString();
}

// ─────────────────────────────────────────────────────────────────────────────
// Testy setSourceDataDate
// ─────────────────────────────────────────────────────────────────────────────

TEST(AirQualityIndexTest, SourceDataDate) {
    AirQualityIndex idx(10, QDateTime::currentDateTime());
    QDateTime srcDate = QDateTime::fromString("2024-01-15 08:00:00",
                                              "yyyy-MM-dd hh:mm:ss");
    idx.setSourceDataDate(srcDate);
    EXPECT_EQ(idx.sourceDataDate(), srcDate);
}
