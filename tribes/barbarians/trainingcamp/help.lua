-- The Barbarian Training Camp

include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	-- need to get this again, so the building description will be of type "trainingsite"
	local building_description = wl.Game():get_building_description("barbarians", building_description.name)
	return

	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[‘He who is strong shall neither forgive nor forget, but revenge injustice suffered – in the past and for all future.’]], _[[Chief Chat’Karuth in a speech to his army.]]) ..

	--General Section
	building_help_general_string("barbarians", building_description, "soldier",
		_"Trains soldiers in Attack up to level %1$s, and in Health up to level %2$s."
			:bformat(building_description.max_attack+1,building_description.max_hp+1)
			.. "<br>" .."Equips the soldiers with all necessary weapons and armor parts.",
		_"Barbarian soldiers cannot be trained in ‘Defense’ and will remain at their initial level.") ..

	--Dependencies
	-- TODO still a lot of manual paramaters in here
	dependencies_training("barbarians", building_description, "fulltrained-evade", "untrained+evade") ..

	rt(h3(_"Training Attack:")) ..

	dependencies_training_weapons("barbarians", building_description, {"sharpax", "broadax"}, "axfactory") ..
	dependencies_training_weapons("barbarians", building_description,
		{"sharpax", "broadax", "bronzeax", "battleax", "warriorsax"}, "warmill") ..

	rt(h3(_"Training Health:")) ..
	dependencies_training_weapons("barbarians", building_description, {"helm", "mask", "warhelm"}, "helmsmithy") ..

	rt(h3(_"Training Both")) ..
	dependencies_training_food("barbarians", { {"fish", "meat"}, {"pittabread"}}) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description) ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If all needed wares are delivered in time, a training camp can train one new soldier in %1$s and %2$s to the final level in %3$s on average.":bformat(_"attack",_"health",_"%1$im%2$is":bformat(4,40)))
   end
}
