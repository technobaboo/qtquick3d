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
#include "qdemonrendershadercodegenerator.h"

QT_BEGIN_NAMESPACE

QDemonShaderCodeGeneratorBase::QDemonShaderCodeGeneratorBase(const QDemonRenderContextType &ctxType)
    : m_renderContextType(ctxType)
{
}

QDemonShaderCodeGeneratorBase::~QDemonShaderCodeGeneratorBase() {}
void QDemonShaderCodeGeneratorBase::begin()
{
    m_uniforms.clear();
    getVaryings().clear();
    m_attributes.clear();
    m_includes.clear();
    m_codes.clear();
    m_finalShaderBuilder.clear();
    m_codeBuilder.clear();
    m_constantBuffers.clear();
    m_constantBufferParams.clear();
}
void QDemonShaderCodeGeneratorBase::append(const QByteArray &data)
{
    m_codeBuilder.append(data);
    m_codeBuilder.append("\n");
}
// don't add the newline
void QDemonShaderCodeGeneratorBase::appendPartial(const QByteArray &data)
{
    m_codeBuilder.append(data);
}
void QDemonShaderCodeGeneratorBase::addUniform(const QByteArray &name, const QByteArray &type)
{
    m_uniforms.insert(name, type);
}
void QDemonShaderCodeGeneratorBase::addConstantBuffer(const QByteArray &name, const QByteArray &layout)
{
    m_constantBuffers.insert(name, layout);
}
void QDemonShaderCodeGeneratorBase::addConstantBufferParam(const QByteArray &cbName, const QByteArray &paramName, const QByteArray &type)
{
    TParamPair theParamPair(paramName, type);
    TConstantBufferParamPair theBufferParamPair(cbName, theParamPair);
    m_constantBufferParams.push_back(theBufferParamPair);
}

void QDemonShaderCodeGeneratorBase::addAttribute(const QByteArray &name, const QByteArray &type)
{
    m_attributes.insert(name, type);
}

void QDemonShaderCodeGeneratorBase::addVarying(const QByteArray &name, const QByteArray &type)
{
    getVaryings().insert(name, type);
}

void QDemonShaderCodeGeneratorBase::addLocalVariable(const QByteArray &name, const QByteArray &type, int tabCount)
{
    for (; tabCount >= 0; --tabCount)
        m_codeBuilder.append("\t");
    m_codeBuilder.append(type);
    m_codeBuilder.append(" ");
    m_codeBuilder.append(name);
    m_codeBuilder.append(";\n");
}

void QDemonShaderCodeGeneratorBase::addInclude(const QByteArray &name)
{
    m_includes.insert(name);
}

bool QDemonShaderCodeGeneratorBase::hasCode(Enum value)
{
    return m_codes.contains(value);
}
void QDemonShaderCodeGeneratorBase::setCode(Enum value)
{
    m_codes.insert(quint32(value));
}

void QDemonShaderCodeGeneratorBase::setupWorldPosition()
{
    if (!hasCode(WorldPosition)) {
        setCode(WorldPosition);
        addUniform("model_matrix", "mat4");
        append("\tvec3 varWorldPos = (model_matrix * vec4(attr_pos, 1.0)).xyz;");
    }
}

void QDemonShaderCodeGeneratorBase::generateViewVector()
{
    if (!hasCode(ViewVector)) {
        setCode(ViewVector);
        setupWorldPosition();
        addInclude("viewProperties.glsllib");
        append("\tvec3 view_vector = normalize(camera_position - varWorldPos);");
    }
}

void QDemonShaderCodeGeneratorBase::generateWorldNormal()
{
    if (!hasCode(WorldNormal)) {
        setCode(WorldNormal);
        addAttribute("attr_norm", "vec3");
        addUniform("normal_matrix", "mat3");
        append("\tvec3 world_normal = normalize(normal_matrix * objectNormal).xyz;");
    }
}

void QDemonShaderCodeGeneratorBase::generateEnvMapReflection(QDemonShaderCodeGeneratorBase &inFragmentShader)
{
    if (!hasCode(EnvMapReflection)) {
        setCode(EnvMapReflection);
        setupWorldPosition();
        generateWorldNormal();
        addInclude("viewProperties.glsllib");
        addVarying("var_object_to_camera", "vec3");
        append("\tvar_object_to_camera = normalize( varWorldPos - camera_position );");
        // World normal cannot be relied upon in the vertex shader because of bump maps.
        inFragmentShader.append("\tvec3 environment_map_reflection = reflect( "
                                "vec3(var_object_to_camera.x, var_object_to_camera.y, "
                                "var_object_to_camera.z), world_normal.xyz );");
        inFragmentShader.append("\tenvironment_map_reflection *= vec3( 0.5, 0.5, 0 );");
        inFragmentShader.append("\tenvironment_map_reflection += vec3( 0.5, 0.5, 1.0 );");
    }
}

void QDemonShaderCodeGeneratorBase::generateUVCoords()
{
    if (!hasCode(UVCoords)) {
        setCode(UVCoords);
        addAttribute("attr_uv0", "vec2");
        append("\tvec2 uv_coords = attr_uv0;");
    }
}

void QDemonShaderCodeGeneratorBase::generateTextureSwizzle(QDemonRenderTextureSwizzleMode::Enum swizzleMode,
                                                           QByteArray &texSwizzle,
                                                           QByteArray &lookupSwizzle)
{
    QDemonRenderContextType deprecatedContextFlags(QDemonRenderContextValues::GL2 | QDemonRenderContextValues::GLES2);

    if (!(m_renderContextType & deprecatedContextFlags)) {
        switch (swizzleMode) {
        case QDemonRenderTextureSwizzleMode::L8toR8:
        case QDemonRenderTextureSwizzleMode::L16toR16:
            texSwizzle.append(".rgb");
            lookupSwizzle.append(".rrr");
            break;
        case QDemonRenderTextureSwizzleMode::L8A8toRG8:
            texSwizzle.append(".rgba");
            lookupSwizzle.append(".rrrg");
            break;
        case QDemonRenderTextureSwizzleMode::A8toR8:
            texSwizzle.append(".a");
            lookupSwizzle.append(".r");
            break;
        default:
            break;
        }
    }
}

void QDemonShaderCodeGeneratorBase::generateShadedWireframeBase()
{
    // how this all work see
    // http://developer.download.nvidia.com/SDK/10.5/direct3d/Source/SolidWireframe/Doc/SolidWireframe.pdf
    append("// project points to screen space\n"
           "\tvec3 p0 = vec3(viewport_matrix * (gl_in[0].gl_Position / gl_in[0].gl_Position.w));\n"
           "\tvec3 p1 = vec3(viewport_matrix * (gl_in[1].gl_Position / gl_in[1].gl_Position.w));\n"
           "\tvec3 p2 = vec3(viewport_matrix * (gl_in[2].gl_Position / gl_in[2].gl_Position.w));\n"
           "// compute triangle heights\n"
           "\tfloat e1 = length(p1 - p2);\n"
           "\tfloat e2 = length(p2 - p0);\n"
           "\tfloat e3 = length(p1 - p0);\n"
           "\tfloat alpha = acos( (e2*e2 + e3*e3 - e1*e1) / (2.0*e2*e3) );\n"
           "\tfloat beta = acos( (e1*e1 + e3*e3 - e2*e2) / (2.0*e1*e3) );\n"
           "\tfloat ha = abs( e3 * sin( beta ) );\n"
           "\tfloat hb = abs( e3 * sin( alpha ) );\n"
           "\tfloat hc = abs( e2 * sin( alpha ) );\n");
}

void QDemonShaderCodeGeneratorBase::addShaderItemMap(const QByteArray &itemType, const TStrTableStrMap &itemMap)
{
    m_finalShaderBuilder.append("\n");

    for (TStrTableStrMap::const_iterator iter = itemMap.begin(), end = itemMap.end(); iter != end; ++iter) {
        m_finalShaderBuilder.append(itemType);
        m_finalShaderBuilder.append(" ");
        m_finalShaderBuilder.append(iter.value());
        m_finalShaderBuilder.append(" ");
        m_finalShaderBuilder.append(iter.key());
        m_finalShaderBuilder.append(";\n");
    }
}

void QDemonShaderCodeGeneratorBase::addShaderConstantBufferItemMap(const QByteArray &itemType,
                                                                   const TStrTableStrMap &cbMap,
                                                                   TConstantBufferParamArray cbParamsArray)
{
    m_finalShaderBuilder.append("\n");

    // iterate over all constant buffers
    for (TStrTableStrMap::const_iterator iter = cbMap.begin(), end = cbMap.end(); iter != end; ++iter) {
        m_finalShaderBuilder.append(iter.value());
        m_finalShaderBuilder.append(" ");
        m_finalShaderBuilder.append(itemType);
        m_finalShaderBuilder.append(" ");
        m_finalShaderBuilder.append(iter.key());
        m_finalShaderBuilder.append(" {\n");
        // iterate over all param entries and add match
        for (TConstantBufferParamArray::const_iterator iter1 = cbParamsArray.begin(), end = cbParamsArray.end(); iter1 != end; ++iter1) {
            if (iter1->first == iter.key()) {
                m_finalShaderBuilder.append(iter1->second.second);
                m_finalShaderBuilder.append(" ");
                m_finalShaderBuilder.append(iter1->second.first);
                m_finalShaderBuilder.append(";\n");
            }
        }

        m_finalShaderBuilder.append("};\n");
    }
}

QByteArray QDemonShaderCodeGeneratorBase::buildShaderSource()
{
    for (auto iter = m_includes.constBegin(), end = m_includes.constEnd(); iter != end; ++iter) {
        m_finalShaderBuilder.append("#include \"");
        m_finalShaderBuilder.append(*iter);
        m_finalShaderBuilder.append("\"\n");
    }
    addShaderItemMap("attribute", m_attributes);
    addShaderItemMap("uniform", m_uniforms);
    addShaderConstantBufferItemMap("uniform", m_constantBuffers, m_constantBufferParams);
    addShaderItemMap("varying", getVaryings());
    m_finalShaderBuilder.append("\n");
    m_finalShaderBuilder.append(m_codeBuilder);
    return m_finalShaderBuilder;
}

QDemonShaderCodeGeneratorBase &QDemonShaderCodeGeneratorBase::operator<<(const QByteArray &data)
{
    m_codeBuilder.append(data);
    return *this;
}

QDemonShaderVertexCodeGenerator::QDemonShaderVertexCodeGenerator(const QDemonRenderContextType &ctxType)
    : QDemonShaderCodeGeneratorBase(ctxType)
{
}
TStrTableStrMap &QDemonShaderVertexCodeGenerator::getVaryings()
{
    return m_varyings;
}

QDemonShaderTessControlCodeGenerator::QDemonShaderTessControlCodeGenerator(QDemonShaderVertexCodeGenerator &vert,
                                                                           const QDemonRenderContextType &ctxType)
    : QDemonShaderCodeGeneratorBase(ctxType), m_vertGenerator(vert)
{
}

// overwritten from base
void QDemonShaderTessControlCodeGenerator::addShaderItemMap(const QByteArray &itemType, const TStrTableStrMap &itemMap)
{
    QByteArray extVtx("");
    QByteArray extTC("");
    QByteArray type(itemType);
    if (type != QByteArrayLiteral("varying")) {
        extVtx = "[]";
        extTC = "TC[]";
        type = "attribute";
    }

    m_finalShaderBuilder.append("\n");

    for (TStrTableStrMap::const_iterator iter = itemMap.begin(), end = itemMap.end(); iter != end; ++iter) {
        m_finalShaderBuilder.append(type);
        m_finalShaderBuilder.append(" ");
        m_finalShaderBuilder.append(iter.value());
        m_finalShaderBuilder.append(" ");
        m_finalShaderBuilder.append(iter.key());
        m_finalShaderBuilder.append(extVtx);
        m_finalShaderBuilder.append(";\n");
    }

    // if this is varyings write output of tess control shader
    if (!extVtx.isEmpty()) {
        m_finalShaderBuilder.append("\n");
        type = "varying";

        for (TStrTableStrMap::const_iterator iter = itemMap.begin(), end = itemMap.end(); iter != end; ++iter) {
            m_finalShaderBuilder.append(type);
            m_finalShaderBuilder.append(" ");
            m_finalShaderBuilder.append(iter.value());
            m_finalShaderBuilder.append(" ");
            m_finalShaderBuilder.append(iter.key());
            m_finalShaderBuilder.append(extTC);
            m_finalShaderBuilder.append(";\n");
        }
    }
}
TStrTableStrMap &QDemonShaderTessControlCodeGenerator::getVaryings()
{
    return m_vertGenerator.m_varyings;
}

QDemonShaderTessEvalCodeGenerator::QDemonShaderTessEvalCodeGenerator(QDemonShaderTessControlCodeGenerator &tc,
                                                                     const QDemonRenderContextType &ctxType)
    : QDemonShaderCodeGeneratorBase(ctxType), m_tessControlGenerator(tc), m_hasGeometryStage(false)
{
}
// overwritten from base
void QDemonShaderTessEvalCodeGenerator::addShaderItemMap(const QByteArray &itemType, const TStrTableStrMap &itemMap)
{
    QByteArray extTC("");
    QByteArray extTE("");
    QByteArray type(itemType);
    if (type != QByteArrayLiteral("varying")) {
        extTC = "TC[]";
        type = "attribute";
    }
    if (m_hasGeometryStage) {
        extTE = "TE";
    }

    m_finalShaderBuilder.append("\n");

    for (TStrTableStrMap::const_iterator iter = itemMap.begin(), end = itemMap.end(); iter != end; ++iter) {
        m_finalShaderBuilder.append(type);
        m_finalShaderBuilder.append(" ");
        m_finalShaderBuilder.append(iter.value());
        m_finalShaderBuilder.append(" ");
        m_finalShaderBuilder.append(iter.key());
        m_finalShaderBuilder.append(extTC);
        m_finalShaderBuilder.append(";\n");
    }

    // if this are varyings write output of tess eval shader
    if (!extTC.isEmpty()) {
        m_finalShaderBuilder.append("\n");
        type = "varying";

        for (TStrTableStrMap::const_iterator iter = itemMap.begin(), end = itemMap.end(); iter != end; ++iter) {
            m_finalShaderBuilder.append(type);
            m_finalShaderBuilder.append(" ");
            m_finalShaderBuilder.append(iter.value());
            m_finalShaderBuilder.append(" ");
            m_finalShaderBuilder.append(iter.key());
            m_finalShaderBuilder.append(extTE);
            m_finalShaderBuilder.append(";\n");
        }
    }
}
TStrTableStrMap &QDemonShaderTessEvalCodeGenerator::getVaryings()
{
    return m_tessControlGenerator.m_vertGenerator.getVaryings();
}
void QDemonShaderTessEvalCodeGenerator::setGeometryStage(bool hasGeometryStage)
{
    m_hasGeometryStage = hasGeometryStage;
}

QDemonShaderGeometryCodeGenerator::QDemonShaderGeometryCodeGenerator(QDemonShaderVertexCodeGenerator &vert,
                                                                     const QDemonRenderContextType &ctxType)
    : QDemonShaderCodeGeneratorBase(ctxType), m_vertGenerator(vert)
{
}

// overwritten from base
void QDemonShaderGeometryCodeGenerator::addShaderItemMap(const QByteArray &itemType, const TStrTableStrMap &itemMap)
{
    QByteArray inExt("");
    QByteArray type(itemType);
    if (type != QByteArrayLiteral("varying")) {
        type = "attribute";
        if (m_hasTessellationStage)
            inExt = "TE[]";
        else
            inExt = "[]";
    }

    m_finalShaderBuilder.append("\n");

    for (TStrTableStrMap::const_iterator iter = itemMap.begin(), end = itemMap.end(); iter != end; ++iter) {
        m_finalShaderBuilder.append(type);
        m_finalShaderBuilder.append(" ");
        m_finalShaderBuilder.append(iter.value());
        m_finalShaderBuilder.append(" ");
        m_finalShaderBuilder.append(iter.key());
        m_finalShaderBuilder.append(inExt);
        m_finalShaderBuilder.append(";\n");
    }

    // if this are varyings write output of geometry shader
    if (itemType != QByteArrayLiteral("varying")) {
        m_finalShaderBuilder.append("\n");
        type = "varying";

        for (TStrTableStrMap::const_iterator iter = itemMap.begin(), end = itemMap.end(); iter != end; ++iter) {
            m_finalShaderBuilder.append(type);
            m_finalShaderBuilder.append(" ");
            m_finalShaderBuilder.append(iter.value());
            m_finalShaderBuilder.append(" ");
            m_finalShaderBuilder.append(iter.key());
            m_finalShaderBuilder.append(";\n");
        }
    }
}
TStrTableStrMap &QDemonShaderGeometryCodeGenerator::getVaryings()
{
    return m_vertGenerator.m_varyings;
}
void QDemonShaderGeometryCodeGenerator::setTessellationStage(bool hasTessellationStage)
{
    m_hasTessellationStage = hasTessellationStage;
}

QDemonShaderFragmentCodeGenerator::QDemonShaderFragmentCodeGenerator(QDemonShaderVertexCodeGenerator &vert,
                                                                     const QDemonRenderContextType &ctxType)
    : QDemonShaderCodeGeneratorBase(ctxType), m_vertGenerator(vert)
{
}
TStrTableStrMap &QDemonShaderFragmentCodeGenerator::getVaryings()
{
    return m_vertGenerator.m_varyings;
}

QT_END_NAMESPACE
