#pragma once
#include <QString>
#include <QDateTime>
#include <QList>
#include <QPair>

/**
 * @brief Pojedynczy punkt pomiarowy (czas + wartość).
 */
struct DataPoint {
    QDateTime timestamp;
    double    value;
    bool      isValid; ///< false gdy API zwróciło null

    DataPoint(const QDateTime& ts, double v, bool valid = true)
        : timestamp(ts), value(v), isValid(valid) {}
};

/**
 * @brief Kolekcja pomiarów dla jednego czujnika.
 *
 * Odpowiada odpowiedzi endpointu /getData/{sensorId} z API GIOŚ.
 */
class Measurement {
public:
    Measurement() = default;

    /**
     * @brief Konstruktor z ID czujnika i nazwą parametru.
     * @param sensorId ID czujnika.
     * @param paramCode Kod parametru (np. "PM10").
     */
    Measurement(int sensorId, const QString& paramCode);

    int     sensorId()  const { return m_sensorId; }
    QString paramCode() const { return m_paramCode; }
    /** @brief Ustawia kod parametru (API getData nie zwraca go — bierzemy z sensorów). */
    void setParamCode(const QString& code) { m_paramCode = code; }

    const QList<DataPoint>& dataPoints() const { return m_dataPoints; }
    void addDataPoint(const DataPoint& dp) { m_dataPoints.append(dp); }

    /**
     * @brief Zwraca tylko punkty z prawidłowymi wartościami (nie-null).
     * @return Lista prawidłowych punktów pomiarowych.
     */
    QList<DataPoint> validPoints() const;

    /**
     * @brief Sprawdza czy kolekcja zawiera jakiekolwiek dane.
     * @return true jeśli lista punktów nie jest pusta.
     */
    bool isEmpty() const { return m_dataPoints.isEmpty(); }

private:
    int     m_sensorId  = 0;
    QString m_paramCode;
    QList<DataPoint> m_dataPoints;
};
