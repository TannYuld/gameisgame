
int entry(int argc, char **argv) {
	
	window.title = STR("GameIsGame");
	window.scaled_width = 1280; // We need to set the scaled size if we want to handle system scaling (DPI)
	window.scaled_height = 720; 
	window.x = 200;
	window.y = 200;
	window.clear_color = hex_to_rgba(0x000000ff);

	bool pressing = false;
	Vector2 mvec = v2(0,0);

	Gfx_Image* spr_player = load_image_from_disk(fixed_string("player.png"),get_heap_allocator());
	assert(spr_player, "YOOO BUGG!!");

	float64 last_time = os_get_current_time_in_seconds();
	float64 delta_t;

	Vector2 player_pos = v2(0,0);

	while (!window.should_close) {
		float64 now = os_get_current_time_in_seconds();
		if ((int)now != (int)last_time) log("%.2f FPS\n%.2fms", 1.0/(now-last_time), (now-last_time)*1000);
		delta_t = now - last_time;
		last_time = now;
		reset_temporary_storage();
		
		Vector2 input_axis = v2(0,0);
		if(is_key_down('A'))
		{
			input_axis.x -= 1.0;
		}
		if(is_key_down('D'))
		{
			input_axis.x += 1.0;
		}
		if(is_key_down('W'))
		{
			input_axis.y += 1.0;
		}
		if(is_key_down('S'))
		{
			input_axis.y -= 1.0;
		}

		input_axis = v2_normalize(input_axis);

		player_pos = v2_add(player_pos, v2_mulf(input_axis, 1.0 * delta_t));

		Matrix4 xform = m4_scalar(1.0);
		xform = m4_translate(xform, v3(player_pos.x, player_pos.y, 0));
		draw_image_xform(spr_player, xform, v2(.08,.16), COLOR_WHITE);

		os_update(); 
		gfx_update();
	}

	return 0;
}