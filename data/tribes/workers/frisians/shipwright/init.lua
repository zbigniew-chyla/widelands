dirname = path.dirname (__file__)

animations = {
   idle = {
      pictures = path.list_files (dirname .. "idle_??.png"),
      hotspot = {8, 23},
   },
   work = {
      pictures = path.list_files (dirname .. "work_??.png"),
      sound_effect = {
            directory = "sound/hammering",
            name = "hammering",
      },
      hotspot = {9, 24},
      fps = 10
   }
}
add_walking_animations (animations, "walk", dirname, "walk", {11, 24}, 15)
add_walking_animations (animations, "walkload", dirname, "walkload", {10, 26}, 15)

tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_shipwright",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Shipwright"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      hammer = 1
   },

   programs = {
      buildship = {
         "walk=object-or-coords",
         "plant=attrib:shipconstruction unless object",
         "animate=work 500",
         "construct",
         "animate=work 5000",
         "return"
      }
   },

   animations = animations,
}
