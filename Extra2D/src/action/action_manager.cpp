#include "extra2d/action/action_manager.h"
#include "extra2d/scene/node.h"
#include "extra2d/utils/logger.h"

namespace extra2d {

ActionManager* ActionManager::instance_ = nullptr;

ActionManager::ActionManager() {}

ActionManager::~ActionManager() {
    removeAllActions();
}

ActionManager* ActionManager::getInstance() {
    if (!instance_) {
        instance_ = new ActionManager();
    }
    return instance_;
}

void ActionManager::destroyInstance() {
    if (instance_) {
        delete instance_;
        instance_ = nullptr;
    }
}

void ActionManager::addAction(Action* action, Node* target, bool paused) {
    if (!action || !target) {
        return;
    }

    auto it = targets_.find(target);
    if (it == targets_.end()) {
        ActionElement element;
        element.target = target;
        element.paused = paused;
        targets_[target] = element;
        it = targets_.find(target);
    }

    auto& element = it->second;
    element.actions.push_back(action);
    action->startWithTarget(target);
}

void ActionManager::removeAction(Action* action) {
    if (!action) {
        return;
    }

    Node* target = action->getOriginalTarget();
    if (!target) {
        return;
    }

    auto it = targets_.find(target);
    if (it == targets_.end()) {
        return;
    }

    auto& element = it->second;
    for (size_t i = 0; i < element.actions.size(); ++i) {
        if (element.actions[i] == action) {
            removeActionAt(i, element);
            break;
        }
    }
}

void ActionManager::removeActionByTag(int tag, Node* target) {
    if (!target) {
        return;
    }

    auto it = targets_.find(target);
    if (it == targets_.end()) {
        return;
    }

    auto& element = it->second;
    for (size_t i = 0; i < element.actions.size(); ++i) {
        if (element.actions[i]->getTag() == tag) {
            removeActionAt(i, element);
            break;
        }
    }
}

void ActionManager::removeActionsByFlags(unsigned int flags, Node* target) {
    if (!target) {
        return;
    }

    auto it = targets_.find(target);
    if (it == targets_.end()) {
        return;
    }

    auto& element = it->second;
    for (int i = static_cast<int>(element.actions.size()) - 1; i >= 0; --i) {
        if (element.actions[i]->getFlags() & flags) {
            removeActionAt(i, element);
        }
    }
}

void ActionManager::removeAllActionsFromTarget(Node* target) {
    if (!target) {
        return;
    }

    auto it = targets_.find(target);
    if (it == targets_.end()) {
        return;
    }

    auto& element = it->second;
    for (auto* action : element.actions) {
        action->stop();
        deleteAction(action);
    }
    element.actions.clear();
    targets_.erase(it);
}

void ActionManager::removeAllActions() {
    for (auto& pair : targets_) {
        auto& element = pair.second;
        for (auto* action : element.actions) {
            action->stop();
            deleteAction(action);
        }
    }
    targets_.clear();
}

Action* ActionManager::getActionByTag(int tag, Node* target) {
    if (!target) {
        return nullptr;
    }

    auto it = targets_.find(target);
    if (it == targets_.end()) {
        return nullptr;
    }

    auto& element = it->second;
    for (auto* action : element.actions) {
        if (action->getTag() == tag) {
            return action;
        }
    }
    return nullptr;
}

size_t ActionManager::getActionCount(Node* target) const {
    auto it = targets_.find(target);
    if (it == targets_.end()) {
        return 0;
    }
    return it->second.actions.size();
}

void ActionManager::pauseTarget(Node* target) {
    auto it = targets_.find(target);
    if (it != targets_.end()) {
        it->second.paused = true;
    }
}

void ActionManager::resumeTarget(Node* target) {
    auto it = targets_.find(target);
    if (it != targets_.end()) {
        it->second.paused = false;
    }
}

bool ActionManager::isPaused(Node* target) const {
    auto it = targets_.find(target);
    if (it == targets_.end()) {
        return false;
    }
    return it->second.paused;
}

void ActionManager::update(float dt) {
    for (auto it = targets_.begin(); it != targets_.end(); ) {
        auto& element = it->second;
        Node* target = element.target;

        if (!element.paused && target) {
            element.actionIndex = 0;
            while (static_cast<size_t>(element.actionIndex) < element.actions.size()) {
                element.currentAction = element.actions[element.actionIndex];
                element.currentActionSalvaged = false;

                if (element.currentAction->getState() != ActionState::Paused) {
                    element.currentAction->step(dt);
                }

                if (element.currentActionSalvaged) {
                    deleteAction(element.currentAction);
                } else if (element.currentAction->isDone()) {
                    element.currentAction->stop();
                    deleteAction(element.currentAction);
                    element.actions.erase(element.actions.begin() + element.actionIndex);
                    continue;
                }

                element.actionIndex++;
            }
        }

        if (element.actions.empty()) {
            it = targets_.erase(it);
        } else {
            ++it;
        }
    }
}

void ActionManager::removeActionAt(size_t index, ActionElement& element) {
    if (index >= element.actions.size()) {
        return;
    }

    Action* action = element.actions[index];
    if (action == element.currentAction) {
        element.currentActionSalvaged = true;
    }
    action->stop();
    deleteAction(action);
    element.actions.erase(element.actions.begin() + index);
}

void ActionManager::deleteAction(Action* action) {
    if (action) {
        delete action;
    }
}

} // namespace extra2d
