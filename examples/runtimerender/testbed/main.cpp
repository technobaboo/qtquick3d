#include <QtGui/QGuiApplication>
#include <QtGui/QSurfaceFormat>
#include <QtCore/QScopedPointer>

#include "renderwindow.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QSurfaceFormat fmt;
    fmt.setProfile(QSurfaceFormat::CoreProfile);

    // Advanced: Try 4.3 core (so we get compute shaders for instance)
    fmt.setVersion(4, 3);

    QScopedPointer<RenderWindow> renderWindow(new RenderWindow());

    renderWindow->setFormat(fmt);
    renderWindow->show();

    return app.exec();
}
