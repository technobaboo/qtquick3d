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
#ifndef QDEMON_OBJECT_RENDER_PLUGIN_H
#define QDEMON_OBJECT_RENDER_PLUGIN_H

#include <QtDemonRuntimeRender/qtdemonruntimerenderglobal.h>
#include <QtCore/qbytearray.h>

/*
 *	Below are the definitions required in order to write a render plugin for UIComposer.
 *	Please note that calling anything related to opengl is explicitly not allowed except
 *	during either the class gl resource initialization function or during render.  Calling into
 *	openGL and especially changing GL state during any other function may produce corrupt
 *rendering.
 */

QT_BEGIN_NAMESPACE

#ifdef _cplusplus
#extern "C" {
#endif

enum QDemonRenderPluginPropertyTypes {
    QDemonRenderPluginPropertyTypeNone = 0,
    QDemonRenderPluginPropertyTypeLong = 1,
    QDemonRenderPluginPropertyTypeFloat = 2,
    QDemonRenderPluginPropertyTypeCharPtr = 3,
};

enum QDemonRenderPluginDepthTypes {
    QDemonRenderPluginDepthTypeNoDepthBuffer = 0,
    QDemonRenderPluginDepthTypeDepth16, // 16 bit depth buffer
    QDemonRenderPluginDepthTypeDepth24, // 24 bit depth buffer
    QDemonRenderPluginDepthTypeDepth32, // 32 bit depth buffer
};

enum QDemonRenderPluginTextureTypes {
    QDemonRenderPluginTextureTypeNoTexture = 0,
    QDemonRenderPluginTextureTypeRGBA8, // 32 bit format
    QDemonRenderPluginTextureTypeRGB8, // 24 bit format
    QDemonRenderPluginTextureTypeRGB565, // 16 bit format
    QDemonRenderPluginTextureTypeRGBA5551, // 16 bit format
};

enum QDemonRenderPluginColorClearState {
    QDemonRenderPluginColorClearStateClearIsOptional = 0,
    QDemonRenderPluginColorClearStateDoNotClear,
    QDemonRenderPluginColorClearStateAlwaysClear,
};

enum QDemonRenderPluginMSAALevel {
    QDemonRenderPluginMSAALevelNoMSAA = 0, // no MSAA, one also works.
    QDemonRenderPluginMSAALevelTwo = 2, // 2 samples
    QDemonRenderPluginMSAALevelFour = 4, // 4 samples
    QDemonRenderPluginMSAALevelEight = 8, // 8 samples
};

typedef long TBool;
#define TTRUE 1
#define TFALSE 0

#define QDEMON_CURRENT_RENDER_PLUGIN_API_VERSION 2

typedef void *TRenderPluginInstancePtr;
typedef void *TRenderPluginClassPtr;

// We will pass the componentized properties to the instance ptr.
typedef struct _RenderPluginPropertyUpdate
{
    QByteArray m_PropName;
    enum QDemonRenderPluginPropertyTypes m_PropertyType;

    // Is either a float or a long or a const char* depending on the property type.
    // for specify types of properties, example code would be:
    // float value = *((float*)&update.m_PropertyValue)
    // long value = *((long*)&update.m_PropertyValue)
    // char* value = (char*)update.m_PropertyValue
    void *m_PropertyValue;
} TRenderPluginPropertyUpdate;

typedef struct _RenderPluginSurfaceDescription
{
    long m_Width;
    long m_Height;
    enum QDemonRenderPluginDepthTypes m_DepthBuffer;
    enum QDemonRenderPluginTextureTypes m_ColorBuffer;
    TBool m_HasStencilBuffer;
    QDemonRenderPluginMSAALevel m_MSAALevel;
} TRenderPluginSurfaceDescription;

typedef struct _TVec2
{
    float x;
    float y;
} TVec2;

typedef struct _NeedsRenderResult
{
    TBool HasChangedSinceLastFrame;
    TBool HasTransparency;
} TNeedsRenderResult;

struct script_State;

/*
 *	Create a new instance object.  Typename is the name of the plugin file, so for example
 *	gears.plugin generates 'gears' as a type name.
 *
 *	Required API function.
 *
 */
typedef TRenderPluginInstancePtr (*TCreateInstanceFunction)(TRenderPluginClassPtr cls,
                                                            const char *inTypeName);

typedef void (*TCreateInstanceScriptProxy)(TRenderPluginClassPtr cls,
                                           TRenderPluginInstancePtr insPtr,
                                           struct script_State *state);

/*
 *	Update the plugin instance with a list of property updates.  Properties are broken down by
 *component so for example
 *  a color property named leftColor will be broken down into 'leftColor.r', 'leftColor.g',
 *'leftColor.b'.  Vector
 *	properties are broken down into x,y,z components.  The property string has a void* member
 *that is the actual value
 *	or in a charPtr property's case it is the char*.
 *	Please see the comments for m_PropertyValue member of TRenderPluginPropertyUpdate struct.
 *
 *	Optional API function.
 */
typedef void (*TUpdateInstanceFunction)(TRenderPluginClassPtr cls,
                                        TRenderPluginInstancePtr instance,
                                        TRenderPluginPropertyUpdate *updates, long numUpdates);

/*
 *	Query used when the plugin is rendering to an image.  Should return the desired
 *specifications of the plugins
 *	render target.
 *	presScaleFactor - the presentation scale factor when the user has requested scale to fit to
 *be used for the
 *		presentation.
 *
 *	Required API function.
 */
typedef TRenderPluginSurfaceDescription (*TSurfaceQueryFunction)(TRenderPluginClassPtr cls,
                                                                 TRenderPluginInstancePtr instance,
                                                                 TVec2 presScaleFactor);

/*
 *	Query used by the rendering system.  Should return true if the plugin will render something
 *different than it did
 *  the last time it rendered.  This is used so that we can cache render results and also so that we
 *can trigger the
 *	progressive AA algorithm in the case where nothing has changed.
 *
 *  presScaleFactor - the presentation scale factor when the user has requested scale to fit to be
 *used for the
 *		presentation.
 *
 *	OpenGL state may be changed in this function.
 *
 *	Optional API function, returns true by default.
 */
typedef TNeedsRenderResult (*TNeedsRenderFunction)(TRenderPluginClassPtr cls,
                                                   TRenderPluginInstancePtr instance,
                                                   TRenderPluginSurfaceDescription surface,
                                                   TVec2 presScaleFactor);

/*
 *  Render plugin data.
 *	Do not assume the surface requested is the surface given; for some cases it will be but if
 *the system has deemed it
 *	appropriate to render the plugin directly to the back buffer then the surface description
 *presented could differ by
 *	quite a bit.
 *
 *	presScaleFactor - is the presentation scale factor when the user has requested scale to fit
 *to be used for the
 *		presentation.
 *  inClearColorBuffer - True if the plugin needs to clear the color buffer (when rendering to
 *texture) else false
 *		(when rendering to back buffer).
 *
 *	Function should return 'UICTRUE' the image produced by rendering contains transparency;
 *either every pixel wasn't
 *	written to or it is desired for the plugin to blend with background objects.  Else should
 *return UICFALSE.
 *
 *	Required API function.
 */
typedef void (*TRenderFunction)(TRenderPluginClassPtr cls, TRenderPluginInstancePtr instance,
                                TRenderPluginSurfaceDescription surface, TVec2 presScaleFactor,
                                QDemonRenderPluginColorClearState inClearColorBuffer);

/*
 *	Pick - handle a mouse pick into the plugin.
 *	Returns true if the pick was consumed, false otherwise.
 *
 *	Option API function.
 */
typedef TBool (*TPickFunction)(TRenderPluginClassPtr cls, TRenderPluginInstancePtr instance,
                                  TVec2 inMouse, TVec2 inViewport);

/*
 *	Release a given instance of the plugin.
 *
 *	Required API function.
 */
typedef void (*TReleaseInstanceFunction)(TRenderPluginClassPtr cls,
                                         TRenderPluginInstancePtr instance);

/*
 *	Get the plugin API version.  This allows the runtime to account for API changes over time or
 *	refuse to load the plugin.  Plugins should return QDEMON_CURRENT_RENDER_PLUGIN_API_VERSION
 *
 *	Required API function.
 */
typedef long (*TGetAPIVersionFunction)(TRenderPluginClassPtr cls);

/*
 *  Initialize the resources for the class.  Implementing this allows UIComposer to move
 *	expensive initialization outside of the actual presentation run, thus allowing for
 *	a smoother experience during the presentation at the cost of longer startup times.
 *
 *	- plugin path is the path to the .plugin xml file so that clients can find resources
 *		specific to their plugin relative to their .plugin file.
 *
 *	OpenGL state may be changed in this function.
 *
 *	Optional API function.
 */
typedef void (*TInitializeClassGLResourcesFunction)(TRenderPluginClassPtr cls,
                                                    const char *pluginPath);

/*
 *	Release the class allocated with the create proc provided in the shared library.
 *
 *	Required API function.
 */
typedef void (*TReleaseClassFunction)(TRenderPluginClassPtr cls);

/*
 *	Structure returned form the create class function.  Unimplemented functions should be left
 *nullptr.
 */
typedef struct _RenderPluginClass
{
    TRenderPluginClassPtr m_Class;

    TGetAPIVersionFunction GetRenderPluginAPIVersion;
    TInitializeClassGLResourcesFunction InitializeClassGLResources;
    TReleaseClassFunction ReleaseClass;

    TCreateInstanceFunction CreateInstance;
    TCreateInstanceScriptProxy CreateInstanceScriptProxy;
    TUpdateInstanceFunction UpdateInstance;
    TSurfaceQueryFunction QueryInstanceRenderSurface;
    TNeedsRenderFunction NeedsRenderFunction;
    TRenderFunction RenderInstance;
    TPickFunction Pick;
    TReleaseInstanceFunction ReleaseInstance;

} TRenderPluginClass;

// We look for this function name in the shared library
#define QDEMON_RENDER_PLUGIN_CREATE_CLASS_FUNCION_NAME "CreateRenderPlugin"

/*
 *	Function signature we expect mapped to "CreateRenderPlugin".  Example code:
 *
 extern "C" {

#ifdef _WIN32
#define PLUGIN_EXPORT_API __declspec(dllexport)
#else
#define PLUGIN_EXPORT_API
#endif



PLUGIN_EXPORT_API TRenderPluginClass CreateRenderPlugin( const char*)
{
        GearClass* classItem = (GearClass*)malloc( sizeof(GearClass) );
        TRenderPluginClass retval;
        memset( &retval, 0, sizeof( TRenderPluginClass ) );
        retval.m_Class = classItem;
        retval.GetRenderPluginAPIVersion = GetAPIVersion;
        retval.CreateInstance = CreateInstance;
        retval.CreateInstanceScriptProxy = CreateInstanceScriptProxy;
        retval.UpdateInstance = UpdateInstance;
        retval.QueryInstanceRenderSurface = QuerySurface;
        retval.RenderInstance = Render;
        retval.ReleaseInstance = ReleaseInstance;
        retval.ReleaseClass = ReleaseClass;
        return retval;
}

 *  Required API function.
 */

typedef TRenderPluginClass (*TCreateRenderPluginClassFunction)(const char *inTypeName);

#ifdef _cplusplus
}
#endif

QT_END_NAMESPACE

#endif
