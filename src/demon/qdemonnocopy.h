#ifndef QDEMONNOCOPY_H
#define QDEMONNOCOPY_H

#include <QtDemon/qtdemonglobal.h>

QT_BEGIN_NAMESPACE

class Q_DEMON_EXPORT QDemonNoCopy
{
    QDemonNoCopy(const QDemonNoCopy &c);
    QDemonNoCopy &operator=(const QDemonNoCopy &c);

public:
    QDemonNoCopy() {}
};

QT_END_NAMESPACE

#endif // QDEMONNOCOPY_H
