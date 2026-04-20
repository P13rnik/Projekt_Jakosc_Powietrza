#pragma once
#include "analysis/ITrendStrategy.hpp"

/**
 * @brief Strategia trendu liniowego (regresja liniowa metodą najmniejszych kwadratów).
 *
 * Oblicza nachylenie prostej najlepszego dopasowania do danych.
 * Wzorzec Strategy - implementacja interfejsu ITrendStrategy.
 */
class LinearTrendStrategy : public ITrendStrategy {
public:
    /**
     * @brief Oblicza trend liniowy metodą regresji.
     * @param values Wektor wartości pomiarowych.
     * @return TrendResult z nachyleniem, interceptem i opisem.
     */
    TrendResult calculate(const QVector<double>& values) const override;

    QString name() const override { return "Regresja liniowa"; }
};
