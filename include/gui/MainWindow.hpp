#pragma once
#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QProgressBar>
#include <QSplitter>
#include <QList>
#include "api/AsyncFetcher.hpp"
#include "analysis/AirAnalyzer.hpp"
#include "gui/StationListWidget.hpp"
#include "gui/ChartWidget.hpp"
#include "gui/SensorSelectorWidget.hpp"
#include "gui/DateRangeSelector.hpp"
#include "core/Station.hpp"
#include "core/Sensor.hpp"
#include "core/Measurement.hpp"
#include "gui/BatchDownloader.hpp"

/**
 * @brief Główne okno aplikacji AirQuality.
 *
 * Zawiera pasek wyszukiwania, filtr województwa, wybór trendu,
 * listę stacji, selector parametru (PM10/NO2/...), wybór zakresu
 * dat i wykres pomiarów.
 *
 * WIELOWĄTKOWOŚĆ: wszystkie żądania HTTP uruchamiane przez AsyncFetcher
 * w osobnych wątkach z puli Qt — GUI nigdy nie blokuje się na sieci.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    /** @brief Obsługuje pobranie listy stacji (online lub z cache). */
    void onStationsFetched(const QList<Station>& stations, bool fromCache);

    /** @brief Wyświetla błąd pobierania — 3 tryby: info/baner/okno. */
    void onFetchError(const QString& errorMessage);

    /** @brief Po kliknięciu stacji: pobiera jej czujniki w tle. */
    void onStationSelected(const Station& station);

    /** @brief Po pobraniu czujników: wypełnia SensorSelector. */
    void onSensorsFetched(const QList<Sensor>& sensors);

    /** @brief Po zmianie parametru: pobiera dane wybranego czujnika. */
    void onSensorChosen(int sensorId, const QString& paramCode);

    /** @brief Po pobraniu pomiarów: rysuje wykres i oblicza statystyki. */
    void onMeasurementFetched(const Measurement& measurement);

    void onSearchTextChanged(const QString& text);
    void onProvinceFilterChanged(const QString& province);
    void onRefreshClicked();
    void onProgressChanged(int percent);

private:
    void setupUi();
    void setupConnections();
    void applyFilters();
    void setOnlineStatus(bool isOnline);
    void fetchSensorsForStation(int stationId);

    /** @brief Przelicza statystyki bez ponownego pobierania danych. */
    void refreshStats();

    // ── Widgety ──────────────────────────────────────────────────────────────
    QLineEdit*             m_searchEdit;
    QComboBox*             m_provinceCombo;
    QComboBox*             m_trendCombo;
    QPushButton*           m_refreshButton;
    QProgressBar*          m_progressBar;
    QLabel*                m_statusLabel;
    QLabel*                m_offlineBanner;  ///< Widoczny baner przy trybie offline
    StationListWidget*     m_stationList;
    SensorSelectorWidget*  m_sensorSelector;
    DateRangeSelector*     m_dateRangeSelector;
    ChartWidget*           m_chartWidget;
    QSplitter*             m_splitter;

    // ── Logika ───────────────────────────────────────────────────────────────
    AsyncFetcher*  m_fetcher;
    AirAnalyzer    m_analyzer;
    QList<Station> m_allStations;
    QList<Station> m_filteredStations;
    Station        m_currentStation;
    int            m_currentSensorId = 0;
    Measurement    m_lastMeasurement;
    BatchDownloader* m_batchDownloader; ///< Ostatnie pobrane dane — do refreshStats()
};
