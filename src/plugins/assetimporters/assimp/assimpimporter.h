#ifndef ASSIMPIMPORTER_H
#define ASSIMPIMPORTER_H

#include <QtDemonAssetImport/private/qdemonassetimporter_p.h>

#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QtCore/QHash>
#include <QtCore/QTemporaryDir>

namespace Assimp {
class Importer;
}

class aiNode;
class aiCamera;
class aiLight;
class aiScene;

QT_BEGIN_NAMESPACE

class AssimpImporter : public QDemonAssetImporter
{
public:
    AssimpImporter();
    ~AssimpImporter();

    const QString name() const;
    const QStringList inputExtensions() const;
    const QString outputExtension() const;
    const QString type() const;
    const QVariantMap importOptions() const;
    const QString import(const QString &sourceFile, const QDir &savePath, const QVariantMap &options, QStringList *generatedFiles);

private:
    void writeHeader(QTextStream &output);
    void processNode(aiNode *node, QTextStream &output, int tabLevel = 0);
    void generateModelProperties(aiNode *modelNode, QTextStream &output, int tabLevel);
    void generateLightProperties(aiNode *lightNode, QTextStream &output, int tabLevel);
    void generateCameraProperties(aiNode *cameraNode, QTextStream &output, int tabLevel);
    void generateNodeProperties(aiNode *node, QTextStream &output, int tabLevel);
    bool isModel(aiNode *node);
    bool isLight(aiNode *node);
    bool isCamera(aiNode *node);

    Assimp::Importer *m_importer = nullptr;
    const aiScene *m_scene = nullptr;

    QHash<aiNode *, aiCamera *> m_cameras;
    QHash<aiNode *, aiLight *> m_lights;

    QTemporaryDir m_meshCache;
    QDir m_savePath;
};

QT_END_NAMESPACE

#endif // ASSIMPIMPORTER_H
