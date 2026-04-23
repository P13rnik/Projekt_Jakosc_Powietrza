/****************************************************************************
** Meta object code from reading C++ file 'DateRangeSelector.hpp'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../include/gui/DateRangeSelector.hpp"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DateRangeSelector.hpp' doesn't include <QObject>."
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
struct qt_meta_tag_ZN17DateRangeSelectorE_t {};
} // unnamed namespace

template <> constexpr inline auto DateRangeSelector::qt_create_metaobjectdata<qt_meta_tag_ZN17DateRangeSelectorE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "DateRangeSelector",
        "rangeChanged",
        "",
        "from",
        "to",
        "onPresetChanged",
        "index",
        "onCustomDateChanged"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'rangeChanged'
        QtMocHelpers::SignalData<void(const QDateTime &, const QDateTime &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QDateTime, 3 }, { QMetaType::QDateTime, 4 },
        }}),
        // Slot 'onPresetChanged'
        QtMocHelpers::SlotData<void(int)>(5, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 6 },
        }}),
        // Slot 'onCustomDateChanged'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<DateRangeSelector, qt_meta_tag_ZN17DateRangeSelectorE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject DateRangeSelector::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17DateRangeSelectorE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17DateRangeSelectorE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN17DateRangeSelectorE_t>.metaTypes,
    nullptr
} };

void DateRangeSelector::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DateRangeSelector *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->rangeChanged((*reinterpret_cast<std::add_pointer_t<QDateTime>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QDateTime>>(_a[2]))); break;
        case 1: _t->onPresetChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->onCustomDateChanged(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (DateRangeSelector::*)(const QDateTime & , const QDateTime & )>(_a, &DateRangeSelector::rangeChanged, 0))
            return;
    }
}

const QMetaObject *DateRangeSelector::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DateRangeSelector::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17DateRangeSelectorE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int DateRangeSelector::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void DateRangeSelector::rangeChanged(const QDateTime & _t1, const QDateTime & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}
QT_WARNING_POP
