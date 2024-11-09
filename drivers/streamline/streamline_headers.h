#pragma once
#ifdef STREAMLINE_ENABLED
#pragma warning(push)
#pragma warning(disable : 5257) // needed to build cleanly in paranoid warnings as errors mode
#include "../../thirdparty/streamline/include/sl.h"
#include "../../thirdparty/streamline/include/sl_consts.h"
#include "../../thirdparty/streamline/include/sl_dlss.h"
#include "../../thirdparty/streamline/include/sl_dlss_g.h"
#include "../../thirdparty/streamline/include/sl_nis.h"
#include "../../thirdparty/streamline/include/sl_pcl.h"
#include "../../thirdparty/streamline/include/sl_reflex.h"
#pragma warning(pop)
#endif