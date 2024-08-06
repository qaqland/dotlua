local params = { ... }

io.write("Hello")

for k, v in pairs(params) do
	io.write(" " .. v)
end

io.write("\n")
