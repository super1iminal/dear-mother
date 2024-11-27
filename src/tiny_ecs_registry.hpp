#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

class ECSRegistry
{
	// Callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;
	std::vector<ContainerInterface*> registry_filtered; // auto-cleared, doesn't need to be a member of registry list

public:
	// Manually created list of all components this game has
	ComponentContainer<Animation> animations;
	ComponentContainer<DeathTimer> deathTimers;
	ComponentContainer<Motion> motions;
	ComponentContainer<WorldObject> worldObjects;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Player> players;
	ComponentContainer<Mesh*> meshPtrs;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<UIElement> uiElements;
	ComponentContainer<UIButton> uiButtons;
	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<Interactable> interactables;
	ComponentContainer<Deadly> deadlys;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<vec3> colors;
	ComponentContainer<FlashingColor> flashingColors;
	ComponentContainer<Projectile> projectiles;
	ComponentContainer<Health> healthComponents;
	ComponentContainer<Blocker> blockers;
	ComponentContainer<InvincibleTimer> invincibleTimers;
	ComponentContainer<Floor> floors;
	ComponentContainer<Wall> walls;
	ComponentContainer<FloorItem> floorItems;
	ComponentContainer<BaseUI> baseUI;
	ComponentContainer<Crosshair> crosshairs;
	ComponentContainer<Lifetime> lifetimes;
	ComponentContainer<Particle> particles;
	ComponentContainer<PendingRemove> pendingRemoves;
	ComponentContainer<Friction> frictions;
	ComponentContainer<Shooter> shooters;
	ComponentContainer<GameScene> gameSceneComponents;
	ComponentContainer<MenuScene> menuSceneComponents;
	ComponentContainer<HelpScene> helpSceneComponents;
	ComponentContainer<PauseScene> pauseSceneComponents;
	ComponentContainer<ShopScene> shopSceneComponents;
	ComponentContainer<TestScene> testSceneComponents;
	ComponentContainer<Modifier> modifiers;
	ComponentContainer<Inventory> inventory;
	ComponentContainer<ItemStat> itemStats;
	ComponentContainer<RoomCoordinate> roomCoords;
	ComponentContainer<Active> activeComponents;
	ComponentContainer<Door> doors;
	ComponentContainer<MeshFlag> meshFlags;
	ComponentContainer<GameLoadingHelper> gameLoadingHelper;
	ComponentContainer<Map> map;
	ComponentContainer<FloorText> floorTexts;
	ComponentContainer<BossOne> bossOnes;
	ComponentContainer<BossTwo> bossTwos;
	ComponentContainer<BossThree> bossThrees;
	ComponentContainer<DialogueScene> dialogueSceneComponents;
	ComponentContainer<NPC> NPCs;
	ComponentContainer<TextBox> textBoxes; // note that this will contain 2 other entities
	ComponentContainer<DialogueState> dialogueStates; // should be only 1 of em (don't need to save)

	// Set of all items
	std::vector<ItemStat> all_items;

	// Set of all damage items
	std::vector<ItemStat> damage_items;

	// Set of all speed items
	std::vector<ItemStat> speed_items;

	// Set of all range items
	std::vector<ItemStat> range_items;

	// Set of all fire rate items
	std::vector<ItemStat> fire_rate_items;

	// Set of all healing items
	std::vector<ItemStat> healing_items;


	// filtered containers for rendering (unused)
	FilteredComponentContainer<RenderRequest, GameScene> gameSceneRenderRequests;

	// filtered containers for worldobjects
	FilteredComponentContainer<WorldObject, GameScene> gameSceneWorldObjects;

	// filtered containers for buttons
	FilteredComponentContainer<UIButton, MenuScene> menuSceneButtons;
	FilteredComponentContainer<UIButton, HelpScene> helpSceneButtons;
	FilteredComponentContainer<UIButton, PauseScene> pauseSceneButtons;
	FilteredComponentContainer<UIButton, ShopScene> shopSceneButtons;
	FilteredComponentContainer<UIButton, TestScene> testSceneButtons;

	// for active deadlys
	FilteredComponentContainer<Deadly, Active> activeDeadlys;

	// for active shooters
	FilteredComponentContainer<Shooter, Active> activeShooters;

	// for active gamescene objects
	FilteredComponentContainer<Active, GameScene> gameSceneActives;
	FilteredComponentContainer<Active, Interactable> activeInteractables;

	// constructor that adds all containers for looping over them
	// IMPORTANT: Don't forget to add any newly added containers!
	ECSRegistry() :
		gameSceneRenderRequests(renderRequests, gameSceneComponents),
		gameSceneWorldObjects(worldObjects, gameSceneComponents),
		menuSceneButtons(uiButtons, menuSceneComponents),
		helpSceneButtons(uiButtons, helpSceneComponents),
		pauseSceneButtons(uiButtons, pauseSceneComponents),
		shopSceneButtons(uiButtons, shopSceneComponents),
		testSceneButtons(uiButtons, testSceneComponents),
		activeDeadlys(deadlys, activeComponents),
		activeShooters(shooters, activeComponents),
		gameSceneActives(activeComponents, gameSceneComponents),
		activeInteractables(activeComponents, interactables)
	{
		registry_list.push_back(&animations);
		registry_list.push_back(&deathTimers);
		registry_list.push_back(&motions);
		registry_list.push_back(&collisions);
		registry_list.push_back(&worldObjects);
		registry_list.push_back(&players);
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&uiElements);
		registry_list.push_back(&uiButtons);
		registry_list.push_back(&screenStates);
		registry_list.push_back(&interactables);
		registry_list.push_back(&deadlys);
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&colors);
		registry_list.push_back(&flashingColors);
		registry_list.push_back(&projectiles);
		registry_list.push_back(&healthComponents);
		registry_list.push_back(&blockers);
		registry_list.push_back(&invincibleTimers);
		registry_list.push_back(&floors);
		registry_list.push_back(&walls);
		registry_list.push_back(&baseUI);
		registry_list.push_back(&crosshairs);
		registry_list.push_back(&lifetimes);
		registry_list.push_back(&particles);
		registry_list.push_back(&pendingRemoves);
		registry_list.push_back(&frictions);
		registry_list.push_back(&shooters);
		registry_list.push_back(&modifiers);
		registry_list.push_back(&inventory);
		registry_list.push_back(&itemStats);
		registry_list.push_back(&gameSceneComponents);
		registry_list.push_back(&menuSceneComponents);
		registry_list.push_back(&helpSceneComponents);
		registry_list.push_back(&pauseSceneComponents);
		registry_list.push_back(&shopSceneComponents);
		registry_list.push_back(&testSceneComponents);
		registry_list.push_back(&roomCoords);
		registry_list.push_back(&activeComponents);
		registry_list.push_back(&doors);
		registry_list.push_back(&meshFlags);
		registry_list.push_back(&gameLoadingHelper);
		registry_list.push_back(&map);
		registry_list.push_back(&floorTexts);
		registry_list.push_back(&bossOnes);
		registry_list.push_back(&bossTwos);
		registry_list.push_back(&bossThrees);
		registry_list.push_back(&dialogueSceneComponents);
		registry_list.push_back(&NPCs);
		registry_list.push_back(&textBoxes);
		registry_list.push_back(&dialogueStates);

		// filtered components
		registry_filtered.push_back(&gameSceneRenderRequests);
		registry_filtered.push_back(&gameSceneWorldObjects);

		// buttons filtered
		registry_filtered.push_back(&menuSceneButtons);
		registry_filtered.push_back(&helpSceneButtons);
		registry_filtered.push_back(&pauseSceneButtons);
		registry_filtered.push_back(&shopSceneButtons);
		registry_filtered.push_back(&testSceneButtons);
		registry_filtered.push_back(&activeDeadlys);
		registry_filtered.push_back(&activeShooters);
		registry_filtered.push_back(&gameSceneActives);
		registry_filtered.push_back(&activeInteractables);

		// denote sorted component containers
		renderRequests.setSorted(true);
	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
		for (ContainerInterface* reg : registry_filtered)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}
};

extern ECSRegistry registry;