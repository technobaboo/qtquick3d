#ifndef QDEMONUNIONCAST_H
#define QDEMONUNIONCAST_H

#include <QtDemon/qtdemonglobal.h>

QT_BEGIN_NAMESPACE

template <class A, class B>
A QDemonUnionCast(B b)
{
    union AB {
        AB(B bb)
            : _b(bb)
        {
        }
        B _b;
        A _a;
    } u(b);
    return u._a;
}

QT_END_NAMESPACE

#endif // QDEMONUNIONCAST_H
