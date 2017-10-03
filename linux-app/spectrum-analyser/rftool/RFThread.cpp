#include "RFThread.hpp"
#include <iostream>

AD9361_InitParam default_init_param = {
    /* Identification number */
    0, // id_no;
    /* Reference Clock */
    40000000UL, // reference_clk_rate
    /* Base Configuration */
    0, // two_rx_two_tx_mode_enable *** adi,2rx-2tx-mode-enable
    1, // frequency_division_duplex_mode_enable ***
       // adi,frequency-division-duplex-mode-enable
    1, // frequency_division_duplex_independent_mode_enable ***
       // adi,frequency-division-duplex-independent-mode-enable
    1, // tdd_use_dual_synth_mode_enable *** adi,tdd-use-dual-synth-mode-enable
    0, // tdd_skip_vco_cal_enable *** adi,tdd-skip-vco-cal-enable
    0, // tx_fastlock_delay_ns *** adi,tx-fastlock-delay-ns
    0, // rx_fastlock_delay_ns *** adi,rx-fastlock-delay-ns
    0, // rx_fastlock_pincontrol_enable *** adi,rx-fastlock-pincontrol-enable
    0, // tx_fastlock_pincontrol_enable *** adi,tx-fastlock-pincontrol-enable
    0, // external_rx_lo_enable *** adi,external-rx-lo-enable
    0, // external_tx_lo_enable *** adi,external-tx-lo-enable
    5, // dc_offset_tracking_update_event_mask ***
       // adi,dc-offset-tracking-update-event-mask
    6, // dc_offset_attenuation_high_range ***
       // adi,dc-offset-tracking-update-event-mask
    5, // dc_offset_attenuation_low_range ***
       // adi,dc-offset-tracking-update-event-mask
    0x28, // dc_offset_count_high_range ***
          // adi,dc-offset-tracking-update-event-mask
    0x32, // dc_offset_count_low_range ***
          // adi,dc-offset-tracking-update-event-mask
    0,    // tdd_use_fdd_vco_tables_enable *** adi,tdd-use-fdd-vco-tables-enable
    0,    // split_gain_table_mode_enable *** adi,split-gain-table-mode-enable
    40000000UL, // trx_synthesizer_target_fref_overwrite_hz ***
                // adi,trx-synthesizer-target-fref-overwrite-hz
    1, // qec_tracking_slow_mode_enable *** adi,qec-tracking-slow-mode-enable
    /* ENSM Control */
    0, // ensm_enable_pin_pulse_mode_enable ***
       // adi,ensm-enable-pin-pulse-mode-enable
    0, // ensm_enable_txnrx_control_enable ***
       // adi,ensm-enable-txnrx-control-enable
    /* LO Control */
    2400000000UL, // rx_synthesizer_frequency_hz ***
                  // adi,rx-synthesizer-frequency-hz
    2400000000UL, // tx_synthesizer_frequency_hz ***
                  // adi,tx-synthesizer-frequency-hz
    /* Rate & BW Control */
    {983040000, 245760000, 122880000, 61440000, 30720000,
     30720000}, // uint32_t	rx_path_clock_frequencies[6] ***
                // adi,rx-path-clock-frequencies
    {983040000, 122880000, 122880000, 61440000, 30720000,
     30720000}, // uint32_t	tx_path_clock_frequencies[6] ***
                // adi,tx-path-clock-frequencies
    30720000,   // rf_rx_bandwidth_hz *** adi,rf-rx-bandwidth-hz
    30720000,   // rf_tx_bandwidth_hz *** adi,rf-tx-bandwidth-hz
    /* RF Port Control */
    0, // rx_rf_port_input_select *** adi,rx-rf-port-input-select
    0, // tx_rf_port_input_select *** adi,tx-rf-port-input-select
    /* TX Attenuation Control */
    10000, // tx_attenuation_mdB *** adi,tx-attenuation-mdB
    0, // update_tx_gain_in_alert_enable *** adi,update-tx-gain-in-alert-enable
    /* Reference Clock Control */
    1,         // xo_disable_use_ext_refclk_enable ***
               // adi,xo-disable-use-ext-refclk-enable
    {8, 5920}, // dcxo_coarse_and_fine_tune[2] *** adi,dcxo-coarse-and-fine-tune
    0,         // clk_output_mode_select *** adi,clk-output-mode-select
    /* Gain Control */
    2,    // gc_rx1_mode *** adi,gc-rx1-mode
    2,    // gc_rx2_mode *** adi,gc-rx2-mode
    58,   // gc_adc_large_overload_thresh *** adi,gc-adc-large-overload-thresh
    4,    // gc_adc_ovr_sample_size *** adi,gc-adc-ovr-sample-size
    47,   // gc_adc_small_overload_thresh *** adi,gc-adc-small-overload-thresh
    8192, // gc_dec_pow_measurement_duration ***
          // adi,gc-dec-pow-measurement-duration
    0,    // gc_dig_gain_enable *** adi,gc-dig-gain-enable
    800,  // gc_lmt_overload_high_thresh *** adi,gc-lmt-overload-high-thresh
    704,  // gc_lmt_overload_low_thresh *** adi,gc-lmt-overload-low-thresh
    24,   // gc_low_power_thresh *** adi,gc-low-power-thresh
    15,   // gc_max_dig_gain *** adi,gc-max-dig-gain
    /* Gain MGC Control */
    2, // mgc_dec_gain_step *** adi,mgc-dec-gain-step
    2, // mgc_inc_gain_step *** adi,mgc-inc-gain-step
    0, // mgc_rx1_ctrl_inp_enable *** adi,mgc-rx1-ctrl-inp-enable
    0, // mgc_rx2_ctrl_inp_enable *** adi,mgc-rx2-ctrl-inp-enable
    0, // mgc_split_table_ctrl_inp_gain_mode ***
       // adi,mgc-split-table-ctrl-inp-gain-mode
    /* Gain AGC Control */
    10,   // agc_adc_large_overload_exceed_counter ***
          // adi,agc-adc-large-overload-exceed-counter
    2,    // agc_adc_large_overload_inc_steps ***
          // adi,agc-adc-large-overload-inc-steps
    0,    // agc_adc_lmt_small_overload_prevent_gain_inc_enable ***
          // adi,agc-adc-lmt-small-overload-prevent-gain-inc-enable
    10,   // agc_adc_small_overload_exceed_counter ***
          // adi,agc-adc-small-overload-exceed-counter
    4,    // agc_dig_gain_step_size *** adi,agc-dig-gain-step-size
    3,    // agc_dig_saturation_exceed_counter ***
          // adi,agc-dig-saturation-exceed-counter
    1000, // agc_gain_update_interval_us *** adi,agc-gain-update-interval-us
    0,    // agc_immed_gain_change_if_large_adc_overload_enable ***
          // adi,agc-immed-gain-change-if-large-adc-overload-enable
    0,    // agc_immed_gain_change_if_large_lmt_overload_enable ***
          // adi,agc-immed-gain-change-if-large-lmt-overload-enable
    10,   // agc_inner_thresh_high *** adi,agc-inner-thresh-high
    1,    // agc_inner_thresh_high_dec_steps ***
          // adi,agc-inner-thresh-high-dec-steps
    12,   // agc_inner_thresh_low *** adi,agc-inner-thresh-low
    1,  // agc_inner_thresh_low_inc_steps *** adi,agc-inner-thresh-low-inc-steps
    10, // agc_lmt_overload_large_exceed_counter ***
        // adi,agc-lmt-overload-large-exceed-counter
    2,  // agc_lmt_overload_large_inc_steps ***
        // adi,agc-lmt-overload-large-inc-steps
    10, // agc_lmt_overload_small_exceed_counter ***
        // adi,agc-lmt-overload-small-exceed-counter
    5,  // agc_outer_thresh_high *** adi,agc-outer-thresh-high
    2,  // agc_outer_thresh_high_dec_steps ***
        // adi,agc-outer-thresh-high-dec-steps
    18, // agc_outer_thresh_low *** adi,agc-outer-thresh-low
    2,  // agc_outer_thresh_low_inc_steps *** adi,agc-outer-thresh-low-inc-steps
    1,  // agc_attack_delay_extra_margin_us; ***
        // adi,agc-attack-delay-extra-margin-us
    0,  // agc_sync_for_gain_counter_enable ***
        // adi,agc-sync-for-gain-counter-enable
    /* Fast AGC */
    64,  // fagc_dec_pow_measuremnt_duration ***
         // adi,fagc-dec-pow-measurement-duration
    260, // fagc_state_wait_time_ns ***  adi,fagc-state-wait-time-ns
    /* Fast AGC - Low Power */
    0, // fagc_allow_agc_gain_increase ***
       // adi,fagc-allow-agc-gain-increase-enable
    5, // fagc_lp_thresh_increment_time ***  adi,fagc-lp-thresh-increment-time
    1, // fagc_lp_thresh_increment_steps ***  adi,fagc-lp-thresh-increment-steps
    /* Fast AGC - Lock Level */
    10, // fagc_lock_level ***  adi,fagc-lock-level */
    1,  // fagc_lock_level_lmt_gain_increase_en ***
        // adi,fagc-lock-level-lmt-gain-increase-enable
    5,  // fagc_lock_level_gain_increase_upper_limit ***
        // adi,fagc-lock-level-gain-increase-upper-limit
    /* Fast AGC - Peak Detectors and Final Settling */
    1, // fagc_lpf_final_settling_steps ***  adi,fagc-lpf-final-settling-steps
    1, // fagc_lmt_final_settling_steps ***  adi,fagc-lmt-final-settling-steps
    3, // fagc_final_overrange_count ***  adi,fagc-final-overrange-count
    /* Fast AGC - Final Power Test */
    0, // fagc_gain_increase_after_gain_lock_en ***
       // adi,fagc-gain-increase-after-gain-lock-enable
    /* Fast AGC - Unlocking the Gain */
    0,  // fagc_gain_index_type_after_exit_rx_mode ***
        // adi,fagc-gain-index-type-after-exit-rx-mode
    1,  // fagc_use_last_lock_level_for_set_gain_en ***
        // adi,fagc-use-last-lock-level-for-set-gain-enable
    1,  // fagc_rst_gla_stronger_sig_thresh_exceeded_en ***
        // adi,fagc-rst-gla-stronger-sig-thresh-exceeded-enable
    5,  // fagc_optimized_gain_offset ***  adi,fagc-optimized-gain-offset
    10, // fagc_rst_gla_stronger_sig_thresh_above_ll ***
        // adi,fagc-rst-gla-stronger-sig-thresh-above-ll
    1,  // fagc_rst_gla_engergy_lost_sig_thresh_exceeded_en ***
        // adi,fagc-rst-gla-engergy-lost-sig-thresh-exceeded-enable
    1,  // fagc_rst_gla_engergy_lost_goto_optim_gain_en ***
        // adi,fagc-rst-gla-engergy-lost-goto-optim-gain-enable
    10, // fagc_rst_gla_engergy_lost_sig_thresh_below_ll ***
        // adi,fagc-rst-gla-engergy-lost-sig-thresh-below-ll
    8,  // fagc_energy_lost_stronger_sig_gain_lock_exit_cnt ***
        // adi,fagc-energy-lost-stronger-sig-gain-lock-exit-cnt
    1,  // fagc_rst_gla_large_adc_overload_en ***
        // adi,fagc-rst-gla-large-adc-overload-enable
    1,  // fagc_rst_gla_large_lmt_overload_en ***
        // adi,fagc-rst-gla-large-lmt-overload-enable
    0,  // fagc_rst_gla_en_agc_pulled_high_en ***
        // adi,fagc-rst-gla-en-agc-pulled-high-enable
    0,  // fagc_rst_gla_if_en_agc_pulled_high_mode ***
        // adi,fagc-rst-gla-if-en-agc-pulled-high-mode
    64, // fagc_power_measurement_duration_in_state5 ***
        // adi,fagc-power-measurement-duration-in-state5
    /* RSSI Control */
    1,    // rssi_delay *** adi,rssi-delay
    1000, // rssi_duration *** adi,rssi-duration
    3,    // rssi_restart_mode *** adi,rssi-restart-mode
    0, // rssi_unit_is_rx_samples_enable *** adi,rssi-unit-is-rx-samples-enable
    1, // rssi_wait *** adi,rssi-wait
    /* Aux ADC Control */
    256,        // aux_adc_decimation *** adi,aux-adc-decimation
    40000000UL, // aux_adc_rate *** adi,aux-adc-rate
    /* AuxDAC Control */
    1,    // aux_dac_manual_mode_enable ***  adi,aux-dac-manual-mode-enable
    1500, // aux_dac1_default_value_mV ***  adi,aux-dac1-default-value-mV
    1,    // aux_dac1_active_in_rx_enable ***  adi,aux-dac1-active-in-rx-enable
    1,    // aux_dac1_active_in_tx_enable ***  adi,aux-dac1-active-in-tx-enable
    1,    // aux_dac1_active_in_alert_enable ***
          // adi,aux-dac1-active-in-alert-enable
    0,    // aux_dac1_rx_delay_us ***  adi,aux-dac1-rx-delay-us
    0,    // aux_dac1_tx_delay_us ***  adi,aux-dac1-tx-delay-us
    0,    // aux_dac2_default_value_mV ***  adi,aux-dac2-default-value-mV
    0,    // aux_dac2_active_in_rx_enable ***  adi,aux-dac2-active-in-rx-enable
    0,    // aux_dac2_active_in_tx_enable ***  adi,aux-dac2-active-in-tx-enable
    0,    // aux_dac2_active_in_alert_enable ***
          // adi,aux-dac2-active-in-alert-enable
    0,    // aux_dac2_rx_delay_us ***  adi,aux-dac2-rx-delay-us
    0,    // aux_dac2_tx_delay_us ***  adi,aux-dac2-tx-delay-us
    /* Temperature Sensor Control */
    256,          // temp_sense_decimation *** adi,temp-sense-decimation
    1000,         // temp_sense_measurement_interval_ms ***
                  // adi,temp-sense-measurement-interval-ms
    int8_t(0xCE), // temp_sense_offset_signed *** adi,temp-sense-offset-signed
    1,            // temp_sense_periodic_measurement_enable ***
                  // adi,temp-sense-periodic-measurement-enable
    /* Control Out Setup */
    0xFF, // ctrl_outs_enable_mask *** adi,ctrl-outs-enable-mask
    0,    // ctrl_outs_index *** adi,ctrl-outs-index
    /* External LNA Control */
    0, // elna_settling_delay_ns *** adi,elna-settling-delay-ns
    0, // elna_gain_mdB *** adi,elna-gain-mdB
    0, // elna_bypass_loss_mdB *** adi,elna-bypass-loss-mdB
    0, // elna_rx1_gpo0_control_enable *** adi,elna-rx1-gpo0-control-enable
    0, // elna_rx2_gpo1_control_enable *** adi,elna-rx2-gpo1-control-enable
    0, // elna_gaintable_all_index_enable ***
       // adi,elna-gaintable-all-index-enable
    /* Digital Interface Control */
    0,    // digital_interface_tune_skip_mode ***
          // adi,digital-interface-tune-skip-mode
    1,    // digital_interface_tune_fir_disable ***
          // adi,digital-interface-tune-fir-disable
    1,    // pp_tx_swap_enable *** adi,pp-tx-swap-enable
    1,    // pp_rx_swap_enable *** adi,pp-rx-swap-enable
    0,    // tx_channel_swap_enable *** adi,tx-channel-swap-enable
    0,    // rx_channel_swap_enable *** adi,rx-channel-swap-enable
    1,    // rx_frame_pulse_mode_enable *** adi,rx-frame-pulse-mode-enable
    0,    // two_t_two_r_timing_enable *** adi,2t2r-timing-enable
    0,    // invert_data_bus_enable *** adi,invert-data-bus-enable
    0,    // invert_data_clk_enable *** adi,invert-data-clk-enable
    0,    // fdd_alt_word_order_enable *** adi,fdd-alt-word-order-enable
    0,    // invert_rx_frame_enable *** adi,invert-rx-frame-enable
    0,    // fdd_rx_rate_2tx_enable *** adi,fdd-rx-rate-2tx-enable
    0,    // swap_ports_enable *** adi,swap-ports-enable
    0,    // single_data_rate_enable *** adi,single-data-rate-enable
    1,    // lvds_mode_enable *** adi,lvds-mode-enable
    0,    // half_duplex_mode_enable *** adi,half-duplex-mode-enable
    0,    // single_port_mode_enable *** adi,single-port-mode-enable
    0,    // full_port_enable *** adi,full-port-enable
    0,    // full_duplex_swap_bits_enable *** adi,full-duplex-swap-bits-enable
    0,    // delay_rx_data *** adi,delay-rx-data
    0,    // rx_data_clock_delay *** adi,rx-data-clock-delay
    4,    // rx_data_delay *** adi,rx-data-delay
    7,    // tx_fb_clock_delay *** adi,tx-fb-clock-delay
    0,    // tx_data_delay *** adi,tx-data-delay
    75,   // lvds_bias_mV *** adi,lvds-bias-mV
    1,    // lvds_rx_onchip_termination_enable ***
          // adi,lvds-rx-onchip-termination-enable
    0,    // rx1rx2_phase_inversion_en *** adi,rx1-rx2-phase-inversion-enable
    0xFF, // lvds_invert1_control *** adi,lvds-invert1-control
    0x0F, // lvds_invert2_control *** adi,lvds-invert2-control
    /* GPO Control */
    0, // gpo0_inactive_state_high_enable ***
       // adi,gpo0-inactive-state-high-enable
    0, // gpo1_inactive_state_high_enable ***
       // adi,gpo1-inactive-state-high-enable
    0, // gpo2_inactive_state_high_enable ***
       // adi,gpo2-inactive-state-high-enable
    0, // gpo3_inactive_state_high_enable ***
       // adi,gpo3-inactive-state-high-enable
    0, // gpo0_slave_rx_enable *** adi,gpo0-slave-rx-enable
    0, // gpo0_slave_tx_enable *** adi,gpo0-slave-tx-enable
    0, // gpo1_slave_rx_enable *** adi,gpo1-slave-rx-enable
    0, // gpo1_slave_tx_enable *** adi,gpo1-slave-tx-enable
    0, // gpo2_slave_rx_enable *** adi,gpo2-slave-rx-enable
    0, // gpo2_slave_tx_enable *** adi,gpo2-slave-tx-enable
    0, // gpo3_slave_rx_enable *** adi,gpo3-slave-rx-enable
    0, // gpo3_slave_tx_enable *** adi,gpo3-slave-tx-enable
    0, // gpo0_rx_delay_us *** adi,gpo0-rx-delay-us
    0, // gpo0_tx_delay_us *** adi,gpo0-tx-delay-us
    0, // gpo1_rx_delay_us *** adi,gpo1-rx-delay-us
    0, // gpo1_tx_delay_us *** adi,gpo1-tx-delay-us
    0, // gpo2_rx_delay_us *** adi,gpo2-rx-delay-us
    0, // gpo2_tx_delay_us *** adi,gpo2-tx-delay-us
    0, // gpo3_rx_delay_us *** adi,gpo3-rx-delay-us
    0, // gpo3_tx_delay_us *** adi,gpo3-tx-delay-us
    /* Tx Monitor Control */
    37000, // low_high_gain_threshold_mdB *** adi,txmon-low-high-thresh
    0,     // low_gain_dB *** adi,txmon-low-gain
    24,    // high_gain_dB *** adi,txmon-high-gain
    0,     // tx_mon_track_en *** adi,txmon-dc-tracking-enable
    0,     // one_shot_mode_en *** adi,txmon-one-shot-mode-enable
    511,   // tx_mon_delay *** adi,txmon-delay
    8192,  // tx_mon_duration *** adi,txmon-duration
    2,     // tx1_mon_front_end_gain *** adi,txmon-1-front-end-gain
    2,     // tx2_mon_front_end_gain *** adi,txmon-2-front-end-gain
    48,    // tx1_mon_lo_cm *** adi,txmon-1-lo-cm
    48,    // tx2_mon_lo_cm *** adi,txmon-2-lo-cm
    /* GPIO definitions */
    -1, // gpio_resetb *** reset-gpios
    /* MCS Sync */
    -1, // gpio_sync *** sync-gpios
    -1, // gpio_cal_sw1 *** cal-sw1-gpios
    -1, // gpio_cal_sw2 *** cal-sw2-gpios
    /* External LO clocks */
    NULL, //(*ad9361_rfpll_ext_recalc_rate)()
    NULL, //(*ad9361_rfpll_ext_round_rate)()
    NULL  //(*ad9361_rfpll_ext_set_rate)()
};

AD9361_RXFIRConfig rx_fir_config = {
    // BPF PASSBAND 3/20 fs to 1/4 fs
    3, // rx;
    0, // rx_gain;
    1, // rx_dec;
    {-4,    -6,    -37,   35,    186,    86,    -284, -315, 107,   219,
     -4,    271,   558,   -307,  -1182,  -356,  658,  157,  207,   1648,
     790,   -2525, -2553, 748,   865,    -476,  3737, 6560, -3583, -14731,
     -5278, 14819, 14819, -5278, -14731, -3583, 6560, 3737, -476,  865,
     748,   -2553, -2525, 790,   1648,   207,   157,  658,  -356,  -1182,
     -307,  558,   271,   -4,    219,    107,   -315, -284, 86,    186,
     35,    -37,   -6,    -4,    0,      0,     0,    0,    0,     0,
     0,     0,     0,     0,     0,      0,     0,    0,    0,     0,
     0,     0,     0,     0,     0,      0,     0,    0,    0,     0,
     0,     0,     0,     0,     0,      0,     0,    0,    0,     0,
     0,     0,     0,     0,     0,      0,     0,    0,    0,     0,
     0,     0,     0,     0,     0,      0,     0,    0,    0,     0,
     0,     0,     0,     0,     0,      0,     0,    0}, // rx_coef[128];
    64                                                    // rx_coef_size
};

AD9361_TXFIRConfig tx_fir_config = {
    // BPF PASSBAND 3/20 fs to 1/4 fs
    3,  // tx;
    -6, // tx_gain;
    1,  // tx_int;
    {-4,    -6,    -37,   35,    186,    86,    -284, -315, 107,   219,
     -4,    271,   558,   -307,  -1182,  -356,  658,  157,  207,   1648,
     790,   -2525, -2553, 748,   865,    -476,  3737, 6560, -3583, -14731,
     -5278, 14819, 14819, -5278, -14731, -3583, 6560, 3737, -476,  865,
     748,   -2553, -2525, 790,   1648,   207,   157,  658,  -356,  -1182,
     -307,  558,   271,   -4,    219,    107,   -315, -284, 86,    186,
     35,    -37,   -6,    -4,    0,      0,     0,    0,    0,     0,
     0,     0,     0,     0,     0,      0,     0,    0,    0,     0,
     0,     0,     0,     0,     0,      0,     0,    0,    0,     0,
     0,     0,     0,     0,     0,      0,     0,    0,    0,     0,
     0,     0,     0,     0,     0,      0,     0,    0,    0,     0,
     0,     0,     0,     0,     0,      0,     0,    0,    0,     0,
     0,     0,     0,     0,     0,      0,     0,    0}, // tx_coef[128];
    64                                                    // tx_coef_size
};

void RFThread::thread_main() {
  iq_sample temp_buf[256000];
  iq_sample *temp_buf_ptr;
  init_device();
  startStreaming();
  while (!stopRF) {
    // check for changed settings
    if (centerFreqChanged) {
      beginSettingChange();
      centerFreqChanged = false;
      cout << "center freq == " << centerFreq << endl;
      ad9361_set_rx_lo_freq(ad9361_phy, centerFreq);
      ad9361_set_tx_lo_freq(ad9361_phy, centerFreq);
      endSettingChange();
    }
    if (bandwidthChanged) {
      beginSettingChange();
      bandwidthChanged = false;
      uint32_t targetSampleRate = getSampleRateFromBw(bandwidth);
      ad9361_set_rx_rf_bandwidth(ad9361_phy, bandwidth);
      ad9361_set_tx_rf_bandwidth(ad9361_phy, bandwidth);
      ad9361_set_rx_sampling_freq(ad9361_phy, targetSampleRate);
      ad9361_set_tx_sampling_freq(ad9361_phy, targetSampleRate);
      uint32_t actualSampleRate;
      ad9361_get_rx_sampling_freq(ad9361_phy, &actualSampleRate);
      cout << "actual sample rate = " << (actualSampleRate / 1.0e6) << "MSPS"
           << endl;
      currentSampleRate = actualSampleRate;
      // tx sample rate also changes
      txGenConfigChanged = true;
      endSettingChange();
    }
    if (gainChanged) {
      beginSettingChange();
      gainChanged = false;
      ad9361_set_rx_rf_gain(ad9361_phy, 0, gain);
      endSettingChange();
    }
    if (agcEnabledChanged) {
      beginSettingChange();
      agcEnabledChanged = false;
      if (agcEnabled) {
        ad9361_set_rx_gain_control_mode(ad9361_phy, 0, RF_GAIN_SLOWATTACK_AGC);
      } else {
        ad9361_set_rx_gain_control_mode(ad9361_phy, 0, RF_GAIN_MGC);
      }
      endSettingChange();
    }
    if (inputPortChanged) {
      beginSettingChange();
      inputPortChanged = false;
      ad9361_set_rx_rf_port_input(ad9361_phy, inputPort);
      endSettingChange();
    }

    if (txEnableChanged) {
      if (transmitEnabled) {
        set_txrx_en(true, true);
      } else {
        set_txrx_en(false, true);
      }
      txGenConfigChanged = true;
      txEnableChanged = false;
    }

    if (txGenConfigChanged) {
      if (!transmitEnabled) {
        // Zeros
        siggen_config_write(0x00, 0x00);
      } else if (transmitSigMode == TxMode::TX_SINE) {
        uint64_t ddsFreq = (uint64_t(abs(transmitOffset)) * (1 << 24UL)) /
                           getCurrentSampleRate();
        if (ddsFreq >= (1 << 24UL))
          ddsFreq = (1 << 24UL) - 1;
        if (transmitOffset == 0) {
          // DC
          siggen_config_write(0x00, 0x03);
          // Maximum amplitude
          siggen_config_write(0x01, 0xFF << 24);
        } else if (transmitOffset > 0) {
          // Sine
          siggen_config_write(0x00, 0x01);
          // Maximum amplitude + set frequency
          siggen_config_write(0x01, (0xFF << 24) | (ddsFreq & 0xFFFFFF));
          // I = cosθ, Q = sinθ
          // θQ = θI - 90
          siggen_config_write(0x02, 3 * (4096 / 4));
        } else {
          // Sine
          siggen_config_write(0x00, 0x01);
          // Maximum amplitude + set frequency
          siggen_config_write(0x01, (0xFF << 24) | (ddsFreq & 0xFFFFFF));
          // I = cosθ, Q = -sinθ
          // θQ = θI + 90
          siggen_config_write(0x02, 1 * (4096 / 4));
        }
      } else if (transmitSigMode == TxMode::TX_NOISE) {
        siggen_config_write(0x00, 0x02);
      }
      txGenConfigChanged = false;
    };

    if (txPowerChanged) {
      int txAtten = (6 - txPower) * 1000;
      if (txAtten > 89750)
        txAtten = 89750;
      beginSettingChange();
      ad9361_set_tx_attenuation(ad9361_phy, 0, txAtten);
      endSettingChange();
      txPowerChanged = false;
    }
    // do Rx
    // get up to 256k samples per iteration; but don't spend more than 20ms
    // doing so
    auto rxstart = chrono::steady_clock::now();
    temp_buf_ptr = temp_buf;
    for (int i = 0; i < 250; i++) {
      temp_buf_ptr += rx_get_data(temp_buf_ptr);
      if ((chrono::steady_clock::now() - rxstart) > chrono::milliseconds(20))
        break;
    }
    {
      lock_guard<mutex> guard(sample_buf_mutex);
      for (auto ptr = temp_buf; ptr < temp_buf_ptr; ptr++) {
        sample_buf[sample_buf_idx] = *ptr;
        sample_buf_idx++;
        if (sample_buf_read_offset < sample_buf_size)
          sample_buf_read_offset++;
        if (sample_buf_idx >= sample_buf_size) {
          long long us = chrono::duration_cast<chrono::microseconds>(
                             (chrono::high_resolution_clock::now() - last_wrap))
                             .count();
          cout << "throughput = " << sample_buf_size / double(us) << "MSPS"
               << endl;
          sample_buf_idx = 0;
          last_wrap = chrono::high_resolution_clock::now();
        }
      }
    }
  }
  stopStreaming();
  set_txrx_en(false, false);
}

void RFThread::init_device() {
  stopStreaming();
  default_init_param.gpio_resetb = GPIO_RESET_PIN;
  default_init_param.gpio_sync = -1;
  default_init_param.gpio_cal_sw1 = -1;
  default_init_param.gpio_cal_sw2 = -1;
  gpio_init(GPIO_DEVICE_ID);
  gpio_direction(default_init_param.gpio_resetb, 1);

  spi_init(SPI_DEVICE_ID, 1, 0);

  ad9361_init(&ad9361_phy, &default_init_param);

  ad9361_set_tx_fir_config(ad9361_phy, tx_fir_config);
  ad9361_set_rx_fir_config(ad9361_phy, rx_fir_config);
  ad9361_set_trx_fir_en_dis(ad9361_phy, 0);
  // ad9361_bist_loopback(ad9361_phy, 1);
};

void RFThread::startStreaming() {
  set_txrx_en(false, false);
  // ad9361_set_en_state_machine_mode(ad9361_phy, ENSM_MODE_ALERT);
  // if (transmitEnabled) {
  // ad9361_set_en_state_machine_mode(ad9361_phy, ENSM_MODE_FDD);
  if (transmitEnabled) {
    set_txrx_en(true, true);
  } else {
    set_txrx_en(false, true);
  }
  // } else {
  //    //ad9361_set_en_state_machine_mode(ad9361_phy, ENSM_MODE_RX);
  //  }
  enter_rx_streaming_mode();
}

void RFThread::stopStreaming() {
  iq_sample temp_buf[1024];

  leave_rx_streaming_mode();

  // Clear FTDI buffer
  while (rx_get_data(temp_buf))
    ;
}

void RFThread::beginSettingChange() {
  // ad9361_set_en_state_machine_mode(ad9361_phy, ENSM_MODE_ALERT);
  // ad9361_set_en_state_machine_mode(ad9361_phy, ENSM_MODE_WAIT);
  stopStreaming();
  set_txrx_en(false, false);
}

void RFThread::endSettingChange() {
  // ad9361_set_en_state_machine_mode(ad9361_phy, ENSM_MODE_ALERT);
  // ad9361_set_en_state_machine_mode(ad9361_phy, ENSM_MODE_WAIT);
  // set_txrx_en(false, false);
  ad9361_do_calib(ad9361_phy, RFDC_CAL, 0);
  ad9361_do_calib(ad9361_phy, RX_QUAD_CAL, 0);
  ad9361_do_calib(ad9361_phy, RX_BB_TUNE_CAL, 0);
  ad9361_do_calib(ad9361_phy, TX_QUAD_CAL, 0);
  startStreaming();
}

uint32_t RFThread::getSampleRateFromBw(uint32_t bw) {
  const uint32_t max_fs = 61440000;
  const uint32_t min_fs = 1920000;
  uint32_t result_fs = 61440000;
  while ((result_fs > min_fs) && ((result_fs / 2) >= bw))
    result_fs /= 2;
  return result_fs;
}

uint32_t RFThread::getCurrentSampleRate() { return currentSampleRate; };

void RFThread::getSamples(float _Complex *buf, int n) {
  {
    lock_guard<mutex> guard(sample_buf_mutex);
    for (int i = 0; i < n; i++) {
      int idx = (sample_buf_idx + i - n) % sample_buf_size;
      if (idx < 0)
        idx += sample_buf_size;
      buf[i] = sample_buf[idx].i + _Complex_I * sample_buf[idx].q;
    }
  }
}

int RFThread::getRecentSamples(double _Complex *buf, int n) {
  {
    lock_guard<mutex> guard(sample_buf_mutex);
    int available_samples = sample_buf_read_offset;
    // cout << n << " " << available_samples << endl;
    n = min(n, available_samples);
    for (int i = 0; i < n; i++) {
      int idx = (sample_buf_idx + i - sample_buf_read_offset) % sample_buf_size;
      if (idx < 0)
        idx += sample_buf_size;
      buf[i] = sample_buf[idx].i + _Complex_I * sample_buf[idx].q;
    }
    sample_buf_read_offset -= n;
  }
  return n;
}

void RFThread::setCenterFreq(uint64_t freq) {
  centerFreq = freq;
  centerFreqChanged = true;
};

void RFThread::setBandwidth(uint32_t bw) {
  bandwidth = bw;
  bandwidthChanged = true;
}

void RFThread::setGain(int rxgain) {
  gain = rxgain;
  gainChanged = true;
}

void RFThread::setAgcEnable(bool agcEn) {
  agcEnabled = agcEn;
  agcEnabledChanged = true;
}

void RFThread::setInputPort(int port) {
  inputPort = port;
  inputPortChanged = true;
}

void RFThread::setTxEnable(bool en) {
  transmitEnabled = en;
  txEnableChanged = true;
  txGenConfigChanged = true;
}

void RFThread::setTxMode(TxMode mode) {
  transmitSigMode = mode;
  txGenConfigChanged = true;
}

void RFThread::setTxOffset(int32_t offset) {
  transmitOffset = offset;
  txGenConfigChanged = true;
}

void RFThread::setTxPower(int power) {
  txPower = power;
  txPowerChanged = true;
}

uint64_t RFThread::getCenterFreq() { return centerFreq; }
int RFThread::getGain() { return gain; };
bool RFThread::getAgcEnable() { return agcEnabled; };
int RFThread::getInputPort() { return inputPort; };

void RFThread::start() {
  centerFreqChanged = true;
  bandwidthChanged = true;
  gainChanged = true;
  agcEnabledChanged = true;
  inputPortChanged = true;
  stopRF = false;
  rf_thread = thread([this]() { this->thread_main(); });
}

void RFThread::stop() {
  stopRF = true;
  rf_thread.join();
}
