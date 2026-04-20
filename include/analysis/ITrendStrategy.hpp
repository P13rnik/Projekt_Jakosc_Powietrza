#pragma once
#include <QVector>
#include <QString>

/**
 * @brief Wynik obliczenia trendu.
 */
struct TrendResult {
    double slope;       ///< Nachylenie trendu (wartość/godzinę)
    double intercept;   ///< Punkt przecięcia z osią Y
    QString description; ///< Tekstowy opis trendu ("rosnący", "malejący", "stabilny")
};

/**
 * @brief Interfejs strategii obliczania trendu (wzorzec Strategy).
 *
 * Pozwala na wymienną podmianę algorytmu trendu bez zmiany klasy AirAnalyzer.
 * Zaimplementowane strategie: LinearTrendStrategy, MovingAverageTrendStrategy.
 */
class ITrendStrategy {
public:
    virtual ~ITrendStrategy() = default;

    /**
     * @brief Oblicza trend dla podanego ciągu wartości.
     * @param values Wektor wartości pomiarowych (chronologicznie).
     * @return Obiekt TrendResult z wynikami obliczeń.
     */
    virtual TrendResult calculate(const QVector<double>& values) const = 0;

    /**
     * @brief Zwraca nazwę strategii.
     * @return Nazwa algorytmu trendu.
     */
    virtual QString name() const = 0;
};
