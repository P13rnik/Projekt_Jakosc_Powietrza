#include "core/Measurement.hpp"

Measurement::Measurement(int sensorId, const QString& paramCode)
    : m_sensorId(sensorId)
    , m_paramCode(paramCode)
{}

QList<DataPoint> Measurement::validPoints() const {
    QList<DataPoint> result;
    for (const auto& dp : m_dataPoints) {
        if (dp.isValid) result.append(dp);
    }
    return result;
}
