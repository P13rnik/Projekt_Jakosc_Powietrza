#pragma once
#include <QObject>
#include <QFuture>
#include <QFutureWatcher>
#include <QList>
#include "core/Station.hpp"
#include "core/Measurement.hpp"
#include "core/Sensor.hpp"

/**
 * @brief Asynchroniczne pobieranie danych z API GIOŚ — nie blokuje GUI.
 *
 * WIELOWĄTKOWOŚĆ: QtConcurrent::run() uruchamia każde żądanie HTTP
 * w osobnym wątku z puli wątków Qt. GUI wątek nigdy nie czeka na sieć.
 * QFutureWatcher emituje sygnały Qt gdy wątek kończy pracę (wzorzec Observer).
 *
 * Obsługa offline: przy błędzie sieci automatycznie próbuje wczytać
 * ostatnie zapisane dane z lokalnego cache JSON.
 */
class AsyncFetcher : public QObject {
    Q_OBJECT

public:
    explicit AsyncFetcher(QObject* parent = nullptr);
    ~AsyncFetcher();

    /**
     * @brief Pobiera wszystkie stacje w tle. Przy błędzie sieci — cache.
     */
    void fetchStationsAsync();

    /**
     * @brief Pobiera pomiary w tle. Przy błędzie sieci — cache.
     * @param stationId       ID stacji pomiarowej.
     * @param specificSensorId ID czujnika (0 = użyj pierwszego dostępnego).
     */
    void fetchMeasurementAsync(int stationId, int specificSensorId = 0);

    /** @brief Anuluje bieżące pobieranie. */
    void cancel();

    bool isFetching() const { return m_isFetching; }

signals:
    /** @brief Stacje pobrane (fromCache=true gdy z lokalnej bazy). */
    void stationsFetched(const QList<Station>& stations, bool fromCache);

    /** @brief Pomiary pobrane lub wczytane z cache. */
    void measurementFetched(const Measurement& measurement);

    /** @brief Dane pochodzą z lokalnego cache — brak połączenia z internetem. */
    void offlineModeActive();

    /** @brief Ani API ani cache nie dostarczyły danych — opis problemu. */
    void fetchError(const QString& errorMessage);

    /** @brief Postęp pobierania 0-100 do paska postępu. */
    void progressChanged(int percent);

private:
    QFutureWatcher<QList<Station>>* m_stationWatcher;
    QFutureWatcher<Measurement>*    m_measurementWatcher;
    bool m_isFetching = false;

    void setupWatchers();
};
