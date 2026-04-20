#pragma once
#include <QString>

/**
 * @brief Model czujnika (stanowisko pomiarowe) przypisanego do stacji.
 *
 * Odpowiada obiektowi "sensor" z API GIOŚ. Każda stacja może mieć
 * wiele czujników mierzących różne parametry (PM10, PM2.5, NO2, itd.).
 */
class Sensor {
public:
    Sensor() = default;

    /**
     * @brief Konstruktor inicjalizujący czujnik.
     * @param id Unikalny identyfikator czujnika.
     * @param stationId ID stacji macierzystej.
     * @param paramName Nazwa parametru (np. "PM10").
     * @param paramCode Kod parametru (np. "PM10").
     * @param paramFormula Wzór chemiczny (np. "PM10").
     */
    Sensor(int id, int stationId, const QString& paramName,
           const QString& paramCode, const QString& paramFormula);

    int     id()           const { return m_id; }
    int     stationId()    const { return m_stationId; }
    QString paramName()    const { return m_paramName; }
    QString paramCode()    const { return m_paramCode; }
    QString paramFormula() const { return m_paramFormula; }

private:
    int     m_id          = 0;
    int     m_stationId   = 0;
    QString m_paramName;
    QString m_paramCode;
    QString m_paramFormula;
};
