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
#ifndef QDEMON_RENDER_INPUT_STREAM_FACTORY_H
#define QDEMON_RENDER_INPUT_STREAM_FACTORY_H

#include <QtDemonRuntimeRender/qtdemonruntimerenderglobal.h>

#include <QtDemon/qdemondataref.h>
#include <QtCore/QSharedPointer>
#include <QtCore/QIODevice>
#include <QtCore/qmutex.h>

QT_BEGIN_NAMESPACE
// This class is threadsafe.
class Q_DEMONRUNTIMERENDER_EXPORT QDemonInputStreamFactory
{
public:
    QAtomicInt ref;
    QMutex m_mutex;

    QDemonInputStreamFactory();

    ~QDemonInputStreamFactory();
    // These directories must have a '/' on them
    void addSearchDirectory(const QString &inDirectory);
    QSharedPointer<QIODevice> getStreamForFile(const QString &inFilename, bool inQuiet = false);
    // Return a path for this file.  Returns true if GetStreamForFile would return a valid
    // stream.
    // else returns false
    bool getPathForFile(const QString &inFilename, QString &outFile, bool inQuiet = false);
};
QT_END_NAMESPACE

#endif
