#include "gui/SensorSelectorWidget.hpp"

SensorSelectorWidget::SensorSelectorWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

// Buduje układ: etykieta "Parametr:" + ComboBox z listą czujników.
void SensorSelectorWidget::setupUi() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    m_label = new QLabel("Parametr:", this);
    m_combo = new QComboBox(this);
    m_combo->setMinimumWidth(160);
    m_combo->setEnabled(false);
    m_combo->setToolTip("Wybierz mierzony parametr (PM10, NO2, SO2 itp.)");

    layout->addWidget(m_label);
    layout->addWidget(m_combo, 1);

    connect(m_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SensorSelectorWidget::onComboChanged);
}

// Wypełnia listę dostępnymi czujnikami. Blokuje sygnały podczas ładowania
// żeby nie emitować fałszywego sensorSelected() przy każdym dodaniu elementu.
void SensorSelectorWidget::setSensors(const QList<Sensor>& sensors) {
    m_sensors = sensors;
    m_combo->blockSignals(true);
    m_combo->clear();

    for (const auto& s : sensors) {
        // Etykieta: "PM10 (pył zawieszony PM10)"
        QString label = s.paramCode();
        if (!s.paramName().isEmpty() && s.paramName() != s.paramCode())
            label += "  –  " + s.paramName();
        m_combo->addItem(label, s.id());
    }

    m_combo->blockSignals(false);
    m_combo->setEnabled(!sensors.isEmpty());

    // Emituj sygnał dla domyślnego wyboru (pierwszy czujnik)
    if (!sensors.isEmpty())
        emit sensorSelected(sensors.first().id(), sensors.first().paramCode());
}

void SensorSelectorWidget::clear() {
    m_sensors.clear();
    m_combo->clear();
    m_combo->setEnabled(false);
}

int SensorSelectorWidget::selectedSensorId() const {
    if (m_combo->currentIndex() < 0) return 0;
    return m_combo->currentData().toInt();
}

// Emituje sensorSelected() z ID i kodem parametru wybranego czujnika.
void SensorSelectorWidget::onComboChanged(int index) {
    if (index < 0 || index >= m_sensors.size()) return;
    emit sensorSelected(m_sensors[index].id(), m_sensors[index].paramCode());
}
