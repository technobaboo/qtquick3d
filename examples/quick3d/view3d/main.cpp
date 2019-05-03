#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQuick3d/QDemonView3D>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);
    QSurfaceFormat::setDefaultFormat(QDemonView3D::idealSurfaceFormat());

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
