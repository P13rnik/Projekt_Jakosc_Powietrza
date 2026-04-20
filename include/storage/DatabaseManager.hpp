#pragma once
#include <QString>
#include <QList>
#include "core/Station.hpp"
#include "core/Sensor.hpp"
#include "core/Measurement.hpp"

/**
 * @brief Menedżer lokalnej bazy danych (wzorzec Singleton).
 *
 * Odpowiada za zapis i odczyt danych do/z lokalnych plików JSON.
 * Używany jako fallback gdy brak połączenia z API GIOŚ.
 *
 * Pliki danych:
 *   - data/stations.json                 - lista stacji
 *   - data/sensors/station_{id}.json     - czujniki per stacja
 *   - data/measurements/sensor_{id}.json - pomiary per czujnik
 */
class DatabaseManager {
public:
    /** @brief Zwraca jedyną instancję Singletona. */
    static DatabaseManager& instance();

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    /** @brief Ustawia katalog roboczy i tworzy wszystkie podkatalogi. */
    void setDataDirectory(const QString& path);

    /** @brief Zapisuje listę stacji do stations.json. */
    void saveStations(const QList<Station>& stations);

    /** @brief Wczytuje listę stacji z stations.json. */
    QList<Station> loadStations() const;

    /**
     * @brief Zapisuje listę czujników stacji do sensors/station_{id}.json.
     * @param stationId ID stacji.
     * @param sensors   Lista czujników do zapisania.
     */
    void saveSensors(int stationId, const QList<Sensor>& sensors);

    /**
     * @brief Wczytuje czujniki stacji z cache.
     * @return Lista czujników lub pusta lista jeśli brak cache.
     */
    QList<Sensor> loadSensors(int stationId) const;

    /** @brief Zapisuje pomiary czujnika. */
    void saveMeasurement(const Measurement& measurement);

    /**
     * @brief Wczytuje pomiary czujnika z cache.
     * @return Measurement z danymi lub pusty jeśli brak pliku.
     */
    Measurement loadMeasurement(int sensorId) const;

    /** @brief true jeśli stations.json istnieje. */
    bool hasLocalData() const;

    /** @brief true jeśli cache czujników dla stacji istnieje. */
    bool hasSensorsCache(int stationId) const;

private:
    DatabaseManager() = default;

    QString stationsFilePath() const;
    QString sensorsFilePath(int stationId) const;
    QString measurementFilePath(int sensorId) const;

    QString m_dataDir = "data";
};
