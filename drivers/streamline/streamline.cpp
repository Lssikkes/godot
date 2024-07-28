#include "streamline.h"
#include "drivers/streamline/streamline_data.h"
#include "streamline_context.h"

#include "core/config/engine.h"
#include "core/config/project_settings.h"
#include "core/input/input.h"

Streamline *Streamline::singleton = nullptr;

void Streamline::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_parameter", "parameter_type", "value"), &Streamline::set_parameter);
	ClassDB::bind_method(D_METHOD("get_capability", "capability_type"), &Streamline::get_capability);

	BIND_ENUM_CONSTANT(STREAMLINE_PARAM_REFLEX_MODE);
	BIND_ENUM_CONSTANT(STREAMLINE_PARAM_REFLEX_FRAME_LIMIT_US);

	BIND_ENUM_CONSTANT(STREAMLINE_CAPABILITY_DLSS);
	BIND_ENUM_CONSTANT(STREAMLINE_CAPABILITY_DLSS_G);
	BIND_ENUM_CONSTANT(STREAMLINE_CAPABILITY_NIS);
	BIND_ENUM_CONSTANT(STREAMLINE_CAPABILITY_REFLEX);
	BIND_ENUM_CONSTANT(STREAMLINE_CAPABILITY_PCL);
}

void Streamline::register_singleton() {
	GDREGISTER_CLASS(Streamline);
	Engine::get_singleton()->add_singleton(Engine::Singleton("Streamline", Streamline::get_singleton()));

	GLOBAL_DEF("rendering/streamline/streamline_log", false);
	GLOBAL_DEF("rendering/streamline/streamline_imgui", false);
	GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/streamline/reflex_mode", PROPERTY_HINT_RANGE, "0,2,1"), 0);
	GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/streamline/reflex_frame_limit_us", PROPERTY_HINT_RANGE, "0,1000000,1"), 0);
}

Streamline *Streamline::get_singleton() {
	return singleton;
}

Streamline::Streamline() {
	singleton = this;
}

Streamline::~Streamline() {
	singleton = nullptr;
}

void Streamline::emit_marker(StreamlineMarkerType marker) {
#ifdef STREAMLINE_ENABLED
	StreamlineContext &sl_context = StreamlineContext::get();

	switch (marker) {
		case StreamlineMarkerType::STREAMLINE_MARKER_INITIALIZE:
			sl_context.initialize();
			return;
		case StreamlineMarkerType::STREAMLINE_MARKER_AFTER_DEVICE_CREATION:
			sl_context.load_functions_post_init();

			if (sl_context.streamline_capabilities.reflexAvailable) {
				sl::ReflexOptions reflex_options = {};
				reflex_options.frameLimitUs = 0;
				reflex_options.virtualKey = 0x7C; // VK_F13;
				reflex_options.mode = sl::ReflexMode::eOff;
				reflex_options.useMarkersToOptimize = true;
				reflex_options.idThread = 0;
				sl_context.reflex_set_options(reflex_options);
			}

			if (sl_context.streamline_capabilities.pclAvailable) {
				sl::PCLOptions pcl_options = {};
				pcl_options.virtualKey = sl::PCLHotKey::eVK_F13; // VK_F13;
				pcl_options.idThread = 0;
				sl_context.pcl_set_options(pcl_options);
			}

			set_parameter(STREAMLINE_PARAM_REFLEX_MODE, (double)GLOBAL_GET("rendering/streamline/reflex_mode"));
			set_parameter(STREAMLINE_PARAM_REFLEX_FRAME_LIMIT_US, (double)GLOBAL_GET("rendering/streamline/reflex_frame_limit_us"));
			return;
		case StreamlineMarkerType::STREAMLINE_MARKER_BEFORE_DEVICE_DESTROY:
			if (sl_context.slShutdown) {
				sl_context.slShutdown();
				sl_context.slShutdown = nullptr;
			}
			return;
		default:
			break;
	}

	if (sl_context.isGame == false || sl_context.streamline_capabilities.reflexAvailable == false) {
		// Make sure we still get frame tokens, needed for DLSS.
		if (marker == StreamlineMarkerType::STREAMLINE_MARKER_BEFORE_MESSAGE_LOOP) {
			sl_context.streamline_framemarker = sl_context.get_frame_token();
		}
		return;
	}

	static unsigned long long lastPingFrameDetected = 0; // TODO: Do this differently to support multi window?
	sl::PCLMarker sl_marker;
	switch (marker) {
		case StreamlineMarkerType::STREAMLINE_MARKER_MODIFY_SWAPCHAIN:
			sl_context.dlssg_disable();
			return;
		case StreamlineMarkerType::STREAMLINE_MARKER_BEFORE_MESSAGE_LOOP:
			if (sl_context.dlssg_delay > 0) {
				--sl_context.dlssg_delay;
			}
			if (sl_context.pcl_options_dirty) {
				sl_context.pcl_set_options(sl_context.pcl_options);
			}
			if (sl_context.reflex_options_dirty) {
				sl_context.reflex_set_options(sl_context.reflex_options);
			}
			sl_context.streamline_framemarker = sl_context.get_frame_token();
			if (sl_context.reflex_options.mode != sl::ReflexMode::eOff || StreamlineContext::get().reflex_options.frameLimitUs > 0) {
				sl_context.reflex_sleep((sl::FrameToken *)sl_context.streamline_framemarker);
			}
			sl_marker = sl::PCLMarker::eControllerInputSample; // eInputSample
			break;
		case StreamlineMarkerType::STREAMLINE_MARKER_BEGIN_RENDER:
			sl_marker = sl::PCLMarker::eRenderSubmitStart;
			break;
		case StreamlineMarkerType::STREAMLINE_MARKER_END_RENDER:
			sl_marker = sl::PCLMarker::eRenderSubmitEnd;
			break;
		case StreamlineMarkerType::STREAMLINE_MARKER_BEGIN_SIMULATION:
			// Handle ping events before simulation starts
			if (Input::get_singleton()->get_last_ping_frame() != lastPingFrameDetected) {
				lastPingFrameDetected = Input::get_singleton()->get_last_ping_frame();
				emit_marker(StreamlineMarkerType::STREAMLINE_MARKER_PC_PING);
			}

			sl_marker = sl::PCLMarker::eSimulationStart;
			break;
		case StreamlineMarkerType::STREAMLINE_MARKER_END_SIMULATION:
			sl_marker = sl::PCLMarker::eSimulationEnd;
			break;
		case StreamlineMarkerType::STREAMLINE_MARKER_BEGIN_PRESENT:
			sl_marker = sl::PCLMarker::ePresentStart;
			break;
		case StreamlineMarkerType::STREAMLINE_MARKER_END_PRESENT:
			sl_marker = sl::PCLMarker::ePresentEnd;
			break;
		case StreamlineMarkerType::STREAMLINE_MARKER_PC_PING:
			sl_marker = sl::PCLMarker::ePCLatencyPing;
			break;
		default:
			return;
	};
	if (sl_context.streamline_framemarker) {
		sl_context.pcl_marker((sl::FrameToken *)sl_context.streamline_framemarker, sl_marker);
	}
#endif
}
void Streamline::set_parameter(StreamlineParameterType parameterType, const Variant &value) {
	_THREAD_SAFE_METHOD_
	switch (parameterType) {
#ifdef STREAMLINE_ENABLED
		case StreamlineParameterType::STREAMLINE_PARAM_REFLEX_MODE: {
			double val = (double)value;
			sl::ReflexMode newMode;
			if (val > 1.0) {
				newMode = sl::ReflexMode::eLowLatencyWithBoost;
			} else if (val > 0.0) {
				newMode = sl::ReflexMode::eLowLatency;
			} else {
				newMode = sl::ReflexMode::eOff;
			}

			if (StreamlineContext::get().reflex_options.mode != newMode) {
				StreamlineContext::get().reflex_options_dirty = true;
			}
			StreamlineContext::get().reflex_options.mode = newMode;
			break;
		}
		case StreamlineParameterType::STREAMLINE_PARAM_REFLEX_FRAME_LIMIT_US: {
			double val = (double)value;
			bool updated = StreamlineContext::get().reflex_options.frameLimitUs != (unsigned int)val;
			StreamlineContext::get().reflex_options.frameLimitUs = (unsigned int)val;
			if (updated) {
				StreamlineContext::get().reflex_options_dirty = true;
			}
			break;
		}
#endif
		default:
			break;
	}
}
bool Streamline::get_capability(StreamlineCapabilityType p_capabilityType) {
#ifdef STREAMLINE_ENABLED
	_THREAD_SAFE_METHOD_
	switch (p_capabilityType) {
		case STREAMLINE_CAPABILITY_DLSS:
			return StreamlineContext::get().streamline_capabilities.dlssAvailable;
		case STREAMLINE_CAPABILITY_DLSS_G:
			return StreamlineContext::get().streamline_capabilities.dlssGAvailable;
		case STREAMLINE_CAPABILITY_NIS:
			return StreamlineContext::get().streamline_capabilities.nisAvailable;
		case STREAMLINE_CAPABILITY_REFLEX:
			return StreamlineContext::get().streamline_capabilities.reflexAvailable;
		case STREAMLINE_CAPABILITY_PCL:
			return StreamlineContext::get().streamline_capabilities.reflexAvailable;
	}
#endif
	return false;
}
void Streamline::set_internal_parameter(const char *key, const Variant &value) {
#ifdef STREAMLINE_ENABLED
	_THREAD_SAFE_METHOD_
	if (strcmp(key, "vulkan_physical_device") == 0) {
#if STREAMLINE_ENABLED_VULKAN
		void *phys_device = (void *)value;
		StreamlineContext::get().streamline_capabilities = StreamlineContext::get().enumerate_support_vulkan(phys_device);
#endif
	}
#endif
}
