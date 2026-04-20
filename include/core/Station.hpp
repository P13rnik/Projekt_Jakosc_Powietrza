#pragma once
#include <QString>
#include <QList>
#include "core/Sensor.hpp"

/**
 * @brief Model stacji pomiarowej GIOŚ.
 *
 * Przechowuje dane stacji zgodnie ze strukturą API GIOŚ:
 * https://powietrze.gios.gov.pl/pjp/content/api
 */
class Station {
public:
    Station() = default;

    /**
     * @brief Konstruktor inicjalizujący pola stacji.
     * @param id Unikalny identyfikator stacji.
     * @param name Nazwa stacji.
     * @param city Miasto, w którym znajduje się stacja.
     * @param lat Szerokość geograficzna.
     * @param lon Długość geograficzna.
     */
    Station(int id, const QString& name, const QString& city,
            double lat, double lon);

    int     id()        const { return m_id; }
    QString name()      const { return m_name; }
    QString city()      const { return m_city; }
    QString province()  const { return m_province; }
    QString address()   const { return m_address; }
    double  latitude()  const { return m_lat; }
    double  longitude() const { return m_lon; }

    void setProvince(const QString& province) { m_province = province; }
    void setAddress(const QString& address)   { m_address = address; }

    const QList<Sensor>& sensors() const { return m_sensors; }
    void addSensor(const Sensor& sensor)  { m_sensors.append(sensor); }

    /**
     * @brief Serializuje stację do formatu JSON.
     * @return Obiekt nlohmann::json z danymi stacji.
     */
    // (implementacja w Station.cpp z użyciem nlohmann/json)

private:
    int     m_id       = 0;
    QString m_name;
    QString m_city;
    QString m_province;
    QString m_address;
    double  m_lat      = 0.0;
    double  m_lon      = 0.0;
    QList<Sensor> m_sensors;
};
