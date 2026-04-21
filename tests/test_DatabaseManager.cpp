#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include "storage/DatabaseManager.hpp"
#include "utils/AppExceptions.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// Fixture: tymczasowy katalog dla każdego testu
// ─────────────────────────────────────────────────────────────────────────────
class DatabaseManagerTest : public ::testing::Test {
protected:
    QTemporaryDir tmpDir;

    void SetUp() override {
        ASSERT_TRUE(tmpDir.isValid());
        DatabaseManager::instance().setDataDirectory(tmpDir.path());
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// Testy zapisu i odczytu stacji
// ─────────────────────────────────────────────────────────────────────────────
TEST_F(DatabaseManagerTest, SaveAndLoadStations) {
    QList<Station> stations;
    stations.append(Station(1, "Stacja Centrum", "Warszawa", 52.23, 21.01));
    stations.append(Station(2, "Stacja Południe", "Kraków",  50.06, 19.94));
    stations[0].setProvince("mazowieckie");
    stations[1].setProvince("małopolskie");

    DatabaseManager::instance().saveStations(stations);

    auto loaded = DatabaseManager::instance().loadStations();
    ASSERT_EQ(loaded.size(), 2);
    EXPECT_EQ(loaded[0].id(),   1);
    EXPECT_EQ(loaded[0].name(), "Stacja Centrum");
    EXPECT_EQ(loaded[0].city(), "Warszawa");
    EXPECT_DOUBLE_EQ(loaded[0].latitude(),  52.23);
    EXPECT_DOUBLE_EQ(loaded[0].longitude(), 21.01);
    EXPECT_EQ(loaded[0].province(), "mazowieckie");

    EXPECT_EQ(loaded[1].id(),   2);
    EXPECT_EQ(loaded[1].city(), "Kraków");
}

TEST_F(DatabaseManagerTest, HasLocalDataFalseWhenNoFile) {
    // Świeży katalog tymczasowy – brak pliku stations.json
    EXPECT_FALSE(DatabaseManager::instance().hasLocalData());
}

TEST_F(DatabaseManagerTest, HasLocalDataTrueAfterSave) {
    QList<Station> stations;
    stations.append(Station(1, "Test", "TestCity", 0.0, 0.0));
    DatabaseManager::instance().saveStations(stations);
    EXPECT_TRUE(DatabaseManager::instance().hasLocalData());
}

TEST_F(DatabaseManagerTest, LoadStationsThrowsWhenFileAbsent) {
    EXPECT_THROW(DatabaseManager::instance().loadStations(), FileException);
}

// ─────────────────────────────────────────────────────────────────────────────
// Testy parsowania JSON – obsługa null w wartościach pomiarów
// ─────────────────────────────────────────────────────────────────────────────
TEST_F(DatabaseManagerTest, SaveAndLoadMeasurementWithNulls) {
    Measurement m(42, "PM10");
    QDateTime base = QDateTime::fromString("2024-06-01T10:00:00", Qt::ISODate);
    m.addDataPoint(DataPoint(base,                  25.5, true));
    m.addDataPoint(DataPoint(base.addSecs(3600),     0.0, false)); // null
    m.addDataPoint(DataPoint(base.addSecs(7200),    30.0, true));
    m.addDataPoint(DataPoint(base.addSecs(10800),    0.0, false)); // null
    m.addDataPoint(DataPoint(base.addSecs(14400),   18.0, true));

    DatabaseManager::instance().saveMeasurement(m);
    auto loaded = DatabaseManager::instance().loadMeasurement(42);

    ASSERT_EQ(loaded.dataPoints().size(), 5);

    // Sprawdź prawidłowe punkty
    EXPECT_TRUE(loaded.dataPoints()[0].isValid);
    EXPECT_DOUBLE_EQ(loaded.dataPoints()[0].value, 25.5);

    // Sprawdź null-e
    EXPECT_FALSE(loaded.dataPoints()[1].isValid);
    EXPECT_FALSE(loaded.dataPoints()[3].isValid);

    // Sprawdź że validPoints() pomija null-e
    auto valid = loaded.validPoints();
    ASSERT_EQ(valid.size(), 3);
    EXPECT_DOUBLE_EQ(valid[0].value, 25.5);
    EXPECT_DOUBLE_EQ(valid[1].value, 30.0);
    EXPECT_DOUBLE_EQ(valid[2].value, 18.0);
}

TEST_F(DatabaseManagerTest, LoadMeasurementReturnsEmptyWhenAbsent) {
    auto m = DatabaseManager::instance().loadMeasurement(9999);
    EXPECT_TRUE(m.isEmpty());
}

TEST_F(DatabaseManagerTest, ParseExceptionOnCorruptJson) {
    // Zapisz nieprawidłowy JSON do pliku pomiarów
    QFile file(tmpDir.path() + "/measurements/sensor_55.json");
    QDir().mkpath(tmpDir.path() + "/measurements");
    file.open(QIODevice::WriteOnly);
    file.write("{ this is: not valid json ]]]");
    file.close();

    EXPECT_THROW(DatabaseManager::instance().loadMeasurement(55), ParseException);
}
