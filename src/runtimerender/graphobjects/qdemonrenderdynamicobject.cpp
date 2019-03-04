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

#include "qdemonrenderdynamicobject.h"

#include <QtDemonRuntimeRender/qdemonrenderdynamicobjectsystem.h>

#include <QtCore/qdir.h>

QT_BEGIN_NAMESPACE

QDemonDynamicObject::QDemonDynamicObject(QDemonGraphObjectTypes::Enum inType, const QString &inObjName,
                               quint32 inDSByteSize, quint32 thisObjSize)
    : QDemonGraphObject(inType)
    , className(inObjName)
    , dataSectionByteSize(inDSByteSize)
    , thisObjectSize(thisObjSize)
{
}

template <typename TDataType>
void QDemonDynamicObject::setPropertyValueT(const dynamic::QDemonPropertyDefinition &inDefinition,
                                       const TDataType &inValue)
{
    if (sizeof(inValue) != inDefinition.byteSize) {
        Q_ASSERT(false);
        return;
    }
    ::memcpy(getDataSectionBegin() + inDefinition.offset, &inValue, sizeof(inValue));
}

void QDemonDynamicObject::setPropertyValue(const dynamic::QDemonPropertyDefinition &inDefinition,
                                           bool inValue)
{
    setPropertyValueT(inDefinition, inValue);
}

void QDemonDynamicObject::setPropertyValue(const dynamic::QDemonPropertyDefinition &inDefinition,
                                      float inValue)
{
    setPropertyValueT(inDefinition, inValue);
}
void QDemonDynamicObject::setPropertyValue(const dynamic::QDemonPropertyDefinition &inDefinition,
                                      float inValue, quint32 inOffset)
{
    if (sizeof(float) > (inDefinition.byteSize - inOffset)) {
        Q_ASSERT(false);
        return;
    }
    ::memcpy(getDataSectionBegin() + inDefinition.offset + inOffset, &inValue, sizeof(inValue));
}
void QDemonDynamicObject::setPropertyValue(const dynamic::QDemonPropertyDefinition &inDefinition,
                                      const QVector2D &inValue)
{
    setPropertyValueT(inDefinition, inValue);
}
void QDemonDynamicObject::setPropertyValue(const dynamic::QDemonPropertyDefinition &inDefinition,
                                      const QVector3D &inValue)
{
    setPropertyValueT(inDefinition, inValue);
}
void QDemonDynamicObject::setPropertyValue(const dynamic::QDemonPropertyDefinition &inDefinition,
                                      const QVector4D &inValue)
{
    setPropertyValueT(inDefinition, inValue);
}
void QDemonDynamicObject::setPropertyValue(const dynamic::QDemonPropertyDefinition &inDefinition,
                                      qint32 inValue)
{
    setPropertyValueT(inDefinition, inValue);
}
void QDemonDynamicObject::setPropertyValue(const dynamic::QDemonPropertyDefinition &inDefinition,
                                      const QString &inValue)
{
    Q_ASSERT(inDefinition.dataType == QDemonRenderShaderDataTypes::Texture2D);
    setPropertyValueT(inDefinition, inValue);
}
template <typename TStrType>
void QDemonDynamicObject::setStrPropertyValueT(dynamic::QDemonPropertyDefinition &inDefinition,
                                               const char *inValue,
                                               const char *inProjectDir,
                                               TStrType &ioWorkspace)
{
    if (inValue == nullptr)
        inValue = "";
    if (inDefinition.dataType == QDemonRenderShaderDataTypes::Integer) {
        // TODO: Can the enum values be anything but 8bit chars?
        QDemonConstDataRef<QString> theEnumValues = inDefinition.enumValueNames;
        for (int idx = 0, end = theEnumValues.size(); idx < end; ++idx) {
            if (theEnumValues[idx].compare(QString::fromLocal8Bit(inValue)) == 0) {
                setPropertyValueT(inDefinition, idx);
                break;
            }
        }
    } else if (inDefinition.dataType == QDemonRenderShaderDataTypes::Texture2D) {
        if (inProjectDir == nullptr)
            inProjectDir = "";

        const bool RequiresCombineBaseAndRelative = inValue && (::strncmp(inValue, ".", 1) == 0);
        if (RequiresCombineBaseAndRelative) {
            const QString absolute = QDir(QString::fromLocal8Bit(inProjectDir)).filePath(QString::fromLocal8Bit(inValue));
            ioWorkspace = absolute;
            setPropertyValueT(inDefinition, ioWorkspace);
            // We also adjust the image path in the definition
            // I could not find a better place
            inDefinition.imagePath = ioWorkspace;
        } else {
            setPropertyValueT(inDefinition, inValue);
        }
    } else if (inDefinition.dataType == QDemonRenderShaderDataTypes::Image2D) {
        setPropertyValueT(inDefinition, inValue);
    } else if (inDefinition.dataType == QDemonRenderShaderDataTypes::DataBuffer) {
        setPropertyValueT(inDefinition, inValue);
    } else {
        Q_ASSERT(false);
    }
}

void QDemonDynamicObject::setPropertyValue(const dynamic::QDemonPropertyDefinition &inDefinition,
                                      const char *inValue, const char *inProjectDir,
                                      QString &ioWorkspace)
{
    setStrPropertyValueT(const_cast<dynamic::QDemonPropertyDefinition &>(inDefinition), inValue,
                         inProjectDir, ioWorkspace);
}

//void QDemonDynamicObject::SetPropertyValue(const dynamic::SPropertyDefinition &inDefinition,
//                                      const char *inValue, const char *inProjectDir,
//                                      QString &ioWorkspace)
//{
//    SetStrPropertyValueT(const_cast<dynamic::SPropertyDefinition &>(inDefinition), inValue,
//                         inProjectDir, ioWorkspace);
//}

QT_END_NAMESPACE
