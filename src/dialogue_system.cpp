#include "dialogue_system.hpp"

#include "help_system.hpp"

DialogueSystem::DialogueSystem()
{

}

DialogueSystem::~DialogueSystem() {

}

void DialogueSystem::init(RenderSystem* renderer_arg, GLFWwindow* window_arg) {
	this->renderer = renderer_arg;
	this->window = window_arg;
}

void reset_interacted_this_run(bool b) {
	//dialogue["interacted_this_run"] = true;
	//std::ofstream output_file(d_path);
	//if (!output_file) {
	//	std::cerr << "Failed to open file for writing: " << d_path << std::endl;
	//	return false;
	//}
	//try {
	//	output_file << dialogue.dump(4); // Pretty-print with 4-space indentation
	//}
	//catch (const std::exception& e) {
	//	std::cerr << "Failed to write JSON (interaction count and run_interaction_count) to file: " << e.what() << std::endl;
	//	return false;
	//}
	//output_file.close(); // Close the output file
}

bool DialogueSystem::load_dialogue() {
    // Get NPC info
    assert(registry.dialogueStates.size() > 0 && "NO DIALOGUE STATES");
    assert(registry.dialogueStates.size() == 1 && "MULTIPLE DIALOGUE STATES");
    Entity npc = registry.dialogueStates.components[0].talking_to;
    NPC& npc_data = registry.NPCs.get(npc);
    std::string d_path = npc_data.dialogue_path;
    current_sent = 0;
    valid_textbox = false;

    // Read the dialogue file
    std::ifstream input_file(d_path);
    if (!input_file) {
        std::cerr << "Failed to open file: " << d_path << std::endl;
        return false;
    }
    try {
        input_file >> dialogue;
    }
    catch (const json::parse_error& e) {
        std::cerr << "Parse failed: " << e.what() << std::endl;
        return false;
    }
    input_file.close(); // Close the input file

    // Determine the current run
    if (!dialogue.contains("current_run") || dialogue["current_run"].is_null() || !dialogue.contains("interacted_this_run") || !dialogue["interacted_this_run"]) {
        if (!dialogue.contains("next_run") || dialogue["next_run"].is_null()) {
            std::cout << "No more runs!" << std::endl;
            current_sent = -1;
            return false;
        }
        // Set current_run from next_run since it's the first interaction of this run
        dialogue["current_run"] = dialogue["next_run"];
    }
    current_run = dialogue["current_run"];

    std::string run_key = "run" + std::to_string(current_run);

    // Only update next_run if we haven't interacted this run yet
    if (!dialogue.contains("interacted_this_run") || !dialogue["interacted_this_run"]) {
        if (!dialogue.contains(run_key)) {
            std::cerr << "Run key " << run_key << " not found in dialogue." << std::endl;
            return false;
        }

        if (dialogue[run_key].contains("next_run") && !dialogue[run_key]["next_run"].is_null()) {
            dialogue["next_run"] = dialogue[run_key]["next_run"];
        }
        else {
            dialogue["next_run"] = nullptr;  // No more runs
        }

        // Mark that we've interacted this run
        dialogue["interacted_this_run"] = true;
    }

    // Get current interaction within the run
    if (!dialogue[run_key].contains("next_interaction") || dialogue[run_key]["next_interaction"].is_null()) {
        std::cout << "No more interactions in run " << current_run << "!" << std::endl;
        current_sent = -1;
        return false;
    }
    current_interaction = dialogue[run_key]["next_interaction"];

    // Update the interaction count
    std::string interaction_key = "int" + std::to_string(current_interaction);

    if (!dialogue[run_key].contains(interaction_key)) {
        std::cerr << "Interaction key " << interaction_key << " not found in run " << run_key << "." << std::endl;
        return false;
    }

    std::string next_interaction_value = dialogue[run_key][interaction_key]["next"];

    if (next_interaction_value == "none" || next_interaction_value.empty()) {
        dialogue[run_key]["next_interaction"] = nullptr;  // No more interactions
    }
    else {
        // Extract next interaction number from "intX"
        int next_interaction = std::stoi(next_interaction_value.substr(3)); // Skip "int"
        dialogue[run_key]["next_interaction"] = next_interaction;
    }

    // Write updated dialogue back to file
    std::ofstream output_file(d_path);
    if (!output_file) {
        std::cerr << "Failed to open file for writing: " << d_path << std::endl;
        return false;
    }
    try {
        output_file << dialogue.dump(4); // Pretty-print with 4-space indentation
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to write JSON to file: " << e.what() << std::endl;
        return false;
    }
    output_file.close();

    return true;
}



void DialogueSystem::delete_previous_textbox() {
	if (valid_textbox) {
		// delete the previous textbox
		TextBox& to_del_textbox = registry.textBoxes.get(current_textbox);
		Entity to_del_text = to_del_textbox.textbox_text;
		Entity to_del_texture = to_del_textbox.textbox_sprite;
		registry.remove_all_components_of(to_del_text);
		registry.remove_all_components_of(to_del_texture);
		registry.remove_all_components_of(current_textbox);
	}
}

bool DialogueSystem::continue_dialogue() {
	// Remove previous textbox
	delete_previous_textbox();
	valid_textbox = false;

	if (current_sent == -1) {
		printf("No more sentences.\n");
		return false;
	}

	assert(current_interaction != -1 && "CURRENT INTERACTION SHOULD NOT BE -1");

	std::string run_key = "run" + std::to_string(current_run);
	std::string interaction_key = "int" + std::to_string(current_interaction);
	std::string sent_key = "sent" + std::to_string(current_sent);

	if (dialogue[run_key][interaction_key][sent_key].is_null()) {
		// If the sentence doesn't exist, end the dialogue
		return false;
	}

	std::string text = dialogue[run_key][interaction_key][sent_key]["text"];

	current_textbox = UISystem::createTextBox(
		renderer,
		vec2(window_width_px / 2, window_height_px / 2),
		vec2(DIALOGUE_BOX_WIDTH, DIALOGUE_BOX_HEIGHT),
		DIALOGUE_TEXT_COLOR,
		SCENE_TYPE::DIALOGUE,
		text
	);
	valid_textbox = true;

	std::string next_sent = dialogue[run_key][interaction_key][sent_key]["next_sent"];

	if (next_sent == "none" || next_sent.empty()) {
		// If there is no next sentence, set current_sent to -1 to end the dialogue upon next continue
		current_sent = -1;
	}
	else {
		// Extract the sentence number from the next_sent key
		current_sent = std::stoi(next_sent.substr(4)); // substr skips "sent"
	}

	return true;
}

void DialogueSystem::end_dialogue() {
	printf("dialogue done. going back to game.\n");
	delete_previous_textbox();
	scene_manager.set_scene(SCENE_TYPE::GAME);
}

void DialogueSystem::on_mouse_button(GLFWwindow* window, int button, int action, int mods)
{
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		if (!continue_dialogue()) {
			end_dialogue();
		}
	}
	
}

bool DialogueSystem::is_mouse_within_button(WorldObject buttonObject)
{
	// TODO: this method should be common to every system lmao
	return false;
}

void DialogueSystem::on_key(int key, int sc, int action, int mod) {
	if (action == GLFW_PRESS && key == GLFW_KEY_E) {
		if (!continue_dialogue()) {
			end_dialogue();
		}
	}
}

// if we switch to C++ 17, we can use std::filesystem and won't need a list of files
bool reset_dialogue_run_status() {
    // List your dialogue file names here
    std::vector<std::string> dialogue_files = {
        "old_robot.json",
        "scarecrow.json",
        // Add all your dialogue file names
    };

    for (const auto& filename : dialogue_files) {
        std::string file_path = dialogue_path(filename);

        std::ifstream input_file(file_path);
        if (!input_file) {
            std::cerr << "Failed to open file: " << file_path << std::endl;
            continue;
        }

        json dialogue;
        try {
            input_file >> dialogue;
        }
        catch (const json::parse_error& e) {
            std::cerr << "Parse failed for file " << file_path << ": " << e.what() << std::endl;
            input_file.close();
            continue;
        }
        input_file.close();

        // Reset the fields
        dialogue["interacted_this_run"] = false;
        dialogue["current_run"] = nullptr;

        // Write back to the file
        std::ofstream output_file(file_path);
        if (!output_file) {
            std::cerr << "Failed to open file for writing: " << file_path << std::endl;
            continue;
        }
        try {
            output_file << dialogue.dump(4);
        }
        catch (const std::exception& e) {
            std::cerr << "Failed to write JSON to file " << file_path << ": " << e.what() << std::endl;
            output_file.close();
            continue;
        }
        output_file.close();
    }

    return true;
}
