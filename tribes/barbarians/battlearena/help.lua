include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return
		--rt(h1(_"The Barbarian Battle Arena")) ..
	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[‘No better friend you have in battle than the enemy’s blow that misses.’]], _[[Said to originate from Neidhardt, the famous trainer.]]) ..

	--General Section TODO string design
	building_help_general_string("barbarians", building_description, "soldier",
		rt(_[[‘Evade’ increases the soldier’s chance not to be hit by the enemy and so to remain totally unaffected.]]) ..
		rt("_</p><p font-weight=bold font-decoration=underline font-size=12>Note: </p>_<p font-size=12>Barbarian soldiers cannot be trained in \"Defense\" and will remain at the level with which they came.<br><br></p>"),
		_"Trains soldiers in Evade up to level 2.",
		"--") ..

	--Dependencies TODO
		rt(h2(_"Dependencies")) ..
		dependencies_old({"tribes/barbarians/soldier/untrained.png","tribes/barbarians/battlearena/menu.png","tribes/barbarians/soldier/untrained+evade.png"}) ..
		dependencies_old({"tribes/barbarians/soldier/fulltrained-evade.png","tribes/barbarians/battlearena/menu.png","tribes/barbarians/soldier/fulltrained.png"}) ..
		rt(h3(_"Evade Training:")) ..
		image_line("tribes/barbarians/pittabread/menu.png",1,p(_"%s and":format(_"Pitta Bread"))) ..
		image_line("tribes/barbarians/strongbeer/menu.png",1,p(_"%s and":format(_"Strong Beer"))) ..
		image_line("tribes/barbarians/fish/menu.png;tribes/barbarians/meat/menu.png",1,p(_"%s or %s":format(_"Fish",_"Meat"))) ..
	--Building Section
	building_help_building_section("barbarians", building_description) ..

		--rt(h2(_"Building")) ..
		--text_line(_"Upgraded from:", "n/a") ..
		--rt(h3(_"Build cost:")) ..
		--help_building_line("barbarians", "gold", ngettext("%i Gold", "%i Gold", 4), 4) ..
		--help_building_line("barbarians", "thatchreed", ngettext("%i Thatch Reed", "%i Thatch Reeds", 3), 3) ..
		--help_building_line("barbarians", "grout", ngettext("%i Grout", "%i Grout", 6), 6) ..
		--help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 4), 4) ..
		--help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 6), 6) ..
		--rt(h3(_"Dismantle yields:")) ..
		--help_building_line("barbarians", "gold", ngettext("%i Gold", "%i Gold", 2), 2) ..
		--help_building_line("barbarians", "thatchreed", ngettext("%i Thatch Reed", "%i Thatch Reeds", 2), 2) ..
		--help_building_line("barbarians", "grout", ngettext("%i Grout", "%i Grout", 3), 3) ..
		--help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 2), 2) ..
		--help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 3), 3) ..
		--text_line(_"Upgradeable to:","n/a") ..
	--Workers Section
	building_help_crew_string("barbarians", building_description, {"trainer"}) ..

		--rt(h2(_"Workers")) ..
		--rt(h3(_"Crew required:")) ..
		--image_line("tribes/barbarians/trainer/menu.png", 1, p(_"Trainer")) ..
		--text_line(_"Worker uses:","n/a") ..
		--text_line(_"Experience levels:", "n/a") ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If all needed wares are delivered in time, a battle arena can train %1$s for one soldier from 0 to the highest level in %2$s on average.":bformat(_"evade",_"%1$im%2$is":bformat(1,10)))
   end
}
