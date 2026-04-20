#pragma once
#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QList>
#include "core/Station.hpp"

/**
 * @brief Widget listy stacji pomiarowych z możliwością zaznaczenia.
 *
 * Wyświetla stacje jako elementy listy z nazwą miasta i stacji.
 * Emituje sygnał stationSelected() po kliknięciu na stację.
 */
class StationListWidget : public QWidget {
    Q_OBJECT

public:
    explicit StationListWidget(QWidget* parent = nullptr);

    /**
     * @brief Wypełnia listę podanymi stacjami.
     * @param stations Lista stacji do wyświetlenia.
     */
    void setStations(const QList<Station>& stations);

    /**
     * @brief Czyści listę stacji.
     */
    void clear();

    /**
     * @brief Zwraca aktualnie zaznaczoną stację.
     * @return Wskaźnik na stację lub nullptr jeśli brak zaznaczenia.
     */
    const Station* selectedStation() const;

signals:
    /**
     * @brief Emitowany po kliknięciu na stację.
     * @param station Zaznaczona stacja.
     */
    void stationSelected(const Station& station);

private slots:
    void onItemClicked(QListWidgetItem* item);

private:
    void setupUi();

    QListWidget*   m_listWidget;
    QVBoxLayout*   m_layout;
    QList<Station> m_stations; ///< Kopia listy do dostępu po indeksie
};
