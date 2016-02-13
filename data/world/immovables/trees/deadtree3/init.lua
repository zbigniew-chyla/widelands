dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "deadtree3",
   descname = _ "Dead Tree",
   editor_category = "trees_dead",
   size = "none",
   attributes = {},
   programs = {
      program = {
        "animate=idle 20000",
       "remove=16"
      }
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 23, 61 },
      },
   }
}