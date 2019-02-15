#ifndef QDEMONINVASIVESET_H
#define QDEMONINVASIVESET_H

#include <QtDemon/qtdemonglobal.h>

#include <QtCore/QVector>
#include <limits>

QT_BEGIN_NAMESPACE

template <typename TObjectType, typename TGetSetIndexOp, typename TSetSetIndexOp>
class InvasiveSet
{
    QVector<TObjectType *> mSet;

    InvasiveSet(const InvasiveSet &other);
    InvasiveSet &operator=(const InvasiveSet &other);

public:
    InvasiveSet()
    {
    }

    bool insert(TObjectType &inObject)
    {
        quint32 currentIdx = TGetSetIndexOp()(inObject);
        if (currentIdx == std::numeric_limits<quint32>::max()) {
            TSetSetIndexOp()(inObject, mSet.size());
            mSet.push_back(&inObject);
            return true;
        }
        return false;
    }

    bool remove(TObjectType &inObject)
    {
        quint32 currentIdx = TGetSetIndexOp()(inObject);
        if (currentIdx != std::numeric_limits<quint32>::max()) {
            TObjectType *theEnd = mSet.back();
            TObjectType *theObj = &inObject;
            if (theEnd != theObj) {
                TSetSetIndexOp()(*theEnd, currentIdx);
                mSet[currentIdx] = theEnd;
            }
            mSet.pop_back();
            TSetSetIndexOp()(inObject, std::numeric_limits<quint32>::max());
            return true;
        }
        return false;
    }

    bool contains(TObjectType &inObject) { return TGetSetIndexOp()(inObject) != std::numeric_limits<quint32>::max(); }

    void clear()
    {
        for (int idx = 0; idx < mSet.size(); ++idx)
            TSetSetIndexOp()(*(mSet[idx]), std::numeric_limits<quint32>::max());
        mSet.clear();
    }

    TObjectType *operator[](quint32 idx) { return mSet[idx]; }
    const TObjectType *operator[](quint32 idx) const { return mSet[idx]; }
    quint32 size() const { return mSet.size(); }
    TObjectType **begin() { return mSet.begin(); }
    TObjectType **end() { return mSet.end(); }
    const TObjectType **begin() const { return mSet.begin(); }
    const TObjectType **end() const { return mSet.end(); }
    const TObjectType *back() const { return mSet.back(); }
    TObjectType *back() { return mSet.back(); }
};

QT_END_NAMESPACE

#endif // QDEMONINVASIVESET_H
