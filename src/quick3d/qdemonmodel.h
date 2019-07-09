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

#ifndef QDEMONMODEL_H
#define QDEMONMODEL_H

#include <QtQuick3d/qdemonnode.h>
#include <QtQml/QQmlListProperty>
#include <QtQuick3d/QDemonMaterial>
#include <QtCore/QVector>
#include <QtCore/QUrl>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QDemonModel : public QDemonNode
{
    Q_OBJECT
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(int skeletonRoot READ skeletonRoot WRITE setSkeletonRoot NOTIFY skeletonRootChanged)
    Q_PROPERTY(QDemonTessModeValues tesselationMode READ tesselationMode WRITE setTesselationMode NOTIFY tesselationModeChanged)
    Q_PROPERTY(float edgeTess READ edgeTess WRITE setEdgeTess NOTIFY edgeTessChanged)
    Q_PROPERTY(float innerTess READ innerTess WRITE setInnerTess NOTIFY innerTessChanged)
    Q_PROPERTY(bool isWireframeMode READ isWireframeMode WRITE setIsWireframeMode NOTIFY isWireframeModeChanged)
    Q_PROPERTY(QQmlListProperty<QDemonMaterial> materials READ materials)

public:
    enum QDemonTessModeValues {
        NoTess = 0,
        TessLinear = 1,
        TessPhong = 2,
        TessNPatch = 3,
    };
    Q_ENUM(QDemonTessModeValues)


    QDemonModel();
    ~QDemonModel() override;

    QDemonObject::Type type() const override;

    QUrl source() const;
    int skeletonRoot() const;
    QDemonTessModeValues tesselationMode() const;
    float edgeTess() const;
    float innerTess() const;
    bool isWireframeMode() const;

    QQmlListProperty<QDemonMaterial> materials();

public Q_SLOTS:
    void setSource(const QUrl &source);
    void setSkeletonRoot(int skeletonRoot);
    void setTesselationMode(QDemonTessModeValues tesselationMode);
    void setEdgeTess(float edgeTess);
    void setInnerTess(float innerTess);
    void setIsWireframeMode(bool isWireframeMode);

Q_SIGNALS:
    void sourceChanged(const QUrl &source);
    void skeletonRootChanged(int skeletonRoot);
    void tesselationModeChanged(QDemonTessModeValues tesselationMode);
    void edgeTessChanged(float edgeTess);
    void innerTessChanged(float innerTess);
    void isWireframeModeChanged(bool isWireframeMode);

protected:
    QDemonRenderGraphObject *updateSpatialNode(QDemonRenderGraphObject *node) override;

private:
    enum QDemonModelDirtyType {
        SourceDirty =           0x00000001,
        SkeletonRootDirty =     0x00000002,
        TesselationModeDirty =  0x00000004,
        TesselationEdgeDirty =  0x00000008,
        TesselationInnerDirty = 0x00000010,
        WireframeDirty =        0x00000020,
        MaterialsDirty =        0x00000040
    };

    QString translateSource();
    QUrl m_source;
    int m_skeletonRoot = -1;
    QDemonTessModeValues m_tesselationMode = QDemonTessModeValues::NoTess;
    float m_edgeTess = 1.0f;
    float m_innerTess = 1.0f;
    bool m_isWireframeMode = false;

    quint32 m_dirtyAttributes = 0xffffffff; // all dirty by default
    void markDirty(QDemonModelDirtyType type);

    static void qmlAppendMaterial(QQmlListProperty<QDemonMaterial> *list, QDemonMaterial *material);
    static QDemonMaterial *qmlMaterialAt(QQmlListProperty<QDemonMaterial> *list, int index);
    static int qmlMaterialsCount(QQmlListProperty<QDemonMaterial> *list);
    static void qmlClearMaterials(QQmlListProperty<QDemonMaterial> *list);

    QVector<QDemonMaterial *> m_materials;
};

QT_END_NAMESPACE

#endif // QDEMONMODEL_H
