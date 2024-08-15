#define MAX_ENTITY 1024
#define MAX_DRAW_LAYER 16

typedef enum EntityType
{
	entt_nil = 0,
	entt_player = 1,
	entt_platform = 2,
	entt_noninteractable = 3,
	entt_interactable = 4,
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

typedef struct Sprite
{
	Gfx_Image* image;
	u8 draw_layer;
} Sprite;

typedef struct Transform
{
	Vector2 pos;
	Vector2 size;
} Transform;

typedef struct Entity
{
	Transform transform;
	bool is_valid;
	Sprite sprite;
	EntityData data;
} Entity;

typedef struct GameWorld
{
	Entity entity_list[MAX_ENTITY];
	Entity* draw_list[MAX_ENTITY];
} GameWorld;

GameWorld* game_world = 0;

Entity* create_entity(int layer)
{
	Entity* entity_found = 0;
	const int layer_size = MAX_ENTITY / MAX_DRAW_LAYER;
	for(int i = layer_size * layer; i < layer_size * (layer + 1); i++)
	{
		Entity* existing_entity = &game_world->entity_list[i];
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

bool check_collision(Transform* a, Transform* b) {
    // Check if there's no overlap along the X-axis
    if (a->pos.x + a->size.x < b->pos.x || b->pos.x + b->size.x < a->pos.x) {
        return false;
    }
    // Check if there's no overlap along the Y-axis
    if (a->pos.y + a->size.y < b->pos.y || b->pos.y + b->size.y < a->pos.y) {
        return false;
    }
    // If both axes overlap, a collision is happening
    return true;
}

void entity_destroy(Entity* selected_entity)
{
	selected_entity->is_valid = false;
	memset(selected_entity, 0, sizeof(Entity));
}

void resolve_boundry_collision(Transform* trans,Vector2 velocity, Vector2 center, Vector2 half_size)
{
	float32 left_boundary = center.x - half_size.x;
    float32 right_boundary = center.x + half_size.x;
    float32 top_boundary = center.y + half_size.y;
    float32 bottom_boundary = center.y - half_size.y;

	if (trans->pos.x < left_boundary) {
        trans->pos.x = left_boundary;
        velocity.x = 0;
    }
    else if (trans->pos.x > right_boundary) {
        trans->pos.x = right_boundary;
        velocity.x = 0;
    }

    
    if (trans->pos.y < bottom_boundary) {
        trans->pos.y = bottom_boundary;
        velocity.y = 0;
    }
    else if (trans->pos.y > top_boundary) {
        trans->pos.y = top_boundary;
        velocity.y = 0;
    }
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
	// game_world->entity_list = alloc(get_heap_allocator(), sizeof(Entity[MAX_ENTITY]) * MAX_DRAW_LAYER);
	
	for(int i = 0; i < MAX_ENTITY; i++)
	{
		game_world->entity_list[i].is_valid = false;
	}

	bool pressing = false;
	Vector2 mvec = v2(0,0);

	Gfx_Image* spr_player = load_image_from_disk(fixed_string("player.png"),get_heap_allocator());
	Gfx_Image* spr_rock = load_image_from_disk(fixed_string("Asteroid.png"),get_heap_allocator());
	Gfx_Image* spr_platform = load_image_from_disk(fixed_string("Platform.png"), get_heap_allocator());
	assert(spr_player, "YOOO BUGG!!");

	float64 last_time = os_get_current_time_in_seconds();
	float64 delta_t;

	const float virtual_width = 296.0;
	const float virtual_height = 187.0;
	float zoom_margin = 2;
	float zoom = (window.width / virtual_width) + (float) zoom_margin; // 0.23125

	Entity* ent_player = create_entity(5);
	Entity* ent_platform = create_entity(4);

	ent_player->transform.size = v2(8.0,16.0);
	ent_player->sprite.draw_layer = 5;
	ent_player->sprite.image = spr_player;
	ent_player->transform.pos = v2(50.0,50.0);

	ent_platform->transform.size = v2(108.0, 76.0);
	ent_platform->sprite.image = spr_platform;
	ent_platform->sprite.draw_layer = 5;
	ent_platform->transform.pos = v2(0 - (ent_platform->transform.size.x / 2), 0 - (ent_platform->transform.size.y / 2));//v2(window.height / 2 , window.width / 2 );
	ent_platform->data.entity_type = entt_platform;



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

		Vector2 velocity_v = v2_mulf(input_axis, 45.0 * delta_t);
		Vector2 target_v = v2_add(ent_player->transform.pos, velocity_v);
		

		// if(check_collision(ent_player, ent_platform))
		// {
		// 	target_v.x -= velocity_v.x;
		// }

		int kaka = 0;
		for(int i = 0; i < MAX_ENTITY; i++)
		{
			Entity* en = &game_world->entity_list[i];
			if(en->is_valid && en != ent_player)
			{
				kaka++;
				log("Entity size:(%f),(%f)   type:%i",en->transform.size.x,en->transform.size.y,en->data.entity_type);
				switch(en->data.entity_type)
				{
					case entt_platform:
						Vector2 square_center = v2(ent_platform->transform.pos.x + 
							(ent_platform->transform.size.x / 2), ent_platform->transform.pos.y + 
							(ent_platform->transform.size.y / 2));
						Vector2 half_size = v2_divf(ent_platform->transform.size,2.0);

						resolve_boundry_collision(&ent_player->transform,velocity_v,square_center, half_size);

					break;

					default:
						Transform tmp_t = ent_player->transform;
						tmp_t.pos.x += velocity_v.x;
						bool x_coll_align = check_collision(&tmp_t, en);
						tmp_t = ent_player->transform;
						tmp_t.pos.y += velocity_v.y;
						bool y_coll_align = check_collision(&tmp_t, en);

						if(x_coll_align) target_v.x = ent_player->transform.pos.x;
						if(y_coll_align) target_v.y = ent_player->transform.pos.y;
					break;
				}
			}
		}

		ent_player->transform.pos = target_v;

		for(int i = 0; i < MAX_ENTITY; i++)
		{
			Entity* en = &game_world->entity_list[i];
			if(en->is_valid)
			{
				draw_image(en->sprite.image, en->transform.pos, en->transform.size, COLOR_WHITE);
			}
		}

		os_update(); 
		gfx_update();
	}

	return 0;
}