#include "extra2d/action/action_interval.h"
#include "extra2d/scene/node.h"

namespace extra2d {

ActionInterval::ActionInterval(float duration)
    : FiniteTimeAction(duration) {
}

bool ActionInterval::isDone() const {
    return elapsed_ >= duration_;
}

void ActionInterval::startWithTarget(Node* target) {
    FiniteTimeAction::startWithTarget(target);
    elapsed_ = 0.0f;
    firstTick_ = true;
    onStart();
}

void ActionInterval::stop() {
    FiniteTimeAction::stop();
}

void ActionInterval::step(float dt) {
    if (state_ != ActionState::Running) {
        return;
    }

    if (firstTick_) {
        firstTick_ = false;
        elapsed_ = 0.0f;
    } else {
        elapsed_ += dt;
    }

    float progress = 0.0f;
    if (duration_ > 0.0f) {
        progress = std::min(1.0f, elapsed_ / duration_);
    } else {
        progress = 1.0f;
    }

    if (easeFunc_) {
        progress = easeFunc_(progress);
    }

    onUpdate(progress);

    if (progress >= 1.0f) {
        state_ = ActionState::Completed;
        onComplete();
    }
}

} // namespace extra2d
