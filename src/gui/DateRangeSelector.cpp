#include "gui/DateRangeSelector.hpp"
#include <QLabel>

DateRangeSelector::DateRangeSelector(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void DateRangeSelector::setupUi() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    layout->addWidget(new QLabel("Okres:", this));

    m_presetCombo = new QComboBox(this);
    // API GIOŚ zwraca ~20 pomiarów godzinowych — max ~20h danych
    m_presetCombo->addItem("Wszystkie dane",  0);   // 0 = brak filtrowania
    m_presetCombo->addItem("Ostatnie 12h",   12);
    m_presetCombo->addItem("Ostatnie 6h",     6);
    m_presetCombo->addItem("Niestandardowy", -1);   // -1 = tryb ręczny
    layout->addWidget(m_presetCombo);

    layout->addWidget(new QLabel("Od:", this));
    m_fromEdit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(-3), this);
    m_fromEdit->setDisplayFormat("dd.MM.yyyy hh:mm");
    m_fromEdit->setCalendarPopup(true);
    m_fromEdit->setEnabled(false);
    layout->addWidget(m_fromEdit);

    layout->addWidget(new QLabel("Do:", this));
    m_toEdit = new QDateTimeEdit(QDateTime::currentDateTime(), this);
    m_toEdit->setDisplayFormat("dd.MM.yyyy hh:mm");
    m_toEdit->setCalendarPopup(true);
    m_toEdit->setEnabled(false);
    layout->addWidget(m_toEdit);

    m_applyButton = new QPushButton("Zastosuj", this);
    m_applyButton->setEnabled(false);
    m_applyButton->setStyleSheet(
        "QPushButton { padding: 3px 10px; border-radius: 3px; "
        "background: #1976D2; color: white; }"
        "QPushButton:hover { background: #1565C0; }"
        "QPushButton:disabled { background: #90CAF9; }");
    layout->addWidget(m_applyButton);

    layout->addStretch();

    // Podepnij ZANIM wywołamy onPresetChanged
    connect(m_presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DateRangeSelector::onPresetChanged);
    connect(m_applyButton, &QPushButton::clicked,
            this, &DateRangeSelector::onCustomDateChanged);

    // Ustaw domyślny zakres (72h) — flaga blokuje emit rangeChanged
    m_initialized = false;
    onPresetChanged(0);
    m_initialized = true;
}

QDateTime DateRangeSelector::from() const { return m_fromEdit->dateTime(); }
QDateTime DateRangeSelector::to()   const { return m_toEdit->dateTime();   }

void DateRangeSelector::onPresetChanged(int index) {
    int hours    = m_presetCombo->itemData(index).toInt();
    bool isCustom = (hours == -1); // -1 = tryb niestandardowy

    m_fromEdit->setEnabled(isCustom);
    m_toEdit->setEnabled(isCustom);
    m_applyButton->setEnabled(isCustom);

    if (!isCustom) {
        m_fromEdit->blockSignals(true);
        m_toEdit->blockSignals(true);

        QDateTime now = QDateTime::currentDateTime();
        QDateTime from;
        if (hours > 0)
            from = now.addSecs(-hours * 3600LL);
        // hours == 0: "Wszystkie dane" — from pozostaje invalid (brak filtra)

        m_fromEdit->setDateTime(from.isValid() ? from : now.addDays(-1));
        m_toEdit->setDateTime(now);

        m_fromEdit->blockSignals(false);
        m_toEdit->blockSignals(false);

        if (m_initialized)
            emit rangeChanged(from, now);
    }
}

void DateRangeSelector::onCustomDateChanged() {
    QDateTime f = m_fromEdit->dateTime();
    QDateTime t = m_toEdit->dateTime();
    if (f < t)
        emit rangeChanged(f, t);
}

// Wymusza odświeżenie zakresu — przydatne gdy nowe dane zostały załadowane.
// Aktualizuje "Do:" do bieżącej chwili i emituje rangeChanged.
void DateRangeSelector::forceEmit() {
    int index = m_presetCombo->currentIndex();
    int hours = m_presetCombo->itemData(index).toInt();

    if (hours == -1) {
        // Tryb niestandardowy — emituj obecne wartości
        onCustomDateChanged();
    } else {
        // Preset (0 = wszystkie dane, >0 = N godzin)
        m_fromEdit->blockSignals(true);
        m_toEdit->blockSignals(true);
        QDateTime now  = QDateTime::currentDateTime();
        QDateTime from = (hours > 0) ? now.addSecs(-hours * 3600LL) : QDateTime();
        m_fromEdit->setDateTime(from.isValid() ? from : now.addDays(-1));
        m_toEdit->setDateTime(now);
        m_fromEdit->blockSignals(false);
        m_toEdit->blockSignals(false);
        emit rangeChanged(from, now);
    }
}
