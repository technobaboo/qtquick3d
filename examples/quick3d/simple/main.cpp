#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QtGui>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

#ifdef Q_OS_LINUX
    // We need this for renderdoc
    QSurfaceFormat df = QSurfaceFormat::defaultFormat();
    df.setVersion(3, 3);
    QSurfaceFormat::setDefaultFormat(df);
#endif

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
