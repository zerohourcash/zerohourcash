/****************************************************************************
** Meta object code from reading C++ file 'zhrctoken.h'
**
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "qt/zhrctoken.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QVector>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'zhrctoken.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.7. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ZRCToken_t {
    QByteArrayData data[24];
    char stringdata0[313];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ZRCToken_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ZRCToken_t qt_meta_stringdata_ZRCToken = {
    {
QT_MOC_LITERAL(0, 0, 8), // "ZRCToken"
QT_MOC_LITERAL(1, 9, 20), // "on_goToSendTokenPage"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 23), // "on_goToReceiveTokenPage"
QT_MOC_LITERAL(4, 55, 19), // "on_goToAddTokenPage"
QT_MOC_LITERAL(5, 75, 22), // "on_currentTokenChanged"
QT_MOC_LITERAL(6, 98, 5), // "index"
QT_MOC_LITERAL(7, 104, 14), // "on_dataChanged"
QT_MOC_LITERAL(8, 119, 7), // "topLeft"
QT_MOC_LITERAL(9, 127, 11), // "bottomRight"
QT_MOC_LITERAL(10, 139, 12), // "QVector<int>"
QT_MOC_LITERAL(11, 152, 5), // "roles"
QT_MOC_LITERAL(12, 158, 17), // "on_currentChanged"
QT_MOC_LITERAL(13, 176, 7), // "current"
QT_MOC_LITERAL(14, 184, 8), // "previous"
QT_MOC_LITERAL(15, 193, 15), // "on_rowsInserted"
QT_MOC_LITERAL(16, 209, 5), // "first"
QT_MOC_LITERAL(17, 215, 4), // "last"
QT_MOC_LITERAL(18, 220, 14), // "contextualMenu"
QT_MOC_LITERAL(19, 235, 16), // "copyTokenAddress"
QT_MOC_LITERAL(20, 252, 16), // "copyTokenBalance"
QT_MOC_LITERAL(21, 269, 13), // "copyTokenName"
QT_MOC_LITERAL(22, 283, 17), // "copySenderAddress"
QT_MOC_LITERAL(23, 301, 11) // "removeToken"

    },
    "ZRCToken\0on_goToSendTokenPage\0\0"
    "on_goToReceiveTokenPage\0on_goToAddTokenPage\0"
    "on_currentTokenChanged\0index\0"
    "on_dataChanged\0topLeft\0bottomRight\0"
    "QVector<int>\0roles\0on_currentChanged\0"
    "current\0previous\0on_rowsInserted\0first\0"
    "last\0contextualMenu\0copyTokenAddress\0"
    "copyTokenBalance\0copyTokenName\0"
    "copySenderAddress\0removeToken"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ZRCToken[] = {

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
       1,    0,   84,    2, 0x0a /* Public */,
       3,    0,   85,    2, 0x0a /* Public */,
       4,    0,   86,    2, 0x0a /* Public */,
       5,    1,   87,    2, 0x0a /* Public */,
       7,    3,   90,    2, 0x0a /* Public */,
       7,    2,   97,    2, 0x2a /* Public | MethodCloned */,
      12,    2,  102,    2, 0x0a /* Public */,
      15,    3,  107,    2, 0x0a /* Public */,
      18,    1,  114,    2, 0x0a /* Public */,
      19,    0,  117,    2, 0x0a /* Public */,
      20,    0,  118,    2, 0x0a /* Public */,
      21,    0,  119,    2, 0x0a /* Public */,
      22,    0,  120,    2, 0x0a /* Public */,
      23,    0,  121,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QModelIndex,    6,
    QMetaType::Void, QMetaType::QModelIndex, QMetaType::QModelIndex, 0x80000000 | 10,    8,    9,   11,
    QMetaType::Void, QMetaType::QModelIndex, QMetaType::QModelIndex,    8,    9,
    QMetaType::Void, QMetaType::QModelIndex, QMetaType::QModelIndex,   13,   14,
    QMetaType::Void, QMetaType::QModelIndex, QMetaType::Int, QMetaType::Int,    6,   16,   17,
    QMetaType::Void, QMetaType::QPoint,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void ZRCToken::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ZRCToken *_t = static_cast<ZRCToken *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_goToSendTokenPage(); break;
        case 1: _t->on_goToReceiveTokenPage(); break;
        case 2: _t->on_goToAddTokenPage(); break;
        case 3: _t->on_currentTokenChanged((*reinterpret_cast< QModelIndex(*)>(_a[1]))); break;
        case 4: _t->on_dataChanged((*reinterpret_cast< const QModelIndex(*)>(_a[1])),(*reinterpret_cast< const QModelIndex(*)>(_a[2])),(*reinterpret_cast< const QVector<int>(*)>(_a[3]))); break;
        case 5: _t->on_dataChanged((*reinterpret_cast< const QModelIndex(*)>(_a[1])),(*reinterpret_cast< const QModelIndex(*)>(_a[2]))); break;
        case 6: _t->on_currentChanged((*reinterpret_cast< QModelIndex(*)>(_a[1])),(*reinterpret_cast< QModelIndex(*)>(_a[2]))); break;
        case 7: _t->on_rowsInserted((*reinterpret_cast< QModelIndex(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 8: _t->contextualMenu((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 9: _t->copyTokenAddress(); break;
        case 10: _t->copyTokenBalance(); break;
        case 11: _t->copyTokenName(); break;
        case 12: _t->copySenderAddress(); break;
        case 13: _t->removeToken(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 2:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QVector<int> >(); break;
            }
            break;
        }
    }
}

const QMetaObject ZRCToken::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_ZRCToken.data,
      qt_meta_data_ZRCToken,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *ZRCToken::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ZRCToken::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZRCToken.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int ZRCToken::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
