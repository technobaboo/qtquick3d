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
#ifndef QDEMON_RENDER_PLUGIN_PROPERTY_VALUE_H
#define QDEMON_RENDER_PLUGIN_PROPERTY_VALUE_H
#include <Qt3DSRender.h>
#include <StringTable.h>
#include <Qt3DSDiscriminatedUnion.h>
#include <Qt3DSRenderPlugin.h>

namespace qt3ds {
namespace foundation {

    template <>
    struct DestructTraits<CRegisteredString>
    {
        void destruct(CRegisteredString &) {}
    };
}
}

namespace qt3ds {
namespace render {

    template <typename TDatatype>
    struct SRenderPluginPropertyValueTypeMap
    {
    };

    template <>
    struct SRenderPluginPropertyValueTypeMap<qint32>
    {
        enum { TypeMap = RenderPluginPropertyValueTypes::Long };
    };
    template <>
    struct SRenderPluginPropertyValueTypeMap<float>
    {
        enum { TypeMap = RenderPluginPropertyValueTypes::Float };
    };
    template <>
    struct SRenderPluginPropertyValueTypeMap<CRegisteredString>
    {
        enum { TypeMap = RenderPluginPropertyValueTypes::String };
    };
    template <>
    struct SRenderPluginPropertyValueTypeMap<bool>
    {
        enum { TypeMap = RenderPluginPropertyValueTypes::Boolean };
    };

    struct SRenderPluginPropertyValueUnionTraits
    {
        typedef RenderPluginPropertyValueTypes::Enum TIdType;
        enum {
            TBufferSize = sizeof(CRegisteredString),
        };

        static TIdType getNoDataId()
        {
            return RenderPluginPropertyValueTypes::NoRenderPluginPropertyValue;
        }

        template <typename TDataType>
        static TIdType getType()
        {
            return (TIdType)SRenderPluginPropertyValueTypeMap<TDataType>::TypeMap;
        }

        template <typename TRetType, typename TVisitorType>
        static TRetType visit(char *inData, TIdType inType, TVisitorType inVisitor)
        {
            switch (inType) {
            case RenderPluginPropertyValueTypes::String:
                return inVisitor(*NVUnionCast<CRegisteredString *>(inData));
            case RenderPluginPropertyValueTypes::Float:
                return inVisitor(*NVUnionCast<float *>(inData));
            case RenderPluginPropertyValueTypes::Long:
                return inVisitor(*NVUnionCast<qint32 *>(inData));
            default:
                Q_ASSERT(false);
            case RenderPluginPropertyValueTypes::NoRenderPluginPropertyValue:
                return inVisitor();
            }
        }

        template <typename TRetType, typename TVisitorType>
        static TRetType visit(const char *inData, TIdType inType, TVisitorType inVisitor)
        {
            switch (inType) {
            case RenderPluginPropertyValueTypes::String:
                return inVisitor(*NVUnionCast<const CRegisteredString *>(inData));
            case RenderPluginPropertyValueTypes::Float:
                return inVisitor(*NVUnionCast<const float *>(inData));
            case RenderPluginPropertyValueTypes::Long:
                return inVisitor(*NVUnionCast<const qint32 *>(inData));
            default:
                Q_ASSERT(false);
            case RenderPluginPropertyValueTypes::NoRenderPluginPropertyValue:
                return inVisitor();
            }
        }
    };

    typedef 
        DiscriminatedUnion<
                               DiscriminatedUnionGenericBase<SRenderPluginPropertyValueUnionTraits,
                                                             SRenderPluginPropertyValueUnionTraits::
                                                                 TBufferSize>,
                           SRenderPluginPropertyValueUnionTraits::TBufferSize>
            TRenderPluginPropertyValueUnionType;

    struct SRenderPluginPropertyValue : public TRenderPluginPropertyValueUnionType
    {
        typedef TRenderPluginPropertyValueUnionType TBase;
        SRenderPluginPropertyValue() {}
        SRenderPluginPropertyValue(const CRegisteredString &str)
            : TBase(str)
        {
        }
        SRenderPluginPropertyValue(qint32 value)
            : TBase(value)
        {
        }
        SRenderPluginPropertyValue(float value)
            : TBase(value)
        {
        }
        SRenderPluginPropertyValue(const SRenderPluginPropertyValue &other)
            : TBase(static_cast<const TBase &>(other))
        {
        }
        SRenderPluginPropertyValue &operator=(const SRenderPluginPropertyValue &other)
        {
            TBase::operator=(other);
            return *this;
        }
    };

    struct SRenderPropertyValueUpdate
    {
        // Should be the componentized name, so colors get .r .g .b appended
        // and vectors get .x .y etc. This interface only updates a component at a time.
        CRegisteredString m_PropertyName;
        SRenderPluginPropertyValue m_Value;
        SRenderPropertyValueUpdate() {}
        SRenderPropertyValueUpdate(CRegisteredString str, const SRenderPluginPropertyValue &v)
            : m_PropertyName(str)
            , m_Value(v)
        {
        }
    };
}
}

#endif