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
#include <QtDemonRuntimeRender/qdemonrenderplugin.h>
#include <Qt3DSAtomic.h>
#include <Qt3DSFoundation.h>
#include <Qt3DSBroadcastingAllocator.h>
#include <SerializationTypes.h>
#include <IOStreams.h>
#include <Qt3DSSystem.h>
#include <FileTools.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRuntimeRender/qdemonrenderstring.h>
#include <qdemonrenderpluginpropertyvalue.h>
#include <qdemonrenderinputstreamfactory.h>

QT_BEGIN_NAMESPACE

namespace {
// Legacy definitions...
// API version 1 definitions
typedef struct _RenderPluginSurfaceDescriptionV1
{
    long m_Width;
    long m_Height;
    enum QDEMONRenderPluginDepthTypes m_DepthBuffer;
    enum QDEMONRenderPluginTextureTypes m_ColorBuffer;
    TBool m_HasStencilBuffer;
} TRenderPluginSurfaceDescriptionV1;

typedef TNeedsRenderResult (*TNeedsRenderFunctionV1)(TRenderPluginClassPtr cls,
                                                     TRenderPluginInstancePtr instance,
                                                     TRenderPluginSurfaceDescriptionV1 surface,
                                                     TVec2 presScaleFactor);

typedef void (*TRenderFunctionV1)(TRenderPluginClassPtr cls, TRenderPluginInstancePtr instance,
                                  TRenderPluginSurfaceDescriptionV1 surface,
                                  TVec2 presScaleFactor,
                                  QDEMONRenderPluginColorClearState inClearColorBuffer);

// End API version 1 definitions

TRenderPluginSurfaceDescription ToCInterface(const SOffscreenRendererEnvironment &env)
{
    TRenderPluginSurfaceDescription retval;
    retval.m_Width = (long)env.m_Width;
    retval.m_Height = (long)env.m_Height;
    retval.m_ColorBuffer = static_cast<QDEMONRenderPluginTextureTypes>(env.m_Format);
    retval.m_DepthBuffer = static_cast<QDEMONRenderPluginDepthTypes>(env.m_Depth);
    retval.m_HasStencilBuffer = env.m_Stencil ? TTRUE : TFALSE;
    retval.m_MSAALevel = QDEMONRenderPluginMSAALevelNoMSAA;
    // note no supersampling AA support for plugins
    // we fall back to 4xMSAA
    switch (env.m_MSAAMode) {
    case AAModeValues::X2:
        retval.m_MSAALevel = QDEMONRenderPluginMSAALevelTwo;
        break;
    case AAModeValues::SSAA:
    case AAModeValues::X4:
        retval.m_MSAALevel = QDEMONRenderPluginMSAALevelFour;
        break;
    case AAModeValues::X8:
        retval.m_MSAALevel = QDEMONRenderPluginMSAALevelEight;
        break;
    default:
        Q_ASSERT(false);
        // fallthrough intentional.
    case AAModeValues::NoAA:
        break;
    };
    return retval;
}

TRenderPluginSurfaceDescriptionV1 ToCInterfaceV1(const SOffscreenRendererEnvironment &env)
{
    TRenderPluginSurfaceDescriptionV1 retval;
    retval.m_Width = (long)env.m_Width;
    retval.m_Height = (long)env.m_Height;
    retval.m_ColorBuffer = static_cast<QDEMONRenderPluginTextureTypes>(env.m_Format);
    retval.m_DepthBuffer = static_cast<QDEMONRenderPluginDepthTypes>(env.m_Depth);
    retval.m_HasStencilBuffer = env.m_Stencil ? TTRUE : TFALSE;
    return retval;
}

TVec2 ToCInterface(const QVector2D &item)
{
    TVec2 retval = { item.x, item.y };
    return retval;
}

QDEMONRenderPluginColorClearState ToCInterface(SScene::RenderClearCommand inClearCommand)
{
    switch (inClearCommand) {
    case SScene::DoNotClear:
        return QDEMONRenderPluginColorClearStateDoNotClear;
    case SScene::AlwaysClear:
        return QDEMONRenderPluginColorClearStateAlwaysClear;
    default:
        Q_ASSERT(false); // fallthrough intentional
    case SScene::ClearIsOptional:
        return QDEMONRenderPluginColorClearStateClearIsOptional;
    };
}

class SRenderPluginPropertyData
{
    SRenderPluginPropertyValue m_Value;
    bool m_Dirty;

public:
    SRenderPluginPropertyData()
        : m_Dirty(false)
    {
    }
    SRenderPluginPropertyData(const SRenderPluginPropertyData &other)
        : m_Value(other.m_Value)
        , m_Dirty(other.m_Dirty)
    {
    }
    SRenderPluginPropertyData &operator=(const SRenderPluginPropertyData &other)
    {
        m_Value = other.m_Value;
        m_Dirty = other.m_Dirty;
        return *this;
    }

    bool IsDirty() const
    {
        return m_Value.getType() != RenderPluginPropertyValueTypes::NoRenderPluginPropertyValue
                && m_Dirty;
    }
    void SetValue(const SRenderPluginPropertyValue &value)
    {
        m_Value = value;
        m_Dirty = true;
    }

    TRenderPluginPropertyUpdate ClearDirty(QString inPropName)
    {
        m_Dirty = false;
        TRenderPluginPropertyUpdate retval;
        memset(&retval, 0, sizeof(TRenderPluginPropertyUpdate));
        retval.m_PropName = inPropName.c_str();
        switch (m_Value.getType()) {
        case RenderPluginPropertyValueTypes::Long: {
            retval.m_PropertyType = QDEMONRenderPluginPropertyTypeLong;
            long temp = (long)m_Value.getData<qint32>();
            retval.m_PropertyValue = *reinterpret_cast<void **>(&temp);
        } break;
        case RenderPluginPropertyValueTypes::Float: {
            retval.m_PropertyType = QDEMONRenderPluginPropertyTypeFloat;
            float temp = m_Value.getData<float>();
            retval.m_PropertyValue = *reinterpret_cast<void **>(&temp);
        } break;
        case RenderPluginPropertyValueTypes::Boolean: {
            retval.m_PropertyType = QDEMONRenderPluginPropertyTypeLong;
            long temp = m_Value.getData<bool>() ? TTRUE : TFALSE;
            retval.m_PropertyValue = *reinterpret_cast<void **>(&temp);
        } break;
        case RenderPluginPropertyValueTypes::String: {
            retval.m_PropertyType = QDEMONRenderPluginPropertyTypeCharPtr;
            const char *temp = m_Value.getData<QString>().c_str();
            retval.m_PropertyValue = reinterpret_cast<void *>(const_cast<char *>(temp));
        } break;
        default:
            Q_ASSERT(false);
        }
        return retval;
    }
};

typedef QVector<SRenderPluginPropertyData> TPropertyValueList;

struct IInternalPluginClass : public IRenderPluginClass
{
    virtual void PushUpdates(TRenderPluginInstancePtr instance,
                             TPropertyValueList &propertyValues) = 0;
    virtual void Update(QDemonConstDataRef<SRenderPropertyValueUpdate> updateBuffer,
                        TPropertyValueList &propertyValues) = 0;
    virtual qint32 GetAPIVersion() = 0;
};

static QDemonRenderTextureFormats::Enum ToTextureFormat(QDEMONRenderPluginTextureTypes inTextureType)
{
    switch (inTextureType) {
    default:
    case QDEMONRenderPluginTextureTypeRGBA8:
        return QDemonRenderTextureFormats::RGBA8;
    case QDEMONRenderPluginTextureTypeRGB8:
        return QDemonRenderTextureFormats::RGB8;
    case QDEMONRenderPluginTextureTypeRGB565:
        return QDemonRenderTextureFormats::RGB565;
    case QDEMONRenderPluginTextureTypeRGBA5551:
        return QDemonRenderTextureFormats::RGBA5551;
    }
}

static OffscreenRendererDepthValues::Enum ToDepthValue(QDEMONRenderPluginDepthTypes inType)
{
    switch (inType) {
    default:
    case QDEMONRenderPluginDepthTypeDepth16:
        return OffscreenRendererDepthValues::Depth16;
    case QDEMONRenderPluginDepthTypeDepth24:
        return OffscreenRendererDepthValues::Depth24;
    case QDEMONRenderPluginDepthTypeDepth32:
        return OffscreenRendererDepthValues::Depth32;
    }
}

static AAModeValues::Enum ToAAMode(QDEMONRenderPluginMSAALevel inMode)
{
    switch (inMode) {
    case QDEMONRenderPluginMSAALevelTwo:
        return AAModeValues::X2;
    case QDEMONRenderPluginMSAALevelFour:
        return AAModeValues::X4;
    case QDEMONRenderPluginMSAALevelEight:
        return AAModeValues::X8;
    default:
        Q_ASSERT(false); // fallthrough intentional
    case QDEMONRenderPluginMSAALevelNoMSAA:
        return AAModeValues::NoAA;
    }
}

struct InstanceImpl : public IRenderPluginInstance
{
    TRenderPluginInstancePtr m_Instance;
    TRenderPluginClass m_Class;
    QSharedPointer<IInternalPluginClass> m_Owner;
    QString m_RendererType;
    // Backing store of property values
    QVector<SRenderPluginPropertyData> m_PropertyValues;
    bool m_Dirty;
    QDemonRenderContext *m_RenderContext;

    InstanceImpl(TRenderPluginInstancePtr instance, TRenderPluginClass cls,
                 IInternalPluginClass &owner)
        : m_Instance(instance)
        , m_Class(cls)
        , m_Owner(owner)
        , m_RendererType(QString::fromLocal8Bit(IRenderPluginInstance::IRenderPluginOffscreenRendererType()))
        , m_Dirty(false)
        , m_RenderContext(nullptr)
    {
    }

    virtual ~InstanceImpl() { m_Class.ReleaseInstance(m_Class.m_Class, m_Instance); }

    void addCallback(IOffscreenRendererCallback *cb) override
    {

    }
    void CreateScriptProxy(script_State *state) override
    {
        if (m_Class.CreateInstanceScriptProxy)
            m_Class.CreateInstanceScriptProxy(m_Class.m_Class, m_Instance, state);
    }

    // Arbitrary const char* returned to indicate the type of this renderer
    // Can be overloaded to form the basis of an RTTI type system.
    // Not currently used by the rendering system.
    QString GetOffscreenRendererType() override { return m_RendererType; }

    SOffscreenRendererEnvironment GetDesiredEnvironment(QVector2D inPresentationScaleFactor) override
    {
        if (m_Class.QueryInstanceRenderSurface) {
            QDEMONRenderPluginMSAALevel theLevel = QDEMONRenderPluginMSAALevelNoMSAA;
            TRenderPluginSurfaceDescription desc = m_Class.QueryInstanceRenderSurface(
                        m_Class.m_Class, m_Instance, ToCInterface(inPresentationScaleFactor));
            if (m_Owner->GetAPIVersion() > 1)
                theLevel = desc.m_MSAALevel;

            return SOffscreenRendererEnvironment(
                        (quint32)desc.m_Width, (quint32)desc.m_Height, ToTextureFormat(desc.m_ColorBuffer),
                        ToDepthValue(desc.m_DepthBuffer), desc.m_HasStencilBuffer ? true : false,
                        ToAAMode(theLevel));
        } else {
            Q_ASSERT(false);
        }
        return SOffscreenRendererEnvironment();
    }

    // Returns true of this object needs to be rendered, false if this object is not dirty
    SOffscreenRenderFlags NeedsRender(const SOffscreenRendererEnvironment &inEnvironment,
                                      QVector2D inPresentationScaleFactor,
                                      const SRenderInstanceId instanceId) override
    {
        if (m_Dirty) {
            m_Dirty = false;
            m_Owner->PushUpdates(m_Instance, m_PropertyValues);
        }
        if (m_Class.NeedsRenderFunction) {
            if (m_Owner->GetAPIVersion() > 1) {
                TNeedsRenderResult result = m_Class.NeedsRenderFunction(
                            m_Class.m_Class, m_Instance, ToCInterface(inEnvironment),
                            ToCInterface(inPresentationScaleFactor));
                return SOffscreenRenderFlags(result.HasTransparency ? true : false,
                                             result.HasChangedSinceLastFrame ? true : false);
            } else {
                TNeedsRenderFunctionV1 theV1Function =
                        reinterpret_cast<TNeedsRenderFunctionV1>(m_Class.NeedsRenderFunction);

                TNeedsRenderResult result =
                        theV1Function(m_Class.m_Class, m_Instance, ToCInterfaceV1(inEnvironment),
                                      ToCInterface(inPresentationScaleFactor));
                return SOffscreenRenderFlags(result.HasTransparency ? true : false,
                                             result.HasChangedSinceLastFrame ? true : false);
            }
        }
        return SOffscreenRenderFlags(true, true);
    }
    // Returns true if the rendered result image has transparency, or false
    // if it should be treated as a completely opaque image.
    // It is the IOffscreenRenderer's job to clear any buffers (color, depth, stencil) that it
    // needs to.  It should not assume that it's buffers are clear;
    // Sometimes we scale the width and height of the main presentation in order to fit a window.
    // If we do so, the scale factor tells the subpresentation renderer how much the system has
    // scaled.
    void Render(const SOffscreenRendererEnvironment &inEnvironment,
                QDemonRenderContext &inRenderContext, QVector2D inPresentationScaleFactor,
                SScene::RenderClearCommand inColorBufferNeedsClear,
                const SRenderInstanceId instanceId) override
    {
        m_RenderContext = &inRenderContext;
        if (m_Class.RenderInstance) {
            inRenderContext.PushPropertySet();
            if (m_Owner->GetAPIVersion() > 1) {
                m_Class.RenderInstance(m_Class.m_Class, m_Instance, ToCInterface(inEnvironment),
                                       ToCInterface(inPresentationScaleFactor),
                                       ToCInterface(inColorBufferNeedsClear));
            } else {
                TRenderFunctionV1 theV1Function =
                        reinterpret_cast<TRenderFunctionV1>(m_Class.RenderInstance);
                theV1Function(m_Class.m_Class, m_Instance, ToCInterfaceV1(inEnvironment),
                              ToCInterface(inPresentationScaleFactor),
                              ToCInterface(inColorBufferNeedsClear));
            }

            inRenderContext.PopPropertySet(true);
        }
    }

    void RenderWithClear(const SOffscreenRendererEnvironment &inEnvironment,
                         QDemonRenderContext &inRenderContext, QVector2D inPresScale,
                         SScene::RenderClearCommand inClearBuffer, QVector3D inClearColor,
                         const SRenderInstanceId id)
    {
        Q_ASSERT(false);
    }

    // Implementors should implement one of the two interfaces below.

    // If this renderer supports picking that can return graph objects
    // then return an interface here.
    IGraphObjectPickQuery *GetGraphObjectPickQuery(const SRenderInstanceId) override { return nullptr; }

    // If you *don't* support the GraphObjectPickIterator interface, then you should implement this
    // interface
    // The system will just ask you to pick.
    // If you return true, then we will assume that you swallowed the pick and will continue no
    // further.
    // else we will assume you did not and will continue the picking algorithm.
    bool Pick(const QVector2D &inMouseCoords, const QVector2D &inViewportDimensions,
              const SRenderInstanceId instanceId) override
    {
        if (m_Class.Pick) {
            if (m_RenderContext) {
                m_RenderContext->PushPropertySet();
                bool retval = m_Class.Pick(m_Class.m_Class, m_Instance, ToCInterface(inMouseCoords),
                                           ToCInterface(inViewportDimensions))
                        ? true
                        : false;
                m_RenderContext->PopPropertySet(true);
                return retval;
            }
        }
        return false;
    }

    TRenderPluginInstancePtr GetRenderPluginInstance() override { return m_Instance; }
    void Update(QDemonConstDataRef<SRenderPropertyValueUpdate> updateBuffer) override
    {
        m_Dirty = true;
        m_Owner->Update(updateBuffer, m_PropertyValues);
    }
    IRenderPluginClass &GetPluginClass() override { return *m_Owner; }
};

typedef QPair<QString, RenderPluginPropertyValueTypes::Enum> TStringTypePair;

struct PluginClassImpl : public IInternalPluginClass
{
    typedef QHash<QString, quint32> TStringIndexMap;
    TRenderPluginClass m_Class;
    QString m_Type;
    CLoadedDynamicLibrary *m_DynamicLibrary;
    QVector<SRenderPluginPropertyDeclaration> m_RegisteredProperties;
    TStringIndexMap m_ComponentNameToComponentIndexMap;
    QVector<TStringTypePair> m_FullPropertyList;
    QVector<TRenderPluginPropertyUpdate> m_UpdateBuffer;
    QString m_TempString;
    qint32 m_APIVersion;

    PluginClassImpl(TRenderPluginClass inClass,
                    QString inType, CLoadedDynamicLibrary *inLibrary)
        : m_Class(inClass)
        , m_Type(inType)
        , m_DynamicLibrary(inLibrary)
        , m_APIVersion(m_Class.GetRenderPluginAPIVersion(m_Class.m_Class))
    {
    }
    ~PluginClassImpl()
    {
        if (m_Class.ReleaseClass)
            m_Class.ReleaseClass(m_Class.m_Class);
        if (m_DynamicLibrary)
            delete m_DynamicLibrary
    }

    QSharedPointer<IRenderPluginInstance> CreateInstance() override
    {
        if (m_Class.CreateInstance) {
            TRenderPluginInstancePtr instance =
                    m_Class.CreateInstance(m_Class.m_Class, m_Type.c_str());
            if (instance) {
                InstanceImpl *retval = new InstanceImpl(
                            m_Foundation, instance, m_Class, *this, m_StringTable);
                return retval;
            }
        }
        return QSharedPointer<IRenderPluginInstance>();
    }

    qint32 GetAPIVersion() override { return m_APIVersion; }

    void AddFullPropertyType(const char *name, RenderPluginPropertyValueTypes::Enum inType)
    {
        quint32 itemIndex = (quint32)m_FullPropertyList.size();
        QString regName = QString::fromLocal8Bit(name);
        bool inserted =
                m_ComponentNameToComponentIndexMap.insert(regName, itemIndex).second;
        if (inserted) {
            m_FullPropertyList.push_back(eastl::make_pair(regName, inType));
        } else {
            // Duplicate property declaration.
            Q_ASSERT(false);
        }
    }

    void AddFullPropertyType(const char *name, const char *extension,
                             RenderPluginPropertyValueTypes::Enum inType)
    {
        m_TempString.assign(name);
        if (!isTrivial(extension)) {
            m_TempString.append(".");
            m_TempString.append(extension);
        }
        AddFullPropertyType(m_TempString.c_str(), inType);
    }

    void RegisterProperty(const SRenderPluginPropertyDeclaration &dec) override
    {
        quint32 startOffset = (quint32)m_FullPropertyList.size();

        switch (dec.m_Type) {

        case SRenderPluginPropertyTypes::Vector2:
            AddFullPropertyType(dec.m_Name, "x", RenderPluginPropertyValueTypes::Float);
            AddFullPropertyType(dec.m_Name, "y", RenderPluginPropertyValueTypes::Float);
            break;
        case SRenderPluginPropertyTypes::Color:
            AddFullPropertyType(dec.m_Name, "r", RenderPluginPropertyValueTypes::Float);
            AddFullPropertyType(dec.m_Name, "g", RenderPluginPropertyValueTypes::Float);
            AddFullPropertyType(dec.m_Name, "b", RenderPluginPropertyValueTypes::Float);
            break;
        case SRenderPluginPropertyTypes::Vector3:
            AddFullPropertyType(dec.m_Name, "x", RenderPluginPropertyValueTypes::Float);
            AddFullPropertyType(dec.m_Name, "y", RenderPluginPropertyValueTypes::Float);
            AddFullPropertyType(dec.m_Name, "z", RenderPluginPropertyValueTypes::Float);
            break;
        case SRenderPluginPropertyTypes::Boolean:
            AddFullPropertyType(dec.m_Name, RenderPluginPropertyValueTypes::Boolean);
            break;
        case SRenderPluginPropertyTypes::Float:
            AddFullPropertyType(dec.m_Name, RenderPluginPropertyValueTypes::Float);
            break;
        case SRenderPluginPropertyTypes::Long:
            AddFullPropertyType(dec.m_Name, RenderPluginPropertyValueTypes::Long);
            break;
        case SRenderPluginPropertyTypes::String:
            AddFullPropertyType(dec.m_Name, RenderPluginPropertyValueTypes::String);
            break;
        default:
            Q_ASSERT(false);
            break;
        }
        m_RegisteredProperties.push_back(dec);
        m_RegisteredProperties.back().m_StartOffset = startOffset;
    }

    QDemonConstDataRef<SRenderPluginPropertyDeclaration> GetRegisteredProperties() override
    {
        return m_RegisteredProperties;
    }

    SRenderPluginPropertyDeclaration GetPropertyDeclaration(QString inPropName) override
    {
        for (quint32 idx = 0, end = m_RegisteredProperties.size(); idx < end; ++idx) {
            if (m_RegisteredProperties[idx].m_Name == inPropName)
                return m_RegisteredProperties[idx];
        }
        Q_ASSERT(false);
        return SRenderPluginPropertyDeclaration();
    }

    // From which you can get the property name breakdown
    virtual QPair<QString, RenderPluginPropertyValueTypes::Enum>
    GetPropertyValueInfo(quint32 inIndex) override
    {
        if (inIndex < m_FullPropertyList.size())
            return m_FullPropertyList[inIndex];
        Q_ASSERT(false);
        return QPair<QString, RenderPluginPropertyValueTypes::Enum>(
                    QString(), RenderPluginPropertyValueTypes::NoRenderPluginPropertyValue);
    }

    void PushUpdates(TRenderPluginInstancePtr instance, TPropertyValueList &propertyValues) override
    {
        m_UpdateBuffer.clear();
        for (quint32 idx = 0, end = propertyValues.size(); idx < end; ++idx) {
            SRenderPluginPropertyData &theData(propertyValues[idx]);
            if (theData.IsDirty())
                m_UpdateBuffer.push_back(theData.ClearDirty(m_FullPropertyList[idx].first));
        }
        if (m_Class.UpdateInstance)
            m_Class.UpdateInstance(m_Class.m_Class, instance, m_UpdateBuffer.data(),
                                   (long)m_UpdateBuffer.size());
    }

    void Update(QDemonConstDataRef<SRenderPropertyValueUpdate> updateBuffer,
                TPropertyValueList &propertyValues) override
    {
        for (quint32 idx = 0, end = updateBuffer.size(); idx < end; ++idx) {
            const SRenderPropertyValueUpdate &update = updateBuffer[idx];
            TStringIndexMap::iterator iter =
                    m_ComponentNameToComponentIndexMap.find(update.m_PropertyName);
            if (iter == m_ComponentNameToComponentIndexMap.end()) {
                Q_ASSERT(false);
                continue;
            }

            quint32 propIndex = iter->second;
            if (update.m_Value.getType() != m_FullPropertyList[propIndex].second) {
                Q_ASSERT(false);
                continue;
            }
            if (propIndex >= propertyValues.size())
                propertyValues.resize(propIndex + 1);
            propertyValues[propIndex].SetValue(update.m_Value);
        }
    }
};

struct PluginInstanceKey
{
    QString m_Path;
    void *m_InstanceKey;
    PluginInstanceKey(QString p, void *ik)
        : m_Path(p)
        , m_InstanceKey(ik)
    {
    }
    bool operator==(const PluginInstanceKey &rhs) const
    {
        return m_Path == rhs.m_Path && m_InstanceKey == rhs.m_InstanceKey;
    }
};
}

namespace eastl {
template <>
struct hash<PluginInstanceKey>
{
    size_t operator()(const PluginInstanceKey &k) const
    {
        return hash<QString>()(k.m_Path)
                ^ hash<size_t>()(reinterpret_cast<size_t>(k.m_InstanceKey));
    }
    bool operator()(const PluginInstanceKey &lhs, const PluginInstanceKey &rhs) const
    {
        return lhs.m_Path == rhs.m_Path && lhs.m_InstanceKey == rhs.m_InstanceKey;
    }
};
}

namespace {

struct SLoadedPluginData
{
    QString m_PluginPath;
    eastl::vector<SRenderPluginPropertyDeclaration> m_Properties;
};

typedef eastl::vector<SLoadedPluginData> TLoadedPluginDataList;

struct PluginManagerImpl : public IRenderPluginManager, public IRenderPluginManagerCore
{
    typedef QHash<QString, QSharedPointer<IRenderPluginClass>> TLoadedClassMap;
    typedef QHash<PluginInstanceKey, QSharedPointer<IRenderPluginInstance>> TInstanceMap;
    TLoadedClassMap m_LoadedClasses;
    TInstanceMap m_Instances;
    QSharedPointer<QDemonRenderContext> m_RenderContext;
    IInputStreamFactory &m_InputStreamFactory;
    TStr m_DllDir;
    TLoadedPluginDataList m_LoadedPluginData;

    PluginManagerImpl(IInputStreamFactory &inFactory)
        : m_InputStreamFactory(inFactory)
    {
    }

    IRenderPluginClass *GetRenderPlugin(QString inRelativePath) override
    {
        TLoadedClassMap::iterator iter = m_LoadedClasses.find(inRelativePath);
        if (iter != m_LoadedClasses.end())
            return iter->second;

        return QSharedPointer<IRenderPluginClass>();
    }

    IRenderPluginClass *GetOrCreateRenderPlugin(QString inRelativePath) override
    {
        TLoadedClassMap::iterator iter = m_LoadedClasses.find(inRelativePath);
        if (iter != m_LoadedClasses.end()) {
            return iter->second;
        }

        // We insert right here to keep us from going down this path potentially for every instance.
        iter =
                m_LoadedClasses
                .insert(inRelativePath, QSharedPointer<IRenderPluginClass>())
                .first;
        QString xmlDir, fname, extension;

        CFileTools::Split(inRelativePath.c_str(), xmlDir, fname, extension);

        QString sharedLibrary(xmlDir);
        QString subdir(System::getPlatformGLStr());
        QString libdir;
        QString libpath;

        CFileTools::CombineBaseAndRelative(xmlDir.c_str(), subdir.c_str(), libdir);
        CFileTools::CombineBaseAndRelative(libdir.c_str(), fname.c_str(), libpath);
#ifdef _DEBUG
        libpath.append("d");
#endif
        libpath.append(System::g_DLLExtension);
        QString loadPath;
        if (m_DllDir.size()) {
            // Then we have to copy the dll to the dll directory before loading because the
            // filesystem
            // the plugin is on may not be executable.
            QString targetFile;
            CFileTools::CombineBaseAndRelative(m_DllDir.c_str(), fname.c_str(), targetFile);
#ifdef _DEBUG
            targetFile.append("d");
#endif
            targetFile.append(System::g_DLLExtension);

            qCInfo(TRACE_INFO, "Copying plugin shared library from %s to %s",
                   libpath.c_str(), targetFile.c_str());

            // try to open the library.
            QSharedPointer<IRefCountedInputStream> theStream =
                    m_InputStreamFactory.GetStreamForFile(libpath.c_str());
            if (!theStream) {
                qCCritical(INVALID_OPERATION, "Failed to load render plugin %s",
                           libpath.c_str());
                return nullptr;
            }
            CFileSeekableIOStream outStream(targetFile.c_str(), FileWriteFlags());
            if (!outStream.IsOpen()) {
                qCCritical(INVALID_OPERATION, "Failed to load render plugin %s",
                           targetFile.c_str());
                return nullptr;
            }

            quint8 buf[1024] = { 0 };
            for (quint32 len = theStream->Read(toDataRef(buf, 1024)); len;
                 len = theStream->Read(toDataRef(buf, 1024))) {
                outStream.Write(toDataRef(buf, len));
            }
            loadPath = targetFile;
        } else {
            QString path;
            m_InputStreamFactory.GetPathForFile(libpath.c_str(), path);
            loadPath = path.toUtf8().data();
        }
        CLoadedDynamicLibrary *library = nullptr;
        TRenderPluginClass newPluginClass;
        memSet(&newPluginClass, 0, sizeof(newPluginClass));

        // Do not load plugin dlls during compilation steps or when we don't have a valid render
        // context.
        // They may try opengl access at some point and that would end in disaster during binary
        // save steps.
        if ((quint32)m_RenderContext->GetRenderContextType() != QDemonRenderContextValues::NullContext) {
            library = CLoadedDynamicLibrary::Create(loadPath.c_str(), m_Foundation);
            if (!library) {
                // try loading it from the system instead of from this specific path.  This means do
                // not use any extensions or any special
                // sauce.
                loadPath = fname;
#ifdef _DEBUG
                loadPath.append("d");
#endif
                library = CLoadedDynamicLibrary::Create(loadPath.c_str(), m_Foundation);
            }
        }

        if (library) {
            TCreateRenderPluginClassFunction CreateClass =
                    reinterpret_cast<TCreateRenderPluginClassFunction>(
                        library->FindFunction("CreateRenderPlugin"));
            if (CreateClass) {
                newPluginClass = CreateClass(fname.c_str());
                if (newPluginClass.m_Class) {
                    // Check that the required functions are there.
                    if (newPluginClass.CreateInstance == nullptr
                            || newPluginClass.QueryInstanceRenderSurface == nullptr
                            || newPluginClass.RenderInstance == nullptr
                            || newPluginClass.ReleaseInstance == nullptr
                            || newPluginClass.ReleaseClass == nullptr) {
                        if (newPluginClass.ReleaseClass)
                            newPluginClass.ReleaseClass(newPluginClass.m_Class);
                        qCCritical(INVALID_OPERATION,
                                   "Failed to load render plugin: %s, required functions "
                                   "missing.  Required functions are:"
                                   "CreateInstance, QueryInstanceRenderSurface, "
                                   "RenderInstance, ReleaseInstance, ReleaseClass",
                                   inRelativePath.c_str());
                        delete library;
                        memSet(&newPluginClass, 0, sizeof(newPluginClass));
                    }
                }
            }
        }
        if (newPluginClass.m_Class) {
            PluginClassImpl *retval = new PluginClassImpl(
                        m_StringTable, newPluginClass,
                        QString::fromLocal8Bit(fname.c_str()), library);

            iter->second = retval;
            if (newPluginClass.InitializeClassGLResources) {
                m_RenderContext->PushPropertySet();
                newPluginClass.InitializeClassGLResources(newPluginClass.m_Class, loadPath.c_str());
                m_RenderContext->PopPropertySet(true);
            }
            return iter->second;
        }
        return nullptr;
    }

    void SetDllDir(const char *inDllDir) override { m_DllDir.assign(nonNull(inDllDir)); }

    IRenderPluginInstance *GetOrCreateRenderPluginInstance(QString inRelativePath,
                                                           void *inKey) override
    {
        PluginInstanceKey theKey(inRelativePath, inKey);
        TInstanceMap::iterator iter = m_Instances.find(theKey);
        if (iter == m_Instances.end()) {
            IRenderPluginClass *theClass = GetOrCreateRenderPlugin(inRelativePath);
            QSharedPointer<IRenderPluginInstance> theInstance;
            if (theClass)
                theInstance = theClass->CreateInstance();

            iter = m_Instances.insert(theKey, theInstance).first;
        }
        return iter->second.mPtr;
    }

    void Save(SWriteBuffer &ioBuffer,
              const SStrRemapMap &inRemapMap,
              const char * /*inProjectDir*/) const override
    {
        quint32 numClasses = m_LoadedClasses.size();
        ioBuffer.write(numClasses);
        for (TLoadedClassMap::const_iterator iter = m_LoadedClasses.begin(),
             end = m_LoadedClasses.end();
             iter != end; ++iter) {
            QString saveStr = iter->first;
            saveStr.Remap(inRemapMap);
            ioBuffer.write(saveStr);
            if (iter->second) {
                QDemonConstDataRef<SRenderPluginPropertyDeclaration> theProperties =
                        const_cast<IRenderPluginClass &>((*iter->second)).GetRegisteredProperties();
                ioBuffer.write(theProperties.size());
                for (quint32 idx = 0, end = theProperties.size(); idx < end; ++idx) {
                    SRenderPluginPropertyDeclaration theDec(theProperties[idx]);
                    theDec.m_Name.Remap(inRemapMap);
                    ioBuffer.write(theDec);
                }
            } else
                ioBuffer.write((quint32)0);
        }
    }

    void Load(QDemonDataRef<quint8> inData, CStrTableOrDataRef inStrDataBlock,
              const char * /*inProjectDir*/) override
    {
        SDataReader theReader(inData.begin(), inData.end());
        quint32 numClasses = theReader.LoadRef<quint32>();
        ForwardingAllocator alloc(m_Foundation.getAllocator(), "tempstrings");
        TStr workStr(alloc);
        QVector<SRenderPluginPropertyDeclaration> propertyBuffer(m_Foundation.getAllocator(),
                                                                  "tempprops");
        for (quint32 classIdx = 0; classIdx < numClasses; ++classIdx) {
            QString classPath = theReader.LoadRef<QString>();
            classPath.Remap(inStrDataBlock);
            quint32 numProperties = theReader.LoadRef<quint32>();
            propertyBuffer.clear();
            for (quint32 propIdx = 0; propIdx < numProperties; ++propIdx) {
                propertyBuffer.push_back(theReader.LoadRef<SRenderPluginPropertyDeclaration>());
                propertyBuffer.back().m_Name.Remap(inStrDataBlock);
            }
            m_LoadedPluginData.push_back(SLoadedPluginData());
            m_LoadedPluginData.back().m_PluginPath = classPath;
            m_LoadedPluginData.back().m_Properties.assign(propertyBuffer.begin(),
                                                          propertyBuffer.end());
        }
    }
    IRenderPluginManager &GetRenderPluginManager(QDemonRenderContext &rc) override
    {
        m_RenderContext = rc;
        for (quint32 idx = 0, end = m_LoadedPluginData.size(); idx < end; ++idx) {
            // Now we can attempt to load the class.
            IRenderPluginClass *theClass =
                    GetOrCreateRenderPlugin(m_LoadedPluginData[idx].m_PluginPath);
            if (theClass) {
                eastl::vector<SRenderPluginPropertyDeclaration> &propertyBuffer(
                            m_LoadedPluginData[idx].m_Properties);
                for (quint32 propIdx = 0, propEnd = propertyBuffer.size(); propIdx < propEnd;
                     ++propIdx) {
                    theClass->RegisterProperty(propertyBuffer[propIdx]);
                }
            }
        }
        m_LoadedPluginData.clear();
        return *this;
    }
};
}

IRenderPluginManagerCore &IRenderPluginManagerCore::Create(IInputStreamFactory &inFactory)
{
    return *new PluginManagerImpl(inFactory);
}

QT_END_NAMESPACE
