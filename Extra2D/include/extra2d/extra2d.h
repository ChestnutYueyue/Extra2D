#pragma once

// Easy2D v3.0 - 统一入口头文件
// 包含所有公共 API

// Core
#include <extra2d/core/types.h>
#include <extra2d/core/string.h>
#include <extra2d/core/color.h>
#include <extra2d/core/math_types.h>

// Platform
#include <extra2d/platform/window.h>
#include <extra2d/platform/input.h>

// Graphics
#include <extra2d/graphics/render_backend.h>
#include <extra2d/graphics/texture.h>
#include <extra2d/graphics/font.h>
#include <extra2d/graphics/camera.h>
#include <extra2d/graphics/shader_system.h>

#include <extra2d/graphics/render_target.h>
#include <extra2d/graphics/vram_manager.h>

// Scene
#include <extra2d/scene/node.h>
#include <extra2d/scene/scene.h>
#include <extra2d/scene/sprite.h>
#include <extra2d/scene/shape_node.h>
#include <extra2d/scene/scene_manager.h>
#include <extra2d/scene/transition_scene.h>
#include <extra2d/scene/transition_fade_scene.h>
#include <extra2d/scene/transition_slide_scene.h>
#include <extra2d/scene/transition_scale_scene.h>
#include <extra2d/scene/transition_flip_scene.h>
#include <extra2d/scene/transition_box_scene.h>

// Event
#include <extra2d/event/event.h>
#include <extra2d/event/event_queue.h>
#include <extra2d/event/event_dispatcher.h>
#include <extra2d/event/input_codes.h>

// Audio
#include <extra2d/audio/audio_engine.h>
#include <extra2d/audio/sound.h>

// Resource
#include <extra2d/resource/resource_manager.h>

// Utils
#include <extra2d/utils/logger.h>
#include <extra2d/utils/timer.h>
#include <extra2d/utils/data.h>
#include <extra2d/utils/random.h>

// Application
#include <extra2d/app/application.h>

#ifdef __SWITCH__
#include <switch.h>
#endif
