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
#pragma once
#ifndef QDEMON_RENDER_DYNAMIC_OBJECT_H
#define QDEMON_RENDER_DYNAMIC_OBJECT_H
#include <qdemonrender.h>
#include <QtDemonRuntimeRender/qdemonrendergraphobject.h>
#include <QtDemonRuntimeRender/qdemonrendernode.h>

QT_BEGIN_NAMESPACE

namespace dynamic {
struct SPropertyDefinition;
}

// Dynamic objects are objects that have variable number of properties during runtime.
struct SDynamicObject : public SGraphObject
{
    CRegisteredString m_ClassName;
    NodeFlags m_Flags;
    quint32 m_DataSectionByteSize;
    quint32 m_ThisObjectSize;

    SDynamicObject(GraphObjectTypes::Enum inType, CRegisteredString inClassName,
                   quint32 inDSByteSize, quint32 thisObjSize);

    quint8 *GetDataSectionBegin()
    {
        quint8 *thisObjectStart = reinterpret_cast<quint8 *>(this);
        quint8 *retval = thisObjectStart + m_ThisObjectSize;
        Q_ASSERT((reinterpret_cast<size_t>(retval) % 4 == 0));
        return retval;
    }

    const quint8 *GetDataSectionBegin() const
    {
        return const_cast<SDynamicObject *>(this)->GetDataSectionBegin();
    }

    quint8 *GetDataSectionEnd() { return GetDataSectionBegin() + m_DataSectionByteSize; }

    template <typename TDataType>
    void SetPropertyValueT(const dynamic::SPropertyDefinition &inDefinition,
                           const TDataType &inType);
    template <typename TStrType>
    void SetStrPropertyValueT(dynamic::SPropertyDefinition &inDefinition,
                              const char8_t *inValue, const char8_t *inProjectDir,
                              TStrType &ioWorkspace, IStringTable &inStrTable);

    void SetPropertyValue(const dynamic::SPropertyDefinition &inDefinition, bool inValue);
    void SetPropertyValue(const dynamic::SPropertyDefinition &inDefinition, float inValue);
    void SetPropertyValue(const dynamic::SPropertyDefinition &inDefinition, float inValue,
                          quint32 inOffset);
    void SetPropertyValue(const dynamic::SPropertyDefinition &inDefinition,
                          const QVector2D &inValue);
    void SetPropertyValue(const dynamic::SPropertyDefinition &inDefinition,
                          const QVector3D &inValue);
    void SetPropertyValue(const dynamic::SPropertyDefinition &inDefinition,
                          const QVector4D &inValue);
    void SetPropertyValue(const dynamic::SPropertyDefinition &inDefinition, qint32 inValue);
    void SetPropertyValue(const dynamic::SPropertyDefinition &inDefinition,
                          CRegisteredString inValue);

    void SetPropertyValue(const dynamic::SPropertyDefinition &inDefinition,
                          const char8_t *inValue, const char8_t *inProjectDir,
                          CRenderString &ioWorkspace, IStringTable &inStrTable);

    void SetPropertyValue(const dynamic::SPropertyDefinition &inDefinition,
                          const char8_t *inValue, const char8_t *inProjectDir,
                          QString &ioWorkspace, IStringTable &inStrTable);

    // Generic method used during serialization
    // to remap string and object pointers
    template <typename TRemapperType>
    void Remap(TRemapperType &inRemapper)
    {
        SGraphObject::Remap(inRemapper);
        inRemapper.Remap(m_ClassName);
    }
};

QT_END_NAMESPACE
#endif
