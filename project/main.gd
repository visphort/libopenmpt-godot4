# Pretty shoddily written. Main purpose is to show how the GDNative library works.

extends Control

@onready var module = %module
@onready var generator_player = %generator_player
@onready var patternscroller = %patternscroller
@onready var container = $patternscroller/container
@onready var info = %info
@onready var seek = %set_seek
@onready var speed = %set_velocity
@onready var playing = %playing
@onready var file_name = %file_name
@onready var volume = %volume

@onready var defaultpos = -get_viewport_rect().size.y / 8

var spectrum: AudioEffectSpectrumAnalyzerInstance
var labels: Array[Label] = []
var prev_pattern: int = -1
var prev_row: int = -1
var playing_channels: int = 0

var update_seek: bool = true

func _input(event: InputEvent) -> void:
	if event is InputEventMouseMotion:
		var offset = patternscroller.size.x - get_viewport_rect().size.x
		if offset > 0:
			patternscroller.position.x = -offset * get_global_mouse_position().x / get_viewport_rect().size.x
		else:
			patternscroller.position.x = -offset/2


func set_metadata() -> void:
	var vals: PackedStringArray = module.get_metadata_keys()
	%metadata/container/type.text = \
	str(module.get_metadata("type"), " (", module.get_metadata("type_long"), ")")

	if module.get_metadata("originaltype") != "":
		%metadata/container/originaltype.text = \
		str(module.get_metadata("originaltype"), " (", module.get_metadata("originaltype_long"), ")")
	else:
		%metadata/container/originaltype.text = ""

	if module.get_metadata("originaltype") != "":
		%metadata/container/container.text = \
		str(module.get_metadata("container"), " (", module.get_metadata("container_long"), ")")
	else:
		%metadata/container/originaltype.text = ""

	%metadata/container/tracker.text = str(module.get_metadata("tracker"))
	%metadata/container/artist.text = str(module.get_metadata("artist"))
	%metadata/container/title.text = str(module.get_metadata("title"))
	%metadata/container/date.text = str(module.get_metadata("date"))
	%metadata/container/message.text = str(module.get_metadata("message"))
	%metadata/container/warnings.text = str(module.get_metadata("warnings"))

func reset_patternscroller_position():
	patternscroller.position.x = (get_viewport_rect().size.x - patternscroller.size.x)/2

func init_module() -> void:
	if not module.is_module_loaded():
		info.text = "Could not load '" + module.module_file_path +  "'"
		set_process(false)
		return

	for i in labels:
		i.queue_free()
	
	labels.clear()

	for i in container.get_children():
		container.remove_child(i)
		i.queue_free()

	patternscroller.reset_size()

	for i in range(module.get_num_channels()):
		var label = Label.new()
		labels.append(label)
		container.add_child(label)

	set_metadata()
	update_labels()
	%playing.button_pressed = false
	reset()
	call_deferred("reset_patternscroller_position")
	file_name.text = module.module_file_path

func reset():
	generator_player.play()
	module.set_audio_generator_playback(generator_player)
	seek.max_value = module.get_duration_seconds()

func _ready() -> void:
	spectrum = AudioServer.get_bus_effect_instance(AudioServer.get_bus_index("Master"), 0)
	init_module()
	pass
	
func update_labels() -> void:
	var pattern: int = module.get_current_pattern()
	for i in range(module.get_num_channels()):
		labels[i].text = module.get_channel_string(pattern, i)

func update_label_volumes() -> void:
	for i in range(module.get_num_channels()):
		labels[i].scale.y = 1 + module.get_current_channel_volume(i) / 10.0
		labels[i].modulate = Color.DARK_GRAY.darkened(0.8).lightened(module.get_current_channel_volume(i) / 1.2)
	if module.get_current_row() != prev_row:
		prev_row = module.get_current_row()
		var tween: Tween = create_tween()
		tween.tween_property(
			patternscroller,
			"position:y",
			get_viewport_rect().size.y/2 - patternscroller.size.y * module.get_current_row() / module.get_pattern_num_rows(module.get_current_pattern()),
			0.1)

func _process(delta: float) -> void:
	update_label_volumes()
	info.text = str(
		"speed: ", module.get_current_speed(), "\n",
		"tempo: ", module.get_current_tempo(), "\n",
		"bpm: ", module.get_current_estimated_bpm(), "\n",
		"order: ", module.get_current_order(), "/", module.get_num_orders(), "\n",
		"order_pattern: ", module.get_order_pattern(module.get_current_order()), "\n",
		"row: ", module.get_current_row(), "\n",
		"num_channels: ", module.get_num_channels(), "\n",
		"position: ", "%.2f" % module.get_position_seconds(), "/", "%.2f" % module.get_duration_seconds(), "\n",
		"playing_channels: ", module.get_current_playing_channels(), "\n"
	)
	volume.value = (60 + linear_to_db(spectrum.get_magnitude_for_frequency_range(0, 11050).length())) * 100 / 60

	if update_seek:
		seek.value = module.get_position_seconds()
	if prev_pattern != module.get_current_pattern():
		update_labels()
		prev_pattern = module.get_current_pattern()


func _on_HSlider_value_changed(value: float) -> void:
	generator_player.pitch_scale = value

func _on_HSlider2_value_changed(value: float) -> void:
	seek.value = value

func _on_HSlider2_drag_started() -> void:
	update_seek = false

func _on_HSlider2_drag_ended(value_changed: bool) -> void:
	update_seek = true
	module.set_position_seconds(seek.value)

func _on_set_seek_gui_input(event: InputEvent) -> void:
	if event == InputEventMouseButton:
		if event.button_index == MOUSE_BUTTON_LEFT:
			module.set_position_seconds(seek.value)


func _on_load_button_pressed() -> void:
	$file.popup_centered(Vector2i(600, 400))
	await $file.file_selected;
	set_process(false)
	module.stop()
	module.module_file_path = $file.current_path
	if not module.is_module_loaded():
		info.text = "Could not load '" + module.module_file_path +  "'"
	else:
		init_module()
		set_process(true)

func _on_show_controls_toggled(button_pressed: bool) -> void:
	%playback_controls.visible = button_pressed
	%metadata.visible = button_pressed


func _on_playing_toggled(button_pressed: bool) -> void:
	if button_pressed:
		reset()
		module.start(false)
	else:
		module.stop()
