#include "extra2d/action/action_instant.h"
#include "extra2d/scene/node.h"

namespace extra2d {

ActionInstant::ActionInstant() {
    duration_ = 0.0f;
}

bool ActionInstant::isDone() const {
    return done_;
}

void ActionInstant::startWithTarget(Node* target) {
    FiniteTimeAction::startWithTarget(target);
    done_ = false;
}

void ActionInstant::step(float dt) {
    (void)dt;
    if (state_ != ActionState::Running) {
        return;
    }
    execute();
    done_ = true;
    state_ = ActionState::Completed;
    onComplete();
}

} // namespace extra2d
