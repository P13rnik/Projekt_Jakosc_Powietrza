#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QList>
#include "core/Station.hpp"
#include "core/Sensor.hpp"
#include "core/Measurement.hpp"
#include "core/AirQualityIndex.hpp"

/**
 * @brief Klient REST API GIOŚ (Główny Inspektorat Ochrony Środowiska).
 *
 * Bazowy URL API: https://api.gios.gov.pl/pjp-api/v1/rest/
 * Endpointy:
 *   - GET /station/findAll         - lista wszystkich stacji
 *   - GET /station/sensors/{id}    - czujniki (stanowiska) danej stacji
 *   - GET /data/getData/{id}       - pomiary konkretnego czujnika
 *   - GET /index/getIndex/{id}     - indeks jakości powietrza dla stacji
 */
class GiosClient : public QObject {
    Q_OBJECT

public:
    explicit GiosClient(QObject* parent = nullptr);

    /**
     * @brief Pobiera listę wszystkich stacji pomiarowych (synchronicznie).
     *
     * Używane wewnątrz AsyncFetcher w osobnym wątku.
     * @return Lista stacji z API.
     * @throw NetworkException jeśli serwer jest niedostępny.
     * @throw ParseException jeśli odpowiedź JSON jest nieprawidłowa.
     */
    QList<Station> fetchAllStations();

    /**
     * @brief Pobiera listę czujników (stanowisk) przypisanych do stacji.
     * @param stationId Unikalny identyfikator stacji.
     * @return Lista czujników danej stacji.
     * @throw NetworkException jeśli serwer jest niedostępny.
     * @throw ParseException jeśli odpowiedź JSON jest nieprawidłowa.
     */
    QList<Sensor> fetchSensorsForStation(int stationId);

    /**
     * @brief Pobiera pomiary dla konkretnego czujnika (synchronicznie).
     * @param sensorId Unikalny identyfikator czujnika.
     * @return Obiekt Measurement z historią pomiarów.
     * @throw NetworkException jeśli serwer jest niedostępny.
     * @throw ParseException jeśli odpowiedź JSON jest nieprawidłowa.
     */
    Measurement fetchSensorData(int sensorId);

    /**
     * @brief Pobiera indeks jakości powietrza dla stacji.
     * @param stationId Unikalny identyfikator stacji.
     * @return Obiekt AirQualityIndex z indeksem ogólnym i cząstkowymi.
     * @throw NetworkException jeśli serwer jest niedostępny.
     * @throw ParseException jeśli odpowiedź JSON jest nieprawidłowa.
     */
    AirQualityIndex fetchAirQualityIndex(int stationId);

    /**
     * @brief Sprawdza dostępność API (ping).
     * @return true jeśli API odpowiada w czasie TIMEOUT_MS.
     */
    bool isApiAvailable();

private:
    QByteArray      sendRequest(const QString& endpoint);
    QList<Station>  parseStations(const QByteArray& json);
    QList<Sensor>   parseSensors(int stationId, const QByteArray& json);
    Measurement     parseMeasurement(int sensorId, const QByteArray& json);
    AirQualityIndex parseAirQualityIndex(int stationId, const QByteArray& json);

    QNetworkAccessManager* m_networkManager;
    static constexpr auto BASE_URL   = "https://api.gios.gov.pl/pjp-api/v1/rest";
    static constexpr int  TIMEOUT_MS = 10000;
};
