#include "OpenMPT.hpp"
#include "global_constants.hpp"
#include "object.hpp"
#include "property_info.hpp"
#include "string.hpp"
#include <libopenmpt/libopenmpt.hpp>

void OpenMPT::_bind_methods() {
	using namespace godot;

	ClassDB::bind_method(D_METHOD("set_audio_generator_playback", "audio_stream_player"), &OpenMPT::set_audio_generator_playback);
	ClassDB::bind_method(D_METHOD("_fill_thread_func"),  &OpenMPT::_fill_thread_func);
	ClassDB::bind_method(D_METHOD("is_module_loaded"),  &OpenMPT::is_module_loaded);
	ClassDB::bind_method(D_METHOD("cell_is_empty", "pattern", "row", "channel"), &OpenMPT::cell_is_empty);
	ClassDB::bind_method(D_METHOD("get_cell", "pattern", "row", "channel"), &OpenMPT::get_cell);
	ClassDB::bind_method(D_METHOD("get_current_cell", "channel"), &OpenMPT::get_current_cell);
	ClassDB::bind_method(D_METHOD("get_cell_command", "pattern", "row", "channel", "command"), &OpenMPT::get_cell_command);
	ClassDB::bind_method(D_METHOD("get_current_cell_command", "channel", "command"), &OpenMPT::get_current_cell_command);
	ClassDB::bind_method(D_METHOD("get_pattern_commands"), &OpenMPT::get_pattern_commands);
	ClassDB::bind_method(D_METHOD("set_repeat_count", "repeat_count"), &OpenMPT::set_repeat_count);
	ClassDB::bind_method(D_METHOD("get_duration_seconds"), &OpenMPT::get_duration_seconds);
	ClassDB::bind_method(D_METHOD("get_row_string", "pattern", "row"), &OpenMPT::get_row_string);
	ClassDB::bind_method(D_METHOD("get_channel_string", "pattern", "channel"), &OpenMPT::get_channel_string);
	ClassDB::bind_method(D_METHOD("get_pattern_num_rows", "pattern"), &OpenMPT::get_pattern_num_rows);
	ClassDB::bind_method(D_METHOD("get_position_seconds"), &OpenMPT::get_position_seconds);
	ClassDB::bind_method(D_METHOD("set_position_seconds", "position"), &OpenMPT::set_position_seconds);
	ClassDB::bind_method(D_METHOD("get_current_estimated_bpm"), &OpenMPT::get_current_estimated_bpm);
	ClassDB::bind_method(D_METHOD("get_current_speed"), &OpenMPT::get_current_speed);
	ClassDB::bind_method(D_METHOD("get_current_tempo"), &OpenMPT::get_current_tempo);
	ClassDB::bind_method(D_METHOD("get_current_order"), &OpenMPT::get_current_order);
	ClassDB::bind_method(D_METHOD("get_current_row"), &OpenMPT::get_current_row);
	ClassDB::bind_method(D_METHOD("get_current_pattern"), &OpenMPT::get_current_pattern);
	ClassDB::bind_method(D_METHOD("get_current_playing_channels"), &OpenMPT::get_current_playing_channels);
	ClassDB::bind_method(D_METHOD("get_current_channel_volume", "channel"), &OpenMPT::get_current_channel_volume);
	ClassDB::bind_method(D_METHOD("get_current_channel_volume_left", "channel"), &OpenMPT::get_current_channel_volume_left);
	ClassDB::bind_method(D_METHOD("get_current_channel_volume_right", "channel"), &OpenMPT::get_current_channel_volume_right);
	ClassDB::bind_method(D_METHOD("get_num_subsongs"), &OpenMPT::get_num_subsongs);
	ClassDB::bind_method(D_METHOD("get_num_channels"), &OpenMPT::get_num_channels);
	ClassDB::bind_method(D_METHOD("get_num_patterns"), &OpenMPT::get_num_patterns);
	ClassDB::bind_method(D_METHOD("get_num_orders"), &OpenMPT::get_num_orders);
	ClassDB::bind_method(D_METHOD("get_num_instruments"), &OpenMPT::get_num_instruments);
	ClassDB::bind_method(D_METHOD("get_num_samples"), &OpenMPT::get_num_samples);
	ClassDB::bind_method(D_METHOD("get_subsong_names"), &OpenMPT::get_subsong_names);
	ClassDB::bind_method(D_METHOD("get_channel_names"), &OpenMPT::get_channel_names);
	ClassDB::bind_method(D_METHOD("get_order_names"), &OpenMPT::get_order_names);
	ClassDB::bind_method(D_METHOD("get_pattern_names"), &OpenMPT::get_pattern_names);
	ClassDB::bind_method(D_METHOD("get_instrument_names"), &OpenMPT::get_instrument_names);
	ClassDB::bind_method(D_METHOD("get_sample_names"), &OpenMPT::get_sample_names);
	ClassDB::bind_method(D_METHOD("get_order_pattern", "order"), &OpenMPT::get_order_pattern);
	ClassDB::bind_method(D_METHOD("get_metadata", "meta_key"), &OpenMPT::get_metadata);
	ClassDB::bind_method(D_METHOD("get_metadata_keys"), &OpenMPT::get_metadata_keys);
	ClassDB::bind_method(D_METHOD("set_module_file_path", "path"), &OpenMPT::set_module_file_path);
	ClassDB::bind_method(D_METHOD("load_module_file", "path"), &OpenMPT::load_module_file);
	ClassDB::bind_method(D_METHOD("get_module_file_path"), &OpenMPT::get_module_file_path);
	ClassDB::bind_method(D_METHOD("start", "reset_position"), &OpenMPT::start);
	ClassDB::bind_method(D_METHOD("stop"), &OpenMPT::stop);

	ADD_PROPERTY(
		PropertyInfo(godot::Variant::STRING, "module_file_path", godot::PROPERTY_HINT_FILE),
		"set_module_file_path",
		"get_module_file_path");
}

OpenMPT::OpenMPT() {
	EDITOR_CHECK;
	PRINT_MESSAGE("Initializing");
	PRINT_MESSAGE("libopenmpt version: core " + get_core_version() + " lib "+ get_library_version());
}

void OpenMPT::load_module_data() {
	EDITOR_CHECK;

	if (module_file_path.is_empty()) {
		return;
	}

	PRINT_MESSAGE("Loading: " + module_file_path);

	module_file = godot::FileAccess::open(module_file_path, godot::FileAccess::ModeFlags::READ);
	module_file->reference();

	if (module_file->get_open_error() != godot::Error::OK) {
		PRINT_ERROR(godot::String("Could not open '") + module_file_path + "' for reading.");
		module_file->close();
		module_file->unreference();
		return;
	}

	godot::PackedByteArray bytes = module_file->get_buffer(module_file->get_length());

	try {
		if (module) {
			delete module;
			module = nullptr;
		}
		module = new openmpt::module(bytes.ptr(), bytes.size());
		set_repeat_count(-1);
	} catch (openmpt::exception e) {
		PRINT_ERROR(godot::String("Could not initialize OpenMPT module: ") + e.what());
		module_file->close();
		module_file->unreference();
		return;
	}

	PRINT_MESSAGE("Load Finished");

	// Apparently you shouldnt call delete on it and instead call member
	// funcs for freeing
	module_file->close();
	module_file->unreference();
}

godot::String OpenMPT::get_cell(int pattern, int row, int channel) {
	if (!module) {
		PRINT_ERROR("No Module Loaded");
		return godot::String();
	}
	return module->format_pattern_row_channel(pattern, row, channel).c_str();
}

godot::String OpenMPT::get_current_cell(int channel) {
	if (!module) {
		PRINT_ERROR("No Module Loaded");
		return godot::String();
	}
	return module->format_pattern_row_channel(
		module->get_current_pattern(),
		module->get_current_row(),
		channel).c_str();
}

godot::Dictionary OpenMPT::get_pattern_commands() {
	godot::Dictionary c;

	c["note"] = openmpt::module::command_index::command_note;
	c["instrument"] = openmpt::module::command_index::command_instrument;
	c["volumeffect"] = openmpt::module::command_index::command_volumeffect;
	c["volume"] = openmpt::module::command_index::command_volume;
	c["effect"] = openmpt::module::command_index::command_effect;

	return c;
}

godot::String OpenMPT::get_cell_command(int pattern, int row, int channel, int command) {
	MOD_NULL_CHECK_STRING;
	return module->format_pattern_row_channel_command(pattern, row, channel, command).c_str();
}

godot::String OpenMPT::get_current_cell_command(int channel, int command) {
	MOD_NULL_CHECK_STRING;
	return module->format_pattern_row_channel_command(
		module->get_current_pattern(),
		module->get_current_row(),
		channel, command).c_str();
}



bool OpenMPT::cell_is_empty(int pattern, int row, int channel) {
	return module->get_pattern_row_channel_command(pattern, row, channel, openmpt::module::command_note) == 0 &&
		   module->get_pattern_row_channel_command(pattern, row, channel, openmpt::module::command_instrument) == 0 &&
		   module->get_pattern_row_channel_command(pattern, row, channel, openmpt::module::command_effect) == 0 &&
		   module->get_pattern_row_channel_command(pattern, row, channel, openmpt::module::command_volumeffect) == 0 &&
		   module->get_pattern_row_channel_command(pattern, row, channel, openmpt::module::command_volume) == 0;
}


int64_t OpenMPT::get_current_row() {
	MOD_NULL_CHECK;

	return module->get_current_row();
}

int64_t OpenMPT::get_current_pattern() {
	MOD_NULL_CHECK;

	return module->get_current_pattern();
}

godot::String OpenMPT::get_row_string(int pattern, int row) {
	MOD_NULL_CHECK_STRING;

	godot::String rstr;

	for (int i = 0; i < module->get_num_channels(); i++) {
		rstr += module->format_pattern_row_channel(pattern, row, i).c_str();
		rstr += " ";
	}

	return rstr;
}

godot::String OpenMPT::get_channel_string(int pattern, int channel) {
	MOD_NULL_CHECK_STRING;

	godot::String rstr;

	for (int i = 0; i < module->get_pattern_num_rows(pattern); i++) {
		rstr += module->format_pattern_row_channel(pattern, i, channel).c_str();
		rstr += "\n";
	}

	return rstr;
}

