#pragma once

#include "streamline_data.h"
#include "streamline_headers.h"

#ifdef STREAMLINE_ENABLED
class StreamlineContext {
public:
	// Interposer
	PFun_slInit *slInit = nullptr;
	PFun_slShutdown *slShutdown = nullptr;
	PFun_slIsFeatureSupported *slIsFeatureSupported = nullptr;
	PFun_slGetFeatureFunction *slGetFeatureFunction = nullptr;
	PFun_slGetNewFrameToken *slGetNewFrameToken = nullptr;
	PFun_slAllocateResources *slAllocateResources = nullptr;
	PFun_slFreeResources *slFreeResources = nullptr;
	PFun_slEvaluateFeature *slEvaluateFeature = nullptr;
	PFun_slSetTag *slSetTag = nullptr;
	PFun_slSetConstants *slSetConstants = nullptr;
	PFun_slSetFeatureLoaded *slSetFeatureLoaded = nullptr;

	// Reflex
	sl::ReflexOptions reflex_options = {};
	bool reflex_options_dirty = true;
	PFun_slReflexGetState *slReflexGetState = nullptr;
	PFun_slReflexSetOptions *slReflexSetOptions = nullptr;
	PFun_slReflexSleep *slReflexSleep = nullptr;

	// PCL
	sl::PCLOptions pcl_options = {};
	bool pcl_options_dirty = true;
	PFun_slPCLSetMarker *slPCLSetMarker = nullptr;
	PFun_slPCLSetOptions *slPCLSetOptions = nullptr;

	// DLSS Super Resolution
	PFun_slDLSSGetOptimalSettings *slDLSSGetOptimalSettings = nullptr;
	PFun_slDLSSGetState *slDLSSGetState = nullptr;
	PFun_slDLSSSetOptions *slDLSSSetOptions = nullptr;

	// DLSS Frame Generation
	PFun_slDLSSGGetState *slDLSSGGetState = nullptr;
	PFun_slDLSSGSetOptions *slDLSSGSetOptions = nullptr;

	// NIS
	PFun_slNISSetOptions *slNISSetOptions = nullptr;

	void load_functions();
	void load_functions_post_init();
	static const char *result_to_string(sl::Result result);

	void reflex_set_options(const sl::ReflexOptions &opts);
	void reflex_sleep(sl::FrameToken *frameToken);
	void reflex_get_state(sl::ReflexState &reflexState);
	void pcl_set_options(const sl::PCLOptions &opts);
	void pcl_marker(sl::FrameToken *frameToken, sl::PCLMarker marker);
	sl::FrameToken *get_frame_token();

	void dlssg_disable();

	static void initialize();

	static StreamlineContext &get();

	sl::FrameToken *last_token = nullptr;
	bool isGame = false;

	sl::ViewportHandle dlssg_viewport;
	int dlssg_delay = 0;

#if STREAMLINE_ENABLED_VULKAN
	StreamlineCapabilities enumerate_support_vulkan(void *vk_physical_device);
#endif

	void *streamline_framemarker = nullptr;
	StreamlineCapabilities streamline_capabilities;
};
#endif
