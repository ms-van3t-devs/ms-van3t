local tools = {}

-- module to work with binary programs

function tools.find_bin ( bin )
    -- which is a shell build-in to find binaries in PATH 
    local which = io.popen ( 'which ' .. bin, 'r' )
    if ( which ~= nil ) then
        local content = which:read "*a"
        which:close ()
        return content
    end
    return nil
end

function tools.check_bin ( bin )
    local found_bin = tools.find_bin ( bin )
    if ( found_bin == nil ) then
        print ( "ERROR: " .. bin .. " not found in PATH!" )
        os.exit ( 1 ) -- return nil
    end
    return found_bin:gsub("\n","")
end

return tools
