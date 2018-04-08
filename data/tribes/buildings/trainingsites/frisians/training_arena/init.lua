dirname = path.dirname (__file__)

tribes:new_trainingsite_type {
   msgctxt = "frisians_building",
   name = "frisians_training_arena",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Training Arena"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      brick = 6,
      granite = 3,
      log = 4,
      gold = 3,
      thatch_reed = 6
   },
   return_on_dismantle = {
      brick = 3,
      granite = 2,
      log = 2,
      gold = 1,
      thatch_reed = 3
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {101, 73},
         fps = 10,
      },
      working = {
         pictures = path.list_files (dirname .. "working_??.png"),
         hotspot = {101, 73},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {101, 67},
      },
   },

   aihints = {
      prohibited_till = 1500,
      very_weak_ai_limit = 0,
      weak_ai_limit = 1
   },

   working_positions = {
      frisians_trainer = 1
   },

   inputs = {
      { name = "smoked_fish", amount = 8 },
      { name = "smoked_meat", amount = 8 },
      { name = "mead", amount = 8 },
      { name = "honey_bread", amount = 8 },
      { name = "sword_long", amount = 4 },
      { name = "sword_curved", amount = 4 },
      { name = "sword_double", amount = 4 },
      { name = "helmet_golden", amount = 2 },
      { name = "fur_garment_golden", amount = 2 },
   },
   outputs = {
      "frisians_soldier",
      "scrap_iron",
      "scrap_metal_mixed",
      "fur_garment_old",
   },

   ["soldier attack"] = {
      min_level = 3,
      max_level = 5,
      food = {
         {"smoked_fish", "smoked_meat"},
         {"mead"},
         {"honey_bread"}
      },
      weapons = {
         "sword_long",
         "sword_curved",
         "sword_double",
      }
   },
   ["soldier health"] = {
      min_level = 1,
      max_level = 1,
      food = {
         {"smoked_fish", "smoked_meat"},
         {"honey_bread", "mead"}
      },
      weapons = {
         "helmet_golden",
      }
   },
   ["soldier defense"] = {
      min_level = 1,
      max_level = 1,
      food = {
         {"smoked_fish", "smoked_meat"},
         {"honey_bread", "mead"}
      },
      weapons = {
         "fur_garment_golden",
      }
   },

   programs = {
      sleep = {
         -- TRANSLATORS: Completed/Skipped/Did not start sleeping because ...
         descname = _"sleeping",
         actions = {
            "sleep=5000",
            "check_soldier=soldier attack 9", -- dummy check to get sleep rated as skipped - else it will change statistics
         }
      },
      upgrade_soldier_attack_3 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("frisians_building", "upgrading soldier attack from level 3 to level 4"),
         actions = {
            "check_soldier=soldier attack 3",
            "animate=working 22800",
            "check_soldier=soldier attack 3", -- Because the soldier can be expelled by the player
            "consume=sword_long:2 honey_bread mead:2 smoked_fish,smoked_meat",
            "train=soldier attack 3 4",
            "produce=scrap_metal_mixed:2"
         }
      },
      upgrade_soldier_attack_4 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("frisians_building", "upgrading soldier attack from level 4 to level 5"),
         actions = {
            "check_soldier=soldier attack 4",
            "animate=working 15600",
            "check_soldier=soldier attack 4", -- Because the soldier can be expelled by the player
            "consume=sword_curved:2 honey_bread mead:2 smoked_fish,smoked_meat",
            "train=soldier attack 4 5",
            "produce=scrap_iron:4"
         }
      },
      upgrade_soldier_attack_5 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("frisians_building", "upgrading soldier attack from level 5 to level 6"),
         actions = {
            "check_soldier=soldier attack 5",
            "animate=working 15600",
            "check_soldier=soldier attack 5", -- Because the soldier can be expelled by the player
            "consume=sword_double:2 honey_bread mead:2 smoked_fish,smoked_meat",
            "train=soldier attack 5 6",
            "produce=scrap_iron:2 scrap_metal_mixed:2"
         }
      },
      upgrade_soldier_defense_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("frisians_building", "upgrading soldier defense from level 1 to level 2"),
         actions = {
            "check_soldier=soldier defense 1",
            "animate=working 22800",
            "check_soldier=soldier defense 1", -- Because the soldier can be expelled by the player
            "consume=fur_garment_golden honey_bread,mead smoked_fish,smoked_meat",
            "train=soldier defense 1 2",
            "produce=scrap_iron fur_garment_old"
         }
      },
      upgrade_soldier_health_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext ("frisians_building", "upgrading soldier health from level 1 to level 2"),
         actions = {
            "check_soldier=soldier health 1",
            "animate=working 22800",
            "check_soldier=soldier health 1", -- Because the soldier can be expelled by the player
            "consume=helmet_golden honey_bread,mead smoked_fish,smoked_meat",
            "train=soldier health 1 2",
            "produce=scrap_iron:2"
         }
      },
   },

   soldier_capacity = 6,
   trainer_patience = 3
}