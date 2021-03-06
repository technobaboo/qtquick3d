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

#ifndef QSSG_RENDER_SHADER_CACHE_H
#define QSSG_RENDER_SHADER_CACHE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtQuick3DUtils/private/qssgdataref_p.h>

#include <QtCore/QString>

#include <QtCore/QSharedPointer>
#include <QtCore/QVector>

QT_BEGIN_NAMESPACE
class QSSGRenderShaderProgram;
class QSSGRenderContext;
class QSSGInputStreamFactory;
class QSSGPerfTimer;

enum class ShaderCacheProgramFlagValues : quint32
{
    TessellationEnabled = 1 << 0, // tessellation enabled
    GeometryShaderEnabled = 1 << 1, // geometry shader enabled
};

Q_DECLARE_FLAGS(QSSGShaderCacheProgramFlags, ShaderCacheProgramFlagValues)

namespace QSSGShaderDefines
{
QByteArray lightProbe();
QByteArray lightProbe2();
QByteArray iblFov();
QByteArray ssm();
QByteArray ssao();
QByteArray ssdo();
QByteArray cgLighting();
}

// There are a number of macros used to turn on or off various features.  This allows those
// features
// to be propagated into the shader cache's caching mechanism.  They will be translated into
//#define name value where value is 1 or zero depending on if the feature is enabled or not.
struct QSSGShaderPreprocessorFeature
{
    QByteArray name;
    mutable bool enabled = false;
    QSSGShaderPreprocessorFeature() = default;
    QSSGShaderPreprocessorFeature(const QByteArray &inName, bool val) : name(inName), enabled(val) {}
    bool operator<(const QSSGShaderPreprocessorFeature &inOther) const;
    bool operator==(const QSSGShaderPreprocessorFeature &inOther) const;
};

typedef QVector<QSSGShaderPreprocessorFeature> TShaderFeatureSet;

inline const QVector<QSSGShaderPreprocessorFeature> shaderCacheNoFeatures()
{
    return QVector<QSSGShaderPreprocessorFeature>();
}

// Hash is dependent on the order of the keys; so make sure their order is consistent!!
uint hashShaderFeatureSet(const QVector<QSSGShaderPreprocessorFeature> &inFeatureSet);

struct QSSGShaderCacheKey
{
    QByteArray m_key;
    QVector<QSSGShaderPreprocessorFeature> m_features;
    uint m_hashCode = 0;

    explicit QSSGShaderCacheKey(const QByteArray &key = QByteArray()) : m_key(key), m_hashCode(0) {}

    QSSGShaderCacheKey(const QSSGShaderCacheKey &other) = default;
    QSSGShaderCacheKey &operator=(const QSSGShaderCacheKey &other) = default;

    void generateHashCode()
    {
        m_hashCode = qHash(m_key);
        m_hashCode = m_hashCode ^ hashShaderFeatureSet(m_features);
    }

    bool operator==(const QSSGShaderCacheKey &inOther) const
    {
        return m_key == inOther.m_key && m_features == inOther.m_features;
    }
};


class QSSGShaderCache
{
    enum class ShaderType
    {
        Vertex, TessControl, TessEval, Fragment, Geometry, Compute
    };

public:
    QAtomicInt ref;
private:
    typedef QHash<QSSGShaderCacheKey, QSSGRef<QSSGRenderShaderProgram>> TShaderMap;
    QSSGRef<QSSGRenderContext> m_renderContext;
    QSSGPerfTimer *m_perfTimer;
    TShaderMap m_shaders;
    QString m_cacheFilePath;
    QByteArray m_vertexCode;
    QByteArray m_tessCtrlCode;
    QByteArray m_tessEvalCode;
    QByteArray m_geometryCode;
    QByteArray m_fragmentCode;
    QByteArray m_insertStr;
    QString m_flagString;
    QString m_contextTypeString;
    QSSGShaderCacheKey m_tempKey;

    QSSGRef<QSSGInputStreamFactory> m_inputStreamFactory;
    bool m_shaderCompilationEnabled;

    void addBackwardCompatibilityDefines(ShaderType shaderType);

    void addShaderExtensionStrings(ShaderType shaderType, bool isGLES);

    void addShaderPreprocessor(QByteArray &str,
                               const QByteArray &inKey,
                               ShaderType shaderType,
                               const QVector<QSSGShaderPreprocessorFeature> &inFeatures);

public:
    QSSGShaderCache(const QSSGRef<QSSGRenderContext> &ctx,
                const QSSGRef<QSSGInputStreamFactory> &inInputStreamFactory,
                QSSGPerfTimer *inPerfTimer);
    ~QSSGShaderCache();
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
    void setShaderCachePersistenceEnabled(const QString &inDirectory);
    bool isShaderCachePersistenceEnabled() const;
    // It is up to the caller to ensure that inFeatures contains unique keys.
    // It is also up the the caller to ensure the keys are ordered in some way.
    QSSGRef<QSSGRenderShaderProgram> getProgram(const QByteArray &inKey,
                                                    const QVector<QSSGShaderPreprocessorFeature> &inFeatures);

    // Replace an existing program in the cache for the same key with this program.
    // The shaders returned by *CompileProgram functions can be released by this object
    // due to ForceCompileProgram or SetProjectDirectory, so clients need to either not
    // hold on to them or they need to addref/release them to ensure they still have
    // access to them.
    // The flags just tell us under what gl state to compile the program in order to hopefully
    // reduce program compilations.
    // It is up to the caller to ensure that inFeatures contains unique keys.
    // It is also up the the caller to ensure the keys are ordered in some way.
    QSSGRef<QSSGRenderShaderProgram> forceCompileProgram(const QByteArray &inKey,
                                                                     const QByteArray &inVert,
                                                                     const QByteArray &inFrag,
                                                                     const QByteArray &inTessCtrl,
                                                                     const QByteArray &inTessEval,
                                                                     const QByteArray &inGeom,
                                                                     const QSSGShaderCacheProgramFlags &inFlags,
                                                                     const QVector<QSSGShaderPreprocessorFeature> &inFeatures,
                                                                     bool separableProgram,
                                                                     bool fromDisk = false);

    // It is up to the caller to ensure that inFeatures contains unique keys.
    // It is also up the the caller to ensure the keys are ordered in some way.
    QSSGRef<QSSGRenderShaderProgram> compileProgram(const QByteArray &inKey,
                                                                const QByteArray &inVert,
                                                                const QByteArray &inFrag,
                                                                const QByteArray &inTessCtrl,
                                                                const QByteArray &inTessEval,
                                                                const QByteArray &inGeom,
                                                                const QSSGShaderCacheProgramFlags &inFlags,
                                                                const QVector<QSSGShaderPreprocessorFeature> &inFeatures,
                                                                bool separableProgram = false);

    // Used to disable any shader compilation during loading.  This is used when we are just
    // interested in going from uia->binary
    // and we expect to run on a headless server of sorts.  See the UICCompiler project for its
    // only current use case.
    void setShaderCompilationEnabled(bool inEnableShaderCompilation);

    // Upping the shader version invalidates all previous cache files.
    static quint32 getShaderVersion() { return 4; }
    static const QString getShaderCacheFileName() { return QStringLiteral("shadercache.xml"); }

    static QSSGRef<QSSGShaderCache> createShaderCache(const QSSGRef<QSSGRenderContext> &inContext,
                                                          const QSSGRef<QSSGInputStreamFactory> &inInputStreamFactory,
                                                          QSSGPerfTimer *inPerfTimer);
};

QT_END_NAMESPACE

#endif
