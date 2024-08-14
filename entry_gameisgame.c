#define MAX_ENTITY 1024

typedef enum EntityType
{
	entt_nil = 0,
	entt_player = 1,
	entt_noninteractable = 2,
	entt_interactable = 3,
} EntityType;

typedef enum InteractableType
{
	intt_nil = 0,
	intt_kaka = 1,
} InteractableType;

typedef struct EntityData
{
	EntityType entity_type;
	InteractableType interactable_type;
} EntityData;

typedef struct Entity
{
	Vector2 pos;
	Vector2 size;
	bool is_valid;
	Gfx_Image sprite;
	EntityData data;
} Entity;

typedef struct GameWorld
{
	Entity entity_list[MAX_ENTITY];
} GameWorld;

GameWorld* game_world = 0;

Entity* create_entity()
{
	Entity* entity_found = 0;
	for(int i = 0; i < MAX_ENTITY; i++)
	{
		Entity* existing_entity = &game_world->entity_list[i];
		// log("Attempt#%i to find ram slot for entity.\nis_valid:%i\nram location:%p\n",i,existing_entity->is_valid,existing_entity);
		if(!existing_entity->is_valid)
		{
			entity_found = existing_entity;
			existing_entity->is_valid = true;
			break;
		}
	}
	assert(entity_found, "No more space in ram for more enTITIES!");
	return entity_found;
}

void entity_destroy(Entity* selected_entity)
{
	selected_entity->is_valid = false;
	memset(selected_entity, 0, sizeof(Entity));
}

int entry(int argc, char **argv) 
{
	
	window.title = STR("GameIsGame");
	window.scaled_width = 1280; // We need to set the scaled size if we want to handle system scaling (DPI)
	window.scaled_height = 720; 
	window.x = 200;
	window.y = 200;
	window.clear_color = hex_to_rgba(0x000000ff);

	game_world = alloc(get_heap_allocator(), sizeof(GameWorld));

	bool pressing = false;
	Vector2 mvec = v2(0,0);

	Gfx_Image* spr_player = load_image_from_disk(fixed_string("player.png"),get_heap_allocator());
	assert(spr_player, "YOOO BUGG!!");

	float64 last_time = os_get_current_time_in_seconds();
	float64 delta_t;

	float zoom = 4.324; // 0.23125

	Entity* ent_player = create_entity();

	for(int i = 0; i < 10; i++)
	{
		Entity* en = entity_create();
		en->pos = v2(i * 10.0, 0.0);
	}

	// Vector2 player_pos = v2(0,0);

	while (!window.should_close) {
		float64 now = os_get_current_time_in_seconds();
		if ((int)now != (int)last_time) log("%.2f FPS\n%.2fms", 1.0/(now-last_time), (now-last_time)*1000);
		delta_t = now - last_time;
		last_time = now;
		reset_temporary_storage();

		draw_frame.projection = m4_make_orthographic_projection(window.width * -.5, window.width * .5, window.height * -.5, window.height * .5, -1, 10);
		draw_frame.view = m4_make_scale(v3(1/zoom,1/zoom,1.0));

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

		ent_player->pos = v2_add(ent_player->pos, v2_mulf(input_axis, 45.0 * delta_t));

		Matrix4 xform = m4_scalar(1.0);
		xform = m4_translate(xform, v3(ent_player->pos.x, ent_player->pos.y, 0));
		draw_image_xform(spr_player, xform, v2(8.0,16.0), COLOR_WHITE);

		for(int i = 0; i < MAX_ENTITY; i++)
		{
			Entity* en = &game_world->entity_list[i];
			if(en->is_valid)
			{
				draw_image(en->sprite, en->pos, en->size, COLOR_WHITE);
			}
		}

		os_update(); 
		gfx_update();
	}

	return 0;
}