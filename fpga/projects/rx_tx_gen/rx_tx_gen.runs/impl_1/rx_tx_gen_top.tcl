proc start_step { step } {
  set stopFile ".stop.rst"
  if {[file isfile .stop.rst]} {
    puts ""
    puts "*** Halting run - EA reset detected ***"
    puts ""
    puts ""
    return -code error
  }
  set beginFile ".$step.begin.rst"
  set platform "$::tcl_platform(platform)"
  set user "$::tcl_platform(user)"
  set pid [pid]
  set host ""
  if { [string equal $platform unix] } {
    if { [info exist ::env(HOSTNAME)] } {
      set host $::env(HOSTNAME)
    }
  } else {
    if { [info exist ::env(COMPUTERNAME)] } {
      set host $::env(COMPUTERNAME)
    }
  }
  set ch [open $beginFile w]
  puts $ch "<?xml version=\"1.0\"?>"
  puts $ch "<ProcessHandle Version=\"1\" Minor=\"0\">"
  puts $ch "    <Process Command=\".planAhead.\" Owner=\"$user\" Host=\"$host\" Pid=\"$pid\">"
  puts $ch "    </Process>"
  puts $ch "</ProcessHandle>"
  close $ch
}

proc end_step { step } {
  set endFile ".$step.end.rst"
  set ch [open $endFile w]
  close $ch
}

proc step_failed { step } {
  set endFile ".$step.error.rst"
  set ch [open $endFile w]
  close $ch
}

set_msg_config -id {Common 17-41} -limit 10000000
set_msg_config -id {HDL 9-1061} -limit 100000
set_msg_config -id {HDL 9-1654} -limit 100000

start_step init_design
set ACTIVE_STEP init_design
set rc [catch {
  create_msg_db init_design.pb
  set_param xicom.use_bs_reader 1
  set_property design_mode GateLvl [current_fileset]
  set_param project.singleFileAddWarning.threshold 0
  set_property webtalk.parent_dir /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.cache/wt [current_project]
  set_property parent.project_path /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.xpr [current_project]
  set_property ip_output_repo /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.cache/ip [current_project]
  set_property ip_cache_permissions {read write} [current_project]
  set_property XPM_LIBRARIES {XPM_CDC XPM_MEMORY} [current_project]
  add_files -quiet /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.runs/synth_1/rx_tx_gen_top.dcp
  add_files -quiet /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/command_fifo/command_fifo.dcp
  set_property netlist_only true [get_files /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/command_fifo/command_fifo.dcp]
  add_files -quiet /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/response_fifo/response_fifo.dcp
  set_property netlist_only true [get_files /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/response_fifo/response_fifo.dcp]
  add_files -quiet /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/rx_packet_fifo/rx_packet_fifo.dcp
  set_property netlist_only true [get_files /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/rx_packet_fifo/rx_packet_fifo.dcp]
  add_files -quiet /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/iq_sample_fifo/iq_sample_fifo.dcp
  set_property netlist_only true [get_files /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/iq_sample_fifo/iq_sample_fifo.dcp]
  add_files -quiet /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/sys_pll/sys_pll.dcp
  set_property netlist_only true [get_files /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/sys_pll/sys_pll.dcp]
  add_files -quiet /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/sg_ila/sg_ila.dcp
  set_property netlist_only true [get_files /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/sg_ila/sg_ila.dcp]
  read_xdc -mode out_of_context -ref command_fifo -cells U0 /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/command_fifo/command_fifo_ooc.xdc
  set_property processing_order EARLY [get_files /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/command_fifo/command_fifo_ooc.xdc]
  read_xdc -ref command_fifo -cells U0 /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/command_fifo/command_fifo/command_fifo.xdc
  set_property processing_order EARLY [get_files /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/command_fifo/command_fifo/command_fifo.xdc]
  read_xdc -mode out_of_context -ref response_fifo -cells U0 /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/response_fifo/response_fifo_ooc.xdc
  set_property processing_order EARLY [get_files /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/response_fifo/response_fifo_ooc.xdc]
  read_xdc -ref response_fifo -cells U0 /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/response_fifo/response_fifo/response_fifo.xdc
  set_property processing_order EARLY [get_files /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/response_fifo/response_fifo/response_fifo.xdc]
  read_xdc -mode out_of_context -ref rx_packet_fifo -cells U0 /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/rx_packet_fifo/rx_packet_fifo_ooc.xdc
  set_property processing_order EARLY [get_files /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/rx_packet_fifo/rx_packet_fifo_ooc.xdc]
  read_xdc -ref rx_packet_fifo -cells U0 /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/rx_packet_fifo/rx_packet_fifo/rx_packet_fifo.xdc
  set_property processing_order EARLY [get_files /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/rx_packet_fifo/rx_packet_fifo/rx_packet_fifo.xdc]
  read_xdc -mode out_of_context -ref iq_sample_fifo -cells U0 /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/iq_sample_fifo/iq_sample_fifo_ooc.xdc
  set_property processing_order EARLY [get_files /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/iq_sample_fifo/iq_sample_fifo_ooc.xdc]
  read_xdc -ref iq_sample_fifo -cells U0 /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/iq_sample_fifo/iq_sample_fifo/iq_sample_fifo.xdc
  set_property processing_order EARLY [get_files /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/iq_sample_fifo/iq_sample_fifo/iq_sample_fifo.xdc]
  read_xdc -mode out_of_context -ref sys_pll -cells inst /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/sys_pll/sys_pll_ooc.xdc
  set_property processing_order EARLY [get_files /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/sys_pll/sys_pll_ooc.xdc]
  read_xdc -prop_thru_buffers -ref sys_pll -cells inst /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/sys_pll/sys_pll_board.xdc
  set_property processing_order EARLY [get_files /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/sys_pll/sys_pll_board.xdc]
  read_xdc -ref sys_pll -cells inst /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/sys_pll/sys_pll.xdc
  set_property processing_order EARLY [get_files /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/sys_pll/sys_pll.xdc]
  read_xdc -mode out_of_context -ref sg_ila -cells U0 /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/sg_ila/sg_ila_ooc.xdc
  set_property processing_order EARLY [get_files /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/sg_ila/sg_ila_ooc.xdc]
  read_xdc -ref sg_ila -cells U0 /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/sg_ila/ila_v6_2/constraints/ila.xdc
  set_property processing_order EARLY [get_files /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/sg_ila/ila_v6_2/constraints/ila.xdc]
  read_xdc /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/constrs_1/new/rftool_rev1.xdc
  read_xdc -ref command_fifo -cells U0 /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/command_fifo/command_fifo/command_fifo_clocks.xdc
  set_property processing_order LATE [get_files /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/command_fifo/command_fifo/command_fifo_clocks.xdc]
  read_xdc -ref response_fifo -cells U0 /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/response_fifo/response_fifo/response_fifo_clocks.xdc
  set_property processing_order LATE [get_files /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/response_fifo/response_fifo/response_fifo_clocks.xdc]
  read_xdc -ref rx_packet_fifo -cells U0 /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/rx_packet_fifo/rx_packet_fifo/rx_packet_fifo_clocks.xdc
  set_property processing_order LATE [get_files /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/rx_packet_fifo/rx_packet_fifo/rx_packet_fifo_clocks.xdc]
  read_xdc -ref iq_sample_fifo -cells U0 /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/iq_sample_fifo/iq_sample_fifo/iq_sample_fifo_clocks.xdc
  set_property processing_order LATE [get_files /home/dave/misc-projects/rftool-fpga/projects/rx_tx_gen/rx_tx_gen.srcs/sources_1/ip/iq_sample_fifo/iq_sample_fifo/iq_sample_fifo_clocks.xdc]
  link_design -top rx_tx_gen_top -part xc7a50tftg256-2
  write_hwdef -file rx_tx_gen_top.hwdef
  close_msg_db -file init_design.pb
} RESULT]
if {$rc} {
  step_failed init_design
  return -code error $RESULT
} else {
  end_step init_design
  unset ACTIVE_STEP 
}

start_step opt_design
set ACTIVE_STEP opt_design
set rc [catch {
  create_msg_db opt_design.pb
  opt_design 
  write_checkpoint -force rx_tx_gen_top_opt.dcp
  report_drc -file rx_tx_gen_top_drc_opted.rpt
  close_msg_db -file opt_design.pb
} RESULT]
if {$rc} {
  step_failed opt_design
  return -code error $RESULT
} else {
  end_step opt_design
  unset ACTIVE_STEP 
}

start_step place_design
set ACTIVE_STEP place_design
set rc [catch {
  create_msg_db place_design.pb
  implement_debug_core 
  place_design 
  write_checkpoint -force rx_tx_gen_top_placed.dcp
  report_io -file rx_tx_gen_top_io_placed.rpt
  report_utilization -file rx_tx_gen_top_utilization_placed.rpt -pb rx_tx_gen_top_utilization_placed.pb
  report_control_sets -verbose -file rx_tx_gen_top_control_sets_placed.rpt
  close_msg_db -file place_design.pb
} RESULT]
if {$rc} {
  step_failed place_design
  return -code error $RESULT
} else {
  end_step place_design
  unset ACTIVE_STEP 
}

start_step route_design
set ACTIVE_STEP route_design
set rc [catch {
  create_msg_db route_design.pb
  route_design 
  write_checkpoint -force rx_tx_gen_top_routed.dcp
  report_drc -file rx_tx_gen_top_drc_routed.rpt -pb rx_tx_gen_top_drc_routed.pb -rpx rx_tx_gen_top_drc_routed.rpx
  report_methodology -file rx_tx_gen_top_methodology_drc_routed.rpt -rpx rx_tx_gen_top_methodology_drc_routed.rpx
  report_timing_summary -warn_on_violation -max_paths 10 -file rx_tx_gen_top_timing_summary_routed.rpt -rpx rx_tx_gen_top_timing_summary_routed.rpx
  report_power -file rx_tx_gen_top_power_routed.rpt -pb rx_tx_gen_top_power_summary_routed.pb -rpx rx_tx_gen_top_power_routed.rpx
  report_route_status -file rx_tx_gen_top_route_status.rpt -pb rx_tx_gen_top_route_status.pb
  report_clock_utilization -file rx_tx_gen_top_clock_utilization_routed.rpt
  close_msg_db -file route_design.pb
} RESULT]
if {$rc} {
  write_checkpoint -force rx_tx_gen_top_routed_error.dcp
  step_failed route_design
  return -code error $RESULT
} else {
  end_step route_design
  unset ACTIVE_STEP 
}

start_step write_bitstream
set ACTIVE_STEP write_bitstream
set rc [catch {
  create_msg_db write_bitstream.pb
  set_property XPM_LIBRARIES {XPM_CDC XPM_MEMORY} [current_project]
  catch { write_mem_info -force rx_tx_gen_top.mmi }
  write_bitstream -force -no_partial_bitfile rx_tx_gen_top.bit 
  catch { write_sysdef -hwdef rx_tx_gen_top.hwdef -bitfile rx_tx_gen_top.bit -meminfo rx_tx_gen_top.mmi -file rx_tx_gen_top.sysdef }
  catch {write_debug_probes -quiet -force debug_nets}
  close_msg_db -file write_bitstream.pb
} RESULT]
if {$rc} {
  step_failed write_bitstream
  return -code error $RESULT
} else {
  end_step write_bitstream
  unset ACTIVE_STEP 
}

