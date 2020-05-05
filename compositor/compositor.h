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

#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandView>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class Window;
class QOpenGLTexture;
class QWaylandIviApplication;
class QWaylandIviSurface;

class View : public QWaylandView
{
    Q_OBJECT
public:
    explicit View(int iviId) : m_iviId(iviId) {}
    QOpenGLTexture *getTexture();
    int iviId() const { return m_iviId; }

    QRect globalGeometry() const { return QRect(globalPosition(), surface()->size()); }
    void setGlobalPosition(const QPoint &globalPos) { m_pos = globalPos; m_positionSet = true; }
    QPoint globalPosition() const { return m_pos; }
    QPoint mapToLocal(const QPoint &globalPos) const;
    QSize size() const { return surface() ? surface()->size() : QSize(); }

    void initPosition(const QSize &screenSize, const QSize &surfaceSize);

private:
    friend class Compositor;
    QOpenGLTexture *m_texture = nullptr;
    bool m_positionSet = false;
    QPoint m_pos;
    int m_iviId;
};

class Compositor : public QWaylandCompositor
{
    Q_OBJECT
public:
    Compositor(Window *window);
    ~Compositor() override;
    void create() override;

    QList<View*> views() const { return m_views; }
    View *viewAt(const QPoint &position);
    void raise(View *view);

    void handleMousePress(const QPoint &position, Qt::MouseButton button);
    void handleMouseRelease(const QPoint &position, Qt::MouseButton button, Qt::MouseButtons buttons);
    void handleMouseMove(const QPoint &position);
    void handleMouseWheel(Qt::Orientation orientation, int delta);

    void handleKeyPress(quint32 nativeScanCode);
    void handleKeyRelease(quint32 nativeScanCode);

    void startRender();
    void endRender();

private slots:
    void onIviSurfaceCreated(QWaylandIviSurface *iviSurface);
    void onSurfaceDestroyed();
    void triggerRender();

    void viewSurfaceDestroyed();
private:
    Window *m_window = nullptr;
    QWaylandIviApplication *m_iviApplication = nullptr;
    QList<View*> m_views;
    QPointer<View> m_mouseView;
};

QT_END_NAMESPACE

#endif // COMPOSITOR_H
