return
{
	meshes = {
		srcext = 'msh', dstext = 'vib',
		tool = 'MeshBuilder.exe',

		"ctf_ceiling",
		"ctf_cement",
		"ctf_floor",
		"ctf_metal",
		"ctf_railing",
		"ctf_walls",
	},
	shaders = {
		srcext = 'shd', dstext = 'shb',
		tool = 'ShaderBuilder.exe',
		deps = {"monsters.inc"},

		-- NOTE the first letter of each shader file determines the type
		-- recommend prefices 'v_' for vertex and 'f_' for fragment/pixel
		"vertex",
		"f_opaque",
		"f_transparent",
	},
	effects = {
		srcext = 'fxt', dstext = 'fxb',
		tool = 'EffectBuilder.exe',

		"opaque",
		"xlucent",
	},
	textures = {
		srcext = 'png', dstext = 'dds',
		tool = 'TextureBuilder.exe',

		"FFFFFF-1",
		"cement_wall_D",
		"floor_D",
		"metal_brace_D",
		"railing_D",
		"wall_D",
	},
	materials = {
		srcext = 'mtt', dstext = 'mtb',
		tool = 'MaterialBuilder.exe',

		"lime_opaque",
		"ctf_cement",
		"ctf_floor",
		"ctf_metal",
		"ctf_railing",
		"ctf_walls",
	},
}