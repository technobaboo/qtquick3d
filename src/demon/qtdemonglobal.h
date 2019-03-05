#ifndef QTDEMONGLOBAL_H
#define QTDEMONGLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#ifndef QT_STATIC
#if defined(QT_BUILD_DEMON_LIB)
#define Q_DEMON_EXPORT Q_DECL_EXPORT
#else
#define Q_DEMON_EXPORT Q_DECL_IMPORT
#endif
#else
#define Q_DEMON_EXPORT
#endif

template<typename T>
class QDemonRef
{
    T *d;

public:
    T *data() const { return d; }
    T *get() const { return d; }
    T *take()
    {
        T *t = d;
        d = nullptr;
        return t;
    }
    bool isNull() const { return !d; }
    operator bool() const { return d; }
    bool operator!() const { return !d; }
    T &operator*() const { return *d; }
    T *operator->() const { return d; }

    // constructors
    constexpr QDemonRef() : d(nullptr) {}
    constexpr QDemonRef(std::nullptr_t) : d(nullptr) {}

    QDemonRef(T *ptr) : d(ptr)
    {
        if (d)
            d->ref.ref();
    }
    template<typename X>
    QDemonRef(X *ptr) : d(ptr)
    {
        if (d)
            d->ref.ref();
    }
    QDemonRef(const QDemonRef<T> &other) : d(other.d)
    {
        if (d)
            d->ref.ref();
    }
    template<typename X>
    QDemonRef(const QDemonRef<X> &other) : d(other.get())
    {
        if (d)
            d->ref.ref();
    }

    QDemonRef(QDemonRef<T> &&other) : d(other.take()) {}
    template<typename X>
    QDemonRef(QDemonRef<X> &&other) : d(other.take())
    {
    }

    ~QDemonRef()
    {
        if (d && !d->ref.deref())
            delete d;
    }

    template<typename X>
    QDemonRef<T> &operator=(const QDemonRef<X> &other)
    {
        if (d != other.get()) {
            if (d && !d->ref.deref())
                delete d;
            d = other.get();
            if (d)
                d->ref.ref();
        }
        return *this;
    }
    template<typename X>
    QDemonRef<T> &operator=(QDemonRef<X> &&other)
    {
        clear();
        d = other.take();
        return *this;
    }
    QDemonRef<T> &operator=(const QDemonRef<T> &other)
    {
        if (d != other.get()) {
            if (d && !d->ref.deref())
                delete d;
            d = other.get();
            if (d)
                d->ref.ref();
        }
        return *this;
    }
    QDemonRef<T> &operator=(QDemonRef<T> &&other)
    {
        qSwap(d, other.d);
        return *this;
    }

    void swap(QDemonRef<T> &other) { qSwap(d, other.d); }

    void clear()
    {
        if (d && !d->ref.deref())
            delete d;
        d = nullptr;
    }
};

template<class T, class X>
bool operator==(const QDemonRef<T> &ptr1, const QDemonRef<X> &ptr2)
{
    return ptr1.get() == ptr2.get();
}
template<class T, class X>
bool operator!=(const QDemonRef<T> &ptr1, const QDemonRef<X> &ptr2)
{
    return ptr1.get() != ptr2.get();
}
template<class T, class X>
bool operator==(const QDemonRef<T> &ptr1, const X *ptr2)
{
    return ptr1.get() == ptr2;
}
template<class T, class X>
bool operator!=(const QDemonRef<T> &ptr1, const X *ptr2)
{
    return ptr1.get() != ptr2;
}
template<class T, class X>
bool operator==(const T *ptr1, const QDemonRef<X> &ptr2)
{
    return ptr1 == ptr2.get();
}
template<class T, class X>
bool operator!=(const T *ptr1, const QDemonRef<X> &ptr2)
{
    return ptr1 != ptr2.get();
}
template<class T>
bool operator==(const QDemonRef<T> &lhs, std::nullptr_t)
{
    return !lhs.get();
}
template<class T>
bool operator!=(const QDemonRef<T> &lhs, std::nullptr_t)
{
    return lhs.get();
}
template<class T>
bool operator==(std::nullptr_t, const QDemonRef<T> &rhs)
{
    return !rhs.get();
}
template<class T>
bool operator!=(std::nullptr_t, const QDemonRef<T> &rhs)
{
    return rhs.get();
}

QT_END_NAMESPACE

#endif // QTDEMONGLOBAL_H
