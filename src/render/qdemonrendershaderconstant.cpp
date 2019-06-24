#include "qdemonrendershaderconstant.h"
#include <QtDemonRender/QDemonRenderShaderProgram>
#include <QtDemonRender/qdemonrendercontext.h>

QT_BEGIN_NAMESPACE

void QDemonRenderShaderConstantBuffer::validate(const QDemonRef<QDemonRenderShaderProgram> &inShader)
{
    // A constant buffer might not be set at first call
    // due to the fact that they are compiled from a cache file
    // Now it must exists.
    if (m_constBuffer)
        return;

    const QDemonRef<QDemonRenderConstantBuffer> &cb = m_context->getConstantBuffer(m_name);
    if (cb) {
        cb->setupBuffer(inShader.data(), m_location, m_size, m_paramCount);
        // cb->addRef();
        m_constBuffer = cb;
    } else {
        Q_ASSERT(false);
    }
}

void QDemonRenderShaderConstantBuffer::bindToProgram(const QDemonRef<QDemonRenderShaderProgram> &inShader)
{
    if (m_constBuffer)
        m_constBuffer->bindToShaderProgram(inShader, m_location, m_binding);
}

void QDemonRenderShaderStorageBuffer::validate(const QDemonRef<QDemonRenderShaderProgram> &)
{
    // A constant buffer might not be set at first call
    // due to the fact that they are compile from a cache file
    // Now it must exists.
    if (m_storageBuffer)
        return;

    const QDemonRef<QDemonRenderStorageBuffer> &sb = m_context->getStorageBuffer(m_name);
    if (sb) {
        m_storageBuffer = sb;
    } else {
        Q_ASSERT(false);
    }
}

void QDemonRenderShaderStorageBuffer::bindToProgram(const QDemonRef<QDemonRenderShaderProgram> &)
{
    if (m_storageBuffer)
        m_storageBuffer->bindToShaderProgram(m_location);
}

void QDemonRenderShaderAtomicCounterBuffer::validate(const QDemonRef<QDemonRenderShaderProgram> &)
{
    // A constant buffer might not be set at first call
    // due to the fact that they are compile from a cache file
    // Now it must exists.
    if (m_atomicCounterBuffer)
        return;

    const QDemonRef<QDemonRenderAtomicCounterBuffer> &acb = m_context->getAtomicCounterBuffer(m_name);
    if (Q_LIKELY(acb))
        m_atomicCounterBuffer = acb;
    else
        Q_ASSERT(false);
}

void QDemonRenderShaderAtomicCounterBuffer::bindToProgram(const QDemonRef<QDemonRenderShaderProgram> &)
{
    if (m_atomicCounterBuffer)
        m_atomicCounterBuffer->bindToShaderProgram(m_location);
}

QT_END_NAMESPACE
