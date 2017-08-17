-- =======================================================================
--                                 Player 1
-- =======================================================================
p1:forbid_buildings("all")
p1:allow_buildings{"frisians_sentinel","frisians_warehouse"}

hq = p1:place_building("frisians_headquarters",
   wl.Game().map.player_slots[1].starting_field, false, true)
hq:set_wares {
   log = 40,
   brick = 50,
   clay = 20,
   granite = 40,
   water = 40,
   coal = 20,
   thatch_reed = 20,
   fruit = 10,
   fish = 10,
   meat = 10,
   smoked_fish = 10,
   smoked_meat = 10,
   bread_frisians = 10,
   ration = 30,
   iron = 2,
   iron_ore = 5,
   gold_ore = 1,
}
hq:set_workers {
   frisians_woodcutter = 3,
   frisians_forester = 5,
   frisians_clay_burner = 4,
   frisians_builder = 10,
   frisians_blacksmith = 4,
   frisians_miner = 5,
   frisians_smelter = 2,
   frisians_smoker = 2,
   frisians_seamstress = 2,
   frisians_landlady = 3,
   frisians_berry_farmer = 3,
   frisians_fruit_collector = 3,
   frisians_beekeeper = 2,
   frisians_fisher = 3,
   frisians_hunter = 1,
   frisians_geologist = 2,
   frisians_farmer = 3,
   frisians_reed_farmer = 2,
   frisians_baker = 1,
   frisians_brewer = 1,
   frisians_trainer = 3,
}
hq:set_soldiers({0,0,0,0}, 20)

-- =======================================================================
--                                 Player 2
-- =======================================================================
p2:forbid_buildings("all")
p2:allow_buildings{
   "frisians_woodcutters_house",
   "frisians_foresters_house",
   "frisians_well",
   "frisians_claypit",
   "frisians_brick_burners_house",
   "frisians_fishers_house",
   "frisians_hunters_house",
   "frisians_quarry",
   "frisians_smokery",
   "frisians_tavern",
   "frisians_coalmine",
   "frisians_ironmine",
   "frisians_goldmine",
   "frisians_rockmine",
   "frisians_coalmine_deep",
   "frisians_ironmine_deep",
   "frisians_goldmine_deep",
   "frisians_rockmine_deep",
   "frisians_farm",
   "frisians_bakery",
   "frisians_brewery",
   "frisians_furnace",
   "frisians_blacksmithy",
   "frisians_mead_brewery",
   "frisians_honey_bread_bakery",
   "frisians_drinking_hall",
   "frisians_sentinel",
   "frisians_outpost"}

hq2 = p2:place_building("frisians_headquarters",
   wl.Game().map.player_slots[2].starting_field, false, true)
hq2:set_wares {
   log = 100,
   brick = 150,
   clay = 50,
   granite = 100,
   thatch_reed = 50,
   water = 50,
   coal = 100,
   iron = 50,
   gold = 5,
}
hq2:set_workers {
   frisians_woodcutter = 5,
   frisians_forester = 10,
   frisians_clay_burner = 5,
   frisians_builder = 20,
   frisians_blacksmith = 5,
   frisians_miner = 10,
   frisians_smelter = 4,
   frisians_smoker = 6,
   frisians_landlady = 5,
   frisians_berry_farmer = 5,
   frisians_fruit_collector = 5,
   frisians_beekeeper = 4,
   frisians_fisher = 10,
   frisians_hunter = 2,
   frisians_geologist = 3,
   frisians_farmer = 10,
   frisians_reed_farmer = 5,
   frisians_baker = 4,
   frisians_brewer = 4,
   frisians_reindeer = 20,
}
hq2:set_soldiers({0,0,0,0}, 100)
