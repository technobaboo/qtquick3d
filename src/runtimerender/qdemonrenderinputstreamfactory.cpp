/****************************************************************************
**
** Copyright (C) 2008-2012 NVIDIA Corporation.
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt 3D Studio.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "qdemonrenderinputstreamfactory.h"

#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QUrl>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

#include <limits>

QT_BEGIN_NAMESPACE

namespace {
struct SInputStream : public IInputStream
{
    QString m_Path;
    QFile m_File;

    SInputStream(const QString &inPath)
        : m_Path(inPath)
        , m_File(inPath)
    {
        m_File.open(QIODevice::ReadOnly);
    }
    virtual ~SInputStream() override
    {
    }

    quint32 Read(QDemonDataRef<quint8> data) override
    {
        return m_File.read((char *)data.begin(), data.size());
    }

    bool Write(QDemonConstDataRef<quint8> /*data*/) override
    {
        Q_ASSERT(false);
        return false;
    }

    void SetPosition(qint64 inOffset) override
    {
        if (inOffset > std::numeric_limits<qint32>::max() || inOffset < std::numeric_limits<qint32>::min()) {
            qCritical("Attempt to seek further than platform allows");
            Q_ASSERT(false);
            return;
        } else {
            m_File.seek(inOffset);
        }
    }
    qint64 GetPosition() const override
    {
        return m_File.pos();
    }

    static SInputStream *OpenFile(const QString &inPath)
    {
        return new SInputStream(inPath);
    }
};

QString NormalizePathForQtUsage(const QString &path)
{
    // path can be a file path or a qrc URL string.

    QString filePath = QDir::cleanPath(path);

    if (filePath.startsWith(QLatin1String("qrc:/")))
        return filePath.mid(3);
    else
        return filePath;
}

struct SFactory : public IInputStreamFactory
{
    QMutex m_Mutex;

    const QString Q3DSTUDIO_TAG = QStringLiteral("qt3dstudio");

    SFactory()
    {
        // Add the top-level qrc directory
        if (!QDir::searchPaths(Q3DSTUDIO_TAG).contains(QLatin1String(":/")))
            QDir::addSearchPath(Q3DSTUDIO_TAG, QStringLiteral(":/"));
    }

    QFileInfo matchCaseInsensitiveFile(const QString& file)
    {
//        qCWarning(WARNING, PERF_INFO, "Case-insensitive matching with file: %s",
//                  file.toLatin1().constData());
        const QStringList searchDirectories = QDir::searchPaths(Q3DSTUDIO_TAG);
        for (const auto &directoryPath : searchDirectories) {
            QFileInfo fileInfo(file);
            QDirIterator it(directoryPath, {fileInfo.fileName()}, QDir::NoFilter,
                            QDirIterator::Subdirectories);
            while (it.hasNext()) {
                QString filePath = it.next();
                if (filePath.compare(QDir::cleanPath(directoryPath + '/' + file),
                                     Qt::CaseInsensitive) == 0) {
                    return QFileInfo(filePath);
                }
            }
        }

        return QFileInfo();
    }

    void AddSearchDirectory(const char *inDirectory) override
    {
        QMutexLocker factoryLocker(&m_Mutex);
        QString localDir = NormalizePathForQtUsage(QString::fromLocal8Bit(inDirectory));
        QDir directory(localDir);
        if (!directory.exists()) {
            qCritical("Adding search directory: %s", inDirectory);
            return;
        }

        if (!QDir::searchPaths(Q3DSTUDIO_TAG).contains(localDir))
            QDir::addSearchPath(Q3DSTUDIO_TAG, localDir);
    }


    QSharedPointer<IInputStream> GetStreamForFile(const QString &inFilename, bool inQuiet) override
    {
        QMutexLocker factoryLocker(&m_Mutex);
        QString localFile = NormalizePathForQtUsage(inFilename);
        QFileInfo fileInfo = QFileInfo(localFile);
        SInputStream *inputStream = nullptr;
        // Try to match the file with the search paths
        if (!fileInfo.exists())
            fileInfo.setFile(QStringLiteral("qt3dstudio:") + localFile);

        // Try to match the case-insensitive file with the given search paths
        if (!fileInfo.exists())
            fileInfo = matchCaseInsensitiveFile(localFile);

        if (fileInfo.exists())
            inputStream = SInputStream::OpenFile(fileInfo.absoluteFilePath());

        if (!inputStream && !inQuiet) {
            // Print extensive debugging information.
            qCritical("Failed to find file: %s", inFilename.toLatin1().data());
            qCritical("Searched path: %s", QDir::searchPaths(Q3DSTUDIO_TAG).join(',').toLatin1().constData());
        }
        return QSharedPointer<IInputStream>(inputStream);
    }

    bool GetPathForFile(const QString &inFilename, QString &outFile,
                        bool inQuiet = false) override
    {
        QSharedPointer<IInputStream> theStream = GetStreamForFile(inFilename, inQuiet);
        if (theStream) {
            SInputStream *theRealStream = static_cast<SInputStream *>(theStream.data());
            outFile = theRealStream->m_Path;
            return true;
        }
        return false;
    }
};
}

IInputStreamFactory &IInputStreamFactory::Create()
{
    return *new SFactory();
}

QT_END_NAMESPACE
