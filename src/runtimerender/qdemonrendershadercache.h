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
#ifndef QDEMON_RENDER_SHADER_CACHE_H
#define QDEMON_RENDER_SHADER_CACHE_H
#include <QtDemonRuntimeRender/qdemonrender.h>
#include <QtDemon/qdemonrefcounted.h>
#include <Qt3DSFlags.h>
#include <StringTable.h>

QT_BEGIN_NAMESPACE
struct ShaderCacheProgramFlagValues
{
    enum Enum {
        TessellationEnabled = 1 << 0, // tessellation enabled
        GeometryShaderEnabled = 1 << 1, // geometry shader enabled
    };
};
struct SShaderCacheProgramFlags : public QDemonFlags<ShaderCacheProgramFlagValues::Enum, quint32>
{
    // tessellation enabled
    void SetTessellationEnabled(bool inValue)
    {
        clearOrSet(inValue, ShaderCacheProgramFlagValues::TessellationEnabled);
    }
    bool IsTessellationEnabled() const
    {
        return this->operator&(ShaderCacheProgramFlagValues::TessellationEnabled);
    }
    // geometry shader enabled
    void SetGeometryShaderEnabled(bool inValue)
    {
        clearOrSet(inValue, ShaderCacheProgramFlagValues::GeometryShaderEnabled);
    }
    bool IsGeometryShaderEnabled() const
    {
        return this->operator&(ShaderCacheProgramFlagValues::GeometryShaderEnabled);
    }
};
// There are a number of macros used to turn on or off various features.  This allows those
// features
// to be propagated into the shader cache's caching mechanism.  They will be translated into
//#define name value where value is 1 or zero depending on if the feature is enabled or not.
struct SShaderPreprocessorFeature
{
    CRegisteredString m_Name;
    bool m_Enabled;
    SShaderPreprocessorFeature()
        : m_Enabled(false)
    {
    }
    SShaderPreprocessorFeature(CRegisteredString name, bool val)
        : m_Name(name)
        , m_Enabled(val)
    {
    }
    bool operator<(const SShaderPreprocessorFeature &inOther) const;
    bool operator==(const SShaderPreprocessorFeature &inOther) const;
};

typedef QDemonConstDataRef<SShaderPreprocessorFeature> TShaderFeatureSet;

inline TShaderFeatureSet ShaderCacheNoFeatures() { return TShaderFeatureSet(); }

// Hash is dependent on the order of the keys; so make sure their order is consistent!!
size_t HashShaderFeatureSet(QDemonConstDataRef<SShaderPreprocessorFeature> inFeatureSet);

class IShaderCache : public QDemonRefCounted
{
protected:
    virtual ~IShaderCache() {}
public:
    // If directory is nonnull, then we attempt to load any shaders from shadercache.xml in
    // inDirectory
    // and save any new ones out to the same file.  The shaders are marked by the gl version
    // used when saving.
    // If we can't open shadercache.xml from inDirectory for writing (at least), then we still
    // consider the
    // shadercache to be disabled.
    // This call immediately blocks and attempts to load all applicable shaders from the
    // shadercache.xml file in
    // the given directory.
    virtual void SetShaderCachePersistenceEnabled(const char8_t *inDirectory) = 0;
    virtual bool IsShaderCachePersistenceEnabled() const = 0;
    // It is up to the caller to ensure that inFeatures contains unique keys.
    // It is also up the the caller to ensure the keys are ordered in some way.
    virtual QDemonRenderShaderProgram *
    GetProgram(CRegisteredString inKey,
               QDemonConstDataRef<SShaderPreprocessorFeature> inFeatures) = 0;

    // Replace an existing program in the cache for the same key with this program.
    // The shaders returned by *CompileProgram functions can be released by this object
    // due to ForceCompileProgram or SetProjectDirectory, so clients need to either not
    // hold on to them or they need to addref/release them to ensure they still have
    // access to them.
    // The flags just tell us under what gl state to compile the program in order to hopefully
    // reduce program compilations.
    // It is up to the caller to ensure that inFeatures contains unique keys.
    // It is also up the the caller to ensure the keys are ordered in some way.
    virtual QDemonRenderShaderProgram *
    ForceCompileProgram(CRegisteredString inKey, const char8_t *inVert, const char8_t *inFrag,
                        const char8_t *inTessCtrl, const char8_t *inTessEval,
                        const char8_t *inGeom, const SShaderCacheProgramFlags &inFlags,
                        TShaderFeatureSet inFeatures, bool separableProgram,
                        bool fromDisk = false) = 0;

    // It is up to the caller to ensure that inFeatures contains unique keys.
    // It is also up the the caller to ensure the keys are ordered in some way.
    virtual QDemonRenderShaderProgram *
    CompileProgram(CRegisteredString inKey, const char8_t *inVert, const char8_t *inFrag,
                   const char8_t *inTessCtrl, const char8_t *inTessEval, const char8_t *inGeom,
                   const SShaderCacheProgramFlags &inFlags, TShaderFeatureSet inFeatures,
                   bool separableProgram = false) = 0;

    // Used to disable any shader compilation during loading.  This is used when we are just
    // interested in going from uia->binary
    // and we expect to run on a headless server of sorts.  See the UICCompiler project for its
    // only current use case.
    virtual void SetShaderCompilationEnabled(bool inEnableShaderCompilation) = 0;

    // Upping the shader version invalidates all previous cache files.
    static quint32 GetShaderVersion() { return 4; }
    static const char8_t *GetShaderCacheFileName() { return "shadercache.xml"; }

    static IShaderCache &CreateShaderCache(QDemonRenderContext &inContext,
                                           IInputStreamFactory &inInputStreamFactory,
                                           IPerfTimer &inPerfTimer);
};
QT_END_NAMESPACE

#endif
