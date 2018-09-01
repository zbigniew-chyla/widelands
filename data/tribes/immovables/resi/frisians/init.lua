dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "frisians_resi_none",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("immovable", "Resources: None"),
   helptext_script = dirname .. "helptexts/none.lua",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = { dirname .. "pics/none.png" },
         hotspot = {2, 49},
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "frisians_resi_water",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("immovable", "Resources: Water Vein"),
   helptext_script = dirname .. "helptexts/water.lua",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pics/water_?.png"),
         hotspot = {2, 49},
         fps = 4,
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "frisians_resi_coal_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("immovable", "Resources: Coal Vein"),
   helptext_script = dirname .. "helptexts/coal_1.lua",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pics/coal_1_?.png"),
         hotspot = {2, 49},
         fps = 4,
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "frisians_resi_gold_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("immovable", "Resources: Gold Vein"),
   helptext_script = dirname .. "helptexts/gold_1.lua",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pics/gold_1_?.png"),
         hotspot = {2, 49},
         fps = 4,
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "frisians_resi_iron_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("immovable", "Resources: Iron Vein"),
   helptext_script = dirname .. "helptexts/iron_1.lua",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pics/iron_1_?.png"),
         hotspot = {2, 49},
         fps = 4,
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "frisians_resi_stones_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("immovable", "Resources: Some Stones"),
   helptext_script = dirname .. "helptexts/stones_1.lua",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pics/stones_1_?.png"),
         hotspot = {2, 49},
         fps = 4,
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "frisians_resi_coal_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("immovable", "Resources: Main Coal Vein"),
   helptext_script = dirname .. "helptexts/coal_2.lua",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pics/coal_2_?.png"),
         hotspot = {2, 49},
         fps = 4,
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "frisians_resi_gold_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("immovable", "Resources: Main Gold Vein"),
   helptext_script = dirname .. "helptexts/gold_2.lua",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pics/gold_2_?.png"),
         hotspot = {2, 49},
         fps = 4,
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "frisians_resi_iron_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("immovable", "Resources: Main Iron Vein"),
   helptext_script = dirname .. "helptexts/iron_2.lua",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pics/iron_2_?.png"),
         hotspot = {2, 49},
         fps = 4,
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "frisians_resi_stones_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("immovable", "Resources: A Lot of Stones"),
   helptext_script = dirname .. "helptexts/stones_2.lua",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pics/stones_2_?.png"),
         hotspot = {2, 49},
         fps = 4,
      },
   }
}