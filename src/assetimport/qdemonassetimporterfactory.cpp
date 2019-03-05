/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
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

#include "qdemonassetimporterfactory_p.h"
#include "qdemonassetimporterplugin_p.h"
#include "qdemonassetimporter_p.h"

#include <QtCore/qdebug.h>
#include <QtCore/private/qfactoryloader_p.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader, (QDemonAssetImporterFactoryInterface_iid, QLatin1String("/assetimporters"), Qt::CaseInsensitive))
#if QT_CONFIG(library)
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, directLoader, (QDemonAssetImporterFactoryInterface_iid, QLatin1String(""), Qt::CaseInsensitive))
#endif

QStringList QDemonAssetImporterFactory::keys(const QString &pluginPath)
{
    QStringList list;
    if (!pluginPath.isEmpty()) {
#if QT_CONFIG(library)
        QCoreApplication::addLibraryPath(pluginPath);
        list = directLoader()->keyMap().values();
        if (!list.isEmpty()) {
            const QString postFix = QStringLiteral(" (from ") + QDir::toNativeSeparators(pluginPath) + QLatin1Char(')');
            const QStringList::iterator end = list.end();
            for (QStringList::iterator it = list.begin(); it != end; ++it)
                (*it).append(postFix);
        }
#else
        qWarning("Cannot query QDemonAssetImporter plugins at %s: Library loading is disabled.",
                 pluginPath.toLocal8Bit().constData());
#endif
    }
    list.append(loader()->keyMap().values());
    return list;
}

QDemonAssetImporter *QDemonAssetImporterFactory::create(const QString &name, const QStringList &args, const QString &pluginPath)
{
    if (!pluginPath.isEmpty()) {
#if QT_CONFIG(library)
        QCoreApplication::addLibraryPath(pluginPath);
        if (QDemonAssetImporter *ret = qLoadPlugin<QDemonAssetImporter, QDemonAssetImporterPlugin>(directLoader(), name, args))
            return ret;
#else
        qWarning("Cannot load QDemonAssetImporter plugin from %s. Library loading is disabled.",
                 pluginPath.toLocal8Bit().constData());
#endif
    }
    if (QDemonAssetImporter *ret = qLoadPlugin<QDemonAssetImporter, QDemonAssetImporterPlugin>(loader(), name, args))
        return ret;
    return nullptr;
}

QT_END_NAMESPACE
