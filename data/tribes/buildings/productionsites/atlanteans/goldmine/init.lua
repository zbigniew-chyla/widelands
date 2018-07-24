dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_goldmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Gold Mine"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "mine",

   buildcost = {
      log = 7,
      planks = 4,
      spidercloth = 1
   },
   return_on_dismantle = {
      log = 3,
      planks = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 50, 56 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 50, 56 },
      },
      empty = {
         pictures = path.list_files(dirname .. "empty_??.png"),
         hotspot = { 50, 56 },
      },
   },

   aihints = {
      mines = "gold",
      prohibited_till = 1200
   },

   working_positions = {
      atlanteans_miner = 3
   },

   inputs = {
      { name = "smoked_fish", amount = 10 },
      { name = "smoked_meat", amount = 6 },
      { name = "atlanteans_bread", amount = 10 }
   },
   outputs = {
      "gold_ore"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining gold because ...
         descname = _"mining gold",
         actions = {
            "sleep=43000",
            "return=skipped unless economy needs gold_ore",
            "consume=smoked_fish,smoked_meat:2 atlanteans_bread:2",
            "animate=working 60000",
            "mine=gold 4 100 15 2",
            "produce=gold_ore",
            "sleep=2000",
            "mine=gold 4 100 1 2",
            "mine=gold 4 100 1 2",
            "produce=gold_ore:2"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Gold",
      heading = _"Main Gold Vein Exhausted",
      message =
         pgettext("atlanteans_building", "This gold mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider dismantling or destroying it."),
   },
}
