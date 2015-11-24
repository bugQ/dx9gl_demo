return
{
	meshes = {
		srcext = 'msh', dstext = 'vib',
		tool = 'MeshBuilder.exe',

		"ball",
		"plane",
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

		"sprite",
		"xlucent",
	},
	materials = {
		srcext = 'mtt', dstext = 'mtb',
		tool = 'MaterialBuilder.exe',

		"fuschia_opaque",
		"lime_opaque",
		"ochre_thirdalpha",
		"violet_halfalpha",
	},
}