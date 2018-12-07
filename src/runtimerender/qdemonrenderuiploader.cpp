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
#ifdef QDEMON_RENDER_ENABLE_LOAD_UIP

#include <Qt3DSRenderUIPLoader.h>
#include <QtDemonRuntimeRender/qdemonrenderpresentation.h>
#include <QtDemonRuntimeRender/qdemonrendernode.h>
#include <QtDemonRuntimeRender/qdemonrenderlight.h>
#include <QtDemonRuntimeRender/qdemonrendercamera.h>
#include <QtDemonRuntimeRender/qdemonrenderlayer.h>
#include <QtDemonRuntimeRender/qdemonrendermodel.h>
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterial.h>
#include <QtDemonRuntimeRender/qdemonrenderimage.h>
#include <QtDemonRuntimeRender/qdemonrenderbuffermanager.h>
#include <Qt3DSRenderUIPSharedTranslation.h>
#include <vector>
#include <map>
#include <set>
#ifdef EA_PLATFORM_WINDOWS
#pragma warning(disable : 4201)
#endif
#include <Qt3DSDMXML.h>
#include <Qt3DSTypes.h>
#include <Qt3DSVector3.h>
#include <Qt3DSMetadata.h>
#include <Qt3DSDMWStrOps.h>
#include <Qt3DSDMWStrOpsImpl.h>
#include <QtDemon/qdemonoption.h>
#include <Qt3DSContainers.h>
#include <Qt3DSFoundation.h>
#include <Qt3DSBroadcastingAllocator.h>
#include <Qt3DSDMComposerTypeDefinitions.h>
#include <StrConvertUTF.h>
#include <Qt3DSRenderEffectSystem.h>
#include <Qt3DSRenderString.h>
#include <FileTools.h>
#include <Qt3DSRenderDynamicObjectSystemCommands.h>
#include <QtDemonRuntimeRender/qdemonrendereffect.h>
#include <Qt3DSDMMetaDataTypes.h>
#include <Qt3DSTextRenderer.h>
#include <Qt3DSRenderPlugin.h>
#include <Qt3DSRenderPluginGraphObject.h>
#include <Qt3DSRenderPluginPropertyValue.h>
#include <Qt3DSRenderDynamicObjectSystem.h>
#include <Qt3DSRenderCustomMaterialSystem.h>
#include <Qt3DSRenderMaterialHelpers.h>
#include <QtDemonRuntimeRender/qdemonrenderpath.h>
#include <QtDemonRuntimeRender/qdemonrenderpathsubpath.h>
#include <Qt3DSRenderPathManager.h>

using Option;
using Empty;
using float;
using QVector3D;
using QVector;
using quint32;
using RenderLightTypes;
using DefaultMaterialLighting;
using ImageMappingModes;
using DefaultMaterialBlendMode;
using QDemonRenderTextureCoordOp;
using IStringTable;
using NVFoundationBase;
using namespace qt3ds;
using namespace foundation;
using TIdObjectMap;
using IBufferManager;
using IEffectSystem;
using SPresentation;
using SScene;
using SLayer;
using SNode;
using SLight;
using SCamera;
using SModel;
using SText;
using SDefaultMaterial;
using SImage;
using SGraphObject;
using SDynamicObject;
using SEffect;
using SCustomMaterial;
using GraphObjectTypes;
using NodeFlags;
using QString;
using CRenderString;
using CFileTools;
using SReferencedMaterial;
using IUIPReferenceResolver;
using SPath;
using SPathSubPath;
using SLightmaps;

namespace qt3dsdm {
template <>
struct WStrOps<SFloat2>
{
    void StrTo(const char *buffer, SFloat2 &item, QVector<char> &ioTempBuf)
    {
        quint32 len = (quint32)strlen(buffer);
        ioTempBuf.resize(len + 1);
        ::memcpy(ioTempBuf.data(), buffer, (len + 1) * sizeof(char));
        MemoryBuffer<RawAllocator> unused;
        qt3dsdm::IStringTable *theTable(nullptr);
        WCharTReader reader(ioTempBuf.begin(), unused, *theTable);
        reader.ReadRef(QDemonDataRef<float>(item.m_Floats, 2));
    }
};

template <>
struct WStrOps<SFloat3>
{
    void StrTo(const char *buffer, SFloat3 &item, QVector<char> &ioTempBuf)
    {
        quint32 len = (quint32)strlen(buffer);
        ioTempBuf.resize(len + 1);
        ::memcpy(ioTempBuf.data(), buffer, (len + 1) * sizeof(char));
        MemoryBuffer<RawAllocator> unused;
        qt3dsdm::IStringTable *theTable(nullptr);
        WCharTReader reader(ioTempBuf.begin(), unused, *theTable);
        reader.ReadRef(QDemonDataRef<float>(item.m_Floats, 3));
    }
};
}

namespace {

typedef eastl::basic_string<char> TStrType;
struct IPropertyParser
{
    virtual ~IPropertyParser() {}
    virtual QDemonOption<TStrType> ParseStr(const char *inName) = 0;
    virtual QDemonOption<float> ParseFloat(const char *inName) = 0;
    virtual QDemonOption<QVector2D> ParseVec2(const char *inName) = 0;
    virtual QDemonOption<QVector3D> ParseVec3(const char *inName) = 0;
    virtual QDemonOption<bool> ParseBool(const char *inName) = 0;
    virtual QDemonOption<quint32> ParseU32(const char *inName) = 0;
    virtual QDemonOption<qint32> ParseI32(const char *inName) = 0;
    virtual QDemonOption<SGraphObject *> ParseGraphObject(const char *inName) = 0;
    virtual QDemonOption<SNode *> ParseNode(const char *inName) = 0;
};
struct SMetaPropertyParser : public IPropertyParser
{
    Q3DStudio::IRuntimeMetaData &m_MetaData;
    TStrType m_TempStr;
    QString m_Type;
    QString m_ClassId;

    SMetaPropertyParser(const char *inType, const char *inClass,
                        Q3DStudio::IRuntimeMetaData &inMeta)
        : m_MetaData(inMeta)
        , m_Type(inMeta.GetStringTable()->GetRenderStringTable().RegisterStr(inType))
        , m_ClassId(inMeta.GetStringTable()->GetRenderStringTable().RegisterStr(inClass))
    {
    }

    QString Register(const char *inName)
    {
        return m_MetaData.GetStringTable()->GetRenderStringTable().RegisterStr(inName);
    }

    QDemonOption<TStrType> ParseStr(const char *inName) override
    {
        QString theName(Register(inName));
        Q3DStudio::ERuntimeDataModelDataType theType(
            m_MetaData.GetPropertyType(m_Type, theName, m_ClassId));
        if (theType != Q3DStudio::ERuntimeDataModelDataTypeObjectRef
            && theType != Q3DStudio::ERuntimeDataModelDataTypeLong4) {
            return m_MetaData.GetPropertyValueString(m_Type, theName, m_ClassId);
        }
        return Empty();
    }
    QDemonOption<float> ParseFloat(const char *inName) override
    {
        return m_MetaData.GetPropertyValueFloat(m_Type, Register(inName), m_ClassId);
    }
    QDemonOption<QVector2D> ParseVec2(const char *inName) override
    {
        QDemonOption<QVector3D> theProperty =
            m_MetaData.GetPropertyValueVector2(m_Type, Register(inName), m_ClassId);
        if (theProperty.hasValue()) {
            return QVector2D(theProperty->x, theProperty->y);
        }
        return Empty();
    }
    QDemonOption<QVector3D> ParseVec3(const char *inName) override
    {
        QDemonOption<QVector3D> theProperty =
            m_MetaData.GetPropertyValueVector3(m_Type, Register(inName), m_ClassId);
        if (theProperty.hasValue()) {
            return *theProperty;
        }
        return Empty();
    }
    QDemonOption<bool> ParseBool(const char *inName) override
    {
        return m_MetaData.GetPropertyValueBool(m_Type, Register(inName), m_ClassId);
    }

    QDemonOption<quint32> ParseU32(const char *inName) override
    {
        QDemonOption<qint32> retval = m_MetaData.GetPropertyValueLong(m_Type, Register(inName), m_ClassId);
        if (retval.hasValue())
            return (quint32)retval.getValue();
        return Empty();
    }

    QDemonOption<qint32> ParseI32(const char *inName) override
    {
        QDemonOption<qint32> retval = m_MetaData.GetPropertyValueLong(m_Type, Register(inName), m_ClassId);
        if (retval.hasValue())
            return (qint32)retval.getValue();
        return Empty();
    }

    QDemonOption<SGraphObject *> ParseGraphObject(const char *) override { return Empty(); }
    QDemonOption<SNode *> ParseNode(const char *) override { return Empty(); }
};

class IDOMReferenceResolver
{
protected:
    virtual ~IDOMReferenceResolver() {}
public:
    virtual SGraphObject *ResolveReference(SGraphObject &inRootObject, const char *path) = 0;
};

struct SDomReaderPropertyParser : public IPropertyParser
{
    qt3dsdm::IDOMReader &m_Reader;
    QVector<char> &m_TempBuf;
    IDOMReferenceResolver &m_Resolver;
    SGraphObject &m_Object;

    SDomReaderPropertyParser(qt3dsdm::IDOMReader &reader, QVector<char> &inTempBuf,
                             IDOMReferenceResolver &inResolver, SGraphObject &inObject)
        : m_Reader(reader)
        , m_TempBuf(inTempBuf)
        , m_Resolver(inResolver)
        , m_Object(inObject)
    {
    }
    QDemonOption<TStrType> ParseStr(const char *inName) override
    {
        const char *retval;
        if (m_Reader.Att(inName, retval))
            return TStrType(retval);
        return Empty();
    }
    QDemonOption<float> ParseFloat(const char *inName) override
    {
        float retval;
        if (m_Reader.Att(inName, retval))
            return retval;
        return Empty();
    }
    QDemonOption<QVector2D> ParseVec2(const char *inName) override
    {
        qt3dsdm::SFloat2 retval;
        const char *tempData;
        if (m_Reader.UnregisteredAtt(inName, tempData)) {
            qt3dsdm::WStrOps<qt3dsdm::SFloat2>().StrTo(tempData, retval, m_TempBuf);
            return QVector2D(retval.m_Floats[0], retval.m_Floats[1]);
        }
        return Empty();
    }
    QDemonOption<QVector3D> ParseVec3(const char *inName) override
    {
        qt3dsdm::SFloat3 retval;
        const char *tempData;
        if (m_Reader.UnregisteredAtt(inName, tempData)) {
            qt3dsdm::WStrOps<qt3dsdm::SFloat3>().StrTo(tempData, retval, m_TempBuf);
            return QVector3D(retval.m_Floats[0], retval.m_Floats[1], retval.m_Floats[2]);
        }
        return Empty();
    }
    QDemonOption<bool> ParseBool(const char *inName) override
    {
        bool retval;
        if (m_Reader.Att(inName, retval))
            return retval;
        return Empty();
    }

    QDemonOption<quint32> ParseU32(const char *inName) override
    {
        quint32 retval;
        if (m_Reader.Att(inName, retval))
            return retval;
        return Empty();
    }

    QDemonOption<qint32> ParseI32(const char *inName) override
    {
        qint32 retval;
        if (m_Reader.Att(inName, retval))
            return retval;
        return Empty();
    }

    QDemonOption<SGraphObject *> ParseGraphObject(const char *inName) override
    {
        const char *temp;
        if (m_Reader.UnregisteredAtt(inName, temp)) {
            // Now we need to figure out if this is an element reference or if it is a relative path
            // from the current element.
            SGraphObject *retval = m_Resolver.ResolveReference(m_Object, temp);
            if (retval)
                return retval;
        }
        return Empty();
    }

    QDemonOption<SNode *> ParseNode(const char *inName) override
    {
        QDemonOption<SGraphObject *> obj = ParseGraphObject(inName);
        if (obj.hasValue()) {
            if (GraphObjectTypes::IsNodeType((*obj)->m_Type))
                return static_cast<SNode *>((*obj));
        }
        return Empty();
    }
};

template <typename TDataType>
struct SParserHelper
{
};
template <>
struct SParserHelper<TStrType>
{
    static QDemonOption<TStrType> Parse(const char *inName, IPropertyParser &inParser)
    {
        return inParser.ParseStr(inName);
    }
};
template <>
struct SParserHelper<float>
{
    static QDemonOption<float> Parse(const char *inName, IPropertyParser &inParser)
    {
        return inParser.ParseFloat(inName);
    }
};
template <>
struct SParserHelper<QVector2D>
{
    static QDemonOption<QVector2D> Parse(const char *inName, IPropertyParser &inParser)
    {
        return inParser.ParseVec2(inName);
    }
};
template <>
struct SParserHelper<QVector3D>
{
    static QDemonOption<QVector3D> Parse(const char *inName, IPropertyParser &inParser)
    {
        return inParser.ParseVec3(inName);
    }
};
template <>
struct SParserHelper<bool>
{
    static QDemonOption<bool> Parse(const char *inName, IPropertyParser &inParser)
    {
        return inParser.ParseBool(inName);
    }
};
template <>
struct SParserHelper<quint32>
{
    static QDemonOption<quint32> Parse(const char *inName, IPropertyParser &inParser)
    {
        return inParser.ParseU32(inName);
    }
};
template <>
struct SParserHelper<qint32>
{
    static QDemonOption<qint32> Parse(const char *inName, IPropertyParser &inParser)
    {
        return inParser.ParseI32(inName);
    }
};
template <>
struct SParserHelper<SGraphObject *>
{
    static QDemonOption<SGraphObject *> Parse(const char *inName, IPropertyParser &inParser)
    {
        return inParser.ParseGraphObject(inName);
    }
};
template <>
struct SParserHelper<SNode *>
{
    static QDemonOption<SNode *> Parse(const char *inName, IPropertyParser &inParser)
    {
        return inParser.ParseNode(inName);
    }
};

struct SPathAndAnchorIndex
{
    SPathSubPath *m_Segment;
    quint32 m_AnchorIndex;
    SPathAndAnchorIndex(SPathSubPath *inSegment, quint32 inAnchorIndex)
        : m_Segment(inSegment)
        , m_AnchorIndex(inAnchorIndex)
    {
    }
    SPathAndAnchorIndex()
        : m_Segment(nullptr)
        , m_AnchorIndex(0)
    {
    }
};

struct SRenderUIPLoader : public IDOMReferenceResolver
{
    typedef qt3dsdm::IDOMReader::Scope TScope;
    typedef eastl::map<QString, QString> TIdStringMap;
    typedef eastl::hash_map<QString, SPathAndAnchorIndex> TIdPathAnchorIndexMap;
    qt3dsdm::IDOMReader &m_Reader;
    Q3DStudio::IRuntimeMetaData &m_MetaData;
    IStringTable &m_StrTable;
    NVFoundationBase &m_Foundation;
    NVAllocatorCallback &m_PresentationAllocator;
    TIdObjectMap &m_ObjectMap;
    IBufferManager &m_BufferManager;
    SPresentation *m_Presentation;
    QVector<char> m_TempBuf;
    TStrType m_TempParseString;
    IEffectSystem &m_EffectSystem;
    const char *m_PresentationDir;
    CRenderString m_PathString;
    IRenderPluginManager &m_RenderPluginManager;
    ICustomMaterialSystem &m_CustomMaterialSystem;
    IDynamicObjectSystem &m_DynamicObjectSystem;
    IPathManager &m_PathManager;
    TIdStringMap m_RenderPluginSourcePaths;
    IUIPReferenceResolver *m_ReferenceResolver;
    MemoryBuffer<RawAllocator> m_TempBuffer;
    MemoryBuffer<RawAllocator> m_ValueBuffer;
    TIdPathAnchorIndexMap m_AnchorIdToPathAndAnchorIndexMap;

    SRenderUIPLoader(qt3dsdm::IDOMReader &inReader, const char *inFullPathToPresentationFile,
                     Q3DStudio::IRuntimeMetaData &inMetaData, IStringTable &inStrTable
                     // Allocator for datastructures we need to parse the file.
                     ,
                     NVFoundationBase &inFoundation
                     // Allocator used for the presentation objects themselves
                     ,
                     NVAllocatorCallback &inPresentationAllocator
                     // Map of string ids to objects
                     ,
                     TIdObjectMap &ioObjectMap, IBufferManager &inBufferManager,
                     IEffectSystem &inEffectSystem, const char *inPresentationDir,
                     IRenderPluginManager &inRPM,
                     ICustomMaterialSystem &inCMS,
                     IDynamicObjectSystem &inDynamicSystem,
                     IPathManager &inPathManager, IUIPReferenceResolver *inResolver)
        : m_Reader(inReader)
        , m_MetaData(inMetaData)
        , m_StrTable(inStrTable)
        , m_Foundation(inFoundation)
        , m_PresentationAllocator(inPresentationAllocator)
        , m_ObjectMap(ioObjectMap)
        , m_BufferManager(inBufferManager)
        , m_Presentation(QDEMON_NEW(inPresentationAllocator, SPresentation)())
        , m_TempBuf(inFoundation.getAllocator(), "SRenderUIPLoader::m_TempBuf")
        , m_EffectSystem(inEffectSystem)
        , m_PresentationDir(inPresentationDir)
        , m_RenderPluginManager(inRPM)
        , m_CustomMaterialSystem(inCMS)
        , m_DynamicObjectSystem(inDynamicSystem)
        , m_PathManager(inPathManager)
        , m_ReferenceResolver(inResolver)
    {
        std::string presentationFile = inFullPathToPresentationFile;
        std::string::size_type pos = presentationFile.find_last_of("\\/");
        if (pos != std::string::npos) {
            std::string path = presentationFile.substr(0, pos);
            m_Presentation->m_PresentationDirectory = inStrTable.RegisterStr(path.c_str());
        }
    }

    SGraphObject *ResolveReference(SGraphObject &inRoot, const char *path) override
    {
        if (m_ReferenceResolver) {
            QString resolvedReference =
                m_ReferenceResolver->ResolveReference(inRoot.m_Id, path);
            if (resolvedReference.IsValid()) {
                TIdObjectMap::iterator iter = m_ObjectMap.find(resolvedReference);
                if (iter != m_ObjectMap.end())
                    return iter->second;
            }
        }
        return nullptr;
    }

    static bool IsNode(GraphObjectTypes::Enum inType)
    {
        return GraphObjectTypes::IsNodeType(inType);
    }
    template <typename TDataType>
    bool ParseProperty(IPropertyParser &inParser, const char *inName, TDataType &outData)
    {
        QDemonOption<TDataType> theValue(SParserHelper<TDataType>::Parse(inName, inParser));
        if (theValue.hasValue()) {
            outData = theValue;
            return true;
        }
        return false;
    }
    bool ParseOpacityProperty(IPropertyParser &inParser, const char *inName, float &outOpacity)
    {
        if (ParseProperty(inParser, inName, outOpacity)) {
            outOpacity /= 100.0f;
            return true;
        }
        return false;
    }

    bool ParseRadianProperty(IPropertyParser &inParser, const char *inName, QVector3D &ioRotation)
    {
        if (ParseProperty(inParser, inName, ioRotation)) {
            TORAD(ioRotation.x);
            TORAD(ioRotation.y);
            TORAD(ioRotation.z);
            return true;
        }
        return false;
    }
    bool ParseRadianProperty(IPropertyParser &inParser, const char *inName, float &ioRotation)
    {
        if (ParseProperty(inParser, inName, ioRotation)) {
            TORAD(ioRotation);
            return true;
        }
        return false;
    }

    void ParseRotationOrder(IPropertyParser &inParser, const char *inName,
                            quint32 &ioRotationOrder)
    {
        if (ParseProperty(inParser, inName, m_TempParseString))
            ioRotationOrder = MapRotationOrder(m_TempParseString.c_str());
    }
    void ParseOrientation(IPropertyParser &inParser, const char *inName, NodeFlags &ioFlags)
    {
        if (ParseProperty(inParser, inName, m_TempParseString)) {
            if (m_TempParseString == "Left Handed")
                ioFlags.SetLeftHanded(true);
            else
                ioFlags.SetLeftHanded(false);
        }
    }
    void ParseOrthographicProperty(IPropertyParser &inParser, const char *inName,
                                   NodeFlags &ioFlags)
    {
        bool isOrthographic;
        if (ParseProperty(inParser, inName, isOrthographic))
            ioFlags.SetOrthographic(isOrthographic);
    }
    template <typename TEnumType>
    static bool ConvertEnumFromStr(const char *inStr, TEnumType &ioEnum)
    {
        SEnumNameMap *theMap = SEnumParseMap<TEnumType>::GetMap();
        for (SEnumNameMap *item = theMap; item->m_Name; ++item) {
            // hack to match advanced overlay types, whose name start with a '*'
            const char *p = inStr;
            if (*p == '*')
                ++p;
            if (qt3dsdm::AreEqual(p, item->m_Name)) {
                ioEnum = static_cast<TEnumType>(item->m_Enum);
                return true;
            }
        }
        return false;
    }

    template <typename TEnumType>
    void ParseEnumProperty(IPropertyParser &inParser, const char *inName, TEnumType &ioEnum)
    {
        if (ParseProperty(inParser, inName, m_TempParseString)) {
            ConvertEnumFromStr(m_TempParseString.c_str(), ioEnum);
        }
    }
    void ParseAndResolveSourcePath(IPropertyParser &inParser, const char *inName,
                                   QString &ioString)
    {
        if (ParseProperty(inParser, inName, m_TempParseString))
            ioString = m_StrTable.RegisterStr(m_TempParseString.c_str());
    }
    void ParseProperty(IPropertyParser &inParser, const char *inName, SImage *&ioImage)
    {
        if (ParseProperty(inParser, inName, m_TempParseString)) {
            TIdObjectMap::iterator theIter =
                m_ObjectMap.find(m_StrTable.RegisterStr(m_TempParseString.c_str() + 1));
            if (theIter != m_ObjectMap.end()
                && theIter->second->m_Type == GraphObjectTypes::Image) {
                ioImage = static_cast<SImage *>(theIter->second);
            } else {
                Q_ASSERT(false);
            }
        }
    }
    void ParseProperty(IPropertyParser &inParser, const char *inName, QString &ioStr)
    {
        if (ParseProperty(inParser, inName, m_TempParseString))
            ioStr = m_StrTable.RegisterStr(m_TempParseString.c_str());
    }

    void ParseNodeFlagsProperty(IPropertyParser &inParser, const char *inName,
                                NodeFlags &ioFlags,
                                NodeFlagValues::Enum prop)
    {
        bool temp;
        if (ParseProperty(inParser, inName, temp))
            ioFlags.ClearOrSet(temp, prop);
    }

    void ParseNodeFlagsInverseProperty(IPropertyParser &inParser, const char *inName,
                                       NodeFlags &ioFlags,
                                       NodeFlagValues::Enum prop)
    {
        bool temp;
        if (ParseProperty(inParser, inName, temp))
            ioFlags.ClearOrSet(!temp, prop);
    }

// Create a mapping from UICRenderPropertyNames to the string in the UIP file.
#define Scene_ClearColor "backgroundcolor"
#define Scene_UseClearColor "bgcolorenable"
#define Node_Rotation "rotation"
#define Node_Position "position"
#define Node_Scale "scale"
#define Node_Pivot "pivot"
#define Node_LocalOpacity "opacity"
#define Node_RotationOrder "rotationorder"
#define Node_LeftHanded "orientation"
#define Layer_TemporalAAEnabled "temporalaa"
#define Layer_LayerEnableDepthTest "disabledepthtest"
#define Layer_LayerEnableDepthPrePass "disabledepthprepass"
#define Layer_ClearColor "backgroundcolor"
#define Layer_Background "background"
#define Layer_BlendType "blendtype"
#define Layer_Size "size"
#define Layer_Location "location"
#define Layer_TexturePath "sourcepath"
#define Layer_HorizontalFieldValues "horzfields"
#define Layer_Left "left"
#define Layer_LeftUnits "leftunits"
#define Layer_Width "width"
#define Layer_WidthUnits "widthunits"
#define Layer_Right "right"
#define Layer_RightUnits "rightunits"
#define Layer_VerticalFieldValues "vertfields"
#define Layer_Top "top"
#define Layer_TopUnits "topunits"
#define Layer_Height "height"
#define Layer_HeightUnits "heightunits"
#define Layer_Bottom "bottom"
#define Layer_BottomUnits "bottomunits"
#define Layer_AoStrength "aostrength"
#define Layer_AoDistance "aodistance"
#define Layer_AoSoftness "aosoftness"
#define Layer_AoBias "aobias"
#define Layer_AoSamplerate "aosamplerate"
#define Layer_AoDither "aodither"
#define Layer_ShadowStrength "shadowstrength"
#define Layer_ShadowDist "shadowdist"
#define Layer_ShadowSoftness "shadowsoftness"
#define Layer_ShadowBias "shadowbias"
#define Layer_LightProbe "lightprobe"
#define Layer_ProbeBright "probebright"
#define Layer_FastIbl "fastibl"
#define Layer_ProbeHorizon "probehorizon"
#define Layer_ProbeFov "probefov"
#define Layer_LightProbe2 "lightprobe2"
#define Layer_Probe2Fade "probe2fade"
#define Layer_Probe2Window "probe2window"
#define Layer_Probe2Pos "probe2pos"
#define Camera_ClipNear "clipnear"
#define Camera_ClipFar "clipfar"
#define Camera_FOV "fov"
#define Camera_FOVHorizontal "fovhorizontal"
#define Camera_Orthographic "orthographic"
#define Camera_ScaleMode "scalemode"
#define Camera_ScaleAnchor "scaleanchor"
#define Light_LightType "lighttype"
#define Light_DiffuseColor "lightdiffuse"
#define Light_SpecularColor "lightspecular"
#define Light_AmbientColor "lightambient"
#define Light_Brightness "brightness"
#define Light_LinearFade "linearfade"
#define Light_ExponentialFade "expfade"
#define Light_AreaWidth "areawidth"
#define Light_AreaHeight "areaheight"
#define Light_CastShadow "castshadow"
#define Light_ShadowBias "shdwbias"
#define Light_ShadowFactor "shdwfactor"
#define Light_ShadowMapRes "shdwmapres"
#define Light_ShadowMapFar "shdwmapfar"
#define Light_ShadowMapFov "shdwmapfov"
#define Light_ShadowFilter "shdwfilter"
#define Model_MeshPath "sourcepath"
#define Model_TessellationMode "tessellation"
#define Model_EdgeTess "edgetess"
#define Model_InnerTess "innertess"
#define Lightmaps_LightmapIndirect "lightmapindirect"
#define Lightmaps_LightmapRadiosity "lightmapradiosity"
#define Lightmaps_LightmapShadow "lightmapshadow"
#define Material_Lighting "shaderlighting"
#define Material_BlendMode "blendmode"
#define MaterialBase_IblProbe "iblprobe"
#define Material_DiffuseColor "diffuse"
#define Material_DiffuseMaps_0 "diffusemap"
#define Material_DiffuseMaps_1 "diffusemap2"
#define Material_DiffuseMaps_2 "diffusemap3"
#define Material_EmissivePower "emissivepower"
#define Material_EmissiveColor "emissivecolor"
#define Material_EmissiveMap "emissivemap"
#define Material_EmissiveMap2 "emissivemap2"
#define Material_SpecularReflection "specularreflection"
#define Material_SpecularMap "specularmap"
#define Material_SpecularModel "specularmodel"
#define Material_SpecularTint "speculartint"
#define Material_IOR "ior"
#define Material_FresnelPower "fresnelPower"
#define Material_SpecularAmount "specularamount"
#define Material_SpecularRoughness "specularroughness"
#define Material_RoughnessMap "roughnessmap"
#define Material_Opacity "opacity"
#define Material_OpacityMap "opacitymap"
#define Material_BumpMap "bumpmap"
#define Material_BumpAmount "bumpamount"
#define Material_NormalMap "normalmap"
#define Material_DisplacementMap "displacementmap"
#define Material_DisplaceAmount "displaceamount"
#define Material_TranslucencyMap "translucencymap"
#define Material_TranslucentFalloff "translucentfalloff"
#define Material_DiffuseLightWrap "diffuselightwrap"
#define Material_ReferencedMaterial "referencedmaterial"
#define Material_VertexColors "vertexcolors"
#define Image_ImagePath "sourcepath"
#define Image_OffscreenRendererId "subpresentation"
#define Image_Scale_X "scaleu"
#define Image_Scale_Y "scalev"
#define Image_Pivot_X "pivotu"
#define Image_Pivot_Y "pivotv"
#define Image_Rotation "rotationuv"
#define Image_Position_X "positionu"
#define Image_Position_Y "positionv"
#define Image_MappingMode "mappingmode"
#define Image_HorizontalTilingMode "tilingmodehorz"
#define Image_VerticalTilingMode "tilingmodevert"
#define Text_Text "textstring"
#define Text_Font "font"
#define Text_FontSize "size"
#define Text_HorizontalAlignment "horzalign"
#define Text_VerticalAlignment "vertalign"
#define Text_Leading "leading"
#define Text_Tracking "tracking"
#define Text_DropShadow "dropshadow"
#define Text_DropShadowStrength "dropshadowstrength"
#define Text_DropShadowOffset "dropshadowoffset"
#define Text_DropShadowHorizontalAlignment "dropshadowhorzalign"
#define Text_DropShadowVerticalAlignment "dropshadowvertalign"
#define Text_TextColor "textcolor"
#define Text_BackColor "backcolor"
#define Text_EnableAcceleratedFont "enableacceleratedfont"
#define Layer_ProgressiveAAMode "progressiveaa"
#define Layer_MultisampleAAMode "multisampleaa"
#define Light_Scope "scope"
#define Path_PathType "pathtype"
#define Path_PaintStyle "paintstyle"
#define Path_Width "width"
#define Path_Opacity "opacity"
#define Path_LinearError "linearerror"
#define Path_EdgeTessAmount "edgetessamount"
#define Path_InnerTessAmount "innertessamount"
#define Path_BeginCapping "begincap"
#define Path_BeginCapOffset "begincapoffset"
#define Path_BeginCapOpacity "begincapopacity"
#define Path_BeginCapWidth "begincapwidth"
#define Path_EndCapping "endcap"
#define Path_EndCapOffset "endcapoffset"
#define Path_EndCapOpacity "endcapopacity"
#define Path_EndCapWidth "endcapwidth"
#define Path_PathBuffer "sourcepath"
#define SubPath_Closed "closed"

// Fill in implementations for the actual parse tables.
#define HANDLE_QDEMON_RENDER_PROPERTY(type, name, dirty)                                              \
    ParseProperty(inParser, type##_##name, inItem.m_##name);
#define HANDLE_QDEMON_RENDER_REAL_VEC2_PROPERTY(type, name, dirty)                                    \
    ParseProperty(inParser, type##_##name, inItem.m_##name);
#define HANDLE_QDEMON_RENDER_VEC3_PROPERTY(type, name, dirty)                                         \
    ParseProperty(inParser, type##_##name, inItem.m_##name);
#define HANDLE_QDEMON_RENDER_COLOR_PROPERTY(type, name, dirty)                                        \
    ParseProperty(inParser, type##_##name, inItem.m_##name);
#define HANDLE_QDEMON_RENDER_RADIAN_PROPERTY(type, name, dirty)                                       \
    ParseRadianProperty(inParser, type##_##name, inItem.m_##name);
#define HANDLE_QDEMON_RENDER_VEC3_RADIAN_PROPERTY(type, name, dirty)                                  \
    ParseRadianProperty(inParser, type##_##name, inItem.m_##name);
#define HANDLE_QDEMON_RENDER_OPACITY_PROPERTY(type, name, dirty)                                      \
    ParseOpacityProperty(inParser, type##_##name, inItem.m_##name);
#define HANDLE_QDEMON_ROTATION_ORDER_PROPERTY(type, name, dirty)                                      \
    ParseRotationOrder(inParser, type##_##name, inItem.m_##name);
#define HANDLE_QDEMON_NODE_ORIENTATION_PROPERTY(type, name, dirty)                                    \
    ParseOrientation(inParser, type##_##name, inItem.m_Flags);
#define HANDLE_QDEMON_RENDER_DEPTH_TEST_PROPERTY(type, name, dirty)                                   \
    if (ParseProperty(inParser, type##_##name, inItem.m_##name))                                   \
        inItem.m_##name = !inItem.m_##name;
#define HANDLE_QDEMON_NODE_FLAGS_PROPERTY(type, name, dirty)                                          \
    ParseNodeFlagsProperty(inParser, type##_##name, inItem.m_Flags,                                \
                           NodeFlagValues::name);
#define HANDLE_QDEMON_NODE_FLAGS_INVERSE_PROPERTY(type, name, dirty)                                  \
    ParseNodeFlagsInverseProperty(inParser, type##_##name, inItem.m_Flags,                         \
                                  NodeFlagValues::name);
#define HANDLE_QDEMON_RENDER_ENUM_PROPERTY(type, name, dirty)                                         \
    ParseEnumProperty(inParser, type##_##name, inItem.m_##name);
#define HANDLE_QDEMON_RENDER_SOURCEPATH_PROPERTY(type, name, dirty)                                   \
    ParseAndResolveSourcePath(inParser, type##_##name, inItem.m_##name);
#define HANDLE_QDEMON_RENDER_ARRAY_PROPERTY(type, name, index, dirty)                                 \
    ParseProperty(inParser, type##_##name##_##index, inItem.m_##name[index]);
#define HANDLE_QDEMON_RENDER_VEC2_PROPERTY(type, name, dirty)                                         \
    ParseProperty(inParser, type##_##name##_##X, inItem.m_##name.x);                               \
    ParseProperty(inParser, type##_##name##_##Y, inItem.m_##name.y);
#define HANDLE_QDEMON_RENDER_COLOR_VEC3_PROPERTY(                                                     \
    type, name, dirty) // noop by intention already handled by HANDLE_QDEMON_RENDER_COLOR_PROPERTY
#define HANDLE_QDEMON_RENDER_TRANSFORM_VEC3_PROPERTY(                                                 \
    type, name, dirty) // noop by intention already handled by HANDLE_QDEMON_RENDER_VEC3_PROPERTY

    // Call the correct parser functions.
    void ParseProperties(SScene &inItem, IPropertyParser &inParser)
    {
        ITERATE_QDEMON_RENDER_SCENE_PROPERTIES
    }
    void ParseProperties(SNode &inItem, IPropertyParser &inParser)
    {
        bool eyeball;
        if (ParseProperty(inParser, "eyeball", eyeball))
            inItem.m_Flags.SetActive(eyeball);
        ITERATE_QDEMON_RENDER_NODE_PROPERTIES
        ParseProperty(inParser, "boneid", inItem.m_SkeletonId);
        bool ignoreParent = false;
        if (ParseProperty(inParser, "ignoresparent", ignoreParent))
            inItem.m_Flags.SetIgnoreParentTransform(ignoreParent);
    }
    void ParseProperties(SLayer &inItem, IPropertyParser &inParser)
    {
        ParseProperties(static_cast<SNode &>(inItem), inParser);
        ITERATE_QDEMON_RENDER_LAYER_PROPERTIES
        ParseProperty(inParser, "aosamplerate", inItem.m_AoSamplerate);
    }
    void ParseProperties(SCamera &inItem, IPropertyParser &inParser)
    {
        ParseProperties(static_cast<SNode &>(inItem), inParser);
        ITERATE_QDEMON_RENDER_CAMERA_PROPERTIES
    }
    void ParseProperties(SLight &inItem, IPropertyParser &inParser)
    {
        ParseProperties(static_cast<SNode &>(inItem), inParser);
        ITERATE_QDEMON_RENDER_LIGHT_PROPERTIES
        ParseProperty(inParser, "shdwmapres", inItem.m_ShadowMapRes);
    }
    void ParseProperties(SModel &inItem, IPropertyParser &inParser)
    {
        ParseProperties(static_cast<SNode &>(inItem), inParser);
        ITERATE_QDEMON_RENDER_MODEL_PROPERTIES
        ParseProperty(inParser, "poseroot", inItem.m_SkeletonRoot);
    }

    void ParseProperties(SText &inItem, IPropertyParser &inParser)
    {
        ParseProperties(static_cast<SNode &>(inItem), inParser);
        ITERATE_QDEMON_RENDER_TEXT_PROPERTIES
    }
    void ParseProperties(SLightmaps &inItem, IPropertyParser &inParser)
    {
        ITERATE_QDEMON_RENDER_LIGHTMAP_PROPERTIES
    }
    void ParseProperties(SDefaultMaterial &inItem, IPropertyParser &inParser)
    {
        ITERATE_QDEMON_RENDER_MATERIAL_PROPERTIES
        ParseProperties(inItem.m_Lightmaps, inParser);
    }
    void ParseProperties(SReferencedMaterial &inItem, IPropertyParser &inParser)
    {
        ITERATE_QDEMON_RENDER_REFERENCED_MATERIAL_PROPERTIES
        // Propagate lightmaps
        if (inItem.m_ReferencedMaterial
            && inItem.m_ReferencedMaterial->m_Type == GraphObjectTypes::DefaultMaterial)
            ParseProperties(
                static_cast<SDefaultMaterial *>(inItem.m_ReferencedMaterial)->m_Lightmaps,
                inParser);
        else if (inItem.m_ReferencedMaterial
                 && inItem.m_ReferencedMaterial->m_Type == GraphObjectTypes::CustomMaterial)
            ParseProperties(
                static_cast<SCustomMaterial *>(inItem.m_ReferencedMaterial)->m_Lightmaps, inParser);
    }
    void ParseProperties(SImage &inItem, IPropertyParser &inParser)
    {
        ITERATE_QDEMON_RENDER_IMAGE_PROPERTIES
    }
    template <typename TDataType>
    void SetDynamicObjectProperty(SDynamicObject &inEffect,
                                  const dynamic::SPropertyDefinition &inPropDesc,
                                  const TDataType &inProp)
    {
        ::memcpy(inEffect.GetDataSectionBegin() + inPropDesc.m_Offset, &inProp, sizeof(TDataType));
    }
    template <typename TDataType>
    void SetDynamicObjectProperty(SDynamicObject &inEffect,
                                  const dynamic::SPropertyDefinition &inPropDesc,
                                  QDemonOption<TDataType> inProp)
    {
        if (inProp.hasValue()) {
            SetDynamicObjectProperty(inEffect, inPropDesc, *inProp);
        }
    }
    void ParseProperties(SCustomMaterial &inItem, IPropertyParser &inParser)
    {
        ParseProperties(static_cast<SDynamicObject &>(inItem), inParser);
        ParseProperties(inItem.m_Lightmaps, inParser);
        ITERATE_QDEMON_RENDER_CUSTOM_MATERIAL_PROPERTIES
    }
    void ParseProperties(SDynamicObject &inDynamicObject, IPropertyParser &inParser)
    {
        QDemonConstDataRef<dynamic::SPropertyDefinition> theProperties =
            m_DynamicObjectSystem.GetProperties(inDynamicObject.m_ClassName);

        for (quint32 idx = 0, end = theProperties.size(); idx < end; ++idx) {
            const dynamic::SPropertyDefinition &theDefinition(theProperties[idx]);
            switch (theDefinition.m_DataType) {
            case QDemonRenderShaderDataTypes::bool:
                SetDynamicObjectProperty(inDynamicObject, theDefinition,
                                         inParser.ParseBool(theDefinition.m_Name));
                break;
            case QDemonRenderShaderDataTypes::float:
                SetDynamicObjectProperty(inDynamicObject, theDefinition,
                                         inParser.ParseFloat(theDefinition.m_Name));
                break;
            case QDemonRenderShaderDataTypes::qint32:
                if (theDefinition.m_IsEnumProperty == false)
                    SetDynamicObjectProperty(inDynamicObject, theDefinition,
                                             inParser.ParseU32(theDefinition.m_Name));
                else {
                    QDemonOption<QString> theEnum = inParser.ParseStr(theDefinition.m_Name);
                    if (theEnum.hasValue()) {
                        QDemonConstDataRef<QString> theEnumNames =
                            theDefinition.m_EnumValueNames;
                        for (quint32 idx = 0, end = theEnumNames.size(); idx < end; ++idx) {
                            if (theEnum->compare(theEnumNames[idx].c_str()) == 0) {
                                SetDynamicObjectProperty(inDynamicObject, theDefinition, idx);
                                break;
                            }
                        }
                    }
                }
                break;
            case QDemonRenderShaderDataTypes::QVector3D:
                SetDynamicObjectProperty(inDynamicObject, theDefinition,
                                         inParser.ParseVec3(theDefinition.m_Name));
                break;
            case QDemonRenderShaderDataTypes::QVector2D:
                SetDynamicObjectProperty(inDynamicObject, theDefinition,
                                         inParser.ParseVec2(theDefinition.m_Name));
                break;
            case QDemonRenderShaderDataTypes::QDemonRenderTexture2DPtr:
            case QDemonRenderShaderDataTypes::QDemonRenderImage2DPtr: {
                QDemonOption<QString> theTexture = inParser.ParseStr(theDefinition.m_Name);
                if (theTexture.hasValue()) {
                    QString theStr;
                    if (theTexture->size())
                        theStr = m_StrTable.RegisterStr(theTexture->c_str());

                    SetDynamicObjectProperty(inDynamicObject, theDefinition, theStr);
                }
            } break;
            case QDemonRenderShaderDataTypes::QDemonRenderDataBufferPtr:
                break;
            default:
                Q_ASSERT(false);
                break;
            }
        }
    }
    void ParseProperties(SPath &inItem, IPropertyParser &inParser)
    {
        ParseProperties(static_cast<SNode &>(inItem), inParser);
        ITERATE_QDEMON_RENDER_PATH_PROPERTIES
    }
    void ParseProperties(SPathSubPath &inItem, IPropertyParser &inParser)
    {
        ITERATE_QDEMON_RENDER_PATH_SUBPATH_PROPERTIES
    }

    void AddPluginPropertyUpdate(eastl::vector<SRenderPropertyValueUpdate> &ioUpdates,
                                 IRenderPluginClass &,
                                 const SRenderPluginPropertyDeclaration &inDeclaration,
                                 QDemonOption<float> data)
    {
        if (data.hasValue()) {
            ioUpdates.push_back(
                SRenderPropertyValueUpdate(inDeclaration.m_Name, *data));
        }
    }
    void AddPluginPropertyUpdate(eastl::vector<SRenderPropertyValueUpdate> &ioUpdates,
                                 IRenderPluginClass &inClass,
                                 const SRenderPluginPropertyDeclaration &inDeclaration,
                                 QDemonOption<QVector2D> data)
    {
        if (data.hasValue()) {
            ioUpdates.push_back(SRenderPropertyValueUpdate(
                inClass.GetPropertyValueInfo(inDeclaration.m_StartOffset).first, data->x));
            ioUpdates.push_back(SRenderPropertyValueUpdate(
                inClass.GetPropertyValueInfo(inDeclaration.m_StartOffset + 1).first, data->y));
        }
    }
    void AddPluginPropertyUpdate(eastl::vector<SRenderPropertyValueUpdate> &ioUpdates,
                                 IRenderPluginClass &inClass,
                                 const SRenderPluginPropertyDeclaration &inDeclaration,
                                 QDemonOption<QVector3D> data)
    {
        if (data.hasValue()) {
            ioUpdates.push_back(SRenderPropertyValueUpdate(
                inClass.GetPropertyValueInfo(inDeclaration.m_StartOffset).first, data->x));
            ioUpdates.push_back(SRenderPropertyValueUpdate(
                inClass.GetPropertyValueInfo(inDeclaration.m_StartOffset + 1).first, data->y));
            ioUpdates.push_back(SRenderPropertyValueUpdate(
                inClass.GetPropertyValueInfo(inDeclaration.m_StartOffset + 2).first, data->z));
        }
    }
    void AddPluginPropertyUpdate(eastl::vector<SRenderPropertyValueUpdate> &ioUpdates,
                                 IRenderPluginClass &,
                                 const SRenderPluginPropertyDeclaration &inDeclaration,
                                 QDemonOption<qint32> dataOpt)
    {
        if (dataOpt.hasValue()) {
            long data = static_cast<long>(*dataOpt);
            ioUpdates.push_back(
                SRenderPropertyValueUpdate(inDeclaration.m_Name, (qint32)data));
        }
    }
    void AddPluginPropertyUpdate(eastl::vector<SRenderPropertyValueUpdate> &ioUpdates,
                                 IRenderPluginClass &,
                                 const SRenderPluginPropertyDeclaration &inDeclaration,
                                 QDemonOption<QString> dataOpt)
    {
        if (dataOpt.hasValue()) {
            QString &data = dataOpt.getValue();
            ioUpdates.push_back(SRenderPropertyValueUpdate(
                inDeclaration.m_Name, m_StrTable.RegisterStr(data.c_str())));
        }
    }
    void AddPluginPropertyUpdate(eastl::vector<SRenderPropertyValueUpdate> &ioUpdates,
                                 IRenderPluginClass &,
                                 const SRenderPluginPropertyDeclaration &inDeclaration,
                                 QDemonOption<bool> dataOpt)
    {
        if (dataOpt.hasValue()) {
            bool &data = dataOpt.getValue();
            ioUpdates.push_back(
                SRenderPropertyValueUpdate(inDeclaration.m_Name, data));
        }
    }
    void ParseProperties(SRenderPlugin &inRenderPlugin, IPropertyParser &inParser)
    {
        IRenderPluginClass *theClass =
            m_RenderPluginManager.GetRenderPlugin(inRenderPlugin.m_PluginPath);
        if (theClass) {
            QDemonConstDataRef<SRenderPluginPropertyDeclaration>
                theClassProps = theClass->GetRegisteredProperties();
            if (theClassProps.size()) {
                IRenderPluginInstance *theInstance =
                    m_RenderPluginManager.GetOrCreateRenderPluginInstance(
                        inRenderPlugin.m_PluginPath, &inRenderPlugin);
                if (theInstance) {
                    eastl::vector<SRenderPropertyValueUpdate> theUpdates;
                    for (quint32 idx = 0, end = theClassProps.size(); idx < end; ++idx) {
                        const SRenderPluginPropertyDeclaration &theDec(
                            theClassProps[idx]);
                        QString tempStr;
                        switch (theDec.m_Type) {
                        case SRenderPluginPropertyTypes::Float:
                            AddPluginPropertyUpdate(theUpdates, *theClass, theDec,
                                                    inParser.ParseFloat(theDec.m_Name.c_str()));
                            break;
                        case SRenderPluginPropertyTypes::Vector2:
                            AddPluginPropertyUpdate(theUpdates, *theClass, theDec,
                                                    inParser.ParseVec2(theDec.m_Name.c_str()));
                            break;
                        case SRenderPluginPropertyTypes::Color:
                        case SRenderPluginPropertyTypes::Vector3:
                            AddPluginPropertyUpdate(theUpdates, *theClass, theDec,
                                                    inParser.ParseVec3(theDec.m_Name.c_str()));
                            break;
                        case SRenderPluginPropertyTypes::Long:
                            AddPluginPropertyUpdate(theUpdates, *theClass, theDec,
                                                    inParser.ParseI32(theDec.m_Name.c_str()));
                            break;
                        case SRenderPluginPropertyTypes::String:
                            AddPluginPropertyUpdate(theUpdates, *theClass, theDec,
                                                    inParser.ParseStr(theDec.m_Name.c_str()));
                            break;
                        case SRenderPluginPropertyTypes::Boolean:
                            AddPluginPropertyUpdate(theUpdates, *theClass, theDec,
                                                    inParser.ParseBool(theDec.m_Name.c_str()));
                            break;
                        default:
                            Q_ASSERT(false);
                        }
                    }
                    theInstance->Update(
                        toConstDataRef(theUpdates.data(), theUpdates.size()));
                }
            }
        }
    }

#undef HANDLE_QDEMON_RENDER_PROPERTY
#undef HANDLE_QDEMON_RENDER_ENUM_PROPERTY
#undef HANDLE_QDEMON_RENDER_RADIAN_PROPERTY
#undef HANDLE_QDEMON_RENDER_SOURCEPATH_PROPERTY
#undef HANDLE_QDEMON_RENDER_ARRAY_PROPERTY
#undef HANDLE_QDEMON_NODE_FLAGS_PROPERTY
#undef HANDLE_QDEMON_ROTATION_ORDER_PROPERTY
#undef HANDLE_QDEMON_RENDER_OPACITY_PROPERTY
#undef HANDLE_QDEMON_NODE_ORIENTATION_PROPERTY
#undef HANDLE_QDEMON_RENDER_DEPTH_TEST_PROPERTY
#undef HANDLE_QDEMON_RENDER_VEC2_PROPERTY

    void ParseGraphPass1(SGraphObject *inParent)
    {
        TScope __elemScope(m_Reader);
        qt3dsdm::ComposerObjectTypes::Enum theObjType =
            qt3dsdm::ComposerObjectTypes::Convert(m_Reader.GetElementName());
        SGraphObject *theNewObject(nullptr);
        const char *theId;
        m_Reader.Att("id", theId);

        switch (theObjType) {
        case qt3dsdm::ComposerObjectTypes::Scene: {
            SScene *theScene = QDEMON_NEW(m_PresentationAllocator, SScene)();
            theNewObject = theScene;
            m_Presentation->m_Scene = theScene;
            theScene->m_Presentation = m_Presentation;
        } break;
        case qt3dsdm::ComposerObjectTypes::Layer:
            theNewObject = QDEMON_NEW(m_PresentationAllocator, SLayer)();
            break;
        case qt3dsdm::ComposerObjectTypes::Group:
            theNewObject = QDEMON_NEW(m_PresentationAllocator, SNode)();
            break;
        case qt3dsdm::ComposerObjectTypes::Component:
            theNewObject = QDEMON_NEW(m_PresentationAllocator, SNode)();
            break;
        case qt3dsdm::ComposerObjectTypes::Camera:
            theNewObject = QDEMON_NEW(m_PresentationAllocator, SCamera)();
            break;
        case qt3dsdm::ComposerObjectTypes::Light:
            theNewObject = QDEMON_NEW(m_PresentationAllocator, SLight)();
            break;
        case qt3dsdm::ComposerObjectTypes::Model:
            theNewObject = QDEMON_NEW(m_PresentationAllocator, SModel)();
            break;
        case qt3dsdm::ComposerObjectTypes::Material:
            theNewObject = QDEMON_NEW(m_PresentationAllocator, SDefaultMaterial)();
            break;
        case qt3dsdm::ComposerObjectTypes::ReferencedMaterial:
            theNewObject = QDEMON_NEW(m_PresentationAllocator, SReferencedMaterial)();
            break;
        case qt3dsdm::ComposerObjectTypes::Image:
            theNewObject = QDEMON_NEW(m_PresentationAllocator, SImage)();
            break;
        case qt3dsdm::ComposerObjectTypes::Text:
            theNewObject = QDEMON_NEW(m_PresentationAllocator, SText)();
            break;
        case qt3dsdm::ComposerObjectTypes::Path:
            theNewObject = QDEMON_NEW(m_PresentationAllocator, SPath)();
            break;
        case qt3dsdm::ComposerObjectTypes::SubPath: {
            SPathSubPath *thePath = QDEMON_NEW(m_PresentationAllocator, SPathSubPath)();
            theNewObject = thePath;
            quint32 anchorCount = 0;
            TScope _childScope(m_Reader);
            for (bool success = m_Reader.MoveToFirstChild("PathAnchorPoint"); success;
                 success = m_Reader.MoveToNextSibling("PathAnchorPoint")) {
                const char *theId;
                m_Reader.Att("id", theId);
                QString theIdStr = m_StrTable.RegisterStr(theId);
                m_AnchorIdToPathAndAnchorIndexMap.insert(
                    eastl::make_pair(theIdStr, SPathAndAnchorIndex(thePath, anchorCount)));
                ++anchorCount;
            }
            m_PathManager.ResizePathSubPathBuffer(*thePath, anchorCount);
        } break;
        case qt3dsdm::ComposerObjectTypes::Effect: {
            const char *effectClassId;
            m_Reader.Att("class", effectClassId);
            QString theStr = m_StrTable.RegisterStr(effectClassId + 1);
            if (m_EffectSystem.IsEffectRegistered(theStr))
                theNewObject = m_EffectSystem.CreateEffectInstance(theStr, m_PresentationAllocator);
        } break;
        case qt3dsdm::ComposerObjectTypes::RenderPlugin: {
            const char *classId;
            m_Reader.Att("class", classId);
            if (!isTrivial(classId)) {
                ++classId;
                TIdStringMap::iterator iter =
                    m_RenderPluginSourcePaths.find(m_StrTable.RegisterStr(classId));
                if (iter != m_RenderPluginSourcePaths.end()) {
                    QString thePluginPath = m_StrTable.RegisterStr(iter->second.c_str());
                    IRenderPluginClass *theClass =
                        m_RenderPluginManager.GetRenderPlugin(thePluginPath);
                    if (theClass) {
                        SRenderPlugin *thePlugin =
                            QDEMON_NEW(m_PresentationAllocator, SRenderPlugin)();
                        thePlugin->m_PluginPath = thePluginPath;
                        thePlugin->m_Flags.SetActive(true);
                        theNewObject = thePlugin;
                    }
                }
            }
        } break;
        case qt3dsdm::ComposerObjectTypes::CustomMaterial: {
            const char *materialClassId;
            m_Reader.Att("class", materialClassId);
            QString theStr = m_StrTable.RegisterStr(materialClassId + 1);
            if (m_CustomMaterialSystem.IsMaterialRegistered(theStr))
                theNewObject =
                    m_CustomMaterialSystem.CreateCustomMaterial(theStr, m_PresentationAllocator);
        } break;
        default:
            // Ignoring unknown objects entirely at this point
            break;
        }
        if (theNewObject) {
            QString theObjectId(m_StrTable.RegisterStr(theId));
            m_ObjectMap.insert(eastl::make_pair(theObjectId, theNewObject));
            theNewObject->m_Id = theObjectId;
            // setup hierarchy
            bool isParentNode;
            bool isChildNode;
            if (inParent) {
                switch (inParent->m_Type) {
                case GraphObjectTypes::Scene:
                    if (theNewObject->m_Type == GraphObjectTypes::Layer) {
                        static_cast<SScene *>(inParent)->AddChild(
                            *static_cast<SLayer *>(theNewObject));
                    } else {
                        // Something added to a scene that was not a layer.
                        Q_ASSERT(false);
                    }
                    break;

                case GraphObjectTypes::DefaultMaterial:
                    if (theNewObject->m_Type != GraphObjectTypes::Image) {
                        // Something added to a material that is not an image...
                        // how odd.
                        Q_ASSERT(false);
                    } else {
                        static_cast<SImage *>(theNewObject)->m_Parent =
                            static_cast<SDefaultMaterial *>(inParent);
                        QString thePath = QString(theNewObject->m_Id.c_str());
                        if (thePath.find("probe") != QString::npos)
                            static_cast<SImage *>(theNewObject)->m_MappingMode =
                                ImageMappingModes::LightProbe;
                    }
                    break;

                case GraphObjectTypes::CustomMaterial:
                    if (theNewObject->m_Type == GraphObjectTypes::Image) {
                        static_cast<SImage *>(theNewObject)->m_Parent =
                            static_cast<SCustomMaterial *>(inParent);
                        QString thePath = QString(theNewObject->m_Id.c_str());
                        if (thePath.find("probe") != QString::npos) {
                            static_cast<SImage *>(theNewObject)->m_MappingMode =
                                ImageMappingModes::LightProbe;
                        }
                    } else {
                        Q_ASSERT(false);
                    }
                    break;
                case GraphObjectTypes::ReferencedMaterial:
                    if (theNewObject->m_Type == GraphObjectTypes::Image) {
                        // nothing to do yet
                    } else {
                        Q_ASSERT(false);
                    }
                    break;
                case GraphObjectTypes::Path:

                    if (GraphObjectTypes::IsMaterialType(theNewObject->m_Type))
                        static_cast<SPath *>(inParent)->AddMaterial(theNewObject);

                    else if (theNewObject->m_Type == GraphObjectTypes::PathSubPath)
                        static_cast<SPath *>(inParent)->AddSubPath(
                            *static_cast<SPathSubPath *>(theNewObject));

                    break;

                default:
                    isParentNode = IsNode(inParent->m_Type);
                    isChildNode = IsNode(theNewObject->m_Type);
                    if (isParentNode && isChildNode) {
                        static_cast<SNode *>(inParent)->AddChild(
                            *static_cast<SNode *>(theNewObject));
                    } else if (isParentNode) {
                        if (inParent->m_Type == GraphObjectTypes::Model
                            && IsMaterial(theNewObject)) {
                            static_cast<SModel *>(inParent)->AddMaterial(*theNewObject);
                        } else {
                            if (inParent->m_Type == GraphObjectTypes::Layer
                                && theNewObject->m_Type == GraphObjectTypes::Effect) {
                                static_cast<SLayer *>(inParent)->AddEffect(
                                    *static_cast<SEffect *>(theNewObject));
                            } else if (inParent->m_Type == GraphObjectTypes::Layer
                                       && theNewObject->m_Type == GraphObjectTypes::Image) {
                                QString thePath = QString(theNewObject->m_Id.c_str());
                                if (thePath.find("probe2") != QString::npos) {
                                    static_cast<SLayer *>(inParent)->m_LightProbe2 =
                                        static_cast<SImage *>(theNewObject);
                                } else {
                                    static_cast<SLayer *>(inParent)->m_LightProbe =
                                        static_cast<SImage *>(theNewObject);
                                }
                            } else {
                                if (theNewObject->m_Type == GraphObjectTypes::RenderPlugin) {
                                    SRenderPlugin *childObj =
                                        static_cast<SRenderPlugin *>(theNewObject);
                                    if (inParent->m_Type == GraphObjectTypes::Layer) {
                                        static_cast<SLayer *>(inParent)->m_RenderPlugin = childObj;
                                    } else {
                                        Q_ASSERT(false);
                                    }
                                } else {
                                    Q_ASSERT(false);
                                }
                            }
                        }
                    } else {
                        if (inParent->m_Type == GraphObjectTypes::Image
                            && theNewObject->m_Type == GraphObjectTypes::RenderPlugin) {
                            static_cast<SImage *>(inParent)->m_RenderPlugin =
                                static_cast<SRenderPlugin *>(theNewObject);
                        } else {
                            Q_ASSERT(false);
                        }
                    }
                }
            }
            for (bool valid = m_Reader.MoveToFirstChild(); valid;
                 valid = m_Reader.MoveToNextSibling())
                ParseGraphPass1(theNewObject);
        } else {
            for (bool valid = m_Reader.MoveToFirstChild(); valid;
                 valid = m_Reader.MoveToNextSibling())
                ParseGraphPass1(nullptr);
        }
    }

    template <typename TObjType>
    void ParsePass2Properties(TObjType &inObject, const char *inClassId)
    {
        const char *theTypeName = m_Reader.GetNarrowElementName();
        SMetaPropertyParser theMetaParser(theTypeName, inClassId, m_MetaData);
        // Set default values
        ParseProperties(inObject, theMetaParser);

        // Now setup property values from the element itself.
        SDomReaderPropertyParser theReaderParser(m_Reader, m_TempBuf, *this, inObject);
        ParseProperties(inObject, theReaderParser);
    }

    // Parse the instance properties from the graph.
    void ParseGraphPass2()
    {
        TScope __instanceScope(m_Reader);
        const char *theId;
        m_Reader.Att("id", theId);
        const char *theClass = "";
        m_Reader.Att("class", theClass);
        TIdObjectMap::iterator theObject = m_ObjectMap.find(m_StrTable.RegisterStr(theId));
        if (theObject != m_ObjectMap.end()) {
            switch (theObject->second->m_Type) {
            case GraphObjectTypes::Scene:
                ParsePass2Properties(*static_cast<SScene *>(theObject->second), theClass);
                break;
            case GraphObjectTypes::Node:
                ParsePass2Properties(*static_cast<SNode *>(theObject->second), theClass);
                break;
            case GraphObjectTypes::Layer:
                ParsePass2Properties(*static_cast<SLayer *>(theObject->second), theClass);
                break;
            case GraphObjectTypes::Camera:
                ParsePass2Properties(*static_cast<SCamera *>(theObject->second), theClass);
                break;
            case GraphObjectTypes::Light:
                ParsePass2Properties(*static_cast<SLight *>(theObject->second), theClass);
                break;
            case GraphObjectTypes::Model:
                ParsePass2Properties(*static_cast<SModel *>(theObject->second), theClass);
                break;
            case GraphObjectTypes::DefaultMaterial:
                ParsePass2Properties(*static_cast<SDefaultMaterial *>(theObject->second), theClass);
                break;
            case GraphObjectTypes::ReferencedMaterial:
                ParsePass2Properties(*static_cast<SReferencedMaterial *>(theObject->second),
                                     theClass);
                break;
            case GraphObjectTypes::Image:
                ParsePass2Properties(*static_cast<SImage *>(theObject->second), theClass);
                break;
            case GraphObjectTypes::Text:
                ParsePass2Properties(*static_cast<SText *>(theObject->second), theClass);
                break;
            case GraphObjectTypes::Effect:
                ParsePass2Properties(*static_cast<SEffect *>(theObject->second), theClass);
                break;
            case GraphObjectTypes::RenderPlugin:
                ParsePass2Properties(*static_cast<SRenderPlugin *>(theObject->second),
                                     theClass);
                break;
            case GraphObjectTypes::CustomMaterial:
                ParsePass2Properties(*static_cast<SCustomMaterial *>(theObject->second), theClass);
                break;
            case GraphObjectTypes::Path:
                ParsePass2Properties(*static_cast<SPath *>(theObject->second), theClass);
                break;
            case GraphObjectTypes::PathSubPath:
                ParsePass2Properties(*static_cast<SPathSubPath *>(theObject->second), theClass);
                break;
            default:
                Q_ASSERT(false);
                break;
            }
        }
        for (bool valid = m_Reader.MoveToFirstChild(); valid; valid = m_Reader.MoveToNextSibling())
            ParseGraphPass2();
    }

    static bool ParseVec2(SDomReaderPropertyParser &inParser, const char *inName, QVector2D &outValue)
    {
        QDemonOption<QVector2D> result = inParser.ParseVec2(inName);

        if (result.hasValue())
            outValue = *result;

        return result.hasValue();
    }

    static bool ParseFloat(SDomReaderPropertyParser &inParser, const char *inName, float &outValue)
    {
        QDemonOption<float> result = inParser.ParseFloat(inName);
        if (result.hasValue())
            outValue = *result;
        return result.hasValue();
    }

    void ParseState(bool inSetSetValues)
    {
        TScope __slideScope(m_Reader);
        for (bool valid = m_Reader.MoveToFirstChild(); valid;
             valid = m_Reader.MoveToNextSibling()) {
            if (strcmp(m_Reader.GetNarrowElementName(), "Add") == 0
                || (inSetSetValues && strcmp(m_Reader.GetNarrowElementName(), "Set") == 0)) {
                const char *theId;
                m_Reader.Att("ref", theId);
                QString theIdStr(m_StrTable.RegisterStr(theId + 1));
                TIdObjectMap::iterator theObject = m_ObjectMap.find(theIdStr);
                if (theObject != m_ObjectMap.end()) {
                    SDomReaderPropertyParser parser(m_Reader, m_TempBuf, *this, *theObject->second);
                    switch (theObject->second->m_Type) {
                    case GraphObjectTypes::Scene:
                        ParseProperties(*reinterpret_cast<SScene *>(theObject->second), parser);
                        break;
                    case GraphObjectTypes::Node:
                        ParseProperties(*reinterpret_cast<SNode *>(theObject->second), parser);
                        break;
                    case GraphObjectTypes::Layer:
                        ParseProperties(*reinterpret_cast<SLayer *>(theObject->second), parser);
                        break;
                    case GraphObjectTypes::Camera:
                        ParseProperties(*reinterpret_cast<SCamera *>(theObject->second), parser);
                        break;
                    case GraphObjectTypes::Light:
                        ParseProperties(*reinterpret_cast<SLight *>(theObject->second), parser);
                        break;
                    case GraphObjectTypes::Model:
                        ParseProperties(*reinterpret_cast<SModel *>(theObject->second), parser);
                        break;
                    case GraphObjectTypes::DefaultMaterial:
                        ParseProperties(*reinterpret_cast<SDefaultMaterial *>(theObject->second),
                                        parser);
                        break;
                    case GraphObjectTypes::ReferencedMaterial:
                        ParseProperties(*static_cast<SReferencedMaterial *>(theObject->second),
                                        parser);
                        break;
                    case GraphObjectTypes::Image:
                        ParseProperties(*reinterpret_cast<SImage *>(theObject->second), parser);
                        break;
                    case GraphObjectTypes::Text:
                        ParseProperties(*static_cast<SText *>(theObject->second), parser);
                        break;
                    case GraphObjectTypes::Effect:
                        ParseProperties(*static_cast<SEffect *>(theObject->second), parser);
                        break;
                    case GraphObjectTypes::RenderPlugin:
                        ParseProperties(
                            *static_cast<SRenderPlugin *>(theObject->second), parser);
                        break;
                    case GraphObjectTypes::CustomMaterial:
                        ParseProperties(
                            *static_cast<SCustomMaterial *>(theObject->second),
                            parser);
                        break;
                    case GraphObjectTypes::Path:
                        ParseProperties(*static_cast<SPath *>(theObject->second),
                                        parser);
                        break;
                    case GraphObjectTypes::PathSubPath:
                        ParseProperties(
                            *static_cast<SPathSubPath *>(theObject->second), parser);
                        break;
                    default:
                        Q_ASSERT(false);
                        break;
                    }
                } else {
                    TIdPathAnchorIndexMap::iterator iter =
                        m_AnchorIdToPathAndAnchorIndexMap.find(theIdStr);
                    if (iter != m_AnchorIdToPathAndAnchorIndexMap.end()) {
                        SDomReaderPropertyParser parser(m_Reader, m_TempBuf, *this,
                                                        *iter->second.m_Segment);
                        QDemonDataRef<SPathAnchorPoint> thePathBuffer =
                            m_PathManager.GetPathSubPathBuffer(*iter->second.m_Segment);
                        quint32 anchorIndex = iter->second.m_AnchorIndex;
                        quint32 numAnchors = thePathBuffer.size();
                        if (anchorIndex < numAnchors) {
                            SPathAnchorPoint &thePoint(thePathBuffer[anchorIndex]);
                            ParseVec2(parser, "position", thePoint.m_Position);
                            ParseFloat(parser, "incomingangle", thePoint.m_IncomingAngle);
                            thePoint.m_OutgoingAngle = thePoint.m_IncomingAngle + 180.0f;
                            ParseFloat(parser, "incomingdistance", thePoint.m_IncomingDistance);
                            ParseFloat(parser, "outgoingdistance", thePoint.m_OutgoingDistance);
                        }
                    }
                }
            }
        }
    }

    void AddPluginProperty(IRenderPluginClass &pluginClass,
                           SRenderPluginPropertyTypes::Enum inPropType,
                           QString &tempStr, const char *propName)
    {
        tempStr.assign(propName);
        SRenderPluginPropertyDeclaration theDec(
            m_StrTable.RegisterStr(tempStr.c_str()), inPropType);
        pluginClass.RegisterProperty(theDec);
    }

    SPresentation *Load(bool inSetValuesFromSlides)
    {
        {
            TScope __outerScope(m_Reader);
            if (m_Reader.MoveToFirstChild("ProjectSettings")) {
                m_Reader.Att("presentationWidth", m_Presentation->m_PresentationDimensions.x);
                m_Reader.Att("presentationHeight", m_Presentation->m_PresentationDimensions.y);
                // Upsize them to a multiple of four.
                m_Presentation->m_PresentationDimensions.x =
                    (float)ITextRenderer::NextMultipleOf4(
                        (quint32)m_Presentation->m_PresentationDimensions.x);
                m_Presentation->m_PresentationDimensions.y =
                    (float)ITextRenderer::NextMultipleOf4(
                        (quint32)m_Presentation->m_PresentationDimensions.y);
                const char *thePresentationRotation = "";
                if (m_Reader.Att("presentationRotation", thePresentationRotation)) {
                    bool success = SRenderUIPLoader::ConvertEnumFromStr(
                        thePresentationRotation, m_Presentation->m_PresentationRotation);
                    (void)success;
                    Q_ASSERT(success);
                }
            }
        }
        {
            TScope __outerScope(m_Reader);
            if (m_Reader.MoveToFirstChild("Classes")) {
                for (bool valid = m_Reader.MoveToFirstChild(); valid;
                     valid = m_Reader.MoveToNextSibling()) {
                    const char *idStr = "", *name = "", *sourcepath = "";
                    m_Reader.Att("id", idStr);
                    m_Reader.Att("name", name);
                    m_Reader.Att("sourcepath", sourcepath);
                    if (AreEqual(m_Reader.GetNarrowElementName(), "Effect")) {
                        QString theId(m_StrTable.RegisterStr(idStr));
                        if (m_EffectSystem.IsEffectRegistered(theId) == false) {
                            // File should already be loaded.
                            QDemonOption<qt3dsdm::SMetaDataEffect> theEffectMetaData =
                                m_MetaData.GetEffectMetaDataBySourcePath(sourcepath);
                            if (theEffectMetaData.hasValue()) {
                                IUIPLoader::CreateEffectClassFromMetaEffect(
                                    theId, m_Foundation, m_EffectSystem, *theEffectMetaData,
                                    m_StrTable);
                            } else {
                                Q_ASSERT(false);
                            }
                        }
                    } else if (AreEqual(m_Reader.GetNarrowElementName(), "CustomMaterial")) {
                        QString theId(m_StrTable.RegisterStr(idStr));
                        if (m_CustomMaterialSystem.IsMaterialRegistered(theId) == false) {
                            // File should already be loaded.
                            QDemonOption<qt3dsdm::SMetaDataCustomMaterial> theMetaData =
                                m_MetaData.GetMaterialMetaDataBySourcePath(sourcepath);
                            if (theMetaData.hasValue()) {
                                IUIPLoader::CreateMaterialClassFromMetaMaterial(
                                    theId, m_Foundation, m_CustomMaterialSystem, *theMetaData,
                                    m_StrTable);
                            } else {
                                Q_ASSERT(false);
                            }
                        }
                    } else if (AreEqual(m_Reader.GetNarrowElementName(), "RenderPlugin")) {
                        QString theId(m_StrTable.RegisterStr(idStr));
                        m_MetaData.LoadPluginXMLFile(m_Reader.GetNarrowElementName(), idStr, name,
                                                     sourcepath);
                        eastl::vector<Q3DStudio::TRuntimeMetaDataStrType> theProperties;
                        IRenderPluginClass *thePluginClass =
                            m_RenderPluginManager.GetOrCreateRenderPlugin(
                                m_StrTable.RegisterStr(sourcepath));
                        if (thePluginClass) {
                            m_RenderPluginSourcePaths.insert(
                                eastl::make_pair(m_StrTable.RegisterStr(idStr), sourcepath));
                            m_MetaData.GetInstanceProperties(m_Reader.GetNarrowElementName(), idStr,
                                                             theProperties, false);
                            QString thePropertyStr;
                            QString metaType =
                                m_MetaData.GetStringTable()->GetRenderStringTable().RegisterStr(
                                    m_Reader.GetNarrowElementName());
                            QString metaId =
                                m_MetaData.GetStringTable()->GetRenderStringTable().RegisterStr(
                                    idStr);
                            for (quint32 idx = 0, end = theProperties.size(); idx < end; ++idx) {
                                using namespace Q3DStudio;
                                QString metaProp =
                                    m_MetaData.GetStringTable()->GetRenderStringTable().RegisterStr(
                                        theProperties[idx].c_str());
                                Q3DStudio::ERuntimeDataModelDataType thePropType =
                                    m_MetaData.GetPropertyType(metaType, metaProp, metaId);
                                switch (thePropType) {
                                case ERuntimeDataModelDataTypeFloat:
                                    AddPluginProperty(
                                        *thePluginClass,
                                        SRenderPluginPropertyTypes::Float,
                                        thePropertyStr, metaProp.c_str());
                                    break;
                                case ERuntimeDataModelDataTypeFloat2:
                                    AddPluginProperty(
                                        *thePluginClass,
                                        SRenderPluginPropertyTypes::Vector2,
                                        thePropertyStr, metaProp.c_str());
                                    break;
                                case ERuntimeDataModelDataTypeFloat3:
                                    if (m_MetaData.GetAdditionalType(metaType, metaProp, metaId)
                                        != ERuntimeAdditionalMetaDataTypeColor)
                                        AddPluginProperty(
                                            *thePluginClass,
                                            SRenderPluginPropertyTypes::Vector3,
                                            thePropertyStr, metaProp.c_str());
                                    else
                                        AddPluginProperty(
                                            *thePluginClass,
                                            SRenderPluginPropertyTypes::Color,
                                            thePropertyStr, metaProp.c_str());
                                    break;
                                case ERuntimeDataModelDataTypeLong:
                                    AddPluginProperty(*thePluginClass,
                                                      SRenderPluginPropertyTypes::Long,
                                                      thePropertyStr, metaProp.c_str());
                                    break;
                                case ERuntimeDataModelDataTypeString:
                                case ERuntimeDataModelDataTypeStringRef:
                                    AddPluginProperty(
                                        *thePluginClass,
                                        SRenderPluginPropertyTypes::String,
                                        thePropertyStr, metaProp.c_str());
                                    break;
                                case ERuntimeDataModelDataTypeBool:
                                    AddPluginProperty(
                                        *thePluginClass,
                                        SRenderPluginPropertyTypes::Boolean,
                                        thePropertyStr, metaProp.c_str());
                                    break;
                                default:
                                    Q_ASSERT(false);
                                }
                            }
                        }
                    }
                }
            }
        }
        {
            TScope __outerScope(m_Reader);
            if (m_Reader.MoveToFirstChild("BufferData")) {
                {
                    TScope __imageScope(m_Reader);
                    for (bool valid = m_Reader.MoveToFirstChild("ImageBuffer"); valid;
                         valid = m_Reader.MoveToNextSibling()) {
                        const char *srcPath;
                        m_Reader.UnregisteredAtt("sourcepath", srcPath);
                        QString imgPath = m_StrTable.RegisterStr(srcPath);
                        bool hasTransparency = false;
                        m_Reader.Att("hasTransparency", hasTransparency);
                        m_BufferManager.SetImageHasTransparency(imgPath, hasTransparency);
                    }
                }
            }
        }
        {
            TScope __outerScope(m_Reader);
            {
                if (m_Reader.MoveToFirstChild("Graph")) {
                    {
                        TScope __graphScope(m_Reader);
                        for (bool valid = m_Reader.MoveToFirstChild(); valid;
                             valid = m_Reader.MoveToNextSibling())
                            ParseGraphPass1(nullptr);
                    }
                    {
                        TScope __graphScope(m_Reader);
                        for (bool valid = m_Reader.MoveToFirstChild(); valid;
                             valid = m_Reader.MoveToNextSibling())
                            ParseGraphPass2();
                    }
                }
            }
        }
        TScope __outerScope(m_Reader);
        if (m_Reader.MoveToFirstChild("Logic")) {
            for (bool valid = m_Reader.MoveToFirstChild("State"); valid;
                 valid = m_Reader.MoveToNextSibling()) {
                {
                    TScope __slideScope(m_Reader);
                    ParseState(true); // parse master
                    for (bool subSlide = m_Reader.MoveToFirstChild("State"); subSlide;
                         subSlide = m_Reader.MoveToNextSibling("State")) {
                        TScope __subSlideScope(m_Reader);
                        ParseState(false); // parse slide setting only *add* values
                    }
                }
                {
                    TScope __slideScope(m_Reader);
                    if (inSetValuesFromSlides && m_Reader.MoveToFirstChild("State"))
                        ParseState(true); // parse slide setting only *set* values
                }
            }
        }

        return m_Presentation;
    }
};
}

SPresentation *IUIPLoader::LoadUIPFile(
    qt3dsdm::IDOMReader &inReader, const char *inFullPathToPresentationFile,
    Q3DStudio::IRuntimeMetaData &inMetaData, IStringTable &inStrTable,
    NVFoundationBase &inFoundation
    // Allocator used for the presentation objects themselves
    // this allows clients to pre-allocate a block of memory just for
    // the scene graph
    ,
    NVAllocatorCallback &inPresentationAllocator
    // Map of string ids to objects
    ,
    TIdObjectMap &ioObjectMap, IBufferManager &inBufferManager, IEffectSystem &inEffectSystem,
    const char *inPresentationDir, IRenderPluginManager &inPluginManager,
    ICustomMaterialSystem &inCMS, IDynamicObjectSystem &inDynamicSystem,
    IPathManager &inPathManager, IUIPReferenceResolver *inResolver,
    bool inSetValuesFromSlides)
{
    SRenderUIPLoader theLoader(inReader, inFullPathToPresentationFile, inMetaData, inStrTable,
                               inFoundation, inPresentationAllocator, ioObjectMap, inBufferManager,
                               inEffectSystem, inPresentationDir, inPluginManager, inCMS,
                               inDynamicSystem, inPathManager, inResolver);
    return theLoader.Load(inSetValuesFromSlides);
}
using namespace qt3dsdm;

inline QDemonRenderTextureFormats::Enum
ConvertTypeAndFormatToTextureFormat(const char *inType, const char *inFormat,
                                    NVFoundationBase &inFoundation)
{
    QDemonRenderTextureFormats::Enum retval = QDemonRenderTextureFormats::RGBA8;
    if (AreEqual(inType, "ubyte")) {
        if (AreEqual(inFormat, "rgb"))
            retval = QDemonRenderTextureFormats::RGB8;
        else if (AreEqual(inFormat, "rgba"))
            retval = QDemonRenderTextureFormats::RGBA8;
        else if (AreEqual(inFormat, "alpha"))
            retval = QDemonRenderTextureFormats::Alpha8;
        else if (AreEqual(inFormat, "lum"))
            retval = QDemonRenderTextureFormats::Luminance8;
        else if (AreEqual(inFormat, "lum_alpha"))
            retval = QDemonRenderTextureFormats::LuminanceAlpha8;
    } else if (AreEqual(inType, "ushort")) {
        if (AreEqual(inFormat, "rgb"))
            retval = QDemonRenderTextureFormats::RGB565;
        else if (AreEqual(inFormat, "rgba"))
            retval = QDemonRenderTextureFormats::RGBA5551;
    } else {
        qCCritical(INVALID_PARAMETER, "Unsupported texture type %s, defaulting to RGBA8",
            inType);
    }
    return retval;
}

inline QDemonRenderTextureMagnifyingOp::Enum
ConvertFilterToMagOp(const char *inFilter, NVFoundationBase &inFoundation)
{
    if (AreEqual(inFilter, "linear"))
        return QDemonRenderTextureMagnifyingOp::Linear;
    if (AreEqual(inFilter, "nearest"))
        return QDemonRenderTextureMagnifyingOp::Nearest;
    else {
        qCCritical(INVALID_PARAMETER, "Unsupported filter type %s, defaulting to linear",
            inFilter);
        return QDemonRenderTextureMagnifyingOp::Linear;
    }
}

inline QDemonRenderTextureCoordOp::Enum
ConvertTextureCoordOp(const char *inWrap, NVFoundationBase &inFoundation)
{
    if (AreEqual(inWrap, "clamp"))
        return QDemonRenderTextureCoordOp::ClampToEdge;
    if (AreEqual(inWrap, "repeat"))
        return QDemonRenderTextureCoordOp::Repeat;
    else {
        qCCritical(INVALID_PARAMETER, "Unsupported wrap type %s, defaulting to clamp",
            inWrap);
        return QDemonRenderTextureCoordOp::ClampToEdge;
    }
}

template <typename TCharStr>
QString ConvertUTFtoQString(const TCharStr *string);

template <>
QString ConvertUTFtoQString(const char16_t *string)
{
    return QString::fromUtf16(string);
}

template <>
QString ConvertUTFtoQString(const char32_t *string)
{
    return QString::fromUcs4(string);
}

template <>
QString ConvertUTFtoQString(const wchar_t *string)
{
    return QString::fromWCharArray(string);
}

// Re-register all strings because we can't be sure that the meta data system and the effect
// system are sharing the same string table.
void IUIPLoader::CreateEffectClassFromMetaEffect(
    QString inEffectName, NVFoundationBase &inFoundation, IEffectSystem &inEffectSystem,
    const qt3dsdm::SMetaDataEffect &inMetaDataEffect, IStringTable &inStrTable)
{
    using namespace dynamic;
    if (inEffectSystem.IsEffectRegistered(inEffectName)) {
        qCCritical(INVALID_OPERATION, "Effect %s is already registered",
            inEffectName.c_str());
        Q_ASSERT(false);
        return;
    }
    QVector<SPropertyDeclaration> thePropertyDeclarations(
        inFoundation.getAllocator(), "IUIPLoader::CreateEffectClassFromMetaEffect");
    QVector<QString> theEnumNames(
        inFoundation.getAllocator(), "IUIPLoader::CreateEffectClassFromMetaEffect");
    CRenderString theConvertStr;
    CRenderString theConvertShaderTypeStr;
    CRenderString theConvertShaderVersionStr;

    for (quint32 idx = 0, end = inMetaDataEffect.m_Properties.size(); idx < end; ++idx)
        thePropertyDeclarations.push_back(
            SPropertyDeclaration(inMetaDataEffect.m_Properties[idx].m_Name.c_str(),
                                 inMetaDataEffect.m_Properties[idx].m_DataType));
    inEffectSystem.RegisterEffect(inEffectName, thePropertyDeclarations);
    for (quint32 idx = 0, end = inMetaDataEffect.m_Properties.size(); idx < end; ++idx) {
        const SPropertyDefinition &theDefinition(inMetaDataEffect.m_Properties[idx]);
        if (theDefinition.m_EnumValueNames.size()) {
            theEnumNames.clear();
            for (quint32 enumIdx = 0, enumEnd = theDefinition.m_EnumValueNames.size();
                 enumIdx < enumEnd; ++enumIdx)
                theEnumNames.push_back(
                    inStrTable.RegisterStr(theDefinition.m_EnumValueNames[enumIdx]));
            inEffectSystem.SetEffectPropertyEnumNames(
                inEffectName, inStrTable.RegisterStr(theDefinition.m_Name), theEnumNames);
        }
        if (theDefinition.m_DataType == QDemonRenderShaderDataTypes::QDemonRenderTexture2DPtr)
            inEffectSystem.SetEffectPropertyTextureSettings(
                inEffectName, inStrTable.RegisterStr(theDefinition.m_Name),
                inStrTable.RegisterStr(theDefinition.m_ImagePath), theDefinition.m_TexUsageType,
                theDefinition.m_CoordOp, theDefinition.m_MagFilterOp, theDefinition.m_MinFilterOp);
    }
    for (quint32 idx = 0, end = inMetaDataEffect.m_Shaders.size(); idx < end; ++idx) {
        const qt3dsdm::SMetaDataShader &theShader = inMetaDataEffect.m_Shaders[idx];
        theConvertStr.clear();
        theConvertStr = ConvertUTFtoQString(
            theShader.m_Code.c_str()).toStdString();
        theConvertShaderTypeStr = ConvertUTFtoQString(
            theShader.m_Type.c_str()).toStdString();
        theConvertShaderVersionStr = ConvertUTFtoQString(
            theShader.m_Version.c_str()).toStdString();

        inEffectSystem.SetShaderData(inStrTable.RegisterStr(theShader.m_Name.c_str()),
                                     theConvertStr.c_str(), theConvertShaderVersionStr.c_str(),
                                     theConvertStr.c_str(), theShader.m_HasGeomShader,
                                     theShader.m_IsComputeShader);
    }

    inEffectSystem.SetEffectCommands(inEffectName, inMetaDataEffect.m_EffectCommands);
}

void IUIPLoader::CreateMaterialClassFromMetaMaterial(
    QString inClassName, NVFoundationBase &inFoundation,
    ICustomMaterialSystem &inMaterialSystem,
    const qt3dsdm::SMetaDataCustomMaterial &inMetaDataMaterial, IStringTable &inStrTable)
{
    using namespace dynamic;
    if (inMaterialSystem.IsMaterialRegistered(inClassName)) {
        qCCritical(INVALID_OPERATION, "Effect %s is already registered",
            inClassName.c_str());
        Q_ASSERT(false);
        return;
    }
    QVector<SPropertyDeclaration> thePropertyDeclarations(
        inFoundation.getAllocator(),
        "IUIPLoader::CreateMaterialClassFromMetaMaterial");
    QVector<QString> theEnumNames(
        inFoundation.getAllocator(),
        "IUIPLoader::CreateMaterialClassFromMetaMaterial");
    CRenderString theConvertStr;
    CRenderString theConvertShaderTypeStr;
    CRenderString theConvertShaderVersionStr;
    for (quint32 idx = 0, end = inMetaDataMaterial.m_Properties.size(); idx < end; ++idx)
        thePropertyDeclarations.push_back(
            SPropertyDeclaration(inMetaDataMaterial.m_Properties[idx].m_Name.c_str(),
                                 inMetaDataMaterial.m_Properties[idx].m_DataType));
    inMaterialSystem.RegisterMaterialClass(inClassName, thePropertyDeclarations);
    for (quint32 idx = 0, end = inMetaDataMaterial.m_Properties.size(); idx < end; ++idx) {
        const SPropertyDefinition &theDefinition(inMetaDataMaterial.m_Properties[idx]);
        if (theDefinition.m_EnumValueNames.size()) {
            theEnumNames.clear();
            for (quint32 enumIdx = 0, enumEnd = theDefinition.m_EnumValueNames.size();
                 enumIdx < enumEnd; ++enumIdx)
                theEnumNames.push_back(
                    inStrTable.RegisterStr(theDefinition.m_EnumValueNames[enumIdx]));
            inMaterialSystem.SetPropertyEnumNames(
                inClassName, inStrTable.RegisterStr(theDefinition.m_Name), theEnumNames);
        }
        if (theDefinition.m_DataType == QDemonRenderShaderDataTypes::QDemonRenderTexture2DPtr)
            inMaterialSystem.SetPropertyTextureSettings(
                inClassName, inStrTable.RegisterStr(theDefinition.m_Name),
                inStrTable.RegisterStr(theDefinition.m_ImagePath), theDefinition.m_TexUsageType,
                theDefinition.m_CoordOp, theDefinition.m_MagFilterOp, theDefinition.m_MinFilterOp);
    }
    if (inMetaDataMaterial.m_Shaders.size()) {
        for (quint32 idx = 0, end = (quint32)inMetaDataMaterial.m_Shaders.size(); idx < end; ++idx) {
            const qt3dsdm::SMetaDataShader &theShader = inMetaDataMaterial.m_Shaders[idx];
            theConvertStr = ConvertUTFtoQString(
                theShader.m_Code.c_str()).toStdString();
            theConvertShaderTypeStr = ConvertUTFtoQString(
                theShader.m_Type.c_str()).toStdString();
            theConvertShaderVersionStr = ConvertUTFtoQString(
                theShader.m_Version.c_str()).toStdString();
            inMaterialSystem.SetMaterialClassShader(
                inStrTable.RegisterStr(theShader.m_Name.c_str()), theConvertShaderTypeStr.c_str(),
                theConvertShaderVersionStr.c_str(), theConvertStr.c_str(),
                theShader.m_HasGeomShader, theShader.m_IsComputeShader);
        }
    }

    inMaterialSystem.SetCustomMaterialCommands(inClassName,
                                               inMetaDataMaterial.m_CustomMaterialCommands);
    inMaterialSystem.SetCustomMaterialTransparency(inClassName,
                                                   inMetaDataMaterial.m_HasTransparency);
    inMaterialSystem.SetCustomMaterialRefraction(inClassName, inMetaDataMaterial.m_HasRefraction);
    inMaterialSystem.SetCustomMaterialAlwaysDirty(inClassName, inMetaDataMaterial.m_AlwaysDirty);
    inMaterialSystem.SetCustomMaterialShaderKey(inClassName, inMetaDataMaterial.m_ShaderKey);
    inMaterialSystem.SetCustomMaterialLayerCount(inClassName, inMetaDataMaterial.m_LayerCount);
}

#endif
