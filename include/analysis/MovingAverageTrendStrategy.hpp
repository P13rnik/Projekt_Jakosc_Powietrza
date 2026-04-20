#pragma once
#include "analysis/ITrendStrategy.hpp"

/**
 * @brief Strategia trendu oparta na średniej kroczącej.
 *
 * Porównuje średnią z pierwszej i ostatniej połowy danych.
 * Wzorzec Strategy - implementacja interfejsu ITrendStrategy.
 */
class MovingAverageTrendStrategy : public ITrendStrategy {
public:
    /**
     * @brief Konstruktor z parametrem okna średniej kroczącej.
     * @param windowSize Rozmiar okna (domyślnie 3).
     */
    explicit MovingAverageTrendStrategy(int windowSize = 3)
        : m_windowSize(windowSize) {}

    /**
     * @brief Oblicza trend przez porównanie średnich z połówek danych.
     * @param values Wektor wartości pomiarowych.
     * @return TrendResult z opisem kierunku trendu.
     */
    TrendResult calculate(const QVector<double>& values) const override;

    QString name() const override { return "Średnia krocząca"; }

private:
    int m_windowSize;
};
