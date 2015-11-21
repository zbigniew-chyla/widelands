dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_ironmine_deeper",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Deeper Iron Mine"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "mine",

   enhancement_cost = {
      log = 4,
      granite = 2
   },
   return_on_dismantle_on_enhanced = {
      log = 2,
      granite = 1
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 60, 37 },
      },
      build = {
         template = "build_??",
         directory = dirname,
         hotspot = { 60, 37 },
      },
      working = {
         template = "working_??",
         directory = dirname,
         hotspot = { 60, 37 },
      },
      empty = {
         template = "empty_??",
         directory = dirname,
         hotspot = { 60, 37 },
      },
   },

   aihints = {
      mines = "iron",
   },

   working_positions = {
      barbarians_miner = 1,
      barbarians_miner_chief = 1,
      barbarians_miner_master = 1,
   },

   inputs = {
      meal = 6
   },
   outputs = {
      "iron_ore"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining iron because ...
         descname = _"mining iron",
         actions = {
            "sleep=40000",
            "return=skipped unless economy needs iron_ore",
            "consume=meal",
            "animate=working 16000",
            "mine=iron 2 100 10 2",
            "produce=iron_ore",
            "animate=working 16000",
            "mine=iron 2 100 10 2",
            "produce=iron_ore:2",
            "animate=working 16000",
            "mine=iron 2 100 10 2",
            "produce=iron_ore:2"
         }
      },
   },
   out_of_resource_notification = {
      title = _"Main Iron Vein Exhausted",
      message =
         pgettext("barbarians_building", "This iron mine’s main vein is exhausted. Expect strongly diminished returns on investment. This mine can’t be enhanced any further, so you should consider dismantling or destroying it."),
   },
}