#pragma once
#include <memory>
#include <QVector>
#include <QPair>
#include <QDateTime>
#include "core/Measurement.hpp"
#include "analysis/ITrendStrategy.hpp"

/**
 * @brief Wynik analizy ekstremów (min/max z datami).
 */
struct ExtremesResult {
    double    minValue;
    QDateTime minTime;
    double    maxValue;
    QDateTime maxTime;
};

/**
 * @brief Klasa odpowiedzialna za analizę statystyczną pomiarów jakości powietrza.
 *
 * Implementuje obliczenia: średnia, ekstrema, trend.
 * Strategia trendu jest wymienialna (wzorzec Strategy).
 */
class AirAnalyzer {
public:
    AirAnalyzer();

    /**
     * @brief Ustawia strategię obliczania trendu (wzorzec Strategy).
     * @param strategy Wskaźnik na wybraną implementację ITrendStrategy.
     */
    void setTrendStrategy(std::unique_ptr<ITrendStrategy> strategy);

    /**
     * @brief Oblicza średnią arytmetyczną z prawidłowych pomiarów.
     * @param measurement Obiekt pomiarów czujnika.
     * @return Średnia wartość lub -1.0 jeśli brak danych.
     */
    double calculateAverage(const Measurement& measurement) const;

    /**
     * @brief Oblicza średnią z wybranego zakresu czasowego.
     * @param measurement Obiekt pomiarów.
     * @param from Początek zakresu (włącznie).
     * @param to Koniec zakresu (włącznie).
     * @return Średnia z okresu lub -1.0 jeśli brak danych w zakresie.
     */
    double calculateAverageInRange(const Measurement& measurement,
                                   const QDateTime& from,
                                   const QDateTime& to) const;

    /**
     * @brief Wyznacza wartości ekstremalne z datami wystąpienia.
     * @param measurement Obiekt pomiarów czujnika.
     * @return Struktura ExtremesResult z min/max i datami.
     * @throw DataException jeśli brak prawidłowych danych.
     */
    ExtremesResult findExtremes(const Measurement& measurement) const;

    /**
     * @brief Oblicza trend używając aktualnie ustawionej strategii.
     * @param measurement Obiekt pomiarów czujnika.
     * @return Wynik trendu (nachylenie, intercept, opis).
     * @throw DataException jeśli brak strategii lub danych.
     */
    TrendResult calculateTrend(const Measurement& measurement) const;

private:
    std::unique_ptr<ITrendStrategy> m_trendStrategy;

    QVector<double> extractValidValues(const Measurement& m) const;
};
