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
#ifndef QDEMON_RENDER_BACKEND_H
#define QDEMON_RENDER_BACKEND_H

#include <QtDemonRender/qtdemonrenderglobal.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemon/QDemonBounds3>

#include <QtGui/qsurfaceformat.h>

QT_BEGIN_NAMESPACE

#define HandleToID_cast(staticType, dynamicType, handle) static_cast<staticType>(reinterpret_cast<dynamicType>(handle))

class Q_DEMONRENDER_EXPORT QDemonRenderBackend
{
    QDemonRenderBackend(const QDemonRenderBackend &) = delete;
    QDemonRenderBackend &operator=(const QDemonRenderBackend &) = delete;

public:
    QAtomicInt ref;

    QDemonRenderBackend() = default;
    virtual ~QDemonRenderBackend() {}
    /// opaque buffer object handle
    typedef struct _QDemonRenderBackendBufferObject *QDemonRenderBackendBufferObject;
    /// opaque attribute layout object handle
    typedef struct _QDemonRenderBackendAttribLayoutObject *QDemonRenderBackendAttribLayoutObject;
    /// opaque input assembler object handle
    typedef struct _QDemonRenderBackendInputAssemblerObject *QDemonRenderBackendInputAssemblerObject;
    /// opaque texture object handle
    typedef struct _QDemonRenderBackendTextureObject *QDemonRenderBackendTextureObject;
    /// opaque sampler object handle
    typedef struct _QDemonRenderBackendSamplerObject *QDemonRenderBackendSamplerObject;
    /// opaque renderbuffer object handle
    typedef struct _QDemonRenderBackendRenderbufferObject *QDemonRenderBackendRenderbufferObject;
    /// opaque framebuffer object handle
    typedef struct _QDemonRenderBackendRenderTargetObject *QDemonRenderBackendRenderTargetObject;
    /// opaque vertex shader object handle
    typedef struct _QDemonRenderBackendVertexShaderObject *QDemonRenderBackendVertexShaderObject;
    /// opaque fragment shader object handle
    typedef struct _QDemonRenderBackendFragmentShaderObject *QDemonRenderBackendFragmentShaderObject;
    /// opaque tesselation control shader object handle
    typedef struct _QDemonRenderBackendTessControlShaderObject *QDemonRenderBackendTessControlShaderObject;
    /// opaque tesselation evaluation shader object handle
    typedef struct _QDemonRenderBackendTessEvaluationShaderObject *QDemonRenderBackendTessEvaluationShaderObject;
    /// opaque geometry shader object handle
    typedef struct _QDemonRenderBackendGeometryShaderObject *QDemonRenderBackendGeometryShaderObject;
    /// opaque compute shader object handle
    typedef struct _QDemonRenderBackendComputeShaderObject *QDemonRenderBackendComputeShaderObject;
    /// opaque shader program object handle
    typedef struct _QDemonRenderBackendShaderProgramObject *QDemonRenderBackendShaderProgramObject;
    /// opaque depth stencil state object handle
    typedef struct _QDemonRenderBackendDepthStencilStateObject *QDemonRenderBackendDepthStencilStateObject;
    /// opaque rasterizer state object handle
    typedef struct _QDemonRenderBackendRasterizerStateObject *QDemonRenderBackendRasterizerStateObject;
    /// opaque query object handle
    typedef struct _QDemonRenderBackendQueryObject *QDemonRenderBackendQueryObject;
    /// opaque sync object handle
    typedef struct _QDemonRenderBackendSyncObject *QDemonRenderBackendSyncObject;
    /// opaque sync object handle
    typedef struct _QDemonRenderBackendProgramPipeline *QDemonRenderBackendProgramPipeline;
    /// opaque sync object handle
    typedef struct _QDemonRenderBackendPathObject *QDemonRenderBackendPathObject;

    // backend capability caps
    typedef struct
    {
        enum Enum {
            ConstantBuffer, ///< Constant buffer support query
            DepthStencilTexture, ///< depth stencil texture format suport query
            DxtImages, ///< DXT image support query
            FpRenderTarget, ///< render to floating point target support query
            MsTexture, ///< Multisample texture support query
            TexSwizzle, ///< Texture swizzle support query
            FastBlits, ///< Hardware supports fast blits
            Tessellation, ///< Hardware supports tessellation
            Compute, ///< Hardware supports compute shader
            Geometry, ///< Hardware supports geometry shader
            SampleQuery, ///< Hardware supports query calls of type samples
            TimerQuery, ///< Hardware supports query calls of type timer
            CommandSync, ///< Hardware supports command sync object
            TextureArray, ///< Hardware supports texture arrays
            StorageBuffer, ///< Hardware supports shader storage buffers
            AtomicCounterBuffer, ///< Hardware supports atomic counter buffers
            ShaderImageLoadStore, ///< Hardware supports shader image load store operations
            ProgramPipeline, ///< Driver supports separate programs
            PathRendering, ///< Driver support path rendering
            AdvancedBlend, ///< Driver supports advanced blend modes
            BlendCoherency, ///< Hardware supports blend coherency
            gpuShader5, // for high precision sampling
            AdvancedBlendKHR, ///< Driver supports advanced blend modes
            VertexArrayObject,
            StandardDerivatives,
            TextureLod
        };
    } QDemonRenderBackendCaps;

    // backend queries
    typedef struct
    {
        enum Enum {
            MaxTextureSize, ///< Return max supported texture size
            MaxTextureArrayLayers, ///< Return max supported layer count for texture arrays
            MaxConstantBufferSlots, ///< Return max supported constant buffe slots for a single
            /// shader stage
            MaxConstantBufferBlockSize ///< Return max supported size for a single constant
            /// buffer block
        };
    } QDemonRenderBackendQuery;

    /// backend interface

    /**
     * @brief get the backend type
     *
     * @return true backend type
     */
    virtual QDemonRenderContextType getRenderContextType() const = 0;

    /**
     * @brief get the version of the shading language
     * @return version string, must be copied by clients to be retained.
     */
    virtual const char *getShadingLanguageVersion() = 0;

    /**
     * @brief get maximum supported texture image units that
     *	can be used to access texture maps from the vertex shader and the fragment processor
     *combined.
     *
     * @return max texture size
     */
    virtual qint32 getMaxCombinedTextureUnits() = 0;

    /**
     * @brief query Backend capabilities
     *
     * @param[in] inCap		CAPS flag to query
     *						@ConstantBuffer, @DepthStencilTexture, ...
     *
     * @return true if supported
     */
    virtual bool getRenderBackendCap(QDemonRenderBackendCaps::Enum inCap) const = 0;

    /**
     * @brief query Backend values
     *
     * @param[in] inQuery		Query flag to get value  for
     *							@MaxTextureSize, @MaxTextureArrayLayers,
     *...
     * @param[in/out] params	the query result is stored here
     *
     * @return no return
     */
    virtual void getRenderBackendValue(QDemonRenderBackendQuery::Enum inQuery, qint32 *params) const = 0;

    /**
     * @brief query for bit depth of the depth buffer
     *
     * @return depth buffer bitplanes
     */
    virtual qint32 getDepthBits() const = 0;

    /**
     * @brief query for bit depth of the stencil buffer
     *
     * @return stencil buffer bitplanes
     */
    virtual qint32 getStencilBits() const = 0;

    /*
     * @brief set a backend rende state
     *
     * @param[in] bEnable	enable/disable state
     * @param[in] value		type of state
     *
     * @return no return
     */
    virtual void setRenderState(bool bEnable, const QDemonRenderState value) = 0;

    /**
     * @brief get a backend rende state
     *
     * @param[in] value		type of state
     *
     * @return  true if state enabled otherwise false
     */
    virtual bool getRenderState(const QDemonRenderState value) = 0;

    /**
     * @brief get current depth function
     *
     * @return  active depth function
     */
    virtual QDemonRenderBoolOp getDepthFunc() = 0;

    /**
     * @brief create a depth stencil state object
     *
     * @param[in] enableDepth			enable depth test
     * @param[in] depthMask				enable depth writes
     * @param[in] depthFunc				depth compare function
     * @param[in] enableStencil			enable stencil test
     * @param[in] stencilFuncFront		stencil setup front faces
     * @param[in] stencilFuncBack		stencil setup back faces
     * @param[in] depthStencilOpFront	depth/stencil operations front faces
     * @param[in] depthStencilOpBack	depth/stencil operations back faces
     *
     * @return  opaque handle to state object
     */
    virtual QDemonRenderBackendDepthStencilStateObject createDepthStencilState(
            bool enableDepth,
            bool depthMask,
            QDemonRenderBoolOp depthFunc,
            bool enableStencil,
            QDemonRenderStencilFunctionArgument &stencilFuncFront,
            QDemonRenderStencilFunctionArgument &stencilFuncBack,
            QDemonRenderStencilOperationArgument &depthStencilOpFront,
            QDemonRenderStencilOperationArgument &depthStencilOpBack) = 0;

    /**
     * @brief release a depth stencil state object
     *
     * @param[in] depthStencilState		pointer to state object
     *
     * @return  none
     */
    virtual void releaseDepthStencilState(QDemonRenderBackendDepthStencilStateObject depthStencilState) = 0;

    /**
     * @brief create a rasterizer state object
     *
     * @param[in] depthBias			any othe value than 0 enables depth bias
     * @param[in] depthScale		any othe value than 0 enables depth scale
     * @param[in] cullFace			select face to cull front or back
     *
     * @return  opaque handle to state object
     */
    virtual QDemonRenderBackendRasterizerStateObject createRasterizerState(float depthBias,
                                                                           float depthScale,
                                                                           QDemonRenderFace cullFace) = 0;

    /**
     * @brief release a rasterizer state object
     *
     * @param[in] rasterizerState		pointer to state object
     *
     * @return  none
     */
    virtual void releaseRasterizerState(QDemonRenderBackendRasterizerStateObject rasterizerState) = 0;

    /**
     * @brief set depth stencil state
     *
     * @param[in] depthStencilState		pointer to state object
     *
     * @return  none
     */
    virtual void setDepthStencilState(QDemonRenderBackendDepthStencilStateObject depthStencilState) = 0;

    /**
     * @brief set rasterizer state
     *
     * @param[in] rasterizerState		pointer to state object
     *
     * @return  none
     */
    virtual void setRasterizerState(QDemonRenderBackendRasterizerStateObject rasterizerState) = 0;

    /**
     * @brief set current depth function
     *
     * @param[in] func		type of function
     *
     * @return no return
     */
    virtual void setDepthFunc(const QDemonRenderBoolOp func) = 0;

    /**
     * @brief query if depth write is enabled
     *
     * @return true if enabled
     */
    virtual bool getDepthWrite() = 0;

    /**
     * @brief enable / disable depth writes
     *
     * @param[in] bEnable	true for enable
     *
     * @return no return
     */
    virtual void setDepthWrite(bool bEnable) = 0;

    /**
     * @brief enable / disable color channel writes
     *
     * @param[in] bRed		true for enable red channel
     * @param[in] bGreen	true for enable green channel
     * @param[in] bBlue		true for enable blue channel
     * @param[in] bAlpha	true for enable alpha channel
     *
     * @return no return
     */
    virtual void setColorWrites(bool bRed, bool bGreen, bool bBlue, bool bAlpha) = 0;

    /**
     * @brief enable / disable multisample rendering
     *
     * @param[in] bEnable	true for enable
     *
     * @return no return
     */
    virtual void setMultisample(bool bEnable) = 0;

    /**
     * @brief query blend functions
     *
     * @param[out] pBlendFuncArg	blending functions
     *
     * @return no return
     */
    virtual void getBlendFunc(QDemonRenderBlendFunctionArgument *pBlendFuncArg) = 0;

    /**
     * @brief set blend functions
     *
     * @param[in] pBlendFuncArg	blending functions
     *
     * @return no return
     */
    virtual void setBlendFunc(const QDemonRenderBlendFunctionArgument &blendFuncArg) = 0;

    /**
     * @brief set blend equation
     *
     * @param[in] pBlendEquArg	blending equation
     *
     * @return no return
     */
    virtual void setBlendEquation(const QDemonRenderBlendEquationArgument &pBlendEquArg) = 0;

    /**
     * @brief guarantee blend coherency
     *
     *
     * @return no return
     */
    virtual void setBlendBarrier(void) = 0;

    /**
     * @brief query scissor rectangle
     *
     * @param[out] pRect	contains scissor rect
     *
     * @return no return
     */
    virtual void getScissorRect(QRect *pRect) = 0;

    /**
     * @brief set scissor rectangle
     *
     * @param[out] pRect	contains scissor rect
     *
     * @return no return
     */
    virtual void setScissorRect(const QRect &rect) = 0;

    /**
     * @brief query viewport rectangle
     *
     * @param[out] pRect	contains viewport rect
     *
     * @return no return
     */
    virtual void getViewportRect(QRect *pRect) = 0;

    /**
     * @brief set viewport rectangle
     *
     * @param[out] pRect	contains viewport rect
     *
     * @return no return
     */
    virtual void setViewportRect(const QRect &rect) = 0;

    /**
     * @brief query viewport rectangle
     *
     * @param[in] clearColor	clear color
     *
     * @return no return
     */
    virtual void setClearColor(const QVector4D *pClearColor) = 0;

    /**
     * @brief query viewport rectangle
     *
     * @param[in] flags	clear flags
     *
     * @return no return
     */
    virtual void clear(QDemonRenderClearFlags flags) = 0;

    /**
     * @brief create a buffer object
     *
     * @param[in] size			Size of the buffer
     * @param[in] bindFlags		Where to bind this buffer (e.g. vertex, index, ...)
     *							For OpenGL this should be a single
     *value
     * @param[in] usage			Usage of the buffer (e.g. static, dynamic...)
     * @param[in] hostPtr       A pointer to the buffer data that is allocated by the
     *application.
     *
     * @return The created buffer object or nullptr if the creation failed.
     */
    virtual QDemonRenderBackendBufferObject createBuffer(size_t size,
                                                         QDemonRenderBufferBindFlags bindFlags,
                                                         QDemonRenderBufferUsageType usage,
                                                         const void *hostPtr = nullptr) = 0;

    /**
     * @brief bind a buffer object
     *
     * @param[in] bo			Pointer to buffer object
     * @param[in] bindFlags		Where to bind this buffer (e.g. vertex, index, ...)
     *							For OpenGL this should be a single
     *value
     *
     * @return no return.
     */
    virtual void bindBuffer(QDemonRenderBackendBufferObject bo, QDemonRenderBufferBindFlags bindFlags) = 0;

    /**
     * @brief Release a single buffer object
     *
     * @param[in] bo			Pointer to buffer object
     *
     * @return no return.
     */
    virtual void releaseBuffer(QDemonRenderBackendBufferObject bo) = 0;

    /**
     * @brief update a whole buffer object
     *
     * @param[in] bo			Pointer to buffer object
     * @param[in] bindFlags		Where to bind this buffer (e.g. vertex, index, ...)
     *							For OpenGL this should be a single
     *value
     * @param[in] size			Size of the data buffer
     * @param[in] usage			Usage of the buffer (e.g. static, dynamic...)
     * @param[in] data			A pointer to the buffer data that is allocated by the
     *application.
     *
     * @return no return.
     */
    virtual void updateBuffer(QDemonRenderBackendBufferObject bo,
                              QDemonRenderBufferBindFlags bindFlags,
                              size_t size,
                              QDemonRenderBufferUsageType usage,
                              const void *data) = 0;

    /**
     * @brief update a range of a buffer object
     *
     * @param[in] bo			Pointer to buffer object
     * @param[in] bindFlags		Where to bind this buffer (e.g. vertex, index, ...)
     *							For OpenGL this should be a single
     *value
     * @param[in] size			Size of the data buffer
     * @param[in] usage			Usage of the buffer (e.g. static, dynamic...)
     * @param[in] data			A pointer to the buffer data that is allocated by the
     *application.
     *
     * @return no return.
     */
    virtual void updateBufferRange(QDemonRenderBackendBufferObject bo,
                                   QDemonRenderBufferBindFlags bindFlags,
                                   size_t offset,
                                   size_t size,
                                   const void *data) = 0;

    /**
     * @brief Get a pointer to the buffer data ( GL(ES) >= 3 only )
     *
     * @param[in] bo			Pointer to buffer object
     * @param[in] bindFlags		Where to bind this buffer (e.g. vertex, index, ...)
     *							For OpenGL this should be a single
     *value
     * @param[in] offset		Byte offset into the data buffer
     * @param[in] length		Byte length of mapping size
     * @param[in] access		Access of the buffer (e.g. read, write, ...)
     *
     * @return pointer to mapped data or null.
     */
    virtual void *mapBuffer(QDemonRenderBackendBufferObject bo,
                            QDemonRenderBufferBindFlags bindFlags,
                            size_t offset,
                            size_t length,
                            QDemonRenderBufferAccessFlags accessFlags) = 0;

    /**
     * @brief Unmap a previously mapped buffer ( GL(ES) >= 3 only )
     *		  This functions transfers the content to the hardware buffer
     *
     * @param[in] bo			Pointer to buffer object
     * @param[in] bindFlags		Where to bind this buffer (e.g. vertex, index, ...)
     *							For OpenGL this should be a single
     *value
     *
     * @return true if successful
     */
    virtual bool unmapBuffer(QDemonRenderBackendBufferObject bo, QDemonRenderBufferBindFlags bindFlags) = 0;

    /**
     * @brief Set a memory barrier
     *
     * @param[in] barriers		Flags for barriers
     *
     * @return no return.
     */
    virtual void setMemoryBarrier(QDemonRenderBufferBarrierFlags barriers) = 0;

    /**
     * @brief create a query object
     *
     * @return The created query object or nullptr if the creation failed.
     */
    virtual QDemonRenderBackendQueryObject createQuery() = 0;

    /**
     * @brief delete query objects
     *
     * @param[in] qo		Handle to query object
     *
     * @return  no return
     */
    virtual void releaseQuery(QDemonRenderBackendQueryObject qo) = 0;

    /**
     * @brief Start query recording
     *
     * @param[in] qo		Handle to query object
     * @param[in] type		Type of query
     *
     * @return  no return
     */
    virtual void beginQuery(QDemonRenderBackendQueryObject qo, QDemonRenderQueryType type) = 0;

    /**
     * @brief End query recording
     *
     * @param[in] qo		Handle to query object
     * @param[in] type		Type of query
     *
     * @return  no return
     */
    virtual void endQuery(QDemonRenderBackendQueryObject qo, QDemonRenderQueryType type) = 0;

    /**
     * @brief Get a query result
     *
     * @param[in]  qo		Handle to query object
     * @param[in]  type		Type of query
     * @param[out] params	Contains result of query regarding query type
     *
     * @return  no return
     */
    virtual void getQueryResult(QDemonRenderBackendQueryObject qo, QDemonRenderQueryResultType resultType, quint32 *params) = 0;

    /**
     * @brief Get a query result
     *
     * @param[in]  qo		Handle to query object
     * @param[in]  type		Type of query
     * @param[out] params	Contains result of query regarding query type 64 bit returns
     *
     * @return  no return
     */
    virtual void getQueryResult(QDemonRenderBackendQueryObject qo, QDemonRenderQueryResultType resultType, quint64 *params) = 0;

    /**
     * @brief Record the GPU time using the query object
     *
     * @param[in]  qo		Handle to query object
     *
     * @return  no return
     */
    virtual void setQueryTimer(QDemonRenderBackendQueryObject qo) = 0;

    /**
     * @brief create a sync object and place it in the command queue
     *
     * @param[in] tpye			Type to sync
     * @param[in] syncFlags		Currently unused
     *
     * @return The created sync object or nullptr if the creation failed.
     */
    virtual QDemonRenderBackendSyncObject createSync(QDemonRenderSyncType tpye, QDemonRenderSyncFlags syncFlags) = 0;

    /**
     * @brief delete sync object
     *
     * @param[in] so		Handle to sync object
     *
     * @return  no return
     */
    virtual void releaseSync(QDemonRenderBackendSyncObject so) = 0;

    /**
     * @brief wait for sync object to be signaled
     *
     * @param[in] so			Handle to sync object
     * @param[in] syncFlags		Currently unused
     * @param[in] timeout		Currently ignored
     *
     * @return  no return
     */
    virtual void waitSync(QDemonRenderBackendSyncObject so, QDemonRenderCommandFlushFlags syncFlags, quint64 timeout) = 0;

    /**
     * @brief create a render target object
     *
     *
     * @return The created render target object or nullptr if the creation failed.
     */
    virtual QDemonRenderBackendRenderTargetObject createRenderTarget() = 0;

    /**
     * @brief Release a single render target object
     *
     * @param[in] rto			Pointer to render target object
     *
     * @return no return.
     */
    virtual void releaseRenderTarget(QDemonRenderBackendRenderTargetObject rto) = 0;

    /**
     * @brief Attach a renderbuffer object to the framebuffer
     *
     * @param[in] rto			Pointer to render target object
     * @param[in] attachment	Attachment point (e.g COLOR0, DEPTH)
     * @param[in] rbo			Pointer to renderbuffer object
     *
     * @return no return.
     */
    virtual void renderTargetAttach(QDemonRenderBackendRenderTargetObject rto,
                                    QDemonRenderFrameBufferAttachment attachment,
                                    QDemonRenderBackendRenderbufferObject rbo) = 0;

    /**
     * @brief Attach a texture object to the render target
     *
     * @param[in] rto			Pointer to render target object
     * @param[in] attachment	Attachment point (e.g COLOR0, DEPTH)
     * @param[in] to			Pointer to texture object
     * @param[in] target		Attachment texture target
     *
     * @return no return.
     */
    virtual void renderTargetAttach(QDemonRenderBackendRenderTargetObject rto,
                                    QDemonRenderFrameBufferAttachment attachment,
                                    QDemonRenderBackendTextureObject to,
                                    QDemonRenderTextureTargetType target = QDemonRenderTextureTargetType::Texture2D) = 0;

    /**
     * @brief Attach a texture object to the render target
     *
     * @param[in] rto			Pointer to render target object
     * @param[in] attachment	Attachment point (e.g COLOR0, DEPTH)
     * @param[in] to			Pointer to texture object
     * @param[in] level			Texture mip level
     * @param[in] layer			Texture layer or slice
     *
     * @return no return.
     */
    virtual void renderTargetAttach(QDemonRenderBackendRenderTargetObject rto,
                                    QDemonRenderFrameBufferAttachment attachment,
                                    QDemonRenderBackendTextureObject to,
                                    qint32 level,
                                    qint32 layer) = 0;

    /**
     * @brief Make a render target active
     *
     * @param[in] rto			Pointer to render target object
     *
     * @return no return.
     */
    virtual void setRenderTarget(QDemonRenderBackendRenderTargetObject rto) = 0;

    /**
     * @brief Check if a render target is ready for render
     *
     * @param[in] rto			Pointer to render target object
     *
     * @return true if usable.
     */
    virtual bool renderTargetIsValid(QDemonRenderBackendRenderTargetObject rto) = 0;

    /**
     * @brief Make a render target active for reading
     *
     * @param[in] rto			Pointer to render target object
     *
     * @return no return.
     */
    virtual void setReadTarget(QDemonRenderBackendRenderTargetObject rto) = 0;

    /**
     * @brief Set active buffers for drawing
     *
     * @param[in] rto				Pointer to render target object
     * @param[in] inDrawBufferSet	Pointer to array of enabled render targets
     *
     * @return no return.
     */
    virtual void setDrawBuffers(QDemonRenderBackendRenderTargetObject rto, QDemonConstDataRef<qint32> inDrawBufferSet) = 0;

    /**
     * @brief Set active buffer for reading
     *
     * @param[in] rto				Pointer to render target object
     * @param[in] inReadFace		Buffer to read from
     *
     * @return no return.
     */
    virtual void setReadBuffer(QDemonRenderBackendRenderTargetObject rto, QDemonReadFace inReadFace) = 0;

    /**
     * @brief Copy framebuffer attachments. Source is set with SetReadTarget dest with
     * SetRenderTarget
     *
     * @param[in] srcX0				Lower left X coord of source rectangle
     * @param[in] srcY0				Lower left Y coord of source rectangle
     * @param[in] srcX1				Upper right X coord of source rectangle
     * @param[in] srcY1				Upper right Y coord of source rectangle
     * @param[in] dstX0				Lower left X coord of dest rectangle
     * @param[in] dstY0				Lower left Y coord of dest rectangle
     * @param[in] dstX1				Upper right X coord of dest rectangle
     * @param[in] dstY1				Upper right Y coord of dest rectangle
     * @param[in] inDrawBufferSet	pointer to array of enabled render targets
     * @param[in] filter			Copy filter method (NEAREST or LINEAR)
     *
     * @return no return.
     */
    virtual void blitFramebuffer(qint32 srcX0,
                                 qint32 srcY0,
                                 qint32 srcX1,
                                 qint32 srcY1,
                                 qint32 dstX0,
                                 qint32 dstY0,
                                 qint32 dstX1,
                                 qint32 dstY1,
                                 QDemonRenderClearFlags flags,
                                 QDemonRenderTextureMagnifyingOp::Enum filter) = 0;

    /**
     * @brief create a render buffer object
     *
     * @param[in] storageFormat	Format of the buffer
     * @param[in] width			Buffer with
     * @param[in] height		Buffer height
     *
     * @return The created render buffer object or nullptr if the creation failed.
     */
    virtual QDemonRenderBackendRenderbufferObject createRenderbuffer(QDemonRenderRenderBufferFormat storageFormat,
                                                                     qint32 width,
                                                                     qint32 height) = 0;

    /**
     * @brief Release a single renderbuffer object
     *
     * @param[in] bo			Pointer to renderbuffer object
     *
     * @return no return.
     */
    virtual void releaseRenderbuffer(QDemonRenderBackendRenderbufferObject rbo) = 0;

    /**
     * @brief resize a render buffer object
     *
     * @param[in] storageFormat	Format of the buffer
     * @param[in] width			Buffer with
     * @param[in] height		Buffer height
     *
     * @return True on success
     */
    virtual bool resizeRenderbuffer(QDemonRenderBackendRenderbufferObject rbo,
                                    QDemonRenderRenderBufferFormat storageFormat,
                                    qint32 width,
                                    qint32 height) = 0;

    /**
     * @brief create a texture object
     *
     * @return The created texture object or nullptr if the creation failed..
     */
    virtual QDemonRenderBackendTextureObject createTexture() = 0;

    /**
     * @brief set texture data for a 2D texture
     *
     * @param[in] to				Pointer to texture object
     * @param[in] target			Texture target 2D, 3D
     * @param[in] level				Texture mip level
     * @param[in] internalFormat	format of the texture
     * @param[in] width				texture width
     * @param[in] height			texture height
     * @param[in] border			border
     * @param[in] format			format of provided pixel data
     * @param[in] hostPtr			A pointer to the buffer data that is allocated by the
     * application.
     *
     * @return No return
     */
    virtual void setTextureData2D(QDemonRenderBackendTextureObject to,
                                  QDemonRenderTextureTargetType target,
                                  qint32 level,
                                  QDemonRenderTextureFormat internalFormat,
                                  qint32 width,
                                  qint32 height,
                                  qint32 border,
                                  QDemonRenderTextureFormat format,
                                  const void *hostPtr = nullptr) = 0;

    /**
     * @brief set texture data for the face of a Cube map
     *
     * @param[in] to				Pointer to texture object
     * @param[in] target			Texture target face
     * @param[in] level				Texture mip level
     * @param[in] internalFormat	format of the texture
     * @param[in] width				texture width
     * @param[in] height			texture height
     * @param[in] border			border
     * @param[in] format			format of provided pixel data
     * @param[in] hostPtr			A pointer to the buffer data that is allocated by the
     * application.
     *
     * @return No return
     */
    virtual void setTextureDataCubeFace(QDemonRenderBackendTextureObject to,
                                        QDemonRenderTextureTargetType target,
                                        qint32 level,
                                        QDemonRenderTextureFormat internalFormat,
                                        qint32 width,
                                        qint32 height,
                                        qint32 border,
                                        QDemonRenderTextureFormat format,
                                        const void *hostPtr = nullptr) = 0;

    /**
     * @brief create a storage for a 2D texture including mip levels
     *		  Note that this makes texture immutable in size and format
     *
     * @param[in] to				Pointer to texture object
     * @param[in] target			Texture target 2D, 3D
     * @param[in] levels			Texture mip level count
     * @param[in] internalFormat	format of the texture
     * @param[in] width				texture width
     * @param[in] height			texture height
     *
     * @return No return
     */
    virtual void createTextureStorage2D(QDemonRenderBackendTextureObject to,
                                        QDemonRenderTextureTargetType target,
                                        qint32 levels,
                                        QDemonRenderTextureFormat internalFormat,
                                        qint32 width,
                                        qint32 height) = 0;

    /**
     * @brief set texture sub data for a 2D texture
     *
     * @param[in] to				Pointer to texture object
     * @param[in] target			Texture target 2D, 3D
     * @param[in] level				Texture mip level
     * @param[in] xOffset			Texture x offset
     * @param[in] yOffset			Texture y offset
     * @param[in] width				Texture width
     * @param[in] height			Texture height
     * @param[in] border			border
     * @param[in] format			format of texture
     * @param[in] hostPtr			A pointer to the buffer data that is allocated by the
     * application.
     *
     * @return No return
     */
    virtual void setTextureSubData2D(QDemonRenderBackendTextureObject to,
                                     QDemonRenderTextureTargetType target,
                                     qint32 level,
                                     qint32 xOffset,
                                     qint32 yOffset,
                                     qint32 width,
                                     qint32 height,
                                     QDemonRenderTextureFormat format,
                                     const void *hostPtr = nullptr) = 0;

    /**
     * @brief set compressed texture data for a 2D texture
     *
     * @param[in] to				Pointer to texture object
     * @param[in] target			Texture target 2D, 3D
     * @param[in] level				Texture mip level
     * @param[in] internalFormat	format of the texture
     * @param[in] width				texture width
     * @param[in] height			texture height
     * @param[in] border			border
     * @param[in] imageSize			image size in bytes located at hostPtr
     * @param[in] hostPtr			A pointer to the buffer data that is allocated by the
     * application.
     *
     * @return No return
     */
    virtual void setCompressedTextureData2D(QDemonRenderBackendTextureObject to,
                                            QDemonRenderTextureTargetType target,
                                            qint32 level,
                                            QDemonRenderTextureFormat internalFormat,
                                            qint32 width,
                                            qint32 height,
                                            qint32 border,
                                            qint32 imageSize,
                                            const void *hostPtr = nullptr) = 0;

    /**
     * @brief set compressed texture data for a Cubemap face
     *
     * @param[in] to				Pointer to texture object
     * @param[in] target			Texture target 2D, 3D
     * @param[in] level				Texture mip level
     * @param[in] internalFormat	format of the texture
     * @param[in] width				texture width
     * @param[in] height			texture height
     * @param[in] border			border
     * @param[in] imageSize			image size in bytes located at hostPtr
     * @param[in] hostPtr			A pointer to the buffer data that is allocated by the
     * application.
     *
     * @return No return
     */
    virtual void setCompressedTextureDataCubeFace(QDemonRenderBackendTextureObject to,
                                                  QDemonRenderTextureTargetType target,
                                                  qint32 level,
                                                  QDemonRenderTextureFormat internalFormat,
                                                  qint32 width,
                                                  qint32 height,
                                                  qint32 border,
                                                  qint32 imageSize,
                                                  const void *hostPtr = nullptr) = 0;

    /**
     * @brief set compressed texture sub data for a 2D texture
     *
     * @param[in] to				Pointer to texture object
     * @param[in] target			Texture target 2D, 3D
     * @param[in] level				Texture mip level
     * @param[in] xOffset			Texture x offset
     * @param[in] yOffset			Texture y offset
     * @param[in] width				texture width
     * @param[in] height			texture height
     * @param[in] format			format of provided pixel data
     * @param[in] imageSize			image size in bytes located at hostPtr
     * @param[in] hostPtr			A pointer to the buffer data that is allocated by the
     * application.
     *
     * @return No return
     */
    virtual void setCompressedTextureSubData2D(QDemonRenderBackendTextureObject to,
                                               QDemonRenderTextureTargetType target,
                                               qint32 level,
                                               qint32 xOffset,
                                               qint32 yOffset,
                                               qint32 width,
                                               qint32 height,
                                               QDemonRenderTextureFormat format,
                                               qint32 imageSize,
                                               const void *hostPtr = nullptr) = 0;

    /**
     * @brief establish a multisampled 2D texture
     *
     * @param[in] to				Pointer to texture object
     * @param[in] target			Texture target 2D MS
     * @param[in] samples			Textures sample count
     * @param[in] internalFormat	Format of the texture
     * @param[in] width				Texture width
     * @param[in] height			Texture height
     * @param[in] bool				Fixed sample locations
     *
     * @return No return
     */
    virtual void setMultisampledTextureData2D(QDemonRenderBackendTextureObject to,
                                              QDemonRenderTextureTargetType target,
                                              qint32 samples,
                                              QDemonRenderTextureFormat internalFormat,
                                              qint32 width,
                                              qint32 height,
                                              bool fixedsamplelocations) = 0;

    /**
     * @brief set texture data for a 3D texture or 2D texture array
     *
     * @param[in] to				Pointer to texture object
     * @param[in] target			Texture target 2D, 3D
     * @param[in] level				Texture mip level
     * @param[in] internalFormat	format of the texture
     * @param[in] width				texture width
     * @param[in] height			texture height
     * @param[in] depth				texture depth or slice count
     * @param[in] border			border
     * @param[in] format			format of provided pixel data
     * @param[in] hostPtr			A pointer to the buffer data that is allocated by the
     * application.
     *
     * @return No return
     */
    virtual void setTextureData3D(QDemonRenderBackendTextureObject to,
                                  QDemonRenderTextureTargetType target,
                                  qint32 level,
                                  QDemonRenderTextureFormat internalFormat,
                                  qint32 width,
                                  qint32 height,
                                  qint32 depth,
                                  qint32 border,
                                  QDemonRenderTextureFormat format,
                                  const void *hostPtr = nullptr) = 0;

    /**
     * @brief generate mipmap levels
     *
     * @param[in] to				Pointer to texture object
     * @param[in] target			Texture target 2D,...
     * @param[in] hint				How to generate mips (Nicest)
     *
     * @return No return
     */
    virtual void generateMipMaps(QDemonRenderBackendTextureObject to,
                                 QDemonRenderTextureTargetType target,
                                 QDemonRenderHint::Enum genType) = 0;

    /**
     * @brief bind a texture object
     *
     * @param[in] to			Pointer to texture object
     * @param[in] target		Where to bind this texture (e.g. 2D, 3D, ...)
     * @param[in] unit			Which unit to bind this texture
     *
     * @return no return.
     */
    virtual void bindTexture(QDemonRenderBackendTextureObject to,
                             QDemonRenderTextureTargetType target,
                             qint32 unit) = 0;

    /**
     * @brief bind a image/texture object
     *
     * @param[in] to			Pointer to texture object
     * @param[in] unit			Which unit to bind this texture
     * @param[in] level			Which level to bind
     * @param[in] layered		Bind layered texture (cube map, array,... )
     * @param[in] level			Specify layer. Only valid of layered=false.
     * @param[in] access		Access mode ( read, write, read-write )
     * @param[in] format		Texture format must be compatible with Image format
     *
     * @return no return.
     */
    virtual void bindImageTexture(QDemonRenderBackendTextureObject to,
                                  quint32 unit,
                                  qint32 level,
                                  bool layered,
                                  qint32 layer,
                                  QDemonRenderImageAccessType accessFlags,
                                  QDemonRenderTextureFormat format) = 0;

    /**
     * @brief Release a single texture object
     *
     * @param[in] to			Pointer to buffer object
     *
     * @return no return.
     */
    virtual void releaseTexture(QDemonRenderBackendTextureObject to) = 0;

    /**
     * @brief query texture swizzle mode
     *		  This is mainly for luminance, alpha replacement with R8 formats
     *
     * @param[in]  inFormat			input texture format to check
     *
     * @return texture swizzle mode
     */
    virtual QDemonRenderTextureSwizzleMode getTextureSwizzleMode(const QDemonRenderTextureFormat inFormat) const = 0;

    /**
     * @ brief create a sampler
     *
     * @param[in] minFilter		Texture min filter
     * @param[in] magFilter		Texture mag filter
     * @param[in] wrapS			Texture coord generation for S
     * @param[in] wrapT			Texture coord generation for T
     * @param[in] wrapR			Texture coord generation for R
     * @param[in] minLod		Texture min level of detail
     * @param[in] maxLod		Texture max level of detail
     * @param[in] lodBias		Texture level of detail example
     * @param[in] compareMode	Texture compare mode
     * @param[in] compareFunc	Texture compare function
     * @param[in] anisoFilter	Aniso filter value [1.0, 16.0]
     * @param[in] borderColor	Texture border color float[4]
     *
     * @return The created sampler object or nullptr if the creation failed.
     */
    virtual QDemonRenderBackendSamplerObject createSampler(
            QDemonRenderTextureMinifyingOp::Enum minFilter = QDemonRenderTextureMinifyingOp::Linear,
            QDemonRenderTextureMagnifyingOp::Enum magFilter = QDemonRenderTextureMagnifyingOp::Linear,
            QDemonRenderTextureCoordOp::Enum wrapS = QDemonRenderTextureCoordOp::ClampToEdge,
            QDemonRenderTextureCoordOp::Enum wrapT = QDemonRenderTextureCoordOp::ClampToEdge,
            QDemonRenderTextureCoordOp::Enum wrapR = QDemonRenderTextureCoordOp::ClampToEdge,
            qint32 minLod = -1000,
            qint32 maxLod = 1000,
            float lodBias = 0.0,
            QDemonRenderTextureCompareMode compareMode = QDemonRenderTextureCompareMode::NoCompare,
            QDemonRenderTextureCompareOp compareFunc = QDemonRenderTextureCompareOp::LessThanOrEqual,
            float anisotropy = 1.0,
            float *borderColor = nullptr) = 0;

    /**
     * @ brief update a sampler
     *
     * @param[in] so			Pointer to sampler object
     * @param[in] target		Texture target 2D, 3D
     * @param[in] minFilter		Texture min filter
     * @param[in] magFilter		Texture mag filter
     * @param[in] wrapS			Texture coord generation for S
     * @param[in] wrapT			Texture coord generation for T
     * @param[in] wrapR			Texture coord generation for R
     * @param[in] minLod		Texture min level of detail
     * @param[in] maxLod		Texture max level of detail
     * @param[in] lodBias		Texture level of detail bias (unused)
     * @param[in] compareMode	Texture compare mode
     * @param[in] compareFunc	Texture compare function
     * @param[in] anisoFilter	Aniso filter value [1.0, 16.0]
     * @param[in] borderColor	Texture border color float[4] (unused)
     *
     * @return No return
     */
    virtual void updateSampler(QDemonRenderBackendSamplerObject so,
                               QDemonRenderTextureTargetType target,
                               QDemonRenderTextureMinifyingOp::Enum minFilter = QDemonRenderTextureMinifyingOp::Linear,
                               QDemonRenderTextureMagnifyingOp::Enum magFilter = QDemonRenderTextureMagnifyingOp::Linear,
                               QDemonRenderTextureCoordOp::Enum wrapS = QDemonRenderTextureCoordOp::ClampToEdge,
                               QDemonRenderTextureCoordOp::Enum wrapT = QDemonRenderTextureCoordOp::ClampToEdge,
                               QDemonRenderTextureCoordOp::Enum wrapR = QDemonRenderTextureCoordOp::ClampToEdge,
                               float minLod = -1000.0,
                               float maxLod = 1000.0,
                               float lodBias = 0.0,
                               QDemonRenderTextureCompareMode compareMode = QDemonRenderTextureCompareMode::NoCompare,
                               QDemonRenderTextureCompareOp compareFunc = QDemonRenderTextureCompareOp::LessThanOrEqual,
                               float anisotropy = 1.0,
                               float *borderColor = nullptr) = 0;

    /**
     * @ brief Update a textures swizzle mode
     *
     * @param[in] so			Pointer to texture object
     * @param[in] target		Texture target 2D, 3D
     * @param[in] swizzleMode	Texture swizzle mode
     *
     * @return No return
     */
    virtual void updateTextureSwizzle(QDemonRenderBackendTextureObject to,
                                      QDemonRenderTextureTargetType target,
                                      QDemonRenderTextureSwizzleMode swizzleMode) = 0;

    /**
     * @ brief Update state belonging to a texture object
     *
     * @param[in] so			Pointer to texture object
     * @param[in] target		Texture target 2D, 3D, Cube
     * @param[in] baseLevel		Texture base level
     * @param[in] maxLevel		Texture max level
     *
     * @return No return
     */
    virtual void updateTextureObject(QDemonRenderBackendTextureObject to,
                                     QDemonRenderTextureTargetType target,
                                     qint32 baseLevel,
                                     qint32 maxLevel) = 0;

    /**
     * @brief Release a single sampler object
     *
     * @param[in] so			Pointer to sampler object
     *
     * @return no return.
     */
    virtual void releaseSampler(QDemonRenderBackendSamplerObject so) = 0;

    /**
     * @brief create a attribute layout object
     *
     * @param[in] attribs	Array off vertex attributes.
     *
     * @return The created attribute layout object or nullptr if the creation failed.
     */
    virtual QDemonRenderBackendAttribLayoutObject createAttribLayout(QDemonConstDataRef<QDemonRenderVertexBufferEntry> attribs) = 0;

    /**
     * @brief Release a attribute layoutr object
     *
     * @param[in] ao			Pointer to attribute layout object
     *
     * @return no return.
     */
    virtual void releaseAttribLayout(QDemonRenderBackendAttribLayoutObject ao) = 0;

    /**
     * @brief create a input assembler object
     *
     * @param[in] attribLayout		Pointer to QDemonRenderBackendAttribLayoutObject object
     * @param[in] buffers			list of vertex buffers
     * @param[in] indexBuffer		index buffer object
     * @param[in] strides			list of strides of the buffer
     * @param[in] offsets			list of offsets into the buffer
     * @param[in] patchVertexCount	vertext count for a patch. Only valid for patch primitives
     *
     * @return The created input assembler object or nullptr if the creation failed.
     */
    virtual QDemonRenderBackendInputAssemblerObject createInputAssembler(QDemonRenderBackendAttribLayoutObject attribLayout,
                                                                         QDemonConstDataRef<QDemonRenderBackendBufferObject> buffers,
                                                                         const QDemonRenderBackendBufferObject indexBuffer,
                                                                         QDemonConstDataRef<quint32> strides,
                                                                         QDemonConstDataRef<quint32> offsets,
                                                                         quint32 patchVertexCount) = 0;

    /**
     * @brief Release a input assembler object
     *
     * @param[in] iao					Pointer to attribute layout object
     *
     * @return no return.
     */
    virtual void releaseInputAssembler(QDemonRenderBackendInputAssemblerObject iao) = 0;

    /**
     * @brief Set a input assembler object.
     *		  This setup the render engine vertex assmebly
     *
     * @param[in] iao					Pointer to attribute layout object
     * @param[in] po					Pointer program object
     *
     * @return false if it fails.
     */
    virtual bool setInputAssembler(QDemonRenderBackendInputAssemblerObject iao, QDemonRenderBackendShaderProgramObject po) = 0;

    /**
     * @brief Set the per patch vertex count
     *
     * @param[in] iao					Pointer to attribute layout object
     * @param[in] count					Count of vertices per patch
     *
     * @return false if it fails.
     */
    virtual void setPatchVertexCount(QDemonRenderBackendInputAssemblerObject iao, quint32 count) = 0;

    /**
     * @brief create a vertex shader object
     *
     * @param[in] source			Pointer to shader source
     * @param[in/out] errorMessage	Pointer to copy the error message
     * @param[in] binary			True if the source is actually a binary program
     *
     * @return The created vertex shader object or nullptr if the creation failed.
     */
    virtual QDemonRenderBackendVertexShaderObject createVertexShader(QDemonConstDataRef<qint8> source,
                                                                     QByteArray &errorMessage,
                                                                     bool binary) = 0;

    /**
     * @brief release a vertex shader object
     *
     * @param[in] vso		Pointer to vertex shader object
     *
     * @return No Return.
     */
    virtual void releaseVertexShader(QDemonRenderBackendVertexShaderObject vso) = 0;

    /**
     * @brief create a fragment shader object
     *
     * @param[in] source			Pointer to shader source
     * @param[in/out] errorMessage	Pointer to copy the error message
     * @param[in] binary			True if the source is actually a binary program
     *
     * @return The created vertex shader object or nullptr if the creation failed.
     */
    virtual QDemonRenderBackendFragmentShaderObject createFragmentShader(QDemonConstDataRef<qint8> source,
                                                                         QByteArray &errorMessage,
                                                                         bool binary) = 0;

    /**
     * @brief release a fragment shader object
     *
     * @param[in] vso		Pointer to fragment shader object
     *
     * @return No Return.
     */
    virtual void releaseFragmentShader(QDemonRenderBackendFragmentShaderObject fso) = 0;

    /**
     * @brief create a tessellation control shader object
     *
     * @param[in] source			Pointer to shader source
     * @param[in/out] errorMessage	Pointer to copy the error message
     * @param[in] binary			True if the source is actually a binary program
     *
     * @return The created tessellation control shader object or nullptr if the creation failed.
     */
    virtual QDemonRenderBackendTessControlShaderObject createTessControlShader(QDemonConstDataRef<qint8> source,
                                                                               QByteArray &errorMessage,
                                                                               bool binary) = 0;

    /**
     * @brief release a tessellation control shader object
     *
     * @param[in] tcso		Pointer to tessellation control shader object
     *
     * @return No Return.
     */
    virtual void releaseTessControlShader(QDemonRenderBackendTessControlShaderObject tcso) = 0;

    /**
     * @brief create a tessellation evaluation shader object
     *
     * @param[in] source			Pointer to shader source
     * @param[in/out] errorMessage	Pointer to copy the error message
     * @param[in] binary			True if the source is actually a binary program
     *
     * @return The created tessellation evaluation shader object or nullptr if the creation failed.
     */
    virtual QDemonRenderBackendTessEvaluationShaderObject createTessEvaluationShader(QDemonConstDataRef<qint8> source,
                                                                                     QByteArray &errorMessage,
                                                                                     bool binary) = 0;

    /**
     * @brief release a tessellation evaluation shader object
     *
     * @param[in] tcso		Pointer to tessellation evaluation shader object
     *
     * @return No Return.
     */
    virtual void releaseTessEvaluationShader(QDemonRenderBackendTessEvaluationShaderObject teso) = 0;

    /**
     * @brief create a geometry shader object
     *
     * @param[in] source			Pointer to shader source
     * @param[in/out] errorMessage	Pointer to copy the error message
     * @param[in] binary			True if the source is actually a binary program
     *
     * @return The created geometry shader object or nullptr if the creation failed.
     */
    virtual QDemonRenderBackendGeometryShaderObject createGeometryShader(QDemonConstDataRef<qint8> source,
                                                                         QByteArray &errorMessage,
                                                                         bool binary) = 0;

    /**
     * @brief release a geometry shader object
     *
     * @param[in] tcso		Pointer to geometry shader object
     *
     * @return No Return.
     */
    virtual void releaseGeometryShader(QDemonRenderBackendGeometryShaderObject gso) = 0;

    /**
     * @brief create a compute shader object
     *
     * @param[in] source			Pointer to shader source
     * @param[in/out] errorMessage	Pointer to copy the error message
     * @param[in] binary			True if the source is actually a binary program
     *
     * @return The created compute shader object or nullptr if the creation failed.
     */
    virtual QDemonRenderBackendComputeShaderObject createComputeShader(QDemonConstDataRef<qint8> source,
                                                                       QByteArray &errorMessage,
                                                                       bool binary) = 0;

    /**
     * @brief release a compute shader object
     *
     * @param[in] cso		Pointer to compute shader object
     *
     * @return No Return.
     */
    virtual void releaseComputeShader(QDemonRenderBackendComputeShaderObject cso) = 0;

    /**
     * @brief attach a vertex shader object to a program object
     *
     * @param[in] po		Pointer to program object
     * @param[in] vso		Pointer to vertex shader object
     *
     * @return No Return.
     */
    virtual void attachShader(QDemonRenderBackendShaderProgramObject po, QDemonRenderBackendVertexShaderObject vso) = 0;

    /**
     * @brief detach a vertex shader object to a program object
     *
     * @param[in] po		Pointer to program object
     * @param[in] vso		Pointer to vertex shader object
     *
     * @return No Return.
     */
    virtual void detachShader(QDemonRenderBackendShaderProgramObject po, QDemonRenderBackendVertexShaderObject vso) = 0;

    /**
     * @brief attach a fragment shader object to a program object
     *
     * @param[in] po		Pointer to program object
     * @param[in] fso		Pointer to fragment shader object
     *
     * @return No Return.
     */
    virtual void attachShader(QDemonRenderBackendShaderProgramObject po, QDemonRenderBackendFragmentShaderObject fso) = 0;

    /**
     * @brief detach a fragment shader object to a program object
     *
     * @param[in] po		Pointer to program object
     * @param[in] fso		Pointer to fragment shader object
     *
     * @return No Return.
     */
    virtual void detachShader(QDemonRenderBackendShaderProgramObject po, QDemonRenderBackendFragmentShaderObject fso) = 0;

    /**
     * @brief attach a tessellation control shader object to a program object
     *
     * @param[in] po		Pointer to program object
     * @param[in] tcso		Pointer to tessellation control shader object
     *
     * @return No Return.
     */
    virtual void attachShader(QDemonRenderBackendShaderProgramObject po, QDemonRenderBackendTessControlShaderObject tcso) = 0;

    /**
     * @brief detach a tessellation control shader object to a program object
     *
     * @param[in] po		Pointer to program object
     * @param[in] tcso		Pointer to tessellation control shader object
     *
     * @return No Return.
     */
    virtual void detachShader(QDemonRenderBackendShaderProgramObject po, QDemonRenderBackendTessControlShaderObject tcso) = 0;

    /**
     * @brief attach a tessellation evaluation shader object to a program object
     *
     * @param[in] po		Pointer to program object
     * @param[in] teso		Pointer to tessellation evaluation shader object
     *
     * @return No Return.
     */
    virtual void attachShader(QDemonRenderBackendShaderProgramObject po, QDemonRenderBackendTessEvaluationShaderObject teso) = 0;

    /**
     * @brief detach a tessellation evaluation shader object to a program object
     *
     * @param[in] po		Pointer to program object
     * @param[in] teso		Pointer to tessellation evaluation shader object
     *
     * @return No Return.
     */
    virtual void detachShader(QDemonRenderBackendShaderProgramObject po, QDemonRenderBackendTessEvaluationShaderObject teso) = 0;

    /**
     * @brief attach a geometry shader object to a program object
     *
     * @param[in] po		Pointer to program object
     * @param[in] teso		Pointer to geometry shader object
     *
     * @return No Return.
     */
    virtual void attachShader(QDemonRenderBackendShaderProgramObject po, QDemonRenderBackendGeometryShaderObject gso) = 0;

    /**
     * @brief detach a geometry shader object to a program object
     *
     * @param[in] po		Pointer to program object
     * @param[in] teso		Pointer to geometry shader object
     *
     * @return No Return.
     */
    virtual void detachShader(QDemonRenderBackendShaderProgramObject po, QDemonRenderBackendGeometryShaderObject gso) = 0;

    /**
     * @brief attach a compute shader object to a program object
     *
     * @param[in] po		Pointer to program object
     * @param[in] cso		Pointer to compute shader object
     *
     * @return No Return.
     */
    virtual void attachShader(QDemonRenderBackendShaderProgramObject po, QDemonRenderBackendComputeShaderObject cso) = 0;

    /**
     * @brief detach a compute shader object to a program object
     *
     * @param[in] po		Pointer to program object
     * @param[in] cso		Pointer to compute shader object
     *
     * @return No Return.
     */
    virtual void detachShader(QDemonRenderBackendShaderProgramObject po, QDemonRenderBackendComputeShaderObject cso) = 0;

    /**
     * @brief create a shader program object
     *
     * @param[in] isSeparable	Tell the backend that this program is separable
     *
     * @return The created shader program object or nullptr if the creation failed.
     */
    virtual QDemonRenderBackendShaderProgramObject createShaderProgram(bool isSeparable) = 0;

    /**
     * @brief release a shader program object
     *
     * @param[in] po		Pointer to shader program object
     *
     * @return No Return.
     */
    virtual void releaseShaderProgram(QDemonRenderBackendShaderProgramObject po) = 0;

    /**
     * @brief link a shader program object
     *
     * @param[in] po				Pointer to shader program object
     * @param[in/out] errorMessage	Pointer to copy the error message
     *
     * @return True if program is succesful linked.
     */
    virtual bool linkProgram(QDemonRenderBackendShaderProgramObject po, QByteArray &errorMessage) = 0;

    /**
     * @brief Make a program current
     *
     * @param[in] po				Pointer to shader program object
     *
     * @return No return
     */
    virtual void setActiveProgram(QDemonRenderBackendShaderProgramObject po) = 0;

    /**
     * @brief create a program pipeline object
     *
     *
     * @return The created program pipeline object or nullptr if the creation failed.
     */
    virtual QDemonRenderBackendProgramPipeline createProgramPipeline() = 0;

    /**
     * @brief release a program pipeline object
     *
     * @param[in] ppo		Pointer to program pipeline object
     *
     * @return No Return.
     */
    virtual void releaseProgramPipeline(QDemonRenderBackendProgramPipeline ppo) = 0;

    /**
     * @brief Make a program pipeline current
     *
     * @param[in] ppo		Pointer to program pipeline object
     *
     * @return No return
     */
    virtual void setActiveProgramPipeline(QDemonRenderBackendProgramPipeline ppo) = 0;

    /**
     * @brief Make a program stage active for this pipeline
     *
     * @param[in] ppo		Pointer to program pipeline object
     * @param[in] flags		Shader stage flags to which this po is bound to
     * @param[in] po		Pointer to shader program object
     *
     * @return No return
     */
    virtual void setProgramStages(QDemonRenderBackendProgramPipeline ppo,
                                  QDemonRenderShaderTypeFlags flags,
                                  QDemonRenderBackendShaderProgramObject po) = 0;

    /**
     * @brief Runs a compute program
     *
     * @param[in] po				Pointer to shader program object
     * @param[in] numGroupsX		The number of work groups to be launched in the X
     * dimension
     * @param[in] numGroupsY		The number of work groups to be launched in the Y
     * dimension
     * @param[in] numGroupsZ		The number of work groups to be launched in the Z
     * dimension
     *
     * @return No return
     */
    virtual void dispatchCompute(QDemonRenderBackendShaderProgramObject po, quint32 numGroupsX, quint32 numGroupsY, quint32 numGroupsZ) = 0;

    /**
     * @brief Query constant count for a program object
     *
     * @param[in] po				Pointer to shader program object
     *
     * @return Return active constant count
     */
    virtual qint32 getConstantCount(QDemonRenderBackendShaderProgramObject po) = 0;

    /**
     * @brief Query constant buffer count for a program object
     *
     * @param[in] po				Pointer to shader program object
     *
     * @return Return active constant buffer count
     */
    virtual qint32 getConstantBufferCount(QDemonRenderBackendShaderProgramObject po) = 0;

    /**
     * @brief Query constant information by ID
     *
     * @param[in] po				Pointer to shader program object
     * @param[in] id				Constant ID
     * @param[in] bufSize			Max char for nameBuf
     * @param[out] numElem			Usually one unless for arrays
     * @param[out] type				Constant data type (QVector4D, QVector3D,...)
     * @param[out] binding			Unit binding point for samplers and images
     * @param[out] nameBuf			Name of the constant
     *
     * @return Return current constant location or -1 if not found
     */
    virtual qint32 getConstantInfoByID(QDemonRenderBackendShaderProgramObject po,
                                       quint32 id,
                                       quint32 bufSize,
                                       qint32 *numElem,
                                       QDemonRenderShaderDataType *type,
                                       qint32 *binding,
                                       char *nameBuf) = 0;

    /**
     * @brief Query constant buffer information by ID
     *
     * @param[in] po				Pointer to shader program object
     * @param[in] id				Constant buffer ID
     * @param[in] nameBufSize		Size of nameBuf
     * @param[out] paramCount		Count ot parameter contained in the buffer
     * @param[out] bufferSize		Data size of the constant buffer
     * @param[out] length			Actual characters written
     * @param[out] nameBuf			Receives the name of the buffer
     *
     * @return Return current constant buffer location or -1 if not found
     */
    virtual qint32 getConstantBufferInfoByID(QDemonRenderBackendShaderProgramObject po,
                                             quint32 id,
                                             quint32 nameBufSize,
                                             qint32 *paramCount,
                                             qint32 *bufferSize,
                                             qint32 *length,
                                             char *nameBuf) = 0;

    /**
     * @brief Query constant buffer param indices
     *
     * @param[in] po				Pointer to shader program object
     * @param[in] id				Constant buffer ID
     * @param[out] indices			Receives the indices of the uniforms within the
     * constant buffer
     *
     * @return no return value
     */
    virtual void getConstantBufferParamIndices(QDemonRenderBackendShaderProgramObject po, quint32 id, qint32 *indices) = 0;

    /**
     * @brief Query constant buffer param info by indices
     *
     * @param[in] po				Pointer to shader program object
     * @param[in] count				Number of indices
     * @param[in] indices			The indices of the uniforms within the constant
     * buffer
     * @param[out] type				Array of param types ( float ,int, ...)
     * @param[out] size				Array of param size
     * @param[out] offset			Array of param offsets within the constant buffer
     *
     * @return no return value
     */
    virtual void getConstantBufferParamInfoByIndices(QDemonRenderBackendShaderProgramObject po,
                                                     quint32 count,
                                                     quint32 *indices,
                                                     QDemonRenderShaderDataType *type,
                                                     qint32 *size,
                                                     qint32 *offset) = 0;

    /**
     * @brief Bind program constant block
     *
     * @param[in] po				Pointer to shader program object
     * @param[in] blockIndex		Constant block index returned by
     * GetConstantBufferInfoByID
     * @param[in] binding			Block binding location which should be the same as index
     * in ProgramSetConstantBlock
     *
     * @return No return
     */
    virtual void programSetConstantBlock(QDemonRenderBackendShaderProgramObject po, quint32 blockIndex, quint32 binding) = 0;

    /**
     * @brief Bind constant buffer for usage in the current active shader program
     *
     * @param[in] index				Constant ID
     * @param[in] bo				Pointer to constant buffer object
     *
     * @return No return
     */
    virtual void programSetConstantBuffer(quint32 index, QDemonRenderBackendBufferObject bo) = 0;

    /**
     * @brief Query storage buffer count for a program object
     *
     * @param[in] po				Pointer to shader program object
     *
     * @return Return active storage buffer count
     */
    virtual qint32 getStorageBufferCount(QDemonRenderBackendShaderProgramObject po) = 0;

    /**
     * @brief Query storage buffer information by ID
     *
     * @param[in] po				Pointer to shader program object
     * @param[in] id				Storage buffer ID
     * @param[in] nameBufSize		Size of nameBuf
     * @param[out] paramCount		Count of parameter contained in the buffer
     * @param[out] bufferSize		Data size of the constant buffer
     * @param[out] length			Actual characters written
     * @param[out] nameBuf			Receives the name of the buffer
     *
     * @return Return current storage buffer binding or -1 if not found
     */
    virtual qint32 getStorageBufferInfoByID(QDemonRenderBackendShaderProgramObject po,
                                            quint32 id,
                                            quint32 nameBufSize,
                                            qint32 *paramCount,
                                            qint32 *bufferSize,
                                            qint32 *length,
                                            char *nameBuf) = 0;

    /**
     * @brief Bind a storage buffer for usage in the current active shader program
     *
     * @param[in] index				Constant ID
     * @param[in] bo				Pointer to storage buffer object
     *
     * @return No return
     */
    virtual void programSetStorageBuffer(quint32 index, QDemonRenderBackendBufferObject bo) = 0;

    /**
     * @brief Query atomic counter buffer count for a program object
     *
     * @param[in] po				Pointer to shader program object
     *
     * @return Return active atomic buffer count
     */
    virtual qint32 getAtomicCounterBufferCount(QDemonRenderBackendShaderProgramObject po) = 0;

    /**
     * @brief Query atomic counter buffer information by ID
     *
     * @param[in] po				Pointer to shader program object
     * @param[in] id				Storage buffer ID
     * @param[in] nameBufSize		Size of nameBuf
     * @param[out] paramCount		Count of parameter contained in the buffer
     * @param[out] bufferSize		Data size of the constant buffer
     * @param[out] length			Actual characters written
     * @param[out] nameBuf			Receives the name of the buffer
     *
     * @return Return current storage buffer binding or -1 if not found
     */
    virtual qint32 getAtomicCounterBufferInfoByID(QDemonRenderBackendShaderProgramObject po,
                                                  quint32 id,
                                                  quint32 nameBufSize,
                                                  qint32 *paramCount,
                                                  qint32 *bufferSize,
                                                  qint32 *length,
                                                  char *nameBuf) = 0;

    /**
     * @brief Bind a atomic counter buffer for usage in the current active shader program
     *
     * @param[in] index				Constant ID
     * @param[in] bo				Pointer to atomic counter buffer object
     *
     * @return No return
     */
    virtual void programSetAtomicCounterBuffer(quint32 index, QDemonRenderBackendBufferObject bo) = 0;

    /**
     * @brief Set constant value
     *
     * @param[in] po				Pointer program object
     * @param[in] id				Constant ID
     * @param[in] type				Constant data type (QVector4D, QVector3D,...)
     * @param[in] count				Element count
     * @param[in] value				Pointer to constant value
     * @param[in] transpose			Transpose a matrix
     *
     * @return No return
     */
    virtual void setConstantValue(QDemonRenderBackendShaderProgramObject po,
                                  quint32 id,
                                  QDemonRenderShaderDataType type,
                                  qint32 count,
                                  const void *value,
                                  bool transpose = false) = 0;

    /**
     * @brief Draw the current active vertex buffer
     *
     * @param[in] drawMode	Draw mode (Triangles, ....)
     * @param[in] start		Start vertex
     * @param[in] count		Vertex count
     *
     * @return no return.
     */
    virtual void draw(QDemonRenderDrawMode drawMode, quint32 start, quint32 count) = 0;

    /**
     * @brief Draw the current active vertex buffer using an indirect buffer
     *		  This means the setup of the draw call is stored in a buffer bound to
     *		  QDemonRenderBufferBindValues::Draw_Indirect
     *
     * @param[in] drawMode	Draw mode (Triangles, ....)
     * @param[in] indirect	Offset into a indirect drawing setup buffer
     *
     * @return no return.
     */
    virtual void drawIndirect(QDemonRenderDrawMode drawMode, const void *indirect) = 0;

    /**
     * @brief Draw the current active index buffer
     *
     * @param[in] drawMode	Draw mode (Triangles, ....)
     * @param[in] count		Index count
     * @param[in] type		Index type (quint16, quint8)
     * @param[in] indices	Pointer to index buffer. In the case of buffer objects
     *						this is an offset into the active index buffer
     *object.
     *
     * @return no return.
     */
    virtual void drawIndexed(QDemonRenderDrawMode drawMode, quint32 count, QDemonRenderComponentType type, const void *indices) = 0;

    /**
     * @brief Draw the current active index buffer using an indirect buffer
     *		  This means the setup of the draw call is stored in a buffer bound to
     *		  QDemonRenderBufferBindValues::Draw_Indirect
     *
     * @param[in] drawMode	Draw mode (Triangles, ....)
     * @param[in] type		Index type (quint16, quint8)
     * @param[in] indices	Offset into a indirect drawing setup buffer
     *
     * @return no return.
     */
    virtual void drawIndexedIndirect(QDemonRenderDrawMode drawMode, QDemonRenderComponentType type, const void *indirect) = 0;

    /**
     * @brief Read a pixel rectangle from render target (from bottom left)
     *
     * @param[in]  rto		Pointer to render target object
     * @param[in]  x		Windows X start coord
     * @param[in]  y		Windows Y start coord
     * @param[in]  width	Read width dim
     * @param[in]  height	Read height dim
     * @param[out] pixels	Returned pixel data
     *
     * @return No return
     */
    virtual void readPixel(QDemonRenderBackendRenderTargetObject rto,
                           qint32 x,
                           qint32 y,
                           qint32 width,
                           qint32 height,
                           QDemonRenderReadPixelFormat inFormat,
                           void *pixels) = 0;

    /**
     * @brief Create a NV path render object
     *
     * @param[in] range		Number of internal objects
     *
     * @return return path object on success or nullptr
     */
    virtual QDemonRenderBackendPathObject createPathNVObject(size_t range) = 0;

    /**
     * @brief Relase a NV path render object
     *
     * @param[in] po		Created path object
     * @param[in] range		Number of internal objects
     *
     * @return return path object on success or nullptr
     */
    virtual void releasePathNVObject(QDemonRenderBackendPathObject po, size_t range) = 0;

    /**
     * @brief Set the path commands and data.
     *
     * @param[in]  inPathObject		Pointer to NV path object
     * @param[in]  inPathCommands	vector of path commands ( moveTo,... )
     * @param[in]  inPathCoords		vector of path coords
     *
     * @return No return
     */
    virtual void setPathSpecification(QDemonRenderBackendPathObject inPathObject,
                                      QDemonConstDataRef<quint8> inPathCommands,
                                      QDemonConstDataRef<float> inPathCoords) = 0;

    /**
     * @brief Get Bounds of the path object
     *
     * @param[in]  inPathObject		Pointer to NV path object
     *
     * @return return bounds
     */
    virtual QDemonBounds3 getPathObjectBoundingBox(QDemonRenderBackendPathObject inPathObject) = 0;
    virtual QDemonBounds3 getPathObjectFillBox(QDemonRenderBackendPathObject inPathObject) = 0;
    virtual QDemonBounds3 getPathObjectStrokeBox(QDemonRenderBackendPathObject inPathObject) = 0;

    /**
     * @brief Set stroke width. Defaults to 0 if unset.
     *
     * @param[in]  inPathObject		Pointer to NV path object
     *
     * @return No return
     */
    virtual void setStrokeWidth(QDemonRenderBackendPathObject inPathObject, float inStrokeWidth) = 0;

    /**
     * @brief Path transform commands
     *
     * @param[in]  inPathObject		Pointer to NV path object
     *
     * @return No return
     */
    virtual void setPathProjectionMatrix(const QMatrix4x4 inPathProjection) = 0;
    virtual void setPathModelViewMatrix(const QMatrix4x4 inPathModelview) = 0;

    /**
     * @brief Path stencil pass operations
     *
     * @param[in]  inPathObject		Pointer to NV path object
     *
     * @return No return
     */
    virtual void stencilStrokePath(QDemonRenderBackendPathObject inPathObject) = 0;
    virtual void stencilFillPath(QDemonRenderBackendPathObject inPathObject) = 0;

    /**
     * @brief Does a instanced stencil fill pass
     *
     * @param[in]  po						Base of path objects
     * @param[in]  numPaths					Number path objects
     * @param[in]  type						type ( byte, int,... )
     * @param[in]  charCodes				charachter string
     * @param[in]  fillMode					Fille mode
     * @param[in]  stencilMask				Stencil mask
     * @param[in]  transformType			Transforming glyphs
     * @param[in]  transformValues			Pointer to array of transforms
     *
     * @return No return
     */
    virtual void stencilFillPathInstanced(QDemonRenderBackendPathObject po,
                                          size_t numPaths,
                                          QDemonRenderPathFormatType type,
                                          const void *charCodes,
                                          QDemonRenderPathFillMode fillMode,
                                          quint32 stencilMask,
                                          QDemonRenderPathTransformType transformType,
                                          const float *transformValues) = 0;

    /**
     * @brief Does a instanced stencil stroke pass
     *
     * @param[in]  po						Base of path objects
     * @param[in]  numPaths					Number path objects
     * @param[in]  type						type ( byte, int,... )
     * @param[in]  charCodes				charachter string
     * @param[in]  stencilRef				Stencil reference
     * @param[in]  stencilMask				Stencil mask
     * @param[in]  transformType			Transforming glyphs
     * @param[in]  transformValues			Pointer to array of transforms
     *
     * @return No return
     */
    virtual void stencilStrokePathInstancedN(QDemonRenderBackendPathObject po,
                                             size_t numPaths,
                                             QDemonRenderPathFormatType type,
                                             const void *charCodes,
                                             qint32 stencilRef,
                                             quint32 stencilMask,
                                             QDemonRenderPathTransformType transformType,
                                             const float *transformValues) = 0;

    /**
     * @brief Does a instanced cover fill pass
     *
     * @param[in]  po						Base of path objects
     * @param[in]  numPaths					Number path objects
     * @param[in]  type						type ( byte, int,... )
     * @param[in]  charCodes				charachter string
     * @param[in]  coverMode				Cover mode
     * @param[in]  transformType			Transforming glyphs
     * @param[in]  transformValues			Pointer to array of transforms
     *
     * @return No return
     */
    virtual void coverFillPathInstanced(QDemonRenderBackendPathObject po,
                                        size_t numPaths,
                                        QDemonRenderPathFormatType type,
                                        const void *charCodes,
                                        QDemonRenderPathCoverMode coverMode,
                                        QDemonRenderPathTransformType transformType,
                                        const float *transformValues) = 0;

    /**
     * @brief Does a instanced cover stroke pass
     *
     * @param[in]  po						Base of path objects
     * @param[in]  numPaths					Number path objects
     * @param[in]  type						type ( byte, int,... )
     * @param[in]  charCodes				charachter string
     * @param[in]  coverMode				Cover mode
     * @param[in]  transformType			Transforming glyphs
     * @param[in]  transformValues			Pointer to array of transforms
     *
     * @return No return
     */
    virtual void coverStrokePathInstanced(QDemonRenderBackendPathObject po,
                                          size_t numPaths,
                                          QDemonRenderPathFormatType type,
                                          const void *charCodes,
                                          QDemonRenderPathCoverMode coverMode,
                                          QDemonRenderPathTransformType transformType,
                                          const float *transformValues) = 0;

    /**
     * @brief Path stencil and depth offset
     *
     * @param[in]  inSlope		slope
     * @param[in]  inBias		bias
     *
     * @return No return
     */
    virtual void setPathStencilDepthOffset(float inSlope, float inBias) = 0;

    /**
     * @brief Path cover function
     *
     * @param[in]  inDepthFunction		depth function
     *
     * @return No return
     */
    virtual void setPathCoverDepthFunc(QDemonRenderBoolOp inDepthFunction) = 0;

    /**
     * @brief Load glyphs
     *
     * @param[in]  po						Base of path objects
     * @param[in]  fontTarget				System font, or application font,...
     * @param[in]  fontName					Name of font ( may include path )
     * @param[in]  fontStyle				Bold or italic
     * @param[in]  numGlyphs				Glyph count
     * @param[in]  type						type ( byte, int,... )
     * @param[in]  charCodes				charachter string
     * @param[in]  handleMissingGlyphs		skip or use
     * @param[in]  pathParameterTemplate	template
     * @param[in]  emScale					scale ( true type scale e.g. 2048 )
     *
     * @return No return
     */
    virtual void loadPathGlyphs(QDemonRenderBackendPathObject po,
                                QDemonRenderPathFontTarget fontTarget,
                                const void *fontName,
                                QDemonRenderPathFontStyleFlags fontStyle,
                                size_t numGlyphs,
                                QDemonRenderPathFormatType type,
                                const void *charCodes,
                                QDemonRenderPathMissingGlyphs handleMissingGlyphs,
                                QDemonRenderBackendPathObject pathParameterTemplate,
                                float emScale) = 0;

    /**
     * @brief Load indexed font set
     *
     * @param[in]  po						Base of path objects
     * @param[in]  fontTarget				System font, or application font,...
     * @param[in]  fontName					Name of font ( may include path )
     * @param[in]  fontStyle				Bold or italic
     * @param[in]  firstGlyphIndex			First glyph
     * @param[in]  numGlyphs				Glyph count
     * @param[in]  pathParameterTemplate	template
     * @param[in]  emScale					scale ( true type scale e.g. 2048 )
     *
     * @return return load status
     */
    virtual QDemonRenderPathReturnValues loadPathGlyphsIndexed(QDemonRenderBackendPathObject po,
                                                                     QDemonRenderPathFontTarget fontTarget,
                                                                     const void *fontName,
                                                                     QDemonRenderPathFontStyleFlags fontStyle,
                                                                     quint32 firstGlyphIndex,
                                                                     size_t numGlyphs,
                                                                     QDemonRenderBackendPathObject pathParameterTemplate,
                                                                     float emScale) = 0;

    /**
     * @brief Load indexed font set
     *
     * @param[in]	fontTarget				System font, or application font,...
     * @param[in]	fontName				Name of font ( may include path )
     * @param[in]	fontStyle				Bold or italic
     * @param[in]	pathParameterTemplate	template
     * @param[in]	emScale					scale ( true type scale e.g. 2048 )
     * @param[out]	po						contains glyph count
     *
     * @return returnr base path object
     */
    virtual QDemonRenderBackendPathObject loadPathGlyphsIndexedRange(QDemonRenderPathFontTarget fontTarget,
                                                                     const void *fontName,
                                                                     QDemonRenderPathFontStyleFlags fontStyle,
                                                                     QDemonRenderBackendPathObject pathParameterTemplate,
                                                                     float emScale,
                                                                     quint32 *count) = 0;

    /**
     * @brief Load font set
     *
     * @param[in]  po						Base of path objects
     * @param[in]  fontTarget				System font, or application font,...
     * @param[in]  fontName					Name of font ( may include path )
     * @param[in]  fontStyle				Bold or italic
     * @param[in]  firstGlyph				First glyph
     * @param[in]  numGlyphs				Glyph count
     * @param[in]  handleMissingGlyphs		skip or use
     * @param[in]  pathParameterTemplate	template
     * @param[in]  emScale					scale ( true type scale e.g. 2048 )
     *
     * @return No return
     */
    virtual void loadPathGlyphRange(QDemonRenderBackendPathObject po,
                                    QDemonRenderPathFontTarget fontTarget,
                                    const void *fontName,
                                    QDemonRenderPathFontStyleFlags fontStyle,
                                    quint32 firstGlyph,
                                    size_t numGlyphs,
                                    QDemonRenderPathMissingGlyphs handleMissingGlyphs,
                                    QDemonRenderBackendPathObject pathParameterTemplate,
                                    float emScale) = 0;

    /**
     * @brief Query font metrics
     *
     * @param[in]  po						Base of path objects
     * @param[in]  numPaths					Number path objects
     * @param[in]  metricQueryMask			Qeury bit mask
     * @param[in]  type						type ( byte, int,... )
     * @param[in]  charCodes				charachter string
     * @param[in]  stride					scale ( true type scale e.g. 2048 )
     * @param[out] metrics					Filled with font metric values
     *
     * @return No return
     */
    virtual void getPathMetrics(QDemonRenderBackendPathObject po,
                                size_t numPaths,
                                QDemonRenderPathGlyphFontMetricFlags metricQueryMask,
                                QDemonRenderPathFormatType type,
                                const void *charCodes,
                                size_t stride,
                                float *metrics) = 0;

    /**
     * @brief Query font metrics
     *
     * @param[in]  po						Base of path objects
     * @param[in]  numPaths					Number path objects
     * @param[in]  metricQueryMask			Qeury bit mask
     * @param[in]  stride					scale ( true type scale e.g. 2048 )
     * @param[out] metrics					Filled with font metric values
     *
     * @return No return
     */
    virtual void getPathMetricsRange(QDemonRenderBackendPathObject po,
                                     size_t numPaths,
                                     QDemonRenderPathGlyphFontMetricFlags metricQueryMask,
                                     size_t stride,
                                     float *metrics) = 0;

    /**
     * @brief Query path spacing
     *
     * @param[in]  po						Base of path objects
     * @param[in]  numPaths					Number path objects
     * @param[in]  pathListMode				How to compute spacing
     * @param[in]  type						type ( byte, int,... )
     * @param[in]  charCodes				charachter string
     * @param[in]  advanceScale
     * @param[in]  kerningScale
     * @param[in]  transformType
     * @param[out] metrics					Filled with font metric values
     *
     * @return No return
     */
    virtual void getPathSpacing(QDemonRenderBackendPathObject po,
                                size_t numPaths,
                                QDemonRenderPathListMode pathListMode,
                                QDemonRenderPathFormatType type,
                                const void *charCodes,
                                float advanceScale,
                                float kerningScale,
                                QDemonRenderPathTransformType transformType,
                                float *spacing) = 0;

    virtual QSurfaceFormat format() const = 0;

protected:
    /// struct for what the backend supports
    typedef struct QDemonRenderBackendSupport
    {
        union {
            struct
            {
                bool bDXTImagesSupported : 1; ///< compressed images supported
                bool bAnistropySupported : 1; ///< anistropic filtering supported
                bool bTextureSwizzleSupported : 1; ///< texture swizzle supported
                bool bDepthStencilSupported : 1; ///< depth stencil textures are supported
                bool bFPRenderTargetsSupported : 1; ///< floating point render targets are
                /// supported
                bool bConstantBufferSupported : 1; ///< Constant (uniform) buffers are supported
                bool bMsTextureSupported : 1; ///< Multisample textures are esupported
                bool bFastBlitsSupported : 1; ///< The hardware supports fast memor blits
                bool bTessellationSupported : 1; ///< Hardware supports tessellation
                bool bComputeSupported : 1; ///< Hardware supports compute shader
                bool bGeometrySupported : 1; ///< Hardware supports geometry shader
                bool bTimerQuerySupported : 1; ///< Hardware supports timer queries
                bool bProgramInterfaceSupported : 1; ///< API supports program interface queries
                bool bStorageBufferSupported : 1; ///< Shader storage buffers are supported
                bool bAtomicCounterBufferSupported : 1; ///< Atomic counter buffers are
                /// supported
                bool bShaderImageLoadStoreSupported : 1; ///< Shader image load / store
                /// operations are supported
                bool bProgramPipelineSupported : 1; ///< Driver supports separate programs
                bool bNVPathRenderingSupported : 1; ///< Driver NV path rendering
                bool bNVAdvancedBlendSupported : 1; ///< Advanced blend modes supported
                bool bNVBlendCoherenceSupported : 1; ///< Advanced blend done coherently
                /// supported
                bool bGPUShader5ExtensionSupported : 1;
                bool bKHRAdvancedBlendSupported : 1; ///< Advanced blend modes supported
                bool bKHRBlendCoherenceSupported : 1; ///< Advanced blend done coherently
                bool bVertexArrayObjectSupported : 1;
                bool bStandardDerivativesSupported : 1;
                bool bTextureLodSupported : 1;
            } bits;

            quint32 u32Values;
        } caps;
    } QDemonRenderBackendSupportBits;

    QDemonRenderBackendSupportBits m_backendSupport; ///< holds the backend support bits
};

QT_END_NAMESPACE

#endif
