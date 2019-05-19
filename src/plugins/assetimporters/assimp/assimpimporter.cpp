#include "assimpimporter.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/Logger.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/postprocess.h>

QT_BEGIN_NAMESPACE

AssimpImporter::AssimpImporter()
{
    m_importer = new Assimp::Importer();
}

AssimpImporter::~AssimpImporter()
{
    delete m_importer;
}

const QString AssimpImporter::name() const
{
    return QStringLiteral("assimp");
}

const QStringList AssimpImporter::inputExtensions() const
{
    QStringList extensions;
    extensions.append(QStringLiteral("fbx"));
    extensions.append(QStringLiteral("dae"));
    extensions.append(QStringLiteral("obj"));
    extensions.append(QStringLiteral("blend"));
    extensions.append(QStringLiteral("gltf"));
    extensions.append(QStringLiteral("glb"));
    return extensions;
}

const QString AssimpImporter::outputExtension() const
{
    return QStringLiteral(".qml");
}

const QString AssimpImporter::type() const
{
    return QStringLiteral("Scene");
}

const QVariantMap AssimpImporter::importOptions() const
{
    return QVariantMap();
}

const QString AssimpImporter::import(const QString &sourceFile, const QDir &savePath, const QVariantMap &options, QStringList *generatedFiles)
{
    const aiScene *scene = m_importer->ReadFile(sourceFile.toStdString(), aiProcessPreset_TargetRealtime_Quality);
    if (!scene) {
        // Scene failed to load, use logger to get the reason
        return QString::fromLocal8Bit(m_importer->GetErrorString());
    }

    return QString();
}

QT_END_NAMESPACE
