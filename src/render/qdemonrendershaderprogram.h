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

typedef QHash<QString, QDemonRenderShaderConstantBase *> TShaderConstantMap;
typedef QHash<QString, QDemonRenderShaderBufferBase *> TShaderBufferMap;

///< A shader program is an object composed of a multiple shaders (vertex, fragment,
///geometry,....)
class Q_DEMONRENDER_EXPORT QDemonRenderShaderProgram
{
public:
    struct ProgramType
    {
        enum Enum { Graphics, Compute };
    };

private:
    QDemonRenderContextImpl &m_Context; ///< pointer to context
    QSharedPointer<QDemonRenderBackend> m_Backend; ///< pointer to backend
    const char *m_ProgramName; /// Name of the program
    QDemonRenderBackend::QDemonRenderBackendShaderProgramObject m_ProgramHandle; ///< opaque backend handle
    TShaderConstantMap m_Constants; ///< map of shader constants
    TShaderBufferMap m_ShaderBuffers; ///< map of shader buffers
    ProgramType::Enum m_ProgramType; ///< shader type
    QString m_ErrorMessage; ///< contains the error message if linking fails

    /**
         * @brief create vertex shader
         *
         * @param[in] context					Pointer to render context
         * @param[in] vertexShaderSource		Fragment shader source code
         * @param[in] binaryProgram				True if binary program
         *
         * @return pointer to vertex shader object
         */
    static QDemonOption<QDemonRenderVertexShader *>
    createVertexShader(QDemonRenderContextImpl &context, QDemonConstDataRef<qint8> vertexShaderSource,
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
    static QDemonOption<QDemonRenderFragmentShader *>
    createFragmentShader(QDemonRenderContextImpl &context,
                         QDemonConstDataRef<qint8> fragmentShaderSource, bool binaryProgram = false);

    /**
         * @brief create tesselation control shader
         *
         * @param[in] context					Pointer to render context
         * @param[in] tessControlShaderSource	Tessellation control shader source code
         * @param[in] binaryProgram				True if binary program
         *
         * @return pointer to tessellation control shader
         */
    static QDemonOption<QDemonRenderTessControlShader *>
    createTessControlShader(QDemonRenderContextImpl &context,
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
    static QDemonOption<QDemonRenderTessEvaluationShader *>
    createTessEvaluationShader(QDemonRenderContextImpl &context,
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
    static QDemonOption<QDemonRenderGeometryShader *>
    createGeometryShader(QDemonRenderContextImpl &context,
                         QDemonConstDataRef<qint8> geometryShaderSource, bool binaryProgram = false);

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
    QDemonRenderShaderProgram(QDemonRenderContextImpl &context,
                              const char *programName, bool separableProgram);

    /// destructor
    ~QDemonRenderShaderProgram();

    /**
         * @brief attach a shader to the program
         *
         * @param[in] pShader		Pointer to shader object
         *
         * @return No return.
         */
    template <typename TShaderObject>
    void Attach(TShaderObject *pShader);

    /**
         * @brief detach a shader from the program
         *
         * @param[in] pShader		Pointer to shader object
         *
         * @return No return.
         */
    template <typename TShaderObject>
    void Detach(TShaderObject *pShader);

    /**
         * @brief link a program
         *
         *
         * @return true if succesfuly linked.
         */
    bool Link();

    /**
         * @brief set a shader type
         *
         * @param[in] type		shader type ( graphics or compute )
         *
         * @return No return.
         */
    void SetProgramType(ProgramType::Enum type) { m_ProgramType = type; }
    ProgramType::Enum GetProgramType() const { return m_ProgramType; }

    /**
         * @brief Get Error Message
         *
         * @param[out] messageLength	Pointer to error string
         * @param[out] messageLength	Size of error meesage
         *
         * @return no return.
         */
    void GetErrorMessage(qint32 *messageLength, const char *errorMessage);

    /**
         * @brief Get Error Message
         *
         *
         * @return error message.
         */
    const char *GetErrorMessage();

    /**
         * @brief Query constant class
         *
         * @param[in] constantName	Pointer to constant name
         *
         * @return return a pointer to a constant class.
         */
    QDemonRenderShaderConstantBase *GetShaderConstant(const char *constantName);

    /**
         * @brief Query a shader buffer (constant, ... )
         *
         * @param[in] bufferName	Pointer to constant name
         *
         * @return return a pointer to a constant class.
         */
    QDemonRenderShaderBufferBase *GetShaderBuffer(const char *bufferName);

    // concrete set functions
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, qint32 inValue, const qint32 inCount);
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, const qint32_2 &inValue, const qint32 inCount);
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, const qint32_3 &inValue, const qint32 inCount);
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, const qint32_4 &inValue, const qint32 inCount);
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, bool inValue, const qint32 inCount);
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, const bool_2 &inValue, const qint32 inCount);
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, const bool_3 &inValue, const qint32 inCount);
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, const bool_4 &inValue, const qint32 inCount);
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, const float &inValue, const qint32 inCount);
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, const QVector2D &inValue, const qint32 inCount);
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, const QVector3D &inValue, const qint32 inCount);
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, const QVector4D &inValue, const qint32 inCount);
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, const quint32 &inValue, const qint32 inCount);
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, const quint32_2 &inValue, const qint32 inCount);
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, const quint32_3 &inValue, const qint32 inCount);
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, const quint32_4 &inValue, const qint32 inCount);
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, const QMatrix3x3 &inValue, const qint32 inCount, bool inTranspose = false);
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, const QMatrix4x4 &inValue, const qint32 inCount, bool inTranspose = false);
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, const QDemonConstDataRef<QMatrix4x4> inValue, const qint32 inCount);
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, QDemonRenderTexture2D *inValue, const qint32 inCount);
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, QDemonRenderTexture2D **inValue, const qint32 inCount);
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, QDemonRenderTexture2DArray *inValue, const qint32 inCount);
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, QDemonRenderTextureCube *inValue, const qint32 inCount);
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, QDemonRenderTextureCube **inValue, const qint32 inCount);
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, QDemonRenderImage2D *inValue, const qint32 inCount);
    void SetConstantValue(QDemonRenderShaderConstantBase *inConstant, QDemonRenderDataBuffer *inValue, const qint32 inCount);

    /**
         * @brief Template to set constant value via name
         *
         * @param[in] inConstantName	Pointer to constant name
         * @param[in] inValue			Pointer to data
         * @param[in] inCount			Number of elements (array count)
         *
         * @return return a pointer to a constant class.
         */
    template <typename TDataType>
    void SetPropertyValue(const char *inConstantName, const TDataType &inValue,
                          const qint32 inCount = 1)
    {
        QDemonRenderShaderConstantBase *theConstant = GetShaderConstant(inConstantName);

        if (theConstant) {
            if (theConstant->GetShaderConstantType()
                    == QDemonDataTypeToShaderDataTypeMap<TDataType>::GetType()) {
                SetConstantValue(theConstant, inValue, inCount);
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
    template <typename TDataType>
    void SetPropertyValue(QDemonRenderShaderConstantBase *inConstant, const TDataType &inValue,
                          const qint32 inCount = 1)
    {
        if (inConstant) {
            if (inConstant->GetShaderConstantType()
                    == QDemonDataTypeToShaderDataTypeMap<TDataType>::GetType()) {
                SetConstantValue(inConstant, inValue, inCount);
            } else {
                // Types don't match or property not found
                Q_ASSERT(false);
            }
        }
    }

    virtual void BindComputeInput(QDemonRenderDataBuffer *inBuffer, quint32 inIndex);

    /**
         * @brief get the backend object handle
         *
         * @return the backend object handle.
         */
    QDemonRenderBackend::QDemonRenderBackendShaderProgramObject GetShaderProgramHandle() const
    {
        return m_ProgramHandle;
    }

    /**
         * @brief get the context object
         *
         * @return context which this shader belongs to.
         */
    QDemonRenderContextImpl &GetRenderContext();

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
    static QDemonRenderVertFragCompilationResult Create(
            QDemonRenderContextImpl &context, const char *programName,
            QDemonConstDataRef<qint8> vertShaderSource, QDemonConstDataRef<qint8> fragShaderSource,
            QDemonConstDataRef<qint8> tessControlShaderSource = QDemonConstDataRef<qint8>(),
            QDemonConstDataRef<qint8> tessEvaluationShaderSource = QDemonConstDataRef<qint8>(),
            QDemonConstDataRef<qint8> geometryShaderSource = QDemonConstDataRef<qint8>(),
            bool separateProgram = false,
            QDemonRenderShaderProgramBinaryType::Enum type = QDemonRenderShaderProgramBinaryType::Unknown,
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
    static QDemonRenderVertFragCompilationResult
    CreateCompute(QDemonRenderContextImpl &context, const char *programName,
                  QDemonConstDataRef<qint8> computeShaderSource);
};

// Helper class to cache the lookup of shader properties and apply them quickly in a typesafe
// way.
template <typename TDataType>
struct QDemonRenderCachedShaderProperty
{
    QSharedPointer<QDemonRenderShaderProgram> m_Shader; ///< pointer to shader program
    QDemonRenderShaderConstantBase *m_Constant; ///< poiner to shader constant object

    QDemonRenderCachedShaderProperty(const QString &inConstantName, QSharedPointer<QDemonRenderShaderProgram> inShader)
        : QDemonRenderCachedShaderProperty(qPrintable(inConstantName), inShader)
    {
    }

    QDemonRenderCachedShaderProperty(const char *inConstantName, QSharedPointer<QDemonRenderShaderProgram> inShader)
        : m_Shader(inShader)
        , m_Constant(nullptr)
    {
        QDemonRenderShaderConstantBase *theConstant = inShader->GetShaderConstant(inConstantName);
        if (theConstant) {
            if (theConstant->GetShaderConstantType()
                    == QDemonDataTypeToShaderDataTypeMap<TDataType>::GetType()) {
                m_Constant = theConstant;
            } else {
                // Property types do not match, this probably indicates that the shader changed
                // while the
                // code creating this object did not change.
                Q_ASSERT(false);
            }
        }
    }

    QDemonRenderCachedShaderProperty()
        : m_Shader(nullptr)
        , m_Constant(nullptr)
    {
    }

    void Set(const TDataType &inValue)
    {
        if (m_Constant)
            m_Shader->SetPropertyValue(m_Constant, inValue);
    }

    bool IsValid() const { return m_Constant != 0; }
};

template <typename TDataType, int size>
struct QDemonRenderCachedShaderPropertyArray
{
    QSharedPointer<QDemonRenderShaderProgram> m_Shader; ///< pointer to shader program
    QDemonRenderShaderConstantBase *m_Constant; ///< poiner to shader constant object
    TDataType m_array[size];

    QDemonRenderCachedShaderPropertyArray(const QString &inConstantName,
                                      QSharedPointer<QDemonRenderShaderProgram> inShader)
        : QDemonRenderCachedShaderPropertyArray(qPrintable(inConstantName), inShader)
    {

    }

    QDemonRenderCachedShaderPropertyArray(const char *inConstantName,
                                      QSharedPointer<QDemonRenderShaderProgram> inShader)
        : m_Shader(inShader)
        , m_Constant(nullptr)
    {
        memset(m_array,  0, sizeof(m_array));
        QDemonRenderShaderConstantBase *theConstant = inShader->GetShaderConstant(inConstantName);
        if (theConstant) {
            if (theConstant->m_ElementCount > 1 && theConstant->m_ElementCount <= size &&
                    theConstant->GetShaderConstantType()
                    == QDemonDataTypeToShaderDataTypeMap<TDataType*>::GetType()) {
                m_Constant = theConstant;
            } else {
                // Property types do not match, this probably indicates that the shader changed
                // while the code creating this object did not change.
                Q_ASSERT(false);
            }
        }
    }

    QDemonRenderCachedShaderPropertyArray()
        : m_Shader(nullptr)
        , m_Constant(nullptr)
    {
        memset(m_array,  0, sizeof(m_array));
    }

    void Set(int count)
    {
        if (m_Constant)
            m_Shader->SetPropertyValue(m_Constant, (TDataType*)m_array, qMin(size, count));
    }

    bool IsValid() const { return m_Constant != 0; }
};

// Helper class to cache the lookup of shader properties and apply them quickly in a typesafe
// way.
template <typename TDataType>
struct QDemonRenderCachedShaderBuffer
{
    QSharedPointer<QDemonRenderShaderProgram> m_Shader; ///< pointer to shader program
    TDataType m_ShaderBuffer; ///< poiner to shader buffer object

    QDemonRenderCachedShaderBuffer(const char *inShaderBufferName, QSharedPointer<QDemonRenderShaderProgram> inShader)
        : m_Shader(&inShader)
        , m_ShaderBuffer(nullptr)
    {
        TDataType theShaderBuffer =
                static_cast<TDataType>(inShader->GetShaderBuffer(inShaderBufferName));
        if (theShaderBuffer) {
            m_ShaderBuffer = theShaderBuffer;
        }
    }
    QDemonRenderCachedShaderBuffer()
        : m_Shader(nullptr)
        , m_ShaderBuffer(nullptr)
    {
    }

    void Set()
    {
        if (m_ShaderBuffer) {
            m_ShaderBuffer->Validate(m_Shader);
            m_ShaderBuffer->Update();
            m_ShaderBuffer->BindToProgram(m_Shader);
        }
    }

    bool IsValid() const { return m_ShaderBuffer != 0; }
};

QT_END_NAMESPACE

#endif
