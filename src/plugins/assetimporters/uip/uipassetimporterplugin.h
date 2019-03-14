#ifndef UIPASSETIMPORTERPLUGIN_H
#define UIPASSETIMPORTERPLUGIN_H

#include <QtDemonAssetImport/private/qdemonassetimporterplugin_p.h>

QT_BEGIN_NAMESPACE

class UipAssetImporterPlugin : public QDemonAssetImporterPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QDemonAssetImporterFactoryInterface_iid FILE "uip.json")
public:
    QDemonAssetImporter *create(const QString &key, const QStringList &paramList) override;
};

QT_END_NAMESPACE

#endif // UIPASSETIMPORTERPLUGIN_H
