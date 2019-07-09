/****************************************************************************
**
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


#include <QtTest>
#include <QDebug>
#include <QtDemonAssetImport/QDemonAssetImportManager>
#include <QDir>
#include <QByteArray>

// add necessary includes here

class tst_assetimport : public QObject
{
    Q_OBJECT

public:
    tst_assetimport();
    ~tst_assetimport();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void importFile_data();
    void importFile();

};

tst_assetimport::tst_assetimport()
{

}

tst_assetimport::~tst_assetimport()
{

}

void tst_assetimport::initTestCase()
{

}

void tst_assetimport::cleanupTestCase()
{

}

void tst_assetimport::importFile_data()
{
    QTest::addColumn<QString>("extension");
    QTest::addColumn<bool>("result");
    QTest::addColumn<QByteArray>("expectedHash");

#ifdef __linux__
    QTest::newRow("fbx") << QString("fbx") << true << QByteArray("c9353c3c2f9681d9053d5145d2fe859e");
    QTest::newRow("dae") << QString("dae") << true << QByteArray("affb0aa717928fedebb8ab3edbbd4e00");
    QTest::newRow("obj") << QString("obj") << true << QByteArray("890b94fe5315c9595d97561f8d879464");
    QTest::newRow("blend") << QString("blend") << true << QByteArray("fb6049cbd40c2dee06c0165b0faaf6f5");
    QTest::newRow("gltf") << QString("gltf") << true << QByteArray("2a804e289ce699db8d0c3cc6a72bfc6f");
    QTest::newRow("glb") << QString("glb") << true << QByteArray("2a804e289ce699db8d0c3cc6a72bfc6f");
#elif _WIN32
    QTest::newRow("fbx") << QString("fbx") << true << QByteArray("44960575115bdedd344d0cbbf037cf2c");
    QTest::newRow("dae") << QString("dae") << true << QByteArray("affb0aa717928fedebb8ab3edbbd4e00");
    QTest::newRow("obj") << QString("obj") << true << QByteArray("ee7d36950d4677ac2a01e97c406d1b51");
    QTest::newRow("blend") << QString("blend") << true << QByteArray("6b519f1f36da183a06655ac460cd66e6");
    QTest::newRow("gltf") << QString("gltf") << true << QByteArray("2a804e289ce699db8d0c3cc6a72bfc6f");
    QTest::newRow("glb") << QString("glb") << true << QByteArray("2a804e289ce699db8d0c3cc6a72bfc6f");
#else
    QSKIP("Test not configured for this platform.")
#endif
}

void tst_assetimport::importFile()
{
    QFETCH(QString, extension);
    QFETCH(bool, result);
    QFETCH(QByteArray, expectedHash);

    QDemonAssetImportManager importManager;
    QString file = "resources/cube_scene." + extension;
    QString error;
    QByteArray fileChecksum;

    // Should return "true" if there were no errors opening the source or creating the exported object.
    auto realResult = importManager.importFile(QFINDTESTDATA(file), QDir("./"), &error);
    if(!error.isEmpty()){
        qDebug() << "Error message:" << error;
        QFAIL(error.toStdString().c_str());
    } else {
        // Generate a file hash of the created QML export to verify it was created correctly.
        // Returns empty QByteArray() on failure.
        QFile f("Cube_scene.qml");
        if (f.open(QFile::ReadOnly)) {
            QCryptographicHash hash(QCryptographicHash::Md5);
            if (hash.addData(&f)) {
                fileChecksum = hash.result();
            }
        }
    }

    QCOMPARE(realResult, result);
    QCOMPARE(fileChecksum.toHex(), expectedHash);
}

QTEST_APPLESS_MAIN(tst_assetimport)

#include "tst_assetimport.moc"
