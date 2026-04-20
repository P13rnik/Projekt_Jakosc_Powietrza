#include "gui/BatchDownloader.hpp"
#include "api/GiosClient.hpp"
#include "storage/DatabaseManager.hpp"
#include "utils/Logger.hpp"
#include <QtConcurrent/QtConcurrent>
#include <QThread>

BatchDownloader::BatchDownloader(QObject* parent)
    : QObject(parent)
{}

// Pobiera dane dla wszystkich stacji sekwencyjnie w osobnym wątku.
// Sekwencyjnie (nie równolegle) żeby nie przeciążyć API GIOŚ.
void BatchDownloader::start(const QList<Station>& stations) {
    if (m_running) return;
    m_running    = true;
    m_cancelled  = false;

    int total = stations.size();

    QtConcurrent::run([this, stations, total]() {
        int downloaded = 0;

        for (int i = 0; i < total; ++i) {
            if (m_cancelled) {
                QMetaObject::invokeMethod(this, [this, downloaded, total]() {
                    m_running = false;
                    emit finished(downloaded, total, true);
                }, Qt::QueuedConnection);
                return;
            }

            const Station& station = stations[i];
            int percent = (i * 100) / total;

            QMetaObject::invokeMethod(this, [this, percent, station]() {
                emit progressChanged(percent);
                emit statusChanged(
                    QString("Pobieranie: %1 (%2)")
                        .arg(station.name())
                        .arg(station.city()));
            }, Qt::QueuedConnection);

            try {
                GiosClient client;

                // Pobierz czujniki stacji
                QList<Sensor> sensors = client.fetchSensorsForStation(station.id());
                if (sensors.isEmpty()) continue;

                // Zapisz czujniki do cache
                DatabaseManager::instance().saveSensors(station.id(), sensors);

                // Pobierz dane dla każdego czujnika stacji
                for (const Sensor& sensor : sensors) {
                    if (m_cancelled) break;
                    try {
                        Measurement m = client.fetchSensorData(sensor.id());
                        m.setParamCode(sensor.paramCode());
                        DatabaseManager::instance().saveMeasurement(m);
                        Logger::info("Cache: stacja " + QString::number(station.id())
                                     + " czujnik " + sensor.paramCode());
                    } catch (...) {
                        // Jeden czujnik nie odpowiada — pomijamy, idziemy dalej
                    }
                    // Krótka pauza żeby nie przeciążyć API (rate limit)
                    QThread::msleep(100);
                }

                ++downloaded;

            } catch (...) {
                // Stacja niedostępna — pomijamy
                Logger::warning("Batch: pominięto stację " + QString::number(station.id()));
            }
        }

        QMetaObject::invokeMethod(this, [this, downloaded, total]() {
            m_running = false;
            emit progressChanged(100);
            emit finished(downloaded, total, false);
        }, Qt::QueuedConnection);
    });
}

void BatchDownloader::cancel() {
    m_cancelled = true;
}
