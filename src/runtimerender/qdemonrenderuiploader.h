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
#pragma once
#ifndef QDEMON_RENDER_UIP_LOADER_H
#define QDEMON_RENDER_UIP_LOADER_H

QT_BEGIN_NAMESPACE

#ifdef QDEMON_RENDER_ENABLE_LOAD_UIP

#include <Qt3DSRender.h>
#include <StringTable.h>
#include <Qt3DSContainers.h>
#include <QtDemonRuntimeRender/qdemonrendergraphobject.h>

namespace Q3DStudio {
class IRuntimeMetaData;
}

namespace qt3dsdm {
class IDOMReader;
struct SMetaDataEffect;
struct SMetaDataCustomMaterial;
}

namespace qt3ds {
namespace render {

    class IBufferManager;

    typedef QHash<QString, SGraphObject *> TIdObjectMap;

    struct IUIPReferenceResolver
    {
    protected:
        virtual ~IUIPReferenceResolver() {}
    public:
        virtual QString ResolveReference(QString inStart,
                                                   const char *inReference) = 0;
    };

    struct SPresentation;

    class QDEMON_AUTOTEST_EXPORT IUIPLoader
    {
    public:
        // The reader needs to point to the top of the file, we will search
        // several objects that exist at the top level of the uip file.
        // Returns nullptr if we were incapable of loading the presentation.
        static SPresentation *
        LoadUIPFile(qt3dsdm::IDOMReader &inReader
                    // the full path, including the filename
                    // to the presentation file
                    ,
                    const char *inFullPathToPresentationFile,
                    Q3DStudio::IRuntimeMetaData &inMetaData,
                    TIdObjectMap &ioObjectMap
                    // Buffer manager to load details about the images
                    ,
                    IBufferManager &inBufferManager
                    // To load effects we need the effect system
                    // and the presentation directory
                    ,
                    IEffectSystem &inEffectSystem, const char *inPresentationDir,
                    IRenderPluginManager &inPluginManager, ICustomMaterialSystem &inMaterialSystem,
                    IDynamicObjectSystem &inDynamicSystem, IPathManager &inPathManager
                    // Resolve references to objects; this is done by the main uip loader during
                    // its normal mode of operation so we try to reuse that code.
                    ,
                    IUIPReferenceResolver *inResolver
                    // Set some initial values by going to the master slide then slide 1
                    // Useful for quick testing, sort of equivalent to showing the first frame
                    // of a given presentation
                    ,
                    bool setValuesFromSlides = false);

        static void CreateEffectClassFromMetaEffect(QString inEffectName,
                                                    IEffectSystem &inEffectSystem,
                                                    const qt3dsdm::SMetaDataEffect &inMetaDataEffect);

        static void CreateMaterialClassFromMetaMaterial(
            QString inEffectName,
            ICustomMaterialSystem &inEffectSystem,
            const qt3dsdm::SMetaDataCustomMaterial &inMetaDataMaterial);
    };
}
}

#endif

QT_END_NAMESPACE
#endif
