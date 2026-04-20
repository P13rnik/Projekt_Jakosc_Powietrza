#include "core/AirQualityIndex.hpp"

AirQualityIndex::AirQualityIndex(int stationId, const QDateTime& calcDate)
    : m_stationId(stationId)
    , m_calcDate(calcDate)
{}

void AirQualityIndex::addParamIndex(const QString& paramCode, const IndexLevel& level) {
    m_paramIndices.insert(paramCode, level);
}

IndexLevel AirQualityIndex::paramIndex(const QString& paramCode) const {
    return m_paramIndices.value(paramCode, IndexLevel{-1, "Brak danych"});
}
