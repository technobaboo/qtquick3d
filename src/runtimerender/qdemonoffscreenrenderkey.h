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
#ifndef QDEMON_OFFSCREEN_RENDER_KEY_H
#define QDEMON_OFFSCREEN_RENDER_KEY_H

#include <QtDemon/qdemondiscriminatedunion.h>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

template<>
struct DestructTraits<QString>
{
    void destruct(QString &) {}
};

struct OffscreenRendererKeyTypes
{
    enum Enum {
        NoOffscreenRendererKey = 0,
        RegisteredString,
        VoidPtr,
    };
};

template<typename TDType>
struct QDemonOffscreenRendererKeyTypeMap
{
};
template<>
struct QDemonOffscreenRendererKeyTypeMap<QString>
{
    enum { KeyType = OffscreenRendererKeyTypes::RegisteredString };
};
template<>
struct QDemonOffscreenRendererKeyTypeMap<void *>
{
    enum { KeyType = OffscreenRendererKeyTypes::VoidPtr };
};

struct QDemonOffscreenRendererKeyUnionTraits
{
    typedef OffscreenRendererKeyTypes::Enum TIdType;
    enum {
        TBufferSize = sizeof(QString),
    };

    static TIdType getNoDataId() { return OffscreenRendererKeyTypes::NoOffscreenRendererKey; }

    template<typename TDataType>
    static TIdType getType()
    {
        return static_cast<TIdType>(QDemonOffscreenRendererKeyTypeMap<TDataType>::KeyType);
    }

    template<typename TRetType, typename TVisitorType>
    static TRetType visit(char *inData, TIdType inType, TVisitorType inVisitor)
    {
        switch (inType) {
        case OffscreenRendererKeyTypes::RegisteredString:
            return inVisitor(*reinterpret_cast<QString *>(inData));
        case OffscreenRendererKeyTypes::VoidPtr:
            return inVisitor(*reinterpret_cast<void **>(inData));
        default:
            Q_ASSERT(false);
        case OffscreenRendererKeyTypes::NoOffscreenRendererKey:
            return inVisitor();
        }
    }

    template<typename TRetType, typename TVisitorType>
    static TRetType visit(const char *inData, TIdType inType, TVisitorType inVisitor)
    {
        switch (inType) {
        case OffscreenRendererKeyTypes::RegisteredString:
            return inVisitor(*reinterpret_cast<const QString *>(inData));
        case OffscreenRendererKeyTypes::VoidPtr:
            return inVisitor(*reinterpret_cast<const void **>(&inData));
        default:
            Q_ASSERT(false);
        case OffscreenRendererKeyTypes::NoOffscreenRendererKey:
            return inVisitor();
        }
    }
};

using QDemonOffscreenRendererKeyUnionType = DiscriminatedUnion<DiscriminatedUnionGenericBase<QDemonOffscreenRendererKeyUnionTraits, QDemonOffscreenRendererKeyUnionTraits::TBufferSize>,
                                                               QDemonOffscreenRendererKeyUnionTraits::TBufferSize>;

struct QDemonOffscreenRendererKey : public QDemonOffscreenRendererKeyUnionType
{
    typedef QDemonOffscreenRendererKeyUnionType TBase;
    QDemonOffscreenRendererKey() {}
    QDemonOffscreenRendererKey(const QString &str) : TBase(str) {}
    QDemonOffscreenRendererKey(void *key) : TBase(key) {}
    QDemonOffscreenRendererKey(const QDemonOffscreenRendererKey &other) : TBase(static_cast<const TBase &>(other)) {}
    QDemonOffscreenRendererKey &operator=(const QDemonOffscreenRendererKey &other)
    {
        TBase::operator=(other);
        return *this;
    }
};
QT_END_NAMESPACE

#endif
