#ifndef QTDEMONGLOBAL_H
#define QTDEMONGLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#ifndef QT_STATIC
#  if defined(QT_BUILD_DEMON_LIB)
#    define Q_DEMON_EXPORT Q_DECL_EXPORT
#  else
#    define Q_DEMON_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define Q_DEMON_EXPORT
#endif

QT_END_NAMESPACE

#endif // QTDEMONGLOBAL_H
