#include "core/Station.hpp"

Station::Station(int id, const QString& name, const QString& city,
                 double lat, double lon)
    : m_id(id)
    , m_name(name)
    , m_city(city)
    , m_lat(lat)
    , m_lon(lon)
{}
