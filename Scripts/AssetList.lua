return
{
	meshes = {
		srcext = 'msh', dstext = 'msh',
		tool = 'GenericBuilder.exe',
		-- mesh list proper
		"square",
		"triangle",
	},
	shaders = {
		srcext = 'shd', dstext = 'mesh',
		tool = 'GenericBuilder.exe',
		-- shader list proper
		"vertex",
		"fragment",
	}
}