#pragma once
#ifdef STREAMLINE_ENABLED
#pragma warning(push)
// Need to ignore some warnings to build cleanly in werror=yes mode.
#pragma warning(disable : 5257) // warning C5257: Enumeration was previously declared without a fixed underlying type. 
#pragma warning(disable : 4471) // warning C4471: 'VkResult': a forward declaration of an unscoped enumeration must have an underlying type
#include "../../thirdparty/streamline/include/sl.h"
#include "../../thirdparty/streamline/include/sl_consts.h"
#include "../../thirdparty/streamline/include/sl_dlss.h"
#include "../../thirdparty/streamline/include/sl_dlss_g.h"
#include "../../thirdparty/streamline/include/sl_nis.h"
#include "../../thirdparty/streamline/include/sl_pcl.h"
#include "../../thirdparty/streamline/include/sl_reflex.h"
#pragma warning(pop)
#endif