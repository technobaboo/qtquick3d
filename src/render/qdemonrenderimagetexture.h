/****************************************************************************
**
** Copyright (C) 2008-2012 NVIDIA Corporation.
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

#ifndef QDEMON_RENDER_RENDER_IMAGE_TEXTURE_H
#define QDEMON_RENDER_RENDER_IMAGE_TEXTURE_H

#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrenderbackend.h>

QT_BEGIN_NAMESPACE

class QDemonRenderContext;
class QDemonRenderTexture2D;

// a wrapper class for QDemonRenderTexture2D
// to use with compute shaders and load / store image shaders

class Q_DEMONRENDER_EXPORT QDemonRenderImage2D
{
    Q_DISABLE_COPY(QDemonRenderImage2D)
public:
    QAtomicInt ref;

private:
    QDemonRef<QDemonRenderContext> m_context; ///< pointer to context
    QDemonRef<QDemonRenderBackend> m_backend; ///< pointer to backend
    QDemonRef<QDemonRenderTexture2D> m_texture2D; ///< pointer to texture
    qint32 m_textureUnit; ///< texture unit this texture should use
    QDemonRenderImageAccessType m_accessType; ///< texture / image access type ( read, write, read_write )
    quint32 m_textureLevel; ///< texture level we use for this image

public:
    /**
     * @brief constructor
     *
     * @param[in] context		Pointer to context
     * @param[in] fnd			Pointer to foundation
     * @param[in] inTexture		Pointer to a QDemonRenderTexture2D object
     * @param[in] inAccess		Image access type ( read, write, read_write )
     *
     * @return No return.
     */
    QDemonRenderImage2D(const QDemonRef<QDemonRenderContext> &context,
                        const QDemonRef<QDemonRenderTexture2D> &inTexture,
                        QDemonRenderImageAccessType inAccess);

    ~QDemonRenderImage2D();

    /**
     * @brief	Set the access rights within the shader.
     *			Can be read, write or read_write.
     *
     * @param[in] inAccess		Image access type ( read, write, read_write )
     *
     * @return No return.
     */
    void setAccessType(QDemonRenderImageAccessType inAccess) { m_accessType = inAccess; }

    /**
     * @brief	Set the texture level we use for this image
     *
     * @param[in] inLevel		texture level ( must be in range of max levels )
     *
     * @return No return.
     */
    void setTextureLevel(qint32 inLevel);

    /**
     * @brief	Get texture unit used
     *
     *
     * @return texture unit bound to.
     */
    quint32 textureUnit() const { return m_textureUnit; }

    /**
     * @brief Bind a texture for shader access
     *
     * @param[in] unit		The binding point
     *
     * @return No return.
     */
    void bind(qint32 unit);

    /**
     * @brief get the backend object handle
     *		  here we return the handle from the wrapped texture
     *
     * @return the backend object handle.
     */
    QDemonRenderBackend::QDemonRenderBackendTextureObject handle();

};

QT_END_NAMESPACE

#endif
