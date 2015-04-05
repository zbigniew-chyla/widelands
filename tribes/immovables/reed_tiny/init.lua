dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "reed_tiny",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = _"Reed (tiny)",
   size = "small",
   attributes = { "field" },
   programs = {
		program = {
			"animate=idle 22000",
			"transform=reed_small",
      }
   },
   helptext = {
		default = ""
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 12, 8 },
      },
   }
}
