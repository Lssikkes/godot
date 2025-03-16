#pragma once

#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/variant/binder_common.h"
#include "core/variant/variant.h"
#include "streamline_data.h"

class Streamline : public Object {
	GDCLASS(Streamline, Object);
	_THREAD_SAFE_CLASS_
protected:
	static void _bind_methods();
	static Streamline *singleton;

public:
	static Streamline *get_singleton();
	static void register_singleton();

	void emit_marker(StreamlineMarkerType p_marker);
	void set_parameter(StreamlineParameterType p_parameterType, const Variant &p_value);
	bool get_capability(StreamlineCapabilityType p_capabilityType);

	void set_internal_parameter(const char *p_key, void *p_value);
	void *get_internal_parameter(StreamlineInternalParameterType p_internal_parameter_type);

	void update_project_settings();

	Streamline();
	virtual ~Streamline();
};

VARIANT_ENUM_CAST(StreamlineParameterType);
VARIANT_ENUM_CAST(StreamlineCapabilityType);
