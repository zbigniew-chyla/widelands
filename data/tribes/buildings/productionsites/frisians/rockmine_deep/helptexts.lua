function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "")
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Digs granite out of the ground in mountain terrain.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("frisians_building", "This mine exploits all of the resource down to the deepest level. But even after having done so, it will still have a %s chance of finding some more granite."):bformat("10%")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("frisians_building", "The deep rock mine needs %s to produce three blocks of granite."):bformat(ngettext("%d second", "%d seconds", 76):bformat(76))
end
