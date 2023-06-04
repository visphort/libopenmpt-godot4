#ifndef OPENMPT_GODOT_H
#define OPENMPT_GODOT_H

// We don't need windows.h in this example plugin but many others do, and it can
// lead to annoying situations due to the ton of macros it defines.
// So we include it and make sure CI warns us if we use something that conflicts
// with a Windows define.
#include "memory.hpp"
#ifdef WIN32
#include <windows.h>
#endif

#include <engine.hpp>
#include <callable.hpp>
#include <string.hpp>
#include <utility_functions.hpp>
#include <input.hpp>
#include <ref.hpp>
#include <file_access.hpp>
#include <ref_counted.hpp>
#include <sprite2d.hpp>
#include <audio_stream_player.hpp>
#include <audio_stream_generator.hpp>
#include <audio_stream_generator_playback.hpp>
#include <thread.hpp>
#include <global_constants.hpp>
#include <binder_common.hpp>
#include <packed_float32_array.hpp>
#include <packed_string_array.hpp>
#include <packed_byte_array.hpp>
#include <packed_vector2_array.hpp>

#include <bits/stdint-intn.h>
#include <bits/types/__FILE.h>
#include <libopenmpt/libopenmpt.hpp>
#include <string>
#include <vector>
#include <stdio.h>

#define PRINT_ERROR(desc) (godot::UtilityFunctions::printerr("[libopenmpt-godot error]: ", __func__, " -> ", __FILE__, ":", __LINE__, (desc)))
#define PRINT_MESSAGE(msg) (godot::UtilityFunctions::print("[libopenmpt-godot]: ", (msg)))

#define EDITOR_CHECK { \
	if (godot::Engine::get_singleton()->is_editor_hint()) { \
		return;\
	} \
}

#define MOD_NULL_CHECK {\
	if (!module) { \
		PRINT_ERROR("No Module Loaded"); \
		return -1; \
	} \
}

#define MOD_NULL_CHECK_STRING {\
	if (!module) { \
		PRINT_ERROR("No Module Loaded"); \
		return ""; \
	} \
}

#define MOD_NULL_CHECK_STRING_ARRAY {\
	if (!module) { \
		PRINT_ERROR("No Module Loaded"); \
		return godot::PackedStringArray(); \
	} \
}

#define MOD_NULL_CHECK_VOID {\
	if (!module) { \
		PRINT_ERROR("No Module Loaded"); \
		return; \
	} \
}

class OpenMPT : public godot::Node {
	GDCLASS(OpenMPT, godot::Node)

protected:
	static void _bind_methods();

private:
	godot::String data;
	godot::Ref<godot::AudioStreamGeneratorPlayback> audgen = nullptr;
	int sample_rate = 0;
	openmpt::module *module = nullptr;
	godot::Ref<godot::FileAccess> module_file = nullptr;
	int module_log;
	godot::String module_file_path;

	// godot_float32 is a float, so it should work as a float buffer for our
	// purposes
	godot::PackedFloat32Array lbuf;
	godot::PackedFloat32Array rbuf;
	godot::PackedVector2Array pushbuf;
	int bufsize = 0;

	godot::Thread *fill_thread = nullptr;
	bool continue_fill_thread = false;

	godot::PackedStringArray strvec2godot(std::vector<std::string> v) {
		godot::PackedStringArray p;
		for (int i = 0; i < v.size(); i++) {
			p.append(godot::String(v[i].c_str()));
		}
		return p;
	}

public:
	~OpenMPT() {
		EDITOR_CHECK;
		PRINT_MESSAGE("Exiting");
		if (fill_thread) {
			if (fill_thread->is_alive()) {
				PRINT_MESSAGE("Finishing thread");
				continue_fill_thread = false;
				fill_thread->wait_to_finish();
				PRINT_MESSAGE("Thread finished");
			}
			PRINT_MESSAGE("Freeing");
			// fill_thread->free();
			godot::memdelete(fill_thread);
			fill_thread = nullptr;
		}

		if (audgen.ptr() != nullptr) {
			audgen->unreference();
		}

		if (module) {
			delete module;
			module = nullptr;
		}
	}

	OpenMPT();

	static inline uint8_t get_byte(int v, int n) {
		return (v >> (n * 8)) & 0xff;
	}

	static godot::String get_core_version() {
		int v = openmpt::get_core_version();
		char buf[32];
		snprintf(buf, 32, "%d.%d.%d.%d", get_byte(v, 3), get_byte(v, 2), get_byte(v, 1), get_byte(v, 0));
		return godot::String(buf);
	}

	static godot::String get_library_version() {
		int v = openmpt::get_core_version();
		char buf[32];
		snprintf(buf, 32, "%d.%d.%d.%d", get_byte(v, 3), get_byte(v, 2), get_byte(v, 1), get_byte(v, 0));
		return godot::String(buf);
	}

	private:

	void load_module_data();

	bool is_module_loaded() {
		return module != nullptr;
	}

	void set_module_file_path(godot::String path) {
		module_file_path = path;
		load_module_data();
	}

	void load_module_file(godot::String path) {
		set_module_file_path(path);
	}

	godot::String get_module_file_path() {
		return module_file_path;
	}

	godot::String get_cell(int pattern, int row, int channel);
	godot::String get_current_cell(int channel);
	godot::String get_cell_command(int pattern, int row, int channel, int command);
	godot::String get_current_cell_command(int channel, int command);
	godot::Dictionary get_pattern_commands();
	bool cell_is_empty(int pattern, int row, int channel);


	void set_repeat_count(int repeat_count) {
		MOD_NULL_CHECK_VOID;
		module->set_repeat_count(repeat_count);
	}

	double get_duration_seconds() {
		MOD_NULL_CHECK;
		return module->get_duration_seconds();
	}

	godot::String get_row_string(int pattern, int row);

	godot::String get_channel_string(int pattern, int channel);


	int64_t get_pattern_num_rows(int pattern) {
		MOD_NULL_CHECK;
		return module->get_pattern_num_rows(pattern);
	}

	double get_position_seconds() {
		MOD_NULL_CHECK;
		return module->get_position_seconds();
	}

	void set_position_seconds(double seconds) {
		MOD_NULL_CHECK_VOID;
		module->set_position_seconds(seconds);
	}

	double get_current_estimated_bpm() {
		MOD_NULL_CHECK;
		return module->get_current_estimated_bpm();
	}

	int64_t get_current_speed() {
		MOD_NULL_CHECK;
		return module->get_current_speed();
	}

	int64_t get_current_tempo() {
		MOD_NULL_CHECK;
		return module->get_current_tempo();
	}

	int64_t get_current_order() {
		MOD_NULL_CHECK;
		return module->get_current_order();
	}

	double set_position_order_row(int order, int row) {
		MOD_NULL_CHECK;
		return module->set_position_order_row(order, row);
	}

	int64_t get_current_row();

	int64_t get_current_pattern();

	int64_t get_current_playing_channels() {
		MOD_NULL_CHECK;
		return module->get_current_playing_channels();
	}

	double get_current_channel_volume(int channel) {
		MOD_NULL_CHECK;
		return module->get_current_channel_vu_mono(channel);
	}

	double get_current_channel_volume_left(int channel) {
		MOD_NULL_CHECK;
		return module->get_current_channel_vu_left(channel);
	}

	double get_current_channel_volume_right(int channel) {
		MOD_NULL_CHECK;
		return module->get_current_channel_vu_left(channel);
	}

	int64_t get_num_subsongs() {
		MOD_NULL_CHECK;
		return module->get_num_subsongs();
	}

	int64_t get_num_channels() {
		MOD_NULL_CHECK;
		return module->get_num_channels();
	}

	int64_t get_num_patterns() {
		MOD_NULL_CHECK;
		return module->get_num_patterns();
	}

	int64_t get_num_orders() {
		MOD_NULL_CHECK;
		return module->get_num_orders();
	}

	int64_t get_num_instruments() {
		MOD_NULL_CHECK;
		return module->get_num_instruments();
	}

	int64_t get_num_samples() {
		MOD_NULL_CHECK;
		return module->get_num_instruments();
	}

	godot::PackedStringArray get_subsong_names() {
		MOD_NULL_CHECK_STRING_ARRAY;

		return strvec2godot(module->get_subsong_names());
	}

	godot::PackedStringArray get_channel_names() {
		MOD_NULL_CHECK_STRING_ARRAY;

		return strvec2godot(module->get_channel_names());
	}

	godot::PackedStringArray get_order_names() {
		MOD_NULL_CHECK_STRING_ARRAY;

		return strvec2godot(module->get_order_names());
	}

	godot::PackedStringArray get_pattern_names() {
		MOD_NULL_CHECK_STRING_ARRAY;

		return strvec2godot(module->get_pattern_names());
	}

	godot::PackedStringArray get_instrument_names() {
		MOD_NULL_CHECK_STRING_ARRAY;

		return strvec2godot(module->get_instrument_names());
	}

	godot::PackedStringArray get_sample_names() {
		MOD_NULL_CHECK_STRING_ARRAY;

		return strvec2godot(module->get_sample_names());
	}

	int64_t get_order_pattern(int order) {
		MOD_NULL_CHECK;
		return module->get_order_pattern(order);
	}

	godot::PackedStringArray get_metadata_keys() {
		MOD_NULL_CHECK_STRING_ARRAY;

		return strvec2godot(module->get_metadata_keys());
	}

	godot::String get_metadata(godot::String meta_key) {
		MOD_NULL_CHECK_STRING;

		return module->get_metadata(meta_key.utf8().get_data()).c_str();
	}


	void stop() {
		if (!fill_thread) {
			return;
		}

		if (fill_thread->is_alive()) {
			continue_fill_thread = false;
			fill_thread->wait_to_finish();
		}

		fill_thread->unreference();
		// fill_thread->free();
		memfree(fill_thread);
		fill_thread = nullptr;
	}


	void start(bool reset_position = false) {
		if (fill_thread && fill_thread->is_alive()) {
			stop();
		}

		if (!module) {
			PRINT_ERROR("No Module Loaded");
			return;
		}

		if (reset_position) {
			module->set_position_seconds(0);
		}

		continue_fill_thread = true;

		_fill_buffer();

		fill_thread = memnew(godot::Thread);
		fill_thread->reference();
		fill_thread->start(godot::Callable(this, "_fill_thread_func"));
	}

	void set_audio_generator_playback(godot::AudioStreamPlayer *a) {
		stop();

		if (audgen.ptr() != nullptr) {
			audgen->unreference();
		}

		audgen = a->get_stream_playback();
		audgen->reference();

		if (!audgen.is_valid()) {
			PRINT_ERROR("Could not load audio generator (invalid parameter)?");
		}

		godot::Ref<godot::AudioStreamGenerator> stream = a->get_stream();
		stream->reference();

		if (!stream.is_valid()) {
			PRINT_ERROR("Could not load audio stream (invalid parameter)?");
		}

		sample_rate = stream->get_mix_rate();

		stream->unreference();
	}

	void _fill_thread_func() {
		while (continue_fill_thread) {
			_fill_buffer();
		}
		PRINT_MESSAGE("Thread Exiting");
	}

	void _fill_buffer() {
		// It's been clarified in the documentation now that its the number
		// of frames that can be pushed? I don't remember.
		int count = audgen->get_frames_available();

		if (count == 0) {
			return;
		}

		if (!audgen->can_push_buffer(count)) {
			return;
		}
		
		if (count > bufsize) {
			bufsize = count;
			lbuf.resize(bufsize);
			rbuf.resize(bufsize);
		}

		pushbuf.resize(count);

		// buf should always read count samples, not bufsize.
		int frames = module->read(sample_rate, count, lbuf.ptrw(), rbuf.ptrw());
		// if (frames == 0) {
		// 	// end of song
		// 	continue_fill_thread = false;
		// }
		for (int i = 0; i < count; i++) {
			pushbuf.set(i, godot::Vector2(lbuf[i], rbuf[i]));
		}
		audgen->push_buffer(pushbuf);
	}

};

#endif // OPENMPT_GODOT_H
