#include "analysis/AirAnalyzer.hpp"
#include "analysis/LinearTrendStrategy.hpp"
#include "utils/AppExceptions.hpp"
#include <algorithm>
#include <climits>

AirAnalyzer::AirAnalyzer()
    : m_trendStrategy(std::make_unique<LinearTrendStrategy>())
{}

void AirAnalyzer::setTrendStrategy(std::unique_ptr<ITrendStrategy> strategy) {
    m_trendStrategy = std::move(strategy);
}

double AirAnalyzer::calculateAverage(const Measurement& measurement) const {
    auto points = measurement.validPoints();
    if (points.isEmpty()) return -1.0;
    double sum = 0;
    for (const auto& dp : points) sum += dp.value;
    return sum / points.size();
}

// Porównanie przez msec unikamy problemów ze strefami czasowymi
double AirAnalyzer::calculateAverageInRange(const Measurement& measurement,
                                            const QDateTime& from,
                                            const QDateTime& to) const {
    qint64 fromMs = from.isValid() ? from.toMSecsSinceEpoch() : LLONG_MIN;
    qint64 toMs   = to.isValid()   ? to.toMSecsSinceEpoch()   : LLONG_MAX;
    double sum = 0; int count = 0;
    for (const auto& dp : measurement.validPoints()) {
        qint64 ts = dp.timestamp.toMSecsSinceEpoch();
        if (ts >= fromMs && ts <= toMs) { sum += dp.value; ++count; }
    }
    return count > 0 ? sum / count : -1.0;
}

ExtremesResult AirAnalyzer::findExtremes(const Measurement& measurement) const {
    auto points = measurement.validPoints();
    if (points.isEmpty())
        throw DataException("Brak prawidłowych danych do wyznaczenia ekstremów.");

    ExtremesResult result;
    result.minValue = points.first().value; result.minTime = points.first().timestamp;
    result.maxValue = points.first().value; result.maxTime = points.first().timestamp;

    for (const auto& dp : points) {
        if (dp.value < result.minValue) { result.minValue = dp.value; result.minTime = dp.timestamp; }
        if (dp.value > result.maxValue) { result.maxValue = dp.value; result.maxTime = dp.timestamp; }
    }
    return result;
}

TrendResult AirAnalyzer::calculateTrend(const Measurement& measurement) const {
    if (!m_trendStrategy)
        throw DataException("Nie ustawiono strategii trendu.");
    auto values = extractValidValues(measurement);
    if (values.isEmpty())
        throw DataException("Brak danych do obliczenia trendu.");
    return m_trendStrategy->calculate(values);
}

// BUG FIX: sortuj chronologicznie przed przekazaniem do strategii trendu.
// API GIOŚ zwraca dane od najnowszego do najstarszego — bez sortowania
// regresja liniowa i średnia krocząca obliczają trend odwrotnie.
QVector<double> AirAnalyzer::extractValidValues(const Measurement& m) const {
    QList<DataPoint> points = m.validPoints();
    std::sort(points.begin(), points.end(), [](const DataPoint& a, const DataPoint& b) {
        return a.timestamp.toMSecsSinceEpoch() < b.timestamp.toMSecsSinceEpoch();
    });
    QVector<double> values;
    values.reserve(points.size());
    for (const auto& dp : points) values.append(dp.value);
    return values;
}
