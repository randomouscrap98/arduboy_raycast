-- Tiles --
tiles,frames,width,height = image({
	filename = "tiles2.png",
	width = 32,
	height = 32,
	rawtiles = true,
})
raycast_helper("tilesheet", false, {
	["32"] = tiles,
	["16"] = image_resize(tiles, width, height, 16, 16),
	["8"] = image_resize(tiles, width, height, 8, 8),
	["4"] = image_resize(tiles, width, height, 4, 4),
})

-- Sprites --
sprites,frames,width,height = image({
	filename = "spritesheet.png",
	width = 32,
	height = 32,
	usemask = true,
	rawtiles = true,
})
raycast_helper("spritesheet", true, {
	["32"] = sprites,
	["16"] = image_resize(sprites, width, height, 16, 16),
	["8"] = image_resize(sprites, width, height, 8, 8),
	["4"] = image_resize(sprites, width, height, 4, 4),
})