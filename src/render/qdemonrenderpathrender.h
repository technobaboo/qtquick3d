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
#ifndef QDEMON_RENDER_PATH_RENDER_H
#define QDEMON_RENDER_PATH_RENDER_H

#include <QtDemonRender/qdemonrenderbackend.h>
#include <QtDemon/QDemonBounds3>

QT_BEGIN_NAMESPACE

class QDemonRenderContextImpl;
class QDemonRenderPathSpecification;
class QDemonRenderPathFontSpecification;

///< A program pipeline is a collection of a multiple programs (vertex, fragment, geometry,....)
class QDemonRenderPathRender
{
protected:
    QSharedPointer<QDemonRenderContextImpl> m_Context; ///< pointer to context
    QSharedPointer<QDemonRenderBackend> m_Backend; ///< pointer to backend

public:
    /**
         * @brief constructor
         *
         * @param[in] context		Pointer to render context
         * @param[in] fnd			Pointer to foundation
          * @param[in] range		Number of internal objects
         *
         * @return No return.
         */
    QDemonRenderPathRender(QSharedPointer<QDemonRenderContextImpl> context, size_t range);

    /// @brief destructor
    ~QDemonRenderPathRender();

    /**
         * @brief get the backend object handle
         *
         * @return the backend object handle.
         */
    QDemonRenderBackend::QDemonRenderBackendPathObject GetPathHandle() { return m_PathRenderHandle; }

    // The render context can create a path specification object.
    void SetPathSpecification(QSharedPointer<QDemonRenderPathSpecification> inCommandBuffer);

    QDemonBounds3 GetPathObjectBoundingBox();
    QDemonBounds3 GetPathObjectFillBox();
    QDemonBounds3 GetPathObjectStrokeBox();

    void SetStrokeWidth(float inStrokeWidth);
    float GetStrokeWidth() const;

    void StencilStroke();
    void StencilFill();

    /**
         * @brief static create function
         *
         * @param[in] context		Pointer to render context
         * @param[in] range			Number of internal objects
         *
         * @return the backend object handle.
         */
    static QSharedPointer<QDemonRenderPathRender> Create(QSharedPointer<QDemonRenderContextImpl> context, size_t range);

private:
    QDemonRenderBackend::QDemonRenderBackendPathObject m_PathRenderHandle; ///< opaque backend handle
    size_t m_Range; ///< range of internal objects
    float m_StrokeWidth;
};

QT_END_NAMESPACE

#endif
