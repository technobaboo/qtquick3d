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
#include <QtDemonRuntimeRender/qdemonrender.h>
#include <QtDemonRuntimeRender/qdemonrenderdynamicobject.h>
#include <QtDemonRuntimeRender/qdemonrenderdynamicobjectsystem.h>
#include <QtDemonRuntimeRender/qdemonrenderstring.h>

#include <QtCore/qdir.h>

QT_BEGIN_NAMESPACE

SDynamicObject::SDynamicObject(GraphObjectTypes::Enum inType, const QString &inObjName,
                               quint32 inDSByteSize, quint32 thisObjSize)
    : SGraphObject(inType)
    , m_ClassName(inObjName)
    , m_DataSectionByteSize(inDSByteSize)
    , m_ThisObjectSize(thisObjSize)
{
}

template <typename TDataType>
void SDynamicObject::SetPropertyValueT(const dynamic::SPropertyDefinition &inDefinition,
                                       const TDataType &inValue)
{
    if (sizeof(inValue) != inDefinition.m_ByteSize) {
        Q_ASSERT(false);
        return;
    }
    ::memcpy(GetDataSectionBegin() + inDefinition.m_Offset, &inValue, sizeof(inValue));
}

void SDynamicObject::SetPropertyValue(const dynamic::SPropertyDefinition &inDefinition,
                                      bool inValue)
{
    SetPropertyValueT(inDefinition, inValue);
}

void SDynamicObject::SetPropertyValue(const dynamic::SPropertyDefinition &inDefinition,
                                      float inValue)
{
    SetPropertyValueT(inDefinition, inValue);
}
void SDynamicObject::SetPropertyValue(const dynamic::SPropertyDefinition &inDefinition,
                                      float inValue, quint32 inOffset)
{
    if (sizeof(float) > (inDefinition.m_ByteSize - inOffset)) {
        Q_ASSERT(false);
        return;
    }
    ::memcpy(GetDataSectionBegin() + inDefinition.m_Offset + inOffset, &inValue, sizeof(inValue));
}
void SDynamicObject::SetPropertyValue(const dynamic::SPropertyDefinition &inDefinition,
                                      const QVector2D &inValue)
{
    SetPropertyValueT(inDefinition, inValue);
}
void SDynamicObject::SetPropertyValue(const dynamic::SPropertyDefinition &inDefinition,
                                      const QVector3D &inValue)
{
    SetPropertyValueT(inDefinition, inValue);
}
void SDynamicObject::SetPropertyValue(const dynamic::SPropertyDefinition &inDefinition,
                                      const QVector4D &inValue)
{
    SetPropertyValueT(inDefinition, inValue);
}
void SDynamicObject::SetPropertyValue(const dynamic::SPropertyDefinition &inDefinition,
                                      qint32 inValue)
{
    SetPropertyValueT(inDefinition, inValue);
}
void SDynamicObject::SetPropertyValue(const dynamic::SPropertyDefinition &inDefinition,
                                      const QString &inValue)
{
    Q_ASSERT(inDefinition.m_DataType == QDemonRenderShaderDataTypes::Texture2D);
    SetPropertyValueT(inDefinition, inValue);
}
template <typename TStrType>
void SDynamicObject::SetStrPropertyValueT(dynamic::SPropertyDefinition &inDefinition,
                                          const char8_t *inValue, const char8_t *inProjectDir,
                                          TStrType &ioWorkspace, IStringTable &inStrTable)
{
    if (inValue == nullptr)
        inValue = "";
    if (inDefinition.m_DataType == QDemonRenderShaderDataTypes::Integer) {
        QDemonConstDataRef<QString> theEnumValues = inDefinition.m_EnumValueNames;
        for (qint32 idx = 0, end = (qint32)theEnumValues.size(); idx < end; ++idx) {
            if (strcmp(theEnumValues[idx].c_str(), inValue) == 0) {
                SetPropertyValueT(inDefinition, idx);
                break;
            }
        }
    } else if (inDefinition.m_DataType == QDemonRenderShaderDataTypes::QDemonRenderTexture2DPtr) {
        if (inProjectDir == nullptr)
            inProjectDir = "";
        if (CFileTools::RequiresCombineBaseAndRelative(inValue)) {
            QString absolute = QDir(inProjectDir).filePath(inValue);
            ioWorkspace.assign(absolute.toLatin1().constData());
            SetPropertyValueT(inDefinition, inStrTable.RegisterStr(ioWorkspace.c_str()));
            // We also adjust the image path in the definition
            // I could not find a better place
            inDefinition.m_ImagePath = inStrTable.RegisterStr(ioWorkspace.c_str());
        } else {
            SetPropertyValueT(inDefinition, inStrTable.RegisterStr(inValue));
        }
    } else if (inDefinition.m_DataType == QDemonRenderShaderDataTypes::QDemonRenderImage2DPtr) {
        SetPropertyValueT(inDefinition, inStrTable.RegisterStr(inValue));
    } else if (inDefinition.m_DataType == QDemonRenderShaderDataTypes::QDemonRenderDataBufferPtr) {
        SetPropertyValueT(inDefinition, inStrTable.RegisterStr(inValue));
    } else {
        Q_ASSERT(false);
    }
}

void SDynamicObject::SetPropertyValue(const dynamic::SPropertyDefinition &inDefinition,
                                      const char8_t *inValue, const char8_t *inProjectDir,
                                      CRenderString &ioWorkspace, IStringTable &inStrTable)
{
    SetStrPropertyValueT(const_cast<dynamic::SPropertyDefinition &>(inDefinition), inValue,
                         inProjectDir, ioWorkspace, inStrTable);
}

void SDynamicObject::SetPropertyValue(const dynamic::SPropertyDefinition &inDefinition,
                                      const char8_t *inValue, const char8_t *inProjectDir,
                                      QString &ioWorkspace, IStringTable &inStrTable)
{
    SetStrPropertyValueT(const_cast<dynamic::SPropertyDefinition &>(inDefinition), inValue,
                         inProjectDir, ioWorkspace, inStrTable);
}

QT_END_NAMESPACE
