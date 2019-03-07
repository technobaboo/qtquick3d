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
#ifndef QDEMON_RENDER_SHADER_PROGRAM_H
#define QDEMON_RENDER_SHADER_PROGRAM_H

#include <QtDemonRender/qdemonrendershaderconstant.h>

#include <QtCore/QString>
#include <QtCore/QHash>

QT_BEGIN_NAMESPACE

///< forward declarations
class QDemonRenderContextImpl;
class QDemonRenderVertexShader;
class QDemonRenderFragmentShader;
class QDemonRenderTessControlShader;
class QDemonRenderTessEvaluationShader;
class QDemonRenderGeometryShader;
class QDemonRenderShaderConstantBase;
class QDemonRenderShaderBufferBase;
class QDemonRenderComputeShader;

typedef QHash<QString, QDemonRef<QDemonRenderShaderConstantBase>> TShaderConstantMap;
typedef QHash<QByteArray, QDemonRef<QDemonRenderShaderBufferBase>> TShaderBufferMap;

///< A shader program is an object composed of a multiple shaders (vertex, fragment,
/// geometry,....)
class Q_DEMONRENDER_EXPORT QDemonRenderShaderProgram
{
public:
    QAtomicInt ref;

    enum class ProgramType
    {
        Graphics,
        Compute
    };

private:
    QDemonRef<QDemonRenderContextImpl> m_context; ///< pointer to context
    QDemonRef<QDemonRenderBackend> m_backend; ///< pointer to backend
    const char *m_programName; /// Name of the program
    QDemonRenderBackend::QDemonRenderBackendShaderProgramObject m_programHandle; ///< opaque backend handle
    TShaderConstantMap m_constants; ///< map of shader constants
    TShaderBufferMap m_shaderBuffers; ///< map of shader buffers
    ProgramType m_programType; ///< shader type
    QByteArray m_errorMessage; ///< contains the error message if linking fails

    /**
     * @brief create vertex shader
     *
     * @param[in] context					Pointer to render context
     * @param[in] vertexShaderSource		Fragment shader source code
     * @param[in] binaryProgram				True if binary program
     *
     * @return pointer to vertex shader object
     */
    static QDemonOption<QDemonRenderVertexShader *> createVertexShader(const QDemonRef<QDemonRenderContextImpl> &context,
                                                                       QDemonConstDataRef<qint8> vertexShaderSource,
                                                                       bool binaryProgram = false);

    /**
     * @brief create fragment shader
     *
     * @param[in] context					Pointer to render context
     * @param[in] fragmentShaderSource		Fragment shader source code
     * @param[in] binaryProgram				True if binary program
     *
     * @return pointer to fragment shader object
     */
    static QDemonOption<QDemonRenderFragmentShader *> createFragmentShader(const QDemonRef<QDemonRenderContextImpl> &context,
                                                                           QDemonConstDataRef<qint8> fragmentShaderSource,
                                                                           bool binaryProgram = false);

    /**
     * @brief create tesselation control shader
     *
     * @param[in] context					Pointer to render context
     * @param[in] tessControlShaderSource	Tessellation control shader source code
     * @param[in] binaryProgram				True if binary program
     *
     * @return pointer to tessellation control shader
     */
    static QDemonOption<QDemonRenderTessControlShader *> createTessControlShader(const QDemonRef<QDemonRenderContextImpl> &context,
                                                                                 QDemonConstDataRef<qint8> tessControlShaderSource,
                                                                                 bool binaryProgram = false);

    /**
     * @brief create tesselation evaluation shader
     *
     * @param[in] context						Pointer to render context
     * @param[in] tessEvaluationShaderSource	Tessellation evaluation shader source code
     * @param[in] binaryProgram					True if binary program
     *
     * @return pointer to tessellation evaluation shader
     */
    static QDemonOption<QDemonRenderTessEvaluationShader *> createTessEvaluationShader(const QDemonRef<QDemonRenderContextImpl> &context,
                                                                                       QDemonConstDataRef<qint8> tessEvaluationShaderSource,
                                                                                       bool binaryProgram = false);

    /**
     * @brief create geometry shader
     *
     * @param[in] context					Pointer to render context
     * @param[in] geometryShaderSource		Geometry shader source code
     * @param[in] binaryProgram				True if binary program
     *
     * @return pointer to geometry shader
     */
    static QDemonOption<QDemonRenderGeometryShader *> createGeometryShader(const QDemonRef<QDemonRenderContextImpl> &context,
                                                                           QDemonConstDataRef<qint8> geometryShaderSource,
                                                                           bool binaryProgram = false);

public:
    /**
     * @brief constructor
     *
     * @param[in] context			Pointer to render context
     * @param[in] fnd				Pointer to foundation
     * @param[in] programName		Pointer to string of program name
     * @param[in] separableProgram	True if this is a separable program
     *
     * @return No return.
     */
    QDemonRenderShaderProgram(const QDemonRef<QDemonRenderContextImpl> &context, const char *programName, bool separableProgram);

    /// destructor
    virtual ~QDemonRenderShaderProgram();

    /**
     * @brief attach a shader to the program
     *
     * @param[in] pShader		Pointer to shader object
     *
     * @return No return.
     */
    template<typename TShaderObject>
    void attach(TShaderObject *pShader);

    /**
     * @brief detach a shader from the program
     *
     * @param[in] pShader		Pointer to shader object
     *
     * @return No return.
     */
    template<typename TShaderObject>
    void detach(TShaderObject *pShader);

    /**
     * @brief link a program
     *
     *
     * @return true if succesfuly linked.
     */
    bool link();

    /**
     * @brief set a shader type
     *
     * @param[in] type		shader type ( graphics or compute )
     *
     * @return No return.
     */
    void setProgramType(ProgramType type) { m_programType = type; }
    ProgramType getProgramType() const { return m_programType; }

    /**
     * @brief Get Error Message
     *
     *
     * @return error message.
     */
    QByteArray getErrorMessage();

    /**
     * @brief Query constant class
     *
     * @param[in] constantName	Pointer to constant name
     *
     * @return return a pointer to a constant class.
     */
    QDemonRef<QDemonRenderShaderConstantBase> getShaderConstant(const char *constantName);

    /**
     * @brief Query a shader buffer (constant, ... )
     *
     * @param[in] bufferName	Pointer to constant name
     *
     * @return return a pointer to a constant class.
     */
    QDemonRef<QDemonRenderShaderBufferBase> getShaderBuffer(const char *bufferName);

    // concrete set functions
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, qint32 inValue, const qint32 inCount);
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, const qint32_2 &inValue, const qint32 inCount);
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, const qint32_3 &inValue, const qint32 inCount);
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, const qint32_4 &inValue, const qint32 inCount);
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, bool inValue, const qint32 inCount);
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, const bool_2 &inValue, const qint32 inCount);
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, const bool_3 &inValue, const qint32 inCount);
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, const bool_4 &inValue, const qint32 inCount);
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, const float &inValue, const qint32 inCount);
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, const QVector2D &inValue, const qint32 inCount);
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, const QVector3D &inValue, const qint32 inCount);
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, const QVector4D &inValue, const qint32 inCount);
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, const quint32 &inValue, const qint32 inCount);
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, const quint32_2 &inValue, const qint32 inCount);
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, const quint32_3 &inValue, const qint32 inCount);
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, const quint32_4 &inValue, const qint32 inCount);
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, const QMatrix3x3 &inValue, const qint32 inCount, bool inTranspose = false);
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, const QMatrix4x4 &inValue, const qint32 inCount, bool inTranspose = false);
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, const QDemonConstDataRef<QMatrix4x4> inValue, const qint32 inCount);
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, QDemonRenderTexture2D *inValue, const qint32 inCount);
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, QDemonRenderTexture2D **inValue, const qint32 inCount);
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, QDemonRenderTexture2DArray *inValue, const qint32 inCount);
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, QDemonRenderTextureCube *inValue, const qint32 inCount);
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, QDemonRenderTextureCube **inValue, const qint32 inCount);
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, QDemonRenderImage2D *inValue, const qint32 inCount);
    void setConstantValue(QDemonRenderShaderConstantBase *inConstant, QDemonRenderDataBuffer *inValue, const qint32 inCount);

    /**
     * @brief Template to set constant value via name
     *
     * @param[in] inConstantName	Pointer to constant name
     * @param[in] inValue			Pointer to data
     * @param[in] inCount			Number of elements (array count)
     *
     * @return return a pointer to a constant class.
     */
    template<typename TDataType>
    void setPropertyValue(const char *inConstantName, const TDataType &inValue, const qint32 inCount = 1)
    {
        QDemonRef<QDemonRenderShaderConstantBase> theConstant = getShaderConstant(inConstantName);

        if (theConstant) {
            if (theConstant->getShaderConstantType() == QDemonDataTypeToShaderDataTypeMap<TDataType>::getType()) {
                setConstantValue(theConstant.data(), inValue, inCount);
            } else {
                // Types don't match or property not found
                Q_ASSERT(false);
            }
        }
    }

    /**
     * @brief Template to set constant value shader constant object
     *
     * @param[in] inConstant	Pointer shader constant object
     * @param[in] inValue		Pointer to data
     * @param[in] inCount		Number of elements (array count)
     *
     * @return return a pointer to a constant class.
     */
    template<typename TDataType>
    void setPropertyValue(QDemonRenderShaderConstantBase *inConstant, const TDataType &inValue, const qint32 inCount = 1)
    {
        if (inConstant) {
            if (inConstant->getShaderConstantType() == QDemonDataTypeToShaderDataTypeMap<TDataType>::getType()) {
                setConstantValue(inConstant, inValue, inCount);
            } else {
                // Types don't match or property not found
                Q_ASSERT(false);
            }
        }
    }

    virtual void bindComputeInput(QDemonRenderDataBuffer *inBuffer, quint32 inIndex);

    /**
     * @brief get the backend object handle
     *
     * @return the backend object handle.
     */
    QDemonRenderBackend::QDemonRenderBackendShaderProgramObject getShaderProgramHandle() const
    {
        return m_programHandle;
    }

    /**
     * @brief get the context object
     *
     * @return context which this shader belongs to.
     */
    QDemonRef<QDemonRenderContextImpl> getRenderContext();

    /**
     * @brief Create a shader program
     *
     * @param[in] context						Pointer to context
     * @param[in] programName					Name of the program
     * @param[in] vertShaderSource				Vertex shader source code
     * @param[in] fragShaderSource				Fragment shader source code
     * @param[in] tessControlShaderSource		tessellation control shader source code
     * @param[in] tessEvaluationShaderSource	tessellation evaluation shader source code
     * @param[in] separateProgram				True if this will we a separate
     * program
     * @param[in] type							Binary program type
     * @param[in] binaryProgram					True if program is binary
     *
     * @return a render result
     */
    static QDemonRenderVertFragCompilationResult create(
            const QDemonRef<QDemonRenderContextImpl> &context,
            const char *programName,
            QDemonConstDataRef<qint8> vertShaderSource,
            QDemonConstDataRef<qint8> fragShaderSource,
            QDemonConstDataRef<qint8> tessControlShaderSource = QDemonConstDataRef<qint8>(),
            QDemonConstDataRef<qint8> tessEvaluationShaderSource = QDemonConstDataRef<qint8>(),
            QDemonConstDataRef<qint8> geometryShaderSource = QDemonConstDataRef<qint8>(),
            bool separateProgram = false,
            QDemonRenderShaderProgramBinaryType type = QDemonRenderShaderProgramBinaryType::Unknown,
            bool binaryProgram = false);

    /**
     * @brief Create a compute shader program
     *
     * @param[in] context						Pointer to context
     * @param[in] programName					Name of the program
     * @param[in] computeShaderSource			Compute shader source code
     *
     * @return a render result
     */
    static QDemonRenderVertFragCompilationResult createCompute(const QDemonRef<QDemonRenderContextImpl> &context,
                                                               const char *programName,
                                                               QDemonConstDataRef<qint8> computeShaderSource);
};

// Helper class to cache the lookup of shader properties and apply them quickly in a typesafe
// way.
template<typename TDataType>
struct QDemonRenderCachedShaderProperty
{
    QDemonRef<QDemonRenderShaderProgram> shader; ///< pointer to shader program
    QDemonRef<QDemonRenderShaderConstantBase> constant; ///< poiner to shader constant object

    QDemonRenderCachedShaderProperty(const QByteArray &inConstantName, QDemonRef<QDemonRenderShaderProgram> inShader)
        : QDemonRenderCachedShaderProperty(inConstantName.constData(), inShader)
    {
    }

    QDemonRenderCachedShaderProperty(const char *inConstantName, QDemonRef<QDemonRenderShaderProgram> inShader)
        : shader(inShader), constant(nullptr)
    {
        QDemonRef<QDemonRenderShaderConstantBase> theConstant = inShader->getShaderConstant(inConstantName);
        if (theConstant) {
            if (theConstant->getShaderConstantType() == QDemonDataTypeToShaderDataTypeMap<TDataType>::getType()) {
                constant = theConstant;
            } else {
                // Property types do not match, this probably indicates that the shader changed
                // while the
                // code creating this object did not change.
                Q_ASSERT(false);
            }
        }
    }

    QDemonRenderCachedShaderProperty() = default;

    void set(const TDataType &inValue)
    {
        // TODO: Make sure the raw pointer her is ok
        if (constant)
            shader->setPropertyValue(constant.data(), inValue);
    }

    bool isValid() const { return constant != nullptr; }
};

template<typename TDataType, int size>
struct QDemonRenderCachedShaderPropertyArray
{
    QDemonRef<QDemonRenderShaderProgram> shader; ///< pointer to shader program
    QDemonRef<QDemonRenderShaderConstantBase> constant; ///< poiner to shader constant object
    TDataType m_array[size];

    QDemonRenderCachedShaderPropertyArray(const QString &inConstantName, QDemonRef<QDemonRenderShaderProgram> inShader)
        : QDemonRenderCachedShaderPropertyArray(qPrintable(inConstantName), inShader)
    {
    }

    QDemonRenderCachedShaderPropertyArray(const char *inConstantName, QDemonRef<QDemonRenderShaderProgram> inShader)
        : shader(inShader), constant(nullptr)
    {
        memset(m_array, 0, sizeof(m_array));
        QDemonRef<QDemonRenderShaderConstantBase> theConstant = inShader->getShaderConstant(inConstantName);
        if (theConstant) {
            if (theConstant->m_elementCount > 1 && theConstant->m_elementCount <= size
                && theConstant->getShaderConstantType() == QDemonDataTypeToShaderDataTypeMap<TDataType *>::getType()) {
                constant = theConstant;
            } else {
                // Property types do not match, this probably indicates that the shader changed
                // while the code creating this object did not change.
                Q_ASSERT(false);
            }
        }
    }

    QDemonRenderCachedShaderPropertyArray() : shader(nullptr), constant(nullptr)
    {
        memset(m_array, 0, sizeof(m_array));
    }

    void set(int count)
    {
        if (constant)
            shader->setPropertyValue(constant.data(), static_cast<TDataType *>(m_array), qMin(size, count));
    }

    bool isValid() const { return constant != nullptr; }
};

// Helper class to cache the lookup of shader properties and apply them quickly in a typesafe
// way.
template<typename TDataType>
struct QDemonRenderCachedShaderBuffer
{
    QDemonRef<QDemonRenderShaderProgram> shader; ///< pointer to shader program
    QDemonRef<TDataType> shaderBuffer; ///< poiner to shader buffer object

    QDemonRenderCachedShaderBuffer(const char *inShaderBufferName, QDemonRef<QDemonRenderShaderProgram> inShader)
        : shader(inShader), shaderBuffer(nullptr)
    {
        QDemonRef<TDataType> theShaderBuffer = static_cast<TDataType *>(inShader->getShaderBuffer(inShaderBufferName).get());
        if (theShaderBuffer) {
            shaderBuffer = theShaderBuffer;
        }
    }
    QDemonRenderCachedShaderBuffer() : shader(nullptr), shaderBuffer(nullptr) {}

    void set()
    {
        if (shaderBuffer) {
            shaderBuffer->validate(shader);
            shaderBuffer->update();
            shaderBuffer->bindToProgram(shader);
        }
    }

    bool isValid() const { return shaderBuffer != 0; }
};

QT_END_NAMESPACE

#endif
