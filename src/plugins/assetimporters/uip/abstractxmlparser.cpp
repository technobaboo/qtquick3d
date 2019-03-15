#include "abstractxmlparser.h"

#include <QtCore/QDebug>

QString AbstractXmlParser::readerErrorString() const
{
    return QObject::tr("Failed to parse %1: line %2: column %3: offset %4: %5")
            .arg(m_sourceInfo.fileName())
            .arg(m_reader.lineNumber())
            .arg(m_reader.columnNumber())
            .arg(m_reader.characterOffset())
            .arg(m_reader.errorString());
}

bool AbstractXmlParser::setSource(const QString &filename)
{
    if (m_sourceFile.isOpen())
        m_sourceFile.close();

    m_sourceFile.setFileName(filename);
    if (!m_sourceFile.exists()) {
        qWarning() << QObject::tr("Source file %1 does not exist").arg(filename);
        return false;
    }
    if (!m_sourceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << QObject::tr("Failed to open %1").arg(filename);
        return false;
    }

    m_parseTimer.start();

    m_sourceInfo = QFileInfo(filename);

    m_reader.setDevice(&m_sourceFile);

    return true;
}

bool AbstractXmlParser::setSourceData(const QByteArray &data)
{
    m_parseTimer.start();

    m_reader.clear();
    m_reader.addData(data);

    return true;
}

QXmlStreamReader *AbstractXmlParser::reader()
{
    return &m_reader;
}
