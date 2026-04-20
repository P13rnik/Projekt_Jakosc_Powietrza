#include "storage/DatabaseManager.hpp"
#include "utils/AppExceptions.hpp"
#include "utils/Logger.hpp"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager inst;
    return inst;
}

// Tworzy wszystkie potrzebne podkatalogi przy starcie aplikacji.
void DatabaseManager::setDataDirectory(const QString& path) {
    m_dataDir = path;
    QDir().mkpath(path);
    QDir().mkpath(path + "/measurements");
    QDir().mkpath(path + "/sensors");
    Logger::info("Katalog danych: " + QDir(path).absolutePath());
}

QString DatabaseManager::stationsFilePath() const {
    return m_dataDir + "/stations.json";
}

QString DatabaseManager::sensorsFilePath(int stationId) const {
    return m_dataDir + "/sensors/station_" + QString::number(stationId) + ".json";
}

QString DatabaseManager::measurementFilePath(int sensorId) const {
    return m_dataDir + "/measurements/sensor_" + QString::number(sensorId) + ".json";
}

bool DatabaseManager::hasLocalData() const {
    return QFile::exists(stationsFilePath());
}

bool DatabaseManager::hasSensorsCache(int stationId) const {
    return QFile::exists(sensorsFilePath(stationId));
}

// ── Stacje ────────────────────────────────────────────────────────────────────

void DatabaseManager::saveStations(const QList<Station>& stations) {
    QJsonArray arr;
    for (const Station& s : stations) {
        QJsonObject obj;
        obj["id"]       = s.id();
        obj["name"]     = s.name();
        obj["city"]     = s.city();
        obj["province"] = s.province();
        obj["address"]  = s.address();
        obj["lat"]      = s.latitude();
        obj["lon"]      = s.longitude();
        arr.append(obj);
    }
    QFile file(stationsFilePath());
    if (!file.open(QIODevice::WriteOnly))
        throw FileException("Nie można zapisać: " + stationsFilePath().toStdString());
    file.write(QJsonDocument(arr).toJson());
    Logger::info("Zapisano " + QString::number(stations.size()) + " stacji do "
                 + stationsFilePath());
}

QList<Station> DatabaseManager::loadStations() const {
    QFile file(stationsFilePath());
    if (!file.exists())
        throw FileException("Plik stacji nie istnieje: " + stationsFilePath().toStdString());
    if (!file.open(QIODevice::ReadOnly))
        throw FileException("Nie można otworzyć: " + stationsFilePath().toStdString());

    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError)
        throw ParseException("Błąd parsowania stations.json: "
                             + err.errorString().toStdString());

    QList<Station> stations;
    for (const QJsonValue& val : doc.array()) {
        QJsonObject obj = val.toObject();
        Station s(obj["id"].toInt(), obj["name"].toString(),
                  obj["city"].toString(),
                  obj["lat"].toDouble(), obj["lon"].toDouble());
        s.setProvince(obj["province"].toString());
        s.setAddress(obj["address"].toString());
        stations.append(s);
    }
    Logger::info("Wczytano " + QString::number(stations.size()) + " stacji z cache.");
    return stations;
}

// ── Czujniki ──────────────────────────────────────────────────────────────────

void DatabaseManager::saveSensors(int stationId, const QList<Sensor>& sensors) {
    QDir().mkpath(m_dataDir + "/sensors");
    QJsonArray arr;
    for (const Sensor& s : sensors) {
        QJsonObject obj;
        obj["id"]        = s.id();
        obj["stationId"] = s.stationId();
        obj["paramName"] = s.paramName();
        obj["paramCode"] = s.paramCode();
        obj["paramFormula"] = s.paramFormula();
        arr.append(obj);
    }
    QFile file(sensorsFilePath(stationId));
    if (!file.open(QIODevice::WriteOnly))
        throw FileException("Nie można zapisać czujników stacji "
                            + std::to_string(stationId));
    file.write(QJsonDocument(arr).toJson());
    Logger::info("Zapisano " + QString::number(sensors.size())
                 + " czujników stacji " + QString::number(stationId));
}

QList<Sensor> DatabaseManager::loadSensors(int stationId) const {
    QFile file(sensorsFilePath(stationId));
    if (!file.exists() || !file.open(QIODevice::ReadOnly))
        return {};  // pusta lista — caller sprawdza

    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError)
        return {};

    QList<Sensor> sensors;
    for (const QJsonValue& val : doc.array()) {
        QJsonObject obj = val.toObject();
        sensors.append(Sensor(
            obj["id"].toInt(),
            obj["stationId"].toInt(),
            obj["paramName"].toString(),
            obj["paramCode"].toString(),
            obj["paramFormula"].toString()
            ));
    }
    Logger::info("Wczytano " + QString::number(sensors.size())
                 + " czujników stacji " + QString::number(stationId) + " z cache.");
    return sensors;
}

// ── Pomiary ───────────────────────────────────────────────────────────────────

void DatabaseManager::saveMeasurement(const Measurement& m) {
    QDir().mkpath(m_dataDir + "/measurements");

    QJsonArray arr;
    for (const DataPoint& dp : m.dataPoints()) {
        QJsonObject obj;
        obj["timestamp"] = dp.timestamp.toString(Qt::ISODate);
        if (dp.isValid) obj["value"] = dp.value;
        else            obj["value"] = QJsonValue::Null;
        arr.append(obj);
    }
    QJsonObject root;
    root["sensorId"]  = m.sensorId();
    root["paramCode"] = m.paramCode();
    root["data"]      = arr;

    QFile file(measurementFilePath(m.sensorId()));
    if (!file.open(QIODevice::WriteOnly))
        throw FileException("Nie można zapisać pomiarów czujnika "
                            + std::to_string(m.sensorId()));
    file.write(QJsonDocument(root).toJson());
    Logger::info("Zapisano pomiary czujnika " + QString::number(m.sensorId())
                 + " (" + QString::number(m.dataPoints().size()) + " punktów)");
}

Measurement DatabaseManager::loadMeasurement(int sensorId) const {
    QFile file(measurementFilePath(sensorId));
    if (!file.exists() || !file.open(QIODevice::ReadOnly))
        return Measurement(sensorId, "");

    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError)
        throw ParseException("Błąd parsowania pomiarów czujnika "
                             + std::to_string(sensorId) + ": "
                             + err.errorString().toStdString());

    auto root = doc.object();
    Measurement m(sensorId, root["paramCode"].toString());

    for (const QJsonValue& val : root["data"].toArray()) {
        QJsonObject obj = val.toObject();
        QDateTime ts = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
        if (obj["value"].isNull())
            m.addDataPoint(DataPoint(ts, 0.0, false));
        else
            m.addDataPoint(DataPoint(ts, obj["value"].toDouble(), true));
    }
    Logger::info("Wczytano " + QString::number(m.dataPoints().size())
                 + " punktów z cache (czujnik " + QString::number(sensorId) + ")");
    return m;
}
