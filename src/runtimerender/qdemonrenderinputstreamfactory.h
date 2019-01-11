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



QT_BEGIN_NAMESPACE
// This class is threadsafe.
class IInputStreamFactory
{
protected:
    virtual ~IInputStreamFactory() {}
public:
    // These directories must have a '/' on them
    virtual void AddSearchDirectory(const char *inDirectory) = 0;
    virtual QSharedPointer<QIODevice> GetStreamForFile(const QString &inFilename, bool inQuiet = false) = 0;
    // Return a path for this file.  Returns true if GetStreamForFile would return a valid
    // stream.
    // else returns false
    virtual bool GetPathForFile(const QString &inFilename, QString &outFile, bool inQuiet = false) = 0;

    // Create an input stream factory using this foundation and an platform-optional app
    // directory
    // on android the app directory has no effect; use use the assets bundled with the APK file.
    static QSharedPointer<IInputStreamFactory> Create();
};
QT_END_NAMESPACE

#endif
