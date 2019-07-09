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

#ifndef QDEMONMATERIAL_H
#define QDEMONMATERIAL_H

#include <QtQuick3d/qdemonobject.h>
#include <QtQuick3d/qdemonimage.h>

#include <QtCore/qvector.h>

QT_BEGIN_NAMESPACE

class QDemonSceneManager;
class Q_QUICK3D_EXPORT QDemonMaterial : public QDemonObject
{
    Q_OBJECT
    Q_PROPERTY(QDemonImage *lightmapIndirect READ lightmapIndirect WRITE setLightmapIndirect NOTIFY lightmapIndirectChanged)
    Q_PROPERTY(QDemonImage *lightmapRadiosity READ lightmapRadiosity WRITE setLightmapRadiosity NOTIFY lightmapRadiosityChanged)
    Q_PROPERTY(QDemonImage *lightmapShadow READ lightmapShadow WRITE setLightmapShadow NOTIFY lightmapShadowChanged)
    Q_PROPERTY(QDemonImage *iblProbe READ iblProbe WRITE setIblProbe NOTIFY iblProbeChanged)

    Q_PROPERTY(QDemonImage *emissiveMap2 READ emissiveMap2 WRITE setEmissiveMap2 NOTIFY emissiveMap2Changed)

    Q_PROPERTY(QDemonImage *displacementMap READ displacementMap WRITE setDisplacementMap NOTIFY displacementMapChanged)
    Q_PROPERTY(float displacementAmount READ displacementAmount WRITE setDisplacementAmount NOTIFY displacementAmountChanged)

public:
    QDemonMaterial();
    ~QDemonMaterial() override;

    QDemonObject::Type type() const override = 0;

    QDemonImage *lightmapIndirect() const;
    QDemonImage *lightmapRadiosity() const;
    QDemonImage *lightmapShadow() const;
    QDemonImage *iblProbe() const;

    QDemonImage *emissiveMap2() const;

    QDemonImage *displacementMap() const;
    float displacementAmount() const;

public Q_SLOTS:
    void setLightmapIndirect(QDemonImage *lightmapIndirect);
    void setLightmapRadiosity(QDemonImage *lightmapRadiosity);
    void setLightmapShadow(QDemonImage *lightmapShadow);
    void setIblProbe(QDemonImage *iblProbe);

    void setEmissiveMap2(QDemonImage *emissiveMap2);

    void setDisplacementMap(QDemonImage *displacementMap);
    void setDisplacementAmount(float displacementAmount);

Q_SIGNALS:
    void lightmapIndirectChanged(QDemonImage *lightmapIndirect);
    void lightmapRadiosityChanged(QDemonImage *lightmapRadiosity);
    void lightmapShadowChanged(QDemonImage *lightmapShadow);
    void iblProbeChanged(QDemonImage *iblProbe);

    void emissiveMap2Changed(QDemonImage *emissiveMap2);

    void displacementMapChanged(QDemonImage *displacementMap);
    void displacementAmountChanged(float displacementAmount);

protected:
    QDemonRenderGraphObject *updateSpatialNode(QDemonRenderGraphObject *node) override;
    void itemChange(ItemChange, const ItemChangeData &) override;
public:
    void setDynamicTextureMap(QDemonImage *textureMap);
private:
    void updateSceneRenderer(QDemonSceneManager *sceneRenderer);
    QDemonImage *m_lightmapIndirect = nullptr;
    QDemonImage *m_lightmapRadiosity = nullptr;
    QDemonImage *m_lightmapShadow = nullptr;
    QDemonImage *m_iblProbe = nullptr;

    QDemonImage *m_emissiveMap2 = nullptr;

    QDemonImage *m_displacementMap = nullptr;
    float m_displacementAmount = 0.0f;

    QHash<QObject*, QMetaObject::Connection> m_connections;
    QVector<QDemonImage *> m_dynamicTextureMaps;
};

QT_END_NAMESPACE

#endif // QDEMONMATERIAL_H
