// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qsvganimator_p.h"
#include <QtCore/qdatetime.h>
#include <QtSvg/private/qsvganimate_p.h>

QT_BEGIN_NAMESPACE

static QColor sumColor(const QColor &c1, const QColor &c2)
{
    QRgb rgb1 = c1.rgba();
    QRgb rgb2 = c2.rgba();
    int sumRed = qRed(rgb1) + qRed(rgb2);
    int sumGreen = qGreen(rgb1) + qGreen(rgb2);
    int sumBlue = qBlue(rgb1) + qBlue(rgb2);

    QRgb sumRgb = qRgba(qBound(0, sumRed, 255),
                        qBound(0, sumGreen, 255),
                        qBound(0, sumBlue, 255),
                        255);

    return QColor(sumRgb);
}

QSvgAbstractAnimator::QSvgAbstractAnimator()
    : m_time(0)
    , m_animationDuration(0)
{
}

QSvgAbstractAnimator::~QSvgAbstractAnimator()
{
    for (auto itr = m_animationsCSS.begin(); itr != m_animationsCSS.end(); itr++) {
        QList<QSvgAbstractAnimation *> &nodeAnimations = itr.value();
        for (QSvgAbstractAnimation *anim : nodeAnimations)
            delete anim;
    }
}


void QSvgAbstractAnimator::appendAnimation(const QSvgNode *node, QSvgAbstractAnimation *anim)
{
    if (!node)
        return;

    if (anim->animationType() == QSvgAbstractAnimation::SMIL)
        m_animationsSMIL[node].append(anim);
    else
        m_animationsCSS[node].append(anim);
}

QList<QSvgAbstractAnimation *> QSvgAbstractAnimator::animationsForNode(const QSvgNode *node) const
{
    return combinedAnimationsForNode(node);
}

void QSvgAbstractAnimator::advanceAnimations()
{
    qreal elapsedTime = currentElapsed();
    for (auto itr = m_animationsCSS.begin(); itr != m_animationsCSS.end(); itr++) {
        QList<QSvgAbstractAnimation *> &nodeAnimations = itr.value();
        for (QSvgAbstractAnimation *anim : nodeAnimations) {
            if (!anim->finished())
                anim->evaluateAnimation(elapsedTime);
        }
    }

    for (auto itr = m_animationsSMIL.begin(); itr != m_animationsSMIL.end(); itr++) {
        QList<QSvgAbstractAnimation *> &nodeAnimations = itr.value();
        for (QSvgAbstractAnimation *anim : nodeAnimations) {
            if (!anim->finished())
                anim->evaluateAnimation(elapsedTime);
        }
    }
}

void QSvgAbstractAnimator::setAnimationDuration(qint64 dur)
{
    m_animationDuration = dur;
}

qint64 QSvgAbstractAnimator::animationDuration() const
{
    return m_animationDuration;
}

void QSvgAbstractAnimator::applyAnimationsOnNode(const QSvgNode *node, QPainter *p)
{
    QList<QSvgAbstractAnimation *> nodeAnims = combinedAnimationsForNode(node);

    if (!node || nodeAnims.size() == 0)
        return;

    QTransform transformToNode = p->worldTransform();
    if (node->style().transform)
        transformToNode = node->style().transform->qtransform().inverted() * transformToNode;

    for (auto anim : nodeAnims) {
        if (!anim->isActive())
            continue;

        bool replace = anim->animationType() == QSvgAbstractAnimation::CSS ? true :
                           (static_cast<QSvgAnimateNode *>(anim))->additiveType() == QSvgAnimateNode::Replace;
        QList<QSvgAbstractAnimatedProperty *> props = anim->properties();
        for (auto prop : props) {
            switch (prop->type()) {
            case QSvgAbstractAnimatedProperty::Color:
                if (prop->propertyName() == QLatin1String("fill")) {
                    QBrush brush = p->brush();
                    QColor brushColor = brush.color();
                    QColor animatedColor = prop->interpolatedValue().value<QColor>();
                    brush.setColor(replace == true ? animatedColor : sumColor(brushColor, animatedColor));
                    p->setBrush(brush);
                } else if (prop->propertyName() == QLatin1String("stroke")) {
                    QPen pen =  p->pen();
                    QBrush penBrush = pen.brush();
                    QColor penColor = penBrush.color();
                    QColor animatedColor = prop->interpolatedValue().value<QColor>();
                    penBrush.setColor(replace == true ? animatedColor : sumColor(penColor, animatedColor));
                    pen.setBrush(penBrush);
                    p->setPen(pen);
                }
                break;
            case QSvgAbstractAnimatedProperty::Transform:
                if (replace)
                    p->setWorldTransform(prop->interpolatedValue().value<QTransform>() * transformToNode);
                else
                    p->setWorldTransform(prop->interpolatedValue().value<QTransform>() * p->worldTransform());
                break;
            default:
                break;
            }
        }
    }
}

QList<QSvgAbstractAnimation *> QSvgAbstractAnimator::combinedAnimationsForNode(const QSvgNode *node) const
{
    if (!node)
        return QList<QSvgAbstractAnimation *>();

    return m_animationsSMIL.value(node) + m_animationsCSS.value(node);
}

QSvgAnimator::QSvgAnimator()
{
}

QSvgAnimator::~QSvgAnimator()
{
}

void QSvgAnimator::restartAnimation()
{
    m_time = QDateTime::currentMSecsSinceEpoch();
}

qint64 QSvgAnimator::currentElapsed()
{
    return QDateTime::currentMSecsSinceEpoch() - m_time;
}

void QSvgAnimator::setAnimatorTime(qint64 time)
{
    m_time += time;
}

QSvgAnimationController::QSvgAnimationController()
{
}

QSvgAnimationController::~QSvgAnimationController()
{
}

void QSvgAnimationController::restartAnimation()
{
    m_time = 0;
}

qint64 QSvgAnimationController::currentElapsed()
{
    return m_time;
}

void QSvgAnimationController::setAnimatorTime(qint64 time)
{
    m_time = qMax(0, time);
}

QT_END_NAMESPACE
