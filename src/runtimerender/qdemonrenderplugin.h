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
#ifndef QDEMON_RENDER_PLUGIN_H
#define QDEMON_RENDER_PLUGIN_H

#include <QtDemonRuntimeRender/qdemonrenderplugincinterface.h>
#include <QtDemonRuntimeRender/qdemonoffscreenrendermanager.h>

QT_BEGIN_NAMESPACE

// UICRenderPluginPropertyValue.h
struct SRenderPropertyValueUpdate;

class IRenderPluginInstance : public IOffscreenRenderer
{
protected:
    virtual ~IRenderPluginInstance() {}
public:
    static const char *IRenderPluginOffscreenRendererType() { return "IRenderPluginInstance"; }
    // If this render plugin has an instance ptr, get it.
    virtual TRenderPluginInstancePtr GetRenderPluginInstance() = 0;
    virtual void Update(QDemonConstDataRef<SRenderPropertyValueUpdate> updateBuffer) = 0;
    virtual IRenderPluginClass &GetPluginClass() = 0;
    virtual void CreateScriptProxy(script_State *state) = 0;
};
struct RenderPluginPropertyValueTypes
{
    enum Enum {
        NoRenderPluginPropertyValue = 0,
        Boolean,
        Long,
        Float,
        String,
    };
};

struct SRenderPluginPropertyTypes
{
    enum Enum {
        UnknownRenderPluginPropertyType = 0,
        Float,
        Vector3,
        Vector2,
        Color,
        Boolean,
        Long,
        String,
    };
};

struct SRenderPluginPropertyDeclaration
{
    QString m_Name;
    SRenderPluginPropertyTypes::Enum m_Type;
    // Filled in by the class, ignored if set on registered property
    quint32 m_StartOffset;
    SRenderPluginPropertyDeclaration()
        : m_Type(SRenderPluginPropertyTypes::UnknownRenderPluginPropertyType)
    {
    }
    SRenderPluginPropertyDeclaration(QString n, SRenderPluginPropertyTypes::Enum t)
        : m_Name(n)
        , m_Type(t)
        , m_StartOffset(0)
    {
    }
};

class IRenderPluginClass : public QDemonRefCounted
{
protected:
    virtual ~IRenderPluginClass() {}
public:
    virtual QDemonScopedRefCounted<IRenderPluginInstance> CreateInstance() = 0;
    virtual void RegisterProperty(const SRenderPluginPropertyDeclaration &dec) = 0;
    virtual QDemonConstDataRef<SRenderPluginPropertyDeclaration> GetRegisteredProperties() = 0;
    // The declaration contains an offset
    virtual SRenderPluginPropertyDeclaration
    GetPropertyDeclaration(QString inPropName) = 0;
    // From which you can get the property name breakdown
    virtual QPair<QString, RenderPluginPropertyValueTypes::Enum>
    GetPropertyValueInfo(quint32 inIndex) = 0;
};

class IRenderPluginManager;

class IRenderPluginManagerCore : public QDemonRefCounted
{
public:
    virtual void SetDllDir(const char *inDllDir) = 0;
    virtual void Load(QDemonDataRef<quint8> inData, CStrTableOrDataRef inStrDataBlock,
                      const char *inProjectDir) = 0;
    virtual IRenderPluginManager &GetRenderPluginManager(QDemonRenderContext &rc) = 0;

    static IRenderPluginManagerCore &Create(NVFoundationBase &inFoundation,
                                            IStringTable &strTable,
                                            IInputStreamFactory &inFactory);
};

class IRenderPluginManager : public QDemonRefCounted
{
public:
    virtual IRenderPluginClass *GetRenderPlugin(QString inRelativePath) = 0;
    virtual IRenderPluginClass *GetOrCreateRenderPlugin(QString inRelativePath) = 0;
    // Map a render plugin instance to this key.  The instance's lifetime is managed by the
    // manager so a client does not
    // need to manage it.
    virtual IRenderPluginInstance *
    GetOrCreateRenderPluginInstance(QString inRelativePath, void *inKey) = 0;

    virtual void Save(SWriteBuffer &ioBuffer,
                      const SStrRemapMap &inRemapMap,
                      const char *inProjectDir) const = 0;
};
QT_END_NAMESPACE

#endif
