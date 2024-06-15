-- Tiles (use automatic mipmapping) --
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

-- Sprites (use manual mipmapping) --
function loadss(swidth)
	sprites = image({
		filename = "spritesheet" .. swidth .. ".png",
		width = swidth,
		height = swidth,
		usemask = true,
		rawtiles = true,
	})
	return sprites
end

raycast_helper("spritesheet", true, {
	["32"] = loadss(32),
	["16"] = loadss(16),
	["8"] = loadss(8),
	["4"] = loadss(4),
})