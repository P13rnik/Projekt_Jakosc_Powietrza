#include "api/AsyncFetcher.hpp"
#include "api/GiosClient.hpp"
#include "storage/DatabaseManager.hpp"
#include "utils/Logger.hpp"
#include <QtConcurrent/QtConcurrent>
#include <stdexcept>

AsyncFetcher::AsyncFetcher(QObject* parent)
    : QObject(parent)
    , m_stationWatcher(new QFutureWatcher<QList<Station>>(this))
    , m_measurementWatcher(new QFutureWatcher<Measurement>(this))
{
    setupWatchers();
}

AsyncFetcher::~AsyncFetcher() {}

// Podłącza watchery do sygnałów Qt — wzorzec Observer.
// WIELOWĄTKOWOŚĆ: QtConcurrent::run() uruchamia żądania HTTP w puli wątków Qt.
void AsyncFetcher::setupWatchers() {

    connect(m_stationWatcher, &QFutureWatcher<QList<Station>>::finished,
            this, [this]() {
                m_isFetching = false;
                try {
                    emit stationsFetched(m_stationWatcher->result(), false);
                } catch (const std::exception& e) {
                    Logger::warning("Błąd API stacji, próba cache: " + QString(e.what()));
                    try {
                        auto& db = DatabaseManager::instance();
                        if (db.hasLocalData()) {
                            emit stationsFetched(db.loadStations(), true);
                            return;
                        }
                    } catch (...) {}
                    emit fetchError(QString(e.what()));
                } catch (...) {
                    emit fetchError("Nieznany błąd podczas pobierania stacji.");
                }
            });

    connect(m_measurementWatcher, &QFutureWatcher<Measurement>::finished,
            this, [this]() {
                m_isFetching = false;
                try {
                    Measurement m = m_measurementWatcher->result();
                    // sensorId == -1: wartownik, błąd już wysłany przez invokeMethod
                    if (m.sensorId() != -1)
                        emit measurementFetched(m);
                } catch (const std::exception& e) {
                    emit fetchError(QString(e.what()));
                } catch (...) {
                    emit fetchError("Nieznany błąd podczas pobierania pomiarów.");
                }
            });
}

void AsyncFetcher::fetchStationsAsync() {
    if (m_isFetching) return;
    m_isFetching = true;
    emit progressChanged(0);

    QFuture<QList<Station>> future = QtConcurrent::run([]() -> QList<Station> {
        GiosClient client;
        return client.fetchAllStations();
    });

    m_stationWatcher->setFuture(future);
    emit progressChanged(10);
}

// Pobiera pomiary dla stacji/czujnika w wątku roboczym.
//
// Przepływ ONLINE:
//   1. Pobierz listę czujników (potrzebna do paramCode)
//   2. Zapisz czujniki do cache (data/sensors/station_{id}.json)
//   3. Pobierz dane pomiarowe
//   4. Zapisz pomiary do cache (data/measurements/sensor_{id}.json)
//
// Przepływ OFFLINE (gdy krok 1 lub 3 rzuca wyjątek):
//   1. Wczytaj czujniki z cache (data/sensors/station_{id}.json)
//   2. Ustal resolvedSensorId z czujników
//   3. Wczytaj pomiary z cache (data/measurements/sensor_{id}.json)
//   4. Emituj offlineModeActive + measurementFetched
//
// To rozwiązuje problem gdy specificSensorId==0 przy braku sieci:
// wcześniej nie wiedzieliśmy jakiego sensorId szukać w cache pomiarów.
void AsyncFetcher::fetchMeasurementAsync(int stationId, int specificSensorId) {
    if (m_isFetching) return;
    m_isFetching = true;

    QFuture<Measurement> future = QtConcurrent::run(
        [this, stationId, specificSensorId]() -> Measurement {

            int     resolvedSensorId  = specificSensorId;
            QString resolvedParamCode;

            // ── Krok 1: pobierz czujniki (online lub z cache) ─────────────────
            QList<Sensor> sensors;
            bool sensorsFromCache = false;

            try {
                GiosClient client;
                sensors = client.fetchSensorsForStation(stationId);

                if (!sensors.isEmpty()) {
                    // Zapisz czujniki do cache — potrzebne przy następnym offline
                    try {
                        DatabaseManager::instance().saveSensors(stationId, sensors);
                    } catch (...) {}
                }
            } catch (const std::exception&) {
                // Brak sieci — wczytaj czujniki z cache
                Logger::warning("Brak sieci — wczytuję czujniki z cache (stacja "
                                + QString::number(stationId) + ")");
                sensors = DatabaseManager::instance().loadSensors(stationId);
                sensorsFromCache = true;
            }

            if (sensors.isEmpty()) {
                // Ani online ani cache — błąd
                QString msg = sensorsFromCache
                                  ? "Brak połączenia z internetem i brak danych lokalnych dla tej stacji."
                                  : QString("Stacja %1 nie posiada czujników.").arg(stationId);
                QMetaObject::invokeMethod(this, [this, msg]() {
                    emit fetchError(msg);
                }, Qt::QueuedConnection);
                return Measurement(-1, "");
            }

            // ── Krok 2: ustal który czujnik ───────────────────────────────────
            if (resolvedSensorId <= 0) {
                resolvedSensorId  = sensors.first().id();
                resolvedParamCode = sensors.first().paramCode();
            } else {
                for (const Sensor& s : sensors) {
                    if (s.id() == resolvedSensorId) {
                        resolvedParamCode = s.paramCode();
                        break;
                    }
                }
                if (resolvedParamCode.isEmpty()) {
                    resolvedSensorId  = sensors.first().id();
                    resolvedParamCode = sensors.first().paramCode();
                }
            }

            // ── Krok 3: pobierz pomiary (online lub z cache) ──────────────────
            try {
                GiosClient client;
                Measurement m = client.fetchSensorData(resolvedSensorId);

                if (!resolvedParamCode.isEmpty())
                    m.setParamCode(resolvedParamCode);

                // Zapisz pomiary do cache
                try { DatabaseManager::instance().saveMeasurement(m); } catch (...) {}

                // Jeśli czujniki były z cache to jesteśmy w trybie offline
                if (sensorsFromCache) {
                    QMetaObject::invokeMethod(this, [this]() {
                        emit offlineModeActive();
                    }, Qt::QueuedConnection);
                }

                return m;

            } catch (const std::exception& e) {
                QString msg = e.what();
                Logger::warning("Błąd API pomiarów: " + msg + " — próba cache");

                // Wczytaj pomiary z cache
                try {
                    Measurement cached = DatabaseManager::instance()
                    .loadMeasurement(resolvedSensorId);
                    if (!cached.isEmpty()) {
                        Logger::info("Cache: czujnik " + QString::number(resolvedSensorId));
                        QMetaObject::invokeMethod(this, [this]() {
                            emit offlineModeActive();
                        }, Qt::QueuedConnection);
                        return cached;
                    }
                } catch (...) {}

                // Cache pusty
                QMetaObject::invokeMethod(this, [this, msg]() {
                    emit fetchError(msg);
                }, Qt::QueuedConnection);
                return Measurement(-1, "");
            }
        });

    m_measurementWatcher->setFuture(future);
}

void AsyncFetcher::cancel() {
    m_stationWatcher->cancel();
    m_measurementWatcher->cancel();
    m_isFetching = false;
}
