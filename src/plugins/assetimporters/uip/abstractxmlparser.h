#ifndef ABSTRACTXMLPARSER_H
#define ABSTRACTXMLPARSER_H

#include <QXmlStreamReader>
#include <QFileInfo>
#include <QFile>
#include <QElapsedTimer>

QT_BEGIN_NAMESPACE

class AbstractXmlParser
{
public:
    virtual ~AbstractXmlParser() = default;

    QFileInfo *sourceInfo() { return &m_sourceInfo; }

    quint64 elapsedSinceSetSource() const { return m_parseTimer.elapsed(); }
    QString readerErrorString() const;

protected:
    bool setSource(const QString &filename);
    bool setSourceData(const QByteArray &data);
    QXmlStreamReader *reader();

private:
    QXmlStreamReader m_reader;
    QFileInfo m_sourceInfo;
    QFile m_sourceFile;
    QElapsedTimer m_parseTimer;
};

QT_END_NAMESPACE

#endif // ABSTRACTXMLPARSER_H
