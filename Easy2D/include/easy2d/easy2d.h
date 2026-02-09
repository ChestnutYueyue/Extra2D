#pragma once

// Easy2D v3.0 - 统一入口头文件
// 包含所有公共 API

// Core
#include <easy2d/core/types.h>
#include <easy2d/core/string.h>
#include <easy2d/core/color.h>
#include <easy2d/core/math_types.h>

// Platform
#include <easy2d/platform/window.h>
#include <easy2d/platform/input.h>

// Graphics
#include <easy2d/graphics/render_backend.h>
#include <easy2d/graphics/texture.h>
#include <easy2d/graphics/font.h>
#include <easy2d/graphics/camera.h>
#include <easy2d/graphics/shader_system.h>
#include <easy2d/graphics/texture_pool.h>
#include <easy2d/graphics/render_target.h>
#include <easy2d/graphics/vram_manager.h>

// Scene
#include <easy2d/scene/node.h>
#include <easy2d/scene/scene.h>
#include <easy2d/scene/sprite.h>
#include <easy2d/scene/text.h>
#include <easy2d/scene/shape_node.h>
#include <easy2d/scene/scene_manager.h>
#include <easy2d/scene/transition.h>

// Animation
#include <easy2d/animation/sprite_frame.h>
#include <easy2d/animation/sprite_frame_cache.h>
#include <easy2d/animation/frame_property.h>
#include <easy2d/animation/animation_frame.h>
#include <easy2d/animation/animation_clip.h>
#include <easy2d/animation/animation_controller.h>
#include <easy2d/animation/animation_cache.h>
#include <easy2d/animation/interpolation_engine.h>
#include <easy2d/animation/animated_sprite.h>
#include <easy2d/animation/frame_renderer.h>
#include <easy2d/animation/animation_event.h>
#include <easy2d/animation/animation_node.h>
#include <easy2d/animation/composite_animation.h>
#include <easy2d/animation/ani_parser.h>
#include <easy2d/animation/ani_binary_parser.h>
#include <easy2d/animation/als_parser.h>

// UI
#include <easy2d/ui/widget.h>
#include <easy2d/ui/button.h>

// Action
#include <easy2d/action/action.h>
#include <easy2d/action/actions.h>
#include <easy2d/action/ease.h>

// Event
#include <easy2d/event/event.h>
#include <easy2d/event/event_queue.h>
#include <easy2d/event/event_dispatcher.h>
#include <easy2d/event/input_codes.h>

// Audio
#include <easy2d/audio/audio_engine.h>
#include <easy2d/audio/sound.h>

// Resource
#include <easy2d/resource/resource_manager.h>

// Utils
#include <easy2d/utils/logger.h>
#include <easy2d/utils/timer.h>
#include <easy2d/utils/data.h>
#include <easy2d/utils/random.h>

// Spatial
#include <easy2d/spatial/spatial_index.h>
#include <easy2d/spatial/quadtree.h>
#include <easy2d/spatial/spatial_hash.h>
#include <easy2d/spatial/spatial_manager.h>

// Effects
#include <easy2d/effects/post_process.h>
#include <easy2d/effects/particle_system.h>
#include <easy2d/effects/custom_effect_manager.h>

// Application
#include <easy2d/app/application.h>

// Script
#include <easy2d/script/script_engine.h>
#include <easy2d/script/script_node.h>
