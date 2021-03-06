dirname = path.dirname (__file__)

terrain_affinity = {
   preferred_temperature = 50,
   preferred_humidity = 0.3,
   preferred_fertility = 0.05,
   pickiness = 0.1,
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_raspberry_tiny",
   descname = _ "Raspberry Bush (tiny)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "seed_berrybush" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 20000",
         "remove=60",
         "grow=berry_bush_raspberry_small",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files (dirname .. "tiny/idle_?.png"),
         hotspot = {4, 7},
         scale = 3,
      },
   },
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_raspberry_small",
   descname = _ "Raspberry Bush (small)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 20000",
         "remove=40",
         "grow=berry_bush_raspberry_medium",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files (dirname .. "small/idle_?.png"),
         hotspot = {16, 31},
         scale = 3,
      },
   },
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_raspberry_medium",
   descname = _ "Raspberry Bush (medium)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "flowering" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 40000",
         "remove=20",
         "grow=berry_bush_raspberry_ripe",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files (dirname .. "medium/idle_?.png"),
         hotspot = {17, 34},
         scale = 3,
      },
   },
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_raspberry_ripe",
   descname = _ "Raspberry Bush (ripe)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "ripe_bush" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 400000",
         "remove=",
      },
      harvest = {
         "remove=",
      }
   },
   animations = {
      idle = {
         pictures = path.list_files (dirname .. "ripe/idle_?.png"),
         hotspot = {17, 34},
         scale = 3,
      },
   },
}
