/****************************************************************************
**
** Copyright (C) 2008-2012 NVIDIA Corporation.
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Quick 3D.
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


#ifndef QDEMONOPENGLUTIL_H
#define QDEMONOPENGLUTIL_H

#include <QtDemonRender/qdemonopenglprefix.h>
#include <QtDemonRender/qdemonopengltokens.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>

#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLExtraFunctions>

#include <QVector2D>
#include <QVector3D>
#include <QVector4D>

QT_BEGIN_NAMESPACE

#ifndef GL_TEXTURE_2D_MULTISAMPLE
#define GL_TEXTURE_2D_MULTISAMPLE 0x9100
#endif

#ifndef GL_IMAGE_2D
#define GL_IMAGE_2D 0x904D
#endif

#ifndef GL_MULTISAMPLE_EXT
#define GL_MULTISAMPLE_EXT 0x809D
#endif

#ifndef GL_COLOR_ATTACHMENT1
#define GL_COLOR_ATTACHMENT1 0x8CE1
#define GL_COLOR_ATTACHMENT2 0x8CE2
#define GL_COLOR_ATTACHMENT3 0x8CE3
#define GL_COLOR_ATTACHMENT4 0x8CE4
#define GL_COLOR_ATTACHMENT5 0x8CE5
#define GL_COLOR_ATTACHMENT6 0x8CE6
#define GL_COLOR_ATTACHMENT7 0x8CE7
#endif

#ifndef GL_RED
#define GL_RED 0x1903
#define GL_GREEN 0x1904
#define GL_BLUE 0x1905
#endif

#ifndef GL_PATCHES
#define GL_PATCHES 0x000E
#endif

#ifndef GL_READ_ONLY
#define GL_READ_ONLY 0x88B8
#define GL_WRITE_ONLY 0x88B9
#define GL_READ_WRITE 0x88BA
#endif

#ifndef GL_SHADER_STORAGE_BUFFER
#define GL_SHADER_STORAGE_BUFFER 0x90D2
#endif

#ifndef GL_ATOMIC_COUNTER_BUFFER
#define GL_ATOMIC_COUNTER_BUFFER 0x92C0
#endif

#ifndef GL_DRAW_INDIRECT_BUFFER
#define GL_DRAW_INDIRECT_BUFFER 0x8F3F
#endif

#ifndef GL_VERTEX_SHADER_BIT
#define GL_VERTEX_SHADER_BIT 0x00000001
#endif

#ifndef GL_FRAGMENT_SHADER_BIT
#define GL_FRAGMENT_SHADER_BIT 0x00000002
#endif

#ifndef GL_GEOMETRY_SHADER_BIT
#define GL_GEOMETRY_SHADER_BIT 0x00000004
#endif

#ifndef GL_TESS_CONTROL_SHADER_BIT
#define GL_TESS_CONTROL_SHADER_BIT 0x00000008
#endif

#ifndef GL_TESS_EVALUATION_SHADER_BIT
#define GL_TESS_EVALUATION_SHADER_BIT 0x00000010
#endif

#define QDEMON_RENDER_ITERATE_QDEMON_GL_COLOR_FUNC                                                                     \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_ZERO, Zero)                                                                  \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_ONE, One)                                                                    \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_SRC_COLOR, SrcColor)                                                         \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_ONE_MINUS_SRC_COLOR, OneMinusSrcColor)                                       \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_DST_COLOR, DstColor)                                                         \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_ONE_MINUS_DST_COLOR, OneMinusDstColor)                                       \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_SRC_ALPHA, SrcAlpha)                                                         \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_ONE_MINUS_SRC_ALPHA, OneMinusSrcAlpha)                                       \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_DST_ALPHA, DstAlpha)                                                         \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_ONE_MINUS_DST_ALPHA, OneMinusDstAlpha)                                       \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_CONSTANT_COLOR, ConstantColor)                                               \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_ONE_MINUS_CONSTANT_COLOR, OneMinusConstantColor)                             \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_CONSTANT_ALPHA, ConstantAlpha)                                               \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_ONE_MINUS_CONSTANT_ALPHA, OneMinusConstantAlpha)                             \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC_SRC_ONLY(GL_SRC_ALPHA_SATURATE, SrcAlphaSaturate)

#define QDEMON_RENDER_ITERATE_GL_QDEMON_RENDER_FACE                                                                    \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_FACE(GL_FRONT, Front)                                                        \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_FACE(GL_BACK, Back)                                                          \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_FACE(GL_FRONT_AND_BACK, FrontAndBack)

#define QDEMON_RENDER_ITERATE_GL_QDEMON_RENDER_WINDING                                                                 \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_WINDING(GL_CW, Clockwise)                                                    \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_WINDING(GL_CCW, CounterClockwise)

#define QDEMON_RENDER_ITERATE_GL_QDEMON_BOOL_OP                                                                        \
    QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP(GL_NEVER, Never)                                                            \
    QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP(GL_LESS, Less)                                                              \
    QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP(GL_EQUAL, Equal)                                                            \
    QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP(GL_LEQUAL, LessThanOrEqual)                                                 \
    QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP(GL_GREATER, Greater)                                                        \
    QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP(GL_NOTEQUAL, NotEqual)                                                      \
    QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP(GL_GEQUAL, GreaterThanOrEqual)                                              \
    QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP(GL_ALWAYS, AlwaysTrue)

#define QDEMON_RENDER_ITERATE_GL_QDEMON_HINT                                                                           \
    QDEMON_RENDER_HANDLE_GL_QDEMON_HINT(GL_FASTEST, Fastest)                                                           \
    QDEMON_RENDER_HANDLE_GL_QDEMON_HINT(GL_NICEST, Nicest)                                                             \
    QDEMON_RENDER_HANDLE_GL_QDEMON_HINT(GL_DONT_CARE, Unspecified)

#define QDEMON_RENDER_ITERATE_QDEMON_GL_STENCIL_OP                                                                     \
    QDEMON_RENDER_HANDLE_QDEMON_GL_STENCIL_OP(GL_KEEP, Keep)                                                           \
    QDEMON_RENDER_HANDLE_QDEMON_GL_STENCIL_OP(GL_ZERO, Zero)                                                           \
    QDEMON_RENDER_HANDLE_QDEMON_GL_STENCIL_OP(GL_REPLACE, Replace)                                                     \
    QDEMON_RENDER_HANDLE_QDEMON_GL_STENCIL_OP(GL_INCR, Increment)                                                      \
    QDEMON_RENDER_HANDLE_QDEMON_GL_STENCIL_OP(GL_INCR_WRAP, IncrementWrap)                                             \
    QDEMON_RENDER_HANDLE_QDEMON_GL_STENCIL_OP(GL_DECR, Decrement)                                                      \
    QDEMON_RENDER_HANDLE_QDEMON_GL_STENCIL_OP(GL_DECR_WRAP, DecrementWrap)                                             \
    QDEMON_RENDER_HANDLE_QDEMON_GL_STENCIL_OP(GL_INVERT, Invert)

#define QDEMON_RENDER_ITERATE_GL_QDEMON_BUFFER_COMPONENT_TYPES                                                         \
    QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE(GL_UNSIGNED_BYTE, UnsignedInteger8)                                  \
    QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE(GL_BYTE, Integer8)                                                   \
    QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE(GL_UNSIGNED_SHORT, UnsignedInteger16)                                \
    QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE(GL_SHORT, Integer16)                                                 \
    QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE(GL_UNSIGNED_INT, UnsignedInteger32)                                  \
    QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE_ALIAS(GL_INT, Integer32)                                             \
    QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE(GL_FLOAT, Float32)

#define QDEMON_RENDER_ITERATE_GL_QDEMON_BUFFER_USAGE_TYPE                                                              \
    QDEMON_RENDER_HANDLE_GL_QDEMON_BUFFER_USAGE_TYPE(GL_STATIC_DRAW, Static)                                           \
    QDEMON_RENDER_HANDLE_GL_QDEMON_BUFFER_USAGE_TYPE(GL_DYNAMIC_DRAW, Dynamic)

#define QDEMON_RENDER_ITERATE_GL_QDEMON_TEXTURE_SCALE_OP                                                               \
    QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_SCALE_OP(GL_NEAREST, Nearest)                                               \
    QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_SCALE_OP(GL_LINEAR, Linear)                                                 \
    QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_MINIFYING_OP(GL_NEAREST_MIPMAP_NEAREST, NearestMipmapNearest)               \
    QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_MINIFYING_OP(GL_LINEAR_MIPMAP_NEAREST, LinearMipmapNearest)                 \
    QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_MINIFYING_OP(GL_NEAREST_MIPMAP_LINEAR, NearestMipmapLinear)                 \
    QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_MINIFYING_OP(GL_LINEAR_MIPMAP_LINEAR, LinearMipmapLinear)

#define QDEMON_RENDER_ITERATE_GL_QDEMON_TEXTURE_WRAP_OP                                                                \
    QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_WRAP_OP(GL_CLAMP_TO_EDGE, ClampToEdge)                                      \
    QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_WRAP_OP(GL_MIRRORED_REPEAT, MirroredRepeat)                                 \
    QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_WRAP_OP(GL_REPEAT, Repeat)

#define QDEMON_RENDER_ITERATE_GL_QDEMON_SHADER_UNIFORM_TYPES                                                           \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_FLOAT, Float)                                               \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_FLOAT_VEC2, Vec2)                                           \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_FLOAT_VEC3, Vec3)                                           \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_FLOAT_VEC4, Vec4)                                           \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_INT, Integer)                                               \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_INT_VEC2, IntegerVec2)                                      \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_INT_VEC3, IntegerVec3)                                      \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_INT_VEC4, IntegerVec4)                                      \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_BOOL, Boolean)                                              \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_BOOL_VEC2, BooleanVec2)                                     \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_BOOL_VEC3, BooleanVec3)                                     \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_BOOL_VEC4, BooleanVec4)                                     \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_UNSIGNED_INT, UnsignedInteger)                              \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_UNSIGNED_INT_VEC2, UnsignedIntegerVec2)                     \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_UNSIGNED_INT_VEC3, UnsignedIntegerVec3)                     \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_UNSIGNED_INT_VEC4, UnsignedIntegerVec4)                     \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_FLOAT_MAT3, Matrix3x3)                                      \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_FLOAT_MAT4, Matrix4x4)                                      \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_SAMPLER_2D, Texture2D)                                      \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_SAMPLER_2D_ARRAY, Texture2DArray)                           \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_SAMPLER_CUBE, TextureCube)                                  \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_IMAGE_2D, Image2D)
// cube Sampler and mat22 unsupported

#define QDEMON_RENDER_ITERATE_GL_QDEMON_SHADER_ATTRIB_TYPES                                                            \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_ATTRIB_TYPES(GL_FLOAT, Float32, 1)                                           \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_ATTRIB_TYPES(GL_FLOAT_VEC2, Float32, 2)                                      \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_ATTRIB_TYPES(GL_FLOAT_VEC3, Float32, 3)                                      \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_ATTRIB_TYPES(GL_FLOAT_VEC4, Float32, 4)                                      \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_ATTRIB_TYPES(GL_FLOAT_MAT2, Float32, 4)                                      \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_ATTRIB_TYPES(GL_FLOAT_MAT3, Float32, 9)                                      \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_ATTRIB_TYPES(GL_FLOAT_MAT4, Float32, 16)
#if defined(GL_DEPTH_COMPONENT32)
#define QDEMON_RENDER_ITERATE_GL_QDEMON_RENDERBUFFER_FORMATS                                                           \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_RGBA4, RGBA4)                                                \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_RGB565, RGB565)                                              \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_RGB5_A1, RGBA5551)                                           \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_DEPTH_COMPONENT16, Depth16)                                  \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_DEPTH_COMPONENT24, Depth24)                                  \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_DEPTH_COMPONENT32, Depth32)                                  \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_STENCIL_INDEX8, StencilIndex8)
#else
#define QDEMON_RENDER_ITERATE_GL_QDEMON_RENDERBUFFER_FORMATS                                                           \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_RGBA4, RGBA4)                                                \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_RGB565, RGB565)                                              \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_RGB5_A1, RGBA5551)                                           \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_DEPTH_COMPONENT16, Depth16)                                  \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_DEPTH_COMPONENT24, Depth24)                                  \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_STENCIL_INDEX8, StencilIndex8)
#endif

#define QDEMON_RENDER_ITERATE_GL_QDEMON_FRAMEBUFFER_ATTACHMENTS                                                        \
    QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_COLOR_ATTACHMENT(Color0, 0)                                             \
    QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_COLOR_ATTACHMENT(Color1, 1)                                             \
    QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_COLOR_ATTACHMENT(Color2, 2)                                             \
    QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_COLOR_ATTACHMENT(Color3, 3)                                             \
    QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_COLOR_ATTACHMENT(Color4, 4)                                             \
    QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_COLOR_ATTACHMENT(Color5, 5)                                             \
    QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_COLOR_ATTACHMENT(Color6, 6)                                             \
    QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_COLOR_ATTACHMENT(Color7, 7)                                             \
    QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_ATTACHMENT(GL_DEPTH_ATTACHMENT, Depth)                                  \
    QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_ATTACHMENT(GL_STENCIL_ATTACHMENT, Stencil)                              \
    QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_ATTACHMENT(GL_DEPTH_STENCIL_ATTACHMENT, DepthStencil)

#define QDEMON_RENDER_ITERATE_GL_QDEMON_CLEAR_FLAGS                                                                    \
    QDEMON_RENDER_HANDLE_GL_QDEMON_CLEAR_FLAGS(GL_COLOR_BUFFER_BIT, Color)                                             \
    QDEMON_RENDER_HANDLE_GL_QDEMON_CLEAR_FLAGS(GL_DEPTH_BUFFER_BIT, Depth)                                             \
    QDEMON_RENDER_HANDLE_GL_QDEMON_CLEAR_FLAGS(GL_STENCIL_BUFFER_BIT, Stencil)

#define QDEMON_RENDER_ITERATE_GL_QDEMON_RENDERBUFFER_COVERAGE_FORMATS
#define QDEMON_RENDER_ITERATE_GL_QDEMON_FRAMEBUFFER_COVERAGE_ATTACHMENTS
#define QDEMON_RENDER_ITERATE_GL_QDEMON_CLEAR_COVERAGE_FLAGS

static bool IsGlEsContext(QDemonRenderContextType inContextType)
{
    QDemonRenderContextTypes esContextTypes(QDemonRenderContextType::GLES2 | QDemonRenderContextType::GLES3
                                           | QDemonRenderContextType::GLES3PLUS);

    if (esContextTypes & inContextType)
        return true;

    return false;
}

struct GLConversion
{
    GLConversion() {}

    static const char *processGLError(GLenum error)
    {
        const char *errorString = "";
        switch (error) {
#define stringiseError(error)                                                                                          \
    case error:                                                                                                        \
        errorString = #error;                                                                                          \
        break
            stringiseError(GL_NO_ERROR);
            stringiseError(GL_INVALID_ENUM);
            stringiseError(GL_INVALID_VALUE);
            stringiseError(GL_INVALID_OPERATION);
            stringiseError(GL_INVALID_FRAMEBUFFER_OPERATION);
            stringiseError(GL_OUT_OF_MEMORY);
#undef stringiseError
        default:
            errorString = "Unknown GL error";
            break;
        }
        return errorString;
    }

    static QDemonRenderSrcBlendFunc fromGLToSrcBlendFunc(qint32 value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(srcVal, enumVal)                                                            \
    case srcVal:                                                                                                       \
        return QDemonRenderSrcBlendFunc::enumVal;
#define QDEMON_RENDER_HANDLE_GL_COLOR_FUNC_SRC_ONLY(srcVal, enumVal)                                                   \
    case srcVal:                                                                                                       \
        return QDemonRenderSrcBlendFunc::enumVal;
            QDEMON_RENDER_ITERATE_QDEMON_GL_COLOR_FUNC
#undef QDEMON_RENDER_HANDLE_GL_COLOR_FUNC
#undef QDEMON_RENDER_HANDLE_GL_COLOR_FUNC_SRC_ONLY
        default:
            Q_ASSERT(false);
            return QDemonRenderSrcBlendFunc::Unknown;
        }
    }

    static GLenum fromSrcBlendFuncToGL(QDemonRenderSrcBlendFunc value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(srcVal, enumVal)                                                            \
        case QDemonRenderSrcBlendFunc::enumVal:                                                                            \
        return srcVal;
#define QDEMON_RENDER_HANDLE_GL_COLOR_FUNC_SRC_ONLY(srcVal, enumVal)                                                   \
        case QDemonRenderSrcBlendFunc::enumVal:                                                                            \
        return srcVal;
            QDEMON_RENDER_ITERATE_QDEMON_GL_COLOR_FUNC
#undef QDEMON_RENDER_HANDLE_GL_COLOR_FUNC
#undef QDEMON_RENDER_HANDLE_GL_COLOR_FUNC_SRC_ONLY
        default:
            Q_ASSERT(false);
            return 0;
        }
    }

    static QDemonRenderDstBlendFunc fromGLToDstBlendFunc(qint32 value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(srcVal, enumVal)                                                            \
    case srcVal:                                                                                                       \
        return QDemonRenderDstBlendFunc::enumVal;
#define QDEMON_RENDER_HANDLE_GL_COLOR_FUNC_SRC_ONLY(srcVal, enumVal)
            QDEMON_RENDER_ITERATE_QDEMON_GL_COLOR_FUNC
#undef QDEMON_RENDER_HANDLE_GL_COLOR_FUNC
#undef QDEMON_RENDER_HANDLE_GL_COLOR_FUNC_SRC_ONLY
        default:
            Q_ASSERT(false);
            return QDemonRenderDstBlendFunc::Unknown;
        }
    }

    static GLenum fromDstBlendFuncToGL(QDemonRenderDstBlendFunc value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(srcVal, enumVal)                                                            \
        case QDemonRenderDstBlendFunc::enumVal:                                                                            \
        return srcVal;
#define QDEMON_RENDER_HANDLE_GL_COLOR_FUNC_SRC_ONLY(srcVal, enumVal)
            QDEMON_RENDER_ITERATE_QDEMON_GL_COLOR_FUNC
#undef QDEMON_RENDER_HANDLE_GL_COLOR_FUNC
#undef QDEMON_RENDER_HANDLE_GL_COLOR_FUNC_SRC_ONLY
        default:
            Q_ASSERT(false);
            return 0;
        }
    }

    static GLenum fromBlendEquationToGL(QDemonRenderBlendEquation value, bool nvAdvancedBlendSupported, bool khrAdvancedBlendSupported)
    {
        switch (value) {
        case QDemonRenderBlendEquation::Add:
            return GL_FUNC_ADD;
        case QDemonRenderBlendEquation::Subtract:
            return GL_FUNC_SUBTRACT;
        case QDemonRenderBlendEquation::ReverseSubtract:
            return GL_FUNC_REVERSE_SUBTRACT;
        default:
            Q_ASSERT(nvAdvancedBlendSupported || khrAdvancedBlendSupported);
            break;
        }

        if (nvAdvancedBlendSupported) {
            switch (value) {
            case QDemonRenderBlendEquation::Overlay:
                return GL_OVERLAY_NV;
            case QDemonRenderBlendEquation::ColorBurn:
                return GL_COLORBURN_NV;
            case QDemonRenderBlendEquation::ColorDodge:
                return GL_COLORDODGE_NV;
            default:
                break;
            }
        }

#if defined(GL_KHR_blend_equation_advanced)
        if (khrAdvancedBlendSupported) {
            switch (value) {
            case QDemonRenderBlendEquation::Overlay:
                return GL_OVERLAY_KHR;
            case QDemonRenderBlendEquation::ColorBurn:
                return GL_COLORBURN_KHR;
            case QDemonRenderBlendEquation::ColorDodge:
                return GL_COLORDODGE_KHR;
            default:
                break;
            }
        }
#endif

        Q_ASSERT(false);
        return GL_FUNC_ADD;
    }

    static QDemonRenderFace fromGLToFaces(GLenum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_FACE(x, y)                                                               \
    case x:                                                                                                            \
        return QDemonRenderFace::y;
            QDEMON_RENDER_ITERATE_GL_QDEMON_RENDER_FACE
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_FACE
        default:
            break;
        }
        Q_ASSERT(false);
        return QDemonRenderFace::Unknown;
    }

    static GLenum fromFacesToGL(QDemonRenderFace value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_FACE(x, y)                                                               \
    case QDemonRenderFace::y:                                                                                         \
        return x;
            QDEMON_RENDER_ITERATE_GL_QDEMON_RENDER_FACE
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_FACE
        default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static QDemonReadFace fromGLToReadFaces(GLenum value)
    {
        switch (value) {
        case GL_FRONT:
            return QDemonReadFace::Front;
        case GL_BACK:
            return QDemonReadFace::Back;
        case GL_COLOR_ATTACHMENT0:
            return QDemonReadFace::Color0;
        case GL_COLOR_ATTACHMENT1:
            return QDemonReadFace::Color1;
        case GL_COLOR_ATTACHMENT2:
            return QDemonReadFace::Color2;
        case GL_COLOR_ATTACHMENT3:
            return QDemonReadFace::Color3;
        case GL_COLOR_ATTACHMENT4:
            return QDemonReadFace::Color4;
        case GL_COLOR_ATTACHMENT5:
            return QDemonReadFace::Color5;
        case GL_COLOR_ATTACHMENT6:
            return QDemonReadFace::Color6;
        case GL_COLOR_ATTACHMENT7:
            return QDemonReadFace::Color7;

        default:
            break;
        }
        Q_ASSERT(false);
        return QDemonReadFace::Unknown;
    }

    static GLenum fromReadFacesToGL(QDemonReadFace value)
    {
        switch (value) {
        case QDemonReadFace::Front:
            return GL_FRONT;
        case QDemonReadFace::Back:
            return GL_BACK;
        case QDemonReadFace::Color0:
            return GL_COLOR_ATTACHMENT0;
        case QDemonReadFace::Color1:
            return GL_COLOR_ATTACHMENT1;
        case QDemonReadFace::Color2:
            return GL_COLOR_ATTACHMENT2;
        case QDemonReadFace::Color3:
            return GL_COLOR_ATTACHMENT3;
        case QDemonReadFace::Color4:
            return GL_COLOR_ATTACHMENT4;
        case QDemonReadFace::Color5:
            return GL_COLOR_ATTACHMENT5;
        case QDemonReadFace::Color6:
            return GL_COLOR_ATTACHMENT6;
        case QDemonReadFace::Color7:
            return GL_COLOR_ATTACHMENT7;
        default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static QDemonRenderWinding fromGLToWinding(GLenum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_WINDING(x, y)                                                            \
    case x:                                                                                                            \
        return QDemonRenderWinding::y;
            QDEMON_RENDER_ITERATE_GL_QDEMON_RENDER_WINDING
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_WINDING
        default:
            break;
        }
        Q_ASSERT(false);
        return QDemonRenderWinding::Unknown;
    }

    static GLenum fromWindingToGL(QDemonRenderWinding value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_WINDING(x, y)                                                            \
    case QDemonRenderWinding::y:                                                                                       \
        return x;
            QDEMON_RENDER_ITERATE_GL_QDEMON_RENDER_WINDING
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_WINDING
        default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static QDemonRenderBoolOp fromGLToBoolOp(GLenum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP(x, y)                                                                   \
    case x:                                                                                                            \
        return QDemonRenderBoolOp::y;
            QDEMON_RENDER_ITERATE_GL_QDEMON_BOOL_OP
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP
        default:
            break;
        }
        Q_ASSERT(false);
        return QDemonRenderBoolOp::Unknown;
    }

    static GLenum fromBoolOpToGL(QDemonRenderBoolOp value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP(x, y)                                                                   \
    case QDemonRenderBoolOp::y:                                                                                        \
        return x;
            QDEMON_RENDER_ITERATE_GL_QDEMON_BOOL_OP
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP
        default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static QDemonRenderHint fromGLToHint(GLenum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_HINT(x, y)                                                                      \
    case x:                                                                                                            \
        return QDemonRenderHint::y;
            QDEMON_RENDER_ITERATE_GL_QDEMON_HINT
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_HINT
        default:
            break;
        }
        Q_ASSERT(false);
        return QDemonRenderHint::Unknown;
    }

    static GLenum fromHintToGL(QDemonRenderHint value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_HINT(x, y)                                                                      \
    case QDemonRenderHint::y:                                                                                          \
        return x;
            QDEMON_RENDER_ITERATE_GL_QDEMON_HINT
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_HINT
        default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static QDemonRenderStencilOp fromGLToStencilOp(GLenum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_QDEMON_GL_STENCIL_OP(x, y)                                                                \
    case x:                                                                                                            \
        return QDemonRenderStencilOp::y;
            QDEMON_RENDER_ITERATE_QDEMON_GL_STENCIL_OP
#undef QDEMON_RENDER_HANDLE_QDEMON_GL_STENCIL_OP
        default:
            break;
        }

        Q_ASSERT(false);
        return QDemonRenderStencilOp::Unknown;
    }

    static GLenum fromStencilOpToGL(QDemonRenderStencilOp value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_QDEMON_GL_STENCIL_OP(x, y)                                                                \
    case QDemonRenderStencilOp::y:                                                                                     \
        return x;
            QDEMON_RENDER_ITERATE_QDEMON_GL_STENCIL_OP
#undef QDEMON_RENDER_HANDLE_QDEMON_GL_STENCIL_OP
        default:
            break;
        }

        Q_ASSERT(false);
        return 0;
    }

    static QDemonRenderComponentType fromGLToBufferComponentTypes(GLenum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE(x, y)                                                            \
    case x:                                                                                                            \
        return QDemonRenderComponentType::y;
#define QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE_ALIAS(x, y)
            QDEMON_RENDER_ITERATE_GL_QDEMON_BUFFER_COMPONENT_TYPES
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE_ALIAS
        default:
            break;
        }
        Q_ASSERT(false);
        return QDemonRenderComponentType::Unknown;
    }

    static GLenum fromBufferComponentTypesToGL(QDemonRenderComponentType value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE(x, y)                                                            \
    case QDemonRenderComponentType::y:                                                                                \
        return x;
#define QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE_ALIAS(x, y)                                                      \
    case QDemonRenderComponentType::y:                                                                                \
        return x;
            QDEMON_RENDER_ITERATE_GL_QDEMON_BUFFER_COMPONENT_TYPES
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE
        default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static GLenum fromIndexBufferComponentsTypesToGL(QDemonRenderComponentType value)
    {
        switch (value) {
        case QDemonRenderComponentType::UnsignedInteger8:
            return GL_UNSIGNED_BYTE;
        case QDemonRenderComponentType::UnsignedInteger16:
            return GL_UNSIGNED_SHORT;
        case QDemonRenderComponentType::UnsignedInteger32:
            return GL_UNSIGNED_INT;
        default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static GLenum fromBindBufferFlagsToGL(QDemonRenderBufferType type)
    {
        switch(type) {
        case QDemonRenderBufferType::Vertex:
            return GL_ARRAY_BUFFER;
        case QDemonRenderBufferType::Index:
            return GL_ELEMENT_ARRAY_BUFFER;
        case QDemonRenderBufferType::Constant:
            return GL_UNIFORM_BUFFER;
        case QDemonRenderBufferType::Storage:
            return GL_SHADER_STORAGE_BUFFER;
        case QDemonRenderBufferType::AtomicCounter:
            return GL_ATOMIC_COUNTER_BUFFER;
        case QDemonRenderBufferType::DrawIndirect:
            return GL_DRAW_INDIRECT_BUFFER;
        }
        Q_ASSERT(false);
        return 0;
    }

    static QDemonRenderBufferType fromGLToBindBufferFlags(GLenum value)
    {
        if (value == GL_ARRAY_BUFFER)
            return QDemonRenderBufferType::Vertex;
        else if (value == GL_ELEMENT_ARRAY_BUFFER)
            return QDemonRenderBufferType::Index;
        else if (value == GL_UNIFORM_BUFFER)
            return QDemonRenderBufferType::Constant;
        else if (value == GL_SHADER_STORAGE_BUFFER)
            return QDemonRenderBufferType::Storage;
        else if (value == GL_ATOMIC_COUNTER_BUFFER)
            return QDemonRenderBufferType::AtomicCounter;
        else if (value == GL_DRAW_INDIRECT_BUFFER)
            return QDemonRenderBufferType::DrawIndirect;
        else
            Q_ASSERT(false);

        return QDemonRenderBufferType(0);
    }

    static QDemonRenderBufferUsageType fromGLToBufferUsageType(GLenum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_BUFFER_USAGE_TYPE(x, y)                                                         \
    case x:                                                                                                            \
        return QDemonRenderBufferUsageType::y;
            QDEMON_RENDER_ITERATE_GL_QDEMON_BUFFER_USAGE_TYPE
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_BUFFER_USAGE_TYPE
        default:
            break;
        }
        Q_ASSERT(false);
        return QDemonRenderBufferUsageType::Unknown;
    }

    static GLenum fromBufferUsageTypeToGL(QDemonRenderBufferUsageType value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_BUFFER_USAGE_TYPE(x, y)                                                         \
    case QDemonRenderBufferUsageType::y:                                                                               \
        return x;
            QDEMON_RENDER_ITERATE_GL_QDEMON_BUFFER_USAGE_TYPE
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_BUFFER_USAGE_TYPE
        default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static GLenum fromQueryTypeToGL(QDemonRenderQueryType type)
    {
        GLenum retval = GL_INVALID_ENUM;
        if (type == QDemonRenderQueryType::Samples)
            retval = GL_ANY_SAMPLES_PASSED;
#if defined(GL_TIME_ELAPSED)
        else if (type == QDemonRenderQueryType::Timer)
            retval = GL_TIME_ELAPSED;
#elif defined(GL_TIME_ELAPSED_EXT)
        else if (type == QDemonRenderQueryType::Timer)
            retval = GL_TIME_ELAPSED_EXT;
#endif
        else
            Q_ASSERT(false);

        return retval;
    }

    static GLenum fromQueryResultTypeToGL(QDemonRenderQueryResultType type)
    {
        GLenum retval = GL_INVALID_ENUM;
        if (type == QDemonRenderQueryResultType::ResultAvailable)
            retval = GL_QUERY_RESULT_AVAILABLE;
        else if (type == QDemonRenderQueryResultType::Result)
            retval = GL_QUERY_RESULT;
        else
            Q_ASSERT(false);

        return retval;
    }

    static GLenum fromSyncTypeToGL(QDemonRenderSyncType type)
    {
        GLenum retval = GL_INVALID_ENUM;
        if (type == QDemonRenderSyncType::GpuCommandsComplete)
            retval = GL_SYNC_GPU_COMMANDS_COMPLETE;
        else
            Q_ASSERT(false);

        return retval;
    }

    static QDemonRenderTextureFormat replaceDeprecatedTextureFormat(QDemonRenderContextType type,
                                                                           QDemonRenderTextureFormat value,
                                                                           QDemonRenderTextureSwizzleMode &swizzleMode)
    {
        QDemonRenderContextTypes deprecatedContextFlags(QDemonRenderContextType::GL2 | QDemonRenderContextType::GLES2);
        QDemonRenderTextureFormat newValue = value;
        swizzleMode = QDemonRenderTextureSwizzleMode::NoSwizzle;

        if (!(deprecatedContextFlags & type)) {
            switch (value.format) {
            case QDemonRenderTextureFormat::Luminance8:
                newValue = QDemonRenderTextureFormat::R8;
                swizzleMode = QDemonRenderTextureSwizzleMode::L8toR8;
                break;
            case QDemonRenderTextureFormat::LuminanceAlpha8:
                newValue = QDemonRenderTextureFormat::RG8;
                swizzleMode = QDemonRenderTextureSwizzleMode::L8A8toRG8;
                break;
            case QDemonRenderTextureFormat::Alpha8:
                newValue = QDemonRenderTextureFormat::R8;
                swizzleMode = QDemonRenderTextureSwizzleMode::A8toR8;
                break;
            case QDemonRenderTextureFormat::Luminance16:
                newValue = QDemonRenderTextureFormat::R16;
                swizzleMode = QDemonRenderTextureSwizzleMode::L16toR16;
                break;
            default:
                break;
            }
        }

        return newValue;
    }

    static void NVRenderConvertSwizzleModeToGL(const QDemonRenderTextureSwizzleMode swizzleMode, GLint glSwizzle[4])
    {
        switch (swizzleMode) {
        case QDemonRenderTextureSwizzleMode::L16toR16:
        case QDemonRenderTextureSwizzleMode::L8toR8:
            glSwizzle[0] = GL_RED;
            glSwizzle[1] = GL_RED;
            glSwizzle[2] = GL_RED;
            glSwizzle[3] = GL_ONE;
            break;
        case QDemonRenderTextureSwizzleMode::L8A8toRG8:
            glSwizzle[0] = GL_RED;
            glSwizzle[1] = GL_RED;
            glSwizzle[2] = GL_RED;
            glSwizzle[3] = GL_GREEN;
            break;
        case QDemonRenderTextureSwizzleMode::A8toR8:
            glSwizzle[0] = GL_ZERO;
            glSwizzle[1] = GL_ZERO;
            glSwizzle[2] = GL_ZERO;
            glSwizzle[3] = GL_RED;
            break;
        case QDemonRenderTextureSwizzleMode::NoSwizzle:
        default:
            glSwizzle[0] = GL_RED;
            glSwizzle[1] = GL_GREEN;
            glSwizzle[2] = GL_BLUE;
            glSwizzle[3] = GL_ALPHA;
            break;
        }
    }

    static bool fromUncompressedTextureFormatToGL(QDemonRenderContextType type,
                                                  QDemonRenderTextureFormat value,
                                                  GLenum &outFormat,
                                                  GLenum &outDataType,
                                                  GLenum &outInternalFormat)
    {
        switch (value.format) {
        case QDemonRenderTextureFormat::R8:
            if (type == QDemonRenderContextType::GLES2) {
                outFormat = GL_ALPHA;
                outInternalFormat = GL_ALPHA;
            } else {
                outFormat = GL_RED;
                outInternalFormat = GL_R8;
            }
            outDataType = GL_UNSIGNED_BYTE;
            return true;
        case QDemonRenderTextureFormat::RG8:
            outFormat = GL_RG;
            outInternalFormat = GL_RG8;
            outDataType = GL_UNSIGNED_BYTE;
            return true;
        case QDemonRenderTextureFormat::RGBA8:
            outFormat = GL_RGBA;
            outInternalFormat = GL_RGBA8;
            outDataType = GL_UNSIGNED_BYTE;
            return true;
        case QDemonRenderTextureFormat::RGB8:
            outFormat = GL_RGB;
            outInternalFormat = GL_RGB8;
            outDataType = GL_UNSIGNED_BYTE;
            return true;
        case QDemonRenderTextureFormat::RGB565:
            outFormat = GL_RGB;
            outInternalFormat = GL_RGB8;
            outDataType = GL_UNSIGNED_SHORT_5_6_5;
            return true;
        case QDemonRenderTextureFormat::RGBA5551:
            outFormat = GL_RGBA;
            outInternalFormat = GL_RGBA8;
            outDataType = GL_UNSIGNED_SHORT_5_5_5_1;
            return true;
        case QDemonRenderTextureFormat::Alpha8:
            outFormat = GL_ALPHA;
            outInternalFormat = GL_ALPHA;
            outDataType = GL_UNSIGNED_BYTE;
            return true;
        case QDemonRenderTextureFormat::Luminance8:
            outFormat = GL_LUMINANCE;
            outInternalFormat = GL_LUMINANCE;
            outDataType = GL_UNSIGNED_BYTE;
            return true;
        case QDemonRenderTextureFormat::LuminanceAlpha8:
            outFormat = GL_LUMINANCE_ALPHA;
            outInternalFormat = GL_LUMINANCE_ALPHA;
            outDataType = GL_UNSIGNED_BYTE;
            return true;
        case QDemonRenderTextureFormat::Luminance16:
#if defined(QT_OPENGL_ES)
            outFormat = GL_LUMINANCE16F_EXT;
            outInternalFormat = GL_LUMINANCE16F_EXT;
#else
            outFormat = GL_LUMINANCE;
            outInternalFormat = GL_LUMINANCE;
#endif
            outDataType = GL_UNSIGNED_INT;
            return true;
        default:
            break;
        }

        QDemonRenderContextTypes contextFlags(QDemonRenderContextType::GL2 | QDemonRenderContextType::GLES2);
        // check extented texture formats
        if (!(contextFlags & type)) {
            switch (value.format) {
#if !defined(QT_OPENGL_ES)
            case QDemonRenderTextureFormat::R16: {
                if (IsGlEsContext(type)) {
                    outFormat = GL_RED_INTEGER;
                    outInternalFormat = GL_R16UI;
                } else {
                    outFormat = GL_RED;
                    outInternalFormat = GL_R16;
                }
                outDataType = GL_UNSIGNED_SHORT;
                return true;
            }
#endif
            case QDemonRenderTextureFormat::R16F:
                outFormat = GL_RED;
                outInternalFormat = GL_R16F;
                outDataType = GL_HALF_FLOAT;
                return true;
            case QDemonRenderTextureFormat::R32UI:
                outFormat = GL_RED_INTEGER;
                outInternalFormat = GL_R32UI;
                outDataType = GL_UNSIGNED_INT;
                return true;
            case QDemonRenderTextureFormat::R32F:
                outFormat = GL_RED;
                outInternalFormat = GL_R32F;
                outDataType = GL_FLOAT;
                return true;
            case QDemonRenderTextureFormat::RGBA16F:
                outFormat = GL_RGBA;
                outInternalFormat = GL_RGBA16F;
                outDataType = GL_HALF_FLOAT;
                return true;
            case QDemonRenderTextureFormat::RG16F:
                outFormat = GL_RG;
                outInternalFormat = GL_RG16F;
                outDataType = GL_HALF_FLOAT;
                return true;
            case QDemonRenderTextureFormat::RG32F:
                outFormat = GL_RG;
                outInternalFormat = GL_RG32F;
                outDataType = GL_FLOAT;
                return true;
            case QDemonRenderTextureFormat::RGBA32F:
                outFormat = GL_RGBA;
                outInternalFormat = GL_RGBA32F;
                outDataType = GL_FLOAT;
                return true;
            case QDemonRenderTextureFormat::RGB32F:
                outFormat = GL_RGB;
                outInternalFormat = GL_RGB32F;
                outDataType = GL_FLOAT;
                return true;
            case QDemonRenderTextureFormat::R11G11B10:
                outFormat = GL_RGB;
                outInternalFormat = GL_R11F_G11F_B10F;
                outDataType = GL_UNSIGNED_INT_10F_11F_11F_REV;
                return true;
            case QDemonRenderTextureFormat::RGB9E5:
                outFormat = GL_RGB;
                outInternalFormat = GL_RGB9_E5;
                outDataType = GL_UNSIGNED_INT_5_9_9_9_REV;
                return true;
            case QDemonRenderTextureFormat::SRGB8:
                outFormat = GL_RGB;
                outInternalFormat = GL_SRGB8;
                outDataType = GL_UNSIGNED_BYTE;
                return true;
            case QDemonRenderTextureFormat::SRGB8A8:
                outFormat = GL_RGBA;
                outInternalFormat = GL_SRGB8_ALPHA8;
                outDataType = GL_UNSIGNED_BYTE;
                return true;
            default:
                break;
            }
        }

        Q_ASSERT(false);
        return false;
    }

    static GLenum fromCompressedTextureFormatToGL(QDemonRenderTextureFormat value)
    {
        switch (value.format) {
        case QDemonRenderTextureFormat::RGBA_DXT1:
            return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        case QDemonRenderTextureFormat::RGB_DXT1:
            return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
        case QDemonRenderTextureFormat::RGBA_DXT3:
            return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        case QDemonRenderTextureFormat::RGBA_DXT5:
            return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        default:
            break;
        }

        Q_ASSERT(false);
        return 0;
    }

    static bool fromDepthTextureFormatToGL(QDemonRenderContextType type,
                                           QDemonRenderTextureFormat value,
                                           GLenum &outFormat,
                                           GLenum &outDataType,
                                           GLenum &outInternalFormat)
    {
        QDemonRenderContextTypes theContextFlags(QDemonRenderContextType::GLES2 | QDemonRenderContextType::GL2);

        bool supportDepth24 = !(theContextFlags & type);
        bool supportDepth32f = !(theContextFlags & type);
        bool supportDepth24Stencil8 = !(theContextFlags & type);

        switch (value.format) {
        case QDemonRenderTextureFormat::Depth16:
            outFormat = GL_DEPTH_COMPONENT;
            outInternalFormat = GL_DEPTH_COMPONENT16;
            outDataType = GL_UNSIGNED_SHORT;
            return true;
        case QDemonRenderTextureFormat::Depth24:
            outFormat = GL_DEPTH_COMPONENT;
            outInternalFormat = (supportDepth24) ? GL_DEPTH_COMPONENT24 : GL_DEPTH_COMPONENT16;
            outDataType = (supportDepth24) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
            return true;
        case QDemonRenderTextureFormat::Depth32:
            outFormat = GL_DEPTH_COMPONENT;
            outInternalFormat = (supportDepth32f) ? GL_DEPTH_COMPONENT32F : GL_DEPTH_COMPONENT16;
            outDataType = (supportDepth32f) ? GL_FLOAT : GL_UNSIGNED_SHORT;
            return true;
        case QDemonRenderTextureFormat::Depth24Stencil8:
            outFormat = (supportDepth24Stencil8) ? GL_DEPTH_STENCIL : GL_DEPTH_COMPONENT;
            outInternalFormat = (supportDepth24Stencil8) ? GL_DEPTH24_STENCIL8 : GL_DEPTH_COMPONENT16;
            outDataType = (supportDepth24Stencil8) ? GL_UNSIGNED_INT_24_8 : GL_UNSIGNED_SHORT;
            return true;
        default:
            break;
        }

        Q_ASSERT(false);
        return false;
    }

    static GLenum fromTextureTargetToGL(QDemonRenderTextureTargetType value)
    {
        GLenum retval = 0;
        if (value == QDemonRenderTextureTargetType::Texture2D)
            retval = GL_TEXTURE_2D;
        else if (value == QDemonRenderTextureTargetType::Texture2D_MS)
            retval = GL_TEXTURE_2D_MULTISAMPLE;
        else if (value == QDemonRenderTextureTargetType::Texture2D_Array)
            retval = GL_TEXTURE_2D_ARRAY;
        else if (value == QDemonRenderTextureTargetType::TextureCube)
            retval = GL_TEXTURE_CUBE_MAP;
        else if (value == QDemonRenderTextureTargetType::TextureCubeNegX)
            retval = GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
        else if (value == QDemonRenderTextureTargetType::TextureCubePosX)
            retval = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        else if (value == QDemonRenderTextureTargetType::TextureCubeNegY)
            retval = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
        else if (value == QDemonRenderTextureTargetType::TextureCubePosY)
            retval = GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
        else if (value == QDemonRenderTextureTargetType::TextureCubeNegZ)
            retval = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
        else if (value == QDemonRenderTextureTargetType::TextureCubePosZ)
            retval = GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
        else
            Q_ASSERT(false);

        return retval;
    }

    static QDemonRenderTextureTargetType fromGLToTextureTarget(GLenum value)
    {
        QDemonRenderTextureTargetType retval = QDemonRenderTextureTargetType::Unknown;

        if (value == GL_TEXTURE_2D)
            retval = QDemonRenderTextureTargetType::Texture2D;
        else if (value == GL_TEXTURE_2D_MULTISAMPLE)
            retval = QDemonRenderTextureTargetType::Texture2D_MS;
        else
            Q_ASSERT(false);

        return retval;
    }

    static GLenum fromTextureUnitToGL(QDemonRenderTextureUnit value)
    {
        quint32 v = static_cast<quint32>(value);
        GLenum retval = GL_TEXTURE0;
        retval = GL_TEXTURE0 + v;

        return retval;
    }

    static QDemonRenderTextureUnit fromGLToTextureUnit(GLenum value)
    {
        Q_ASSERT(value > GL_TEXTURE0);

        quint32 v = value - GL_TEXTURE0;
        QDemonRenderTextureUnit retval = QDemonRenderTextureUnit(v);

        return retval;
    }

    static GLenum fromTextureMinifyingOpToGL(QDemonRenderTextureMinifyingOp value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_SCALE_OP(x, y)                                                          \
    case QDemonRenderTextureMinifyingOp::y:                                                                            \
        return x;
#define QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_MINIFYING_OP(x, y)                                                      \
    case QDemonRenderTextureMinifyingOp::y:                                                                            \
        return x;
            QDEMON_RENDER_ITERATE_GL_QDEMON_TEXTURE_SCALE_OP
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_SCALE_OP
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_MINIFYING_OP
        default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static QDemonRenderTextureMinifyingOp fromGLToTextureMinifyingOp(GLenum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_SCALE_OP(x, y)                                                          \
    case x:                                                                                                            \
        return QDemonRenderTextureMinifyingOp::y;
#define QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_MINIFYING_OP(x, y)                                                      \
    case x:                                                                                                            \
        return QDemonRenderTextureMinifyingOp::y;
            QDEMON_RENDER_ITERATE_GL_QDEMON_TEXTURE_SCALE_OP
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_SCALE_OP
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_MINIFYING_OP
        default:
            break;
        }
        Q_ASSERT(false);
        return QDemonRenderTextureMinifyingOp::Unknown;
    }

    static GLenum fromTextureMagnifyingOpToGL(QDemonRenderTextureMagnifyingOp value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_SCALE_OP(x, y)                                                          \
    case QDemonRenderTextureMagnifyingOp::y:                                                                           \
        return x;
#define QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_MINIFYING_OP(x, y)
            QDEMON_RENDER_ITERATE_GL_QDEMON_TEXTURE_SCALE_OP
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_SCALE_OP
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_MINIFYING_OP
        default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static QDemonRenderTextureMagnifyingOp fromGLToTextureMagnifyingOp(GLenum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_SCALE_OP(x, y)                                                          \
    case x:                                                                                                            \
        return QDemonRenderTextureMagnifyingOp::y;
#define QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_MINIFYING_OP(x, y)
            QDEMON_RENDER_ITERATE_GL_QDEMON_TEXTURE_SCALE_OP
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_SCALE_OP
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_MINIFYING_OP
        default:
            break;
        }
        Q_ASSERT(false);
        return QDemonRenderTextureMagnifyingOp::Unknown;
    }

    static GLenum fromTextureCoordOpToGL(QDemonRenderTextureCoordOp value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_WRAP_OP(x, y)                                                           \
    case QDemonRenderTextureCoordOp::y:                                                                                \
        return x;
            QDEMON_RENDER_ITERATE_GL_QDEMON_TEXTURE_WRAP_OP
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_WRAP_OP
        default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static QDemonRenderTextureCoordOp fromGLToTextureCoordOp(GLenum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_WRAP_OP(x, y)                                                           \
    case x:                                                                                                            \
        return QDemonRenderTextureCoordOp::y;
            QDEMON_RENDER_ITERATE_GL_QDEMON_TEXTURE_WRAP_OP
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_WRAP_OP
        default:
            break;
        }
        Q_ASSERT(false);
        return QDemonRenderTextureCoordOp::Unknown;
    }

    static GLenum fromTextureCompareModeToGL(QDemonRenderTextureCompareMode value)
    {
        switch (value) {
        case QDemonRenderTextureCompareMode::NoCompare:
            return GL_NONE;
        case QDemonRenderTextureCompareMode::CompareToRef:
            return GL_COMPARE_REF_TO_TEXTURE;
        default:
            break;
        }

        Q_ASSERT(false);
        return GL_INVALID_ENUM;
    }

    static QDemonRenderTextureCompareMode fromGLToTextureCompareMode(GLenum value)
    {
        switch (value) {
        case GL_NONE:
            return QDemonRenderTextureCompareMode::NoCompare;
        case GL_COMPARE_REF_TO_TEXTURE:
            return QDemonRenderTextureCompareMode::CompareToRef;
        default:
            break;
        }

        Q_ASSERT(false);
        return QDemonRenderTextureCompareMode::Unknown;
    }

    static GLenum fromTextureCompareFuncToGL(QDemonRenderTextureCompareOp value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP(x, y)                                                                   \
    case QDemonRenderTextureCompareOp::y:                                                                              \
        return x;
            QDEMON_RENDER_ITERATE_GL_QDEMON_BOOL_OP
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP
        default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static GLenum fromImageFormatToGL(QDemonRenderTextureFormat value)
    {
        switch (value.format) {
        case QDemonRenderTextureFormat::R8:
            return GL_R8;
        case QDemonRenderTextureFormat::R32I:
            return GL_R32I;
        case QDemonRenderTextureFormat::R32UI:
            return GL_R32UI;
        case QDemonRenderTextureFormat::R32F:
            return GL_R32F;
        case QDemonRenderTextureFormat::RGBA8:
            return GL_RGBA8;
        case QDemonRenderTextureFormat::SRGB8A8:
            return GL_RGBA8_SNORM;
        case QDemonRenderTextureFormat::RG16F:
            return GL_RG16F;
        case QDemonRenderTextureFormat::RGBA16F:
            return GL_RGBA16F;
        case QDemonRenderTextureFormat::RGBA32F:
            return GL_RGBA32F;
        default:
            break;
        }

        Q_ASSERT(false);
        return GL_INVALID_ENUM;
    }

    static GLenum fromImageAccessToGL(QDemonRenderImageAccessType value)
    {
        switch (value) {
        case QDemonRenderImageAccessType::Read:
            return GL_READ_ONLY;
        case QDemonRenderImageAccessType::Write:
            return GL_WRITE_ONLY;
        case QDemonRenderImageAccessType::ReadWrite:
            return GL_READ_WRITE;
        default:
            break;
        }
        Q_ASSERT(false);
        return GL_INVALID_ENUM;
    }

    static GLbitfield fromBufferAccessBitToGL(QDemonRenderBufferAccessFlags flags)
    {
        GLbitfield retval = 0;

        if (flags & QDemonRenderBufferAccessTypeValues::Read)
            retval |= GL_MAP_READ_BIT;
        if (flags & QDemonRenderBufferAccessTypeValues::Write)
            retval |= GL_MAP_WRITE_BIT;
        if (flags & QDemonRenderBufferAccessTypeValues::Invalid)
            retval |= GL_MAP_INVALIDATE_BUFFER_BIT;
        if (flags & QDemonRenderBufferAccessTypeValues::InvalidRange)
            retval |= GL_MAP_INVALIDATE_RANGE_BIT;

        Q_ASSERT(retval);
        return retval;
    }

    static GLbitfield fromMemoryBarrierFlagsToGL(QDemonRenderBufferBarrierFlags flags)
    {
        GLbitfield retval = 0;
#if !defined(QT_OPENGL_ES)
        if (flags & QDemonRenderBufferBarrierValues::AtomicCounter)
            retval |= GL_ATOMIC_COUNTER_BARRIER_BIT;
        if (flags & QDemonRenderBufferBarrierValues::BufferUpdate)
            retval |= GL_BUFFER_UPDATE_BARRIER_BIT;
        if (flags & QDemonRenderBufferBarrierValues::CommandBuffer)
            retval |= GL_COMMAND_BARRIER_BIT;
        if (flags & QDemonRenderBufferBarrierValues::ElementArray)
            retval |= GL_ELEMENT_ARRAY_BARRIER_BIT;
        if (flags & QDemonRenderBufferBarrierValues::Framebuffer)
            retval |= GL_FRAMEBUFFER_BARRIER_BIT;
        if (flags & QDemonRenderBufferBarrierValues::PixelBuffer)
            retval |= GL_PIXEL_BUFFER_BARRIER_BIT;
        if (flags & QDemonRenderBufferBarrierValues::ShaderImageAccess)
            retval |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
        if (flags & QDemonRenderBufferBarrierValues::ShaderStorage)
            retval |= GL_SHADER_STORAGE_BARRIER_BIT;
        if (flags & QDemonRenderBufferBarrierValues::TextureFetch)
            retval |= GL_TEXTURE_FETCH_BARRIER_BIT;
        if (flags & QDemonRenderBufferBarrierValues::TextureUpdate)
            retval |= GL_TEXTURE_UPDATE_BARRIER_BIT;
        if (flags & QDemonRenderBufferBarrierValues::TransformFeedback)
            retval |= GL_TRANSFORM_FEEDBACK_BARRIER_BIT;
        if (flags & QDemonRenderBufferBarrierValues::UniformBuffer)
            retval |= GL_UNIFORM_BARRIER_BIT;
        if (flags & QDemonRenderBufferBarrierValues::VertexAttribArray)
            retval |= GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT;
#endif
        Q_ASSERT(retval);
        return retval;
    }

    static GLbitfield fromShaderTypeFlagsToGL(QDemonRenderShaderTypeFlags flags)
    {
        GLbitfield retval = 0;
        if (flags & QDemonRenderShaderTypeValue::Vertex)
            retval |= GL_VERTEX_SHADER_BIT;
        if (flags & QDemonRenderShaderTypeValue::Fragment)
            retval |= GL_FRAGMENT_SHADER_BIT;
        if (flags & QDemonRenderShaderTypeValue::TessControl)
            retval |= GL_TESS_CONTROL_SHADER_BIT;
        if (flags & QDemonRenderShaderTypeValue::TessEvaluation)
            retval |= GL_TESS_EVALUATION_SHADER_BIT;
        if (flags & QDemonRenderShaderTypeValue::Geometry)
#if defined(QT_OPENGL_ES_3_1)
            retval |= GL_GEOMETRY_SHADER_BIT_EXT;
#else
            retval |= GL_GEOMETRY_SHADER_BIT;
#endif
        Q_ASSERT(retval || !flags);
        return retval;
    }

    static GLenum fromPropertyDataTypesToShaderGL(QDemonRenderShaderDataType value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(gl, nv)                                                    \
    case QDemonRenderShaderDataType::nv:                                                                              \
        return gl;
            QDEMON_RENDER_ITERATE_GL_QDEMON_SHADER_UNIFORM_TYPES
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES
        default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static QDemonRenderShaderDataType fromShaderGLToPropertyDataTypes(GLenum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(gl, nv)                                                    \
    case gl:                                                                                                           \
        return QDemonRenderShaderDataType::nv;
            QDEMON_RENDER_ITERATE_GL_QDEMON_SHADER_UNIFORM_TYPES
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES
        case GL_SAMPLER_2D_SHADOW:
            return QDemonRenderShaderDataType::Texture2D;
#if !defined(QT_OPENGL_ES)
        case GL_UNSIGNED_INT_ATOMIC_COUNTER:
            return QDemonRenderShaderDataType::Integer;
        case GL_UNSIGNED_INT_IMAGE_2D:
            return QDemonRenderShaderDataType::Image2D;
#endif
        default:
            break;
        }
        Q_ASSERT(false);
        return QDemonRenderShaderDataType::Unknown;
    }

    static GLenum fromComponentTypeAndNumCompsToAttribGL(QDemonRenderComponentType compType, quint32 numComps)
    {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_ATTRIB_TYPES(gl, ct, nc)                                                 \
    if (compType == QDemonRenderComponentType::ct && numComps == nc)                                                  \
        return gl;
        QDEMON_RENDER_ITERATE_GL_QDEMON_SHADER_ATTRIB_TYPES
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_ATTRIB_TYPES
        Q_ASSERT(false);
        return 0;
    }

    static void fromAttribGLToComponentTypeAndNumComps(GLenum enumVal, QDemonRenderComponentType &outCompType, quint32 &outNumComps)
    {
        switch (enumVal) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_ATTRIB_TYPES(gl, ct, nc)                                                 \
    case gl:                                                                                                           \
        outCompType = QDemonRenderComponentType::ct;                                                                  \
        outNumComps = nc;                                                                                              \
        return;
            QDEMON_RENDER_ITERATE_GL_QDEMON_SHADER_ATTRIB_TYPES
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_ATTRIB_TYPES
        default:
            break;
        }
        Q_ASSERT(false);
        outCompType = QDemonRenderComponentType::Unknown;
        outNumComps = 0;
    }

    static GLenum fromRenderBufferFormatsToRenderBufferGL(QDemonRenderRenderBufferFormat value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(gl, nv)                                                     \
    case QDemonRenderRenderBufferFormat::nv:                                                                          \
        return gl;
            QDEMON_RENDER_ITERATE_GL_QDEMON_RENDERBUFFER_FORMATS
            QDEMON_RENDER_ITERATE_GL_QDEMON_RENDERBUFFER_COVERAGE_FORMATS
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT
        default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static QDemonRenderRenderBufferFormat fromRenderBufferGLToRenderBufferFormats(GLenum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(gl, nv)                                                     \
    case gl:                                                                                                           \
        return QDemonRenderRenderBufferFormat::nv;
            QDEMON_RENDER_ITERATE_GL_QDEMON_RENDERBUFFER_FORMATS
            QDEMON_RENDER_ITERATE_GL_QDEMON_RENDERBUFFER_COVERAGE_FORMATS
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT
        default:
            break;
        }
        Q_ASSERT(false);
        return QDemonRenderRenderBufferFormat::Unknown;
    }

    static GLenum fromFramebufferAttachmentsToGL(QDemonRenderFrameBufferAttachment value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_COLOR_ATTACHMENT(x, idx)                                            \
    case QDemonRenderFrameBufferAttachment::x:                                                                        \
        return GL_COLOR_ATTACHMENT0 + idx;
#define QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_ATTACHMENT(x, y)                                                    \
    case QDemonRenderFrameBufferAttachment::y:                                                                        \
        return x;
            QDEMON_RENDER_ITERATE_GL_QDEMON_FRAMEBUFFER_ATTACHMENTS
            QDEMON_RENDER_ITERATE_GL_QDEMON_FRAMEBUFFER_COVERAGE_ATTACHMENTS
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_COLOR_ATTACHMENT
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_ATTACHMENT
        default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static QDemonRenderFrameBufferAttachment fromGLToFramebufferAttachments(GLenum value)
    {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_COLOR_ATTACHMENT(x, idx)                                            \
    if (value == GL_COLOR_ATTACHMENT0 + idx)                                                                           \
        return QDemonRenderFrameBufferAttachment::x;
#define QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_ATTACHMENT(x, y)                                                    \
    if (value == x)                                                                                                    \
        return QDemonRenderFrameBufferAttachment::y;
        QDEMON_RENDER_ITERATE_GL_QDEMON_FRAMEBUFFER_ATTACHMENTS
        QDEMON_RENDER_ITERATE_GL_QDEMON_FRAMEBUFFER_COVERAGE_ATTACHMENTS
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_COLOR_ATTACHMENT
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_ATTACHMENT
        Q_ASSERT(false);
        return QDemonRenderFrameBufferAttachment::Unknown;
    }

    static GLbitfield fromClearFlagsToGL(QDemonRenderClearFlags flags)
    {
        GLbitfield retval = 0;
#define QDEMON_RENDER_HANDLE_GL_QDEMON_CLEAR_FLAGS(gl, nv)                                                             \
    if ((flags & QDemonRenderClearValues::nv))                                                                         \
        retval |= gl;
        QDEMON_RENDER_ITERATE_GL_QDEMON_CLEAR_FLAGS
        QDEMON_RENDER_ITERATE_GL_QDEMON_CLEAR_COVERAGE_FLAGS
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_CLEAR_FLAGS
        return retval;
    }

    static QDemonRenderClearFlags fromGLToClearFlags(GLbitfield value)
    {
        QDemonRenderClearFlags retval;
#define QDEMON_RENDER_HANDLE_GL_QDEMON_CLEAR_FLAGS(gl, nv)                                                             \
    if ((value & gl))                                                                                                  \
        retval |= QDemonRenderClearValues::nv;
        QDEMON_RENDER_ITERATE_GL_QDEMON_CLEAR_FLAGS
        QDEMON_RENDER_ITERATE_GL_QDEMON_CLEAR_COVERAGE_FLAGS
#undef QDEMON_RENDER_HANDLE_GL_QDEMON_CLEAR_FLAGS
        return retval;
    }

    static GLenum fromDrawModeToGL(QDemonRenderDrawMode value, bool inTesselationSupported)
    {
        switch (value) {
        case QDemonRenderDrawMode::Points:
            return GL_POINTS;
        case QDemonRenderDrawMode::Lines:
            return GL_LINES;
        case QDemonRenderDrawMode::LineStrip:
            return GL_LINE_STRIP;
        case QDemonRenderDrawMode::LineLoop:
            return GL_LINE_LOOP;
        case QDemonRenderDrawMode::TriangleStrip:
            return GL_TRIANGLE_STRIP;
        case QDemonRenderDrawMode::TriangleFan:
            return GL_TRIANGLE_FAN;
        case QDemonRenderDrawMode::Triangles:
            return GL_TRIANGLES;
        case QDemonRenderDrawMode::Patches:
            return (inTesselationSupported) ? GL_PATCHES : GL_TRIANGLES;
        default:
            break;
        }

        Q_ASSERT(false);
        return GL_INVALID_ENUM;
    }

    static QDemonRenderDrawMode fromGLToDrawMode(GLenum value)
    {
        switch (value) {
        case GL_POINTS:
            return QDemonRenderDrawMode::Points;
        case GL_LINES:
            return QDemonRenderDrawMode::Lines;
        case GL_LINE_STRIP:
            return QDemonRenderDrawMode::LineStrip;
        case GL_LINE_LOOP:
            return QDemonRenderDrawMode::LineLoop;
        case GL_TRIANGLE_STRIP:
            return QDemonRenderDrawMode::TriangleStrip;
        case GL_TRIANGLE_FAN:
            return QDemonRenderDrawMode::TriangleFan;
        case GL_TRIANGLES:
            return QDemonRenderDrawMode::Triangles;
        case GL_PATCHES:
            return QDemonRenderDrawMode::Patches;
        default:
            break;
        }
        Q_ASSERT(false);
        return QDemonRenderDrawMode::Unknown;
    }

    static GLenum fromRenderStateToGL(QDemonRenderState value)
    {
        switch (value) {
        case QDemonRenderState::Blend:
            return GL_BLEND;
        case QDemonRenderState::CullFace:
            return GL_CULL_FACE;
        case QDemonRenderState::DepthTest:
            return GL_DEPTH_TEST;
        case QDemonRenderState::Multisample:
#if defined(QT_OPENGL_ES)
            return GL_MULTISAMPLE_EXT;
#else
            return GL_MULTISAMPLE;
#endif
        case QDemonRenderState::StencilTest:
            return GL_STENCIL_TEST;
        case QDemonRenderState::ScissorTest:
            return GL_SCISSOR_TEST;
        default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static QDemonRenderState fromGLToRenderState(GLenum value)
    {
        switch (value) {
        case GL_BLEND:
            return QDemonRenderState::Blend;
        case GL_CULL_FACE:
            return QDemonRenderState::CullFace;
        case GL_DEPTH_TEST:
            return QDemonRenderState::DepthTest;
#if defined(QT_OPENGL_ES)
        case GL_MULTISAMPLE_EXT:
#else
        case GL_MULTISAMPLE:
#endif
            return QDemonRenderState::Multisample;
        case GL_STENCIL_TEST:
            return QDemonRenderState::StencilTest;
        case GL_SCISSOR_TEST:
            return QDemonRenderState::ScissorTest;
        default:
            break;
        }
        Q_ASSERT(false);
        return QDemonRenderState::Unknown;
    }

    static bool fromReadPixelsToGlFormatAndType(QDemonRenderReadPixelFormat inReadPixels, GLuint *outFormat, GLuint *outType)
    {
        switch (inReadPixels) {
        case QDemonRenderReadPixelFormat::Alpha8:
            *outFormat = GL_ALPHA;
            *outType = GL_UNSIGNED_BYTE;
            break;
        case QDemonRenderReadPixelFormat::RGB565:
            *outFormat = GL_RGB;
            *outType = GL_UNSIGNED_SHORT_5_6_5;
            break;
        case QDemonRenderReadPixelFormat::RGB8:
            *outFormat = GL_RGB;
            *outType = GL_UNSIGNED_BYTE;
            break;
        case QDemonRenderReadPixelFormat::RGBA4444:
            *outFormat = GL_RGBA;
            *outType = GL_UNSIGNED_SHORT_4_4_4_4;
            break;
        case QDemonRenderReadPixelFormat::RGBA5551:
            *outFormat = GL_RGBA;
            *outType = GL_UNSIGNED_SHORT_5_5_5_1;
            break;
        case QDemonRenderReadPixelFormat::RGBA8:
            *outFormat = GL_RGBA;
            *outType = GL_UNSIGNED_BYTE;
            break;
        default:
            *outFormat = 0;
            *outType = 0;
            Q_ASSERT(false);
            return false;
        };

        return true;
    }

    static GLenum fromPathFillModeToGL(QDemonRenderPathFillMode inMode)
    {
        GLenum glFillMode;

        switch (inMode) {
#if !defined(QT_OPENGL_ES)
        case QDemonRenderPathFillMode::Fill:
            glFillMode = GL_PATH_FILL_MODE_NV;
            break;
        case QDemonRenderPathFillMode::CountUp:
            glFillMode = GL_COUNT_UP_NV;
            break;
        case QDemonRenderPathFillMode::CountDown:
            glFillMode = GL_COUNT_DOWN_NV;
            break;
        case QDemonRenderPathFillMode::Invert:
            glFillMode = GL_INVERT;
            break;
#endif
        default:
            Q_ASSERT(false);
            break;
        }

        return glFillMode;
    }

    static GLenum fromPathFontTargetToGL(QDemonRenderPathFontTarget inFontTarget)
    {
        GLenum glFontTarget;

        switch (inFontTarget) {
#if !defined(QT_OPENGL_ES)
        case QDemonRenderPathFontTarget::StandardFont:
            glFontTarget = GL_STANDARD_FONT_NAME_NV;
            break;
        case QDemonRenderPathFontTarget::SystemFont:
            glFontTarget = GL_SYSTEM_FONT_NAME_NV;
            break;
        case QDemonRenderPathFontTarget::FileFont:
            glFontTarget = GL_FILE_NAME_NV;
            break;
#endif
        default:
            Q_ASSERT(false);
            break;
        }

        return glFontTarget;
    }

    static QDemonRenderPathReturnValues fromGLToPathFontReturn(GLenum inReturnValue)
    {
        QDemonRenderPathReturnValues returnValue;

        switch (inReturnValue) {
#if !defined(QT_OPENGL_ES)
        case GL_FONT_GLYPHS_AVAILABLE_NV:
            returnValue = QDemonRenderPathReturnValues::FontGlypsAvailable;
            break;
        case GL_FONT_TARGET_UNAVAILABLE_NV:
            returnValue = QDemonRenderPathReturnValues::FontTargetUnavailable;
            break;
        case GL_FONT_UNAVAILABLE_NV:
            returnValue = QDemonRenderPathReturnValues::FontUnavailable;
            break;
        case GL_FONT_UNINTELLIGIBLE_NV:
            returnValue = QDemonRenderPathReturnValues::FontUnintelligible;
            break;
#endif
        case GL_INVALID_ENUM:
        case GL_INVALID_VALUE:
            returnValue = QDemonRenderPathReturnValues::InvalidEnum;
            break;
        case GL_OUT_OF_MEMORY:
            returnValue = QDemonRenderPathReturnValues::OutOfMemory;
            break;
        default:
            Q_ASSERT(false);
            returnValue = QDemonRenderPathReturnValues::FontTargetUnavailable;
            break;
        }

        return returnValue;
    }

    static GLenum fromPathMissingGlyphsToGL(QDemonRenderPathMissingGlyphs inHandleGlyphs)
    {
        GLenum glMissingGlyphs;

        switch (inHandleGlyphs) {
#if !defined(QT_OPENGL_ES)
        case QDemonRenderPathMissingGlyphs::SkipMissing:
            glMissingGlyphs = GL_SKIP_MISSING_GLYPH_NV;
            break;
        case QDemonRenderPathMissingGlyphs::UseMissing:
            glMissingGlyphs = GL_USE_MISSING_GLYPH_NV;
            break;
#endif
        default:
            Q_ASSERT(false);
            break;
        }

        return glMissingGlyphs;
    }

    static GLenum fromPathListModeToGL(QDemonRenderPathListMode inListMode)
    {
        GLenum glListMode;

        switch (inListMode) {
#if !defined(QT_OPENGL_ES)
        case QDemonRenderPathListMode::AccumAdjacentPairs:
            glListMode = GL_ACCUM_ADJACENT_PAIRS_NV;
            break;
        case QDemonRenderPathListMode::AdjacentPairs:
            glListMode = GL_ADJACENT_PAIRS_NV;
            break;
        case QDemonRenderPathListMode::FirstToRest:
            glListMode = GL_FIRST_TO_REST_NV;
            break;
#endif
        default:
            Q_ASSERT(false);
            break;
        }

        return glListMode;
    }

    static GLenum fromPathCoverModeToGL(QDemonRenderPathCoverMode inMode)
    {
        GLenum glCoverMode;

        switch (inMode) {
#if !defined(QT_OPENGL_ES)
        case QDemonRenderPathCoverMode::ConvexHull:
            glCoverMode = GL_CONVEX_HULL_NV;
            break;
        case QDemonRenderPathCoverMode::BoundingBox:
            glCoverMode = GL_BOUNDING_BOX_NV;
            break;
        case QDemonRenderPathCoverMode::BoundingBoxOfBoundingBox:
            glCoverMode = GL_BOUNDING_BOX_OF_BOUNDING_BOXES_NV;
            break;
        case QDemonRenderPathCoverMode::PathFillCover:
            glCoverMode = GL_PATH_FILL_COVER_MODE_NV;
            break;
        case QDemonRenderPathCoverMode::PathStrokeCover:
            glCoverMode = GL_PATH_STROKE_COVER_MODE_NV;
            break;
#endif
        default:
            Q_ASSERT(false);
            break;
        }

        return glCoverMode;
    }

    static GLenum fromPathTypeToGL(QDemonRenderPathFormatType value)
    {
        switch (value) {
        case QDemonRenderPathFormatType::Byte:
            return GL_BYTE;
        case QDemonRenderPathFormatType::UByte:
            return GL_UNSIGNED_BYTE;
        case QDemonRenderPathFormatType::Short:
            return GL_SHORT;
        case QDemonRenderPathFormatType::UShort:
            return GL_UNSIGNED_SHORT;
        case QDemonRenderPathFormatType::Int:
            return GL_INT;
        case QDemonRenderPathFormatType::Uint:
            return GL_UNSIGNED_INT;
#if !defined(QT_OPENGL_ES)
        case QDemonRenderPathFormatType::Bytes2:
            return GL_2_BYTES_NV;
        case QDemonRenderPathFormatType::Bytes3:
            return GL_3_BYTES_NV;
        case QDemonRenderPathFormatType::Bytes4:
            return GL_4_BYTES_NV;
        case QDemonRenderPathFormatType::Utf8:
            return GL_UTF8_NV;
        case QDemonRenderPathFormatType::Utf16:
            return GL_UTF16_NV;
#endif
        default:
            break;
        }
        Q_ASSERT(false);
        return GL_UNSIGNED_BYTE;
    }

    static GLbitfield fromPathFontStyleToGL(QDemonRenderPathFontStyleFlags flags)
    {
        GLbitfield retval = 0;
#if !defined(QT_OPENGL_ES)
        if (flags & QDemonRenderPathFontStyleValue::Bold)
            retval |= GL_BOLD_BIT_NV;
        if (flags & QDemonRenderPathFontStyleValue::Italic)
            retval |= GL_ITALIC_BIT_NV;
#endif
        Q_ASSERT(retval || !flags);
        return retval;
    }

    static GLenum fromPathTransformToGL(QDemonRenderPathTransformType value)
    {
        switch (value) {
        case QDemonRenderPathTransformType::NoTransform:
            return GL_NONE;
#if !defined(QT_OPENGL_ES)
        case QDemonRenderPathTransformType::TranslateX:
            return GL_TRANSLATE_X_NV;
        case QDemonRenderPathTransformType::TranslateY:
            return GL_TRANSLATE_Y_NV;
        case QDemonRenderPathTransformType::Translate2D:
            return GL_TRANSLATE_2D_NV;
        case QDemonRenderPathTransformType::Translate3D:
            return GL_TRANSLATE_3D_NV;
        case QDemonRenderPathTransformType::Affine2D:
            return GL_AFFINE_2D_NV;
        case QDemonRenderPathTransformType::Affine3D:
            return GL_AFFINE_3D_NV;
        case QDemonRenderPathTransformType::TransposeAffine2D:
            return GL_TRANSPOSE_AFFINE_2D_NV;
        case QDemonRenderPathTransformType::TransposeAffine3D:
            return GL_TRANSPOSE_AFFINE_3D_NV;
#endif
        default:
            break;
        }
        Q_ASSERT(false);
        return GL_UNSIGNED_BYTE;
    }

    static GLbitfield fromPathMetricQueryFlagsToGL(QDemonRenderPathGlyphFontMetricFlags flags)
    {
        GLbitfield retval = 0;
#if !defined(QT_OPENGL_ES)
        if (flags & QDemonRenderPathGlyphFontMetricValues::GlyphWidth)
            retval |= GL_GLYPH_WIDTH_BIT_NV;
        if (flags & QDemonRenderPathGlyphFontMetricValues::GlyphHeight)
            retval |= GL_GLYPH_HEIGHT_BIT_NV;
        if (flags & QDemonRenderPathGlyphFontMetricValues::GlyphHorizontalBearingX)
            retval |= GL_GLYPH_HORIZONTAL_BEARING_X_BIT_NV;
        if (flags & QDemonRenderPathGlyphFontMetricValues::GlyphHorizontalBearingY)
            retval |= GL_GLYPH_HORIZONTAL_BEARING_Y_BIT_NV;
        if (flags & QDemonRenderPathGlyphFontMetricValues::GlyphHorizontalBearingAdvance)
            retval |= GL_GLYPH_HORIZONTAL_BEARING_ADVANCE_BIT_NV;
        if (flags & QDemonRenderPathGlyphFontMetricValues::GlyphVerticalBearingX)
            retval |= GL_GLYPH_VERTICAL_BEARING_X_BIT_NV;
        if (flags & QDemonRenderPathGlyphFontMetricValues::GlyphVerticalBearingY)
            retval |= GL_GLYPH_VERTICAL_BEARING_Y_BIT_NV;
        if (flags & QDemonRenderPathGlyphFontMetricValues::GlyphVerticalBearingAdvance)
            retval |= GL_GLYPH_VERTICAL_BEARING_ADVANCE_BIT_NV;
        if (flags & QDemonRenderPathGlyphFontMetricValues::GlyphHasKerning)
            retval |= GL_GLYPH_HAS_KERNING_BIT_NV;

        if (flags & QDemonRenderPathGlyphFontMetricValues::FontXMinBounds)
            retval |= GL_FONT_X_MIN_BOUNDS_BIT_NV;
        if (flags & QDemonRenderPathGlyphFontMetricValues::FontYMinBounds)
            retval |= GL_FONT_Y_MIN_BOUNDS_BIT_NV;
        if (flags & QDemonRenderPathGlyphFontMetricValues::FontXMaxBounds)
            retval |= GL_FONT_X_MAX_BOUNDS_BIT_NV;
        if (flags & QDemonRenderPathGlyphFontMetricValues::FontYMaxBounds)
            retval |= GL_FONT_Y_MAX_BOUNDS_BIT_NV;
        if (flags & QDemonRenderPathGlyphFontMetricValues::FontUnitsPerEm)
            retval |= GL_FONT_UNITS_PER_EM_BIT_NV;
        if (flags & QDemonRenderPathGlyphFontMetricValues::FontAscender)
            retval |= GL_FONT_ASCENDER_BIT_NV;
        if (flags & QDemonRenderPathGlyphFontMetricValues::FontDescender)
            retval |= GL_FONT_DESCENDER_BIT_NV;
        if (flags & QDemonRenderPathGlyphFontMetricValues::FontHeight)
            retval |= GL_FONT_HEIGHT_BIT_NV;
        if (flags & QDemonRenderPathGlyphFontMetricValues::FontMaxAdvanceWidth)
            retval |= GL_FONT_MAX_ADVANCE_WIDTH_BIT_NV;
        if (flags & QDemonRenderPathGlyphFontMetricValues::FontMaxAdvanceHeight)
            retval |= GL_FONT_MAX_ADVANCE_HEIGHT_BIT_NV;
        if (flags & QDemonRenderPathGlyphFontMetricValues::FontUnderlinePosition)
            retval |= GL_FONT_UNDERLINE_POSITION_BIT_NV;
        if (flags & QDemonRenderPathGlyphFontMetricValues::FontMaxAdvanceWidth)
            retval |= GL_FONT_UNDERLINE_THICKNESS_BIT_NV;
        if (flags & QDemonRenderPathGlyphFontMetricValues::FontHasKerning)
            retval |= GL_FONT_HAS_KERNING_BIT_NV;
        if (flags & QDemonRenderPathGlyphFontMetricValues::FontNumGlyphIndices)
            retval |= GL_FONT_NUM_GLYPH_INDICES_BIT_NV;
#endif
        Q_ASSERT(retval || !flags);
        return retval;
    }
};

QT_END_NAMESPACE

#endif // QDEMONOPENGLUTIL_H
