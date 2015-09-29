return
{
	meshes = {
		ext = 'msh',
		tool = 'GenericBuilder.exe',
		{ src = "square", dst = "square" },
		{ src = "triangle", dst = "triangle" },
	},
	shaders = {
		ext = 'shd',
		tool = 'GenericBuilder.exe',
		{ src = "vertex", dst = "vertex" },
		{ src = "fragment", dst = "fragment" },
	},
}