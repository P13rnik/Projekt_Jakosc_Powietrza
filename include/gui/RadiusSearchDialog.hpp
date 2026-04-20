#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>
#include <QList>
#include "core/Station.hpp"

/**
 * @brief Dialog umożliwiający wyszukiwanie stacji w zadanym promieniu od lokalizacji.
 *
 * Użytkownik podaje lokalizację słownie (np. "Polanka 3, Poznań") oraz promień w km.
 * Klasa używa GeoLocator (wzór haversine) do obliczenia odległości
 * i zwraca przefiltrowaną listę stacji.
 *
 * Uwaga: geocodowanie nazwy lokalizacji na współrzędne wymaga
 * zewnętrznego serwisu (np. Nominatim/OpenStreetMap) lub ręcznego
 * wpisania współrzędnych – w tej wersji używamy pól lat/lon.
 */
class RadiusSearchDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Konstruktor dialogu.
     * @param allStations Pełna lista stacji do przeszukania.
     * @param parent Widget rodzic.
     */
    explicit RadiusSearchDialog(const QList<Station>& allStations,
                                QWidget* parent = nullptr);

    /**
     * @brief Zwraca wyniki wyszukiwania po zaakceptowaniu dialogu.
     * @return Lista stacji w zadanym promieniu, posortowana wg odległości.
     */
    QList<Station> results() const { return m_results; }

private slots:
    void onSearch();

private:
    void setupUi();

    QLineEdit*       m_locationEdit;  ///< Opis słowny lokalizacji (informacyjnie)
    QDoubleSpinBox*  m_latSpin;       ///< Szerokość geograficzna punktu centralnego
    QDoubleSpinBox*  m_lonSpin;       ///< Długość geograficzna punktu centralnego
    QDoubleSpinBox*  m_radiusSpin;    ///< Promień wyszukiwania w km
    QDialogButtonBox* m_buttons;

    const QList<Station>& m_allStations;
    QList<Station>         m_results;
};
