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
#ifndef QDEMON_RENDER_SHADER_CACHE_H
#define QDEMON_RENDER_SHADER_CACHE_H

#include <QtDemon/qdemonflags.h>
#include <QtDemon/qdemondataref.h>

#include <QtCore/QString>

#include <QtCore/QSharedPointer>
#include <QtCore/QVector>

QT_BEGIN_NAMESPACE
class QDemonRenderShaderProgram;
class QDemonRenderContext;
class IInputStreamFactory;
class IPerfTimer;

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
    QString m_Name;
    bool m_Enabled;
    SShaderPreprocessorFeature()
        : m_Enabled(false)
    {
    }
    SShaderPreprocessorFeature(QString name, bool val)
        : m_Name(name)
        , m_Enabled(val)
    {
    }
    bool operator<(const SShaderPreprocessorFeature &inOther) const;
    bool operator==(const SShaderPreprocessorFeature &inOther) const;
};

typedef QVector<SShaderPreprocessorFeature> TShaderFeatureSet;

inline const QVector<SShaderPreprocessorFeature> ShaderCacheNoFeatures() { return QVector<SShaderPreprocessorFeature>(); }

// Hash is dependent on the order of the keys; so make sure their order is consistent!!
uint HashShaderFeatureSet(QVector<SShaderPreprocessorFeature> inFeatureSet);

class IShaderCache
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
    virtual void SetShaderCachePersistenceEnabled(const QString &inDirectory) = 0;
    virtual bool IsShaderCachePersistenceEnabled() const = 0;
    // It is up to the caller to ensure that inFeatures contains unique keys.
    // It is also up the the caller to ensure the keys are ordered in some way.
    virtual QSharedPointer<QDemonRenderShaderProgram>
    GetProgram(QString inKey, const QVector<SShaderPreprocessorFeature> &inFeatures) = 0;

    // Replace an existing program in the cache for the same key with this program.
    // The shaders returned by *CompileProgram functions can be released by this object
    // due to ForceCompileProgram or SetProjectDirectory, so clients need to either not
    // hold on to them or they need to addref/release them to ensure they still have
    // access to them.
    // The flags just tell us under what gl state to compile the program in order to hopefully
    // reduce program compilations.
    // It is up to the caller to ensure that inFeatures contains unique keys.
    // It is also up the the caller to ensure the keys are ordered in some way.
    virtual QSharedPointer<QDemonRenderShaderProgram>
    ForceCompileProgram(QString inKey, const QString &inVert, const QString &inFrag,
                        const QString &inTessCtrl, const QString &inTessEval,
                        const QString &inGeom, const SShaderCacheProgramFlags &inFlags,
                        const QVector<SShaderPreprocessorFeature> &inFeatures, bool separableProgram,
                        bool fromDisk = false) = 0;

    // It is up to the caller to ensure that inFeatures contains unique keys.
    // It is also up the the caller to ensure the keys are ordered in some way.
    virtual QSharedPointer<QDemonRenderShaderProgram>
    CompileProgram(QString inKey, const QString &inVert, const QString &inFrag,
                   const QString &inTessCtrl, const QString &inTessEval, const QString &inGeom,
                   const SShaderCacheProgramFlags &inFlags,
                   const QVector<SShaderPreprocessorFeature> &inFeatures,
                   bool separableProgram = false) = 0;

    // Used to disable any shader compilation during loading.  This is used when we are just
    // interested in going from uia->binary
    // and we expect to run on a headless server of sorts.  See the UICCompiler project for its
    // only current use case.
    virtual void SetShaderCompilationEnabled(bool inEnableShaderCompilation) = 0;

    // Upping the shader version invalidates all previous cache files.
    static quint32 GetShaderVersion() { return 4; }
    static const QString GetShaderCacheFileName() { return QStringLiteral("shadercache.xml"); }

    static QSharedPointer<IShaderCache> CreateShaderCache(QSharedPointer<QDemonRenderContext> inContext,
                                                          QSharedPointer<IInputStreamFactory> inInputStreamFactory,
                                                          QSharedPointer<IPerfTimer> inPerfTimer);
};

struct SShaderCacheKey
{
    QString m_Key;
    QVector<SShaderPreprocessorFeature> m_Features;
    uint m_HashCode;

    SShaderCacheKey(QString key = QString())
        : m_Key(key)
        , m_HashCode(0)
    {
    }

    SShaderCacheKey(const SShaderCacheKey &other)
        : m_Key(other.m_Key)
        , m_Features(other.m_Features)
        , m_HashCode(other.m_HashCode)
    {
    }

    SShaderCacheKey &operator=(const SShaderCacheKey &other)
    {
        m_Key = other.m_Key;
        m_Features = other.m_Features;
        m_HashCode = other.m_HashCode;
        return *this;
    }

    void GenerateHashCode()
    {
        m_HashCode = qHash(m_Key);
        m_HashCode = m_HashCode ^ HashShaderFeatureSet(m_Features);
    }

    bool operator==(const SShaderCacheKey &inOther) const
    {
        return m_Key == inOther.m_Key && m_Features == inOther.m_Features;
    }
};

QT_END_NAMESPACE

#endif
