#ifndef QDEMONQDemonOption_H
#define QDEMONQDemonOption_H

#include <QtDemon/qtdemonglobal.h>

QT_BEGIN_NAMESPACE

struct QDemonEmpty
{
};

template<typename TDataType>
class QDemonOption
{
    TDataType mData;
    bool mHasValue;

public:
    QDemonOption(const TDataType &data) : mData(data), mHasValue(true) {}
    QDemonOption(const QDemonEmpty &) : mHasValue(false) {}
    QDemonOption() : mHasValue(false) {}
    QDemonOption(const QDemonOption &other) : mData(other.mData), mHasValue(other.mHasValue) {}
    QDemonOption &operator=(const QDemonOption &other)
    {
        mData = other.mData;
        mHasValue = other.mHasValue;
        return *this;
    }

    bool isEmpty() const { return !mHasValue; }
    void setEmpty() { mHasValue = false; }
    bool hasValue() const { return mHasValue; }

    const TDataType &getValue() const
    {
        Q_ASSERT(mHasValue);
        return mData;
    }
    TDataType &getValue()
    {
        Q_ASSERT(mHasValue);
        return mData;
    }
    TDataType &unsafeGetValue() { return mData; }

    operator const TDataType &() const { return getValue(); }
    operator TDataType &() { return getValue(); }

    const TDataType *operator->() const { return &getValue(); }
    TDataType *operator->() { return &getValue(); }

    const TDataType &operator*() const { return getValue(); }
    TDataType &operator*() { return getValue(); }
};

QT_END_NAMESPACE

#endif // QDEMONQDemonOption_H
