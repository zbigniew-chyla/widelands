dirname = path.dirname (__file__)

tribes:new_trainingsite_type {
   msgctxt = "frisians_building",
   name = "frisians_training_camp",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Training Camp"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 4,
      granite = 3,
      log = 3,
      gold = 1,
      thatch_reed = 4
   },
   return_on_dismantle = {
      brick = 2,
      granite = 1,
      log = 1,
      gold = 1,
      thatch_reed = 2
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {59, 73},
         fps = 10,
      },
      working = {
         pictures = path.list_files (dirname .. "working_??.png"),
         hotspot = {59, 73},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {59, 62},
      },
   },

   aihints = {
      prohibited_till = 900,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      frisians_trainer = 1
   },

   inputs = {
      { name = "bread_frisians", amount = 6 },
      { name = "beer", amount = 6 },
      { name = "smoked_fish", amount = 6 },
      { name = "smoked_meat", amount = 6 },
      { name = "sword_long", amount = 2 },
      { name = "sword_curved", amount = 2 },
      { name = "sword_double", amount = 2 },
      { name = "helmet", amount = 2 },
      { name = "fur_garment_studded", amount = 2 },
   },
   outputs = {
      "frisians_soldier",
      "scrap_metal_mixed",
      "scrap_iron",
      "fur_garment_old",
   },

   ["soldier attack"] = {
      min_level = 0,
      max_level = 2,
      food = {
         {"smoked_fish", "smoked_meat"},
         {"beer"},
         {"bread_frisians"}
      },
      weapons = {
         "sword_long",
         "sword_curved",
         "sword_double",
      }
   },
   ["soldier health"] = {
      min_level = 0,
      max_level = 0,
      food = {
         {"bread_frisians"},
         {"beer"},
         {"smoked_fish", "smoked_meat"},
      },
      weapons = {
         "helmet",
      }
   },
   ["soldier defense"] = {
      min_level = 0,
      max_level = 0,
      food = {
         {"bread_frisians"},
         {"beer"},
         {"smoked_fish", "smoked_meat"},
      },
      weapons = {
         "fur_garment_studded",
      }
   },

   programs = {
      sleep = {
         -- TRANSLATORS: Completed/Skipped/Did not start sleeping because ...
         descname = _"sleeping",
         actions = {
            "sleep=5000",
            "checksoldier=soldier attack 9", -- dummy check to get sleep rated as skipped - else it will change statistics
         }
      },
      upgrade_soldier_attack_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("frisians_building", "upgrading soldier attack from level 0 to level 1"),
         actions = {
            "checksoldier=soldier attack 0", -- Fails when aren't any soldier of level 0 attack
            "animate=working 26400",
            "checksoldier=soldier attack 0", -- Because the soldier can be expelled by the player
            "consume=sword_long bread_frisians,beer,smoked_fish,smoked_meat",
            "train=soldier attack 0 1",
            "produce=scrap_iron"
         }
      },
      upgrade_soldier_attack_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("frisians_building", "upgrading soldier attack from level 1 to level 2"),
         actions = {
            "checksoldier=soldier attack 1",
            "animate=working 22800",
            "checksoldier=soldier attack 1",
            "consume=sword_curved bread_frisians beer smoked_fish,smoked_meat",
            "train=soldier attack 1 2",
            "produce=scrap_iron:2"
         }
      },
      upgrade_soldier_attack_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("frisians_building", "upgrading soldier attack from level 2 to level 3"),
         actions = {
            "checksoldier=soldier attack 2",
            "animate=working 22800",
            "checksoldier=soldier attack 2",
            "consume=sword_double bread_frisians beer smoked_fish,smoked_meat",
            "train=soldier attack 2 3",
            "produce=scrap_iron scrap_metal_mixed"
         }
      },
      upgrade_soldier_health_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("frisians_building", "upgrading soldier health from level 0 to level 1"),
         actions = {
            "checksoldier=soldier health 0",
            "animate=working 30000",
            "checksoldier=soldier health 0",
            "consume=helmet bread_frisians,beer smoked_fish,smoked_meat",
            "train=soldier health 0 1"
         }
      },
      upgrade_soldier_defense_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("frisians_building", "upgrading soldier defense from level 0 to level 1"),
         actions = {
            "checksoldier=soldier defense 0",
            "animate=working 26400",
            "checksoldier=soldier defense 0",
            "consume=fur_garment_studded bread_frisians,beer smoked_fish,smoked_meat",
            "train=soldier defense 0 1",
            "produce=fur_garment_old"
         }
      },
   },

   soldier_capacity = 10,
   trainer_patience = 5
}
