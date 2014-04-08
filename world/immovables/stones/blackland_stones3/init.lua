dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      player_color_masks = {},
      hotspot = { 30, 75 }
   },
}

world:new_immovable_type{
   name = "stones3",
   descname = _ "Stones",
   size = "big",
   attributes = { "stone" },
   programs = {
      program = {
         "animate=idle",
         "transform=stones2"
      }
   },
   animations = animations
}
