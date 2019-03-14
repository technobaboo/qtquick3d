#include "uipassetimporterplugin.h"
#include "uipimporter.h"

QT_BEGIN_NAMESPACE

QDemonAssetImporter *UipAssetImporterPlugin::create(const QString &key, const QStringList &paramList)
{
    Q_UNUSED(key)
    Q_UNUSED(paramList)
    return new UipImporter();
}

QT_END_NAMESPACE
