#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QColor>

inline QString insertTabs(int n)
{
    QString tabs;
    for (int i = 0; i < n; ++i)
        tabs += "    ";
    return tabs;
}



inline QString qmlComponentName(const QString &name) {
    QString nameCopy = name;
    if (nameCopy.isEmpty())
        return QStringLiteral("Presentation");

    if (nameCopy[0].isLower())
        nameCopy[0] = nameCopy[0].toUpper();

    return nameCopy;
}

inline QString colorToQml(const QColor &color) {
    QString colorString;
    colorString = QStringLiteral("Qt.rgba(") + QString::number(color.redF()) +
            QStringLiteral(", ") + QString::number(color.greenF()) +
            QStringLiteral(", ") + QString::number(color.blueF()) +
            QStringLiteral(", ") + QString::number(color.alphaF()) +
            QStringLiteral(")");
    return colorString;
}

#endif // UTILS_H
