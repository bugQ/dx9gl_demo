return
{
	meshes = {
		srcext = 'msh', dstext = 'bvi',
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
	}
}