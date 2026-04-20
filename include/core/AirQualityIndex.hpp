#pragma once
#include <QString>
#include <QDateTime>

/**
 * @brief Poziom indeksu jakości powietrza (skala GIOŚ: 0-5).
 */
struct IndexLevel {
    int     id;               ///< Poziom numeryczny (0=b.dobry .. 5=b.zły)
    QString name;             ///< Nazwa tekstowa (np. "Bardzo dobry")

    /**
     * @brief Zwraca kolor QSS odpowiadający poziomowi indeksu.
     * @return Łańcuch koloru CSS (np. "#4CAF50").
     */
    QString color() const {
        switch (id) {
            case 0: return "#4CAF50"; // Bardzo dobry - zielony
            case 1: return "#8BC34A"; // Dobry        - jasno-zielony
            case 2: return "#FFEB3B"; // Umiarkowany  - żółty
            case 3: return "#FF9800"; // Dostateczny  - pomarańczowy
            case 4: return "#F44336"; // Zły          - czerwony
            case 5: return "#9C27B0"; // Bardzo zły   - fioletowy
            default: return "#9E9E9E";
        }
    }
};

/**
 * @brief Model indeksu jakości powietrza dla stacji pomiarowej.
 *
 * Odpowiada odpowiedzi endpointu /index/getIndex/{stationId} z API GIOŚ.
 * Zawiera ogólny indeks dla stacji oraz indeksy cząstkowe per parametr.
 */
class AirQualityIndex {
public:
    AirQualityIndex() = default;

    /**
     * @brief Konstruktor z ID stacji i datą obliczenia indeksu.
     * @param stationId ID stacji pomiarowej.
     * @param calcDate Data i czas obliczenia indeksu.
     */
    AirQualityIndex(int stationId, const QDateTime& calcDate);

    int             stationId()       const { return m_stationId; }
    QDateTime       calcDate()        const { return m_calcDate; }
    QDateTime       sourceDataDate()  const { return m_sourceDataDate; }
    IndexLevel      overallIndex()    const { return m_overallIndex; }

    void setStationId(int id)                         { m_stationId = id; }
    void setCalcDate(const QDateTime& dt)             { m_calcDate = dt; }
    void setSourceDataDate(const QDateTime& dt)       { m_sourceDataDate = dt; }
    void setOverallIndex(const IndexLevel& lvl)       { m_overallIndex = lvl; }

    /**
     * @brief Dodaje indeks cząstkowy dla konkretnego parametru.
     * @param paramCode Kod parametru (np. "PM10").
     * @param level Poziom indeksu dla tego parametru.
     */
    void addParamIndex(const QString& paramCode, const IndexLevel& level);

    /**
     * @brief Zwraca indeks dla konkretnego parametru.
     * @param paramCode Kod parametru.
     * @return IndexLevel lub domyślny (id=-1) jeśli brak.
     */
    IndexLevel paramIndex(const QString& paramCode) const;

    /**
     * @brief Sprawdza czy obiekt zawiera prawidłowe dane.
     * @return true jeśli stationId > 0.
     */
    bool isValid() const { return m_stationId > 0; }

private:
    int        m_stationId  = 0;
    QDateTime  m_calcDate;
    QDateTime  m_sourceDataDate;
    IndexLevel m_overallIndex{-1, "Brak danych"};
    QMap<QString, IndexLevel> m_paramIndices; ///< Indeksy per parametr
};
