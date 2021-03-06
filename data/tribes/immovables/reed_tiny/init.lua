dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "reed_tiny",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Reed (tiny)"),
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "field", "seed_reed" },
   programs = {
      program = {
         "animate=idle 22000",
         "transform=reed_small",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 12, 8 },
      },
   }
}
