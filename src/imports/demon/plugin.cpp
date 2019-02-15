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

#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqml.h>

#include <QtQuick3d/qdemoncamera.h>
#include <QtQuick3d/qdemoncustommaterial.h>
#include <QtQuick3d/qdemondefaultmaterial.h>
#include <QtQuick3d/qdemoneffect.h>
#include <QtQuick3d/qdemonimage.h>
#include <QtQuick3d/qdemonlayer.h>
#include <QtQuick3d/qdemonlight.h>
#include <QtQuick3d/qdemonmaterial.h>
#include <QtQuick3d/qdemonmodel.h>
#include <QtQuick3d/qdemonnode.h>
#include <QtQuick3d/qdemonobject.h>
#include <QtQuick3d/qdemonscene.h>
#include <QtQuick3d/qdemonwindow.h>

static void initResources()
{
#ifdef QT_STATIC
    Q_INIT_RESOURCE(qmake_QtDemon);
#endif
}

QT_BEGIN_NAMESPACE

class QDemonPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    QDemonPlugin(QObject *parent = 0) : QQmlExtensionPlugin(parent) { initResources(); }
    void registerTypes(const char *uri) override
    {
        qmlRegisterType<QDemonCamera>(uri, 1, 0, "DemonCamera");
        qmlRegisterType<QDemonCustomMaterial>(uri, 1, 0, "DemonCustomMaterial");
        qmlRegisterType<QDemonDefaultMaterial>(uri, 1, 0, "DemonDefaultMaterial");
        qmlRegisterType<QDemonEffect>(uri, 1, 0, "DemonEffect");
        qmlRegisterType<QDemonImage>(uri, 1, 0, "DemonImage");
        qmlRegisterType<QDemonLayer>(uri, 1, 0, "DemonLayer");
        qmlRegisterType<QDemonLight>(uri, 1, 0, "DemonLight");
        qmlRegisterUncreatableType<QDemonMaterial>(uri, 1, 0, "DemonMaterial", QLatin1String("Material is Abstract"));
        qmlRegisterType<QDemonModel>(uri, 1, 0, "DemonModel");
        qmlRegisterType<QDemonNode>(uri, 1, 0, "DemonNode");
        qmlRegisterUncreatableType<QDemonObject>(uri, 1, 0, "DemonObject", QLatin1String("Object is Abtract"));
        qmlRegisterType<QDemonScene>(uri, 1, 0, "DemonScene");
        qmlRegisterType<QDemonWindow>(uri, 1, 0, "DemonWindow");
    }
};

QT_END_NAMESPACE

#include "plugin.moc"
