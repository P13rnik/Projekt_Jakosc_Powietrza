#include "gui/ChartWidget.hpp"
#include <QtCharts/QChart>
#include <QDateTime>
#include <climits>
#include <algorithm>

ChartWidget::ChartWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

// Tworzy i konfiguruje wykres QtCharts wraz z osiami i etykietami.
void ChartWidget::setupUi() {
    m_layout = new QVBoxLayout(this);

    m_chart  = new QChart();
    m_series = new QLineSeries();
    m_axisX  = new QDateTimeAxis();
    m_axisY  = new QValueAxis();

    m_axisX->setFormat("dd.MM hh:mm");
    m_axisX->setTitleText("Czas");
    m_axisY->setTitleText(QString::fromUtf8("Wartość (µg/m³)"));

    m_chart->addSeries(m_series);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisX);
    m_series->attachAxis(m_axisY);
    m_chart->legend()->hide();
    m_chart->setAnimationOptions(QChart::SeriesAnimations);

    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setMinimumHeight(300);

    m_statsLabel = new QLabel(this);
    m_statsLabel->setAlignment(Qt::AlignCenter);
    m_statsLabel->setStyleSheet("font-size:13px;color:#555;padding:4px;");

    m_noDataLabel = new QLabel(
        QString::fromUtf8("Wybierz stację, aby zobaczyć dane."), this);
    m_noDataLabel->setAlignment(Qt::AlignCenter);
    m_noDataLabel->setStyleSheet("font-size:14px;color:#888;");

    m_layout->addWidget(m_chartView);
    m_layout->addWidget(m_statsLabel);
    m_layout->addWidget(m_noDataLabel);
    m_layout->setContentsMargins(0, 0, 0, 0);

    showNoData();
}

void ChartWidget::showNoData() {
    m_chartView->setVisible(false);
    m_statsLabel->setVisible(false);
    m_noDataLabel->setVisible(true);
}

// Wyświetla pomiary na wykresie z opcjonalnym filtrem zakresu dat.
// BUG FIX: sortowanie chronologiczne — API zwraca od najnowszego do najstarszego.
// BUG FIX: filtrowanie punktów z nieprawidłowym timestampem (epoch 1970).
void ChartWidget::displayMeasurement(const Measurement& measurement,
                                     const QString& stationName,
                                     const QDateTime& from,
                                     const QDateTime& to) {
    m_series->clear();
    QList<DataPoint> points = measurement.validPoints();

    if (points.isEmpty()) {
        m_noDataLabel->setText(
            QString::fromUtf8("Brak danych pomiarowych."));
        showNoData();
        return;
    }

    // Sortuj chronologicznie — API zwraca od najnowszego do najstarszego
    std::sort(points.begin(), points.end(), [](const DataPoint& a, const DataPoint& b) {
        return a.timestamp < b.timestamp;
    });

    // Filtruj nieprawidłowe daty i zakres wybrany przez użytkownika
    QDateTime epochLimit = QDateTime::fromSecsSinceEpoch(86400);
    // Porównuj przez msec — unikamy problemów ze strefami czasowymi
    qint64 fromMs = from.isValid() ? from.toMSecsSinceEpoch() : LLONG_MIN;
    qint64 toMs   = to.isValid()   ? to.toMSecsSinceEpoch()   : LLONG_MAX;
    QList<DataPoint> valid;
    for (const DataPoint& dp : points) {
        if (!dp.timestamp.isValid() || dp.timestamp <= epochLimit) continue;
        qint64 tsMs = dp.timestamp.toMSecsSinceEpoch();
        if (tsMs < fromMs || tsMs > toMs) continue;
        valid.append(dp);
    }

    if (valid.isEmpty()) {
        m_noDataLabel->setText(
            QString::fromUtf8("Brak danych w wybranym przedziale czasu."));
        showNoData();
        return;
    }

    for (const DataPoint& dp : valid)
        m_series->append(dp.timestamp.toMSecsSinceEpoch(), dp.value);

    // Kolor linii wg średniej — skala jakości powietrza GIOŚ
    double avg = 0;
    for (const DataPoint& dp : valid) avg += dp.value;
    avg /= valid.size();
    QPen pen(colorForValue(avg));
    pen.setWidth(2);
    m_series->setPen(pen);

    m_axisX->setRange(valid.first().timestamp, valid.last().timestamp);

    double minV = valid.first().value, maxV = valid.first().value;
    for (const DataPoint& dp : valid) {
        minV = std::min(minV, dp.value);
        maxV = std::max(maxV, dp.value);
    }
    if (qFuzzyCompare(minV, maxV)) { minV -= 1.0; maxV += 1.0; }
    m_axisY->setRange(std::max(0.0, minV - 5.0), maxV + 5.0);

    m_chart->setTitle(stationName + "  –  " + measurement.paramCode());
    m_noDataLabel->setVisible(false);
    m_chartView->setVisible(true);
    m_statsLabel->setVisible(true);
}

// Aktualizuje etykietę statystyk pod wykresem.
void ChartWidget::updateStats(double avg, double min, double max,
                              const QString& trendDesc) {
    m_statsLabel->setText(
        QString::fromUtf8("Wśr: <b>%1</b>  |  Min: <b>%2</b>  |  Maks: <b>%3</b>  |  %4")
            .arg(avg, 0, 'f', 1)
            .arg(min, 0, 'f', 1)
            .arg(max, 0, 'f', 1)
            .arg(trendDesc));
}

// fix clazy-qcolor-from-literal: QColor(r,g,b) zamiast QColor("#xxxxxx")
QColor ChartWidget::colorForValue(double v) const {
    if (v <= 25)  return QColor(76,  175, 80);
    if (v <= 50)  return QColor(205, 220, 57);
    if (v <= 75)  return QColor(255, 152, 0);
    if (v <= 100) return QColor(244, 67,  54);
    return             QColor(156, 39,  176);
}
