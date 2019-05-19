#ifndef ASSIMPIMPORTERPLUGIN_H
#define ASSIMPIMPORTERPLUGIN_H

#include <QtDemonAssetImport/private/qdemonassetimporterplugin_p.h>

QT_BEGIN_NAMESPACE

class AssimpImporterPlugin : public QDemonAssetImporterPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QDemonAssetImporterFactoryInterface_iid FILE "assimp.json")

public:
    QDemonAssetImporter *create(const QString &key, const QStringList &paramList) override;
};

QT_END_NAMESPACE


#endif // ASSIMPIMPORTERPLUGIN_H
