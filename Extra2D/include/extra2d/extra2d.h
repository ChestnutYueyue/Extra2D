#pragma once

// Extra2D - 统一入口头文件
// 包含所有公共 API

// Core
#include <extra2d/core/color.h>
#include <extra2d/core/math_types.h>
#include <extra2d/core/types.h>

// Config
#include <extra2d/config/app_config.h>
#include <extra2d/config/config_loader.h>
#include <extra2d/config/config_manager.h>
#include <extra2d/config/module_config.h>
#include <extra2d/config/module_initializer.h>
#include <extra2d/config/module_registry.h>
#include <extra2d/config/platform_config.h>
#include <extra2d/config/platform_detector.h>

// Platform
#include <extra2d/platform/iinput.h>
#include <extra2d/platform/iwindow.h>
#include <extra2d/platform/keys.h>
#include <extra2d/platform/platform_module.h>

// Graphics
#include <extra2d/graphics/camera.h>
#include <extra2d/graphics/font.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/graphics/shader_system.h>
#include <extra2d/graphics/texture.h>

#include <extra2d/graphics/render_target.h>
#include <extra2d/graphics/viewport_adapter.h>
#include <extra2d/graphics/vram_manager.h>

#include <extra2d/graphics/texture_pool.h>

// Scene
#include <extra2d/scene/node.h>
#include <extra2d/scene/scene.h>
#include <extra2d/scene/scene_manager.h>
#include <extra2d/scene/shape_node.h>
#include <extra2d/scene/sprite.h>

// Event
#include <extra2d/event/event.h>
#include <extra2d/event/event_dispatcher.h>
#include <extra2d/event/event_queue.h>
#include <extra2d/event/input_codes.h>

// Utils
#include <extra2d/utils/logger.h>
#include <extra2d/utils/random.h>
#include <extra2d/utils/timer.h>

// Application
#include <extra2d/app/application.h>

#ifdef __SWITCH__
#include <switch.h>
#endif
