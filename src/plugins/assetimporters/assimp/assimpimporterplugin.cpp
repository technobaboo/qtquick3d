#include "assimpimporterplugin.h"
#include "assimpimporter.h"

QT_BEGIN_NAMESPACE

QDemonAssetImporter *AssimpImporterPlugin::create(const QString &key, const QStringList &paramList)
{
    Q_UNUSED(key)
    Q_UNUSED(paramList)
    return new AssimpImporter();
}

QT_END_NAMESPACE
