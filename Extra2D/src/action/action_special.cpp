#include "extra2d/action/action_special.h"
#include "extra2d/scene/node.h"

namespace extra2d {

// ============================================================================
// Speed
// ============================================================================

Speed* Speed::create(ActionInterval* action, float speed) {
    auto* speedAction = new Speed();
    speedAction->innerAction_ = action;
    speedAction->speed_ = speed;
    return speedAction;
}

Speed::~Speed() {
    delete innerAction_;
}

void Speed::startWithTarget(Node* target) {
    Action::startWithTarget(target);
    innerAction_->startWithTarget(target);
}

void Speed::stop() {
    innerAction_->stop();
    Action::stop();
}

void Speed::step(float dt) {
    if (state_ != ActionState::Running) {
        return;
    }
    innerAction_->step(dt * speed_);
    if (innerAction_->isDone()) {
        state_ = ActionState::Completed;
        onComplete();
    }
}

bool Speed::isDone() const {
    return innerAction_->isDone();
}

Action* Speed::clone() const {
    return Speed::create(innerAction_->clone(), speed_);
}

Action* Speed::reverse() const {
    return Speed::create(innerAction_->reverse(), speed_);
}

// ============================================================================
// Follow
// ============================================================================

Follow* Follow::create(Node* followedNode) {
    return create(followedNode, Rect::Zero());
}

Follow* Follow::create(Node* followedNode, const Rect& boundary) {
    auto* follow = new Follow();
    follow->followedNode_ = followedNode;
    follow->boundary_ = boundary;
    follow->boundarySet_ = (boundary != Rect::Zero());
    return follow;
}

Follow::~Follow() {
    followedNode_ = nullptr;
}

void Follow::startWithTarget(Node* target) {
    Action::startWithTarget(target);
    if (target && followedNode_) {
        halfScreenSize_ = Vec2(0, 0);
        fullScreenSize_ = Vec2(0, 0);
        
        if (boundarySet_) {
            leftBoundary_ = Vec2(boundary_.origin.x, 0);
            rightBoundary_ = Vec2(boundary_.origin.x + boundary_.size.width, 0);
            topBoundary_ = Vec2(0, boundary_.origin.y);
            bottomBoundary_ = Vec2(0, boundary_.origin.y + boundary_.size.height);
        }
    }
}

void Follow::stop() {
    followedNode_ = nullptr;
    Action::stop();
}

void Follow::step(float dt) {
    (void)dt;
    if (state_ != ActionState::Running || !followedNode_ || !target_) {
        return;
    }

    Vec2 pos = followedNode_->getPosition();

    if (boundarySet_) {
        pos.x = std::clamp(pos.x, leftBoundary_.x, rightBoundary_.x);
        pos.y = std::clamp(pos.y, bottomBoundary_.y, topBoundary_.y);
    }

    target_->setPosition(pos);
}

bool Follow::isDone() const {
    return followedNode_ == nullptr || !followedNode_->isRunning();
}

Action* Follow::clone() const {
    return Follow::create(followedNode_, boundary_);
}

Action* Follow::reverse() const {
    return Follow::create(followedNode_, boundary_);
}

// ============================================================================
// TargetedAction
// ============================================================================

TargetedAction* TargetedAction::create(Node* target, FiniteTimeAction* action) {
    auto* targeted = new TargetedAction();
    targeted->targetNode_ = target;
    targeted->innerAction_ = action;
    return targeted;
}

TargetedAction::~TargetedAction() {
    delete innerAction_;
}

void TargetedAction::startWithTarget(Node* target) {
    Action::startWithTarget(target);
    if (targetNode_) {
        innerAction_->startWithTarget(targetNode_);
    }
}

void TargetedAction::stop() {
    innerAction_->stop();
    Action::stop();
}

void TargetedAction::step(float dt) {
    if (state_ != ActionState::Running) {
        return;
    }
    innerAction_->step(dt);
    if (innerAction_->isDone()) {
        state_ = ActionState::Completed;
        onComplete();
    }
}

bool TargetedAction::isDone() const {
    return innerAction_->isDone();
}

Action* TargetedAction::clone() const {
    return TargetedAction::create(targetNode_, innerAction_->clone());
}

Action* TargetedAction::reverse() const {
    return TargetedAction::create(targetNode_, innerAction_->reverse());
}

} // namespace extra2d
