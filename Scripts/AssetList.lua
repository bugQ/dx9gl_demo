return
{
	meshes = {
		srcext = 'msh', dstext = 'vib',
		tool = 'MeshBuilder.exe',
		-- mesh list proper
		"square",
		"triangle",
	},
	shaders = {
		srcext = 'shd', dstext = 'shd',
		tool = 'GenericBuilder.exe',
		-- shader list proper
		"vertex",
		"fragment",
	},
	effects = {
		srcext = 'fxt', dstext = 'fxb',
		tool = 'EffectBuilder.exe',
		-- effect list proper
		"sprite",
	},
}