set_property SRC_FILE_INFO {cfile:/home/dave/misc-projects/rftool-fpga/projects/rx_only/rx_only.srcs/sources_1/ip/sys_pll/sys_pll.xdc rfile:../../../rx_only.srcs/sources_1/ip/sys_pll/sys_pll.xdc id:1 order:EARLY scoped_inst:inst} [current_design]
set_property src_info {type:SCOPED_XDC file:1 line:57 export:INPUT save:INPUT read:READ} [current_design]
set_input_jitter [get_clocks -of_objects [get_ports clock_in]] 0.2
