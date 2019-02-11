#ifndef QTQUICK3DGLOBAL_H
#define QTQUICK3DGLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#ifndef QT_STATIC
#  if defined(QT_BUILD_QUICK3D_LIB)
#    define Q_QUICK3D_EXPORT Q_DECL_EXPORT
#  else
#    define Q_QUICK3D_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define Q_QUICK3D_EXPORT
#endif

QT_END_NAMESPACE

#endif // QTQUICK3DGLOBAL_H
