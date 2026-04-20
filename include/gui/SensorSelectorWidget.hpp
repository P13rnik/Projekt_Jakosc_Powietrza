#pragma once
#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QList>
#include "core/Sensor.hpp"

/**
 * @brief Widget wyboru czujnika (parametru) z listy stanowisk stacji.
 *
 * Wyświetla listę dostępnych parametrów pomiarowych (PM10, PM2.5, NO2, O3, SO2, C6H6)
 * i emituje sygnał sensorSelected() gdy użytkownik zmieni wybór.
 */
class SensorSelectorWidget : public QWidget {
    Q_OBJECT

public:
    explicit SensorSelectorWidget(QWidget* parent = nullptr);

    /**
     * @brief Wypełnia listę czujnikami podanej stacji.
     * @param sensors Lista czujników do wyświetlenia.
     */
    void setSensors(const QList<Sensor>& sensors);

    /** @brief Czyści listę (np. przy zmianie stacji). */
    void clear();

    /** @brief Zwraca ID aktualnie wybranego czujnika lub 0 jeśli lista pusta. */
    int selectedSensorId() const;

signals:
    /**
     * @brief Emitowany gdy użytkownik zmieni wybrany parametr.
     * @param sensorId ID wybranego czujnika.
     * @param paramCode Kod parametru (np. "PM10").
     */
    void sensorSelected(int sensorId, const QString& paramCode);

private slots:
    void onComboChanged(int index);

private:
    void setupUi();

    QComboBox*    m_combo;
    QLabel*       m_label;
    QList<Sensor> m_sensors;
};
