/****************************************************************************
**
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

#ifndef QDEMON_RENDER_LIGHT_CONSTANT_PROPERTIES
#define QDEMON_RENDER_LIGHT_CONSTANT_PROPERTIES

#include <QtDemonRender/qdemonrendershaderprogram.h>

QT_BEGIN_NAMESPACE


template<typename GeneratedShader>
struct QDemonLightConstantProperties
{
    struct LightConstants
    {
        QDemonRenderCachedShaderProperty<QVector4D> position;
        QDemonRenderCachedShaderProperty<QVector4D> direction;
        QDemonRenderCachedShaderProperty<QVector4D> up;
        QDemonRenderCachedShaderProperty<QVector4D> right;
        QDemonRenderCachedShaderProperty<QVector4D> diffuse;
        QDemonRenderCachedShaderProperty<QVector4D> ambient;
        QDemonRenderCachedShaderProperty<QVector4D> specular;
        QDemonRenderCachedShaderProperty<float> spotExponent;
        QDemonRenderCachedShaderProperty<float> spotCutoff;
        QDemonRenderCachedShaderProperty<float> constantAttenuation;
        QDemonRenderCachedShaderProperty<float> linearAttenuation;
        QDemonRenderCachedShaderProperty<float> quadraticAttenuation;
        QDemonRenderCachedShaderProperty<float> range;
        QDemonRenderCachedShaderProperty<float> width;
        QDemonRenderCachedShaderProperty<float> height;
        QDemonRenderCachedShaderProperty<QVector4D> shadowControls;
        QDemonRenderCachedShaderProperty<QMatrix4x4> shadowView;
        QDemonRenderCachedShaderProperty<qint32> shadowIdx;
        QDemonRenderCachedShaderProperty<QVector3D> attenuation;

        static const char *lconstantnames[];

        LightConstants(const QByteArray &lightRef, QDemonRef<QDemonRenderShaderProgram> shader)
            : position(lightRef + lconstantnames[0], shader)
            , direction(lightRef + lconstantnames[1], shader)
            , up(lightRef + lconstantnames[2], shader)
            , right(lightRef + lconstantnames[3], shader)
            , diffuse(lightRef + lconstantnames[4], shader)
            , ambient(lightRef + lconstantnames[5], shader)
            , specular(lightRef + lconstantnames[6], shader)
            , spotExponent(lightRef + lconstantnames[7], shader)
            , spotCutoff(lightRef + lconstantnames[8], shader)
            , constantAttenuation(lightRef + lconstantnames[9], shader)
            , linearAttenuation(lightRef + lconstantnames[10], shader)
            , quadraticAttenuation(lightRef + lconstantnames[11], shader)
            , range(lightRef + lconstantnames[12], shader)
            , width(lightRef + lconstantnames[13], shader)
            , height(lightRef + lconstantnames[14], shader)
            , shadowControls(lightRef + lconstantnames[15], shader)
            , shadowView(lightRef + lconstantnames[16], shader)
            , shadowIdx(lightRef + lconstantnames[17], shader)
            , attenuation(lightRef + lconstantnames[18], shader)
        {
        }

        template<typename LightProps>
        void updateLights(LightProps &props)
        {
            position.set(props.position);
            direction.set(props.direction);
            up.set(props.up);
            right.set(props.right);
            diffuse.set(props.diffuse);
            ambient.set(props.ambient);
            specular.set(props.specular);
            spotExponent.set(props.spotExponent);
            spotCutoff.set(props.spotCutoff);
            constantAttenuation.set(props.constantAttenuation);
            linearAttenuation.set(props.linearAttenuation);
            quadraticAttenuation.set(props.quadraticAttenuation);
            range.set(props.range);
            width.set(props.width);
            height.set(props.height);
            shadowControls.set(props.shadowControls);
            shadowView.set(QMatrix4x4(props.shadowView));
            shadowIdx.set(props.shadowIdx);
            attenuation.set(QVector3D(props.constantAttenuation, props.linearAttenuation, props.quadraticAttenuation));
        }
    };

    QDemonLightConstantProperties(GeneratedShader *shader, bool packed) : m_lightCount("uNumLights", shader->m_shader)
    {
        m_constants.resize(shader->m_lights.size());
        for (int i = 0; i < shader->m_lights.size(); ++i) {
            QByteArray lref;
            if (packed) {
                lref += "light_";
                lref += QByteArray::number(i);
                lref += "_";
            } else {
                lref += "lights[";
                lref += QByteArray::number(i);
                lref += "].";
            }
            m_constants[i] = new LightConstants(lref, shader->m_shader);
        }
        m_lightCount.set(shader->m_lights.size());
        m_lightCountInt = shader->m_lights.size();
    }

    QDemonLightConstantProperties(const QByteArray &lseed, const QByteArray &lcount, GeneratedShader *shader, bool packed, int count)
        : m_lightCount(lcount, shader->m_shader)
    {
        m_constants.resize(count);
        for (int i = 0; i < count; ++i) {
            QByteArray lref = lseed;
            if (packed) {
                lref += "_";
                lref += QByteArray::number(i);
                lref += "_";
            } else {
                lref += "[";
                lref += QByteArray::number(i);
                lref += "].";
            }
            m_constants[i] = new LightConstants(lref, shader->m_shader);
        }
        m_lightCount.set(count);
        m_lightCountInt = count;
    }

    ~QDemonLightConstantProperties() { qDeleteAll(m_constants); }

    void updateLights(const QDemonRef<GeneratedShader> &shader)
    {
        for (int i = 0; i < m_constants.size(); ++i)
            m_constants[i]->updateLights(shader->m_lights[i].lightData);
    }
    template<typename LightProps>
    void updateLights(const QVector<QDemonRef<LightProps>> &props)
    {
        for (int i = 0; i < m_constants.size(); ++i)
            m_constants[i]->updateLights(props[i]->m_lightData);
    }

    QVector<LightConstants *> m_constants;
    QDemonRenderCachedShaderProperty<qint32> m_lightCount;
    int m_lightCountInt;
};


template<typename GeneratedShader>
const char * QDemonLightConstantProperties<GeneratedShader>::LightConstants::lconstantnames[] = { "position",
                                        "direction",
                                        "up",
                                        "right",
                                        "diffuse",
                                        "ambient",
                                        "specular",
                                        "spotExponent",
                                        "spotCutoff",
                                        "constantAttenuation",
                                        "linearAttenuation",
                                        "quadraticAttenuation",
                                        "range",
                                        "width",
                                        "height",
                                        "shadowControls",
                                        "shadowView",
                                        "shadowIdx",
                                        "attenuation" };

QT_END_NAMESPACE

#endif
