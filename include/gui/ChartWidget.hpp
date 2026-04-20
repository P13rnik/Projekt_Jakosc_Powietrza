#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include "core/Measurement.hpp"

/**
 * @brief Widget wykresu pomiarów jakości powietrza.
 *
 * Wyświetla dane jako wykres liniowy z osią czasową.
 * Obsługuje filtrowanie zakresu dat i kolorowanie wg skali GIOŚ.
 */
class ChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit ChartWidget(QWidget* parent = nullptr);

    /**
     * @brief Rysuje wykres pomiarów.
     * @param measurement Dane do wyświetlenia.
     * @param stationName Nazwa stacji (do tytułu wykresu).
     * @param from        Początek zakresu (QDateTime() = bez ograniczenia).
     * @param to          Koniec zakresu   (QDateTime() = bez ograniczenia).
     */
    void displayMeasurement(const Measurement& measurement,
                            const QString& stationName,
                            const QDateTime& from = QDateTime(),
                            const QDateTime& to   = QDateTime());

    /** @brief Aktualizuje etykietę statystyk pod wykresem. */
    void updateStats(double avg, double min, double max,
                     const QString& trendDesc);

    /** @brief Pokazuje stan "brak danych". */
    void showNoData();

private:
    void setupUi();

    /** @brief Kolor linii wg wartości PM10 (skala GIOŚ). */
    QColor colorForValue(double value) const;

    QVBoxLayout*   m_layout;
    QChartView*    m_chartView;
    QChart*        m_chart;
    QLineSeries*   m_series;
    QDateTimeAxis* m_axisX;
    QValueAxis*    m_axisY;
    QLabel*        m_statsLabel;
    QLabel*        m_noDataLabel;
};
