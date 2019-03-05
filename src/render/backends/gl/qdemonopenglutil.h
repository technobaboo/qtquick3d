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

#define QDEMON_RENDER_ITERATE_QDEMON_GL_COLOR_FUNC                                                         \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_ZERO, Zero)                                                  \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_ONE, One)                                                    \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_SRC_COLOR, SrcColor)                                         \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_ONE_MINUS_SRC_COLOR, OneMinusSrcColor)                       \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_DST_COLOR, DstColor)                                         \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_ONE_MINUS_DST_COLOR, OneMinusDstColor)                       \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_SRC_ALPHA, SrcAlpha)                                         \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_ONE_MINUS_SRC_ALPHA, OneMinusSrcAlpha)                       \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_DST_ALPHA, DstAlpha)                                         \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_ONE_MINUS_DST_ALPHA, OneMinusDstAlpha)                       \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_CONSTANT_COLOR, ConstantColor)                               \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_ONE_MINUS_CONSTANT_COLOR, OneMinusConstantColor)             \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_CONSTANT_ALPHA, ConstantAlpha)                               \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(GL_ONE_MINUS_CONSTANT_ALPHA, OneMinusConstantAlpha)             \
    QDEMON_RENDER_HANDLE_GL_COLOR_FUNC_SRC_ONLY(GL_SRC_ALPHA_SATURATE, SrcAlphaSaturate)

#define QDEMON_RENDER_ITERATE_GL_QDEMON_RENDER_FACE                                                        \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_FACE(GL_FRONT, Front)                                            \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_FACE(GL_BACK, Back)                                              \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_FACE(GL_FRONT_AND_BACK, FrontAndBack)

#define QDEMON_RENDER_ITERATE_GL_QDEMON_RENDER_WINDING                                                     \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_WINDING(GL_CW, Clockwise)                                        \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_WINDING(GL_CCW, CounterClockwise)

#define QDEMON_RENDER_ITERATE_GL_QDEMON_BOOL_OP                                                            \
    QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP(GL_NEVER, Never)                                                \
    QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP(GL_LESS, Less)                                                  \
    QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP(GL_EQUAL, Equal)                                                \
    QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP(GL_LEQUAL, LessThanOrEqual)                                     \
    QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP(GL_GREATER, Greater)                                            \
    QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP(GL_NOTEQUAL, NotEqual)                                          \
    QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP(GL_GEQUAL, GreaterThanOrEqual)                                  \
    QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP(GL_ALWAYS, AlwaysTrue)

#define QDEMON_RENDER_ITERATE_GL_QDEMON_HINT                                                               \
    QDEMON_RENDER_HANDLE_GL_QDEMON_HINT(GL_FASTEST, Fastest)                                               \
    QDEMON_RENDER_HANDLE_GL_QDEMON_HINT(GL_NICEST, Nicest)                                                 \
    QDEMON_RENDER_HANDLE_GL_QDEMON_HINT(GL_DONT_CARE, Unspecified)

#define QDEMON_RENDER_ITERATE_QDEMON_GL_STENCIL_OP                                                         \
    QDEMON_RENDER_HANDLE_QDEMON_GL_STENCIL_OP(GL_KEEP, Keep)                                               \
    QDEMON_RENDER_HANDLE_QDEMON_GL_STENCIL_OP(GL_ZERO, Zero)                                               \
    QDEMON_RENDER_HANDLE_QDEMON_GL_STENCIL_OP(GL_REPLACE, Replace)                                         \
    QDEMON_RENDER_HANDLE_QDEMON_GL_STENCIL_OP(GL_INCR, Increment)                                          \
    QDEMON_RENDER_HANDLE_QDEMON_GL_STENCIL_OP(GL_INCR_WRAP, IncrementWrap)                                 \
    QDEMON_RENDER_HANDLE_QDEMON_GL_STENCIL_OP(GL_DECR, Decrement)                                          \
    QDEMON_RENDER_HANDLE_QDEMON_GL_STENCIL_OP(GL_DECR_WRAP, DecrementWrap)                                 \
    QDEMON_RENDER_HANDLE_QDEMON_GL_STENCIL_OP(GL_INVERT, Invert)

#define QDEMON_RENDER_ITERATE_GL_QDEMON_BUFFER_COMPONENT_TYPES                                             \
    QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE(GL_UNSIGNED_BYTE, UnsignedInteger8)                                  \
    QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE(GL_BYTE, Integer8)                                           \
    QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE(GL_UNSIGNED_SHORT, UnsignedInteger16)                                \
    QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE(GL_SHORT, Integer16)                                         \
    QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE(GL_UNSIGNED_INT, UnsignedInteger32)                                  \
    QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE_ALIAS(GL_INT, Integer32)                                     \
    QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE(GL_FLOAT, Float32)

#define QDEMON_RENDER_ITERATE_GL_QDEMON_BUFFER_USAGE_TYPE                                                  \
    QDEMON_RENDER_HANDLE_GL_QDEMON_BUFFER_USAGE_TYPE(GL_STATIC_DRAW, Static)                               \
    QDEMON_RENDER_HANDLE_GL_QDEMON_BUFFER_USAGE_TYPE(GL_DYNAMIC_DRAW, Dynamic)

#define QDEMON_RENDER_ITERATE_GL_QDEMON_TEXTURE_SCALE_OP                                                   \
    QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_SCALE_OP(GL_NEAREST, Nearest)                                   \
    QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_SCALE_OP(GL_LINEAR, Linear)                                     \
    QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_MINIFYING_OP(GL_NEAREST_MIPMAP_NEAREST, NearestMipmapNearest)   \
    QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_MINIFYING_OP(GL_LINEAR_MIPMAP_NEAREST, LinearMipmapNearest)     \
    QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_MINIFYING_OP(GL_NEAREST_MIPMAP_LINEAR, NearestMipmapLinear)     \
    QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_MINIFYING_OP(GL_LINEAR_MIPMAP_LINEAR, LinearMipmapLinear)

#define QDEMON_RENDER_ITERATE_GL_QDEMON_TEXTURE_WRAP_OP                                                    \
    QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_WRAP_OP(GL_CLAMP_TO_EDGE, ClampToEdge)                          \
    QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_WRAP_OP(GL_MIRRORED_REPEAT, MirroredRepeat)                     \
    QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_WRAP_OP(GL_REPEAT, Repeat)

#define QDEMON_RENDER_ITERATE_GL_QDEMON_SHADER_UNIFORM_TYPES                                               \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_FLOAT, Float)                                   \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_FLOAT_VEC2, Vec2)                             \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_FLOAT_VEC3, Vec3)                             \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_FLOAT_VEC4, Vec4)                             \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_INT, Integer)                                     \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_INT_VEC2, IntegerVec2)                              \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_INT_VEC3, IntegerVec3)                              \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_INT_VEC4, IntegerVec4)                              \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_BOOL, Boolean)                             \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_BOOL_VEC2, BooleanVec2)                              \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_BOOL_VEC3, BooleanVec3)                              \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_BOOL_VEC4, BooleanVec4)                              \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_UNSIGNED_INT, UnsignedInteger)                            \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_UNSIGNED_INT_VEC2, UnsignedIntegerVec2)                     \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_UNSIGNED_INT_VEC3, UnsignedIntegerVec3)                     \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_UNSIGNED_INT_VEC4, UnsignedIntegerVec4)                     \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_FLOAT_MAT3, Matrix3x3)                            \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_FLOAT_MAT4, Matrix4x4)                            \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_SAMPLER_2D, Texture2D)               \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_SAMPLER_2D_ARRAY, Texture2DArray)    \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_SAMPLER_CUBE, TextureCube)           \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(GL_IMAGE_2D, Image2D)
// cube Sampler and mat22 unsupported

#define QDEMON_RENDER_ITERATE_GL_QDEMON_SHADER_ATTRIB_TYPES                                                \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_ATTRIB_TYPES(GL_FLOAT, Float32, 1)                                 \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_ATTRIB_TYPES(GL_FLOAT_VEC2, Float32, 2)                            \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_ATTRIB_TYPES(GL_FLOAT_VEC3, Float32, 3)                            \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_ATTRIB_TYPES(GL_FLOAT_VEC4, Float32, 4)                            \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_ATTRIB_TYPES(GL_FLOAT_MAT2, Float32, 4)                            \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_ATTRIB_TYPES(GL_FLOAT_MAT3, Float32, 9)                            \
    QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_ATTRIB_TYPES(GL_FLOAT_MAT4, Float32, 16)
#if defined(GL_DEPTH_COMPONENT32)
#define QDEMON_RENDER_ITERATE_GL_QDEMON_RENDERBUFFER_FORMATS                                               \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_RGBA4, RGBA4)                                    \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_RGB565, RGB565)                                  \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_RGB5_A1, RGBA5551)                               \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_DEPTH_COMPONENT16, Depth16)                      \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_DEPTH_COMPONENT24, Depth24)                      \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_DEPTH_COMPONENT32, Depth32)                      \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_STENCIL_INDEX8, StencilIndex8)
#else
#define QDEMON_RENDER_ITERATE_GL_QDEMON_RENDERBUFFER_FORMATS                                               \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_RGBA4, RGBA4)                                    \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_RGB565, RGB565)                                  \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_RGB5_A1, RGBA5551)                               \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_DEPTH_COMPONENT16, Depth16)                      \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_DEPTH_COMPONENT24, Depth24)                      \
    QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(GL_STENCIL_INDEX8, StencilIndex8)
#endif

#define QDEMON_RENDER_ITERATE_GL_QDEMON_FRAMEBUFFER_ATTACHMENTS                                            \
    QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_COLOR_ATTACHMENT(Color0, 0)                                 \
    QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_COLOR_ATTACHMENT(Color1, 1)                                 \
    QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_COLOR_ATTACHMENT(Color2, 2)                                 \
    QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_COLOR_ATTACHMENT(Color3, 3)                                 \
    QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_COLOR_ATTACHMENT(Color4, 4)                                 \
    QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_COLOR_ATTACHMENT(Color5, 5)                                 \
    QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_COLOR_ATTACHMENT(Color6, 6)                                 \
    QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_COLOR_ATTACHMENT(Color7, 7)                                 \
    QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_ATTACHMENT(GL_DEPTH_ATTACHMENT, Depth)                      \
    QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_ATTACHMENT(GL_STENCIL_ATTACHMENT, Stencil)                  \
    QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_ATTACHMENT(GL_DEPTH_STENCIL_ATTACHMENT, DepthStencil)

#define QDEMON_RENDER_ITERATE_GL_QDEMON_CLEAR_FLAGS                                                        \
    QDEMON_RENDER_HANDLE_GL_QDEMON_CLEAR_FLAGS(GL_COLOR_BUFFER_BIT, Color)                                 \
    QDEMON_RENDER_HANDLE_GL_QDEMON_CLEAR_FLAGS(GL_DEPTH_BUFFER_BIT, Depth)                                 \
    QDEMON_RENDER_HANDLE_GL_QDEMON_CLEAR_FLAGS(GL_STENCIL_BUFFER_BIT, Stencil)

#define QDEMON_RENDER_ITERATE_GL_QDEMON_RENDERBUFFER_COVERAGE_FORMATS
#define QDEMON_RENDER_ITERATE_GL_QDEMON_FRAMEBUFFER_COVERAGE_ATTACHMENTS
#define QDEMON_RENDER_ITERATE_GL_QDEMON_CLEAR_COVERAGE_FLAGS

static bool IsGlEsContext(QDemonRenderContextType inContextType)
{
    QDemonRenderContextType esContextTypes(QDemonRenderContextValues::GLES2
                                           | QDemonRenderContextValues::GLES3
                                           | QDemonRenderContextValues::GLES3PLUS);

    if ((inContextType & esContextTypes))
        return true;

    return false;
}

struct GLConversion
{
    GLConversion()
    { }

    static const char *processGLError(GLenum error)
    {
        const char *errorString = "";
        switch (error) {
#define stringiseError(error)                                                                      \
        case error:                                                                                    \
    errorString = #error;                                                                      \
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

    static QDemonRenderSrcBlendFunc::Enum fromGLToSrcBlendFunc(qint32 value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(srcVal, enumVal)                                            \
        case srcVal:                                                                                   \
    return QDemonRenderSrcBlendFunc::enumVal;
#define QDEMON_RENDER_HANDLE_GL_COLOR_FUNC_SRC_ONLY(srcVal, enumVal)                                   \
        case srcVal:                                                                                   \
    return QDemonRenderSrcBlendFunc::enumVal;
        QDEMON_RENDER_ITERATE_QDEMON_GL_COLOR_FUNC
        #undef QDEMON_RENDER_HANDLE_GL_COLOR_FUNC
        #undef QDEMON_RENDER_HANDLE_GL_COLOR_FUNC_SRC_ONLY
                default:
            Q_ASSERT(false);
        return QDemonRenderSrcBlendFunc::Unknown;
        }
    }

    static GLenum fromSrcBlendFuncToGL(QDemonRenderSrcBlendFunc::Enum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(srcVal, enumVal)                                            \
        case QDemonRenderSrcBlendFunc::enumVal:                                                            \
    return srcVal;
#define QDEMON_RENDER_HANDLE_GL_COLOR_FUNC_SRC_ONLY(srcVal, enumVal)                                   \
        case QDemonRenderSrcBlendFunc::enumVal:                                                            \
    return srcVal;
        QDEMON_RENDER_ITERATE_QDEMON_GL_COLOR_FUNC
        #undef QDEMON_RENDER_HANDLE_GL_COLOR_FUNC
        #undef QDEMON_RENDER_HANDLE_GL_COLOR_FUNC_SRC_ONLY
                default:
            Q_ASSERT(false);
        return 0;
        }
    }

    static QDemonRenderDstBlendFunc::Enum fromGLToDstBlendFunc(qint32 value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(srcVal, enumVal)                                            \
        case srcVal:                                                                                   \
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

    static GLenum fromDstBlendFuncToGL(QDemonRenderDstBlendFunc::Enum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_COLOR_FUNC(srcVal, enumVal)                                            \
        case QDemonRenderDstBlendFunc::enumVal:                                                            \
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

    static GLenum fromBlendEquationToGL(QDemonRenderBlendEquation::Enum value,
                                        bool nvAdvancedBlendSupported,
                                        bool khrAdvancedBlendSupported)
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

    static QDemonRenderFaces::Enum fromGLToFaces(GLenum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_FACE(x, y)                                                   \
        case x:                                                                                        \
    return QDemonRenderFaces::y;
        QDEMON_RENDER_ITERATE_GL_QDEMON_RENDER_FACE
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_FACE
                default:
            break;
        }
        Q_ASSERT(false);
        return QDemonRenderFaces::Unknown;
    }

    static GLenum fromFacesToGL(QDemonRenderFaces::Enum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_FACE(x, y)                                                   \
        case QDemonRenderFaces::y:                                                                         \
    return x;
        QDEMON_RENDER_ITERATE_GL_QDEMON_RENDER_FACE
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_FACE
                default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static QDemonReadFaces::Enum fromGLToReadFaces(GLenum value)
    {
        switch (value) {
        case GL_FRONT:
            return QDemonReadFaces::Front;
        case GL_BACK:
            return QDemonReadFaces::Back;
        case GL_COLOR_ATTACHMENT0:
            return QDemonReadFaces::Color0;
        case GL_COLOR_ATTACHMENT1:
            return QDemonReadFaces::Color1;
        case GL_COLOR_ATTACHMENT2:
            return QDemonReadFaces::Color2;
        case GL_COLOR_ATTACHMENT3:
            return QDemonReadFaces::Color3;
        case GL_COLOR_ATTACHMENT4:
            return QDemonReadFaces::Color4;
        case GL_COLOR_ATTACHMENT5:
            return QDemonReadFaces::Color5;
        case GL_COLOR_ATTACHMENT6:
            return QDemonReadFaces::Color6;
        case GL_COLOR_ATTACHMENT7:
            return QDemonReadFaces::Color7;

        default:
            break;
        }
        Q_ASSERT(false);
        return QDemonReadFaces::Unknown;
    }

    static GLenum fromReadFacesToGL(QDemonReadFaces::Enum value)
    {
        switch (value) {
        case QDemonReadFaces::Front:
            return GL_FRONT;
        case QDemonReadFaces::Back:
            return GL_BACK;
        case QDemonReadFaces::Color0:
            return GL_COLOR_ATTACHMENT0;
        case QDemonReadFaces::Color1:
            return GL_COLOR_ATTACHMENT1;
        case QDemonReadFaces::Color2:
            return GL_COLOR_ATTACHMENT2;
        case QDemonReadFaces::Color3:
            return GL_COLOR_ATTACHMENT3;
        case QDemonReadFaces::Color4:
            return GL_COLOR_ATTACHMENT4;
        case QDemonReadFaces::Color5:
            return GL_COLOR_ATTACHMENT5;
        case QDemonReadFaces::Color6:
            return GL_COLOR_ATTACHMENT6;
        case QDemonReadFaces::Color7:
            return GL_COLOR_ATTACHMENT7;
        default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static QDemonRenderWinding::Enum fromGLToWinding(GLenum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_WINDING(x, y)                                                \
        case x:                                                                                        \
    return QDemonRenderWinding::y;
        QDEMON_RENDER_ITERATE_GL_QDEMON_RENDER_WINDING
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_WINDING
                default:
            break;
        }
        Q_ASSERT(false);
        return QDemonRenderWinding::Unknown;
    }

    static GLenum fromWindingToGL(QDemonRenderWinding::Enum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_WINDING(x, y)                                                \
        case QDemonRenderWinding::y:                                                                       \
    return x;
        QDEMON_RENDER_ITERATE_GL_QDEMON_RENDER_WINDING
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_RENDER_WINDING
                default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static QDemonRenderBoolOp::Enum fromGLToBoolOp(GLenum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP(x, y)                                                       \
        case x:                                                                                        \
    return QDemonRenderBoolOp::y;
        QDEMON_RENDER_ITERATE_GL_QDEMON_BOOL_OP
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP
                default:
            break;
        }
        Q_ASSERT(false);
        return QDemonRenderBoolOp::Unknown;
    }

    static GLenum fromBoolOpToGL(QDemonRenderBoolOp::Enum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP(x, y)                                                       \
        case QDemonRenderBoolOp::y:                                                                        \
    return x;
        QDEMON_RENDER_ITERATE_GL_QDEMON_BOOL_OP
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP
                default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static QDemonRenderHint::Enum fromGLToHint(GLenum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_HINT(x, y)                                                          \
        case x:                                                                                        \
    return QDemonRenderHint::y;
        QDEMON_RENDER_ITERATE_GL_QDEMON_HINT
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_HINT
                default:
            break;
        }
        Q_ASSERT(false);
        return QDemonRenderHint::Unknown;
    }

    static GLenum fromHintToGL(QDemonRenderHint::Enum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_HINT(x, y)                                                          \
        case QDemonRenderHint::y:                                                                          \
    return x;
        QDEMON_RENDER_ITERATE_GL_QDEMON_HINT
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_HINT
                default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static QDemonRenderStencilOp::Enum fromGLToStencilOp(GLenum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_QDEMON_GL_STENCIL_OP(x, y)                                                    \
        case x:                                                                                        \
    return QDemonRenderStencilOp::y;
        QDEMON_RENDER_ITERATE_QDEMON_GL_STENCIL_OP
        #undef QDEMON_RENDER_HANDLE_QDEMON_GL_STENCIL_OP
                default:
            break;
        }

        Q_ASSERT(false);
        return QDemonRenderStencilOp::Unknown;
    }

    static GLenum fromStencilOpToGL(QDemonRenderStencilOp::Enum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_QDEMON_GL_STENCIL_OP(x, y)                                                    \
        case QDemonRenderStencilOp::y:                                                                     \
    return x;
        QDEMON_RENDER_ITERATE_QDEMON_GL_STENCIL_OP
        #undef QDEMON_RENDER_HANDLE_QDEMON_GL_STENCIL_OP
                default:
            break;
        }

        Q_ASSERT(false);
        return 0;
    }

    static QDemonRenderComponentTypes::Enum fromGLToBufferComponentTypes(GLenum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE(x, y)                                                \
        case x:                                                                                        \
    return QDemonRenderComponentTypes::y;
#define QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE_ALIAS(x, y)
        QDEMON_RENDER_ITERATE_GL_QDEMON_BUFFER_COMPONENT_TYPES
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE_ALIAS
                default:
            break;
        }
        Q_ASSERT(false);
        return QDemonRenderComponentTypes::Unknown;
    }

    static GLenum fromBufferComponentTypesToGL(QDemonRenderComponentTypes::Enum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE(x, y)                                                \
        case QDemonRenderComponentTypes::y:                                                                \
    return x;
#define QDEMON_RENDER_HANDLE_GL_QDEMON_COMPONENT_TYPE_ALIAS(x, y)                                          \
        case QDemonRenderComponentTypes::y:                                                                \
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

    static GLenum fromIndexBufferComponentsTypesToGL(QDemonRenderComponentTypes::Enum value)
    {
        switch (value) {
        case QDemonRenderComponentTypes::UnsignedInteger8:
            return GL_UNSIGNED_BYTE;
        case QDemonRenderComponentTypes::UnsignedInteger16:
            return GL_UNSIGNED_SHORT;
        case QDemonRenderComponentTypes::UnsignedInteger32:
            return GL_UNSIGNED_INT;
        default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static GLenum fromBindBufferFlagsToGL(QDemonRenderBufferBindFlags flags)
    {
        quint32 value = flags;
        GLenum retval = GL_INVALID_ENUM;
        if (value & QDemonRenderBufferBindValues::Vertex)
            retval = GL_ARRAY_BUFFER;
        else if (value & QDemonRenderBufferBindValues::Index)
            retval = GL_ELEMENT_ARRAY_BUFFER;
        else if (value & QDemonRenderBufferBindValues::Constant)
            retval = GL_UNIFORM_BUFFER;
        else if (value & QDemonRenderBufferBindValues::Storage)
            retval = GL_SHADER_STORAGE_BUFFER;
        else if (value & QDemonRenderBufferBindValues::Atomic_Counter)
            retval = GL_ATOMIC_COUNTER_BUFFER;
        else if (value & QDemonRenderBufferBindValues::Draw_Indirect)
            retval = GL_DRAW_INDIRECT_BUFFER;
        else
            Q_ASSERT(false);

        return retval;
    }

    static QDemonRenderBufferBindFlags fromGLToBindBufferFlags(GLenum value)
    {
        quint32 retval = 0;

        if (value == GL_ARRAY_BUFFER)
            retval |= QDemonRenderBufferBindValues::Vertex;
        else if (value == GL_ELEMENT_ARRAY_BUFFER)
            retval |= QDemonRenderBufferBindValues::Index;
        else if (value == GL_UNIFORM_BUFFER)
            retval |= QDemonRenderBufferBindValues::Constant;
        else if (value == GL_SHADER_STORAGE_BUFFER)
            retval |= QDemonRenderBufferBindValues::Storage;
        else if (value == GL_ATOMIC_COUNTER_BUFFER)
            retval |= QDemonRenderBufferBindValues::Atomic_Counter;
        else if (value == GL_DRAW_INDIRECT_BUFFER)
            retval |= QDemonRenderBufferBindValues::Draw_Indirect;
        else
            Q_ASSERT(false);

        return QDemonRenderBufferBindFlags(retval);
    }

    static QDemonRenderBufferUsageType::Enum fromGLToBufferUsageType(GLenum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_BUFFER_USAGE_TYPE(x, y)                                             \
        case x:                                                                                        \
    return QDemonRenderBufferUsageType::y;
        QDEMON_RENDER_ITERATE_GL_QDEMON_BUFFER_USAGE_TYPE
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_BUFFER_USAGE_TYPE
                default:
            break;
        }
        Q_ASSERT(false);
        return QDemonRenderBufferUsageType::Unknown;
    }

    static GLenum fromBufferUsageTypeToGL(QDemonRenderBufferUsageType::Enum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_BUFFER_USAGE_TYPE(x, y)                                             \
        case QDemonRenderBufferUsageType::y:                                                               \
    return x;
        QDEMON_RENDER_ITERATE_GL_QDEMON_BUFFER_USAGE_TYPE
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_BUFFER_USAGE_TYPE
                default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static GLenum fromQueryTypeToGL(QDemonRenderQueryType::Enum type)
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

    static GLenum fromQueryResultTypeToGL(QDemonRenderQueryResultType::Enum type)
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

    static GLenum fromSyncTypeToGL(QDemonRenderSyncType::Enum type)
    {
        GLenum retval = GL_INVALID_ENUM;
        if (type == QDemonRenderSyncType::GpuCommandsComplete)
            retval = GL_SYNC_GPU_COMMANDS_COMPLETE;
        else
            Q_ASSERT(false);

        return retval;
    }

    static QDemonRenderTextureFormats::Enum
    replaceDeprecatedTextureFormat(QDemonRenderContextType type, QDemonRenderTextureFormats::Enum value,
                                   QDemonRenderTextureSwizzleMode::Enum &swizzleMode)
    {
        QDemonRenderContextType deprecatedContextFlags(QDemonRenderContextValues::GL2
                                                       | QDemonRenderContextValues::GLES2);
        QDemonRenderTextureFormats::Enum newValue = value;
        swizzleMode = QDemonRenderTextureSwizzleMode::NoSwizzle;

        if (!(type & deprecatedContextFlags)) {
            switch (value) {
            case QDemonRenderTextureFormats::Luminance8:
                newValue = QDemonRenderTextureFormats::R8;
                swizzleMode = QDemonRenderTextureSwizzleMode::L8toR8;
                break;
            case QDemonRenderTextureFormats::LuminanceAlpha8:
                newValue = QDemonRenderTextureFormats::RG8;
                swizzleMode = QDemonRenderTextureSwizzleMode::L8A8toRG8;
                break;
            case QDemonRenderTextureFormats::Alpha8:
                newValue = QDemonRenderTextureFormats::R8;
                swizzleMode = QDemonRenderTextureSwizzleMode::A8toR8;
                break;
            case QDemonRenderTextureFormats::Luminance16:
                newValue = QDemonRenderTextureFormats::R16;
                swizzleMode = QDemonRenderTextureSwizzleMode::L16toR16;
                break;
            default:
                break;
            }
        }

        return newValue;
    }

    static void
    NVRenderConvertSwizzleModeToGL(const QDemonRenderTextureSwizzleMode::Enum swizzleMode,
                                   GLint glSwizzle[4])
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
                                                  QDemonRenderTextureFormats::Enum value,
                                                  GLenum &outFormat, GLenum &outDataType,
                                                  GLenum &outInternalFormat)
    {
        switch (value) {
        case QDemonRenderTextureFormats::R8:
            if (type == QDemonRenderContextValues::GLES2) {
                outFormat = GL_ALPHA;
                outInternalFormat = GL_ALPHA;
            } else {
                outFormat = GL_RED;
                outInternalFormat = GL_R8;
            }
            outDataType = GL_UNSIGNED_BYTE;
            return true;
        case QDemonRenderTextureFormats::RG8:
            outFormat = GL_RG;
            outInternalFormat = GL_RG8;
            outDataType = GL_UNSIGNED_BYTE;
            return true;
        case QDemonRenderTextureFormats::RGBA8:
            outFormat = GL_RGBA;
            outInternalFormat = GL_RGBA8;
            outDataType = GL_UNSIGNED_BYTE;
            return true;
        case QDemonRenderTextureFormats::RGB8:
            outFormat = GL_RGB;
            outInternalFormat = GL_RGB8;
            outDataType = GL_UNSIGNED_BYTE;
            return true;
        case QDemonRenderTextureFormats::RGB565:
            outFormat = GL_RGB;
            outInternalFormat = GL_RGB8;
            outDataType = GL_UNSIGNED_SHORT_5_6_5;
            return true;
        case QDemonRenderTextureFormats::RGBA5551:
            outFormat = GL_RGBA;
            outInternalFormat = GL_RGBA8;
            outDataType = GL_UNSIGNED_SHORT_5_5_5_1;
            return true;
        case QDemonRenderTextureFormats::Alpha8:
            outFormat = GL_ALPHA;
            outInternalFormat = GL_ALPHA;
            outDataType = GL_UNSIGNED_BYTE;
            return true;
        case QDemonRenderTextureFormats::Luminance8:
            outFormat = GL_LUMINANCE;
            outInternalFormat = GL_LUMINANCE;
            outDataType = GL_UNSIGNED_BYTE;
            return true;
        case QDemonRenderTextureFormats::LuminanceAlpha8:
            outFormat = GL_LUMINANCE_ALPHA;
            outInternalFormat = GL_LUMINANCE_ALPHA;
            outDataType = GL_UNSIGNED_BYTE;
            return true;
        case QDemonRenderTextureFormats::Luminance16:
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

        QDemonRenderContextType contextFlags(QDemonRenderContextValues::GL2
                                             | QDemonRenderContextValues::GLES2);
        // check extented texture formats
        if (!(type & contextFlags)) {
            switch (value) {
#if !defined(QT_OPENGL_ES)
            case QDemonRenderTextureFormats::R16: {
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
            case QDemonRenderTextureFormats::R16F:
                outFormat = GL_RED;
                outInternalFormat = GL_R16F;
                outDataType = GL_HALF_FLOAT;
                return true;
            case QDemonRenderTextureFormats::R32UI:
                outFormat = GL_RED_INTEGER;
                outInternalFormat = GL_R32UI;
                outDataType = GL_UNSIGNED_INT;
                return true;
            case QDemonRenderTextureFormats::R32F:
                outFormat = GL_RED;
                outInternalFormat = GL_R32F;
                outDataType = GL_FLOAT;
                return true;
            case QDemonRenderTextureFormats::RGBA16F:
                outFormat = GL_RGBA;
                outInternalFormat = GL_RGBA16F;
                outDataType = GL_HALF_FLOAT;
                return true;
            case QDemonRenderTextureFormats::RG16F:
                outFormat = GL_RG;
                outInternalFormat = GL_RG16F;
                outDataType = GL_HALF_FLOAT;
                return true;
            case QDemonRenderTextureFormats::RG32F:
                outFormat = GL_RG;
                outInternalFormat = GL_RG32F;
                outDataType = GL_FLOAT;
                return true;
            case QDemonRenderTextureFormats::RGBA32F:
                outFormat = GL_RGBA;
                outInternalFormat = GL_RGBA32F;
                outDataType = GL_FLOAT;
                return true;
            case QDemonRenderTextureFormats::RGB32F:
                outFormat = GL_RGB;
                outInternalFormat = GL_RGB32F;
                outDataType = GL_FLOAT;
                return true;
            case QDemonRenderTextureFormats::R11G11B10:
                outFormat = GL_RGB;
                outInternalFormat = GL_R11F_G11F_B10F;
                outDataType = GL_UNSIGNED_INT_10F_11F_11F_REV;
                return true;
            case QDemonRenderTextureFormats::RGB9E5:
                outFormat = GL_RGB;
                outInternalFormat = GL_RGB9_E5;
                outDataType = GL_UNSIGNED_INT_5_9_9_9_REV;
                return true;
            case QDemonRenderTextureFormats::SRGB8:
                outFormat = GL_RGB;
                outInternalFormat = GL_SRGB8;
                outDataType = GL_UNSIGNED_BYTE;
                return true;
            case QDemonRenderTextureFormats::SRGB8A8:
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

    static GLenum fromCompressedTextureFormatToGL(QDemonRenderTextureFormats::Enum value)
    {
        switch (value) {
        case QDemonRenderTextureFormats::RGBA_DXT1:
            return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        case QDemonRenderTextureFormats::RGB_DXT1:
            return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
        case QDemonRenderTextureFormats::RGBA_DXT3:
            return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        case QDemonRenderTextureFormats::RGBA_DXT5:
            return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        default:
            break;
        }

        Q_ASSERT(false);
        return 0;
    }

    static bool fromDepthTextureFormatToGL(QDemonRenderContextType type,
                                           QDemonRenderTextureFormats::Enum value,
                                           GLenum &outFormat, GLenum &outDataType,
                                           GLenum &outInternalFormat)
    {
        QDemonRenderContextType theContextFlags(QDemonRenderContextValues::GLES2
                                                | QDemonRenderContextValues::GL2);

        bool supportDepth24 = !(type & theContextFlags);
        bool supportDepth32f = !(type & theContextFlags);
        bool supportDepth24Stencil8 = !(type & theContextFlags);

        switch (value) {
        case QDemonRenderTextureFormats::Depth16:
            outFormat = GL_DEPTH_COMPONENT;
            outInternalFormat = GL_DEPTH_COMPONENT16;
            outDataType = GL_UNSIGNED_SHORT;
            return true;
        case QDemonRenderTextureFormats::Depth24:
            outFormat = GL_DEPTH_COMPONENT;
            outInternalFormat = (supportDepth24) ? GL_DEPTH_COMPONENT24 : GL_DEPTH_COMPONENT16;
            outDataType = (supportDepth24) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
            return true;
        case QDemonRenderTextureFormats::Depth32:
            outFormat = GL_DEPTH_COMPONENT;
            outInternalFormat =
                    (supportDepth32f) ? GL_DEPTH_COMPONENT32F : GL_DEPTH_COMPONENT16;
            outDataType = (supportDepth32f) ? GL_FLOAT : GL_UNSIGNED_SHORT;
            return true;
        case QDemonRenderTextureFormats::Depth24Stencil8:
            outFormat = (supportDepth24Stencil8) ? GL_DEPTH_STENCIL : GL_DEPTH_COMPONENT;
            outInternalFormat =
                    (supportDepth24Stencil8) ? GL_DEPTH24_STENCIL8 : GL_DEPTH_COMPONENT16;
            outDataType = (supportDepth24Stencil8) ? GL_UNSIGNED_INT_24_8 : GL_UNSIGNED_SHORT;
            return true;
        default:
            break;
        }

        Q_ASSERT(false);
        return false;
    }

    static GLenum fromTextureTargetToGL(QDemonRenderTextureTargetType::Enum value)
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

    static QDemonRenderTextureTargetType::Enum fromGLToTextureTarget(GLenum value)
    {
        QDemonRenderTextureTargetType::Enum retval = QDemonRenderTextureTargetType::Unknown;

        if (value == GL_TEXTURE_2D)
            retval = QDemonRenderTextureTargetType::Texture2D;
        else if (value == GL_TEXTURE_2D_MULTISAMPLE)
            retval = QDemonRenderTextureTargetType::Texture2D_MS;
        else
            Q_ASSERT(false);

        return retval;
    }

    static GLenum fromTextureUnitToGL(QDemonRenderTextureUnit::Enum value)
    {
        quint32 v = value;
        GLenum retval = GL_TEXTURE0;
        retval = GL_TEXTURE0 + v;

        return retval;
    }

    static GLenum fromGLToTextureUnit(GLenum value)
    {
        Q_ASSERT(value > GL_TEXTURE0);

        quint32 v = value - GL_TEXTURE0;
        QDemonRenderTextureUnit::Enum retval =
                QDemonRenderTextureUnit::Enum(QDemonRenderTextureUnit::TextureUnit_0 + v);

        return retval;
    }

    static GLenum fromTextureMinifyingOpToGL(QDemonRenderTextureMinifyingOp::Enum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_SCALE_OP(x, y)                                              \
        case QDemonRenderTextureMinifyingOp::y:                                                            \
    return x;
#define QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_MINIFYING_OP(x, y)                                          \
        case QDemonRenderTextureMinifyingOp::y:                                                            \
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

    static QDemonRenderTextureMinifyingOp::Enum fromGLToTextureMinifyingOp(GLenum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_SCALE_OP(x, y)                                              \
        case x:                                                                                        \
    return QDemonRenderTextureMinifyingOp::y;
#define QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_MINIFYING_OP(x, y)                                          \
        case x:                                                                                        \
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

    static GLenum fromTextureMagnifyingOpToGL(QDemonRenderTextureMagnifyingOp::Enum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_SCALE_OP(x, y)                                              \
        case QDemonRenderTextureMagnifyingOp::y:                                                           \
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

    static QDemonRenderTextureMagnifyingOp::Enum fromGLToTextureMagnifyingOp(GLenum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_SCALE_OP(x, y)                                              \
        case x:                                                                                        \
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

    static GLenum fromTextureCoordOpToGL(QDemonRenderTextureCoordOp::Enum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_WRAP_OP(x, y)                                               \
        case QDemonRenderTextureCoordOp::y:                                                                \
    return x;
        QDEMON_RENDER_ITERATE_GL_QDEMON_TEXTURE_WRAP_OP
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_WRAP_OP
                default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static QDemonRenderTextureCoordOp::Enum fromGLToTextureCoordOp(GLenum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_WRAP_OP(x, y)                                               \
        case x:                                                                                        \
    return QDemonRenderTextureCoordOp::y;
        QDEMON_RENDER_ITERATE_GL_QDEMON_TEXTURE_WRAP_OP
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_TEXTURE_WRAP_OP
                default:
            break;
        }
        Q_ASSERT(false);
        return QDemonRenderTextureCoordOp::Unknown;
    }

    static GLenum fromTextureCompareModeToGL(QDemonRenderTextureCompareMode::Enum value)
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
        return QDemonRenderTextureCompareMode::Unknown;
    }

    static GLenum fromGLToTextureCompareMode(GLenum value)
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
        return GL_INVALID_ENUM;
    }

    static GLenum fromTextureCompareFuncToGL(QDemonRenderTextureCompareOp::Enum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP(x, y)                                                       \
        case QDemonRenderTextureCompareOp::y:                                                              \
    return x;
        QDEMON_RENDER_ITERATE_GL_QDEMON_BOOL_OP
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_BOOL_OP
                default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static GLenum fromImageFormatToGL(QDemonRenderTextureFormats::Enum value)
    {
        switch (value) {
        case QDemonRenderTextureFormats::R8:
            return GL_R8;
        case QDemonRenderTextureFormats::R32I:
            return GL_R32I;
        case QDemonRenderTextureFormats::R32UI:
            return GL_R32UI;
        case QDemonRenderTextureFormats::R32F:
            return GL_R32F;
        case QDemonRenderTextureFormats::RGBA8:
            return GL_RGBA8;
        case QDemonRenderTextureFormats::SRGB8A8:
            return GL_RGBA8_SNORM;
        case QDemonRenderTextureFormats::RG16F:
            return GL_RG16F;
        case QDemonRenderTextureFormats::RGBA16F:
            return GL_RGBA16F;
        case QDemonRenderTextureFormats::RGBA32F:
            return GL_RGBA32F;
        default:
            break;
        }

        Q_ASSERT(false);
        return GL_INVALID_ENUM;
    }


    static GLenum fromImageAccessToGL(QDemonRenderImageAccessType::Enum value)
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
        quint32 value = flags;
        GLbitfield retval = 0;

        if (value & QDemonRenderBufferAccessTypeValues::Read)
            retval |= GL_MAP_READ_BIT;
        if (value & QDemonRenderBufferAccessTypeValues::Write)
            retval |= GL_MAP_WRITE_BIT;
        if (value & QDemonRenderBufferAccessTypeValues::Invalid)
            retval |= GL_MAP_INVALIDATE_BUFFER_BIT;
        if (value & QDemonRenderBufferAccessTypeValues::InvalidRange)
            retval |= GL_MAP_INVALIDATE_RANGE_BIT;

        Q_ASSERT(retval);
        return retval;
    }

    static GLbitfield fromMemoryBarrierFlagsToGL(QDemonRenderBufferBarrierFlags flags)
    {
        quint32 value = flags;
        GLbitfield retval = 0;
#if !defined(QT_OPENGL_ES)
        if (value & QDemonRenderBufferBarrierValues::AtomicCounter)
            retval |= GL_ATOMIC_COUNTER_BARRIER_BIT;
        if (value & QDemonRenderBufferBarrierValues::BufferUpdate)
            retval |= GL_BUFFER_UPDATE_BARRIER_BIT;
        if (value & QDemonRenderBufferBarrierValues::CommandBuffer)
            retval |= GL_COMMAND_BARRIER_BIT;
        if (value & QDemonRenderBufferBarrierValues::ElementArray)
            retval |= GL_ELEMENT_ARRAY_BARRIER_BIT;
        if (value & QDemonRenderBufferBarrierValues::Framebuffer)
            retval |= GL_FRAMEBUFFER_BARRIER_BIT;
        if (value & QDemonRenderBufferBarrierValues::PixelBuffer)
            retval |= GL_PIXEL_BUFFER_BARRIER_BIT;
        if (value & QDemonRenderBufferBarrierValues::ShaderImageAccess)
            retval |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
        if (value & QDemonRenderBufferBarrierValues::ShaderStorage)
            retval |= GL_SHADER_STORAGE_BARRIER_BIT;
        if (value & QDemonRenderBufferBarrierValues::TextureFetch)
            retval |= GL_TEXTURE_FETCH_BARRIER_BIT;
        if (value & QDemonRenderBufferBarrierValues::TextureUpdate)
            retval |= GL_TEXTURE_UPDATE_BARRIER_BIT;
        if (value & QDemonRenderBufferBarrierValues::TransformFeedback)
            retval |= GL_TRANSFORM_FEEDBACK_BARRIER_BIT;
        if (value & QDemonRenderBufferBarrierValues::UniformBuffer)
            retval |= GL_UNIFORM_BARRIER_BIT;
        if (value & QDemonRenderBufferBarrierValues::VertexAttribArray)
            retval |= GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT;
#endif
        Q_ASSERT(retval);
        return retval;
    }

    static GLbitfield fromShaderTypeFlagsToGL(QDemonRenderShaderTypeFlags flags)
    {
        quint32 value = flags;
        GLbitfield retval = 0;
        if (value & QDemonRenderShaderTypeValue::Vertex)
            retval |= GL_VERTEX_SHADER_BIT;
        if (value & QDemonRenderShaderTypeValue::Fragment)
            retval |= GL_FRAGMENT_SHADER_BIT;
        if (value & QDemonRenderShaderTypeValue::TessControl)
            retval |= GL_TESS_CONTROL_SHADER_BIT;
        if (value & QDemonRenderShaderTypeValue::TessEvaluation)
            retval |= GL_TESS_EVALUATION_SHADER_BIT;
        if (value & QDemonRenderShaderTypeValue::Geometry)
#if defined(QT_OPENGL_ES_3_1)
            retval |= GL_GEOMETRY_SHADER_BIT_EXT;
#else
            retval |= GL_GEOMETRY_SHADER_BIT;
#endif
        Q_ASSERT(retval || !value);
        return retval;
    }

    static GLenum fromPropertyDataTypesToShaderGL(QDemonRenderShaderDataTypes::Enum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(gl, nv)                                        \
        case QDemonRenderShaderDataTypes::nv:                                                              \
    return gl;
        QDEMON_RENDER_ITERATE_GL_QDEMON_SHADER_UNIFORM_TYPES
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES
                default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    static QDemonRenderShaderDataTypes::Enum fromShaderGLToPropertyDataTypes(GLenum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES(gl, nv)                                        \
        case gl:                                                                                       \
    return QDemonRenderShaderDataTypes::nv;
        QDEMON_RENDER_ITERATE_GL_QDEMON_SHADER_UNIFORM_TYPES
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_UNIFORM_TYPES
                case GL_SAMPLER_2D_SHADOW:
            return QDemonRenderShaderDataTypes::Texture2D;
#if !defined(QT_OPENGL_ES)
        case GL_UNSIGNED_INT_ATOMIC_COUNTER:
            return QDemonRenderShaderDataTypes::Integer;
        case GL_UNSIGNED_INT_IMAGE_2D:
            return QDemonRenderShaderDataTypes::Image2D;
#endif
        default:
            break;
        }
        Q_ASSERT(false);
        return QDemonRenderShaderDataTypes::Unknown;
    }

    static GLenum fromComponentTypeAndNumCompsToAttribGL(QDemonRenderComponentTypes::Enum compType,
                                                         quint32 numComps)
    {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_ATTRIB_TYPES(gl, ct, nc)                                     \
    if (compType == QDemonRenderComponentTypes::ct && numComps == nc)                                  \
    return gl;
        QDEMON_RENDER_ITERATE_GL_QDEMON_SHADER_ATTRIB_TYPES
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_ATTRIB_TYPES
                Q_ASSERT(false);
        return 0;
    }

    static void fromAttribGLToComponentTypeAndNumComps(
            GLenum enumVal, QDemonRenderComponentTypes::Enum &outCompType, quint32 &outNumComps)
    {
        switch (enumVal) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_ATTRIB_TYPES(gl, ct, nc)                                     \
        case gl:                                                                                       \
    outCompType = QDemonRenderComponentTypes::ct;                                                  \
    outNumComps = nc;                                                                          \
    return;
        QDEMON_RENDER_ITERATE_GL_QDEMON_SHADER_ATTRIB_TYPES
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_SHADER_ATTRIB_TYPES
                default:
            break;
        }
        Q_ASSERT(false);
        outCompType = QDemonRenderComponentTypes::Unknown;
        outNumComps = 0;
    }

    static GLenum
    fromRenderBufferFormatsToRenderBufferGL(QDemonRenderRenderBufferFormats::Enum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(gl, nv)                                         \
        case QDemonRenderRenderBufferFormats::nv:                                                          \
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

    static QDemonRenderRenderBufferFormats::Enum
    fromRenderBufferGLToRenderBufferFormats(GLenum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT(gl, nv)                                         \
        case gl:                                                                                       \
    return QDemonRenderRenderBufferFormats::nv;
        QDEMON_RENDER_ITERATE_GL_QDEMON_RENDERBUFFER_FORMATS
                QDEMON_RENDER_ITERATE_GL_QDEMON_RENDERBUFFER_COVERAGE_FORMATS
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_RENDERBUFFER_FORMAT
                default:
            break;
        }
        Q_ASSERT(false);
        return QDemonRenderRenderBufferFormats::Unknown;
    }

    static GLenum fromFramebufferAttachmentsToGL(QDemonRenderFrameBufferAttachments::Enum value)
    {
        switch (value) {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_COLOR_ATTACHMENT(x, idx)                                \
        case QDemonRenderFrameBufferAttachments::x:                                                        \
    return GL_COLOR_ATTACHMENT0 + idx;
#define QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_ATTACHMENT(x, y)                                        \
        case QDemonRenderFrameBufferAttachments::y:                                                        \
    return x;
        QDEMON_RENDER_ITERATE_GL_QDEMON_FRAMEBUFFER_ATTACHMENTS
                QDEMON_RENDER_ITERATE_GL_QDEMON_FRAMEBUFFER_COVERAGE_ATTACHMENTS
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_COLOR_ATTACHMENT
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_ATTACHMENT
                default:
            break;
        }
        Q_ASSERT(false);
        return QDemonRenderFrameBufferAttachments::Unknown;
    }

    static QDemonRenderFrameBufferAttachments::Enum fromGLToFramebufferAttachments(GLenum value)
    {
#define QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_COLOR_ATTACHMENT(x, idx)                                \
    if (value == GL_COLOR_ATTACHMENT0 + idx)                                                       \
    return QDemonRenderFrameBufferAttachments::x;
#define QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_ATTACHMENT(x, y)                                        \
    if (value == x)                                                                                \
    return QDemonRenderFrameBufferAttachments::y;
        QDEMON_RENDER_ITERATE_GL_QDEMON_FRAMEBUFFER_ATTACHMENTS
                QDEMON_RENDER_ITERATE_GL_QDEMON_FRAMEBUFFER_COVERAGE_ATTACHMENTS
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_COLOR_ATTACHMENT
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_FRAMEBUFFER_ATTACHMENT
                Q_ASSERT(false);
        return QDemonRenderFrameBufferAttachments::Unknown;
    }

    static GLbitfield fromClearFlagsToGL(QDemonRenderClearFlags flags)
    {
        quint32 value = flags;
        GLbitfield retval = 0;
#define QDEMON_RENDER_HANDLE_GL_QDEMON_CLEAR_FLAGS(gl, nv)                                                 \
    if ((value & QDemonRenderClearValues::nv))                                                         \
    retval |= gl;
        QDEMON_RENDER_ITERATE_GL_QDEMON_CLEAR_FLAGS
                QDEMON_RENDER_ITERATE_GL_QDEMON_CLEAR_COVERAGE_FLAGS
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_CLEAR_FLAGS
                return retval;
    }

    static QDemonRenderClearFlags fromGLToClearFlags(GLbitfield value)
    {
        quint32 retval = 0;
#define QDEMON_RENDER_HANDLE_GL_QDEMON_CLEAR_FLAGS(gl, nv)                                                 \
    if ((value & gl))                                                                              \
    retval |= QDemonRenderClearValues::nv;
        QDEMON_RENDER_ITERATE_GL_QDEMON_CLEAR_FLAGS
                QDEMON_RENDER_ITERATE_GL_QDEMON_CLEAR_COVERAGE_FLAGS
        #undef QDEMON_RENDER_HANDLE_GL_QDEMON_CLEAR_FLAGS
                return QDemonRenderClearFlags(retval);
    }

    static GLenum fromDrawModeToGL(QDemonRenderDrawMode::Enum value, bool inTesselationSupported)
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

    static QDemonRenderDrawMode::Enum fromGLToDrawMode(GLenum value)
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

    static GLenum fromRenderStateToGL(QDemonRenderState::Enum value)
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

    static QDemonRenderState::Enum fromGLToRenderState(GLenum value)
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

    static bool fromReadPixelsToGlFormatAndType(QDemonRenderReadPixelFormats::Enum inReadPixels,
                                                GLuint *outFormat, GLuint *outType)
    {
        switch (inReadPixels) {
        case QDemonRenderReadPixelFormats::Alpha8:
            *outFormat = GL_ALPHA;
            *outType = GL_UNSIGNED_BYTE;
            break;
        case QDemonRenderReadPixelFormats::RGB565:
            *outFormat = GL_RGB;
            *outType = GL_UNSIGNED_SHORT_5_6_5;
            break;
        case QDemonRenderReadPixelFormats::RGB8:
            *outFormat = GL_RGB;
            *outType = GL_UNSIGNED_BYTE;
            break;
        case QDemonRenderReadPixelFormats::RGBA4444:
            *outFormat = GL_RGBA;
            *outType = GL_UNSIGNED_SHORT_4_4_4_4;
            break;
        case QDemonRenderReadPixelFormats::RGBA5551:
            *outFormat = GL_RGBA;
            *outType = GL_UNSIGNED_SHORT_5_5_5_1;
            break;
        case QDemonRenderReadPixelFormats::RGBA8:
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

    static GLenum fromPathFillModeToGL(QDemonRenderPathFillMode::Enum inMode)
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

    static GLenum fromPathFontTargetToGL(QDemonRenderPathFontTarget::Enum inFontTarget)
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

    static QDemonRenderPathReturnValues::Enum fromGLToPathFontReturn(GLenum inReturnValue)
    {
        QDemonRenderPathReturnValues::Enum returnValue;

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

    static GLenum fromPathMissingGlyphsToGL(QDemonRenderPathMissingGlyphs::Enum inHandleGlyphs)
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

    static GLenum fromPathListModeToGL(QDemonRenderPathListMode::Enum inListMode)
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

    static GLenum fromPathCoverModeToGL(QDemonRenderPathCoverMode::Enum inMode)
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

    static GLenum fromPathTypeToGL(QDemonRenderPathFormatType::Enum value)
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
        quint32 value = flags;
        GLbitfield retval = 0;
#if !defined(QT_OPENGL_ES)
        if (value & QDemonRenderPathFontStyleValues::Bold)
            retval |= GL_BOLD_BIT_NV;
        if (value & QDemonRenderPathFontStyleValues::Italic)
            retval |= GL_ITALIC_BIT_NV;
#endif
        Q_ASSERT(retval || !value);
        return retval;
    }

    static GLenum fromPathTransformToGL(QDemonRenderPathTransformType::Enum value)
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
        quint32 value = flags;
        GLbitfield retval = 0;
#if !defined(QT_OPENGL_ES)
        if (value & QDemonRenderPathGlyphFontMetricValues::GlyphWidth)
            retval |= GL_GLYPH_WIDTH_BIT_NV;
        if (value & QDemonRenderPathGlyphFontMetricValues::GlyphHeight)
            retval |= GL_GLYPH_HEIGHT_BIT_NV;
        if (value & QDemonRenderPathGlyphFontMetricValues::GlyphHorizontalBearingX)
            retval |= GL_GLYPH_HORIZONTAL_BEARING_X_BIT_NV;
        if (value & QDemonRenderPathGlyphFontMetricValues::GlyphHorizontalBearingY)
            retval |= GL_GLYPH_HORIZONTAL_BEARING_Y_BIT_NV;
        if (value & QDemonRenderPathGlyphFontMetricValues::GlyphHorizontalBearingAdvance)
            retval |= GL_GLYPH_HORIZONTAL_BEARING_ADVANCE_BIT_NV;
        if (value & QDemonRenderPathGlyphFontMetricValues::GlyphVerticalBearingX)
            retval |= GL_GLYPH_VERTICAL_BEARING_X_BIT_NV;
        if (value & QDemonRenderPathGlyphFontMetricValues::GlyphVerticalBearingY)
            retval |= GL_GLYPH_VERTICAL_BEARING_Y_BIT_NV;
        if (value & QDemonRenderPathGlyphFontMetricValues::GlyphVerticalBearingAdvance)
            retval |= GL_GLYPH_VERTICAL_BEARING_ADVANCE_BIT_NV;
        if (value & QDemonRenderPathGlyphFontMetricValues::GlyphHasKerning)
            retval |= GL_GLYPH_HAS_KERNING_BIT_NV;

        if (value & QDemonRenderPathGlyphFontMetricValues::FontXMinBounds)
            retval |= GL_FONT_X_MIN_BOUNDS_BIT_NV;
        if (value & QDemonRenderPathGlyphFontMetricValues::FontYMinBounds)
            retval |= GL_FONT_Y_MIN_BOUNDS_BIT_NV;
        if (value & QDemonRenderPathGlyphFontMetricValues::FontXMaxBounds)
            retval |= GL_FONT_X_MAX_BOUNDS_BIT_NV;
        if (value & QDemonRenderPathGlyphFontMetricValues::FontYMaxBounds)
            retval |= GL_FONT_Y_MAX_BOUNDS_BIT_NV;
        if (value & QDemonRenderPathGlyphFontMetricValues::FontUnitsPerEm)
            retval |= GL_FONT_UNITS_PER_EM_BIT_NV;
        if (value & QDemonRenderPathGlyphFontMetricValues::FontAscender)
            retval |= GL_FONT_ASCENDER_BIT_NV;
        if (value & QDemonRenderPathGlyphFontMetricValues::FontDescender)
            retval |= GL_FONT_DESCENDER_BIT_NV;
        if (value & QDemonRenderPathGlyphFontMetricValues::FontHeight)
            retval |= GL_FONT_HEIGHT_BIT_NV;
        if (value & QDemonRenderPathGlyphFontMetricValues::FontMaxAdvanceWidth)
            retval |= GL_FONT_MAX_ADVANCE_WIDTH_BIT_NV;
        if (value & QDemonRenderPathGlyphFontMetricValues::FontMaxAdvanceHeight)
            retval |= GL_FONT_MAX_ADVANCE_HEIGHT_BIT_NV;
        if (value & QDemonRenderPathGlyphFontMetricValues::FontUnderlinePosition)
            retval |= GL_FONT_UNDERLINE_POSITION_BIT_NV;
        if (value & QDemonRenderPathGlyphFontMetricValues::FontMaxAdvanceWidth)
            retval |= GL_FONT_UNDERLINE_THICKNESS_BIT_NV;
        if (value & QDemonRenderPathGlyphFontMetricValues::FontHasKerning)
            retval |= GL_FONT_HAS_KERNING_BIT_NV;
        if (value & QDemonRenderPathGlyphFontMetricValues::FontNumGlyphIndices)
            retval |= GL_FONT_NUM_GLYPH_INDICES_BIT_NV;
#endif
        Q_ASSERT(retval || !value);
        return retval;
    }
};

QT_END_NAMESPACE

#endif // QDEMONOPENGLUTIL_H
