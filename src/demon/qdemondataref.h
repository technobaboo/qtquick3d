#ifndef QDEMONDATAREF_H
#define QDEMONDATAREF_H

#include <QtDemon/qtdemonglobal.h>

#include <QtCore/qvector.h>
#include <QtCore/qbytearray.h>

QT_BEGIN_NAMESPACE

template<typename T>
struct QDemonDataView
{
    const T *mData;
    int mSize;

    explicit QDemonDataView(const QVector<T> &data) : mData(data.constBegin()), mSize(data.size()) { Q_ASSERT(mSize >= 0); }
    constexpr QDemonDataView(const T *inData, qint32 inSize) : mData(inData), mSize(inSize) { Q_ASSERT(mSize >= 0); }
    constexpr QDemonDataView() : mData(nullptr), mSize(0) {}

    qint32 size() const { return mSize; }

    const T *begin() const { return mData; }
    const T *end() const { return mData + mSize; }

    const T &operator[](int index) const
    {
        Q_ASSERT(index > -1);
        Q_ASSERT(index < mSize);
        return mData[index];
    }

    void clear()
    {
        mData = nullptr;
        mSize = 0;
    }

    operator const void *() { return reinterpret_cast<const void *>(mData); }
};

template<>
struct QDemonDataView<quint8>
{
    const quint8 *mData;
    int mSize;

    explicit QDemonDataView(const QByteArray &data)
        : mData(reinterpret_cast<const quint8 *>(data.constBegin())), mSize(data.size())
    { Q_ASSERT(mSize >= 0); }
    template<typename T>
    explicit QDemonDataView(const QVector<T> &data)
        : mData(reinterpret_cast<const quint8 *>(data.constBegin())), mSize(data.size()*sizeof(T))
    { Q_ASSERT(mSize >= 0); }
    constexpr QDemonDataView(const quint8 *inData, qint32 inSize) : mData(inData), mSize(inSize) { Q_ASSERT(mSize >= 0); }
    template<typename T>
    constexpr QDemonDataView(const T *inData, qint32 inSize)
        : mData(reinterpret_cast<const quint8 *>(inData)), mSize(inSize*sizeof(T))
    { Q_ASSERT(mSize >= 0); }
    constexpr QDemonDataView() : mData(nullptr), mSize(0) {}

    qint32 size() const { return mSize; }

    const quint8 *begin() const { return mData; }
    const quint8 *end() const { return mData + mSize; }

    const quint8 &operator[](int index) const
    {
        Q_ASSERT(index > -1);
        Q_ASSERT(index < mSize);
        return mData[index];
    }

    operator const void *() { return reinterpret_cast<const void *>(mData); }
};

using QDemonByteView = QDemonDataView<quint8>;

template<typename T>
inline QDemonDataView<T> toDataView(const T &type)
{
    return QDemonDataView<T>(&type, 1);
}

template<typename T>
inline QDemonDataView<T> toDataView(const QVector<T> &type)
{
    return QDemonDataView<T>(type);
}

template<typename T>
inline QDemonByteView toByteView(const T &type)
{
    return QDemonByteView(&type, 1);
}

template<typename T>
inline QDemonByteView toByteView(const QVector<T> &type)
{
    return QDemonByteView(type);
}

template<>
inline QDemonByteView toByteView(const QByteArray &type)
{
    return QDemonByteView(type);
}

template<typename T>
inline QDemonDataView<T> toDataView(const T *type, quint32 count)
{
    return QDemonDataView<T>(type, count);
}

template<typename T>
inline QDemonByteView toByteView(const T *type, quint32 count)
{
    return QDemonByteView(type, count);
}

template<typename T>
struct QDemonDataRef
{
    T *mData;
    qint32 mSize;

    QDemonDataRef(T *inData, qint32 inSize) : mData(inData), mSize(inSize) { Q_ASSERT(inSize >= 0); }
    QDemonDataRef() : mData(nullptr), mSize(0) {}
    qint32 size() const { return mSize; }

    T *begin() { return mData; }
    T *end() { return mData + mSize; }

    T *begin() const { return mData; }
    T *end() const { return mData + mSize; }

    T &operator[](qint32 index)
    {
        Q_ASSERT(index >= 0);
        Q_ASSERT(index < mSize);
        return mData[index];
    }

    const T &operator[](qint32 index) const
    {
        Q_ASSERT(index >= 0);
        Q_ASSERT(index < mSize);
        return mData[index];
    }

    void clear()
    {
        mData = nullptr;
        mSize = 0;
    }

    operator QDemonDataView<T>() const { return QDemonDataView<T>(mData, mSize); }
};

using QDemonByteRef = QDemonDataRef<quint8>;

template<typename T>
inline QDemonDataRef<T> toDataRef(T &type)
{
    return QDemonDataRef<T>(&type, 1);
}

template<typename T>
inline QDemonByteRef toByteRef(T &type)
{
    return QDemonByteRef(reinterpret_cast<quint8 *>(&type), sizeof(T));
}

template<typename T>
inline QDemonDataRef<T> toDataRef(T *type, quint32 count)
{
    return QDemonDataRef<T>(type, count);
}

template<typename T>
inline QDemonByteRef toByteRef(T *type, quint32 count)
{
    return QDemonByteRef(reinterpret_cast<quint8 *>(type), sizeof(T) * count);
}

QT_END_NAMESPACE

#endif // QDEMONDATAREF_H
