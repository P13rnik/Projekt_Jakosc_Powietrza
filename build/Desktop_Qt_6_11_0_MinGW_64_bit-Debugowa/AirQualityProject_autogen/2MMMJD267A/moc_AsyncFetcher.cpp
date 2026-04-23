/****************************************************************************
** Meta object code from reading C++ file 'AsyncFetcher.hpp'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../include/api/AsyncFetcher.hpp"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AsyncFetcher.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN12AsyncFetcherE_t {};
} // unnamed namespace

template <> constexpr inline auto AsyncFetcher::qt_create_metaobjectdata<qt_meta_tag_ZN12AsyncFetcherE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "AsyncFetcher",
        "stationsFetched",
        "",
        "QList<Station>",
        "stations",
        "fromCache",
        "measurementFetched",
        "Measurement",
        "measurement",
        "fetchError",
        "errorMessage",
        "progressChanged",
        "percent"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'stationsFetched'
        QtMocHelpers::SignalData<void(const QList<Station> &, bool)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { QMetaType::Bool, 5 },
        }}),
        // Signal 'measurementFetched'
        QtMocHelpers::SignalData<void(const Measurement &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 7, 8 },
        }}),
        // Signal 'fetchError'
        QtMocHelpers::SignalData<void(const QString &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 },
        }}),
        // Signal 'progressChanged'
        QtMocHelpers::SignalData<void(int)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 12 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AsyncFetcher, qt_meta_tag_ZN12AsyncFetcherE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject AsyncFetcher::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12AsyncFetcherE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12AsyncFetcherE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12AsyncFetcherE_t>.metaTypes,
    nullptr
} };

void AsyncFetcher::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AsyncFetcher *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->stationsFetched((*reinterpret_cast<std::add_pointer_t<QList<Station>>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2]))); break;
        case 1: _t->measurementFetched((*reinterpret_cast<std::add_pointer_t<Measurement>>(_a[1]))); break;
        case 2: _t->fetchError((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->progressChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AsyncFetcher::*)(const QList<Station> & , bool )>(_a, &AsyncFetcher::stationsFetched, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AsyncFetcher::*)(const Measurement & )>(_a, &AsyncFetcher::measurementFetched, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (AsyncFetcher::*)(const QString & )>(_a, &AsyncFetcher::fetchError, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (AsyncFetcher::*)(int )>(_a, &AsyncFetcher::progressChanged, 3))
            return;
    }
}

const QMetaObject *AsyncFetcher::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AsyncFetcher::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12AsyncFetcherE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int AsyncFetcher::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void AsyncFetcher::stationsFetched(const QList<Station> & _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void AsyncFetcher::measurementFetched(const Measurement & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void AsyncFetcher::fetchError(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void AsyncFetcher::progressChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}
QT_WARNING_POP
