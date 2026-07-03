function convert_junit_to_text(input_path, output_path)
    local input = io.readfile(input_path)
    if not input then
        raise("failed to open %s for reading", input)
    end

    local function sanitize_xml(raw_xml)
        return raw_xml:gsub('=(%b"")', function(quoted_value)
            return quoted_value:gsub('>', '&gt;')
        end)
    end

    -- doctest output does not escape some closing angled brackets, which cause
    -- Xmake's XML parser to trip. We need to escape them before passing to
    -- Xmake's XML parser.
    input = sanitize_xml(input)

    local output_file = io.open(output_path, "w")
    if not output_file then
        raise("failed to open %s for writing", output_file)
    end

    import("core.base.xml")
    local num_test_cases = 0
    local failures = {}
    xml.scan(input, function(node)
        if node.name == "testcase" then
            num_test_cases = num_test_cases + 1
        elseif node.name == "failure" then
            table.insert(failures, node.children[1].text)
        end
    end)

    if #failures > 0 then
        output_file:write("ERROR: " .. #failures .. " test cases failed\n")
        for _, failure in ipairs(failures) do
            output_file:write(failure .. "\n")
        end
    end
    local num_successes = num_test_cases - #failures
    output_file:write(
        (num_successes / num_test_cases * 100) .. "% tests passed, " ..
        #failures .. " tests failed out of " .. num_test_cases .. "\n"
    )
    output_file:close()

    return {
        num_test_cases = num_test_cases,
        num_successes = num_successes,
        num_failures = #failures,
    }
end
