// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <allegro5/allegro.h>
#include <allegro5/allegro_physfs.h>
#include <luajit-2.1/lua.h>
#include <physfs.h>
#include <stdio.h>

#include "log.h"
#include "main.h"
#include "model.h"
#include "subscription.h"
#include "view.h"

ir_engine ENGINE;

int ir_run_opts(int argc, char *argv[], lua_State *L) {
	char args[BUFSIZ];
	int key = 0;
	int len = 0;
	int stage = 0;
	int value = 0;

	lua_createtable(L, 0, 0);

	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			strncpy(args + len, argv[i], BUFSIZ - len);
			len += strlen(argv[i]);
			if (len > BUFSIZ) {
				ir_error("ir_run_opts: Arguments list too long");
				return 1;
			}
			if (i == argc - 1) {
				args[len] = 0;
			} else {
				args[len++] = ' ';
			}
		}

		for (int i = 0; i < len; i++) {
			switch (stage) {
			case 0:
				if (args[i] == '+') {
					key = i + 1;
					stage = 1;
				}
				break;
			case 1:
				if (args[i] == '=') {
					if (key == i) {
						stage = 0;
					} else {
						args[i] = 0;
						value = i + 1;
						stage = 2;
					}
				}
				break;
			case 2:
				if (value == i && args[i] == '"') {
					value++;
					stage = 3;
				} else if (args[i] == ' ') {
					args[i] = 0;
					lua_pushstring(L, args + value);
					lua_setfield(L, -2, args + key);
					stage = 0;
				} else if (i + 1 == len) {
					lua_pushstring(L, args + value);
					lua_setfield(L, -2, args + key);
					// This probably doesn't matter but my gut tells me that
					// something will go terribly wrong if I don't do this.
					// ~ahill
					stage = 0;
				}
				break;
			case 3:
				if (args[i] == '"') {
					args[i] = 0;
					lua_pushstring(L, args + value);
					lua_setfield(L, -2, args + key);
					stage = 0;
				}
				break;
			}
		}
	}

	return 0;
}

void ir_run_user_ini() {
	uint8_t *buffer;
	FILE *file;
	PHYSFS_File *file_physfs;
	PHYSFS_sint64 length;

	if((file_physfs = PHYSFS_openRead("user.ini"))) {
		length = PHYSFS_fileLength(file_physfs);
		if((buffer = malloc(length))) {
			if(PHYSFS_readBytes(file_physfs, buffer, length) == length) {
				if((file = fopen("user.ini", "w"))) {
					fwrite(buffer, 1, length, file);
					fclose(file);
				}
			}
			free(buffer);
		}
		PHYSFS_close(file_physfs);
	}
}

int ir_run_user(lua_State *L) {
	ALLEGRO_CONFIG *config = al_load_config_file("user.ini");
	ALLEGRO_CONFIG_ENTRY *entry;
	const char *key;
	ALLEGRO_CONFIG_SECTION *section;
	const char *section_name;

	// If we couldn't open the file, copy the user preferences file from the game and use that instead. ~ahill
	if(!config && PHYSFS_exists("user.ini")) {
		ir_info("ir_run_user: User preferences missing but a template exists. Copying.");
		ir_run_user_ini();
		config = al_load_config_file("user.ini");
	}

	lua_getglobal(L, "ir");

	lua_createtable(L, 0, 0);

	if(config) {
		section_name = al_get_first_config_section(config, &section);
		while(section_name) {
			lua_createtable(L, 0, 0);

			key = al_get_first_config_entry(config, section_name, &entry);
			while(key) {
				lua_pushstring(L, al_get_config_value(config, section_name, key));
				lua_setfield(L, -2, key);
				key = al_get_next_config_entry(&entry);
			}

			lua_setfield(L, -2, section_name);
			section_name = al_get_next_config_section(&section);
		}

		al_destroy_config(config);
	} else {
		ir_warn("ir_run_user: \"user.ini\" not found. Expect strange behavior!");
	}

	lua_setfield(L, -2, "user");
	lua_pop(L, 1);

	return 0;
}

int ir_run(int argc, char *argv[], ir_engine *engine) {
	const char *archive_name;

	lua_getglobal(engine->model.state, "ir");
	if (!lua_istable(engine->model.state, -1)) {
		ir_error("ir_run: Failed to get ir table");
		lua_pop(engine->model.state, 1);
		return 1;
	}

	lua_getfield(engine->model.state, -1, "kernel");
	if (!lua_isfunction(engine->model.state, -1)) {
		ir_error("ir_run: Failed to get ir.kernel");
		lua_pop(engine->model.state, 2);
		return 1;
	}

	if (ir_run_opts(argc, argv, engine->model.state)) {
		lua_pop(engine->model.state, 2);
		return 1;
	}

	// Figure out what the main archive is called and load it. ~ahill
	lua_getfield(engine->model.state, -1, "game");
	if(lua_isstring(engine->model.state, -1)) {
		archive_name = lua_tostring(engine->model.state, -1);
	} else {
		archive_name = "game.zip";
	}
	lua_pop(engine->model.state, 1);

	if(!PHYSFS_mount(archive_name, NULL, true)) {
		ir_error("ir_run: Failed to mount %s", archive_name);
		lua_pop(engine->model.state, 2);
		return 1;
	}

	if (ir_run_user(engine->model.state)) {
		lua_pop(engine->model.state, 2);
		return 1;
	}

	if (lua_pcall(engine->model.state, 1, 0, 0)) {
		ir_error(
			"ir_run: Failed to run ir.kernel: %s",
			lua_tostring(engine->model.state, -1)
		);
		lua_pop(engine->model.state, 2);
		return 1;
	}

	lua_pop(engine->model.state, 1);
	PHYSFS_unmount(archive_name);

	return 0;
}

int main(int argc, char *argv[]) {
	int status = 1;

	// al_init registers an atexit function to clean itself up later. ~ahill
	if (!al_init()) {
		// Substituting ir_error for good ol' printf because al_get_time causes
		// undefined behavior if Allegro hasn't been initialized! ~ahill
		printf("00000.0000 EROR main: Failed to initialize Allegro\r\n");
		return 1;
	}

	if (PHYSFS_init(argv[0])) {
		// Is this even needed? It messes with loading user.ini. ~ahill
		// al_set_physfs_file_interface();
		if (!ir_model_new(&ENGINE.model)) {
			if (!ir_subscription_new(&ENGINE.subs)) {
				if (!ir_view_new(&ENGINE.view)) {
					if (!ir_run(argc, argv, &ENGINE)) {
						status = 0;
					}
					ir_view_drop(&ENGINE.view);
				}
				ir_subscription_drop(&ENGINE.subs);
			}
			ir_model_drop(&ENGINE.model);
		}
		PHYSFS_deinit();
	} else {
		ir_error("main: Failed to initialize PhysicsFS");
	}

	return status;
}