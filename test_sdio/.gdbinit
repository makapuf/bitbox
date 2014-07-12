define reload
kill
monitor jtag_reset
load
end

target extended localhost:4242
load
