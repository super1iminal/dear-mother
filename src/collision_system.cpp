#include "collision_system.hpp"

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const WorldObject& worldobject)
{
    // abs is to avoid negative scale due to the facing direction.
    return { abs(worldobject.scale.x), abs(worldobject.scale.y) };
}

// Function to check if two line segments intersect
bool lines_intersect(vec2 p1, vec2 p2, vec2 q1, vec2 q2)
{
    auto orientation = [](vec2 a, vec2 b, vec2 c) {
        float val = (b.y - a.y) * (c.x - b.x) - (b.x - a.x) * (c.y - b.y);
        if (abs(val) < 1e-6)
            return 0; // colinear
        return (val > 0) ? 1 : 2; // clock or counterclock wise
        };

    int o1 = orientation(p1, p2, q1);
    int o2 = orientation(p1, p2, q2);
    int o3 = orientation(q1, q2, p1);
    int o4 = orientation(q1, q2, p2);

    // General case
    if (o1 != o2 && o3 != o4)
        return true;

    return false;
}

bool point_in_triangle(const vec2& pt, const vec2& v0, const vec2& v1, const vec2& v2) {
    // Compute vectors
    vec2 v0v1 = v1 - v0;
    vec2 v0v2 = v2 - v0;
    vec2 v0p = pt - v0;

    // Compute dot products
    float dot00 = glm::dot(v0v2, v0v2);
    float dot01 = glm::dot(v0v2, v0v1);
    float dot02 = glm::dot(v0v2, v0p);
    float dot11 = glm::dot(v0v1, v0v1);
    float dot12 = glm::dot(v0v1, v0p);

    // Compute barycentric coordinates
    float denom = dot00 * dot11 - dot01 * dot01;
    if (denom == 0.0f)
        return false; // Degenerate triangle

    float invDenom = 1.0f / denom;
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    // Check if point is in triangle
    return (u >= 0) && (v >= 0) && (u + v <= 1);
}

bool point_in_AABB(const vec2& p, const vec2& bb_min, const vec2& bb_max) {
    return (p.x >= bb_min.x && p.x <= bb_max.x &&
        p.y >= bb_min.y && p.y <= bb_max.y);
}

// Helper function to check if a triangle intersects an AABB
bool triangle_intersects_AABB(const vec2& v0, const vec2& v1, const vec2& v2, const vec2& bb_min, const vec2& bb_max) {
    // First, check if any of the triangle's vertices are inside the AABB
    if (point_in_AABB(v0, bb_min, bb_max) ||
        point_in_AABB(v1, bb_min, bb_max) ||
        point_in_AABB(v2, bb_min, bb_max)) {
        return true;
    }

    // Next, check if any of the AABB's corners are inside the triangle
    vec2 aabb_corners[4] = {
        bb_min,
        vec2(bb_max.x, bb_min.y),
        bb_max,
        vec2(bb_min.x, bb_max.y)
    };
    if (point_in_triangle(aabb_corners[0], v0, v1, v2) ||
        point_in_triangle(aabb_corners[1], v0, v1, v2) ||
        point_in_triangle(aabb_corners[2], v0, v1, v2) ||
        point_in_triangle(aabb_corners[3], v0, v1, v2)) {
        return true;
    }

    // Finally, check if any edges intersect
    // Edges of the triangle
    vec2 tri_edges[3][2] = {
        { v0, v1 },
        { v1, v2 },
        { v2, v0 }
    };

    // Edges of the AABB
    vec2 aabb_edges[4][2] = {
        { aabb_corners[0], aabb_corners[1] },
        { aabb_corners[1], aabb_corners[2] },
        { aabb_corners[2], aabb_corners[3] },
        { aabb_corners[3], aabb_corners[0] }
    };

    // Check for intersection between each edge pair
    for (int i = 0; i < 3; i++) {
        vec2 p1 = tri_edges[i][0];
        vec2 p2 = tri_edges[i][1];
        for (int j = 0; j < 4; j++) {
            vec2 q1 = aabb_edges[j][0];
            vec2 q2 = aabb_edges[j][1];
            if (lines_intersect(p1, p2, q1, q2)) {
                return true;
            }
        }
    }

    // No intersection found
    return false;
}

bool CollisionSystem::meshCollides(Entity entity_mesh, Entity entity_bb) {
    // Retrieve WorldObjects for both entities
    WorldObject& object_mesh = registry.worldObjects.get(entity_mesh);
    WorldObject& object_bb = registry.worldObjects.get(entity_bb);

    // Retrieve the Mesh for the mesh entity
    Mesh& mesh = *registry.meshPtrs.get(entity_mesh);

    // Compute the half extents of the AABB (Axis-Aligned Bounding Box) for entity_bb
    vec2 bb_half_extent = get_bounding_box(object_bb) / 2.f;
    vec2 bb_min = object_bb.position - bb_half_extent;
    vec2 bb_max = object_bb.position + bb_half_extent;

    // Build the transformation matrix for the mesh entity
    Transform transform;
    transform.translate(object_mesh.position);
    transform.rotate(object_mesh.angle);
    transform.scale(object_mesh.scale);

    // For each triangle in the mesh
    size_t num_triangles = mesh.vertex_indices.size() / 3;
    for (size_t i = 0; i < num_triangles; i++) {
        uint16_t idx0 = mesh.vertex_indices[i * 3];
        uint16_t idx1 = mesh.vertex_indices[i * 3 + 1];
        uint16_t idx2 = mesh.vertex_indices[i * 3 + 2];

        vec3 v0 = vec3(mesh.vertices[idx0].position.x, mesh.vertices[idx0].position.y, 1.0f);
        vec3 v1 = vec3(mesh.vertices[idx1].position.x, mesh.vertices[idx1].position.y, 1.0f);
        vec3 v2 = vec3(mesh.vertices[idx2].position.x, mesh.vertices[idx2].position.y, 1.0f);

        // Transform vertices into world space
        vec3 tv0 = transform.mat * v0;
        vec3 tv1 = transform.mat * v1;
        vec3 tv2 = transform.mat * v2;

        vec2 tv0_2D = vec2(tv0.x, tv0.y);
        vec2 tv1_2D = vec2(tv1.x, tv1.y);
        vec2 tv2_2D = vec2(tv2.x, tv2.y);

        // Check if the triangle intersects the AABB
        if (triangle_intersects_AABB(tv0_2D, tv1_2D, tv2_2D, bb_min, bb_max)) {
            return true; // Collision detected
        }
    }

    // No collision detected
    return false;
}





// This is a KINDA APPROXIMATE check that puts a retangle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box. 
bool CollisionSystem::collides(Entity entity1, Entity entity2)
{
	WorldObject& object1 = registry.worldObjects.get(entity1);
	WorldObject& object2 = registry.worldObjects.get(entity2);
	vec2 dp = object1.position - object2.position;
	vec2 half_extent1 = get_bounding_box(object1) / 2.f;
	vec2 half_extent2 = get_bounding_box(object2) / 2.f;

	// Check collision in x-axis
	if (std::abs(dp.x) < (half_extent1.x + half_extent2.x))
	{
		// Check collision in y-axis
		if (std::abs(dp.y) < (half_extent1.y + half_extent2.y))
		{
			// Collision detected
			if (registry.meshFlags.has(entity1)) {
				return meshCollides(entity1, entity2);
			}
			else if (registry.meshFlags.has(entity2)) {
				return meshCollides(entity2, entity1);
			}
			return true;
		}
	}
	// No collision
	return false;
}

/*
 - Check for collisions between all relevant entities
 - note that the order of the loops depends strongly on how many of each we have.
 - blockers are further in because presumably we'd only have a few (walls, accounting for doors) and we want to check them last
 - projectiles are first because we expect to have many of them
 - projectiles can only collide with one thing at a time
 - note that the order of processing is extremely important for projectiles
 - if a bullet is colliding with both a wall and a player, we are currently processing the player (and deadlys) first
 - players and deadlys can collide with multiple things at a time, which means the order of their processing is less important
 - this will probably be the source of a lot of bugs. if bug, check here
 - this has a lot of repetition that I don't know how to get rid of
*/
void CollisionSystem::add_collisions() {
	// Start with Projectiles. Projectiles can only collide with one thing at a time.
	for (Entity entity_projectile : registry.projectiles.entities)
	{
		WorldObject worldobject_projectile = registry.worldObjects.get(entity_projectile);
		if (registry.projectiles.get(entity_projectile).friendly)
		{
			// Check for projectile-deadly collisions
			// Check if we've already processed this entity (don't technically need it here, but adding in case I rearrange)
			if (registry.collisions.has(entity_projectile)) { continue; } // O(1)
			for (Entity entity_enemy : registry.deadlys.entities)
			{
				if (!registry.activeComponents.has(entity_enemy)) { continue; } // O(1
				WorldObject worldobject_enemy = registry.worldObjects.get(entity_enemy);
				if (collides(entity_projectile, entity_enemy))
				{
					assert(registry.collisions.has(entity_projectile) == false && "Projectile already collided with something");
					registry.collisions.emplace(entity_projectile, entity_enemy, COLLISION_TYPE::PROJECTILE_DEADLY);
					break; // we don't want any more collisions for this projectile
				}
			}
		}
		else {
			// Check for projectile-player collisions
			// Check if we've already processed this entity (don't technically need it here, but adding in case I rearrange)
			if (registry.collisions.has(entity_projectile)) { continue; } // O(1)
			WorldObject worldobject_player = registry.worldObjects.get(registry.players.entities[0]);
			if (collides(entity_projectile, registry.players.entities[0]))
			{
				assert(registry.collisions.has(entity_projectile) == false && "Projectile already collided with something");
				registry.collisions.emplace(entity_projectile, registry.players.entities[0], COLLISION_TYPE::PROJECTILE_PLAYER);
			}
		}
		// Check for projectile-blocker collisions
		// Check if we've already processed this entity
		if (registry.collisions.has(entity_projectile)) { continue; } // O(1)
		for (Entity entity_blocker : registry.blockers.entities)
		{
			if (!registry.activeComponents.has(entity_blocker)) { continue; } 
			WorldObject worldobject_blocker = registry.worldObjects.get(entity_blocker);
			if (collides(entity_projectile, entity_blocker))
			{
				assert(registry.collisions.has(entity_projectile) == false && "Projectile already collided with something");
				registry.collisions.emplace(entity_projectile, entity_blocker, COLLISION_TYPE::PROJECTILE_BLOCKER);
				break; // we don't want any more collisions for this projectile
			}
		}

	}

	// next, check player/deadly-blocker collisions
	for (Entity entity_blocker : registry.blockers.entities) {
		WorldObject worldobject_blocker = registry.worldObjects.get(entity_blocker);
		WorldObject worldobject_player = registry.worldObjects.get(registry.players.entities[0]);
		if (collides(entity_blocker, registry.players.entities[0]))
		{
			// i emplace with duplicates because player can collide with multiple things.
			registry.collisions.emplace_with_duplicates(registry.players.entities[0], entity_blocker, COLLISION_TYPE::PLAYER_BLOCKER);
			if (registry.collisions.has(registry.players.entities[0])) {
			}

		}
		for (Entity entity_deadly : registry.deadlys.entities) {
			WorldObject worldobject_deadly = registry.worldObjects.get(entity_deadly);
			if (collides(entity_blocker, entity_deadly))
			{
				// i emplace with duplicates because deadly can collide with multiple things.
				registry.collisions.emplace_with_duplicates(entity_deadly, entity_blocker, COLLISION_TYPE::DEADLY_BLOCKER);
			}
		}
	}

	// next, check player-deadly collisions
	for (Entity entity_deadly : registry.deadlys.entities)
	{
		WorldObject worldobject_deadly = registry.worldObjects.get(entity_deadly);
		WorldObject worldobject_player = registry.worldObjects.get(registry.players.entities[0]);
		Deadly& deadly = registry.deadlys.get(entity_deadly);
		if (collides(entity_deadly, registry.players.entities[0]))
		{
			registry.collisions.emplace_with_duplicates(registry.players.entities[0], entity_deadly, COLLISION_TYPE::PLAYER_DEADLY);
			deadly.attacking = true;
		}
	}

	// next, check for player-door collisions
	for (Entity entity_door : registry.doors.entities) {
		WorldObject worldobject_door = registry.worldObjects.get(entity_door);
		WorldObject worldobject_player = registry.worldObjects.get(registry.players.entities[0]);
		if (collides(entity_door, registry.players.entities[0]))
		{
			registry.collisions.emplace_with_duplicates(registry.players.entities[0], entity_door, COLLISION_TYPE::PLAYER_DOOR);
		}
	}
}