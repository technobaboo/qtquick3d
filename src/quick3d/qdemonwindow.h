#ifndef QDEMONWINDOW_H
#define QDEMONWINDOW_H

#include <QtCore/qmetatype.h>
#include <QtGui/QWindow>

#include <QtQuick3d/qtquick3dglobal.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QDemonWindow : public QWindow
{
    Q_OBJECT
public:
    QDemonWindow();
};

QT_END_NAMESPACE

#endif // QDEMONWINDOW_H
