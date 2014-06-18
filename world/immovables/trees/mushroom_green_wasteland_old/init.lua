dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "mushroom_green_wasteland_old",
   descname = _ "Green Mushroom Tree (Old)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 1535000",
         "transform=deadtree2 33",
         "seed=mushroom_green_wasteland_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 24, 60 },
         fps = 10,
      },
   },
}
