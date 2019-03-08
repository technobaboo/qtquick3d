#ifndef QDEMONDATAREF_H
#define QDEMONDATAREF_H

#include <QtDemon/qtdemonglobal.h>

QT_BEGIN_NAMESPACE

template<typename TDataType>
struct QDemonConstDataRef
{
    const TDataType *mData;
    int mSize;

    QDemonConstDataRef(const TDataType *inData, qint32 inSize) : mData(inData), mSize(inSize) { Q_ASSERT(mSize > -1); }
    QDemonConstDataRef() : mData(nullptr), mSize(0) {}

    qint32 size() const { return mSize; }

    const TDataType *begin() const { return mData; }
    const TDataType *end() const { return mData + mSize; }

    const TDataType &operator[](int index) const
    {
        Q_ASSERT(index > -1);
        Q_ASSERT(index < mSize);
        return mData[index];
    }
};

template<typename TDataType>
inline QDemonConstDataRef<TDataType> toConstDataRef(const TDataType &type)
{
    return QDemonConstDataRef<TDataType>(&type, 1);
}

template<typename TDataType>
inline QDemonConstDataRef<quint8> toU8ConstDataRef(const TDataType &type)
{
    return QDemonConstDataRef<quint8>(reinterpret_cast<const quint8 *>(&type), sizeof(TDataType));
}

template<typename TDataType>
inline QDemonConstDataRef<TDataType> toConstDataRef(const TDataType *type, quint32 count)
{
    return QDemonConstDataRef<TDataType>(type, count);
}

template<typename TDataType>
inline QDemonConstDataRef<quint8> toU8ConstDataRef(const TDataType *type, quint32 count)
{
    return QDemonConstDataRef<quint8>(reinterpret_cast<const quint8 *>(type), sizeof(TDataType) * count);
}

template<typename TDataType>
struct QDemonDataRef
{
    TDataType *mData;
    qint32 mSize;

    QDemonDataRef(TDataType *inData, qint32 inSize) : mData(inData), mSize(inSize) { Q_ASSERT(inSize >= 0); }
    QDemonDataRef() : mData(nullptr), mSize(0) {}
    qint32 size() const { return mSize; }

    TDataType *begin() { return mData; }
    TDataType *end() { return mData + mSize; }

    TDataType *begin() const { return mData; }
    TDataType *end() const { return mData + mSize; }

    TDataType &operator[](qint32 index)
    {
        Q_ASSERT(index >= 0);
        Q_ASSERT(index < mSize);
        return mData[index];
    }

    const TDataType &operator[](qint32 index) const
    {
        Q_ASSERT(index >= 0);
        Q_ASSERT(index < mSize);
        return mData[index];
    }

    operator QDemonConstDataRef<TDataType>() const { return QDemonConstDataRef<TDataType>(mData, mSize); }
};

template<typename TDataType>
inline QDemonDataRef<TDataType> toDataRef(TDataType &type)
{
    return QDemonDataRef<TDataType>(&type, 1);
}

template<typename TDataType>
inline QDemonDataRef<quint8> toU8DataRef(TDataType &type)
{
    return QDemonDataRef<quint8>(reinterpret_cast<quint8 *>(&type), sizeof(TDataType));
}

template<typename TDataType>
inline QDemonDataRef<TDataType> toDataRef(TDataType *type, quint32 count)
{
    return QDemonDataRef<TDataType>(type, count);
}

template<typename TDataType>
inline QDemonDataRef<quint8> toU8DataRef(TDataType *type, quint32 count)
{
    return QDemonDataRef<quint8>(reinterpret_cast<quint8 *>(type), sizeof(TDataType) * count);
}

QT_END_NAMESPACE

#endif // QDEMONDATAREF_H
