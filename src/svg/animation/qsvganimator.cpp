// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qsvganimator_p.h"
#include <QtCore/qdatetime.h>

QT_BEGIN_NAMESPACE

QSvgAnimator::QSvgAnimator()
    : m_time(0)
    , m_animationDuration(0)
{
}

QSvgAnimator::~QSvgAnimator()
{
    for (auto itr = m_animationsCSS.begin(); itr != m_animationsCSS.end(); itr++) {
        QList<QSvgAbstractAnimation *> &nodeAnimations = itr.value();
        for (QSvgAbstractAnimation *anim : nodeAnimations)
            delete anim;
    }
}

void QSvgAnimator::appendAnimation(const QSvgNode *node, QSvgAbstractAnimation *anim)
{
    if (!node)
        return;

    if (anim->animationType() == QSvgAbstractAnimation::SMIL)
        m_animationsSMIL[node].append(anim);
    else
        m_animationsCSS[node].append(anim);
}

QList<QSvgAbstractAnimation *> QSvgAnimator::animationsForNode(const QSvgNode *node) const
{
    return combinedAnimationsForNode(node);
}

void QSvgAnimator::advanceAnimations()
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

void QSvgAnimator::restartAnimation()
{
    m_time = QDateTime::currentMSecsSinceEpoch();
}

qint64 QSvgAnimator::currentElapsed()
{
    return QDateTime::currentMSecsSinceEpoch() - m_time;
}

void QSvgAnimator::setAnimationDuration(qint64 dur)
{
    m_animationDuration = dur;
}

qint64 QSvgAnimator::animationDuration() const
{
    return m_animationDuration;
}

void QSvgAnimator::fastForwardAnimation(qint64 time)
{
    m_time += time;
}

void QSvgAnimator::applyAnimationsOnNode(const QSvgNode *node, QPainter *p)
{
    QList<QSvgAbstractAnimation *> nodeAnims = combinedAnimationsForNode(node);

    if (!node || nodeAnims.size() == 0)
        return;

    QTransform worldTransform = p->worldTransform();

    for (auto anim : nodeAnims) {
        if (anim->finished())
            continue;

        QList<QSvgAbstractAnimatedProperty *> props = anim->properties();
        for (auto prop : props) {
            switch (prop->type()) {
            case QSvgAbstractAnimatedProperty::Color:
                if (prop->propertyName() == QLatin1String("fill")) {
                    QBrush brush = p->brush();
                    brush.setColor(prop->interpolatedValue().value<QColor>());
                    p->setBrush(brush);
                } else if (prop->propertyName() == QLatin1String("stroke")) {
                    QPen pen = p->pen();
                    pen.setColor(prop->interpolatedValue().value<QColor>());
                    p->setPen(pen);
                }
                break;
            case QSvgAbstractAnimatedProperty::Transform:
                p->setWorldTransform(prop->interpolatedValue().value<QTransform>() * worldTransform, false);
                break;
            default:
                break;
            }
        }
    }
}

QList<QSvgAbstractAnimation *> QSvgAnimator::combinedAnimationsForNode(const QSvgNode *node) const
{
    if (!node)
        return QList<QSvgAbstractAnimation *>();

    return m_animationsSMIL.value(node) + m_animationsCSS.value(node);
}

QT_END_NAMESPACE
