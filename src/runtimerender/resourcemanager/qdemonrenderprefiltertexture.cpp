/****************************************************************************
**
** Copyright (C) 2008-2016 NVIDIA Corporation.
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

#include "qdemonrenderprefiltertexture.h"

#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>

QT_BEGIN_NAMESPACE

QDemonRenderPrefilterTexture::QDemonRenderPrefilterTexture(const QDemonRef<QDemonRenderContext> &inQDemonRenderContext,
                                                           qint32 inWidth,
                                                           qint32 inHeight,
                                                           const QDemonRef<QDemonRenderTexture2D> &inTexture2D,
                                                           QDemonRenderTextureFormat inDestFormat)
    : m_texture2D(inTexture2D), m_destinationFormat(inDestFormat), m_width(inWidth), m_height(inHeight), m_renderContext(inQDemonRenderContext)
{
    // Calculate mip level
    int maxDim = inWidth >= inHeight ? inWidth : inHeight;

    m_maxMipMapLevel = static_cast<int>(logf((float)maxDim) / logf(2.0f));
    // no concept of sizeOfFormat just does'nt make sense
    m_sizeOfFormat = m_destinationFormat.getSizeofFormat();
    m_noOfComponent = m_destinationFormat.getNumberOfComponent();
}

QDemonRef<QDemonRenderPrefilterTexture> QDemonRenderPrefilterTexture::create(const QDemonRef<QDemonRenderContext> &inQDemonRenderContext,
                                                                             qint32 inWidth,
                                                                             qint32 inHeight,
                                                                             const QDemonRef<QDemonRenderTexture2D> &inTexture2D,
                                                                             QDemonRenderTextureFormat inDestFormat)
{
    QDemonRef<QDemonRenderPrefilterTexture> theBSDFMipMap;

    if (inQDemonRenderContext->isComputeSupported()) {
        theBSDFMipMap = new QDemonRenderPrefilterTextureCompute(inQDemonRenderContext, inWidth, inHeight, inTexture2D, inDestFormat);
    }

    if (!theBSDFMipMap) {
        theBSDFMipMap = new QDemonRenderPrefilterTextureCPU(inQDemonRenderContext, inWidth, inHeight, inTexture2D, inDestFormat);
    }

    return theBSDFMipMap;
}

QDemonRenderPrefilterTexture::~QDemonRenderPrefilterTexture() = default;

//------------------------------------------------------------------------------------
// CPU based filtering
//------------------------------------------------------------------------------------

QDemonRenderPrefilterTextureCPU::QDemonRenderPrefilterTextureCPU(const QDemonRef<QDemonRenderContext> &inQDemonRenderContext,
                                                                 int inWidth,
                                                                 int inHeight,
                                                                 const QDemonRef<QDemonRenderTexture2D> &inTexture2D,
                                                                 QDemonRenderTextureFormat inDestFormat)
    : QDemonRenderPrefilterTexture(inQDemonRenderContext, inWidth, inHeight, inTexture2D, inDestFormat)
{
}

inline int QDemonRenderPrefilterTextureCPU::wrapMod(int a, int base)
{
    return (a >= 0) ? a % base : (a % base) + base;
}

inline void QDemonRenderPrefilterTextureCPU::getWrappedCoords(int &sX, int &sY, int width, int height)
{
    if (sY < 0) {
        sX -= width >> 1;
        sY = -sY;
    }
    if (sY >= height) {
        sX += width >> 1;
        sY = height - sY;
    }
    sX = wrapMod(sX, width);
}

QDemonTextureData QDemonRenderPrefilterTextureCPU::createBsdfMipLevel(QDemonTextureData &inCurMipLevel,
                                                                      QDemonTextureData &inPrevMipLevel,
                                                                      int width,
                                                                      int height) //, IPerfTimer& inPerfTimer )
{
    QDemonTextureData retval;
    int newWidth = width >> 1;
    int newHeight = height >> 1;
    newWidth = newWidth >= 1 ? newWidth : 1;
    newHeight = newHeight >= 1 ? newHeight : 1;

    if (inCurMipLevel.data) {
        retval = inCurMipLevel;
        retval.dataSizeInBytes = newWidth * newHeight * inPrevMipLevel.format.getSizeofFormat();
    } else {
        retval.dataSizeInBytes = newWidth * newHeight * inPrevMipLevel.format.getSizeofFormat();
        retval.format = inPrevMipLevel.format; // inLoadedImage.format;
        retval.data = ::malloc(retval.dataSizeInBytes);
    }

    for (int y = 0; y < newHeight; ++y) {
        for (int x = 0; x < newWidth; ++x) {
            float accumVal[4];
            accumVal[0] = 0;
            accumVal[1] = 0;
            accumVal[2] = 0;
            accumVal[3] = 0;
            for (int sy = -2; sy <= 2; ++sy) {
                for (int sx = -2; sx <= 2; ++sx) {
                    int sampleX = sx + (x << 1);
                    int sampleY = sy + (y << 1);
                    getWrappedCoords(sampleX, sampleY, width, height);

                    // Cauchy filter (this is simply because it's the easiest to evaluate, and
                    // requires no complex
                    // functions).
                    float filterPdf = 1.f / (1.f + float(sx * sx + sy * sy) * 2.f);
                    // With FP HDR formats, we're not worried about intensity loss so much as
                    // unnecessary energy gain,
                    // whereas with LDR formats, the fear with a continuous normalization factor is
                    // that we'd lose
                    // intensity and saturation as well.
                    filterPdf /= (retval.format.getSizeofFormat() >= 8) ? 4.71238898f : 4.5403446f;
                    // filterPdf /= 4.5403446f;		// Discrete normalization factor
                    // filterPdf /= 4.71238898f;		// Continuous normalization factor
                    float curPix[4];
                    qint32 byteOffset = (sampleY * width + sampleX) * retval.format.getSizeofFormat();
                    if (byteOffset < 0) {
                        sampleY = height + sampleY;
                        byteOffset = (sampleY * width + sampleX) * retval.format.getSizeofFormat();
                    }

                    retval.format.decodeToFloat(inPrevMipLevel.data, byteOffset, curPix);

                    accumVal[0] += filterPdf * curPix[0];
                    accumVal[1] += filterPdf * curPix[1];
                    accumVal[2] += filterPdf * curPix[2];
                    accumVal[3] += filterPdf * curPix[3];
                }
            }

            quint32 newIdx = (y * newWidth + x) * retval.format.getSizeofFormat();

            retval.format.encodeToPixel(accumVal, retval.data, newIdx);
        }
    }

    return retval;
}

void QDemonRenderPrefilterTextureCPU::build(void *inTextureData, qint32 inTextureDataSize, QDemonRenderTextureFormat inFormat)
{
    m_sizeOfInternalFormat = inFormat.getSizeofFormat();
    m_internalNoOfComponent = inFormat.getNumberOfComponent();

    m_texture2D->setTextureData(QDemonDataRef<quint8>((quint8 *)inTextureData, inTextureDataSize), 0, m_width, m_height, inFormat, m_destinationFormat);

    QDemonTextureData theMipImage;
    QDemonTextureData prevImage;
    prevImage.data = inTextureData;
    prevImage.dataSizeInBytes = inTextureDataSize;
    prevImage.format = inFormat;
    int curWidth = m_width;
    int curHeight = m_height;
    int size = inFormat.getSizeofFormat();
    for (int idx = 1; idx <= m_maxMipMapLevel; ++idx) {
        theMipImage = createBsdfMipLevel(theMipImage, prevImage, curWidth, curHeight); //, m_PerfTimer );
        curWidth = curWidth >> 1;
        curHeight = curHeight >> 1;
        curWidth = curWidth >= 1 ? curWidth : 1;
        curHeight = curHeight >= 1 ? curHeight : 1;
        inTextureDataSize = curWidth * curHeight * size;

        m_texture2D->setTextureData(toU8DataRef((char *)theMipImage.data, (quint32)inTextureDataSize),
                                    (quint8)idx,
                                    (quint32)curWidth,
                                    (quint32)curHeight,
                                    theMipImage.format,
                                    m_destinationFormat);

        if (prevImage.data == inTextureData)
            prevImage = QDemonTextureData();

        QDemonTextureData temp = prevImage;
        prevImage = theMipImage;
        theMipImage = temp;
    }
    ::free(theMipImage.data);
    ::free(prevImage.data);
}

//------------------------------------------------------------------------------------
// GL compute based filtering
//------------------------------------------------------------------------------------

static const char *computeUploadShader(QByteArray &prog, QDemonRenderTextureFormat inFormat, bool binESContext)
{
    if (binESContext) {
        prog += "#version 310 es\n"
                "#extension GL_ARB_compute_shader : enable\n"
                "precision highp float;\n"
                "precision highp int;\n"
                "precision mediump image2D;\n";
    } else {
        prog += "#version 430\n"
                "#extension GL_ARB_compute_shader : enable\n";
    }

    if (inFormat == QDemonRenderTextureFormat::RGBA8) {
        prog += "// Set workgroup layout;\n"
                "layout (local_size_x = 16, local_size_y = 16) in;\n\n"
                "layout (rgba8, binding = 1) readonly uniform image2D inputImage;\n\n"
                "layout (rgba16f, binding = 2) writeonly uniform image2D outputImage;\n\n"
                "void main()\n"
                "{\n"
                "  if ( gl_GlobalInvocationID.x >= gl_NumWorkGroups.x || gl_GlobalInvocationID.y "
                ">= gl_NumWorkGroups.y )\n"
                "    return;\n"
                "  vec4 value = imageLoad(inputImage, ivec2(gl_GlobalInvocationID.xy));\n"
                "  imageStore( outputImage, ivec2(gl_GlobalInvocationID.xy), value );\n"
                "}\n";
    } else {
        prog += "float convertToFloat( in uint inValue )\n"
                "{\n"
                "  uint v = inValue & uint(0xFF);\n"
                "  float f = float(v)/256.0;\n"
                "  return f;\n"
                "}\n";

        prog += "int getMod( in int inValue, in int mod )\n"
                "{\n"
                "  int v = mod * (inValue/mod);\n"
                "  return inValue - v;\n"
                "}\n";

        prog += "vec4 getRGBValue( in int byteNo, vec4 inVal, vec4 inVal1 )\n"
                "{\n"
                "  vec4 result= vec4(0.0);\n"
                "  if( byteNo == 0) {\n"
                "    result.r = inVal.r;\n"
                "    result.g = inVal.g;\n"
                "    result.b = inVal.b;\n"
                "  }\n"
                "  else if( byteNo == 1) {\n"
                "    result.r = inVal.g;\n"
                "    result.g = inVal.b;\n"
                "    result.b = inVal.a;\n"
                "  }\n"
                "  else if( byteNo == 2) {\n"
                "    result.r = inVal.b;\n"
                "    result.g = inVal.a;\n"
                "    result.b = inVal1.r;\n"
                "  }\n"
                "  else if( byteNo == 3) {\n"
                "    result.r = inVal.a;\n"
                "    result.g = inVal1.r;\n"
                "    result.b = inVal1.g;\n"
                "  }\n"
                "  return result;\n"
                "}\n";

        prog += "// Set workgroup layout;\n"
                "layout (local_size_x = 16, local_size_y = 16) in;\n\n"
                "layout (rgba8, binding = 1) readonly uniform image2D inputImage;\n\n"
                "layout (rgba16f, binding = 2) writeonly uniform image2D outputImage;\n\n"
                "void main()\n"
                "{\n"
                "  vec4 result = vec4(0.0);\n"
                "  if ( gl_GlobalInvocationID.x >= gl_NumWorkGroups.x || gl_GlobalInvocationID.y "
                ">= gl_NumWorkGroups.y )\n"
                "    return;\n"
                "  int xpos = (int(gl_GlobalInvocationID.x)*3)/4;\n"
                "  int xmod = getMod(int(gl_GlobalInvocationID.x)*3, 4);\n"
                "  ivec2 readPos = ivec2(xpos, gl_GlobalInvocationID.y);\n"
                "  vec4 value = imageLoad(inputImage, readPos);\n"
                "  vec4 value1 = imageLoad(inputImage, ivec2(readPos.x + 1, readPos.y));\n"
                "  result = getRGBValue( xmod, value, value1);\n"
                "  imageStore( outputImage, ivec2(gl_GlobalInvocationID.xy), result );\n"
                "}\n";
    }
    return prog.constData();
}

static const char *computeWorkShader(QByteArray &prog, bool binESContext)
{
    if (binESContext) {
        prog += "#version 310 es\n"
                "#extension GL_ARB_compute_shader : enable\n"
                "precision highp float;\n"
                "precision highp int;\n"
                "precision mediump image2D;\n";
    } else {
        prog += "#version 430\n"
                "#extension GL_ARB_compute_shader : enable\n";
    }

    prog += "int wrapMod( in int a, in int base )\n"
            "{\n"
            "  return ( a >= 0 ) ? a % base : -(a % base) + base;\n"
            "}\n";

    prog += "void getWrappedCoords( inout int sX, inout int sY, in int width, in int height )\n"
            "{\n"
            "  if (sY < 0) { sX -= width >> 1; sY = -sY; }\n"
            "  if (sY >= height) { sX += width >> 1; sY = height - sY; }\n"
            "  sX = wrapMod( sX, width );\n"
            "}\n";

    prog += "// Set workgroup layout;\n"
            "layout (local_size_x = 16, local_size_y = 16) in;\n\n"
            "layout (rgba16f, binding = 1) readonly uniform image2D inputImage;\n\n"
            "layout (rgba16f, binding = 2) writeonly uniform image2D outputImage;\n\n"
            "void main()\n"
            "{\n"
            "  int prevWidth = int(gl_NumWorkGroups.x) << 1;\n"
            "  int prevHeight = int(gl_NumWorkGroups.y) << 1;\n"
            "  if ( gl_GlobalInvocationID.x >= gl_NumWorkGroups.x || gl_GlobalInvocationID.y >= "
            "gl_NumWorkGroups.y )\n"
            "    return;\n"
            "  vec4 accumVal = vec4(0.0);\n"
            "  for ( int sy = -2; sy <= 2; ++sy )\n"
            "  {\n"
            "    for ( int sx = -2; sx <= 2; ++sx )\n"
            "    {\n"
            "      int sampleX = sx + (int(gl_GlobalInvocationID.x) << 1);\n"
            "      int sampleY = sy + (int(gl_GlobalInvocationID.y) << 1);\n"
            "      getWrappedCoords(sampleX, sampleY, prevWidth, prevHeight);\n"
            "	   if ((sampleY * prevWidth + sampleX) < 0 )\n"
            "        sampleY = prevHeight + sampleY;\n"
            "      ivec2 pos = ivec2(sampleX, sampleY);\n"
            "      vec4 value = imageLoad(inputImage, pos);\n"
            "      float filterPdf = 1.0 / ( 1.0 + float(sx*sx + sy*sy)*2.0 );\n"
            "      filterPdf /= 4.71238898;\n"
            "      accumVal[0] += filterPdf * value.r;\n"
            "	   accumVal[1] += filterPdf * value.g;\n"
            "	   accumVal[2] += filterPdf * value.b;\n"
            "	   accumVal[3] += filterPdf * value.a;\n"
            "    }\n"
            "  }\n"
            "  imageStore( outputImage, ivec2(gl_GlobalInvocationID.xy), accumVal );\n"
            "}\n";

    return prog.constData();
}

inline QDemonConstDataRef<qint8> toRef(const char *data)
{
    size_t len = strlen(data) + 1;
    return QDemonConstDataRef<qint8>((const qint8 *)data, (quint32)len);
}

static bool isGLESContext(const QDemonRef<QDemonRenderContext> &context)
{
    QDemonRenderContextType ctxType = context->getRenderContextType();

    // Need minimum of GL3 or GLES3
    if (ctxType == QDemonRenderContextType::GLES2 || ctxType == QDemonRenderContextType::GLES3
        || ctxType == QDemonRenderContextType::GLES3PLUS) {
        return true;
    }

    return false;
}

#define WORKGROUP_SIZE 16

QDemonRenderPrefilterTextureCompute::QDemonRenderPrefilterTextureCompute(const QDemonRef<QDemonRenderContext> &inQDemonRenderContext,
                                                                         qint32 inWidth,
                                                                         qint32 inHeight,
                                                                         const QDemonRef<QDemonRenderTexture2D> &inTexture2D,
                                                                         QDemonRenderTextureFormat inDestFormat)
    : QDemonRenderPrefilterTexture(inQDemonRenderContext, inWidth, inHeight, inTexture2D, inDestFormat)
{
}

QDemonRenderPrefilterTextureCompute::~QDemonRenderPrefilterTextureCompute() = default;

void QDemonRenderPrefilterTextureCompute::createComputeProgram(const QDemonRef<QDemonRenderContext> &context)
{
    QByteArray computeProg;

    if (!m_bsdfProgram) {
        m_bsdfProgram = context->compileComputeSource("Compute BSDF mipmap shader",
                                                      toRef(computeWorkShader(computeProg, isGLESContext(context))))
                                .m_shader;
    }
}

QDemonRef<QDemonRenderShaderProgram> QDemonRenderPrefilterTextureCompute::getOrCreateUploadComputeProgram(
        const QDemonRef<QDemonRenderContext> &context,
        QDemonRenderTextureFormat inFormat)
{
    QByteArray computeProg;

    if (inFormat == QDemonRenderTextureFormat::RGB8) {
        if (!m_uploadProgram_RGB8) {
            m_uploadProgram_RGB8 = context->compileComputeSource("Compute BSDF mipmap level 0 RGB8 shader",
                                                                 toRef(computeUploadShader(computeProg, inFormat, isGLESContext(context))))
                                           .m_shader;
        }

        return m_uploadProgram_RGB8;
    } else {
        if (!m_uploadProgram_RGBA8) {
            m_uploadProgram_RGBA8 = context->compileComputeSource("Compute BSDF mipmap level 0 RGBA8 shader",
                                                                  toRef(computeUploadShader(computeProg, inFormat, isGLESContext(context))))
                                            .m_shader;
        }

        return m_uploadProgram_RGBA8;
    }
}

void QDemonRenderPrefilterTextureCompute::createLevel0Tex(void *inTextureData, qint32 inTextureDataSize, QDemonRenderTextureFormat inFormat)
{
    QDemonRenderTextureFormat theFormat = inFormat;
    qint32 theWidth = m_width;

    // Since we cannot use RGB format in GL compute
    // we treat it as a RGBA component format
    if (inFormat == QDemonRenderTextureFormat::RGB8) {
        // This works only with 4 byte aligned data
        Q_ASSERT(m_width % 4 == 0);
        theFormat = QDemonRenderTextureFormat::RGBA8;
        theWidth = (m_width * 3) / 4;
    }

    if (m_level0Tex == nullptr) {
        m_level0Tex = m_renderContext->createTexture2D();
        m_level0Tex->setTextureStorage(1, theWidth, m_height, theFormat, theFormat, QDemonDataRef<quint8>((quint8 *)inTextureData, inTextureDataSize));
    } else {
        m_level0Tex->setTextureSubData(QDemonDataRef<quint8>((quint8 *)inTextureData, inTextureDataSize), 0, 0, 0, theWidth, m_height, theFormat);
    }
}

void QDemonRenderPrefilterTextureCompute::build(void *inTextureData, qint32 inTextureDataSize, QDemonRenderTextureFormat inFormat)
{
    bool needMipUpload = (inFormat != m_destinationFormat);
    // re-upload data
    if (!m_textureCreated) {
        m_texture2D->setTextureStorage(m_maxMipMapLevel + 1,
                                       m_width,
                                       m_height,
                                       m_destinationFormat,
                                       inFormat,
                                       (needMipUpload) ? QDemonDataRef<quint8>()
                                                       : QDemonDataRef<quint8>((quint8 *)inTextureData, inTextureDataSize));
        // create a compute shader (if not aloread done) which computes the BSDF mipmaps for this
        // texture
        createComputeProgram(m_renderContext);

        if (!m_bsdfProgram) {
            Q_ASSERT(false);
            return;
        }

        m_textureCreated = true;
    } else if (!needMipUpload) {
        m_texture2D->setTextureSubData(QDemonDataRef<quint8>((quint8 *)inTextureData, inTextureDataSize), 0, 0, 0, m_width, m_height, inFormat);
    }

    if (needMipUpload) {
        createLevel0Tex(inTextureData, inTextureDataSize, inFormat);
    }

    QDemonRef<QDemonRenderImage2D> theInputImage;
    QDemonRef<QDemonRenderImage2D> theOutputImage;
    theInputImage = m_renderContext->createImage2D(m_texture2D, QDemonRenderImageAccessType::ReadWrite);
    theOutputImage = m_renderContext->createImage2D(m_texture2D, QDemonRenderImageAccessType::ReadWrite);

    if (needMipUpload && m_level0Tex) {
        QDemonRef<QDemonRenderShaderProgram> uploadProg = getOrCreateUploadComputeProgram(m_renderContext, inFormat);
        if (!uploadProg)
            return;

        m_renderContext->setActiveShader(uploadProg);

        QDemonRef<QDemonRenderImage2D> theInputImage0;
        theInputImage0 = m_renderContext->createImage2D(m_level0Tex, QDemonRenderImageAccessType::ReadWrite);

        theInputImage0->setTextureLevel(0);
        QDemonRenderCachedShaderProperty<QDemonRenderImage2D *> theCachedinputImage0("inputImage", uploadProg);
        theCachedinputImage0.set(theInputImage0.data());

        theOutputImage->setTextureLevel(0);
        QDemonRenderCachedShaderProperty<QDemonRenderImage2D *> theCachedOutputImage("outputImage", uploadProg);
        theCachedOutputImage.set(theOutputImage.data());

        m_renderContext->dispatchCompute(uploadProg, m_width, m_height, 1);

        // sync
        QDemonRenderBufferBarrierFlags flags(QDemonRenderBufferBarrierValues::ShaderImageAccess);
        m_renderContext->setMemoryBarrier(flags);
    }

    int width = m_width >> 1;
    int height = m_height >> 1;

    m_renderContext->setActiveShader(m_bsdfProgram);

    for (int i = 1; i <= m_maxMipMapLevel; ++i) {
        theOutputImage->setTextureLevel(i);
        QDemonRenderCachedShaderProperty<QDemonRenderImage2D *> theCachedOutputImage("outputImage", m_bsdfProgram);
        theCachedOutputImage.set(theOutputImage.data());
        theInputImage->setTextureLevel(i - 1);
        QDemonRenderCachedShaderProperty<QDemonRenderImage2D *> theCachedinputImage("inputImage", m_bsdfProgram);
        theCachedinputImage.set(theInputImage.data());

        m_renderContext->dispatchCompute(m_bsdfProgram, width, height, 1);

        width = width > 2 ? width >> 1 : 1;
        height = height > 2 ? height >> 1 : 1;

        // sync
        QDemonRenderBufferBarrierFlags flags(QDemonRenderBufferBarrierValues::ShaderImageAccess);
        m_renderContext->setMemoryBarrier(flags);
    }
}

QT_END_NAMESPACE
