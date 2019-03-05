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
#ifndef QDEMON_RENDER_DYNAMIC_OBJECT_H
#define QDEMON_RENDER_DYNAMIC_OBJECT_H

#include <QtDemonRuntimeRender/qdemonrendergraphobject.h>
#include <QtDemonRuntimeRender/qdemonrendernode.h>

#include <QtCore/QString>

QT_BEGIN_NAMESPACE

namespace dynamic {
struct QDemonPropertyDefinition;
}

// Dynamic objects are objects that have variable number of properties during runtime.
struct Q_DEMONRUNTIMERENDER_EXPORT QDemonDynamicObject : public QDemonGraphObject
{
    QString className;
    QDemonNodeFlags flags;
    quint32 dataSectionByteSize;
    quint32 thisObjectSize;

    QDemonDynamicObject(QDemonGraphObjectTypes::Enum inType, const QString &inClassName, quint32 inDSByteSize, quint32 thisObjSize);

    quint8 *getDataSectionBegin()
    {
        quint8 *thisObjectStart = reinterpret_cast<quint8 *>(this);
        quint8 *retval = thisObjectStart + thisObjectSize;
        Q_ASSERT((reinterpret_cast<size_t>(retval) % 4 == 0));
        return retval;
    }

    const quint8 *getDataSectionBegin() const { return const_cast<QDemonDynamicObject *>(this)->getDataSectionBegin(); }

    quint8 *getDataSectionEnd() { return getDataSectionBegin() + dataSectionByteSize; }

    template<typename TDataType>
    void setPropertyValueT(const dynamic::QDemonPropertyDefinition &inDefinition, const TDataType &inType);
    template<typename TStrType>
    void setStrPropertyValueT(dynamic::QDemonPropertyDefinition &inDefinition, const char *inValue, const char *inProjectDir, TStrType &ioWorkspace);

    void setPropertyValue(const dynamic::QDemonPropertyDefinition &inDefinition, bool inValue);
    void setPropertyValue(const dynamic::QDemonPropertyDefinition &inDefinition, float inValue);
    void setPropertyValue(const dynamic::QDemonPropertyDefinition &inDefinition, float inValue, quint32 inOffset);
    void setPropertyValue(const dynamic::QDemonPropertyDefinition &inDefinition, const QVector2D &inValue);
    void setPropertyValue(const dynamic::QDemonPropertyDefinition &inDefinition, const QVector3D &inValue);
    void setPropertyValue(const dynamic::QDemonPropertyDefinition &inDefinition, const QVector4D &inValue);
    void setPropertyValue(const dynamic::QDemonPropertyDefinition &inDefinition, qint32 inValue);
    void setPropertyValue(const dynamic::QDemonPropertyDefinition &inDefinition, const QString &inValue);

    void setPropertyValue(const dynamic::QDemonPropertyDefinition &inDefinition, const char *inValue, const char *inProjectDir, QString &ioWorkspace);

    // Generic method used during serialization
    // to remap string and object pointers
    template<typename TRemapperType>
    void remap(TRemapperType &inRemapper)
    {
        QDemonGraphObject::remap(inRemapper);
        inRemapper.remap(className);
    }
};

QT_END_NAMESPACE
#endif
