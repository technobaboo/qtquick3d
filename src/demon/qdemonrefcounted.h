#ifndef QDEMONREFCOUNTED_H
#define QDEMONREFCOUNTED_H

#include <QtDemon/qdemonnocopy.h>
#include <QtDemon/qtdemonglobal.h>

QT_BEGIN_NAMESPACE

class QDemonReleasable
{
protected:
    virtual ~QDemonReleasable() {}
public:
    virtual void release() = 0;
};

template <typename TObjType>
inline void QDemonSafeRelease(TObjType *&item)
{
    if (item) {
        item->release();
        item = nullptr;
    }
}

/**Scoped pointer that releases its data
        when it is being destroyed*/
template <typename TObjType>
struct QDemonScopedReleasable : public QDemonNoCopy
{
    TObjType *mPtr;
    QDemonScopedReleasable()
        : mPtr(nullptr)
    {
    }
    QDemonScopedReleasable(TObjType *item)
        : mPtr(item)
    {
    }
    QDemonScopedReleasable(TObjType &item)
        : mPtr(&item)
    {
    }
    ~QDemonScopedReleasable() { QDemonSafeRelease(mPtr); }

    QDemonScopedReleasable &operator=(TObjType *inItem)
    {
        if (inItem != mPtr) {
            if (mPtr)
                mPtr->release();
            mPtr = inItem;
        }
        return *this;
    }

    QDemonScopedReleasable &operator=(const QDemonScopedReleasable<TObjType> inItem)
    {
        // try to do the right thing.
        mPtr = inItem.mPtr;
        const_cast<QDemonScopedReleasable<TObjType> &>(inItem).mPtr = nullptr;
        return *this;
    }

    TObjType *forget_unsafe()
    {
        mPtr = nullptr;
        return mPtr;
    }

    TObjType *operator->() { return mPtr; }
    const TObjType *operator->() const { return mPtr; }
    TObjType &operator*() { return *mPtr; }
    const TObjType &operator*() const { return *mPtr; }
    operator TObjType *() { return mPtr; }
    operator const TObjType *() const { return mPtr; }
};

// Marker class for objects that are ref counted.
class Q_DEMON_EXPORT QDemonRefCounted : public QDemonReleasable
{
public:
    virtual void addRef() = 0;
};

/**Helpers to make implementing ref counted objects as concise as possible*/
#define QDEMON_IMPLEMENT_REF_COUNT_RELEASE(alloc)                                                      \
quint32 value = atomicDecrement(&mRefCount);                                                     \
if (value <= 0)                                                                                \
    NVDelete(alloc, this);

#define QDEMON_IMPLEMENT_REF_COUNT_ADDREF_RELEASE(alloc)                                               \
void addRef() { atomicIncrement(&mRefCount); }                                                 \
void release() { QDEMON_IMPLEMENT_REF_COUNT_RELEASE(alloc); }


#define QDEMON_IMPLEMENT_REF_COUNT_ADDREF_RELEASE_OVERRIDE(alloc)                                               \
void addRef() override { atomicIncrement(&mRefCount); }                                                 \
void release() override { QDEMON_IMPLEMENT_REF_COUNT_RELEASE(alloc); }

/**Safe function that checks for null before addrefing the object*/
template <typename TObjType>
inline TObjType *QDemonSafeAddRef(TObjType *item)
{
    if (item) {
        item->addRef();
    }
    return item;
}

/**Scoped pointer that addref's its data upon acquisition and releases its data
        when it is being destroyed*/
template <typename TObjType>
struct Q_DEMON_EXPORT QDemonScopedRefCounted
{
    TObjType *mPtr;
    ~QDemonScopedRefCounted() { QDemonSafeRelease(mPtr); }
    QDemonScopedRefCounted(TObjType *item = nullptr)
        : mPtr(item)
    {
        QDemonSafeAddRef(mPtr);
    }
    QDemonScopedRefCounted(TObjType &item)
        : mPtr(&item)
    {
        QDemonSafeAddRef(mPtr);
    }
    QDemonScopedRefCounted(const QDemonScopedRefCounted<TObjType> &other)
        : mPtr(const_cast<TObjType *>(other.mPtr))
    {
        QDemonSafeAddRef(mPtr);
    }
    QDemonScopedRefCounted<TObjType> &operator=(const QDemonScopedRefCounted<TObjType> &other)
    {
        if (other.mPtr != mPtr) {
            QDemonSafeRelease(mPtr);
            mPtr = const_cast<TObjType *>(other.mPtr);
            QDemonSafeAddRef(mPtr);
        }
        return *this;
    }
    TObjType *forget_unsafe()
    {
        mPtr = nullptr;
        return mPtr;
    }

    TObjType *operator->() { return mPtr; }
    const TObjType *operator->() const { return mPtr; }
    TObjType &operator*() { return *mPtr; }
    const TObjType &operator*() const { return *mPtr; }
    operator TObjType *() { return mPtr; }
    operator const TObjType *() const { return mPtr; }
    bool operator==(QDemonScopedRefCounted<TObjType> &inOther) const
    {
        return mPtr == inOther.mPtr;
    }
    bool operator!=(QDemonScopedRefCounted<TObjType> &inOther) const
    {
        return mPtr != inOther.mPtr;
    }
};

QT_END_NAMESPACE

#endif // QDEMONREFCOUNTED_H
