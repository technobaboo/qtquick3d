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
    virtual TransformType getType() const = 0;
};

class NodeTransform : public AbstractNodeTransform
{
public:
    NodeTransform(TransformType inType)
    {
        m_data = nullptr;
        switch (inType) {
        case TransformType_Matrix4x4:
            m_data = new float[16];
            break;
        case TransformType_Rotation3:
            m_data = new float[3];
            break;
        case TransformType_Rotation4:
            m_data = new float[4];
            break;
        case TransformType_Translate3:
            m_data = new float[3];
            break;
        case TransformType_Scale3:
            m_data = new float[3];
            break;
        default:
            break;
        }

        m_type = inType;
    }

    ~NodeTransform() override { delete[] m_data; }

    operator float *() const override { return m_data; }

    TransformType getType() const override { return m_type; }

    static void doDelete(AbstractNodeTransform *inTransform) { delete inTransform; }

protected:
    float *m_data;
    TransformType m_type;
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
    float m_value; // the value of this struct
    bool m_flag; // to indicate if this value presents in SceneGraph
    FloatFlag()
        : m_value(0.0f)
        , m_flag(false)
    {
    }
    void setValue(float inValue)
    {
        m_value = inValue;
        m_flag = true;
    }
};

struct LongFlag
{
    long m_value; // the value of this struct
    bool m_flag; // to indicate if this value presents in SceneGraph
    LongFlag()
        : m_value(0)
        , m_flag(false)
    {
    }
    void setValue(long inValue)
    {
        m_value = inValue;
        m_flag = true;
    }
};

struct ColorOrTexture
{
    enum Type {
        None, // This information doesn't present in SceneGraph or of Param Type
        Color, // Color type
        Texture, // Texture type
    };

    Type m_type;
    float m_color[4]; // the color, if present
    ColorOrTexture()
        : m_type(None)
    {
        m_color[0] = m_color[1] = m_color[2] = m_color[3] = 0.0f;
    }
    void setColor(const float inColor[])
    {
        m_color[0] = inColor[0];
        m_color[1] = inColor[1];
        m_color[2] = inColor[2];
        m_color[3] = inColor[3];
        m_type = Color;
    }
    void setColor(float c0, float c1, float c2, float c3)
    {
        m_color[0] = c0;
        m_color[1] = c1;
        m_color[2] = c2;
        m_color[3] = c3;
        m_type = Color;
    }
    void setTexture() { m_type = Texture; }
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
    bool m_flag; // to indicate if dae contains texture parameters information
    TextureParameters()
        : m_flag(false)
    {
    }
};

//==============================================================================
// To store information regarding material parameters which normally stored under <extra> tag
//==============================================================================
struct MaterialExtraParameters
{
    FloatFlag m_specLevel;
    FloatFlag m_emissionLevel;
    bool m_flag; // to indicate if dae contains profile_COMMON technique extra information
    MaterialExtraParameters()
        : m_flag(false)
    {
    }
};

//==============================================================================
// To store information regarding material parameters which stored under profile_COMMON technique
//==============================================================================
struct MaterialParameters
{
    MatCommonProfileTechnique m_techniqueType;
    ColorOrTexture m_emission;
    ColorOrTexture m_ambient;
    ColorOrTexture m_diffuse;
    ColorOrTexture m_specular;
    FloatFlag m_shininess;
    ColorOrTexture m_reflective;
    FloatFlag m_reflectivity;
    ColorOrTexture m_transparent;
    LongFlag m_transparentOpaqueType;
    FloatFlag m_transparency;
    FloatFlag m_indexOfRefraction;
    MaterialExtraParameters m_extra;

    MaterialParameters()
        : m_techniqueType(MatCommonProfileTechnique_Count)
    {
    }
    MaterialParameters(MatCommonProfileTechnique inTechniqueType)
        : m_techniqueType(inTechniqueType)
    {
    }
};

//==============================================================================
// To store information regarding animation keyframes
//==============================================================================
struct KeyframeParameters
{
    float m_keyframeTime;
    float m_value;
    float m_inTangentX;
    float m_inTangentY;
    float m_outTangentX;
    float m_outTangentY;

    KeyframeParameters(float inKeyframeTime, float inValue, float inINTANGENTX,
                       float inINTANGENTY, float inOUTTANGENTX, float inOUTTANGENTY)
        : m_keyframeTime(inKeyframeTime)
        , m_value(inValue)
        , m_inTangentX(inINTANGENTX)
        , m_inTangentY(inINTANGENTY)
        , m_outTangentX(inOUTTANGENTX)
        , m_outTangentY(inOUTTANGENTY)
    {
    }

};

//==============================================================================
// To store information regarding skelatal animations
//==============================================================================
struct JointInfo
{
    int m_jointID;
    int m_parentID;
    float m_invBindPose[16];
    float m_localToGlobalBoneSpace[16];

    JointInfo(int jointIndex, int parentID, float *invBindPose, float *localToGlobal)
        : m_jointID(jointIndex)
        , m_parentID(parentID)
    {
        ::memcpy(m_invBindPose, invBindPose, sizeof(float) * 16);
        ::memcpy(m_localToGlobalBoneSpace, localToGlobal, sizeof(float) * 16);
    }

};

} // end QDemonAssetImport namespace

namespace QDemonMeshUtilities {
    class QDemonMeshBuilder;
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

    QDemonRef<QDemonMeshUtilities::QDemonMeshBuilder> m_meshBuilder;
 };

QT_END_NAMESPACE

#endif // QDEMONSCENEGRAPHTRANSLATION_P_H
