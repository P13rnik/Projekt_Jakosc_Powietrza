#include "core/Sensor.hpp"

Sensor::Sensor(int id, int stationId, const QString& paramName,
               const QString& paramCode, const QString& paramFormula)
    : m_id(id)
    , m_stationId(stationId)
    , m_paramName(paramName)
    , m_paramCode(paramCode)
    , m_paramFormula(paramFormula)
{}
