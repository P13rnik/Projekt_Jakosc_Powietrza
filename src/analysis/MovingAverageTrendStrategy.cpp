#include "analysis/MovingAverageTrendStrategy.hpp"
#include <cmath>

/**
 * @brief Oblicza trend metodą średniej kroczącej (moving average).
 *
 * Krok 1 — wygładzanie: każdy punkt zastępowany średnią z sąsiedniego
 * okna (szerokość = windowSize z konstruktora, domyślnie 3).
 * Redukuje szum pomiarowy lepiej niż regresja liniowa.
 * Krok 2 — kierunek: porównuje średnią pierwszej i ostatniej ćwiartki
 * wygładzonej serii. Różnica tych średnich = slope.
 * Dane muszą być posortowane chronologicznie — AirAnalyzer to gwarantuje.
 */
TrendResult MovingAverageTrendStrategy::calculate(const QVector<double>& values) const {
    TrendResult result;
    result.slope = 0.0;
    result.intercept = 0.0;

    int n = values.size();
    if (n < 4) {
        result.description = QString::fromUtf8(
            "Za ma\xc5\x82""o danych (\xc5\x9br. kroc.)");
        return result;
    }

    // Krok 1: wygładzanie oknem kroczącym o szerokości m_windowSize
    int half = std::max(1, m_windowSize / 2);
    QVector<double> smoothed(n);
    for (int i = 0; i < n; ++i) {
        int start = std::max(0, i - half);
        int end   = std::min(n - 1, i + half);
        double sum = 0.0;
        for (int j = start; j <= end; ++j)
            sum += values[j];
        smoothed[i] = sum / (end - start + 1);
    }

    // Krok 2: porównaj pierwszą i ostatnią ćwiartkę wygładzonej serii
    int quarter = std::max(1, n / 4);
    double firstAvg = 0.0, lastAvg = 0.0;
    for (int i = 0; i < quarter; ++i)
        firstAvg += smoothed[i];
    for (int i = n - quarter; i < n; ++i)
        lastAvg += smoothed[i];
    firstAvg /= quarter;
    lastAvg  /= quarter;

    result.slope     = lastAvg - firstAvg;
    result.intercept = firstAvg;

    if (std::abs(result.slope) < 0.5)
        result.description = QString::fromUtf8(
            "Trend stabilny (śr. kroc.)");
    else if (result.slope > 0)
        result.description = QString::fromUtf8(
            "Trend rosnący (śr. kroc.)");
    else
        result.description = QString::fromUtf8(
            "Trend malejący (śr. kroc.)");

    return result;
}

