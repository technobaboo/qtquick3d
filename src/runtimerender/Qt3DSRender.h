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
#pragma once
#ifndef QDEMON_RENDER_H
#define QDEMON_RENDER_H

namespace qt3ds {
class NVAllocatorCallback;
class NVFoundationBase;
namespace foundation {
    class CRegisteredString;
    class IStringTable;
    class CStrTableOrDataRef;
    struct SStrRemapMap;
    struct SWriteBuffer;
    struct SDataReader;
    struct SPtrOffsetMap;
    class IPerfTimer;
}
namespace intrinsics {
}
namespace render {
    class QDemonRenderTexture2D;
    class QDemonRenderTexture2DArray;
    class QDemonRenderTextureCube;
    class QDemonRenderImage2D;
    class QDemonRenderFrameBuffer;
    class QDemonRenderRenderBuffer;
    class QDemonRenderVertexBuffer;
    class QDemonRenderIndexBuffer;
    class QDemonRenderDrawIndirectBuffer;
    class QDemonRenderInputAssembler;
    class QDemonRenderAttribLayout;
    class QDemonRenderDepthStencilState;
    class QDemonRenderContext;
    class QDemonRenderConstantBuffer;
    class QDemonRenderShaderProgram;
    class QDemonRenderShaderConstantBase;
    struct QDemonRenderDrawMode;
    struct QDemonRenderWinding;
    struct QDemonRenderSrcBlendFunc;
    struct QDemonRenderBlendEquation;
    struct QDemonRenderState;
    struct QDemonRenderTextureCoordOp;
    struct QDemonRenderTextureMagnifyingOp;
    struct QDemonRenderDstBlendFunc;
    struct QDemonRenderContextValues;
    struct QDemonRenderClearValues;
    struct QDemonRenderRect;
    struct QDemonRenderRectF;
    struct QDemonRenderRenderBufferFormats;
    struct QDemonRenderTextureFormats;
    struct QDemonRenderTextureSwizzleMode;
    struct QDemonRenderFrameBufferAttachments;
    struct QDemonRenderRect;
    struct QDemonRenderTextureCubeFaces;
    struct STextureDetails;
    struct QDemonRenderShaderDataTypes;
    class QDemonRenderTextureOrRenderBuffer;
    struct QDemonRenderTextureMinifyingOp;
    struct QDemonReadFaces;
    struct QDemonRenderVertexBufferEntry;
    struct NVRenderPtrPtrMap;
    class QDemonRenderComputeShader;
    class QDemonRenderAttribLayout;
    struct QDemonRenderBufferAccessTypeValues;
    struct QDemonRenderImageAccessType;
    struct QDemonRenderBufferBindValues;
    struct DrawArraysIndirectCommand;
    struct QDemonRenderShaderTypeValue;
    class QDemonRenderPathRender;
    class QDemonRenderPathSpecification;
    class QDemonRenderPathFontSpecification;
    class QDemonRenderPathFontItem;
    struct QDemonRenderTextureTypeValue;
    class QDemonRenderProgramPipeline;
}
class NVPlane;
}

namespace eastl {
}

namespace qt3ds {

namespace render {
    using namespace qt3ds;
    using namespace foundation;
    using namespace intrinsics;
    using QDemonRenderTexture2D;
    using QDemonRenderTexture2DArray;
    using QDemonRenderTextureCube;
    using QDemonRenderImage2D;
    using QDemonRenderFrameBuffer;
    using QDemonRenderRenderBuffer;
    using QDemonRenderVertexBuffer;
    using QDemonRenderIndexBuffer;
    using QDemonRenderDrawIndirectBuffer;
    using QDemonRenderInputAssembler;
    using QDemonRenderAttribLayout;
    using QDemonRenderDepthStencilState;
    using QDemonRenderContext;
    using QDemonRenderConstantBuffer;
    using QDemonRenderShaderProgram;
    using QDemonRenderShaderConstantBase;
    using QDemonRenderDrawMode;
    using QDemonRenderWinding;
    using CRegisteredString;
    using IStringTable;
    using QDemonRenderSrcBlendFunc;
    using QDemonRenderBlendEquation;
    using QDemonRenderState;
    using IStringTable;
    using CRegisteredString;
    using QDemonRenderTextureCoordOp;
    using QDemonRenderDstBlendFunc;
    using QDemonRenderRect;
    using QDemonRenderRectF;
    using QDemonRenderRenderBufferFormats;
    using QDemonRenderTextureFormats;
    using QDemonRenderTextureSwizzleMode;
    using QDemonRenderFrameBufferAttachments;
    using QDemonRenderRect;
    using QDemonRenderContextValues;
    using QDemonRenderClearValues;
    using STextureDetails;
    using QDemonRenderShaderDataTypes;
    using QDemonRenderTextureMagnifyingOp;
    using QDemonRenderTextureOrRenderBuffer;
    using QDemonRenderTextureMinifyingOp;
    using QDemonReadFaces;
    using QDemonRenderTextureCubeFaces;
    using SStrRemapMap;
    using SWriteBuffer;
    using SDataReader;
    using NVRenderPtrPtrMap;
    using CStrTableOrDataRef;
    using SPtrOffsetMap;
    using IPerfTimer;
    using QDemonRenderVertexBufferEntry;
    using QDemonRenderComputeShader;
    using QDemonRenderAttribLayout;
    using QDemonRenderBufferAccessTypeValues;
    using QDemonRenderImageAccessType;
    using QDemonRenderBufferBindValues;
    using DrawArraysIndirectCommand;
    using QDemonRenderShaderTypeValue;
    using QDemonRenderPathRender;
    using QDemonRenderPathSpecification;
    using QDemonRenderPathFontSpecification;
    using QDemonRenderPathFontItem;
    using QDemonRenderTextureTypeValue;
    using QDemonRenderProgramPipeline;

    class IQt3DSRenderContextCore;
    class IQt3DSRenderContext;
    class IQt3DSRenderer;
    class IBufferManager;
    struct SRenderMesh;
    class IRenderableObject;
    class IQt3DSRenderer;
    class IBufferManager;
    class IResourceManager;
    class IOffscreenRenderManager;
    struct SNode;
    struct SGraphObject;
    class ITextRenderer;
    class ITextRendererCore;
    class IInputStreamFactory;
    class IRefCountedInputStream;
    class IEffectSystem;
    class IEffectSystemCore;
    class CRenderString;
    class IShaderCache;
    class IQt3DSRenderNodeFilter;
    class IRenderWidget;
    class IRenderWidgetContext;
    struct SShaderVertexCodeGenerator;
    struct SShaderFragmentCodeGenerator;
    class IThreadPool;
    struct SRenderMesh;
    struct SLoadedTexture;
    class IImageBatchLoader;
    class ITextTextureCache;
    class ITextTextureAtlas;
    class IRenderPluginInstance;
    class IRenderPluginClass;
    class IRenderPluginManager;
    class IRenderPluginManagerCore;
    struct SRenderPlugin;
    class IDynamicObjectSystemCore;
    class IDynamicObjectSystem;
    class IDynamicObjectClass;
    struct SRenderSubset;
    struct SModel;
    namespace dynamic {
        struct SPropertyDefinition;
    }
    struct SLight;
    struct SCamera;
    struct SCustomMaterial;
    class ICustomMaterialSystem;
    class ICustomMaterialSystemCore;
    struct SLayer;
    struct SReferencedMaterial;
    struct SPGGraphObject;
    class IPixelGraphicsRenderer;
    class IBufferLoader;
    struct SEffect;
    class IRenderList;
    class IRenderTask;
    class CResourceTexture2D;
    class IPathManagerCore;
    class IPathManager;
    struct SPath;
    struct SPathSubPath;
    class IShaderProgramGenerator;
    class IShaderStageGenerator;
    class IDefaultMaterialShaderGenerator;
    class ICustomMaterialShaderGenerator;
    struct SRenderableImage;
    class Qt3DSShadowMap;
    struct SLightmaps;
}
}

#endif
