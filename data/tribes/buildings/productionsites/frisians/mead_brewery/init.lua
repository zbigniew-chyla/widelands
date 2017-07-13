dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_mead_brewery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Mead Brewery"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement_cost = {
      brick = 4,
      granite = 1,
      log = 1,
      thatch_reed = 2
   },
   return_on_dismantle_on_enhanced = {
      brick = 5,
      granite = 2,
      log = 2,
      thatch_reed = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 56, 47 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 56, 47 },
      },
   },

   aihints = {
      prohibited_till = 1200
   },

   working_positions = {
      frisians_brewer = 1,
      frisians_brewer_master = 1
   },

   inputs = {
      { name = "barley", amount = 8 },
      { name = "water", amount = 8 },
      { name = "honey", amount = 6 },
   },
   outputs = {
      "mead"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start brewing mead because ...
         descname = _"brewing mead",
         actions = {
            "sleep=20000",
            "return=skipped unless economy needs mead or workers need experience",
            "consume=barley water honey",
            "animate=working 32000",
            "produce=mead"
         }
      },
   },
}
