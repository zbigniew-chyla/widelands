dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_goldmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Gold Mine"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "mine",
   enhancement = "frisians_goldmine_deep",

   buildcost = {
      brick = 1,
      granite = 2,
      log = 2,
      thatch_reed = 1
   },
   return_on_dismantle = {
      brick = 1,
      granite = 1,
      log = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 58, 73 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), --TODO no animation yet
         hotspot = { 58, 73 },
      },
      empty = {
         pictures = path.list_files(dirname .. "empty_??.png"), --TODO no image yet
         hotspot = { 58, 73 },
      }
   },

   aihints = {
      mines = "gold",
      mines_percent = 50,
      prohibited_till = 1200
   },

   working_positions = {
      frisians_miner = 1
   },

   inputs = {
      { name = "ration", amount = 8 }
   },
   outputs = {
      "gold_ore"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining gold because ...
         descname = _"mining gold",
         actions = {
            "sleep=45000",
            "return=skipped unless economy needs gold_ore",
            "consume=ration",
            "animate=working 20000",
            "mine=gold 3 50 5 20", --name radius % chance_empty gain_exp_on_empty
            "produce=gold_ore"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Gold",
      heading = _"Main Gold Vein Exhausted",
      message =
         pgettext("frisians_building", "This gold mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider enhancing, dismantling or destroying it."),
   },
}
