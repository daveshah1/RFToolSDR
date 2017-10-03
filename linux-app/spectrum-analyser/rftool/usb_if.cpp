#include "usb_if.hpp"
#include "ftd3xx/ftd3xx.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>
using namespace std;

namespace RFTool {

static const bool debug_enable = false;

void USBInterface::flush() { FT_FlushPipe(handle, 0); }

// Build a 32-byte command from command type and payload
void USBInterface::build_command(uint8_t cmd_type,
                                 const vector<uint8_t> &payload,
                                 uint8_t *buffer) {
  fill(buffer, buffer + command_len, 0);
  // Magic
  buffer[0] = 0x43;
  buffer[1] = 0x4d;
  buffer[2] = 0x4e;
  buffer[3] = 0x44;
  // Cmd type
  buffer[4] = cmd_type;
  for (size_t i = 0; i < min(payload.size(), size_t(command_len - 5)); i++) {
    buffer[i + 5] = payload[i];
  }
  swap_endian(buffer, command_len);
};

bool USBInterface::exec_command(uint8_t *command, bool get_response,
                                uint8_t *response) {
  flush();
  FT_STATUS err;
  ULONG count;
  if (debug_enable)
    cout << "---------------------" << endl;
  print_command(command);
  err = FT_WritePipeEx(handle, 0, command, command_len, &count, timeout);
  if (err != FT_OK) {
    cerr << "write cmd returned error code " << int(err) << endl;
    return false;
  }
  if (get_response) {
    err = FT_ReadPipeEx(handle, 0, response, response_len, &count, timeout);
    if (err != FT_OK) {
      cerr << "read resp returned error code " << int(err) << endl;
      return false;
    }
    print_response(response);
  }
  return true;
};

// Print the response to cout
void USBInterface::print_response(uint8_t *resp) {
  if (debug_enable) {
    const int line_width = 16;
    // Save and restore cout state
    ios::fmtflags f(cout.flags());
    cout << "Response:" << endl;
    for (int i = 0; i < response_len; i++) {
      cout << hex << setw(2) << setfill('0') << int(resp[i]);
      if ((i % line_width) == (line_width - 1)) {
        cout << endl;
      } else {
        cout << " ";
      }
    }
    cout << endl;
    cout.flags(f);
  }
}

void USBInterface::print_command(uint8_t *resp) {
  if (debug_enable) {
    const int line_width = 16;
    // Save and restore cout state
    ios::fmtflags f(cout.flags());
    cout << "Command:" << endl;
    for (int i = 0; i < command_len; i++) {
      cout << hex << setw(2) << setfill('0') << int(resp[i]);
      if ((i % line_width) == (line_width - 1)) {
        cout << endl;
      } else {
        cout << " ";
      }
    }
    cout << endl;
    cout.flags(f);
  }
}

bool USBInterface::spi_transfer(bool is_read, int len, uint16_t addr,
                                uint8_t *buf) {
  uint8_t cmd[command_len], resp[response_len];
  for (int i = 0; i < command_len; i++)
    cmd[i] = 0;
  uint16_t hdr = 0;
  hdr |= is_read ? 0x0000 : 0x8000;
  hdr |= ((len - 1) & 0x07) << 12;
  hdr |= addr & 0x3FF;
  vector<uint8_t> payload;
  payload.push_back(uint8_t(len + 2)); // 2 byte hdr + user data
  payload.push_back((hdr >> 8) & 0xFF);
  payload.push_back(hdr & 0xFF);
  if (!is_read)
    for (int i = 0; i < len; i++)
      payload.push_back(buf[i]);
  build_command(0x10, payload, cmd);
  bool res = exec_command(cmd, true, resp);
  swap_endian(resp, response_len);
  if (res)
    print_response(resp);
  if (is_read)
    for (int i = 0; i < len; i++)
      buf[i] = resp[8 + i];
  return res;
};

bool USBInterface::set_fpga_ctrl(uint8_t ctrl_val) {
  uint8_t cmd[command_len], resp[response_len];
  build_command(0x20, vector<uint8_t>{ctrl_val}, cmd);
  bool res = exec_command(cmd, true, resp);
  if (res)
    print_response(resp);
  return res;
};

bool USBInterface::get_rx_packet(uint8_t *buffer) {
  ULONG count;
  FT_STATUS err =
      FT_ReadPipeEx(handle, 0, buffer, rx_packet_len, &count, timeout);
  if (err != FT_OK) {
    cerr << "read rx returned error code " << int(err) << endl;
    return false;
  }
  // cout << count << endl;
  // swap_endian(buffer, rx_packet_len);
  return true;
};

void USBInterface::get_version(void) {
  DWORD dwVersion;

  FT_GetDriverVersion(NULL, &dwVersion);
  printf("Driver version:%d.%d.%d.%d\r\n", dwVersion >> 24,
         (uint8_t)(dwVersion >> 16), (uint8_t)(dwVersion >> 8),
         dwVersion & 0xFF);

  FT_GetLibraryVersion(&dwVersion);
  printf("Library version:%d.%d.%d\r\n", dwVersion >> 24,
         (uint8_t)(dwVersion >> 16), dwVersion & 0xFFFF);
}

void USBInterface::get_vid_pid() {
  WORD vid, pid;

  if (FT_OK != FT_GetVIDPID(handle, &vid, &pid))
    return;
  printf("VID:%04X PID:%04X\r\n", vid, pid);
}

void USBInterface::turn_off_all_pipes() {
  FT_TRANSFER_CONF conf;

  memset(&conf, 0, sizeof(FT_TRANSFER_CONF));
  conf.wStructSize = sizeof(FT_TRANSFER_CONF);
  conf.pipe[FT_PIPE_DIR_IN].fPipeNotUsed = true;
  conf.pipe[FT_PIPE_DIR_OUT].fPipeNotUsed = true;
  for (DWORD i = 0; i < 4; i++)
    FT_SetTransferParams(&conf, i);
}

bool USBInterface::get_device_lists(int timeout_ms) {
  DWORD count;
  FT_DEVICE_LIST_INFO_NODE nodes[16];

  chrono::steady_clock::time_point const timeout =
      chrono::steady_clock::now() + chrono::milliseconds(timeout_ms);

  do {
    if (FT_OK == FT_CreateDeviceInfoList(&count))
      break;
    this_thread::sleep_for(chrono::microseconds(10));
  } while (chrono::steady_clock::now() < timeout);
  printf("Total %u device(s)\r\n", count);
  if (!count)
    return false;

  if (FT_OK != FT_GetDeviceInfoList(nodes, &count))
    return false;
  return true;
}

bool USBInterface::set_ft600_channel_config(FT_60XCONFIGURATION *cfg,
                                            CONFIGURATION_FIFO_CLK clock,
                                            bool is_600_mode) {
  bool needs_update = false;
  bool current_is_600mode;

  if (cfg->OptionalFeatureSupport &
      CONFIGURATION_OPTIONAL_FEATURE_ENABLENOTIFICATIONMESSAGE_INCHALL) {
    /* Notification in D3XX for Linux is implemented at OS level
     * Turn off notification feature in firmware */
    cfg->OptionalFeatureSupport &=
        ~CONFIGURATION_OPTIONAL_FEATURE_ENABLENOTIFICATIONMESSAGE_INCHALL;
    needs_update = true;
    printf("Turn off firmware notification feature\r\n");
  }

  if (!(cfg->OptionalFeatureSupport &
        CONFIGURATION_OPTIONAL_FEATURE_DISABLECANCELSESSIONUNDERRUN)) {
    /* Turn off feature not supported by D3XX for Linux */
    cfg->OptionalFeatureSupport |=
        CONFIGURATION_OPTIONAL_FEATURE_DISABLECANCELSESSIONUNDERRUN;
    needs_update = true;
    printf("disable cancel session on FIFO underrun 0x%X\r\n",
           cfg->OptionalFeatureSupport);
  }

  if (cfg->FIFOClock != clock)
    needs_update = true;

  if (cfg->FIFOMode == CONFIGURATION_FIFO_MODE_245) {
    printf("FIFO is running at FT245 mode\r\n");
    current_is_600mode = false;
  } else if (cfg->FIFOMode == CONFIGURATION_FIFO_MODE_600) {
    printf("FIFO is running at FT600 mode\r\n");
    current_is_600mode = true;
  } else {
    printf("FIFO is running at unknown mode\r\n");
    exit(-1);
  }

  UCHAR ch;

  if (in_ch_cnt == 1 && out_ch_cnt == 0)
    ch = CONFIGURATION_CHANNEL_CONFIG_1_INPIPE;
  else if (in_ch_cnt == 0 && out_ch_cnt == 1)
    ch = CONFIGURATION_CHANNEL_CONFIG_1_OUTPIPE;
  else {
    UCHAR total = in_ch_cnt < out_ch_cnt ? out_ch_cnt : in_ch_cnt;

    if (total == 4)
      ch = CONFIGURATION_CHANNEL_CONFIG_4;
    else if (total == 2)
      ch = CONFIGURATION_CHANNEL_CONFIG_2;
    else
      ch = CONFIGURATION_CHANNEL_CONFIG_1;

    if (cfg->FIFOMode == CONFIGURATION_FIFO_MODE_245 && total > 1) {
      printf("245 mode only support single channel\r\n");
      return false;
    }
  }

  if (cfg->ChannelConfig == ch && current_is_600mode == is_600_mode &&
      !needs_update)
    return false;
  cfg->ChannelConfig = ch;
  cfg->FIFOClock = clock;
  cfg->FIFOMode =
      is_600_mode ? CONFIGURATION_FIFO_MODE_600 : CONFIGURATION_FIFO_MODE_245;
  return true;
}

bool USBInterface::set_channel_config(bool is_600_mode,
                                      CONFIGURATION_FIFO_CLK clock) {
  DWORD dwType;

  /* Must turn off all pipes before changing chip configuration */
  turn_off_all_pipes();

  FT_GetDeviceInfoDetail(0, NULL, &dwType, NULL, NULL, NULL, NULL, &handle);
  if (!handle)
    return false;

  get_vid_pid();

  union {
    FT_60XCONFIGURATION ft600;
  } cfg;
  if (FT_OK != FT_GetChipConfiguration(handle, &cfg)) {
    printf("Failed to get chip conf\r\n");
    return false;
  }

  bool needs_update;
  needs_update = set_ft600_channel_config(&cfg.ft600, clock, is_600_mode);
  if (needs_update) {
    if (FT_OK != FT_SetChipConfiguration(handle, &cfg))
      printf("Failed to set chip conf\r\n");
    else {
      printf("Configuration changed\r\n");
      this_thread::sleep_for(chrono::seconds(1));
      get_device_lists(6000);
    }
  }

  if (dwType == FT_DEVICE_600 || dwType == FT_DEVICE_601) {
    bool rev_a_chip;
    DWORD dwVersion;

    FT_GetFirmwareVersion(handle, &dwVersion);
    rev_a_chip = dwVersion <= 0x105;

    FT_Close(handle);
    return rev_a_chip;
  }

  FT_Close(handle);
  return false;
}

void USBInterface::turn_off_thread_safe() {
  FT_TRANSFER_CONF conf;

  memset(&conf, 0, sizeof(FT_TRANSFER_CONF));
  conf.wStructSize = sizeof(FT_TRANSFER_CONF);
  conf.pipe[FT_PIPE_DIR_IN].fNonThreadSafeTransfer = true;
  conf.pipe[FT_PIPE_DIR_OUT].fNonThreadSafeTransfer = true;
  conf.pipe[FT_PIPE_DIR_IN].dwURBBufferSize = 512UL * 1024UL;
  conf.pipe[FT_PIPE_DIR_OUT].dwURBBufferSize = 512UL * 1024UL;

  for (DWORD i = 0; i < 4; i++)
    FT_SetTransferParams(&conf, i);
}

void USBInterface::get_queue_status() {
  for (uint8_t channel = 0; channel < out_ch_cnt; channel++) {
    DWORD dwBufferred;

    if (FT_OK != FT_GetUnsentBuffer(handle, channel, NULL, &dwBufferred)) {
      printf("Failed to get unsent buffer size\r\n");
      continue;
    }
    unique_ptr<uint8_t[]> p(new uint8_t[dwBufferred]);

    printf("CH%d OUT unsent buffer size in queue:%u\r\n", channel, dwBufferred);
    if (FT_OK != FT_GetUnsentBuffer(handle, channel, p.get(), &dwBufferred)) {
      printf("Failed to read unsent buffer size\r\n");
      continue;
    }
  }

  for (uint8_t channel = 0; channel < in_ch_cnt; channel++) {
    DWORD dwBufferred;

    if (FT_OK != FT_GetReadQueueStatus(handle, channel, &dwBufferred))
      continue;
    printf("CH%d IN unread buffer size in queue:%u\r\n", channel, dwBufferred);
  }
}

USBInterface::USBInterface() {
  get_version();

  if (!get_device_lists(500))
    throw runtime_error("failed to get device lists");
  rev_a_chip = set_channel_config(fifo_600mode, CONFIGURATION_FIFO_CLK_100);
  // turn_off_thread_safe(); // Check this?
  FT_Create(0, FT_OPEN_BY_INDEX, &handle);

  if (!handle) {
    throw runtime_error("failed to create device");
  }
}

USBInterface::~USBInterface() {
  if (rev_a_chip)
    FT_ResetDevicePort(handle);
  FT_Close(handle);
};
}
