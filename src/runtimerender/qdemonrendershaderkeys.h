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
#ifndef QDEMON_RENDER_SHADER_KEY_H
#define QDEMON_RENDER_SHADER_KEY_H

#include <QtDemon/qdemondataref.h>

#include <QtDemonRender/qdemonrenderbasetypes.h>

#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterial.h>
#include <QtDemonRuntimeRender/qdemonrendertessmodevalues.h>

QT_BEGIN_NAMESPACE
// We have an ever expanding set of properties we like to hash into one or more 32 bit
// quantities.
// Furthermore we would like this set of properties to be convertable to string
// So the shader cache file itself is somewhat human readable/diagnosable.
// To do this we create a set of objects that act as properties to the master shader key.
// These objects are tallied in order to figure out their actual offset into the shader key's
// data store.  They are also run through in order to create the string shader cache key.

struct QDemonShaderKeyPropertyBase
{
    const char *name;
    quint32 offset;
    QDemonShaderKeyPropertyBase(const char *inName = "")
        : name(inName)
        , offset(0)
    {
    }
    quint32 getOffset() const { return offset; }
    void setOffset(quint32 of) { offset = of; }

    template <quint32 TBitWidth>
    quint32 getMaskTemplate() const
    {
        quint32 bit = offset % 32;
        quint32 startValue = (1 << TBitWidth) - 1;
        quint32 mask = startValue << bit;
        return mask;
    }

    quint32 getIdx() const { return offset / 32; }
protected:
    void internalToString(QString &ioStr, const char *inBuffer) const
    {
        ioStr.append(QString::fromLocal8Bit(name));
        ioStr.append(QStringLiteral("="));
        ioStr.append(QString::fromLocal8Bit(inBuffer));
    }

    static void internalToString(QString &ioStr, const char *name, bool inValue)
    {
        if (inValue) {
            ioStr.append(QString::fromLocal8Bit(name));
            ioStr.append(QStringLiteral("="));
            ioStr.append(inValue ? QStringLiteral("true") : QStringLiteral("false"));
        }
    }
};

struct QDemonShaderKeyBoolean : public QDemonShaderKeyPropertyBase
{
    enum {
        BitWidth = 1,
    };

    QDemonShaderKeyBoolean(const char *inName = "")
        : QDemonShaderKeyPropertyBase(inName)
    {
    }

    quint32 getMask() const { return getMaskTemplate<BitWidth>(); }
    void setValue(QDemonDataRef<quint32> inDataStore, bool inValue) const
    {
        quint32 idx = getIdx();
        Q_ASSERT(inDataStore.size() > idx);
        quint32 mask = getMask();
        quint32 &target = inDataStore[idx];
        if (inValue == true) {
            target = target | mask;
        } else {
            mask = ~mask;
            target = target & mask;
        }
    }

    bool getValue(QDemonConstDataRef<quint32> inDataStore) const
    {
        quint32 idx = getIdx();
        quint32 mask = getMask();
        const quint32 &target = inDataStore[idx];
        return (target & mask) ? true : false;
    }

    void toString(QString &ioStr, QDemonConstDataRef<quint32> inKeySet) const
    {
        bool isHigh = getValue(inKeySet);
        internalToString(ioStr, name, isHigh);
    }
};

template <quint32 TBitWidth>
struct QDemonShaderKeyUnsigned : public QDemonShaderKeyPropertyBase
{
    enum {
        BitWidth = TBitWidth,
    };
    QDemonShaderKeyUnsigned(const char *inName = "")
        : QDemonShaderKeyPropertyBase(inName)
    {
    }
    quint32 getMask() const { return getMaskTemplate<BitWidth>(); }
    void setValue(QDemonDataRef<quint32> inDataStore, quint32 inValue) const
    {
        quint32 startValue = (1 << TBitWidth) - 1;
        // Ensure inValue is within range of bit width.
        inValue = inValue & startValue;
        quint32 bit = offset % 32;
        quint32 mask = getMask();
        quint32 idx = getIdx();
        inValue = inValue << bit;
        quint32 &target = inDataStore[idx];
        // Get rid of existing value
        quint32 inverseMask = ~mask;
        target = target & inverseMask;
        target = target | inValue;
    }

    quint32 getValue(QDemonConstDataRef<quint32> inDataStore) const
    {
        quint32 idx = getIdx();
        quint32 bit = offset % 32;
        quint32 mask = getMask();
        const quint32 &target = inDataStore[idx];

        quint32 retval = target & mask;
        retval = retval >> bit;
        return retval;
    }

    void toString(QString &ioStr, QDemonConstDataRef<quint32> inKeySet) const
    {
        quint32 value = getValue(inKeySet);
        char buf[64];
        toStr(value, toDataRef(buf, 64));
        internalToString(ioStr, buf);
    }

private:
    static quint32 toStr(quint32 item, QDemonDataRef<char> buffer)
    {
        // hope the buffer is big enough...
        return static_cast<quint32>(::snprintf(buffer.begin(), buffer.size(), "%u", item));
    }
};

struct QDemonShaderKeyTessellation : public QDemonShaderKeyUnsigned<4>
{
    enum TessellationBits {
        noTessellation = 1 << 0,
        linearTessellation = 1 << 1,
        phongTessellation = 1 << 2,
        npatchTessellation = 1 << 3
    };

    QDemonShaderKeyTessellation(const char *inName = "")
        : QDemonShaderKeyUnsigned<4>(inName)
    {
    }

    bool getBitValue(TessellationBits swizzleBit, QDemonConstDataRef<quint32> inKeySet) const
    {
        return (getValue(inKeySet) & swizzleBit) ? true : false;
    }

    void setBitValue(TessellationBits swizzleBit, bool inValue, QDemonDataRef<quint32> inKeySet)
    {
        quint32 theValue = getValue(inKeySet);
        quint32 mask = swizzleBit;
        if (inValue) {
            theValue = theValue | mask;
        } else {
            mask = ~mask;
            theValue = theValue & mask;
        }
        setValue(inKeySet, theValue);
    }

    void setTessellationMode(QDemonDataRef<quint32> inKeySet,
                             TessModeValues::Enum tessellationMode,
                             bool val)
    {
        switch (tessellationMode) {
        case TessModeValues::NoTess:
            setBitValue(noTessellation, val, inKeySet);
            break;
        case TessModeValues::TessLinear:
            setBitValue(linearTessellation, val, inKeySet);
            break;
        case TessModeValues::TessNPatch:
            setBitValue(npatchTessellation, val, inKeySet);
            break;
        case TessModeValues::TessPhong:
            setBitValue(phongTessellation, val, inKeySet);
            break;
        }
    }

    bool isNoTessellation(QDemonConstDataRef<quint32> inKeySet) const
    {
        return getBitValue(noTessellation, inKeySet);
    }
    void setNoTessellation(QDemonDataRef<quint32> inKeySet, bool val)
    {
        setBitValue(noTessellation, val, inKeySet);
    }

    bool isLinearTessellation(QDemonConstDataRef<quint32> inKeySet) const
    {
        return getBitValue(linearTessellation, inKeySet);
    }
    void setLinearTessellation(QDemonDataRef<quint32> inKeySet, bool val)
    {
        setBitValue(linearTessellation, val, inKeySet);
    }

    bool isNPatchTessellation(QDemonConstDataRef<quint32> inKeySet) const
    {
        return getBitValue(npatchTessellation, inKeySet);
    }
    void setNPatchTessellation(QDemonDataRef<quint32> inKeySet, bool val)
    {
        setBitValue(npatchTessellation, val, inKeySet);
    }

    bool isPhongTessellation(QDemonConstDataRef<quint32> inKeySet) const
    {
        return getBitValue(phongTessellation, inKeySet);
    }
    void setPhongTessellation(QDemonDataRef<quint32> inKeySet, bool val)
    {
        setBitValue(phongTessellation, val, inKeySet);
    }

    void toString(QString &ioStr, QDemonConstDataRef<quint32> inKeySet) const
    {
        ioStr.append(QString::fromLocal8Bit(name));
        ioStr.append(QStringLiteral("={"));
        internalToString(ioStr, "noTessellation", isNoTessellation(inKeySet));
        ioStr.append(QStringLiteral(";"));
        internalToString(ioStr, "linearTessellation", isLinearTessellation(inKeySet));
        ioStr.append(QStringLiteral(";"));
        internalToString(ioStr, "npatchTessellation", isNPatchTessellation(inKeySet));
        ioStr.append(QStringLiteral(";"));
        internalToString(ioStr, "phongTessellation", isPhongTessellation(inKeySet));
        ioStr.append(QStringLiteral("}"));
    }
};

struct QDemonShaderKeyTextureSwizzle : public QDemonShaderKeyUnsigned<5>
{
    enum TextureSwizzleBits {
        noSwizzle = 1 << 0,
        L8toR8 = 1 << 1,
        A8toR8 = 1 << 2,
        L8A8toRG8 = 1 << 3,
        L16toR16 = 1 << 4
    };

    QDemonShaderKeyTextureSwizzle(const char *inName = "")
        : QDemonShaderKeyUnsigned<5>(inName)
    {
    }

    bool getBitValue(TextureSwizzleBits swizzleBit, QDemonConstDataRef<quint32> inKeySet) const
    {
        return (getValue(inKeySet) & swizzleBit) ? true : false;
    }

    void setBitValue(TextureSwizzleBits swizzleBit, bool inValue, QDemonDataRef<quint32> inKeySet)
    {
        quint32 theValue = getValue(inKeySet);
        quint32 mask = swizzleBit;
        if (inValue) {
            theValue = theValue | mask;
        } else {
            mask = ~mask;
            theValue = theValue & mask;
        }
        setValue(inKeySet, theValue);
    }

    void setSwizzleMode(QDemonDataRef<quint32> inKeySet, QDemonRenderTextureSwizzleMode::Enum swizzleMode,
                        bool val)
    {
        switch (swizzleMode) {
        case QDemonRenderTextureSwizzleMode::NoSwizzle:
            setBitValue(noSwizzle, val, inKeySet);
            break;
        case QDemonRenderTextureSwizzleMode::L8toR8:
            setBitValue(L8toR8, val, inKeySet);
            break;
        case QDemonRenderTextureSwizzleMode::A8toR8:
            setBitValue(A8toR8, val, inKeySet);
            break;
        case QDemonRenderTextureSwizzleMode::L8A8toRG8:
            setBitValue(L8A8toRG8, val, inKeySet);
            break;
        case QDemonRenderTextureSwizzleMode::L16toR16:
            setBitValue(L16toR16, val, inKeySet);
            break;
        }
    }

    bool isNoSwizzled(QDemonConstDataRef<quint32> inKeySet) const
    {
        return getBitValue(noSwizzle, inKeySet);
    }
    void setNoSwizzled(QDemonDataRef<quint32> inKeySet, bool val)
    {
        setBitValue(noSwizzle, val, inKeySet);
    }

    bool isL8Swizzled(QDemonConstDataRef<quint32> inKeySet) const
    {
        return getBitValue(L8toR8, inKeySet);
    }
    void setL8Swizzled(QDemonDataRef<quint32> inKeySet, bool val)
    {
        setBitValue(L8toR8, val, inKeySet);
    }

    bool isA8Swizzled(QDemonConstDataRef<quint32> inKeySet) const
    {
        return getBitValue(A8toR8, inKeySet);
    }
    void setA8Swizzled(QDemonDataRef<quint32> inKeySet, bool val)
    {
        setBitValue(A8toR8, val, inKeySet);
    }

    bool isL8A8Swizzled(QDemonConstDataRef<quint32> inKeySet) const
    {
        return getBitValue(L8A8toRG8, inKeySet);
    }
    void setL8A8Swizzled(QDemonDataRef<quint32> inKeySet, bool val)
    {
        setBitValue(L8A8toRG8, val, inKeySet);
    }

    bool isL16Swizzled(QDemonConstDataRef<quint32> inKeySet) const
    {
        return getBitValue(L16toR16, inKeySet);
    }
    void setL16Swizzled(QDemonDataRef<quint32> inKeySet, bool val)
    {
        setBitValue(L16toR16, val, inKeySet);
    }

    void toString(QString &ioStr, QDemonConstDataRef<quint32> inKeySet) const
    {
        ioStr.append(QString::fromLocal8Bit(name));
        ioStr.append(QStringLiteral("={"));
        internalToString(ioStr, "noswizzle", isNoSwizzled(inKeySet));
        ioStr.append(QStringLiteral(";"));
        internalToString(ioStr, "l8swizzle", isL8Swizzled(inKeySet));
        ioStr.append(QStringLiteral(";"));
        internalToString(ioStr, "a8swizzle", isA8Swizzled(inKeySet));
        ioStr.append(QStringLiteral(";"));
        internalToString(ioStr, "l8a8swizzle", isL8A8Swizzled(inKeySet));
        ioStr.append(QStringLiteral(";"));
        internalToString(ioStr, "l16swizzle", isL16Swizzled(inKeySet));
        ioStr.append(QStringLiteral("}"));
    }
};

struct QDemonShaderKeyImageMap : public QDemonShaderKeyUnsigned<5>
{
    enum ImageMapBits {
        Enabled = 1 << 0,
        EnvMap = 1 << 1,
        LightProbe = 1 << 2,
        InvertUV = 1 << 3,
        Premultiplied = 1 << 4,
    };

    QDemonShaderKeyImageMap(const char *inName = "")
        : QDemonShaderKeyUnsigned<5>(inName)
    {
    }

    bool getBitValue(ImageMapBits imageBit, QDemonConstDataRef<quint32> inKeySet) const
    {
        return (getValue(inKeySet) & imageBit) ? true : false;
    }

    void setBitValue(ImageMapBits imageBit, bool inValue, QDemonDataRef<quint32> inKeySet)
    {
        quint32 theValue = getValue(inKeySet);
        quint32 mask = imageBit;
        if (inValue) {
            theValue = theValue | mask;
        } else {
            mask = ~mask;
            theValue = theValue & mask;
        }
        setValue(inKeySet, theValue);
    }

    bool isEnabled(QDemonConstDataRef<quint32> inKeySet) const
    {
        return getBitValue(Enabled, inKeySet);
    }
    void setEnabled(QDemonDataRef<quint32> inKeySet, bool val)
    {
        setBitValue(Enabled, val, inKeySet);
    }

    bool isEnvMap(QDemonConstDataRef<quint32> inKeySet) const
    {
        return getBitValue(EnvMap, inKeySet);
    }
    void setEnvMap(QDemonDataRef<quint32> inKeySet, bool val) { setBitValue(EnvMap, val, inKeySet); }

    bool isLightProbe(QDemonConstDataRef<quint32> inKeySet) const
    {
        return getBitValue(LightProbe, inKeySet);
    }
    void setLightProbe(QDemonDataRef<quint32> inKeySet, bool val)
    {
        setBitValue(LightProbe, val, inKeySet);
    }

    bool isInvertUVMap(QDemonConstDataRef<quint32> inKeySet) const
    {
        return getBitValue(InvertUV, inKeySet);
    }
    void setInvertUVMap(QDemonDataRef<quint32> inKeySet, bool val)
    {
        setBitValue(InvertUV, val, inKeySet);
    }

    bool isPremultiplied(QDemonConstDataRef<quint32> inKeySet) const
    {
        return getBitValue(Premultiplied, inKeySet);
    }
    void setPremultiplied(QDemonDataRef<quint32> inKeySet, bool val)
    {
        setBitValue(Premultiplied, val, inKeySet);
    }

    void toString(QString &ioStr, QDemonConstDataRef<quint32> inKeySet) const
    {
        ioStr.append(QString::fromLocal8Bit(name));
        ioStr.append(QStringLiteral("={"));
        internalToString(ioStr, "enabled", isEnabled(inKeySet));
        ioStr.append(QStringLiteral(";"));
        internalToString(ioStr, "envMap", isEnvMap(inKeySet));
        ioStr.append(QStringLiteral(";"));
        internalToString(ioStr, "lightProbe", isLightProbe(inKeySet));
        ioStr.append(QStringLiteral(";"));
        internalToString(ioStr, "invertUV", isInvertUVMap(inKeySet));
        ioStr.append(QStringLiteral(";"));
        internalToString(ioStr, "premultiplied", isPremultiplied(inKeySet));
        ioStr.append(QStringLiteral("}"));
    }
};

struct QDemonShaderKeySpecularModel : QDemonShaderKeyUnsigned<2>
{
    QDemonShaderKeySpecularModel(const char *inName = "")
        : QDemonShaderKeyUnsigned<2>(inName)
    {
    }

    void setSpecularModel(QDemonDataRef<quint32> inKeySet,
                          DefaultMaterialSpecularModel::Enum inModel)
    {
        setValue(inKeySet, quint32(inModel));
    }

    DefaultMaterialSpecularModel::Enum getSpecularModel(QDemonConstDataRef<quint32> inKeySet) const
    {
        return static_cast<DefaultMaterialSpecularModel::Enum>(getValue(inKeySet));
    }

    void toString(QString &ioStr, QDemonConstDataRef<quint32> inKeySet) const
    {
        ioStr.append(QString::fromLocal8Bit(name));
        ioStr.append(QStringLiteral("="));
        switch (getSpecularModel(inKeySet)) {
        case DefaultMaterialSpecularModel::KGGX:
            ioStr.append(QStringLiteral("KGGX"));
            break;
        case DefaultMaterialSpecularModel::KWard:
            ioStr.append(QStringLiteral("KWard"));
            break;
        case DefaultMaterialSpecularModel::Default:
            ioStr.append(QStringLiteral("Default"));
            break;
        }
        ioStr.append(QStringLiteral(";"));
    }
};

struct QDemonShaderDefaultMaterialKeyProperties
{
    enum {
        LightCount = 7,
    };
    enum ImageMapNames {
        DiffuseMap0 = 0,
        DiffuseMap1,
        DiffuseMap2,
        EmissiveMap,
        EmissiveMap2,
        SpecularMap,
        OpacityMap,
        BumpMap,
        SpecularAmountMap,
        NormalMap,
        DisplacementMap,
        TranslucencyMap,
        LightmapIndirect,
        LightmapRadiosity,
        LightmapShadow,
        RoughnessMap,
        ImageMapCount
    };

    QDemonShaderKeyBoolean m_hasLighting;
    QDemonShaderKeyBoolean m_hasIbl;
    QDemonShaderKeyUnsigned<3> m_lightCount;
    QDemonShaderKeyBoolean m_lightFlags[LightCount];
    QDemonShaderKeyBoolean m_lightAreaFlags[LightCount];
    QDemonShaderKeyBoolean m_lightShadowFlags[LightCount];
    QDemonShaderKeyBoolean m_specularEnabled;
    QDemonShaderKeyBoolean m_fresnelEnabled;
    QDemonShaderKeyBoolean m_vertexColorsEnabled;
    QDemonShaderKeySpecularModel m_specularModel;
    QDemonShaderKeyImageMap m_imageMaps[ImageMapCount];
    QDemonShaderKeyTextureSwizzle m_textureSwizzle[ImageMapCount];
    QDemonShaderKeyTessellation m_tessellationMode;
    QDemonShaderKeyBoolean m_hasSkinning;
    QDemonShaderKeyBoolean m_wireframeMode;

    QDemonShaderDefaultMaterialKeyProperties()
        : m_hasLighting("hasLighting")
        , m_hasIbl("hasIbl")
        , m_lightCount("lightCount")
        , m_specularEnabled("specularEnabled")
        , m_fresnelEnabled("fresnelEnabled")
        , m_vertexColorsEnabled("vertexColorsEnabled")
        , m_specularModel("specularModel")
        , m_tessellationMode("tessellationMode")
        , m_hasSkinning("hasSkinning")
        , m_wireframeMode("wireframeMode")
    {
        m_lightFlags[0].name = "light0HasPosition";
        m_lightFlags[1].name = "light1HasPosition";
        m_lightFlags[2].name = "light2HasPosition";
        m_lightFlags[3].name = "light3HasPosition";
        m_lightFlags[4].name = "light4HasPosition";
        m_lightFlags[5].name = "light5HasPosition";
        m_lightFlags[6].name = "light6HasPosition";
        m_lightAreaFlags[0].name = "light0HasArea";
        m_lightAreaFlags[1].name = "light1HasArea";
        m_lightAreaFlags[2].name = "light2HasArea";
        m_lightAreaFlags[3].name = "light3HasArea";
        m_lightAreaFlags[4].name = "light4HasArea";
        m_lightAreaFlags[5].name = "light5HasArea";
        m_lightAreaFlags[6].name = "light6HasArea";
        m_lightShadowFlags[0].name = "light0HasShadow";
        m_lightShadowFlags[1].name = "light1HasShadow";
        m_lightShadowFlags[2].name = "light2HasShadow";
        m_lightShadowFlags[3].name = "light3HasShadow";
        m_lightShadowFlags[4].name = "light4HasShadow";
        m_lightShadowFlags[5].name = "light5HasShadow";
        m_lightShadowFlags[6].name = "light6HasShadow";
        m_imageMaps[0].name = "diffuseMap0";
        m_imageMaps[1].name = "diffuseMap1";
        m_imageMaps[2].name = "diffuseMap2";
        m_imageMaps[3].name = "emissiveMap";
        m_imageMaps[4].name = "emissiveMap2";
        m_imageMaps[5].name = "specularMap";
        m_imageMaps[6].name = "opacityMap";
        m_imageMaps[7].name = "bumpMap";
        m_imageMaps[8].name = "specularAmountMap";
        m_imageMaps[9].name = "normalMap";
        m_imageMaps[10].name = "displacementMap";
        m_imageMaps[11].name = "translucencyMap";
        m_imageMaps[12].name = "lightmapIndirect";
        m_imageMaps[13].name = "lightmapRadiosity";
        m_imageMaps[14].name = "lightmapShadow";
        m_imageMaps[15].name = "roughnessMap";
        m_textureSwizzle[0].name = "diffuseMap0_swizzle";
        m_textureSwizzle[1].name = "diffuseMap1_swizzle";
        m_textureSwizzle[2].name = "diffuseMap2_swizzle";
        m_textureSwizzle[3].name = "emissiveMap_swizzle";
        m_textureSwizzle[4].name = "emissiveMap2_swizzle";
        m_textureSwizzle[5].name = "specularMap_swizzle";
        m_textureSwizzle[6].name = "opacityMap_swizzle";
        m_textureSwizzle[7].name = "bumpMap_swizzle";
        m_textureSwizzle[8].name = "specularAmountMap_swizzle";
        m_textureSwizzle[9].name = "normalMap_swizzle";
        m_textureSwizzle[10].name = "displacementMap_swizzle";
        m_textureSwizzle[11].name = "translucencyMap_swizzle";
        m_textureSwizzle[12].name = "lightmapIndirect_swizzle";
        m_textureSwizzle[13].name = "lightmapRadiosity_swizzle";
        m_textureSwizzle[14].name = "lightmapShadow_swizzle";
        m_textureSwizzle[15].name = "roughnessMap_swizzle";
        setPropertyOffsets();
    }

    template <typename TVisitor>
    void visitProperties(TVisitor &inVisitor)
    {
        inVisitor.visit(m_hasLighting);
        inVisitor.visit(m_hasIbl);
        inVisitor.visit(m_lightCount);

        for (quint32 idx = 0, end = LightCount; idx < end; ++idx) {
            inVisitor.visit(m_lightFlags[idx]);
        }

        for (quint32 idx = 0, end = LightCount; idx < end; ++idx) {
            inVisitor.visit(m_lightAreaFlags[idx]);
        }

        for (quint32 idx = 0, end = LightCount; idx < end; ++idx) {
            inVisitor.visit(m_lightShadowFlags[idx]);
        }

        inVisitor.visit(m_specularEnabled);
        inVisitor.visit(m_fresnelEnabled);
        inVisitor.visit(m_vertexColorsEnabled);
        inVisitor.visit(m_specularModel);

        for (quint32 idx = 0, end = ImageMapCount; idx < end; ++idx) {
            inVisitor.visit(m_imageMaps[idx]);
            inVisitor.visit(m_textureSwizzle[idx]);
        }

        inVisitor.visit(m_tessellationMode);
        inVisitor.visit(m_hasSkinning);
        inVisitor.visit(m_wireframeMode);
    }

    struct OffsetVisitor
    {
        quint32 m_offset;
        OffsetVisitor()
            : m_offset(0)
        {
        }
        template <typename TPropType>
        void visit(TPropType &inProp)
        {
            // if we cross the 32 bit border we just move
            // to the next dword.
            // This cost a few extra bits but prevents tedious errors like
            // loosing shader key bits because they got moved beyond the 32 border
            quint32 bit = m_offset % 32;
            if (bit + TPropType::BitWidth > 31) {
                m_offset += 32 - bit;
            }

            inProp.setOffset(m_offset);
            m_offset += TPropType::BitWidth;
        }
    };

    void setPropertyOffsets()
    {
        OffsetVisitor visitor;
        visitProperties(visitor);
        // If this assert fires, then the default material key needs more bits.
        Q_ASSERT(visitor.m_offset < 224);
    }
};

struct QDemonShaderDefaultMaterialKey
{
    enum {
        DataBufferSize = 7,
    };
    quint32 m_dataBuffer[DataBufferSize];
    uint m_featureSetHash;

    QDemonShaderDefaultMaterialKey(uint inFeatureSetHash)
        : m_featureSetHash(inFeatureSetHash)
    {
        for (size_t idx = 0; idx < DataBufferSize; ++idx)
            m_dataBuffer[idx] = 0;
    }

    QDemonShaderDefaultMaterialKey()
        : m_featureSetHash(0)
    {
        for (size_t idx = 0; idx < DataBufferSize; ++idx)
            m_dataBuffer[idx] = 0;
    }

    uint hash() const
    {
        uint retval = 0;
        for (size_t idx = 0; idx < DataBufferSize; ++idx)
            retval = retval ^ qHash(m_dataBuffer[idx]);
        return retval ^ m_featureSetHash;
    }

    bool operator==(const QDemonShaderDefaultMaterialKey &other) const
    {
        bool retval = true;
        for (size_t idx = 0; idx < DataBufferSize && retval; ++idx)
            retval = m_dataBuffer[idx] == other.m_dataBuffer[idx];
        return retval && m_featureSetHash == other.m_featureSetHash;
    }

    // Cast operators to make getting properties easier.
    operator QDemonDataRef<quint32>() { return toDataRef(m_dataBuffer, DataBufferSize); }

    operator QDemonConstDataRef<quint32>() const
    {
        return toConstDataRef(m_dataBuffer, DataBufferSize);
    }

    struct StringVisitor
    {
        QByteArray &m_str;
        QDemonConstDataRef<quint32> m_keyStore;
        StringVisitor(QByteArray &s, QDemonConstDataRef<quint32> ks)
            : m_str(s)
            , m_keyStore(ks)
        {
        }
        template <typename TPropType>
        void visit(const TPropType &prop)
        {
            quint32 originalSize = m_str.size();
            if (m_str.size())
                m_str.append(";");
            QString str = QString::fromUtf8(m_str);
            prop.toString(str, m_keyStore);
            // if the only thing we added was the semicolon
            // then nuke the semicolon
            if (originalSize && m_str.size() == int(originalSize + 1))
                m_str.resize(int(originalSize));
        }
    };

    void toString(QByteArray &ioString, QDemonShaderDefaultMaterialKeyProperties &inProperties) const
    {
        StringVisitor theVisitor(ioString, *this);
        inProperties.visitProperties(theVisitor);
    }
};

QT_END_NAMESPACE

#endif
