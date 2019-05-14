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
#ifndef QDEMONN_RENDERQDEMONDER_TYPES_H
#define QDEMONN_RENDERQDEMONDER_TYPES_H

#include <QtDemonRender/qtdemonrenderglobal.h>
#include <QtDemon/QDemonDataRef>
#include <QtDemonRender/qdemonrenderlogging.h>

#include <QtGui/QVector2D>
#include <QtGui/QVector3D>
#include <QtGui/QVector4D>
#include <QtGui/QMatrix4x4>
#include <QtGui/QMatrix3x3>
#include <QFloat16>

#include <cmath>

QT_BEGIN_NAMESPACE

enum class QDemonRenderComponentType
{
    Unknown = 0,
    UnsignedInteger8,
    Integer8,
    UnsignedInteger16,
    Integer16,
    UnsignedInteger32,
    Integer32,
    UnsignedInteger64,
    Integer64,
    Float16,
    Float32,
    Float64
};

inline const char *toString(QDemonRenderComponentType value)
{
    switch (value) {
    case QDemonRenderComponentType::UnsignedInteger8:
        return "UnsignedInteger8";
    case QDemonRenderComponentType::Integer8:
        return "Integer8";
    case QDemonRenderComponentType::UnsignedInteger16:
        return "UnsignedInteger16";
    case QDemonRenderComponentType::Integer16:
        return "Integer16";
    case QDemonRenderComponentType::UnsignedInteger32:
        return "UnsignedInteger32";
    case QDemonRenderComponentType::Integer32:
        return "Integer32";
    case QDemonRenderComponentType::UnsignedInteger64:
        return "UnsignedInteger64";
    case QDemonRenderComponentType::Integer64:
        return "Integer64";
    case QDemonRenderComponentType::Float16:
        return "Float16";
    case QDemonRenderComponentType::Float32:
        return "Float32";
    case QDemonRenderComponentType::Float64:
        return "Float64";
    default:
        break;
    }
    return "Unknown";
}

inline quint32 getSizeOfType(QDemonRenderComponentType value)
{
    switch (value) {
    case QDemonRenderComponentType::UnsignedInteger8:
        return sizeof(quint8);
    case QDemonRenderComponentType::Integer8:
        return sizeof(qint8);
    case QDemonRenderComponentType::UnsignedInteger16:
        return sizeof(quint16);
    case QDemonRenderComponentType::Integer16:
        return sizeof(qint16);
    case QDemonRenderComponentType::UnsignedInteger32:
        return sizeof(quint32);
    case QDemonRenderComponentType::Integer32:
        return sizeof(qint32);
    case QDemonRenderComponentType::UnsignedInteger64:
        return sizeof(quint64);
    case QDemonRenderComponentType::Integer64:
        return sizeof(qint64);
    case QDemonRenderComponentType::Float16:
        return sizeof(qfloat16);
    case QDemonRenderComponentType::Float32:
        return sizeof(float);
    case QDemonRenderComponentType::Float64:
        return sizeof(double);
    default:
        break;
    }
    Q_ASSERT(false);
    return 0;
}

enum class QDemonRenderContextType : quint32
{
    GLES2 = 1 << 0,
    GL2 = 1 << 1,
    GLES3 = 1 << 2,
    GL3 = 1 << 3,
    GLES3PLUS = 1 << 4,
    GL4 = 1 << 5,
    NullContext = 1 << 6,
};

Q_DECLARE_FLAGS(QDemonRenderContextTypes, QDemonRenderContextType)
Q_DECLARE_OPERATORS_FOR_FLAGS(QDemonRenderContextTypes)

enum class QDemonRenderClearValues : quint32
{
    Color = 1 << 0,
    Depth = 1 << 1,
    Stencil = 1 << 3,
    Coverage = 1 << 4,
};

Q_DECLARE_FLAGS(QDemonRenderClearFlags, QDemonRenderClearValues)
Q_DECLARE_OPERATORS_FOR_FLAGS(QDemonRenderClearFlags)

enum class QDemonRenderQueryType
{
    Unknown = 0,
    Samples, ///< samples query object
    Timer, ///< timer query object
};

enum class QDemonRenderQueryResultType
{
    Unknown = 0,
    ResultAvailable, ///< Check if query result is available
    Result, ///< Get actual result
};

enum class QDemonRenderSyncType
{
    Unknown = 0,
    GpuCommandsComplete, ///< sync to Gpu commands finished
};

enum class QDemonRenderSyncValues
{
    Unknown = 0, ///< for future usage
};

Q_DECLARE_FLAGS(QDemonRenderSyncFlags, QDemonRenderSyncValues)
Q_DECLARE_OPERATORS_FOR_FLAGS(QDemonRenderSyncFlags)

enum class QDemonRenderCommandFlushValues
{
    SyncFlushCommands = 0, ///< sync for flushing command
};

Q_DECLARE_FLAGS(QDemonRenderCommandFlushFlags, QDemonRenderCommandFlushValues)
Q_DECLARE_OPERATORS_FOR_FLAGS(QDemonRenderCommandFlushFlags)

enum class QDemonRenderBufferType : quint32
{
    Vertex, ///< Bind as vertex buffer
    Index, ///< Bind as index buffer
    Constant, ///< Bind as constant buffer
    Storage, ///< Bind as shader storage buffer
    AtomicCounter, ///< Bind as atomic counter buffer
    DrawIndirect, ///< Bind as draw indirect buffer
};

enum class QDemonRenderBufferUsageType
{
    Unknown = 0,
    Static, ///< Rarely updated
    Dynamic, ///< Most likely updated every frame
};

enum class QDemonRenderImageAccessType
{
    Unknown = 0,
    Read, ///< Read only access
    Write, ///< Write only access
    ReadWrite, ///< Read and write access
};

enum class QDemonRenderBufferAccessTypeValues
{
    Unknown = 0,
    Read = 1 << 0, ///< Read access
    Write = 1 << 1, ///< Write access
    Invalid = 1 << 2, ///< No sync
    InvalidRange = 1 << 3, ///< No sync
};

Q_DECLARE_FLAGS(QDemonRenderBufferAccessFlags, QDemonRenderBufferAccessTypeValues)
Q_DECLARE_OPERATORS_FOR_FLAGS(QDemonRenderBufferAccessFlags)

///< defines a barrier of ordering the memory transactions to a command relative to those issued
/// before the barrier
enum class QDemonRenderBufferBarrierValues
{
    Unknown = 0,
    VertexAttribArray = 1 << 0, ///< Barrier for vertex attributes sourced from a buffer
    ElementArray = 1 << 1, ///< Barrier for indices sourced from a buffer
    UniformBuffer = 1 << 2, ///< Barrier for shader uniforms sourced from a buffer
    TextureFetch = 1 << 3, ///< Barrier for texture fetches within shaders
    ShaderImageAccess = 1 << 4, ///< Barrier for image access using load / store
    CommandBuffer = 1 << 5, ///< Barrier for indirect drawing
    PixelBuffer = 1 << 6, ///< Barrier for pixel buffer access
    TextureUpdate = 1 << 7, ///< Barrier for texture writes
    BufferUpdate = 1 << 8, ///< Barrier for buffer writes
    Framebuffer = 1 << 9, ///< Barrier for framebuffer writes
    TransformFeedback = 1 << 10, ///< Barrier for transform feedback writes
    AtomicCounter = 1 << 11, ///< Barrier for atomic counter writes
    ShaderStorage = 1 << 12, ///< Barrier for shader storage blocks writes
    All = 0xFFFF, ///< Barrier for all of the above
};

Q_DECLARE_FLAGS(QDemonRenderBufferBarrierFlags, QDemonRenderBufferBarrierValues)
Q_DECLARE_OPERATORS_FOR_FLAGS(QDemonRenderBufferBarrierFlags)

enum class QDemonRenderRenderBufferFormat
{
    Unknown = 0,
    RGBA4,
    RGB565,
    RGBA5551,
    Depth16,
    Depth24,
    Depth32,
    StencilIndex8,
    CoverageNV
};

inline const char *toString(QDemonRenderRenderBufferFormat value)
{
    switch (value) {
    case QDemonRenderRenderBufferFormat::RGBA4:
        return "RGBA4";
    case QDemonRenderRenderBufferFormat::RGB565:
        return "RGB565";
    case QDemonRenderRenderBufferFormat::RGBA5551:
        return "RGBA5551";
    case QDemonRenderRenderBufferFormat::Depth16:
        return "Depth16";
    case QDemonRenderRenderBufferFormat::Depth24:
        return "Depth24";
    case QDemonRenderRenderBufferFormat::Depth32:
        return "Depth32";
    case QDemonRenderRenderBufferFormat::StencilIndex8:
        return "StencilIndex8";
    case QDemonRenderRenderBufferFormat::CoverageNV:
        return "CoverageNV";
    default:
        break;
    }
    return "Unknown";
}

struct QDemonRenderTextureFormat
{
    enum Format {
        Unknown = 0,
        R8,
        R16,
        R16F,
        R32I,
        R32UI,
        R32F,
        RG8,
        RGBA8,
        RGB8,
        SRGB8,
        SRGB8A8,
        RGB565,
        RGBA5551,
        Alpha8,
        Luminance8,
        Luminance16,
        LuminanceAlpha8,
        RGBA16F,
        RG16F,
        RG32F,
        RGB32F,
        RGBA32F,
        R11G11B10,
        RGB9E5,
        RGBA_DXT1,
        RGB_DXT1,
        RGBA_DXT3,
        RGBA_DXT5,
        Depth16,
        Depth24,
        Depth32,
        Depth24Stencil8
    };
    Format format;

    constexpr QDemonRenderTextureFormat(Format f) : format(f) {}

    bool isUncompressedTextureFormat() const
    {
        switch (format) {
        case QDemonRenderTextureFormat::R8:
            return true;
        case QDemonRenderTextureFormat::R16:
            return true;
        case QDemonRenderTextureFormat::R16F:
            return true;
        case QDemonRenderTextureFormat::R32I:
            return true;
        case QDemonRenderTextureFormat::R32UI:
            return true;
        case QDemonRenderTextureFormat::R32F:
            return true;
        case QDemonRenderTextureFormat::RG8:
            return true;
        case QDemonRenderTextureFormat::RGBA8:
            return true;
        case QDemonRenderTextureFormat::RGB8:
            return true;
        case QDemonRenderTextureFormat::SRGB8:
            return true;
        case QDemonRenderTextureFormat::SRGB8A8:
            return true;
        case QDemonRenderTextureFormat::RGB565:
            return true;
        case QDemonRenderTextureFormat::RGBA5551:
            return true;
        case QDemonRenderTextureFormat::Alpha8:
            return true;
        case QDemonRenderTextureFormat::Luminance8:
            return true;
        case QDemonRenderTextureFormat::Luminance16:
            return true;
        case QDemonRenderTextureFormat::LuminanceAlpha8:
            return true;
        case QDemonRenderTextureFormat::RGBA16F:
            return true;
        case QDemonRenderTextureFormat::RG16F:
            return true;
        case QDemonRenderTextureFormat::RG32F:
            return true;
        case QDemonRenderTextureFormat::RGB32F:
            return true;
        case QDemonRenderTextureFormat::RGBA32F:
            return true;
        case QDemonRenderTextureFormat::R11G11B10:
            return true;
        case QDemonRenderTextureFormat::RGB9E5:
            return true;
        default:
            break;
        }
        return false;
    }

    bool isCompressedTextureFormat() const
    {
        switch (format) {
        case QDemonRenderTextureFormat::RGBA_DXT1:
            return true;
        case QDemonRenderTextureFormat::RGB_DXT1:
            return true;
        case QDemonRenderTextureFormat::RGBA_DXT3:
            return true;
        case QDemonRenderTextureFormat::RGBA_DXT5:
            return true;
        default:
            break;
        }
        return false;
    }

    bool isDepthTextureFormat() const
    {
        switch (format) {
        case QDemonRenderTextureFormat::Depth16:
            return true;
        case QDemonRenderTextureFormat::Depth24:
            return true;
        case QDemonRenderTextureFormat::Depth32:
            return true;
        case QDemonRenderTextureFormat::Depth24Stencil8:
            return true;
        default:
            break;
        }
        return false;
    }

    const char *toString() const
    {
        switch (format) {
        case R8:
            return "R8";
        case R16:
            return "R16";
        case R16F:
            return "R16F";
        case R32I:
            return "R32I";
        case R32UI:
            return "R32UI";
        case R32F:
            return "R32F";
        case RG8:
            return "RG8";
        case RGBA8:
            return "RGBA8";
        case RGB8:
            return "RGB8";
        case SRGB8:
            return "SRGB8";
        case SRGB8A8:
            return "SRGB8A8";
        case RGB565:
            return "RGB565";
        case RGBA5551:
            return "RGBA5551";
        case Alpha8:
            return "Alpha8";
        case Luminance8:
            return "Luminance8";
        case Luminance16:
            return "Luminance16";
        case LuminanceAlpha8:
            return "LuminanceAlpha8";
        case RGBA16F:
            return "RGBA16F";
        case RG16F:
            return "RG16F";
        case RG32F:
            return "RG32F";
        case RGB32F:
            return "RGB32F";
        case RGBA32F:
            return "RGBA32F";
        case R11G11B10:
            return "R11G11B10";
        case RGB9E5:
            return "RGB9E5";
        case RGBA_DXT1:
            return "RGBA_DXT1";
        case RGB_DXT1:
            return "RGB_DXT1";
        case RGBA_DXT3:
            return "RGBA_DXT3";
        case RGBA_DXT5:
            return "RGBA_DXT5";
        case Depth16:
            return "Depth16";
        case Depth24:
            return "Depth24";
        case Depth32:
            return "Depth32";
        case Depth24Stencil8:
            return "Depth24Stencil8";
        default:
            break;
        }
        return "Unknown";
    }

    qint32 getSizeofFormat() const
    {
        switch (format) {
        case R8:
            return 1;
        case R16F:
            return 2;
        case R16:
            return 2;
        case R32I:
            return 4;
        case R32F:
            return 4;
        case RGBA8:
            return 4;
        case RGB8:
            return 3;
        case RGB565:
            return 2;
        case RGBA5551:
            return 2;
        case Alpha8:
            return 1;
        case Luminance8:
            return 1;
        case LuminanceAlpha8:
            return 1;
        case Depth16:
            return 2;
        case Depth24:
            return 3;
        case Depth32:
            return 4;
        case Depth24Stencil8:
            return 4;
        case RGB9E5:
            return 4;
        case SRGB8:
            return 3;
        case SRGB8A8:
            return 4;
        case RGBA16F:
            return 8;
        case RG16F:
            return 4;
        case RG32F:
            return 8;
        case RGBA32F:
            return 16;
        case RGB32F:
            return 12;
        case R11G11B10:
            return 4;
        default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    quint32 getNumberOfComponent() const
    {
        switch (format) {
        case R8:
            return 1;
        case R16F:
            return 1;
        case R16:
            return 1;
        case R32I:
            return 1;
        case R32F:
            return 1;
        case RGBA8:
            return 4;
        case RGB8:
            return 3;
        case RGB565:
            return 3;
        case RGBA5551:
            return 4;
        case Alpha8:
            return 1;
        case Luminance8:
            return 1;
        case LuminanceAlpha8:
            return 2;
        case Depth16:
            return 1;
        case Depth24:
            return 1;
        case Depth32:
            return 1;
        case Depth24Stencil8:
            return 2;
        case RGB9E5:
            return 3;
        case SRGB8:
            return 3;
        case SRGB8A8:
            return 4;
        case RGBA16F:
            return 4;
        case RG16F:
            return 2;
        case RG32F:
            return 2;
        case RGBA32F:
            return 4;
        case RGB32F:
            return 3;
        case R11G11B10:
            return 3;
        default:
            break;
        }
        Q_ASSERT(false);
        return 0;
    }

    void decodeToFloat(void *inPtr, qint32 byteOfs, float *outPtr) const
    {
        Q_ASSERT(byteOfs >= 0);
        outPtr[0] = 0.0f;
        outPtr[1] = 0.0f;
        outPtr[2] = 0.0f;
        outPtr[3] = 0.0f;
        quint8 *src = reinterpret_cast<quint8 *>(inPtr);
        // float divisor;		// If we want to support RGBD?
        switch (format) {
        case Alpha8:
            outPtr[0] = (float(src[byteOfs])) / 255.0f;
            break;

        case Luminance8:
        case LuminanceAlpha8:
        case R8:
        case RG8:
        case RGB8:
        case RGBA8:
        case SRGB8:
        case SRGB8A8:
            // NOTE : RGBD Hack here for reference.  Not meant for installation.
            // divisor = (QDemonRenderTextureFormat::getSizeofFormat(inFmt) == 4) ?
            // ((float)src[byteOfs+3]) / 255.0f : 1.0f;
            for (qint32 i = 0; i < getSizeofFormat(); ++i) {
                float val = (float(src[byteOfs + i])) / 255.0f;
                outPtr[i] = (i < 3) ? std::pow(val, 0.4545454545f) : val;
                // Assuming RGBA8 actually means RGBD (which is stupid, I know)
                // if ( QDemonRenderTextureFormat::getSizeofFormat(inFmt) == 4 ) { outPtr[i] /=
                // divisor; }
            }
            // outPtr[3] = divisor;
            break;

        case R32F:
            outPtr[0] = reinterpret_cast<float *>(src + byteOfs)[0];
            break;
        case RG32F:
            outPtr[0] = reinterpret_cast<float *>(src + byteOfs)[0];
            outPtr[1] = reinterpret_cast<float *>(src + byteOfs)[1];
            break;
        case RGBA32F:
            outPtr[0] = reinterpret_cast<float *>(src + byteOfs)[0];
            outPtr[1] = reinterpret_cast<float *>(src + byteOfs)[1];
            outPtr[2] = reinterpret_cast<float *>(src + byteOfs)[2];
            outPtr[3] = reinterpret_cast<float *>(src + byteOfs)[3];
            break;
        case RGB32F:
            outPtr[0] = reinterpret_cast<float *>(src + byteOfs)[0];
            outPtr[1] = reinterpret_cast<float *>(src + byteOfs)[1];
            outPtr[2] = reinterpret_cast<float *>(src + byteOfs)[2];
            break;

        case R16F:
        case RG16F:
        case RGBA16F:
            for (qint32 i = 0; i < (getSizeofFormat() >> 1); ++i) {
                // NOTE : This only works on the assumption that we don't have any denormals,
                // Infs or NaNs.
                // Every pixel in our source image should be "regular"
                quint16 h = reinterpret_cast<quint16 *>(src + byteOfs)[i];
                quint32 sign = (h & 0x8000u) << 16u;
                quint32 exponent = (((((h & 0x7c00u) >> 10) - 15) + 127) << 23);
                quint32 mantissa = ((h & 0x3ffu) << 13);
                quint32 result = sign | exponent | mantissa;

                if (h == 0 || h == 0x8000) {
                    result = 0;
                } // Special case for zero and negative zero
                memcpy(reinterpret_cast<quint32 *>(outPtr) + i, &result, 4);
            }
            break;

        case R11G11B10:
            // place holder
            Q_ASSERT(false);
            break;

        default:
            outPtr[0] = 0.0f;
            outPtr[1] = 0.0f;
            outPtr[2] = 0.0f;
            outPtr[3] = 0.0f;
            break;
        }
    }

    void encodeToPixel(float *inPtr, void *outPtr, qint32 byteOfs) const
    {
        Q_ASSERT(byteOfs >= 0);
        quint8 *dest = reinterpret_cast<quint8 *>(outPtr);
        switch (format) {
        case QDemonRenderTextureFormat::Alpha8:
            dest[byteOfs] = quint8(inPtr[0] * 255.0f);
            break;

        case Luminance8:
        case LuminanceAlpha8:
        case R8:
        case RG8:
        case RGB8:
        case RGBA8:
        case SRGB8:
        case SRGB8A8:
            for (qint32 i = 0; i < getSizeofFormat(); ++i) {
                inPtr[i] = (inPtr[i] > 1.0f) ? 1.0f : inPtr[i];
                if (i < 3)
                    dest[byteOfs + i] = quint8(powf(inPtr[i], 2.2f) * 255.0f);
                else
                    dest[byteOfs + i] = quint8(inPtr[i] * 255.0f);
            }
            break;

        case R32F:
            reinterpret_cast<float *>(dest + byteOfs)[0] = inPtr[0];
            break;
        case RG32F:
            reinterpret_cast<float *>(dest + byteOfs)[0] = inPtr[0];
            reinterpret_cast<float *>(dest + byteOfs)[1] = inPtr[1];
            break;
        case RGBA32F:
            reinterpret_cast<float *>(dest + byteOfs)[0] = inPtr[0];
            reinterpret_cast<float *>(dest + byteOfs)[1] = inPtr[1];
            reinterpret_cast<float *>(dest + byteOfs)[2] = inPtr[2];
            reinterpret_cast<float *>(dest + byteOfs)[3] = inPtr[3];
            break;
        case RGB32F:
            reinterpret_cast<float *>(dest + byteOfs)[0] = inPtr[0];
            reinterpret_cast<float *>(dest + byteOfs)[1] = inPtr[1];
            reinterpret_cast<float *>(dest + byteOfs)[2] = inPtr[2];
            break;

        case R16F:
        case RG16F:
        case RGBA16F:
            for (qint32 i = 0; i < (getSizeofFormat() >> 1); ++i) {
                // NOTE : This also has the limitation of not handling  infs, NaNs and
                // denormals, but it should be
                // sufficient for our purposes.
                if (inPtr[i] > 65519.0f) {
                    inPtr[i] = 65519.0f;
                }
                if (std::fabs(inPtr[i]) < 6.10352E-5f) {
                    inPtr[i] = 0.0f;
                }
                quint32 f = reinterpret_cast<quint32 *>(inPtr)[i];
                quint32 sign = (f & 0x80000000) >> 16;
                qint32 exponent = (f & 0x7f800000) >> 23;
                quint32 mantissa = (f >> 13) & 0x3ff;
                exponent = exponent - 112;
                if (exponent > 31) {
                    exponent = 31;
                }
                if (exponent < 0) {
                    exponent = 0;
                }
                exponent = exponent << 10;
                reinterpret_cast<quint16 *>(dest + byteOfs)[i] = quint16(sign | quint32(exponent) | mantissa);
            }
            break;

        case R11G11B10:
            // place holder
            Q_ASSERT(false);
            break;

        default:
            dest[byteOfs] = 0;
            dest[byteOfs + 1] = 0;
            dest[byteOfs + 2] = 0;
            dest[byteOfs + 3] = 0;
            break;
        }
    }

    bool operator==(const QDemonRenderTextureFormat &other) const { return format == other.format; }
    bool operator!=(const QDemonRenderTextureFormat &other) const { return format != other.format; }
};

enum class QDemonRenderTextureTargetType
{
    Unknown = 0,
    Texture2D,
    Texture2D_MS,
    Texture2D_Array,
    TextureCube,
    TextureCubePosX,
    TextureCubeNegX,
    TextureCubePosY,
    TextureCubeNegY,
    TextureCubePosZ,
    TextureCubeNegZ,
};

enum class QDemonRenderTextureUnit
{
    TextureUnit_0 = 0,
    TextureUnit_1,
    TextureUnit_2,
    TextureUnit_3,
    TextureUnit_4,
    TextureUnit_5,
    TextureUnit_6,
    TextureUnit_7,
    TextureUnit_8,
    TextureUnit_9,
    TextureUnit_10,
    TextureUnit_11,
    TextureUnit_12,
    TextureUnit_13,
    TextureUnit_14,
    TextureUnit_15,
    TextureUnit_16,
    TextureUnit_17,
    TextureUnit_18,
    TextureUnit_19,
    TextureUnit_20,
    TextureUnit_21,
    TextureUnit_22,
    TextureUnit_23,
    TextureUnit_24,
    TextureUnit_25,
    TextureUnit_26,
    TextureUnit_27,
    TextureUnit_28,
    TextureUnit_29,
    TextureUnit_30,
    TextureUnit_31
};

enum class QDemonRenderTextureCompareMode
{
    Unknown = 0,
    NoCompare,
    CompareToRef
};

enum class QDemonRenderTextureSwizzleMode
{
    NoSwizzle = 0,
    L8toR8,
    A8toR8,
    L8A8toRG8,
    L16toR16
};

enum class QDemonRenderTextureCompareOp
{
    Never,
    Less,
    LessThanOrEqual,
    Equal,
    NotEqual,
    Greater,
    GreaterThanOrEqual,
    AlwaysTrue,
};

enum class QDemonRenderTextureMinifyingOp
{
    Unknown = 0,
    Nearest,
    Linear,
    NearestMipmapNearest,
    LinearMipmapNearest,
    NearestMipmapLinear,
    LinearMipmapLinear
};
inline const char *toString(QDemonRenderTextureMinifyingOp value)
{
    switch (value) {
    case QDemonRenderTextureMinifyingOp::Nearest:
        return "Nearest";
    case QDemonRenderTextureMinifyingOp::Linear:
        return "Linear";
    case QDemonRenderTextureMinifyingOp::NearestMipmapNearest:
        return "NearestMipmapNearest";
    case QDemonRenderTextureMinifyingOp::LinearMipmapNearest:
        return "LinearMipmapNearest";
    case QDemonRenderTextureMinifyingOp::NearestMipmapLinear:
        return "NearestMipmapLinear";
    case QDemonRenderTextureMinifyingOp::LinearMipmapLinear:
        return "LinearMipmapLinear";
    default:
        break;
    }
    return "Unknown";
}

enum class QDemonRenderTextureMagnifyingOp
{
    Unknown = 0,
    Nearest,
    Linear
};
inline const char *toString(QDemonRenderTextureMagnifyingOp value)
{
    switch (value) {
    case QDemonRenderTextureMagnifyingOp::Nearest:
        return "Nearest";
    case QDemonRenderTextureMagnifyingOp::Linear:
        return "Linear";
    default:
        break;
    }
    return "Unknown";
}

enum class QDemonRenderTextureCoordOp
{

    Unknown = 0,
    ClampToEdge,
    MirroredRepeat,
    Repeat
};
inline const char *toString(QDemonRenderTextureCoordOp value)
{
    switch (value) {
    case QDemonRenderTextureCoordOp::ClampToEdge:
        return "ClampToEdge";
    case QDemonRenderTextureCoordOp::MirroredRepeat:
        return "MirroredRepeat";
    case QDemonRenderTextureCoordOp::Repeat:
        return "Repeat";
    default:
        break;
    }
    return "Unknown";
}

enum class QDemonRenderHint
{
    Unknown = 0,
    Fastest,
    Nicest,
    Unspecified
};
inline const char *toString(QDemonRenderHint value)
{
    switch (value) {
    case QDemonRenderHint::Fastest:
        return "Fastest";
    case QDemonRenderHint::Nicest:
        return "Nicest";
    case QDemonRenderHint::Unspecified:
        return "Unspecified";
    default:
        break;
    }
    return "Unknown";
}

struct QDemonRenderVertexBufferEntry
{
    const char *m_name;
    /** Datatype of the this entry points to in the buffer */
    QDemonRenderComponentType m_componentType;
    /** Number of components of each data member. 1,2,3, or 4.  Don't be stupid.*/
    quint32 m_numComponents;
    /** Offset from the beginning of the buffer of the first item */
    quint32 m_firstItemOffset;
    /** Attribute input slot used for this entry*/
    quint32 m_inputSlot;

    QDemonRenderVertexBufferEntry(const char *nm,
                                  QDemonRenderComponentType type,
                                  quint32 numComponents,
                                  quint32 firstItemOffset = 0,
                                  quint32 inputSlot = 0)
        : m_name(nm), m_componentType(type), m_numComponents(numComponents), m_firstItemOffset(firstItemOffset), m_inputSlot(inputSlot)
    {
    }

    QDemonRenderVertexBufferEntry()
        : m_name(nullptr), m_componentType(QDemonRenderComponentType::Unknown), m_numComponents(0), m_firstItemOffset(0), m_inputSlot(0)
    {
    }

    QDemonRenderVertexBufferEntry(const QDemonRenderVertexBufferEntry &inOther)
        : m_name(inOther.m_name)
        , m_componentType(inOther.m_componentType)
        , m_numComponents(inOther.m_numComponents)
        , m_firstItemOffset(inOther.m_firstItemOffset)
        , m_inputSlot(inOther.m_inputSlot)
    {
    }

    QDemonRenderVertexBufferEntry &operator=(const QDemonRenderVertexBufferEntry &inOther)
    {
        if (this != &inOther) {
            m_name = inOther.m_name;
            m_componentType = inOther.m_componentType;
            m_numComponents = inOther.m_numComponents;
            m_firstItemOffset = inOther.m_firstItemOffset;
            m_inputSlot = inOther.m_inputSlot;
        }
        return *this;
    }
};

class QDemonRenderShaderProgram;

// typedef QDemonConstDataRef<qint8> TConstI8Ref;

struct Q_DEMONRENDER_EXPORT QDemonRenderVertFragCompilationResult
{
    const char *m_shaderName = nullptr;
    QDemonRef<QDemonRenderShaderProgram> m_shader; ///< contains the program

    QDemonRenderVertFragCompilationResult();
    ~QDemonRenderVertFragCompilationResult();
    QDemonRenderVertFragCompilationResult(const QDemonRenderVertFragCompilationResult &);
    QDemonRenderVertFragCompilationResult & operator=(const QDemonRenderVertFragCompilationResult &other);
};

enum class QDemonRenderFrameBufferAttachment
{
    Unknown = 0,
    Color0,
    Color1,
    Color2,
    Color3,
    Color4,
    Color5,
    Color6,
    Color7,
    Depth,
    Stencil,
    DepthStencil,
    CoverageNV,
    LastAttachment,
};

enum class QDemonRenderDrawMode
{
    Unknown = 0,
    Points,
    LineStrip,
    LineLoop,
    Lines,
    TriangleStrip,
    TriangleFan,
    Triangles,
    Patches,
};

enum class QDemonRenderTextureCubeFace
{
    InvalidFace = 0,
    CubePosX = 1,
    CubeNegX,
    CubePosY,
    CubeNegY,
    CubePosZ,
    CubeNegZ
};

// enums match the NV path extensions
enum class QDemonRenderPathCommands : quint8
{
    Close = 0,
    MoveTo = 2,
    CubicCurveTo = 12,
};

enum class QDemonRenderPathFontTarget
{
    StandardFont = 0,
    SystemFont = 1,
    FileFont = 2,
};

enum class QDemonRenderPathMissingGlyphs
{
    SkipMissing = 0,
    UseMissing = 1,
};

enum class QDemonRenderPathFontStyleValue
{
    Bold = 1 << 0,
    Italic = 1 << 1,
};

Q_DECLARE_FLAGS(QDemonRenderPathFontStyleFlags, QDemonRenderPathFontStyleValue)
Q_DECLARE_OPERATORS_FOR_FLAGS(QDemonRenderPathFontStyleFlags)

enum class QDemonRenderPathReturnValues
{
    FontGlypsAvailable = 0,
    FontTargetUnavailable = 1,
    FontUnavailable = 2,
    FontUnintelligible = 3,
    InvalidEnum = 4,
    OutOfMemory = 5,
};

enum class QDemonRenderPathFormatType
{
    Byte = 1,
    UByte,
    Short,
    UShort,
    Int,
    Uint,
    Float,
    Utf8,
    Utf16,
    Bytes2,
    Bytes3,
    Bytes4,
};

enum class QDemonRenderPathGlyphFontMetricValues
{
    GlyphWidth = 1 << 0,
    GlyphHeight = 1 << 1,
    GlyphHorizontalBearingX = 1 << 2,
    GlyphHorizontalBearingY = 1 << 3,
    GlyphHorizontalBearingAdvance = 1 << 4,
    GlyphVerticalBearingX = 1 << 5,
    GlyphVerticalBearingY = 1 << 6,
    GlyphVerticalBearingAdvance = 1 << 7,
    GlyphHasKerning = 1 << 8,

    FontXMinBounds = 1 << 9,
    FontYMinBounds = 1 << 10,
    FontXMaxBounds = 1 << 11,
    FontYMaxBounds = 1 << 12,
    FontUnitsPerEm = 1 << 13,
    FontAscender = 1 << 14,
    FontDescender = 1 << 15,
    FontHeight = 1 << 16,
    FontMaxAdvanceWidth = 1 << 17,
    FontMaxAdvanceHeight = 1 << 18,
    FontUnderlinePosition = 1 << 19,
    FontUnderlineThickness = 1 << 20,
    FontHasKerning = 1 << 21,
    FontNumGlyphIndices = 1 << 22,
};

Q_DECLARE_FLAGS(QDemonRenderPathGlyphFontMetricFlags, QDemonRenderPathGlyphFontMetricValues)
Q_DECLARE_OPERATORS_FOR_FLAGS(QDemonRenderPathGlyphFontMetricFlags)

enum class QDemonRenderPathListMode
{
    AccumAdjacentPairs = 1,
    AdjacentPairs,
    FirstToRest,
};

enum class QDemonRenderPathFillMode
{
    Fill = 1,
    CountUp,
    CountDown,
    Invert,
};

enum class QDemonRenderPathCoverMode
{
    ConvexHull = 1,
    BoundingBox,
    BoundingBoxOfBoundingBox,
    PathFillCover,
    PathStrokeCover,
};

enum class QDemonRenderPathTransformType
{
    NoTransform = 0,
    TranslateX,
    TranslateY,
    Translate2D,
    Translate3D,
    Affine2D,
    Affine3D,
    TransposeAffine2D,
    TransposeAffine3D,
};

enum class QDemonRenderWinding
{
    Unknown = 0,
    Clockwise,
    CounterClockwise
};

inline const char *toString(QDemonRenderWinding value)
{
    switch (value) {
    case QDemonRenderWinding::Clockwise:
        return "Clockwise";
    case QDemonRenderWinding::CounterClockwise:
        return "CounterClockwise";
    default:
        break;
    }
    return "Unknown";
}

enum class QDemonRenderState
{
    Unknown = 0,
    Blend,
    CullFace,
    DepthTest,
    StencilTest,
    ScissorTest,
    DepthWrite,
    Multisample
};

inline const char *toString(QDemonRenderState value)
{
    switch (value) {
    case QDemonRenderState::Blend:
        return "Blend";
    case QDemonRenderState::CullFace:
        return "CullFace";
    case QDemonRenderState::DepthTest:
        return "DepthTest";
    case QDemonRenderState::StencilTest:
        return "StencilTest";
    case QDemonRenderState::ScissorTest:
        return "ScissorTest";
    case QDemonRenderState::DepthWrite:
        return "DepthWrite";
    case QDemonRenderState::Multisample:
        return "Multisample";
    default:
        break;
    }
    return "Unknown";
}

enum class QDemonRenderSrcBlendFunc
{
    Unknown = 0,
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    ConstantColor,
    OneMinusConstantColor,
    ConstantAlpha,
    OneMinusConstantAlpha,
    SrcAlphaSaturate
};

inline const char *toString(QDemonRenderSrcBlendFunc value)
{
    switch (value) {
    case QDemonRenderSrcBlendFunc::Zero:
        return "Zero";
    case QDemonRenderSrcBlendFunc::One:
        return "One";
    case QDemonRenderSrcBlendFunc::SrcColor:
        return "SrcColor";
    case QDemonRenderSrcBlendFunc::OneMinusSrcColor:
        return "OneMinusSrcColor";
    case QDemonRenderSrcBlendFunc::DstColor:
        return "DstColor";
    case QDemonRenderSrcBlendFunc::OneMinusDstColor:
        return "OneMinusDstColor";
    case QDemonRenderSrcBlendFunc::SrcAlpha:
        return "SrcAlpha";
    case QDemonRenderSrcBlendFunc::OneMinusSrcAlpha:
        return "OneMinusSrcAlpha";
    case QDemonRenderSrcBlendFunc::DstAlpha:
        return "DstAlpha";
    case QDemonRenderSrcBlendFunc::OneMinusDstAlpha:
        return "OneMinusDstAlpha";
    case QDemonRenderSrcBlendFunc::ConstantColor:
        return "ConstantColor";
    case QDemonRenderSrcBlendFunc::OneMinusConstantColor:
        return "OneMinusConstantColor";
    case QDemonRenderSrcBlendFunc::ConstantAlpha:
        return "ConstantAlpha";
    case QDemonRenderSrcBlendFunc::OneMinusConstantAlpha:
        return "OneMinusConstantAlpha";
    case QDemonRenderSrcBlendFunc::SrcAlphaSaturate:
        return "SrcAlphaSaturate";
    default:
        break;
    }
    return "Unknown";
}

enum class QDemonRenderDstBlendFunc
{
    Unknown = 0,
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    ConstantColor,
    OneMinusConstantColor,
    ConstantAlpha,
    OneMinusConstantAlpha
};

inline const char *toString(QDemonRenderDstBlendFunc value)
{
    return toString(static_cast<QDemonRenderSrcBlendFunc>(value));
}

enum class QDemonRenderBlendEquation
{
    Unknown = 0,
    Add,
    Subtract,
    ReverseSubtract,
    Overlay,
    ColorBurn,
    ColorDodge
};

inline const char *toString(QDemonRenderBlendEquation value)
{
    switch (value) {
    case QDemonRenderBlendEquation::Add:
        return "Add";
    case QDemonRenderBlendEquation::Subtract:
        return "Subtract";
    case QDemonRenderBlendEquation::ReverseSubtract:
        return "ReverseSubtract";
    case QDemonRenderBlendEquation::Overlay:
        return "Overlay";
    case QDemonRenderBlendEquation::ColorBurn:
        return "ColorBurn";
    case QDemonRenderBlendEquation::ColorDodge:
        return "ColorDodge";
    default:
        break;
    }
    return "Unknown";
}

enum class QDemonRenderFace
{
    Unknown = 0,
    Front,
    Back,
    FrontAndBack
};

inline const char *toString(QDemonRenderFace value)
{
    switch (value) {
    case QDemonRenderFace::Front:
        return "Front";
    case QDemonRenderFace::Back:
        return "Back";
    case QDemonRenderFace::FrontAndBack:
        return "FrontAndBack";
    default:
        break;
    }
    return "Unknown";
}

enum class QDemonReadFace
{
    Unknown = 0,
    Front,
    Back,
    Color0,
    Color1,
    Color2,
    Color3,
    Color4,
    Color5,
    Color6,
    Color7
};

inline const char *toString(QDemonReadFace value)
{
    switch (value) {
    case QDemonReadFace::Front:
        return "Front";
    case QDemonReadFace::Back:
        return "Back";
    case QDemonReadFace::Color0:
        return "Color0";
    case QDemonReadFace::Color1:
        return "Color1";
    case QDemonReadFace::Color2:
        return "Color2";
    case QDemonReadFace::Color3:
        return "Color3";
    case QDemonReadFace::Color4:
        return "Color4";
    case QDemonReadFace::Color5:
        return "Color5";
    case QDemonReadFace::Color6:
        return "Color6";
    case QDemonReadFace::Color7:
        return "Color7";
    default:
        break;
    }
    return "Unknown";
}

enum class QDemonRenderBoolOp
{

    Unknown = 0,
    Never,
    Less,
    LessThanOrEqual,
    Equal,
    NotEqual,
    Greater,
    GreaterThanOrEqual,
    AlwaysTrue
};

inline const char *toString(QDemonRenderBoolOp value)
{
    switch (value) {
    case QDemonRenderBoolOp::Never:
        return "Never";
    case QDemonRenderBoolOp::Less:
        return "Less";
    case QDemonRenderBoolOp::LessThanOrEqual:
        return "LessThanOrEqual";
    case QDemonRenderBoolOp::Equal:
        return "Equal";
    case QDemonRenderBoolOp::NotEqual:
        return "NotEqual";
    case QDemonRenderBoolOp::Greater:
        return "Greater";
    case QDemonRenderBoolOp::GreaterThanOrEqual:
        return "GreaterThanOrEqual";
    case QDemonRenderBoolOp::AlwaysTrue:
        return "AlwaysTrue";
    default:
        break;
    }
    return "Unknown";
}

enum class QDemonRenderStencilOp
{
    Unknown = 0,
    Keep,
    Zero,
    Replace,
    Increment,
    IncrementWrap,
    Decrement,
    DecrementWrap,
    Invert
};

inline const char *toString(QDemonRenderStencilOp value)
{
    switch (value) {
    case QDemonRenderStencilOp::Keep:
        return "Keep";
    case QDemonRenderStencilOp::Zero:
        return "Zero";
    case QDemonRenderStencilOp::Replace:
        return "Replace";
    case QDemonRenderStencilOp::Increment:
        return "Increment";
    case QDemonRenderStencilOp::IncrementWrap:
        return "IncrementWrap";
    case QDemonRenderStencilOp::Decrement:
        return "Decrement";
    case QDemonRenderStencilOp::DecrementWrap:
        return "DecrementWrap";
    case QDemonRenderStencilOp::Invert:
        return "Invert";
    default:
        break;
    }
    return "Unknown";
}

struct QDemonRenderBlendFunctionArgument
{
    QDemonRenderSrcBlendFunc m_srcRgb;
    QDemonRenderDstBlendFunc m_dstRgb;
    QDemonRenderSrcBlendFunc m_srcAlpha;
    QDemonRenderDstBlendFunc m_dstAlpha;

    QDemonRenderBlendFunctionArgument(QDemonRenderSrcBlendFunc srcRGB,
                                      QDemonRenderDstBlendFunc dstRGB,
                                      QDemonRenderSrcBlendFunc srcAlpha,
                                      QDemonRenderDstBlendFunc dstAlpha)
        : m_srcRgb(srcRGB), m_dstRgb(dstRGB), m_srcAlpha(srcAlpha), m_dstAlpha(dstAlpha)
    {
    }

    // Default blend system premultiplies values.
    QDemonRenderBlendFunctionArgument()
        : m_srcRgb(QDemonRenderSrcBlendFunc::SrcAlpha)
        , m_dstRgb(QDemonRenderDstBlendFunc::OneMinusSrcAlpha)
        , m_srcAlpha(QDemonRenderSrcBlendFunc::One)
        , m_dstAlpha(QDemonRenderDstBlendFunc::OneMinusSrcAlpha)
    {
    }
};

struct QDemonRenderBlendEquationArgument
{
    QDemonRenderBlendEquation m_rgbEquation;
    QDemonRenderBlendEquation m_alphaEquation;

    QDemonRenderBlendEquationArgument(QDemonRenderBlendEquation rgb, QDemonRenderBlendEquation alpha)
        : m_rgbEquation(rgb), m_alphaEquation(alpha)
    {
    }
    QDemonRenderBlendEquationArgument()
        : m_rgbEquation(QDemonRenderBlendEquation::Add), m_alphaEquation(QDemonRenderBlendEquation::Add)
    {
    }
};

struct QDemonRenderStencilOperation
{
    QDemonRenderStencilOp m_stencilFail = QDemonRenderStencilOp::Keep; // What happens when stencil test fails.
    // These values assume the stencil passed

    // What happens when the stencil passes but depth test fail.
    QDemonRenderStencilOp m_depthFail = QDemonRenderStencilOp::Keep;
     // What happens when the stencil and depth tests pass.
    QDemonRenderStencilOp m_depthPass = QDemonRenderStencilOp::Keep;

    QDemonRenderStencilOperation(QDemonRenderStencilOp fail,
                                         QDemonRenderStencilOp depthFail,
                                         QDemonRenderStencilOp depthPass)
        : m_stencilFail(fail), m_depthFail(depthFail), m_depthPass(depthPass)
    {
    }
    QDemonRenderStencilOperation() = default;

    bool operator==(const QDemonRenderStencilOperation &other) const
    {
        return (m_stencilFail == other.m_stencilFail && m_depthFail == other.m_depthFail && m_depthPass == other.m_depthPass);
    }
};

// see glStencilFuncSeparate
struct QDemonRenderStencilFunction
{
    QDemonRenderBoolOp m_function = QDemonRenderBoolOp::AlwaysTrue;
    quint32 m_referenceValue = 0;
    quint32 m_mask = std::numeric_limits<quint32>::max();

    QDemonRenderStencilFunction(QDemonRenderBoolOp function, quint32 referenceValue, quint32 mask)
        : m_function(function), m_referenceValue(referenceValue), m_mask(mask)
    {
    }
    QDemonRenderStencilFunction() = default;

    bool operator==(const QDemonRenderStencilFunction &other) const
    {
        return (m_function == other.m_function && m_referenceValue == other.m_referenceValue && m_mask == other.m_mask);
    }
};

class QDemonRenderFrameBuffer;
class QDemonRenderVertexBuffer;
class QDemonRenderIndexBuffer;
class QDemonRenderProgramPipeline;
class QDemonRenderTextureBase;
class QDemonRenderTexture2D;
class QDemonRenderTexture2DArray;
class QDemonRenderTextureCube;
class QDemonRenderImage2D;
class QDemonRenderDataBuffer;
class QDemonRenderAttribLayout;
class QDemonRenderInputAssembler;

// Return coordinates in pixels but relative to this rect.
static inline constexpr QVector2D toRectRelative(const QRectF &r, const QVector2D &absoluteCoordinates)
{
    return QVector2D(absoluteCoordinates.x() - float(r.x()), absoluteCoordinates.y() - float(r.y()));
}

static inline constexpr QVector2D halfDims(const QRectF &r)
{
    return QVector2D(float(r.width() / 2.0), float(r.height() / 2.0));
}

// Take coordinates in global space and move local space where 0,0 is the center
// of the rect but return value in pixels, not in normalized -1,1 range
static inline QVector2D toNormalizedRectRelative(const QRectF &r, QVector2D absoluteCoordinates)
{
    // normalize them
    const QVector2D relativeCoords(toRectRelative(r, absoluteCoordinates));
    const QVector2D halfD(halfDims(r));
    const QVector2D normalized((relativeCoords.x() / halfD.x()) - 1.0f, (relativeCoords.y() / halfD.y()) - 1.0f);
    return QVector2D(normalized.x() * halfD.x(), normalized.y() * halfD.y());
}

static inline constexpr QVector2D relativeToNormalizedCoordinates(const QRectF &r, QVector2D rectRelativeCoords)
{
    return { (rectRelativeCoords.x() / halfDims(r).x()) - 1.0f, (rectRelativeCoords.y() / halfDims(r).y()) - 1.0f };
}

// Normalized coordinates are in the range of -1,1 where -1 is the left, bottom edges
// and 1 is the top,right edges.
static inline constexpr QVector2D absoluteToNormalizedCoordinates(const QRectF &r, const QVector2D &absoluteCoordinates)
{
    return relativeToNormalizedCoordinates(r, toRectRelative(r, absoluteCoordinates));
}

static inline constexpr QVector2D toAbsoluteCoords(const QRectF &r, const QVector2D &inRelativeCoords)
{
    return QVector2D(inRelativeCoords.x() + float(r.x()), inRelativeCoords.y() + float(r.y()));
}

template<typename TDataType>
struct QDemonRenderGenericVec2
{
    TDataType x;
    TDataType y;
    QDemonRenderGenericVec2(TDataType _x, TDataType _y) : x(_x), y(_y) {}
    QDemonRenderGenericVec2() {}
    bool operator==(const QDemonRenderGenericVec2 &inOther) const { return x == inOther.x && y == inOther.y; }
};

template<typename TDataType>
struct QDemonRenderGenericVec3
{
    TDataType x;
    TDataType y;
    TDataType z;
    QDemonRenderGenericVec3(TDataType _x, TDataType _y, TDataType _z) : x(_x), y(_y), z(_z) {}
    QDemonRenderGenericVec3() {}
    bool operator==(const QDemonRenderGenericVec3 &inOther) const
    {
        return x == inOther.x && y == inOther.y && z == inOther.z;
    }
};

template<typename TDataType>
struct QDemonRenderGenericVec4
{
    TDataType x;
    TDataType y;
    TDataType z;
    TDataType w;
    QDemonRenderGenericVec4(TDataType _x, TDataType _y, TDataType _z, TDataType _w) : x(_x), y(_y), z(_z), w(_w) {}
    QDemonRenderGenericVec4() {}
    bool operator==(const QDemonRenderGenericVec4 &inOther) const
    {
        return x == inOther.x && y == inOther.y && z == inOther.z && w == inOther.w;
    }
};

typedef QDemonRenderGenericVec2<bool> bool_2;
typedef QDemonRenderGenericVec3<bool> bool_3;
typedef QDemonRenderGenericVec4<bool> bool_4;
typedef QDemonRenderGenericVec2<quint32> quint32_2;
typedef QDemonRenderGenericVec3<quint32> quint32_3;
typedef QDemonRenderGenericVec4<quint32> quint32_4;
typedef QDemonRenderGenericVec2<qint32> qint32_2;
typedef QDemonRenderGenericVec3<qint32> qint32_3;
typedef QDemonRenderGenericVec4<qint32> qint32_4;

enum class QDemonRenderShaderDataType : quint32
{
    Unknown = 0,
    Integer, // qint32,
    IntegerVec2, // qint32_2,
    IntegerVec3, // qint32_3,
    IntegerVec4, // qint32_4,
    Boolean, // bool
    BooleanVec2, // bool_2,
    BooleanVec3, // bool_3,
    BooleanVec4, // bool_4,
    Float, // float,
    Vec2, // QVector2D,
    Vec3, // QVector3D,
    Vec4, // QVector4D,
    UnsignedInteger, // quint32,
    UnsignedIntegerVec2, // quint32_2,
    UnsignedIntegerVec3, // quint32_3,
    UnsignedIntegerVec4, // quint32_4,
    Matrix3x3, // QMatrix3x3,
    Matrix4x4, // QMatrix4x4,
    Texture2D, // QDemonRenderTexture2D *,
    Texture2DHandle, // QDemonRenderTexture2D **,
    Texture2DArray, // QDemonRenderTexture2DArray *,
    TextureCube, // QDemonRenderTextureCube *,
    TextureCubeHandle, // QDemonRenderTextureCube **,
    Image2D, // QDemonRenderImage2D *,
    DataBuffer // QDemonRenderDataBuffer *
};

template<typename TDataType>
struct QDemonDataTypeToShaderDataTypeMap
{
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<qint32>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::Integer; }
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<qint32_2>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::IntegerVec2; }
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<qint32_3>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::IntegerVec3; }
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<qint32_4>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::IntegerVec4; }
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<bool>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::Boolean; }
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<bool_2>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::BooleanVec2; }
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<bool_3>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::BooleanVec3; }
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<bool_4>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::BooleanVec4; }
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<float>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::Float; }
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<QVector2D>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::Vec2; }
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<QVector3D>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::Vec3; }
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<QVector4D>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::Vec4; }
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<quint32>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::UnsignedInteger; }
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<quint32_2>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::UnsignedIntegerVec2; }
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<quint32_3>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::UnsignedIntegerVec3; }
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<quint32_4>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::UnsignedIntegerVec4; }
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<QMatrix3x3>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::Matrix3x3; }
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<QMatrix4x4>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::Matrix4x4; }
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<QDemonDataView<QMatrix4x4>>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::Matrix4x4; }
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<QDemonRenderTexture2D *>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::Texture2D; }
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<QDemonRenderTexture2D **>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::Texture2DHandle; }
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<QDemonRenderTexture2DArray *>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::Texture2DArray; }
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<QDemonRenderTextureCube *>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::TextureCube; }
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<QDemonRenderTextureCube **>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::TextureCubeHandle; }
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<QDemonRenderImage2D *>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::Image2D; }
};

template<>
struct QDemonDataTypeToShaderDataTypeMap<QDemonRenderDataBuffer *>
{
    static QDemonRenderShaderDataType getType() { return QDemonRenderShaderDataType::DataBuffer; }
};

enum class QDemonRenderShaderTypeValue
{
    Unknown = 1 << 0,
    Vertex = 1 << 1,
    Fragment = 1 << 2,
    TessControl = 1 << 3,
    TessEvaluation = 1 << 4,
    Geometry = 1 << 5
};

Q_DECLARE_FLAGS(QDemonRenderShaderTypeFlags, QDemonRenderShaderTypeValue)
Q_DECLARE_OPERATORS_FOR_FLAGS(QDemonRenderShaderTypeFlags)

enum class QDemonRenderTextureTypeValue
{
    Unknown = 0,
    Diffuse,
    Specular,
    Environment,
    Bump,
    Normal,
    Displace,
    Emissive,
    Emissive2,
    Anisotropy,
    Translucent,
    LightmapIndirect,
    LightmapRadiosity,
    LightmapShadow
};

inline const char *toString(QDemonRenderTextureTypeValue value)
{
    switch (value) {
    case QDemonRenderTextureTypeValue::Unknown:
        return "Unknown";
    case QDemonRenderTextureTypeValue::Diffuse:
        return "Diffuse";
    case QDemonRenderTextureTypeValue::Specular:
        return "Specular";
    case QDemonRenderTextureTypeValue::Environment:
        return "Environment";
    case QDemonRenderTextureTypeValue::Bump:
        return "Bump";
    case QDemonRenderTextureTypeValue::Normal:
        return "Normal";
    case QDemonRenderTextureTypeValue::Displace:
        return "Displace";
    case QDemonRenderTextureTypeValue::Emissive:
        return "Emissive";
    case QDemonRenderTextureTypeValue::Emissive2:
        return "Emissive2";
    case QDemonRenderTextureTypeValue::Anisotropy:
        return "Anisotropy";
    case QDemonRenderTextureTypeValue::Translucent:
        return "Translucent";
    case QDemonRenderTextureTypeValue::LightmapIndirect:
        return "LightmapIndirect";
    case QDemonRenderTextureTypeValue::LightmapRadiosity:
        return "LightmapRadiosity";
    case QDemonRenderTextureTypeValue::LightmapShadow:
        return "LightmapShadow";
    }
    return nullptr;
}

enum class QDemonRenderReadPixelFormat
{
    Alpha8,
    RGB565,
    RGB8,
    RGBA4444,
    RGBA5551,
    RGBA8
};

static inline int sizeofPixelFormat(QDemonRenderReadPixelFormat f)
{
    switch (f) {
    case QDemonRenderReadPixelFormat::Alpha8:
        return 1;
    case QDemonRenderReadPixelFormat::RGB565:
    case QDemonRenderReadPixelFormat::RGBA5551:
    case QDemonRenderReadPixelFormat::RGBA4444:
        return 2;
    case QDemonRenderReadPixelFormat::RGB8:
        return 3;
    case QDemonRenderReadPixelFormat::RGBA8:
        return 4;
    }
}

// Now for scoped property access.
template<typename TBaseType, typename TDataType>
struct QDemonRenderGenericScopedProperty
{
    typedef void (TBaseType::*TSetter)(TDataType inType);
    typedef TDataType (TBaseType::*TGetter)() const;

    TBaseType &m_context;
    TSetter m_setter;
    TDataType m_initialValue;
    QDemonRenderGenericScopedProperty(TBaseType &ctx, TGetter getter, TSetter setter)
        : m_context(ctx), m_setter(setter), m_initialValue(((ctx).*getter)())
    {
    }
    QDemonRenderGenericScopedProperty(TBaseType &ctx, TGetter getter, TSetter setter, const TDataType &inNewValue)
        : m_context(ctx), m_setter(setter), m_initialValue(((ctx).*getter)())
    {
        ((m_context).*m_setter)(inNewValue);
    }
    ~QDemonRenderGenericScopedProperty() { ((m_context).*m_setter)(m_initialValue); }
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(bool_2)
Q_DECLARE_METATYPE(bool_3)
Q_DECLARE_METATYPE(bool_4)
Q_DECLARE_METATYPE(quint32_2)
Q_DECLARE_METATYPE(quint32_3)
Q_DECLARE_METATYPE(quint32_4)
Q_DECLARE_METATYPE(qint32_2)
Q_DECLARE_METATYPE(qint32_3)
Q_DECLARE_METATYPE(qint32_4)

#endif
