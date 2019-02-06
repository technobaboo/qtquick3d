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

static void initResources()
{
#ifdef QT_STATIC
    Q_INIT_RESOURCE(qmake_QtDemon);
#endif
}

QT_BEGIN_NAMESPACE

class Q3DSStudio3DPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    Q3DSStudio3DPlugin(QObject *parent = 0) : QQmlExtensionPlugin(parent) { initResources(); }
    void registerTypes(const char *uri) override
    {
//        qmlRegisterType<Q3DSStudio3DItem>(uri, 2, 0, "Studio3D");
//        qmlRegisterType<Q3DSPresentationItem>(uri, 2, 0, "Presentation");
//        qmlRegisterType<Q3DSSubPresentationSettings>(uri, 2, 0, "SubPresentationSettings");
//        qmlRegisterType<Q3DSViewerSettings>(uri, 2, 0, "ViewerSettings");

//        qmlRegisterType<Q3DSDataInput>(uri, 2, 0, "DataInput");
//        qmlRegisterType<Q3DSElement>(uri, 2, 0, "Element");
//        qmlRegisterType<Q3DSSceneElement>(uri, 2, 0, "SceneElement");
//        qmlRegisterType<Q3DSViewerSettings>(uri, 2, 0, "ViewerSettings");
//        qmlRegisterType<Q3DSInlineQmlSubPresentation>(uri, 2, 0, "QmlStream");

//        qmlRegisterType<Q3DSStudio3DView>(uri, 2, 2, "View3D");
//#if QT_CONFIG(q3ds_profileui)
//        qmlRegisterType<Q3DSImGuiItem>(uri, 2, 2, "Studio3DProfiler");
//#endif

//        // Stereo API added in 2.3
//        qmlRegisterType<Q3DSViewerSettings, 3>(uri, 2, 3, "ViewerSettings");

//        // API NG
//        qmlRegisterType<Q3DSStudio3DEngine>(uri, 2, 1, "Studio3DEngine");
//        qmlRegisterType<Q3DSLayer3D>(uri, 2, 1, "Layer3D");
//        qmlRegisterUncreatableType<Q3DSObject3D>(uri, 2, 1, "Object3D", QLatin1String("Object3D is a base class"));
//        qmlRegisterUncreatableType<Q3DSNode3D>(uri, 2, 1, "Node3D", QLatin1String("Node3D is a base class"));
//        qmlRegisterType<Q3DSGroup3D>(uri, 2, 1, "Group3D");
//        qmlRegisterType<Q3DSModel3D>(uri, 2, 1, "Model3D");
//        qmlRegisterType<Q3DSCamera3D>(uri, 2, 1, "Camera3D");
//        qmlRegisterType<Q3DSLight3D>(uri, 2, 1, "Light3D");
    }
};

QT_END_NAMESPACE

#include "plugin.moc"
