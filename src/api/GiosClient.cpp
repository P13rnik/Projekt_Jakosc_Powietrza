#include "api/GiosClient.hpp"
#include "utils/AppExceptions.hpp"
#include "utils/Logger.hpp"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrl>
#include <stdexcept>

GiosClient::GiosClient(QObject* parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{}

// Wysyła żądanie GET i czeka na odpowiedź (blokująco — wywoływane z wątku roboczego).
// NIE ustawiamy Accept ani Content-Type — API GIOŚ v1 zwraca JSON-LD
// i odrzuca Accept: application/json kodem 406.
QByteArray GiosClient::sendRequest(const QString& endpoint) {
    QUrl url(QString(BASE_URL) + endpoint);
    QNetworkRequest request(url);

    QNetworkReply* reply = m_networkManager->get(request);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(reply,  &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout,         &loop, &QEventLoop::quit);
    timer.start(TIMEOUT_MS);
    loop.exec();

    if (!timer.isActive()) {
        reply->abort();
        reply->deleteLater();
        throw std::runtime_error("Przekroczono limit czasu — serwer GIOŚ nie odpowiada.");
    }

    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (reply->error() != QNetworkReply::NoError) {
        // Przyjazny komunikat zależny od kontekstu endpointu i kodu HTTP
        QString friendly;
        bool isSensors  = endpoint.contains("/station/sensors/");
        bool isData     = endpoint.contains("/data/getData/");
        bool isStations = endpoint.contains("/station/findAll");

        switch (httpStatus) {
        case 400:
            if (isData)
                friendly = "Ta stacja nie udostępnia danych bieżących w API GIOŚ.\n"
                           "Mogą być dostępne tylko dane archiwalne lub stacja\n"
                           "jest wyłączona z sieci automatycznych pomiarów.";
            else if (isSensors)
                friendly = "Nie można pobrać listy czujników tej stacji.\n"
                           "Stacja może nie być aktywna lub nie posiada danych online.";
            else
                friendly = "Serwer GIOŚ odrzucił zapytanie — spróbuj ponownie.";
            break;
        case 404:
            if (isData || isSensors)
                friendly = "Stacja lub czujnik nie został odnaleziony w bazie GIOŚ.";
            else
                friendly = "Nie znaleziono zasobu w API GIOŚ.";
            break;
        case 406:
            friendly = "Błąd komunikacji z API GIOŚ — nieprawidłowy format odpowiedzi.";
            break;
        case 429:
            friendly = "Zbyt wiele zapytań do API GIOŚ.\nOdczekaj chwilę i spróbuj ponownie.";
            break;
        case 500:
            friendly = "Błąd serwera GIOŚ — spróbuj ponownie za kilka minut.";
            break;
        case 503:
            friendly = "Serwis GIOŚ jest tymczasowo niedostępny.\nSpróbuj ponownie później.";
            break;
        case 0:
            // Kod 0 = Qt nie dostał odpowiedzi — prawdziwy brak sieci
            if (isStations)
                friendly = "OFFLINE: Brak połączenia z internetem.\n"
                           "Aplikacja wczyta stacje z lokalnej bazy danych.";
            else
                friendly = "OFFLINE: Brak połączenia z internetem.\n"
                           "Aplikacja wczyta ostatnie zapisane dane.";
            break;
        default:
            // Inne błędy Qt (SSL, DNS, timeout) — również brak sieci
            if (httpStatus == 0)
                friendly = "OFFLINE: Błąd sieci — brak odpowiedzi serwera.\n"
                           "Aplikacja wczyta ostatnie zapisane dane.";
            else
                friendly = QString("Błąd HTTP %1 z API GIOŚ.\n"
                                   "Spróbuj ponownie za chwilę.").arg(httpStatus);
        }
        reply->deleteLater();
        throw std::runtime_error(friendly.toStdString());
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();
    Logger::info("GIOŚ API: " + endpoint + " -> " + QString::number(data.size()) + " B");
    return data;
}

bool GiosClient::isApiAvailable() {
    try {
        sendRequest("/station/findAll?page=0&size=1");
        return true;
    } catch (...) {
        return false;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Stacje: GET /station/findAll?page=N&size=500
// API v1 stronicuje wyniki. W Polsce ~288 stacji — mieszczą się na jednej stronie.
// ─────────────────────────────────────────────────────────────────────────────
QList<Station> GiosClient::fetchAllStations() {
    QList<Station> allStations;
    int page = 0;
    int totalPages = 1;

    while (page < totalPages) {
        QString endpoint = QString("/station/findAll?page=%1&size=500").arg(page);
        QByteArray data  = sendRequest(endpoint);

        QJsonParseError err;
        auto doc = QJsonDocument::fromJson(data, &err);
        if (err.error != QJsonParseError::NoError)
            throw std::runtime_error("Błąd parsowania stacji: " + err.errorString().toStdString());

        auto root = doc.object();
        if (page == 0 && root.contains("totalPages"))
            totalPages = root["totalPages"].toInt(1);

        QJsonArray array;
        if (root.contains("Lista stacji pomiarowych"))
            array = root["Lista stacji pomiarowych"].toArray();
        else if (doc.isArray())
            array = doc.array();

        if (array.isEmpty()) break;

        // const QJsonValue& — fix ostrzeżenia clazy-range-loop-detach (unikamy kopiowania)
        for (const QJsonValue& val : array) {
            QJsonObject obj = val.toObject();
            if (obj["Identyfikator stacji"].isNull()) continue;

            Station s(
                obj["Identyfikator stacji"].toInt(),
                obj["Nazwa stacji"].toString(),
                obj["Nazwa miasta"].toString(),
                obj["WGS84 φ N"].toString().toDouble(),
                obj["WGS84 λ E"].toString().toDouble()
                );
            s.setProvince(obj["Województwo"].toString());
            s.setAddress(obj["Ulica"].toString());
            allStations.append(s);
        }

        Logger::info(QString("Stacje: strona %1/%2, łącznie %3")
                         .arg(page + 1).arg(totalPages).arg(allStations.size()));
        ++page;
    }

    return allStations;
}

QList<Station> GiosClient::parseStations(const QByteArray& json) {
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError)
        throw std::runtime_error("Błąd parsowania stacji: " + err.errorString().toStdString());

    QJsonArray array;
    auto root = doc.object();
    if (root.contains("Lista stacji pomiarowych"))
        array = root["Lista stacji pomiarowych"].toArray();
    else if (doc.isArray())
        array = doc.array();

    QList<Station> stations;
    for (const QJsonValue& val : array) {
        QJsonObject obj = val.toObject();
        if (obj["Identyfikator stacji"].isNull()) continue;
        Station s(
            obj["Identyfikator stacji"].toInt(),
            obj["Nazwa stacji"].toString(),
            obj["Nazwa miasta"].toString(),
            obj["WGS84 φ N"].toString().toDouble(),
            obj["WGS84 λ E"].toString().toDouble()
            );
        s.setProvince(obj["Województwo"].toString());
        s.setAddress(obj["Ulica"].toString());
        stations.append(s);
    }
    return stations;
}

// ─────────────────────────────────────────────────────────────────────────────
// Czujniki stacji: GET /station/sensors/{stationId}
// ─────────────────────────────────────────────────────────────────────────────
QList<Sensor> GiosClient::fetchSensorsForStation(int stationId) {
    QByteArray data = sendRequest("/station/sensors/" + QString::number(stationId));
    return parseSensors(stationId, data);
}

QList<Sensor> GiosClient::parseSensors(int stationId, const QByteArray& json) {
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError)
        throw std::runtime_error("Błąd parsowania czujników stacji "
                                 + std::to_string(stationId));

    QJsonArray array;
    auto root = doc.object();
    if (root.contains("Lista stanowisk pomiarowych dla podanej stacji"))
        array = root["Lista stanowisk pomiarowych dla podanej stacji"].toArray();
    else if (root.contains("Lista stanowisk pomiarowych"))
        array = root["Lista stanowisk pomiarowych"].toArray();
    else if (doc.isArray())
        array = doc.array();

    QList<Sensor> sensors;
    for (const QJsonValue& val : array) {
        QJsonObject obj = val.toObject();
        if (obj["Identyfikator stanowiska"].isNull()) continue;
        sensors.append(Sensor(
            obj["Identyfikator stanowiska"].toInt(),
            obj["Identyfikator stacji"].toInt(),
            obj["Wskaźnik"].toString(),
            obj["Wskaźnik - kod"].toString(),
            obj["Wskaźnik - wzór"].toString()
            ));
    }
    Logger::info(QString("Pobrano %1 czujników dla stacji %2")
                     .arg(sensors.size()).arg(stationId));
    return sensors;
}

// ─────────────────────────────────────────────────────────────────────────────
// Pomiary czujnika: GET /data/getData/{sensorId}
// ─────────────────────────────────────────────────────────────────────────────
Measurement GiosClient::fetchSensorData(int sensorId) {
    QByteArray data = sendRequest("/data/getData/" + QString::number(sensorId));
    return parseMeasurement(sensorId, data);
}

Measurement GiosClient::parseMeasurement(int sensorId, const QByteArray& json) {
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError)
        throw std::runtime_error("Błąd parsowania pomiarów czujnika "
                                 + std::to_string(sensorId));

    auto root = doc.object();
    Measurement m(sensorId, "");

    QJsonArray values;
    if (root.contains("Lista danych pomiarowych"))
        values = root["Lista danych pomiarowych"].toArray();
    else if (root.contains("values"))
        values = root["values"].toArray();
    else if (doc.isArray())
        values = doc.array();

    if (values.isEmpty()) {
        Logger::warning(QString("Brak danych dla czujnika %1").arg(sensorId));
        return m;
    }

    for (const QJsonValue& val : values) {
        QJsonObject obj = val.toObject();

        QString dateStr;
        if      (obj.contains("Data"))  dateStr = obj["Data"].toString();
        else if (obj.contains("data"))  dateStr = obj["data"].toString();
        else                            dateStr = obj["date"].toString();

        dateStr.replace("T", " ");
        if (dateStr.length() > 19) dateStr = dateStr.left(19);

        QDateTime ts = QDateTime::fromString(dateStr, "yyyy-MM-dd HH:mm:ss");
        if (!ts.isValid()) ts = QDateTime::fromString(dateStr, "yyyy-MM-dd HH:mm");

        if (!ts.isValid()) {
            Logger::warning("Nierozpoznany format daty: " + dateStr);
            continue;
        }

        QJsonValue valueJson;
        if      (obj.contains("Wartość"))  valueJson = obj["Wartość"];
        else if (obj.contains("wartość"))  valueJson = obj["wartość"];
        else                               valueJson = obj["value"];

        if (valueJson.isNull() || valueJson.isUndefined())
            m.addDataPoint(DataPoint(ts, 0.0, false));
        else
            m.addDataPoint(DataPoint(ts, valueJson.toDouble(), true));
    }
    return m;
}

// ─────────────────────────────────────────────────────────────────────────────
// Indeks jakości powietrza: GET /aqindex/getIndex/{stationId}
// ─────────────────────────────────────────────────────────────────────────────
AirQualityIndex GiosClient::fetchAirQualityIndex(int stationId) {
    QByteArray data = sendRequest("/aqindex/getIndex/" + QString::number(stationId));
    return parseAirQualityIndex(stationId, data);
}

AirQualityIndex GiosClient::parseAirQualityIndex(int stationId,
                                                 const QByteArray& json) {
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError)
        throw ParseException("Błąd parsowania indeksu stacji "
                             + std::to_string(stationId) + ": "
                             + err.errorString().toStdString());

    auto root = doc.object();
    if (root.isEmpty() || root["id"].isNull())
        return AirQualityIndex();

    AirQualityIndex idx(
        root["id"].toInt(),
        QDateTime::fromString(root["stCalcDate"].toString(), "yyyy-MM-dd HH:mm:ss")
        );
    idx.setSourceDataDate(
        QDateTime::fromString(root["stSourceDataDate"].toString(), "yyyy-MM-dd HH:mm:ss")
        );

    if (!root["stIndexLevel"].isNull()) {
        auto lvl = root["stIndexLevel"].toObject();
        idx.setOverallIndex(IndexLevel{lvl["id"].toInt(), lvl["indexLevelName"].toString()});
    }

    static const QStringList PARAMS = {
        "so2IndexLevel","no2IndexLevel","pm10IndexLevel",
        "pm25IndexLevel","o3IndexLevel","c6h6IndexLevel"
    };
    static const QStringList CODES = {"SO2","NO2","PM10","PM2.5","O3","C6H6"};
    for (int i = 0; i < PARAMS.size(); ++i) {
        if (!root[PARAMS[i]].isNull()) {
            auto lvl = root[PARAMS[i]].toObject();
            idx.addParamIndex(CODES[i], IndexLevel{
                lvl["id"].toInt(), lvl["indexLevelName"].toString()
            });
        }
    }
    return idx;
}
