/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Wayland module
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "window.h"
#include "compositor.h"

#include <QPainter>
#include <QMatrix4x4>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QMouseEvent>

Window::Window()
{
}

void Window::setCompositor(Compositor *comp) {
    m_compositor = comp;
}

void Window::initializeGL()
{
    m_textureBlitter.create();
}

void Window::paintGL()
{
    m_compositor->startRender();

    QOpenGLFunctions *functions = context()->functions();
    functions->glClearColor(.4f, .7f, .1f, 0.5f);
    functions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLenum currentTarget = GL_TEXTURE_2D;
    m_textureBlitter.bind(currentTarget);
    functions->glEnable(GL_BLEND);
    functions->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const auto views = m_compositor->views();
    for (View *view : views) {
        auto texture = view->getTexture();
        if (!texture)
            continue;
        if (texture->target() != currentTarget) {
            currentTarget = texture->target();
            m_textureBlitter.bind(currentTarget);
        }
        GLuint textureId = texture->textureId();
        QWaylandSurface *surface = view->surface();
        if (surface && surface->hasContent()) {
            QSize s = surface->size();
            view->initPosition(size(), s);
            QPointF pos = view->globalPosition();
            QRectF surfaceGeometry(pos, s);
            QOpenGLTextureBlitter::Origin surfaceOrigin =
                    view->currentBuffer().origin() == QWaylandSurface::OriginTopLeft
                    ? QOpenGLTextureBlitter::OriginTopLeft
                    : QOpenGLTextureBlitter::OriginBottomLeft;
            QMatrix4x4 targetTransform = QOpenGLTextureBlitter::targetTransform(surfaceGeometry, QRect(QPoint(), size()));
            m_textureBlitter.blit(textureId, targetTransform, surfaceOrigin);
        }
    }
    m_textureBlitter.release();
    m_compositor->endRender();
}

void Window::mousePressEvent(QMouseEvent *event)
{
    m_compositor->handleMousePress(event->localPos().toPoint(), event->button());
}

void Window::mouseReleaseEvent(QMouseEvent *event)
{
    m_compositor->handleMouseRelease(event->localPos().toPoint(), event->button(), event->buttons());
}

void Window::mouseMoveEvent(QMouseEvent *event)
{
    m_compositor->handleMouseMove(event->localPos().toPoint());
}

void Window::wheelEvent(QWheelEvent *event)
{
    m_compositor->handleMouseWheel(event->orientation(), event->delta());
}

void Window::keyPressEvent(QKeyEvent *e)
{
    m_compositor->handleKeyPress(e->nativeScanCode());
}

void Window::keyReleaseEvent(QKeyEvent *e)
{
    m_compositor->handleKeyRelease(e->nativeScanCode());
}
