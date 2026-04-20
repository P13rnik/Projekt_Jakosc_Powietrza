#include "gui/StationListWidget.hpp"
#include <QListWidgetItem>

StationListWidget::StationListWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    connect(m_listWidget, &QListWidget::itemClicked,
            this, &StationListWidget::onItemClicked);
}

void StationListWidget::setupUi() {
    m_layout     = new QVBoxLayout(this);
    m_listWidget = new QListWidget(this);
    m_listWidget->setAlternatingRowColors(true);
    m_listWidget->setStyleSheet(
        "QListWidget::item { padding: 6px 8px; }"
        "QListWidget::item:selected { background: #2196F3; color: white; }"
    );
    m_layout->addWidget(m_listWidget);
    m_layout->setContentsMargins(0, 0, 0, 0);
}

void StationListWidget::setStations(const QList<Station>& stations) {
    m_stations = stations;
    m_listWidget->clear();
    for (const auto& s : stations) {
        QString label = s.city() + " – " + s.name();
        auto* item = new QListWidgetItem(label, m_listWidget);
        item->setToolTip(s.address().isEmpty() ? s.city() : s.address());
    }
}

void StationListWidget::clear() {
    m_stations.clear();
    m_listWidget->clear();
}

const Station* StationListWidget::selectedStation() const {
    int row = m_listWidget->currentRow();
    if (row < 0 || row >= m_stations.size()) return nullptr;
    return &m_stations[row];
}

void StationListWidget::onItemClicked(QListWidgetItem* item) {
    int row = m_listWidget->row(item);
    if (row >= 0 && row < m_stations.size())
        emit stationSelected(m_stations[row]);
}
