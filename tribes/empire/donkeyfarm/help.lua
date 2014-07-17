-- The Imperial Donkey Farm

include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_empire")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("empire", building_description, _[[Text needed]], _[[Source needed]]) ..

	--General Section
	building_help_general_string("empire", building_description, "donkey",
		_"Breeds donkeys for adding them to the transportation system.") ..

	--Dependencies
	-- TODO: why is "Produces" empty?
	building_help_dependencies_production("empire", building_description) ..

	--Workers Section
	building_help_crew_string("empire", building_description) ..

	--Building Section
	building_help_building_section("empire", building_description) ..

	--Production Section
	building_help_production_section(_[[Calculation needed]])
  end
}