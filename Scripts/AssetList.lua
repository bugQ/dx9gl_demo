return
{
	meshes = {
		srcext = 'msh', dstext = 'vib',
		tool = 'MeshBuilder.exe',

		"square",
		"triangle",
	},
	shaders = {
		srcext = 'shd', dstext = 'shb',
		tool = 'ShaderBuilder.exe',

		-- NOTE the first letter of each shader file determines the type
		-- recommend prefices 'v_' for vertex and 'f_' for fragment/pixel
		"vertex",
		"fragment",
	},
	effects = {
		srcext = 'fxt', dstext = 'fxb',
		tool = 'EffectBuilder.exe',

		"sprite",
	},
}