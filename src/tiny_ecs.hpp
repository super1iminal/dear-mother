#pragma once

#include <algorithm>
#include <vector>
#include <unordered_map>
#include <set>
#include <functional>
#include <typeindex>
#include <assert.h>
#include <numeric>

// Unique identifier for all entities
class Entity
{
    unsigned int id;
    static unsigned int id_count; // starts from 1, entity 0 is the default initialization
public:
    Entity()
    {
        id = id_count++;
        // Note, indices of already deleted entities aren't re-used in this simple implementation.
    }
    operator unsigned int() { return id; } // this enables automatic casting to int
    bool operator==(const Entity& other) const { return id == other.id; }
    bool operator!=(const Entity& other) const { return id != other.id; }
};

// Common interface to refer to all containers in the ECS registry
struct ContainerInterface
{
    virtual void clear() = 0;
    virtual size_t size() = 0;
    virtual void remove(Entity e) = 0;
    virtual bool has(Entity entity) = 0;
};

// A container that stores components of type 'Component' and associated entities
template <typename Component> // A component can be any class
class ComponentContainer : public ContainerInterface
{
private:
    // The hash map from Entity -> array index.
    std::unordered_multimap<unsigned int, unsigned int> map_entity_componentID;
    bool registered = false;
    bool sorted = false;

    std::vector<Entity> components_entities;

    std::vector<std::function<void(Entity)>> onEntityAddedCallbacks;
    std::vector<std::function<void(Entity)>> onEntityRemovedCallbacks;

    // check if a component has compareFunction, which is necessary for sorting
    // using SFINAE (Substitution Failure Is Not An Error)
    template <typename Component>
    class has_compareFunction
    {
    private:
        // type (Component) to be checked
        template <typename U>

        // both of the test functions do not have implementations; this is okay,
        // since we only intend to use them for compile-time type checking

        // test if the type U has compareFunction, by attempting to call it. 
        // this will succeed if type U does have comapreFunction. returns std::true_type
        static auto test(int) -> decltype(U::compareFunction(std::declval<U>(), std::declval<U>()), std::true_type());

        // if the above test function fails, this is the fallback that is chosen. 
        // it uses a variadic arugment (...) to accept anything, and returns std::false_type
        template <typename>
        static std::false_type test(...);
    public:
        // The value will be true if the first "test" function is selected, false otherwise.
        static constexpr bool value = decltype(test<Component>(0))::value;
    };

public:
    // Container of all components of type 'Component'
    std::vector<Component> components;

    // The set of unique entities
    std::vector<Entity> entities;

    // Constructor that registers the type
    ComponentContainer()
    {
    }

    // Inserting a component c associated to entity e
    inline Component& insert(Entity e, Component c, bool check_for_duplicates = true)
    {
        if (check_for_duplicates)
        {
            // We no longer assert if the entity already has components
            // Instead, we proceed to add the new component
        }
        if (map_entity_componentID.count(e) == 0)
        {
            // Entity e is not in entities vector yet, so add it (order doesn't matter)
            entities.push_back(e);
        }
        map_entity_componentID.insert(std::make_pair(e, (unsigned int)components.size()));
        components.push_back(std::move(c));
        components_entities.push_back(e);
        // add to filtered components
        for (auto& callback : onEntityAddedCallbacks) {
            callback(e);
        }
        return components.back();
    };

    // Inserting a component c associated to entity e
    inline Component& insert_sorted(Entity e, Component c, bool check_for_duplicates = true)
    {
        if (check_for_duplicates)
        {
            // We no longer assert if the entity already has components
            // Instead, we proceed to add the new component
        }
        if (map_entity_componentID.count(e) == 0)
        {
            static_assert(has_compareFunction<Component>::value && "Component does not have compareFunction defined");
            if constexpr (has_compareFunction<Component>::value) {  // check if this component has compareFunction defined
                bool inserted = false;
                for (int i = 0; i < entities.size(); i++) {
                    if (c.compareFunction(c, get(entities[i])) < 1) {  // if this entity should come before entities[i]
                        entities.insert(entities.begin() + i, e);
                        inserted = true;
                        break;
                    }
                }
                if (!inserted) {
                    // entity e goes in at the end of the vector
                    entities.push_back(e);
                }
            }
        }
        map_entity_componentID.insert(std::make_pair(e, (unsigned int)components.size()));
        components.push_back(std::move(c));
        components_entities.push_back(e);
        // add to filtered components
        for (auto& callback : onEntityAddedCallbacks) {
            callback(e);
        }
        return components.back();
    };

    // The emplace function takes the the provided arguments Args, creates a new object of type Component, and inserts it into the ECS system
    template<typename... Args>
    Component& emplace(Entity e, Args &&... args) {
        return insert(e, Component(std::forward<Args>(args)...));
    };
    template<typename... Args>
    Component& emplace_sorted(Entity e, Args &&... args) {
        assert(sorted && "ComponentContainer is not sorted");
        return insert_sorted(e, Component(std::forward<Args>(args)...));
    };
    template<typename... Args>
    Component& emplace_with_duplicates(Entity e, Args &&... args) {
        return insert(e, Component(std::forward<Args>(args)...), false);
    };

    std::vector<Component*> get_all(Entity e) {
        std::vector<Component*> result;
        auto range = map_entity_componentID.equal_range(e);
        for (auto it = range.first; it != range.second; ++it) {
            result.push_back(&components[it->second]);
        }
        return result;
    }

    // A wrapper to return the component of an entity
    Component& get(Entity e) {
        assert(has(e) && "Entity not contained in ECS registry");
        auto range = map_entity_componentID.equal_range(e);
        return components[range.first->second]; // Returns the first component found
    }

    // O(1)
    // Check if entity has a component of type 'Component'
    bool has(Entity entity) {
        return map_entity_componentID.count(entity) > 0;
    }

    // Remove a component and pack the container to re-use the empty space
    void remove(Entity e)
    {
		if (!has(e)) {
			return;
		}
        auto range = map_entity_componentID.equal_range(e);
        std::vector<unsigned int> indices_to_remove;
        for (auto it = range.first; it != range.second; ++it) {
            indices_to_remove.push_back(it->second);
        }
        // Sort indices in reverse order
        std::sort(indices_to_remove.rbegin(), indices_to_remove.rend());
        for (auto idx : indices_to_remove) {
            unsigned int last_idx = components.size() - 1;
            if (idx != last_idx) {
                // Swap last component into idx
                components[idx] = std::move(components.back());
                components_entities[idx] = components_entities.back();
                // Update the map for the swapped entity
                auto swapped_entity = components_entities[idx];
                auto range_swapped = map_entity_componentID.equal_range(swapped_entity);
                for (auto it = range_swapped.first; it != range_swapped.second; ++it) {
                    if (it->second == last_idx) {
                        it->second = idx;
                        break;
                    }
                }
            }
            // Erase the mapping for the entity and index
            auto it = map_entity_componentID.find(e);
            while (it != map_entity_componentID.end() && it->first == e) {
                if (it->second == idx) {
                    it = map_entity_componentID.erase(it);
                    break;
                }
                else {
                    ++it;
                }
            }
            components.pop_back();
            components_entities.pop_back();
        }
        // Remove entity from entities vector and filteredcomponents if no more components associated with it
        if (map_entity_componentID.count(e) == 0) {
            auto it = std::find(entities.begin(), entities.end(), e);
            if (it != entities.end()) {
                entities.erase(it);
            }
            for (auto& callback : onEntityRemovedCallbacks) {
                callback(e);
            }
        }
    };

    // Remove all components of type 'Component'
    void clear()
    {
        map_entity_componentID.clear();
        components.clear();
        components_entities.clear();
        entities.clear();
    }

    // Report the number of components of type 'Component'
    size_t size()
    {
        return components.size();
    }

    // Sort the components and associated entity assignment structures by the comparisonFunction, see std::sort
    template <class Compare>
    void sort(Compare comparisonFunction)
    {
        // Sort the components and components_entities
        std::vector<size_t> indices(components.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::sort(indices.begin(), indices.end(), [&](size_t a, size_t b) {
            return comparisonFunction(components_entities[a], components_entities[b]);
            });
        // Apply the sorted order to components and components_entities
        std::vector<Component> sorted_components;
        std::vector<Entity> sorted_components_entities;
        sorted_components.reserve(components.size());
        sorted_components_entities.reserve(components_entities.size());
        for (auto idx : indices) {
            sorted_components.push_back(std::move(components[idx]));
            sorted_components_entities.push_back(components_entities[idx]);
        }
        components = std::move(sorted_components);
        components_entities = std::move(sorted_components_entities);
        // Rebuild the multimap
        map_entity_componentID.clear();
        for (unsigned int i = 0; i < components_entities.size(); ++i) {
            map_entity_componentID.insert(std::make_pair(components_entities[i], i));
        }
    }
    void registerOnAddCallback(const std::function<void(Entity)>& callback) {
        onEntityAddedCallbacks.push_back(callback);
    }

    void registerOnRemoveCallback(const std::function<void(Entity)>& callback) {
        onEntityRemovedCallbacks.push_back(callback);
    }

    void setSorted(bool sorted_value) {
        sorted = sorted_value;
    }
};

// get and get all return components from the Component (not the FilterComponent).
// don't need to implment insert, get, remove and get all, should honestly remove em
template <typename Component, typename FilterComponent>
class FilteredComponentContainer : public ContainerInterface {
private:
    ComponentContainer<Component>& componentCC;
    ComponentContainer<FilterComponent>& filterCC;

    // Callback functions
    void onComponentAdded(Entity e) {
        if (filterCC.has(e)) {
            if (std::find(entities.begin(), entities.end(), e) == entities.end()) {
                entities.push_back(e);
            }
        }
    }

    void onComponentRemoved(Entity e) {
        if (!componentCC.has(e) || !filterCC.has(e)) {
            auto it = std::find(entities.begin(), entities.end(), e);
            if (it != entities.end()) {
                entities.erase(it);
            }
        }
    }

    void onFilterComponentAdded(Entity e) {
        if (componentCC.has(e)) {
            if (std::find(entities.begin(), entities.end(), e) == entities.end()) {
                entities.push_back(e);
            }
        }
    }

    void onFilterComponentRemoved(Entity e) {
        if (!componentCC.has(e) || !filterCC.has(e)) {
            auto it = std::find(entities.begin(), entities.end(), e);
            if (it != entities.end()) {
                entities.erase(it);
            }
        }
    }

public:
    // Store entities that have both components
    std::vector<Entity> entities;
    FilteredComponentContainer(ComponentContainer<Component>& componentCC,
        ComponentContainer<FilterComponent>& filterCC)
        : componentCC(componentCC), filterCC(filterCC) {
        // Register callbacks
        componentCC.registerOnAddCallback([this](Entity e) { onComponentAdded(e); });
        componentCC.registerOnRemoveCallback([this](Entity e) { onComponentRemoved(e); });
        filterCC.registerOnAddCallback([this](Entity e) { onFilterComponentAdded(e); });
        filterCC.registerOnRemoveCallback([this](Entity e) { onFilterComponentRemoved(e); });

        // Initialize entities vector
        for (const auto& e : componentCC.entities) {
            if (filterCC.has(e)) {
                entities.push_back(e);
            }
        }
    }

    // Implement required methods from ContainerInterface
    void clear() override {
        entities.clear();
    }

    size_t size() override {
        return entities.size();
    }

    bool has(Entity e) override {
        return std::find(entities.begin(), entities.end(), e) != entities.end();
    }

    void remove(Entity e) override {
        componentCC.remove(e);
        filterCC.remove(e);
        auto it = std::find(entities.begin(), entities.end(), e);
        if (it != entities.end()) {
            entities.erase(it);
        }
    }

    // Access components
    Component& get(Entity e) {
        assert(has(e) && "Entity not contained in filtered container");
        return componentCC.get(e);
    }

    std::vector<Component*> get_all(Entity e) {
        assert(has(e) && "Entity not contained in filtered container");
        return componentCC.get_all(e);
    }

    // You can add insert methods if needed, ensuring they update both base containers
};
