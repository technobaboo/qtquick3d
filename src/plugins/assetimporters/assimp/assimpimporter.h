#ifndef ASSIMPIMPORTER_H
#define ASSIMPIMPORTER_H

#include <QtDemonAssetImport/private/qdemonassetimporter_p.h>

#include <QtCore/QStringList>

namespace Assimp {
class Importer;
}

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
    Assimp::Importer *m_importer = nullptr;
};

QT_END_NAMESPACE

#endif // ASSIMPIMPORTER_H
