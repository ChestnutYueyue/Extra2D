#include "extra2d/action/action.h"
#include "extra2d/scene/node.h"

namespace extra2d {

Action::Action() : tag_(-1), flags_(0) {}

bool Action::isDone() const {
    return state_ == ActionState::Completed;
}

void Action::startWithTarget(Node* target) {
    target_ = target;
    originalTarget_ = target;
    state_ = ActionState::Running;
    onStart();
}

void Action::stop() {
    target_ = nullptr;
    state_ = ActionState::Completed;
}

void Action::step(float dt) {
    (void)dt;
}

void Action::update(float time) {
    (void)time;
}

void Action::pause() {
    if (state_ == ActionState::Running) {
        state_ = ActionState::Paused;
    }
}

void Action::resume() {
    if (state_ == ActionState::Paused) {
        state_ = ActionState::Running;
    }
}

void Action::restart() {
    state_ = ActionState::Running;
    onStart();
}

} // namespace extra2d
