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

#include <QtDemon/qdemonutils.h>

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
class QDemonInputStream : public QFile
{
public:
    QDemonInputStream(const QString &inPath) : QFile(inPath), m_path(inPath) {}
    virtual ~QDemonInputStream() override {}
    QString path() const { return m_path; }

private:
    QString m_path;
};

struct QDemonInputStreamFactory : public QDemonInputStreamFactoryInterface
{
    QMutex m_mutex;

    const QString Q3DSTUDIO_TAG = QStringLiteral("qt3dstudio");

    QDemonInputStreamFactory()
    {
        // Add the top-level qrc directory
        if (!QDir::searchPaths(Q3DSTUDIO_TAG).contains(QLatin1String(":/")))
            QDir::addSearchPath(Q3DSTUDIO_TAG, QStringLiteral(":/"));
    }

    QFileInfo matchCaseInsensitiveFile(const QString &file)
    {
        //        qCWarning(WARNING, PERF_INFO, "Case-insensitive matching with file: %s",
        //                  file.toLatin1().constData());
        const QStringList searchDirectories = QDir::searchPaths(Q3DSTUDIO_TAG);
        for (const auto &directoryPath : searchDirectories) {
            QFileInfo fileInfo(file);
            QDirIterator it(directoryPath, { fileInfo.fileName() }, QDir::NoFilter, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                QString filePath = it.next();
                if (filePath.compare(QDir::cleanPath(directoryPath + '/' + file), Qt::CaseInsensitive) == 0) {
                    return QFileInfo(filePath);
                }
            }
        }

        return QFileInfo();
    }

    void addSearchDirectory(const char *inDirectory) override
    {
        QMutexLocker factoryLocker(&m_mutex);
        QString localDir = CFileTools::normalizePathForQtUsage(QString::fromLocal8Bit(inDirectory));
        QDir directory(localDir);
        if (!directory.exists()) {
            qCritical("Adding search directory: %s", inDirectory);
            return;
        }

        if (!QDir::searchPaths(Q3DSTUDIO_TAG).contains(localDir))
            QDir::addSearchPath(Q3DSTUDIO_TAG, localDir);
    }

    QSharedPointer<QIODevice> getStreamForFile(const QString &inFilename, bool inQuiet) override
    {
        QMutexLocker factoryLocker(&m_mutex);
        QString localFile = CFileTools::normalizePathForQtUsage(inFilename);
        QFileInfo fileInfo = QFileInfo(localFile);
        QIODevice *inputStream = nullptr;
        // Try to match the file with the search paths
        if (!fileInfo.exists())
            fileInfo.setFile(QStringLiteral("qt3dstudio:") + localFile);

        // Try to match the case-insensitive file with the given search paths
        if (!fileInfo.exists())
            fileInfo = matchCaseInsensitiveFile(localFile);

        if (fileInfo.exists()) {
            QDemonInputStream *file = new QDemonInputStream(fileInfo.absoluteFilePath());
            if (file->open(QIODevice::ReadOnly))
                inputStream = file;
        }

        if (!inputStream && !inQuiet) {
            // Print extensive debugging information.
            qCritical("Failed to find file: %s", inFilename.toLatin1().data());
            qCritical("Searched path: %s", QDir::searchPaths(Q3DSTUDIO_TAG).join(',').toLatin1().constData());
        }
        return QSharedPointer<QIODevice>(inputStream);
    }

    bool getPathForFile(const QString &inFilename, QString &outFile, bool inQuiet = false) override
    {
        QSharedPointer<QIODevice> theStream = getStreamForFile(inFilename, inQuiet);
        if (theStream) {
            QDemonInputStream *theRealStream = static_cast<QDemonInputStream *>(theStream.data());
            outFile = theRealStream->path();
            return true;
        }
        return false;
    }
};
}

QDemonRef<QDemonInputStreamFactoryInterface> QDemonInputStreamFactoryInterface::create()
{
    return QDemonRef<QDemonInputStreamFactoryInterface>(new QDemonInputStreamFactory());
}

QT_END_NAMESPACE
