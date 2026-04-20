#include "gui/RadiusSearchDialog.hpp"
#include "analysis/GeoLocator.hpp"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QLocale>

RadiusSearchDialog::RadiusSearchDialog(const QList<Station>& allStations,
                                       QWidget* parent)
    : QDialog(parent)
    , m_allStations(allStations)
{
    setWindowTitle("Szukaj stacji w promieniu");
    setMinimumWidth(380);
    setupUi();
}

// Buduje układ dialogu: pole tekstowe lokalizacji, pola lat/lon/promień, przyciski.
void RadiusSearchDialog::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);

    auto* infoLabel = new QLabel(
        "Podaj współrzędne geograficzne środka wyszukiwania\n"
        "i maksymalny promień w kilometrach.", this);
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #555; font-size: 12px;");
    mainLayout->addWidget(infoLabel);

    auto* formGroup = new QGroupBox("Lokalizacja", this);
    auto* form = new QFormLayout(formGroup);

    m_locationEdit = new QLineEdit(this);
    m_locationEdit->setPlaceholderText("np. Polanka 3, Poznań (informacyjnie)");
    form->addRow("Opis:", m_locationEdit);

    // Wymuszamy locale C (kropka jako separator) — polskie Windows używa przecinka,
    // co powoduje że QDoubleSpinBox nie akceptuje cyfr z kropką dziesiętną.
    QLocale dotLocale(QLocale::C);
    dotLocale.setNumberOptions(QLocale::RejectGroupSeparator);

    m_latSpin = new QDoubleSpinBox(this);
    m_latSpin->setLocale(dotLocale);
    m_latSpin->setRange(-90.0, 90.0);
    m_latSpin->setDecimals(6);
    m_latSpin->setSingleStep(0.001);
    m_latSpin->setValue(52.406900);  // Domyślnie Poznań
    m_latSpin->setSuffix("°");
    form->addRow("Szerokość geo. (lat):", m_latSpin);

    m_lonSpin = new QDoubleSpinBox(this);
    m_lonSpin->setLocale(dotLocale);
    m_lonSpin->setRange(-180.0, 180.0);
    m_lonSpin->setDecimals(6);
    m_lonSpin->setSingleStep(0.001);
    m_lonSpin->setValue(16.929900);  // Domyślnie Poznań
    m_lonSpin->setSuffix("°");
    form->addRow("Długość geo. (lon):", m_lonSpin);

    m_radiusSpin = new QDoubleSpinBox(this);
    m_radiusSpin->setLocale(dotLocale);
    m_radiusSpin->setRange(1.0, 500.0);
    m_radiusSpin->setDecimals(1);
    m_radiusSpin->setSingleStep(5.0);
    m_radiusSpin->setValue(50.0);
    m_radiusSpin->setSuffix(" km");
    form->addRow("Promień wyszukiwania:", m_radiusSpin);

    mainLayout->addWidget(formGroup);

    m_buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    m_buttons->addButton(tr("Szukaj"), QDialogButtonBox::AcceptRole);
    mainLayout->addWidget(m_buttons);

    connect(m_buttons, &QDialogButtonBox::accepted, this, &RadiusSearchDialog::onSearch);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

// Wywołuje GeoLocator::findNearest i zamyka dialog jeśli znaleziono stacje.
void RadiusSearchDialog::onSearch() {
    double lat    = m_latSpin->value();
    double lon    = m_lonSpin->value();
    double radius = m_radiusSpin->value();

    m_results = GeoLocator::findNearest(m_allStations, lat, lon, radius);

    if (m_results.isEmpty()) {
        QMessageBox::information(this, "Brak wyników",
                                 QString("Nie znaleziono stacji w promieniu %1 km.")
                                     .arg(radius, 0, 'f', 1));
        return;
    }

    accept();
}
