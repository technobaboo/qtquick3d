/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
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

#ifndef QDEMONSCENEGRAPHTRANSLATION_P_H
#define QDEMONSCENEGRAPHTRANSLATION_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtDemonAssetImport/qtdemonassetimportglobal.h>

#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QByteArray>
#include <QtCore/QBuffer>
#include <QtCore/QTextStream>
#include <QtCore/QSharedPointer>

QT_BEGIN_NAMESPACE

namespace QDemonAssetImport {

enum TransformType {
    TransformType_Matrix4x4,
    TransformType_Rotation3, // rotation defined with 3 angles
    TransformType_Rotation4, // rotation defined with an axis and angle
    TransformType_Translate3,
    TransformType_Scale3,
    TransformType_Unknown
};

class AbstractNodeTransform
{
public:
    virtual ~AbstractNodeTransform() {}
    virtual operator float *() const = 0;
    virtual TransformType GetType() const = 0;
};

class NodeTransform : public AbstractNodeTransform
{
public:
    NodeTransform(TransformType inType)
    {
        m_Data = nullptr;
        switch (inType) {
        case TransformType_Matrix4x4:
            m_Data = new float[16];
            break;
        case TransformType_Rotation3:
            m_Data = new float[3];
            break;
        case TransformType_Rotation4:
            m_Data = new float[4];
            break;
        case TransformType_Translate3:
            m_Data = new float[3];
            break;
        case TransformType_Scale3:
            m_Data = new float[3];
            break;
        default:
            break;
        }

        m_Type = inType;
    }

    ~NodeTransform() override { delete[] m_Data; }

    operator float *() const override { return m_Data; }

    TransformType GetType() const override { return m_Type; }

    static void Delete(AbstractNodeTransform *inTransform) { delete inTransform; }

protected:
    float *m_Data;
    TransformType m_Type;
};

enum TextureMapType {
    TextureMapTypeDiffuse,
    TextureMapTypeOpacity,
    TextureMapTypeSpecular,
    TextureMapTypeRoughness,
    TextureMapTypeEmissive
};

enum MatCommonProfileTechnique {
    MatCommonProfileTechnique_Blinn,
    MatCommonProfileTechnique_Constant,
    MatCommonProfileTechnique_Lambert,
    MatCommonProfileTechnique_Phong,
    MatCommonProfileTechnique_Count
};

enum MatOpaqueType // refer to domFx_opaque_enum
{
    MatOpaqueType_A_ONE, // When a transparent opaque attribute is set to A_ONE, it means the
                        // transparency information will be taken from the alpha channel of the
                        // color, texture, or parameter supplying the value. The value of 1.0 is
                        // opaque in this mode.
    MatOpaqueType_RGB_ZERO, // When a transparent opaque attribute is set to RGB_ZERO, it means the
                           // transparency information will be taken from the red, green, and blue
                           // channels of the color, texture, or parameter supplying the value. Each
                           // channel is modulated independently. The value of 0.0 is opaque in this
                           // mode.
};

struct FloatFlag
{
    float m_Value; // the value of this struct
    bool m_Flag; // to indicate if this value presents in SceneGraph
    FloatFlag()
        : m_Value(0.0f)
        , m_Flag(false)
    {
    }
    void SetValue(float inValue)
    {
        m_Value = inValue;
        m_Flag = true;
    }
};

struct LongFlag
{
    long m_Value; // the value of this struct
    bool m_Flag; // to indicate if this value presents in SceneGraph
    LongFlag()
        : m_Value(0)
        , m_Flag(false)
    {
    }
    void SetValue(long inValue)
    {
        m_Value = inValue;
        m_Flag = true;
    }
};

struct ColorOrTexture
{
    enum Type {
        None, // This information doesn't present in SceneGraph or of Param Type
        Color, // Color type
        Texture, // Texture type
    };

    Type m_Type;
    float m_Color[4]; // the color, if present
    ColorOrTexture()
        : m_Type(None)
    {
        m_Color[0] = m_Color[1] = m_Color[2] = m_Color[3] = 0.0f;
    }
    void SetColor(const float inColor[])
    {
        m_Color[0] = inColor[0];
        m_Color[1] = inColor[1];
        m_Color[2] = inColor[2];
        m_Color[3] = inColor[3];
        m_Type = Color;
    }
    void SetColor(float c0, float c1, float c2, float c3)
    {
        m_Color[0] = c0;
        m_Color[1] = c1;
        m_Color[2] = c2;
        m_Color[3] = c3;
        m_Type = Color;
    }
    void SetTexture() { m_Type = Texture; }
};

//==============================================================================
// To store information regarding texture parameters which normally stored under <extra> tag
//==============================================================================
struct TextureParameters
{
    FloatFlag m_coverageU;
    FloatFlag m_coverageV;
    FloatFlag m_repeatU;
    FloatFlag m_repeatV;
    FloatFlag m_offsetU;
    FloatFlag m_offsetV;
    FloatFlag m_rotateUV;
    LongFlag m_wrapU;
    LongFlag m_wrapV;
    LongFlag m_mirrorU;
    LongFlag m_mirrorV;
    bool m_Flag; // to indicate if dae contains texture parameters information
    TextureParameters()
        : m_Flag(false)
    {
    }
};

//==============================================================================
// To store information regarding material parameters which normally stored under <extra> tag
//==============================================================================
struct MaterialExtraParameters
{
    FloatFlag m_SpecLevel;
    FloatFlag m_EmissionLevel;
    bool m_Flag; // to indicate if dae contains profile_COMMON technique extra information
    MaterialExtraParameters()
        : m_Flag(false)
    {
    }
};

//==============================================================================
// To store information regarding material parameters which stored under profile_COMMON technique
//==============================================================================
struct MaterialParameters
{
    MatCommonProfileTechnique m_TechniqueType;
    ColorOrTexture m_Emission;
    ColorOrTexture m_Ambient;
    ColorOrTexture m_Diffuse;
    ColorOrTexture m_Specular;
    FloatFlag m_Shininess;
    ColorOrTexture m_Reflective;
    FloatFlag m_Reflectivity;
    ColorOrTexture m_Transparent;
    LongFlag m_TransparentOpaqueType;
    FloatFlag m_Transparency;
    FloatFlag m_Index_of_refraction;
    MaterialExtraParameters m_Extra;

    MaterialParameters()
        : m_TechniqueType(MatCommonProfileTechnique_Count)
    {
    }
    MaterialParameters(MatCommonProfileTechnique inTechniqueType)
        : m_TechniqueType(inTechniqueType)
    {
    }
};

//==============================================================================
// To store information regarding animation keyframes
//==============================================================================
struct KeyframeParameters
{
    float m_KeyframeTime;
    float m_Value;
    float m_INTANGENTX;
    float m_INTANGENTY;
    float m_OUTTANGENTX;
    float m_OUTTANGENTY;

    KeyframeParameters(float inKeyframeTime, float inValue, float inINTANGENTX,
                       float inINTANGENTY, float inOUTTANGENTX, float inOUTTANGENTY)
        : m_KeyframeTime(inKeyframeTime)
        , m_Value(inValue)
        , m_INTANGENTX(inINTANGENTX)
        , m_INTANGENTY(inINTANGENTY)
        , m_OUTTANGENTX(inOUTTANGENTX)
        , m_OUTTANGENTY(inOUTTANGENTY)
    {
    }

};

//==============================================================================
// To store information regarding skelatal animations
//==============================================================================
struct JointInfo
{
    int m_JointID;
    int m_ParentID;
    float m_invBindPose[16];
    float m_localToGlobalBoneSpace[16];

    JointInfo(int jointIndex, int parentID, float *invBindPose, float *localToGlobal)
        : m_JointID(jointIndex)
        , m_ParentID(parentID)
    {
        ::memcpy(m_invBindPose, invBindPose, sizeof(float) * 16);
        ::memcpy(m_localToGlobalBoneSpace, localToGlobal, sizeof(float) * 16);
    }

};

} // end QDemonAssetImport namespace

namespace QDemonMeshUtilities {
    class MeshBuilder;
}

class Q_DEMONASSETIMPORT_EXPORT QDemonSceneGraphTranslation
{
public:
    QDemonSceneGraphTranslation();
    virtual ~QDemonSceneGraphTranslation();

    virtual void reset();
    virtual void release();

    virtual void pushGroup(const QString &inName);
    virtual void setGroupSkeletonId(int inId);
    virtual void setIgnoresParentTransform(bool inValue);
    virtual void popGroup();
    virtual void pushModel(const QString &inName);
    virtual void setModelSkeletonRoot(int inId);
    virtual void popModel();
    virtual void pushMaterial(const QString &inName);
    virtual void popMaterial(int inStartFaceIndex, int inNumberOfFaces);
    virtual void pushTexture(const QString &inName, const QString &inSourcePath, int inMapType);
    virtual void popTexture();

    // Translation failed, mark top object as invalid.
    virtual void markInvalid();

    virtual void setTransforms(const QVector<QDemonAssetImport::AbstractNodeTransform *> &inTransforms);
    virtual void setGeometry(const QVector<float> &ioVertices, const QVector<float> &ioNormals,
                             const QVector<float> &ioTexCoords, const QVector<float> &ioTexCoords2,
                             const QVector<float> &ioTexTangents, const QVector<float> &ioTexBinormals,
                             const QVector<float> &ioWeights, const QVector<float> &ioBoneIndex,
                             const QVector<float> &ioColors, const QVector<quint32> &ioFaceIndicies);
    virtual void setMaterial(const QDemonAssetImport::MaterialParameters &inMaterialParameters);
    virtual void setTexture(int inMapType, const QDemonAssetImport::TextureParameters &inTextureParameters);
    virtual void setJointNode(QDemonAssetImport::JointInfo &inJointInfo);

    virtual int cacheAnimationTrack();
    virtual void applyAnimationTrack(int inAnimationTrackIndex);
    virtual void setAnimationTrack(const QString &inBasePropertyName,
                                   const QString &inSubPropertyName);
    virtual void cacheAnimationKey(const QString &inBaseProperty, const QString &inSubPropertyName,
                                   const QDemonAssetImport::KeyframeParameters &inParameters);

    virtual bool save(const QString &filename);

private:
    void insertTab();

    QByteArray m_document;
    QBuffer *m_documentBuffer = nullptr;
    QTextStream m_stream;
    int m_tabDepth = 0;

    QSharedPointer<QDemonMeshUtilities::MeshBuilder> m_meshBuilder;
 };

QT_END_NAMESPACE

#endif // QDEMONSCENEGRAPHTRANSLATION_P_H
