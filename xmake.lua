-- This Source Code Form is subject to the terms of the Mozilla Public
-- License, v. 2.0. If a copy of the MPL was not distributed with this
-- file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_requires("allegro", "luajit")
add_rules("mode.debug", "mode.release")

target("iridium")
    set_kind("binary")
    set_languages("clatest")
    set_warnings("everything")
    add_packages("allegro", "luajit")
    add_files("src/**.c")