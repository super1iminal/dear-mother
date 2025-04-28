#include "collision_handler_system.hpp"

void CollisionHandlerSystem::handle_collisions() {
	// god damn. 
	for (Entity entity : registry.collisions.entities) {
		std::vector<Collision*> collisions = registry.collisions.get_all(entity);

		for (int j = 0; j < collisions.size(); j++) {
			Collision collision = *collisions[j];
			Entity entity_other = collision.other;
			COLLISION_TYPE type = collision.type;
			if ((!registry.activeComponents.has(entity)) || (!registry.activeComponents.has(entity_other)))
				continue;

			// Handle collisions
					// Note that enum words are ordered in terms of what is main and what is other (DEADLY BLOCKER will be DEADLY and then other is BLOCKER)
			switch (type) {
			case COLLISION_TYPE::PLAYER_DEADLY:
				handle_player_deadly(entity, entity_other);
				break;
			case COLLISION_TYPE::DEADLY_BLOCKER:
				handle_actor_blocker(entity, entity_other);
				break;
			case COLLISION_TYPE::PLAYER_BLOCKER:
				handle_actor_blocker(entity, entity_other);
				break;
			case COLLISION_TYPE::PROJECTILE_BLOCKER:
				handle_projectile_blocker(entity, entity_other);
				break;
			case COLLISION_TYPE::PROJECTILE_DEADLY:
				handle_projectile_deadly(entity, entity_other);
				// note that this collision is only added if the projectile is friendly
				break;
			case COLLISION_TYPE::PROJECTILE_PLAYER:
				handle_projectile_player(entity, entity_other);
				// note that this collision is only added if the projectile is not friendly
				break;
			case COLLISION_TYPE::PLAYER_DOOR:
				handle_player_door(entity, entity_other);
				break;
			case COLLISION_TYPE::PLAYER_BOSS_ONE:
				handle_player_boss_one(entity, entity_other);
				break;
			case COLLISION_TYPE::CROSSHAIR_TEXTURED_UI_ELEMENT:
				handle_crosshair_textured_UIElement(entity, entity_other);
				break;
			case COLLISION_TYPE::CROSSHAIR_INTERACTABLE:
				handle_crosshair_interactable(entity, entity_other);
			default:
				printf("Unhandled collision\n");
				break;
			}
		}

	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

void CollisionHandlerSystem::handle_player_door(Entity player, Entity door) {
	// Lock door during combat
	if (registry.players.get(player).combat_state == COMBAT_STATE::NO_COMBAT) {
		// change rooms
		Door& door_component = registry.doors.get(door);
		if (!door_component.stage_switch) {
			ivec2 new_room = registry.doors.get(door).leads_to;
			printf("room switching from %d, %d to %d, %d\n", current_room.x, current_room.y, new_room.x, new_room.y);
			WorldObject& player_worldobject = registry.worldObjects.get(player);

			int dx = new_room.x - current_room.x; // positive if moving to the right
			int dy = new_room.y - current_room.y; // positive if moving up

			if (dx == 1) {
				player_worldobject.position = { WALL_WIDTH + 80, (window_height_px - BASE_UI_HEIGHT) / 2.f + BASE_UI_HEIGHT };
			}
			else if (dx == -1) {
				player_worldobject.position = { window_width_px - (WALL_WIDTH + 80), (window_height_px - BASE_UI_HEIGHT) / 2.f + BASE_UI_HEIGHT };
			}
			else if (dy == 1) {
				player_worldobject.position = { window_width_px / 2, (window_height_px - WALL_WIDTH) - 80 };
			}
			else if (dy == -1) {
				player_worldobject.position = { window_width_px / 2, BASE_UI_HEIGHT + (WALL_WIDTH + 80) };
			}
			change_rooms(new_room);
		}
		else {
			go_next_stage();

			ivec2 new_room = current_room;
			WorldObject& player_worldobject = registry.worldObjects.get(player);

			player_worldobject.position = { window_width_px / 2, (window_height_px - BASE_UI_HEIGHT) / 2.f + BASE_UI_HEIGHT };
			change_rooms(current_room);
		}
	}
}

void CollisionHandlerSystem::handle_player_boss_one(Entity player, Entity boss) {
	std::array<Entity, 2> entities = { player, boss };
	for (Entity entity : entities) {
		if (!registry.invincibleTimers.has(entity) && !registry.deathTimers.has(player)) {
			registry.invincibleTimers.emplace(entity);
			if (registry.players.has(entity)) {
				Modifier& player_modifier = registry.modifiers.get(player);
				if (uniform_dist(rng) * 100 > (100 - player_modifier.dodge_chance)) {
					// successful dodge; do not remove health
					playPlayerDodgeEffect(player);
					// TODO: a special sound effect would be nice
				}
				else {
					registry.healthComponents.get(entity).curr_health -= registry.deadlys.get(boss).melee_damge;

					if (registry.bossOnes.has(boss)) {
						if ((registry.bossOnes.get(boss).boss_state == BOSS_ONE_STATE::ONE_ALIVE_B_L
							|| registry.bossOnes.get(boss).boss_state == BOSS_ONE_STATE::ONE_ALIVE_B_R
							|| registry.bossOnes.get(boss).boss_state == BOSS_ONE_STATE::ONE_ALIVE_T_L
							|| registry.bossOnes.get(boss).boss_state == BOSS_ONE_STATE::ONE_ALIVE_T_R)
							&& registry.bossOnes.get(boss).boss_pos != BOSS_ONE_POS::MOTHER) {
							registry.healthComponents.get(boss).curr_health = 0;
						}
					}

					createParticles(renderer, registry.worldObjects.get(entity).position, uniform_dist, rng, TEXTURE_ASSET_ID::HIT_PARTICLE_PLAYER, current_room);
					Mix_Volume(Mix_PlayChannel(-1, melee_sound, 0), 10);
				}

			}
			else if (!registry.deadlys.get(entity).immune) {
				registry.healthComponents.get(entity).curr_health -= 1;
			}
			else {
				createParticles(renderer, registry.worldObjects.get(entity).position, uniform_dist, rng, TEXTURE_ASSET_ID::HIT_PARTICLE, current_room);
			}
			updateGameUI();

		}
	}
	return;
}

void CollisionHandlerSystem::handle_player_deadly(Entity player, Entity deadly) {

	std::array<Entity, 2> entities = { player, deadly };
	for (Entity entity : entities) {
		if (!registry.invincibleTimers.has(entity) && !registry.deathTimers.has(player)) {
			registry.invincibleTimers.emplace(entity);
			if (registry.players.has(entity)) {
				Modifier& player_modifier = registry.modifiers.get(player);
				if (uniform_dist(rng) * 100 > (100 - player_modifier.dodge_chance)) {
					// successful dodge; do not remove health
					playPlayerDodgeEffect(player);
					// TODO: a special sound effect would be nice
				}
				else if (registry.deadlys.get(deadly).type != 1) {
					registry.healthComponents.get(entity).curr_health -= 1;
					updateGameUI();
					playPlayerDamagedEffect(player);
					createParticles(renderer, registry.worldObjects.get(entity).position, uniform_dist, rng, TEXTURE_ASSET_ID::HIT_PARTICLE_PLAYER, current_room);
					Mix_Volume(Mix_PlayChannel(-1, melee_sound, 0), 10);
				}
			}
		}
	}
	return;
}

void CollisionHandlerSystem::handle_actor_blocker(Entity actor, Entity blocker) {

	if ((registry.deadlys.has(actor) && registry.deadlys.get(actor).type == FLY_TYPE)
		|| registry.bossThrees.has(actor))
		return;
	// Get the WorldObject components of both entities
	WorldObject& worldobject_actor = registry.worldObjects.get(actor);
	WorldObject& worldobject_blocker = registry.worldObjects.get(blocker);

	// Get the bounding boxes (sizes) of both entities
	vec2 bbox_actor = get_bounding_box(worldobject_actor);
	vec2 bbox_blocker = get_bounding_box(worldobject_blocker);

	// Compute half sizes for easier calculation
	float half_width_actor = bbox_actor.x / 2.0f;
	float half_height_actor = bbox_actor.y / 2.0f;
	float half_width_blocker = bbox_blocker.x / 2.0f;
	float half_height_blocker = bbox_blocker.y / 2.0f;

	// Compute the difference in positions
	float dx = worldobject_actor.position.x - worldobject_blocker.position.x;
	float dy = worldobject_actor.position.y - worldobject_blocker.position.y;

	// Compute combined half widths and heights
	float combined_half_widths = half_width_actor + half_width_blocker;
	float combined_half_heights = half_height_actor + half_height_blocker;

	// Check for collision on the x and y axes
	if (std::abs(dx) < combined_half_widths && std::abs(dy) < combined_half_heights) {
		// Collision detected
		float overlap_x = combined_half_widths - std::abs(dx);
		float overlap_y = combined_half_heights - std::abs(dy);

		// Determine the direction of maximum overlap
		if (overlap_x > overlap_y) {
			// Push out along the y-axis
			if (dy > 0) {
				// Actor is below the blocker
				worldobject_actor.position.y += overlap_y;
			}
			else {
				// Actor is above the blocker
				worldobject_actor.position.y -= overlap_y;
			}
		}
		else {
			// Push out along the x-axis
			if (dx > 0) {
				// Actor is to the right of the blocker
				worldobject_actor.position.x += overlap_x;
			}
			else {
				// Actor is to the left of the blocker
				worldobject_actor.position.x -= overlap_x;
			}
		}
	}
}

void CollisionHandlerSystem::handle_projectile_blocker(Entity projectile, Entity blocker) {
	// remove projectile
	registry.pendingRemoves.emplace_with_duplicates(projectile);
	return;
}

void CollisionHandlerSystem::handle_projectile_deadly(Entity projectile, Entity deadly) {
	// Decrease health of deadly
	if (!registry.deadlys.get(deadly).immune) {
		registry.healthComponents.get(deadly).curr_health -= registry.projectiles.get(projectile).damage;
		createParticles(renderer, registry.worldObjects.get(deadly).position, uniform_dist, rng, TEXTURE_ASSET_ID::HIT_PARTICLE, current_room);
	}

	if (registry.bossThrees.has(deadly)) {
		// Player shots to be reflected
		BossThree& boss_three = registry.bossThrees.get(deadly);
		if (boss_three.boss_phase == BOSS_THREE_PHASE::PHASE_TWO) {
			boss_three.reflect_shots++;
		}
	}

	// Remove projectile
	registry.pendingRemoves.emplace_with_duplicates(projectile);
	return;
}

void CollisionHandlerSystem::handle_projectile_player(Entity projectile, Entity player) {
	// Decrease health of player
	if (!registry.invincibleTimers.has(player) && !registry.deathTimers.has(player)) {
		Modifier& player_modifier = registry.modifiers.get(player);
		if (uniform_dist(rng) * 100 > (100 - player_modifier.dodge_chance)) {
			playPlayerDodgeEffect(player);
			// TODO: a special sound effect would be nice
		}
		else {
			registry.invincibleTimers.emplace(player);
			registry.healthComponents.get(player).curr_health -= registry.projectiles.get(projectile).damage;
			playPlayerDamagedEffect(player);
			createParticles(renderer, registry.worldObjects.get(player).position, uniform_dist, rng, TEXTURE_ASSET_ID::HIT_PARTICLE_PLAYER, current_room);

			updateGameUI();
			Mix_Volume(Mix_PlayChannel(-1, player_projectile_damage_sound, 0), 5);
		}
	}

	// Remove projectile
	registry.pendingRemoves.emplace_with_duplicates(projectile);
	return;
}

void CollisionHandlerSystem::handle_crosshair_interactable(Entity crosshair, Entity interactable) {

	// we know that the crosshair has collided with the texturedUIElement, and is colliding with it. first, we set the hover stuff for the uielement
	if (registry.players.components[0].combat_state != COMBAT_STATE::NO_COMBAT || registry.players.components[0].boss_one_beat || registry.players.components[0].boss_one_dead) {
		return; // dont distract player
	}
	if (registry.hasPopUpComponents.has(interactable)) {
		/*printf("UI element %d Already has a popup\n", texturedUIElement);*/
		return; // return if already has a popup
	}
	if (registry.itemStats.has(interactable)) {
		ItemStat& is = registry.itemStats.get(interactable);
		WorldObject& wo = registry.worldObjects.get(interactable);
		Entity popup = UISystem::createTextPopUpAsher(
			renderer,
			wo.position,
			vec2(400, 100),
			vec3(1, 1, 1),
			SCENE_TYPE::GAME,
			itemNameToDescription.at(is.name) + "\nE to pick up, X to scrap",
			interactable,
			500);
	}
	if (registry.NPCs.has(interactable)) {
		NPC& npc = registry.NPCs.get(interactable);
		WorldObject& wo = registry.worldObjects.get(interactable);
		Entity popup = UISystem::createTextPopUpAsher(
			renderer,
			wo.position,
			vec2(400, 100),
			vec3(1, 1, 1),
			SCENE_TYPE::GAME,
			NPCtypeToDescription.at(npc.type) + "\nE to interact",
			interactable,
			500);
	}
}


void CollisionHandlerSystem::handle_crosshair_textured_UIElement(Entity crosshair, Entity texturedUIElement) {

	// we know that the crosshair has collided with the texturedUIElement, and is colliding with it. first, we set the hover stuff for the uielement
	UIElement& uie = registry.uiElements.get(texturedUIElement);
	if (registry.hasPopUpComponents.has(texturedUIElement)) {
		/*printf("UI element %d Already has a popup\n", texturedUIElement);*/
		return; // return if already has a popup
	}

	std::string prefix = "item_ui_";
	std::string uielement_name = uie.name;
	if (uielement_name.compare(0, prefix.length(), prefix) != 0) {
		// ignore, it's not an item
		return;
	}

	// Convert the position to an integer
	int pos = std::stoi(uielement_name.substr(prefix.length()));

	//// create the popup. this will automatically assign a lifetime (here 1s), and set hasPopUpComponent of texturedUIElement to true, and
	//// if the popup is deleted through cleanup in game manager, will also remove hasPopUpComponent of texturedUIElement
	Entity popup = UISystem::createTextPopUpAsher(
		renderer,
		registry.worldObjects.get(texturedUIElement).position,
		vec2(400, 100),
		vec3(1, 1, 1),
		SCENE_TYPE::GAME,
		itemNameToDescription.at(getItemNameFromTexture(registry.renderRequests.get(texturedUIElement).used_texture)) +
		"\nPress " + std::to_string(pos + 1) + " to drop",
		texturedUIElement,
		500);
}