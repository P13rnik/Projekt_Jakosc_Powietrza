#include "analysis/LinearTrendStrategy.hpp"
#include <cmath>

/**
 * @brief Oblicza trend metodą regresji liniowej (metoda najmniejszych kwadratów).
 *
 * Dopasowuje prostą y = ax + b do danych pomiarowych.
 * x = indeks próbki (0, 1, 2, ...), y = wartość pomiaru w µg/m³.
 * Slope (a) > 0 oznacza trend rosnący, a < 0 malejący.
 * Dane muszą być posortowane chronologicznie — AirAnalyzer to gwarantuje.
 */
TrendResult LinearTrendStrategy::calculate(const QVector<double>& values) const {
    TrendResult result;
    result.slope = 0.0;
    result.intercept = 0.0;
    result.description = "Trend stabilny (regresja)";

    int n = values.size();
    if (n < 2) {
        result.description = "Za mało danych";
        return result;
    }

    double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
    for (int i = 0; i < n; ++i) {
        double x = static_cast<double>(i);
        double y = values[i];
        sumX  += x;
        sumY  += y;
        sumXY += x * y;
        sumX2 += x * x;
    }

    double denom = n * sumX2 - sumX * sumX;
    if (std::abs(denom) < 1e-10) {
        result.description = "Brak trendu";
        return result;
    }

    result.slope     = (n * sumXY - sumX * sumY) / denom;
    result.intercept = (sumY - result.slope * sumX) / n;

    // Próg 0.5 µg/m³ na punkt — spójny ze średnią kroczącą, eliminuje szum pomiarowy
    if (std::abs(result.slope) < 0.5)
        result.description = QStringLiteral("Trend stabilny");
    else if (result.slope > 0)
        result.description = QStringLiteral("Trend rosnący");
    else
        result.description = QStringLiteral("Trend malejący");

    return result;
}

