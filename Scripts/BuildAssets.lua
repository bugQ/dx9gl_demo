--[[
	This file contains the logic for building assets
]]

-- Static Data Initialization
--===========================

local s_AuthoredAssetDir, s_BuiltAssetDir
do
	-- AuthoredAssetDir
	do
		local key = "AuthoredAssetDir"
		local errorMessage
		s_AuthoredAssetDir, errorMessage = GetEnvironmentVariable( key )
		if not s_AuthoredAssetDir then
			error( errorMessage )
		end
	end
	-- BuiltAssetDir
	do
		local key = "BuiltAssetDir"
		local errorMessage
		s_BuiltAssetDir, errorMessage = GetEnvironmentVariable( key )
		if not s_BuiltAssetDir then
			error( errorMessage )
		end
	end
end

-- Function Definitions
--=====================

function BuildAsset( i_relativePath )
	-- Get the absolute paths to the source and target
	-- (The "source" is the authored asset,
	-- and the "target" is the built asset that is ready to be used in-game.
	-- For now we will just copy the source to the target
	-- and so the two will be the same,
	-- but later in the class they will often be different
	-- and in a real game they will almost always be different:
	-- The source will be in a format that is optimal for authoring purposes
	-- and the target will be in a format that is optimal for real-time purposes.)
	local path_source = s_AuthoredAssetDir .. i_relativePath
	local path_target = s_BuiltAssetDir .. i_relativePath

	-- If the source file doesn't exist then it can't be built
	do
		local doesSourceExist = DoesFileExist( path_source )
		if not doesSourceExist then
			OutputErrorMessage( "The source asset doesn't exist", path_source )
			return false
		end
	end

	-- Decide if the target needs to be built
	local shouldTargetBeBuilt
	do
		-- The simplest reason a target should be built is if it doesn't exist
		local doesTargetExist = DoesFileExist( path_target )
		if doesTargetExist then
			-- Even if the target exists it may be out-of-date.
			-- If the source has been modified more recently than the target
			-- then the target should be re-built.
			local lastWriteTime_source = GetLastWriteTime( path_source )
			local lastWriteTime_target = GetLastWriteTime( path_target )
			shouldTargetBeBuilt = lastWriteTime_source > lastWriteTime_target
		else
			shouldTargetBeBuilt = true;
		end
	end

	-- Build the target if necessary
	if shouldTargetBeBuilt then
		-- Create the target directory if necessary
		CreateDirectoryIfNecessary( path_target )

		-- Copy the source to the target
		local result, errorMessage = CopyFile( path_source, path_target )
		if result then
			-- Display a message when an asset builds successfully
			print( "Built " .. path_source )
			return true;
		else
			-- Display a message describing why the asset didn't build
			OutputErrorMessage( errorMessage, path_source )
			return false
		end
	else
		return true
	end
end
