/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[19];
    char stringdata0[386];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 29), // "on_actionLoad_Image_triggered"
QT_MOC_LITERAL(2, 41, 0), // ""
QT_MOC_LITERAL(3, 42, 34), // "on_actionSynthetic_Image_trig..."
QT_MOC_LITERAL(4, 77, 14), // "onStateChanged"
QT_MOC_LITERAL(5, 92, 1), // "s"
QT_MOC_LITERAL(6, 94, 22), // "onProgressValueChanged"
QT_MOC_LITERAL(7, 117, 5), // "value"
QT_MOC_LITERAL(8, 123, 15), // "onGraphComputed"
QT_MOC_LITERAL(9, 139, 10), // "onFinished"
QT_MOC_LITERAL(10, 150, 23), // "on_pushButton_2_clicked"
QT_MOC_LITERAL(11, 174, 32), // "on_actionImpulse_noise_triggered"
QT_MOC_LITERAL(12, 207, 33), // "on_actionGaussian_noise_trigg..."
QT_MOC_LITERAL(13, 241, 23), // "onFilteringValueChanged"
QT_MOC_LITERAL(14, 265, 33), // "on_checkBoxComponent_stateCha..."
QT_MOC_LITERAL(15, 299, 4), // "arg1"
QT_MOC_LITERAL(16, 304, 27), // "on_checkBoxHSV_stateChanged"
QT_MOC_LITERAL(17, 332, 30), // "on_actionSave_result_triggered"
QT_MOC_LITERAL(18, 363, 22) // "onAdaptiveValueChanged"

    },
    "MainWindow\0on_actionLoad_Image_triggered\0"
    "\0on_actionSynthetic_Image_triggered\0"
    "onStateChanged\0s\0onProgressValueChanged\0"
    "value\0onGraphComputed\0onFinished\0"
    "on_pushButton_2_clicked\0"
    "on_actionImpulse_noise_triggered\0"
    "on_actionGaussian_noise_triggered\0"
    "onFilteringValueChanged\0"
    "on_checkBoxComponent_stateChanged\0"
    "arg1\0on_checkBoxHSV_stateChanged\0"
    "on_actionSave_result_triggered\0"
    "onAdaptiveValueChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   84,    2, 0x08 /* Private */,
       3,    0,   85,    2, 0x08 /* Private */,
       4,    1,   86,    2, 0x08 /* Private */,
       6,    1,   89,    2, 0x08 /* Private */,
       8,    0,   92,    2, 0x08 /* Private */,
       9,    0,   93,    2, 0x08 /* Private */,
      10,    0,   94,    2, 0x08 /* Private */,
      11,    0,   95,    2, 0x08 /* Private */,
      12,    0,   96,    2, 0x08 /* Private */,
      13,    1,   97,    2, 0x08 /* Private */,
      14,    1,  100,    2, 0x08 /* Private */,
      16,    1,  103,    2, 0x08 /* Private */,
      17,    0,  106,    2, 0x08 /* Private */,
      18,    1,  107,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    7,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    7,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MainWindow *_t = static_cast<MainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_actionLoad_Image_triggered(); break;
        case 1: _t->on_actionSynthetic_Image_triggered(); break;
        case 2: _t->onStateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->onProgressValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->onGraphComputed(); break;
        case 5: _t->onFinished(); break;
        case 6: _t->on_pushButton_2_clicked(); break;
        case 7: _t->on_actionImpulse_noise_triggered(); break;
        case 8: _t->on_actionGaussian_noise_triggered(); break;
        case 9: _t->onFilteringValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->on_checkBoxComponent_stateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->on_checkBoxHSV_stateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->on_actionSave_result_triggered(); break;
        case 13: _t->onAdaptiveValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow.data,
      qt_meta_data_MainWindow,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 14;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
