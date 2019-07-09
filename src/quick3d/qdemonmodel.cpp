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

#include "qdemonmodel.h"
#include "qdemonobject_p.h"

#include <QtDemonRuntimeRender/QDemonRenderGraphObject>
#include <QtDemonRuntimeRender/QDemonRenderCustomMaterial>
#include <QtDemonRuntimeRender/qdemonrenderreferencedmaterial.h>
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterial.h>

#include <QtDemonRuntimeRender/qdemonrendermodel.h>
#include <QtQml/QQmlFile>

QT_BEGIN_NAMESPACE

/*!
   \qmltype DemonModel
   \inqmlmodule QtDemon
   \brief Lets you load a 3D model data
*/
QDemonModel::QDemonModel() {}

QDemonModel::~QDemonModel() {}

QDemonObject::Type QDemonModel::type() const
{
    return QDemonObject::Model;
}

QUrl QDemonModel::source() const
{
    return m_source;
}

int QDemonModel::skeletonRoot() const
{
    return m_skeletonRoot;
}

QDemonModel::QDemonTessModeValues QDemonModel::tesselationMode() const
{
    return m_tesselationMode;
}

float QDemonModel::edgeTess() const
{
    return m_edgeTess;
}

float QDemonModel::innerTess() const
{
    return m_innerTess;
}

bool QDemonModel::isWireframeMode() const
{
    return m_isWireframeMode;
}

QQmlListProperty<QDemonMaterial> QDemonModel::materials()
{
    return QQmlListProperty<QDemonMaterial>(this,
                                            nullptr,
                                            QDemonModel::qmlAppendMaterial,
                                            QDemonModel::qmlMaterialsCount,
                                            QDemonModel::qmlMaterialAt,
                                            QDemonModel::qmlClearMaterials);
}

void QDemonModel::setSource(const QUrl &source)
{
    if (m_source == source)
        return;

    m_source = source;
    emit sourceChanged(m_source);
    markDirty(SourceDirty);
}

void QDemonModel::setSkeletonRoot(int skeletonRoot)
{
    if (m_skeletonRoot == skeletonRoot)
        return;

    m_skeletonRoot = skeletonRoot;
    emit skeletonRootChanged(m_skeletonRoot);
    markDirty(SkeletonRootDirty);
}

void QDemonModel::setTesselationMode(QDemonModel::QDemonTessModeValues tesselationMode)
{
    if (m_tesselationMode == tesselationMode)
        return;

    m_tesselationMode = tesselationMode;
    emit tesselationModeChanged(m_tesselationMode);
    markDirty(TesselationModeDirty);
}

void QDemonModel::setEdgeTess(float edgeTess)
{
    if (qFuzzyCompare(m_edgeTess, edgeTess))
        return;

    m_edgeTess = edgeTess;
    emit edgeTessChanged(m_edgeTess);
    markDirty(TesselationEdgeDirty);
}

void QDemonModel::setInnerTess(float innerTess)
{
    if (qFuzzyCompare(m_innerTess, innerTess))
        return;

    m_innerTess = innerTess;
    emit innerTessChanged(m_innerTess);
    markDirty(TesselationInnerDirty);
}

void QDemonModel::setIsWireframeMode(bool isWireframeMode)
{
    if (m_isWireframeMode == isWireframeMode)
        return;

    m_isWireframeMode = isWireframeMode;
    emit isWireframeModeChanged(m_isWireframeMode);
    markDirty(WireframeDirty);
}

static QDemonRenderGraphObject *getMaterialNodeFromQDemonMaterial(QDemonMaterial *material)
{
    QDemonObjectPrivate *p = QDemonObjectPrivate::get(material);
    return p->spatialNode;
}

QDemonRenderGraphObject *QDemonModel::updateSpatialNode(QDemonRenderGraphObject *node)
{
    if (!node)
        node = new QDemonRenderModel();

    QDemonNode::updateSpatialNode(node);

    auto modelNode = static_cast<QDemonRenderModel *>(node);
    if (m_dirtyAttributes & SourceDirty)
        modelNode->meshPath = QDemonRenderMeshPath::create(translateSource());
    if (m_dirtyAttributes & SkeletonRootDirty)
        modelNode->skeletonRoot = m_skeletonRoot;
    if (m_dirtyAttributes & TesselationModeDirty)
        modelNode->tessellationMode = TessModeValues(m_tesselationMode);
    if (m_dirtyAttributes & TesselationEdgeDirty)
        modelNode->edgeTess = m_edgeTess;
    if (m_dirtyAttributes & TesselationInnerDirty)
        modelNode->innerTess = m_innerTess;
    if (m_dirtyAttributes & WireframeDirty)
        modelNode->wireframeMode = m_isWireframeMode;


    if (m_dirtyAttributes & MaterialsDirty) {
        if (!m_materials.isEmpty()) {
            if (modelNode->materials.isEmpty()) {
                // Easy mode, just add each material
                for (auto material : m_materials) {
                    QDemonRenderGraphObject *graphObject = getMaterialNodeFromQDemonMaterial(material);
                    if (graphObject)
                        modelNode->materials.append(graphObject);
                }
            } else {
                // Hard mode, go through each material and see if they match
                if (modelNode->materials.size() != m_materials.size())
                    modelNode->materials.resize(m_materials.size());
                for (int i = 0; i < m_materials.size(); ++i) {
                    QDemonRenderGraphObject *graphObject = getMaterialNodeFromQDemonMaterial(m_materials[i]);
                    if (modelNode->materials[i] != graphObject)
                        modelNode->materials[i] = graphObject;
                }
            }
        } else {
            // No materials
            modelNode->materials.clear();
        }
    }

    m_dirtyAttributes = 0;

    return modelNode;
}

// Source URL's need a bit of translation for the engine because of the
// use of fragment syntax for specifiying primitives and sub-meshes
// So we need to check for the fragment before translating to a qmlfile

QString QDemonModel::translateSource()
{
    QString fragment;
    if (m_source.hasFragment()) {
        // Check if this is an index, or primitive
        bool isNumber = false;
        m_source.fragment().toInt(&isNumber);
        fragment = QStringLiteral("#") + m_source.fragment();
        // If it wasn't an index, then it was a primitive
        if (!isNumber)
            return fragment;
    }

    return QQmlFile::urlToLocalFileOrQrc(m_source) + fragment;
}

void QDemonModel::markDirty(QDemonModel::QDemonModelDirtyType type)
{
    if (!(m_dirtyAttributes & quint32(type))) {
        m_dirtyAttributes |= quint32(type);
        update();
    }
}

void QDemonModel::qmlAppendMaterial(QQmlListProperty<QDemonMaterial> *list, QDemonMaterial *material)
{
    if (material == nullptr)
        return;
    QDemonModel *self = static_cast<QDemonModel *>(list->object);
    self->m_materials.push_back(material);
    self->markDirty(QDemonModel::MaterialsDirty);

    if(material->parentItem() == nullptr)
        material->setParentItem(self);
}

QDemonMaterial *QDemonModel::qmlMaterialAt(QQmlListProperty<QDemonMaterial> *list, int index)
{
    QDemonModel *self = static_cast<QDemonModel *>(list->object);
    return self->m_materials.at(index);
}

int QDemonModel::qmlMaterialsCount(QQmlListProperty<QDemonMaterial> *list)
{
    QDemonModel *self = static_cast<QDemonModel *>(list->object);
    return self->m_materials.count();
}

void QDemonModel::qmlClearMaterials(QQmlListProperty<QDemonMaterial> *list)
{
    QDemonModel *self = static_cast<QDemonModel *>(list->object);
    self->m_materials.clear();
    self->markDirty(QDemonModel::MaterialsDirty);
}

QT_END_NAMESPACE
