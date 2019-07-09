/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Quick 3D.
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

#ifndef QDEMONOBJECT_H
#define QDEMONOBJECT_H

#include <QtQuick3d/qtquick3dglobal.h>
#include <QtCore/QObject>
#include <QtDemonRuntimeRender/qdemonrendergraphobject.h>
#include <QtQml/qqml.h>
#include <QtQml/qqmlcomponent.h>

QT_BEGIN_NAMESPACE

class QDemonObjectPrivate;
class QDemonSceneManager;

class Q_QUICK3D_EXPORT QDemonObject : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_DECLARE_PRIVATE(QDemonObject)
    Q_DISABLE_COPY(QDemonObject)

    Q_PROPERTY(QDemonObject *parent READ parentItem WRITE setParentItem NOTIFY parentChanged DESIGNABLE false FINAL)
    Q_PRIVATE_PROPERTY(QDemonObject::d_func(), QQmlListProperty<QObject> data READ data DESIGNABLE false)
    Q_PRIVATE_PROPERTY(QDemonObject::d_func(), QQmlListProperty<QObject> resources READ resources DESIGNABLE false)
    Q_PRIVATE_PROPERTY(QDemonObject::d_func(),
                       QQmlListProperty<QDemonObject> children READ children NOTIFY childrenChanged DESIGNABLE false)

    Q_PROPERTY(QByteArray id READ id CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QDemonObject::Type type READ type CONSTANT)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged FINAL)

    Q_PRIVATE_PROPERTY(QDemonObject::d_func(), QQmlListProperty<QQuickState> states READ states DESIGNABLE false)
    Q_PRIVATE_PROPERTY(QDemonObject::d_func(), QQmlListProperty<QQuickTransition> transitions READ transitions DESIGNABLE false)
    Q_PROPERTY(QString state READ state WRITE setState NOTIFY stateChanged)
    Q_PRIVATE_PROPERTY(QDemonObject::d_func(),
                       QQmlListProperty<QDemonObject> visibleChildren READ visibleChildren NOTIFY visibleChildrenChanged DESIGNABLE false)

    Q_CLASSINFO("DefaultProperty", "data")
    Q_CLASSINFO("qt_QmlJSWrapperFactoryMethod", "_q_createJSWrapper(QV4::ExecutionEngine*)")
public:
    enum Type {
        Unknown = 0,
        SceneEnvironment,
        Node,
        Layer,
        Light,
        Camera,
        Model,
        DefaultMaterial,
        Image,
        Text,
        Effect,
        CustomMaterial,
        RenderPlugin,
        ReferencedMaterial,
        Path,
        PathSubPath,
        Lightmaps,
        LastKnownGraphObjectType,
    };
    Q_ENUM(Type)

    enum ItemChange {
        ItemChildAddedChange, // value.item
        ItemChildRemovedChange, // value.item
        ItemSceneChange, // value.window
        ItemVisibleHasChanged, // value.boolValue
        ItemParentHasChanged, // value.item
        ItemOpacityHasChanged, // value.realValue
        ItemActiveFocusHasChanged, // value.boolValue
        ItemRotationHasChanged, // value.realValue
        ItemAntialiasingHasChanged, // value.boolValue
        ItemDevicePixelRatioHasChanged, // value.realValue
        ItemEnabledHasChanged // value.boolValue
    };

    union ItemChangeData {
        ItemChangeData(QDemonObject *v) : item(v) {}
        ItemChangeData(QDemonSceneManager *v) : sceneRenderer(v) {}
        ItemChangeData(qreal v) : realValue(v) {}
        ItemChangeData(bool v) : boolValue(v) {}

        QDemonObject *item;
        QDemonSceneManager *sceneRenderer;
        qreal realValue;
        bool boolValue;
    };

    explicit QDemonObject(QDemonObject *parent = nullptr);
    ~QDemonObject() override;

    QByteArray id() const;
    QString name() const;
    virtual QDemonObject::Type type() const = 0;

    QString state() const;
    void setState(const QString &state);

    QList<QDemonObject *> childItems() const;

    QDemonSceneManager *sceneRenderer() const;
    QDemonObject *parentItem() const;

    bool isEnabled() const;
    bool isVisible() const;

public Q_SLOTS:
    void setName(QString name);
    void update();

    void setParentItem(QDemonObject *parentItem);
    void setEnabled(bool enabled);
    void setVisible(bool visible);

Q_SIGNALS:
    void sceneRendererChanged(QDemonSceneManager *sceneRenderer);
    void parentChanged(QDemonObject *parent);
    void enabledChanged(bool enabled);
    void childrenChanged();
    void stateChanged(const QString &);
    void visibleChildrenChanged();

    void visibleChanged(bool visible);

protected:
    virtual QDemonRenderGraphObject *updateSpatialNode(QDemonRenderGraphObject *node) = 0;
    virtual void itemChange(ItemChange, const ItemChangeData &);
    QDemonObject(QDemonObjectPrivate &dd, QDemonObject *parent = nullptr);

    void classBegin() override;
    void componentComplete() override;

private:
    Q_PRIVATE_SLOT(d_func(), void _q_resourceObjectDeleted(QObject *))
    Q_PRIVATE_SLOT(d_func(), quint64 _q_createJSWrapper(QV4::ExecutionEngine *))

    QByteArray m_id;
    QString m_name;
    bool m_enabled;
    bool m_visible;
    friend QDemonSceneManager;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDemonObject)

#endif // QDEMONOBJECT_H
