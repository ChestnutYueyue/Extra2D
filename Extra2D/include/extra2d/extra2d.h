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

// Animation
#include <extra2d/animation/sprite_frame.h>
#include <extra2d/animation/sprite_frame_cache.h>
#include <extra2d/animation/frame_property.h>
#include <extra2d/animation/animation_frame.h>
#include <extra2d/animation/animation_clip.h>
#include <extra2d/animation/animation_controller.h>
#include <extra2d/animation/animation_cache.h>
#include <extra2d/animation/interpolation_engine.h>
#include <extra2d/animation/animated_sprite.h>
#include <extra2d/animation/frame_renderer.h>
#include <extra2d/animation/animation_event.h>
#include <extra2d/animation/animation_node.h>
#include <extra2d/animation/composite_animation.h>
#include <extra2d/animation/ani_parser.h>
#include <extra2d/animation/ani_binary_parser.h>
#include <extra2d/animation/als_parser.h>

// UI
#include <extra2d/ui/widget.h>
#include <extra2d/ui/button.h>
#include <extra2d/ui/text.h>
#include <extra2d/ui/label.h>
#include <extra2d/ui/progress_bar.h>
#include <extra2d/ui/check_box.h>
#include <extra2d/ui/radio_button.h>
#include <extra2d/ui/slider.h>

// Action
#include <extra2d/action/action.h>
#include <extra2d/action/finite_time_action.h>
#include <extra2d/action/action_interval.h>
#include <extra2d/action/action_instant.h>
#include <extra2d/action/action_interval_actions.h>
#include <extra2d/action/action_instant_actions.h>
#include <extra2d/action/action_ease.h>
#include <extra2d/action/action_special.h>
#include <extra2d/action/action_manager.h>
#include <extra2d/action/ease.h>

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

// Spatial
#include <extra2d/spatial/spatial_index.h>
#include <extra2d/spatial/quadtree.h>
#include <extra2d/spatial/spatial_hash.h>
#include <extra2d/spatial/spatial_manager.h>

// Effects
#include <extra2d/effects/post_process.h>
#include <extra2d/effects/particle_system.h>
#include <extra2d/effects/custom_effect_manager.h>

// Application
#include <extra2d/app/application.h>

#ifdef __SWITCH__
#include <switch.h>
#endif