#include "gui/MainWindow.hpp"
#include "gui/BatchDownloader.hpp"
#include "analysis/LinearTrendStrategy.hpp"
#include "analysis/MovingAverageTrendStrategy.hpp"
#include "api/GiosClient.hpp"
#include "storage/DatabaseManager.hpp"
#include "utils/Logger.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QStatusBar>
#include <QMenuBar>
#include <QAction>
#include <QMessageBox>
#include <QApplication>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QDialogButtonBox>
#include <QLabel>
#include <QtConcurrent/QtConcurrent>
#include <cmath>
#include <climits>
#include <algorithm>

#define STR(x) QString::fromUtf8(x)

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_fetcher(new AsyncFetcher(this))
    , m_batchDownloader(new BatchDownloader(this))
{
    setWindowTitle(STR("AirQuality – Monitoring jakości powietrza"));
    setMinimumSize(1100, 680);
    DatabaseManager::instance().setDataDirectory("data");
    m_analyzer.setTrendStrategy(std::make_unique<LinearTrendStrategy>());
    setupUi();
    setupConnections();
    m_statusLabel = new QLabel(STR("Trwa inicjalizacja..."), this);
    statusBar()->addWidget(m_statusLabel);
    onRefreshClicked();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUi() {
    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* topBar = new QHBoxLayout();

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(STR("Wyszukaj stację lub miasto..."));
    m_searchEdit->setMinimumWidth(220);
    m_searchEdit->setStyleSheet(
        "QLineEdit{padding:5px 8px;border-radius:4px;border:1px solid #ccc;font-size:13px;}");

    m_provinceCombo = new QComboBox(this);
    m_provinceCombo->addItem(STR("Wszystkie województwa"), "");
    m_provinceCombo->setMinimumWidth(200);

    m_trendCombo = new QComboBox(this);
    m_trendCombo->addItem(STR("Trend: regresja liniowa"));
    m_trendCombo->addItem(STR("Trend: średnia krocząca"));

    m_refreshButton = new QPushButton(STR("Pobierz najnowsze dane"), this);
    m_refreshButton->setStyleSheet(
        "QPushButton{background:#1976D2;color:white;padding:6px 16px;"
        "border-radius:4px;font-weight:bold;}"
        "QPushButton:hover{background:#1565C0;}"
        "QPushButton:disabled{background:#90CAF9;}");

    topBar->addWidget(m_searchEdit, 1);
    topBar->addWidget(m_provinceCombo);
    topBar->addWidget(m_trendCombo);
    topBar->addStretch();
    topBar->addWidget(m_refreshButton);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setFixedHeight(4);
    m_progressBar->setTextVisible(false);
    m_progressBar->setStyleSheet(
        "QProgressBar{border:none;background:#eee;}"
        "QProgressBar::chunk{background:#1976D2;}");
    m_progressBar->hide();

    m_offlineBanner = new QLabel(
        STR("⚠  Tryb offline – dane z lokalnej bazy"), this);
    m_offlineBanner->setStyleSheet(
        "QLabel{background:#FFF3E0;color:#E65100;padding:4px 8px;"
        "font-weight:bold;border-bottom:1px solid #FFB74D;}");
    m_offlineBanner->setVisible(false);

    m_splitter = new QSplitter(Qt::Horizontal, this);

    auto* leftPanel  = new QGroupBox(STR("Stacje pomiarowe"), m_splitter);
    auto* leftLayout = new QVBoxLayout(leftPanel);
    m_stationList    = new StationListWidget(leftPanel);
    leftLayout->addWidget(m_stationList);
    leftLayout->setContentsMargins(4, 8, 4, 4);

    auto* rightPanel  = new QGroupBox(STR("Pomiary"), m_splitter);
    auto* rightLayout = new QVBoxLayout(rightPanel);
    auto* controlBar  = new QHBoxLayout();
    m_sensorSelector  = new SensorSelectorWidget(rightPanel);
    m_sensorSelector->setVisible(false);
    m_dateRangeSelector = new DateRangeSelector(rightPanel);
    m_dateRangeSelector->setVisible(false);
    controlBar->addWidget(m_sensorSelector, 1);
    controlBar->addWidget(m_dateRangeSelector, 1);
    m_chartWidget = new ChartWidget(rightPanel);
    rightLayout->addLayout(controlBar);
    rightLayout->addWidget(m_chartWidget, 1);
    rightLayout->setContentsMargins(4, 8, 4, 4);

    m_splitter->addWidget(leftPanel);
    m_splitter->addWidget(rightPanel);
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 3);
    m_splitter->setSizes({280, 820});

    auto* mainLayout = new QVBoxLayout(central);
    mainLayout->addLayout(topBar);
    mainLayout->addWidget(m_progressBar);
    mainLayout->addWidget(m_offlineBanner);
    mainLayout->addWidget(m_splitter, 1);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(4);

    // ── Menu ─────────────────────────────────────────────────────────────────
    auto* fileMenu = menuBar()->addMenu(STR("Plik"));
    connect(fileMenu->addAction(STR("Zamknij")),
            &QAction::triggered, qApp, &QApplication::quit);

    auto* viewMenu   = menuBar()->addMenu(STR("Widok"));
    auto* nearestAct = viewMenu->addAction(
        STR("Znajdź najbliższe stacje..."));
    connect(nearestAct, &QAction::triggered, this, [this]() {
        if (m_allStations.isEmpty()) {
            QMessageBox::warning(this, STR("Brak danych"),
                                 STR("Najpierw pobierz listę stacji!"));
            return;
        }
        QDialog dlg(this);
        dlg.setWindowTitle(STR("Znajdź najbliższe stacje"));
        auto* form    = new QFormLayout(&dlg);
        auto* latEdit = new QLineEdit("52.4064", &dlg);
        latEdit->setValidator(new QDoubleValidator(-90, 90, 6, latEdit));
        auto* lonEdit = new QLineEdit("16.9252", &dlg);
        lonEdit->setValidator(new QDoubleValidator(-180, 180, 6, lonEdit));
        form->addRow(STR("Szerokość geo. (lat):"), latEdit);
        form->addRow(STR("Długość geo. (lon):"), lonEdit);
        auto* btns = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
        btns->button(QDialogButtonBox::Ok)->setText(STR("Szukaj"));
        btns->button(QDialogButtonBox::Cancel)->setText(STR("Anuluj"));
        form->addWidget(btns);
        connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
        connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        if (dlg.exec() != QDialog::Accepted) return;

        double lat = latEdit->text().replace(',', '.').toDouble();
        double lon = lonEdit->text().replace(',', '.').toDouble();
        auto hav = [](double la1, double lo1, double la2, double lo2) {
            auto r = [](double d){ return d * M_PI / 180.0; };
            double a = std::sin(r(la2-la1)/2) * std::sin(r(la2-la1)/2)
                       + std::cos(r(la1)) * std::cos(r(la2))
                             * std::sin(r(lo2-lo1)/2) * std::sin(r(lo2-lo1)/2);
            return 6371.0 * 2 * std::atan2(std::sqrt(a), std::sqrt(1-a));
        };
        m_filteredStations = m_allStations;
        std::sort(m_filteredStations.begin(), m_filteredStations.end(),
                  [&](const Station& a, const Station& b) {
                      return hav(lat, lon, a.latitude(), a.longitude())
                      < hav(lat, lon, b.latitude(), b.longitude());
                  });
        m_stationList->setStations(m_filteredStations);
        m_statusLabel->setText(
            STR("Posortowano stacje względem lokalizacji."));
    });

    viewMenu->addSeparator();
    auto* batchAct = viewMenu->addAction(
        STR("Pobierz wszystko do cache offline..."));
    connect(batchAct, &QAction::triggered, this, [this]() {
        if (m_allStations.isEmpty()) {
            QMessageBox::warning(this, STR("Brak stacji"),
                                 STR("Najpierw pobierz listę stacji online."));
            return;
        }
        if (m_batchDownloader->isRunning()) {
            QMessageBox::information(this, STR("W trakcie"),
                                     STR("Pobieranie już trwa."));
            return;
        }
        auto reply = QMessageBox::question(this,
                                           STR("Pobierz wszystko offline"),
                                           STR("Pobierze dane ze wszystkich %1 stacji do lokalnego cache.\n\n"
                                               "Może to zająć kilka minut.\n"
                                               "Kontynuować?").arg(m_allStations.size()),
                                           QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes) return;

        connect(m_batchDownloader, &BatchDownloader::progressChanged,
                this, &MainWindow::onProgressChanged, Qt::UniqueConnection);
        connect(m_batchDownloader, &BatchDownloader::statusChanged,
                this, [this](const QString& msg) {
                    m_statusLabel->setText(msg);
                }, Qt::UniqueConnection);
        connect(m_batchDownloader, &BatchDownloader::finished,
                this, [this](int downloaded, int total, bool cancelled) {
                    m_progressBar->hide();
                    m_refreshButton->setEnabled(true);
                    if (cancelled) {
                        m_statusLabel->setText(STR("Pobieranie anulowane."));
                    } else {
                        m_statusLabel->setText(
                            STR("Cache offline gotowy: %1 / %2 stacji.")
                                .arg(downloaded).arg(total));
                        QMessageBox::information(this,
                                                 STR("Cache gotowy"),
                                                 STR("Pobrano dane z %1 z %2 stacji.\n"
                                                     "Aplikacja może teraz działać offline.")
                                                     .arg(downloaded).arg(total));
                    }
                }, Qt::UniqueConnection);

        m_progressBar->show();
        m_progressBar->setValue(0);
        m_refreshButton->setEnabled(false);
        m_batchDownloader->start(m_allStations);
    });
}

void MainWindow::setupConnections() {
    connect(m_fetcher, &AsyncFetcher::stationsFetched,
            this, &MainWindow::onStationsFetched);
    connect(m_fetcher, &AsyncFetcher::measurementFetched,
            this, &MainWindow::onMeasurementFetched);
    connect(m_fetcher, &AsyncFetcher::fetchError,
            this, &MainWindow::onFetchError);
    connect(m_fetcher, &AsyncFetcher::progressChanged,
            this, &MainWindow::onProgressChanged);

    connect(m_fetcher, &AsyncFetcher::offlineModeActive, this, [this]() {
        m_offlineBanner->setVisible(true);
        setOnlineStatus(false);
        m_statusLabel->setText(
            STR("⚠  Tryb offline – "
                "wyświetlam ostatnie zapisane dane"));
    });

    connect(m_refreshButton, &QPushButton::clicked,
            this, &MainWindow::onRefreshClicked);
    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &MainWindow::onSearchTextChanged);
    connect(m_provinceCombo, &QComboBox::currentTextChanged,
            this, &MainWindow::onProvinceFilterChanged);
    connect(m_stationList, &StationListWidget::stationSelected,
            this, &MainWindow::onStationSelected);

    connect(m_trendCombo,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
                if (idx == 0)
                    m_analyzer.setTrendStrategy(
                        std::make_unique<LinearTrendStrategy>());
                else
                    m_analyzer.setTrendStrategy(
                        std::make_unique<MovingAverageTrendStrategy>());
                refreshStats();
            });

    connect(m_sensorSelector, &SensorSelectorWidget::sensorSelected,
            this, &MainWindow::onSensorChosen);

    connect(m_dateRangeSelector, &DateRangeSelector::rangeChanged,
            this, [this](const QDateTime&, const QDateTime&) {
                refreshStats();
            });
}

// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::onStationsFetched(
    const QList<Station>& stations, bool fromCache) {
    m_allStations = stations;
    m_progressBar->hide();
    m_refreshButton->setEnabled(true);

    QStringList provinces;
    for (const Station& s : stations)
        if (!s.province().isEmpty() && !provinces.contains(s.province()))
            provinces.append(s.province());
    provinces.sort();

    m_provinceCombo->blockSignals(true);
    while (m_provinceCombo->count() > 1) m_provinceCombo->removeItem(1);
    for (const QString& p : provinces) m_provinceCombo->addItem(p, p);
    m_provinceCombo->blockSignals(false);

    if (!fromCache) {
        try { DatabaseManager::instance().saveStations(stations); }
        catch (const std::exception& e) {
            Logger::warning("Cache stacji: " + QString(e.what()));
        }
    }

    applyFilters();
    setOnlineStatus(!fromCache);
    m_offlineBanner->setVisible(fromCache);
    m_statusLabel->setText(
        STR("Załadowano %1 stacji%2")
            .arg(stations.size())
            .arg(fromCache ? STR(" (⚠ tryb offline)") : ""));
}

void MainWindow::onStationSelected(const Station& station) {
    m_currentStation  = station;
    m_currentSensorId = 0;
    m_lastMeasurement = Measurement();
    m_sensorSelector->clear();
    m_sensorSelector->setVisible(false);
    m_dateRangeSelector->setVisible(false);
    m_progressBar->show();
    m_progressBar->setValue(15);
    m_statusLabel->setText(
        STR("Pobieranie czujników: ") + station.name() + "...");
    fetchSensorsForStation(station.id());
}

// Pobiera czujniki w wątku roboczym — przy braku sieci wczytuje z cache.
void MainWindow::fetchSensorsForStation(int stationId) {
    QFuture<QList<Sensor>> fut = QtConcurrent::run(
        [stationId]() -> QList<Sensor> {
            try {
                GiosClient client;
                QList<Sensor> s = client.fetchSensorsForStation(stationId);
                if (!s.isEmpty()) {
                    try {
                        DatabaseManager::instance().saveSensors(stationId, s);
                    } catch (...) {}
                }
                return s;
            } catch (...) {
                return DatabaseManager::instance().loadSensors(stationId);
            }
        });

    auto* w = new QFutureWatcher<QList<Sensor>>(this);
    connect(w, &QFutureWatcher<QList<Sensor>>::finished,
            this, [this, w]() {
                w->deleteLater();
                try {
                    onSensorsFetched(w->result());
                } catch (...) {
                    m_progressBar->hide();
                    m_fetcher->fetchMeasurementAsync(m_currentStation.id(), 0);
                }
            });
    w->setFuture(fut);
}

void MainWindow::onSensorsFetched(const QList<Sensor>& sensors) {
    if (sensors.isEmpty()) {
        m_progressBar->hide();
        m_statusLabel->setText(
            STR("Stacja nie posiada czujników."));
        return;
    }
    m_sensorSelector->setSensors(sensors);
    m_sensorSelector->setVisible(true);
    m_dateRangeSelector->setVisible(true);
}

void MainWindow::onSensorChosen(int sensorId, const QString& paramCode) {
    m_currentSensorId = sensorId;
    m_progressBar->show();
    m_progressBar->setValue(30);
    m_statusLabel->setText(
        STR("Pobieranie: ") + m_currentStation.name()
        + STR(" – ") + paramCode + "...");
    m_fetcher->fetchMeasurementAsync(m_currentStation.id(), sensorId);
}

void MainWindow::onMeasurementFetched(const Measurement& measurement) {
    m_progressBar->hide();
    m_lastMeasurement = measurement;

    if (measurement.isEmpty()) {
        m_chartWidget->showNoData();
        m_statusLabel->setText(
            STR("Brak danych pomiarowych dla wybranej stacji."));
        return;
    }

    m_dateRangeSelector->forceEmit();

    m_statusLabel->setText(
        STR("Dane: %1 – %2 (%3 pomiarów)")
            .arg(m_currentStation.name())
            .arg(measurement.paramCode())
            .arg(measurement.validPoints().size()));
}

void MainWindow::refreshStats() {
    if (m_lastMeasurement.isEmpty()) return;
    try {
        QDateTime from = m_dateRangeSelector->from();
        QDateTime to   = m_dateRangeSelector->to();
        qint64 fromMs  = from.isValid() ? from.toMSecsSinceEpoch() : LLONG_MIN;
        qint64 toMs    = to.isValid()   ? to.toMSecsSinceEpoch()   : LLONG_MAX;

        Measurement filtered(m_lastMeasurement.sensorId(),
                             m_lastMeasurement.paramCode());
        for (const DataPoint& dp : m_lastMeasurement.validPoints()) {
            qint64 ts = dp.timestamp.toMSecsSinceEpoch();
            if (ts >= fromMs && ts <= toMs)
                filtered.addDataPoint(dp);
        }
        if (filtered.isEmpty()) filtered = m_lastMeasurement;

        m_chartWidget->displayMeasurement(filtered, m_currentStation.name());
        double avg   = m_analyzer.calculateAverage(filtered);
        auto   ext   = m_analyzer.findExtremes(filtered);
        auto   trend = m_analyzer.calculateTrend(filtered);
        m_chartWidget->updateStats(
            avg, ext.minValue, ext.maxValue, trend.description);
    } catch (...) {}
}

void MainWindow::onFetchError(const QString& msg) {
    m_progressBar->hide();
    m_refreshButton->setEnabled(true);
    Logger::error(msg);

    bool isNoData  = msg.contains("nie udostępnia danych")
                    || msg.contains("nie jest aktywna")
                    || msg.contains("nie posiada danych online")
                    || msg.contains("nie został odnaleziony")
                    || msg.contains("nie posiada czujnik")
                    || msg.contains("brak danych lokalnych");
    bool isOffline = msg.startsWith("OFFLINE:");

    if (isNoData) {
        m_statusLabel->setText(
            STR("ℹ  ") + msg.split('\n').first());
        setOnlineStatus(true);
        QMessageBox::information(this,
                                 STR("Brak danych bieżących"), msg);
    } else if (isOffline) {
        m_offlineBanner->setVisible(true);
        setOnlineStatus(false);
        m_statusLabel->setText(
            STR("⚠  Brak połączenia "
                "i brak danych lokalnych."));
        QMessageBox::warning(this,
                             STR("Brak połączenia z internetem"),
                             STR("Brak połączenia z internetem.\n\n"
                                 "Dla tej stacji nie ma zapisanych danych lokalnych.\n"
                                 "Spróbuj wybrać inną stację"
                                 " lub przywróć połączenie."));
    } else {
        m_statusLabel->setText(
            STR("Błąd: ") + msg.split('\n').first());
        QMessageBox::warning(this,
                             STR("Problem z pobieraniem danych"), msg);
        setOnlineStatus(false);
    }
}

void MainWindow::onRefreshClicked() {
    m_refreshButton->setEnabled(false);
    m_offlineBanner->setVisible(false);
    m_sensorSelector->clear();
    m_sensorSelector->setVisible(false);
    m_dateRangeSelector->setVisible(false);
    m_progressBar->show();
    m_progressBar->setValue(5);
    m_fetcher->fetchStationsAsync();
    m_statusLabel->setText(STR("Pobieranie listy stacji..."));
}

void MainWindow::onProgressChanged(int p) { m_progressBar->setValue(p); }
void MainWindow::onSearchTextChanged(const QString&)     { applyFilters(); }
void MainWindow::onProvinceFilterChanged(const QString&) { applyFilters(); }

void MainWindow::applyFilters() {
    QString search   = m_searchEdit->text().toLower().trimmed();
    QString province = m_provinceCombo->currentData().toString();

    m_filteredStations.clear();
    for (const Station& s : m_allStations) {
        bool matchS = search.isEmpty()
        || s.name().toLower().contains(search)
            || s.city().toLower().contains(search)
            || s.address().toLower().contains(search);
        bool matchP = province.isEmpty() || s.province() == province;
        if (matchS && matchP) m_filteredStations.append(s);
    }
    m_stationList->setStations(m_filteredStations);
    m_statusLabel->setText(
        STR("Wyświetlono %1 / %2 stacji")
            .arg(m_filteredStations.size()).arg(m_allStations.size()));
}

void MainWindow::setOnlineStatus(bool online) {
    if (online)
        statusBar()->setStyleSheet("");
    else
        statusBar()->setStyleSheet("QStatusBar{background:#FFF3E0;color:#E65100;}");
}
