dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "shooting_\\d+.png" },
      hotspot = { 6, 29 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {14, 22}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {13, 23}, 10)


tribes:new_worker_type {
   name = "atlanteans_hunter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Hunter",

   buildcost = {
		atlanteans_carrier = 1,
		hunting_bow = 1
	},

	programs = {
		hunt = {
			"findobject type:bob radius:13 attrib:eatable",
			"walk object",
			"animation idle 1500",
			"object remove",
			"createware meat",
			"return"
		}
	},

	-- TRANSLATORS: Helptext for a worker: Hunter
   helptext = _"The Hunter brings fresh, raw meat to the Atlanteans.",
   animations = animations,
}
