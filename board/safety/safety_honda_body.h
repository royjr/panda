AddrCheckStruct honda_body_addr_checks[] = {
  {.msg = {{0x296, 1, 4, .check_checksum = false, .max_counter = 3U, .expected_timestep = 40000U}, { 0 }, { 0 }}},
};
#define HONDA_BODY_ADDR_CHECKS_LEN (sizeof(honda_body_addr_checks) / sizeof(honda_body_addr_checks[0]))
addr_checks honda_body_rx_checks = {honda_body_addr_checks, HONDA_BODY_ADDR_CHECKS_LEN};

// void can_send(CANPacket_t *to_push, uint8_t bus_number, bool skip_tx_hook);
//
// void stop_test(CANPacket_t *to_push){
//   uint8_t bus_num = 1;
//   uint32_t msg_addr = 0x16F118F0;
//   uint8_t msg_len = 8U;
//
//   CANPacket_t to_go;
//   // move the id 3 bits left and then add binary 101 for extended=true, rtr=false, txrequest=true
//   to_go.RIR = (msg_addr << 3) | 5U;
//   to_go.RDTR = (to_push->RDTR & 0xFFFFFFF0) | msg_len;
//   to_go.RDLR = 0x00000020;
//   to_go.RDHR = 0x0;
//
//   can_send(&to_go, bus_num, true);
// }

// void stop_test(CANPacket_t *to_push);

static int honda_body_rx_hook(CANPacket_t *to_push) {
  UNUSED(to_push);
  return true;
}

static int honda_body_tx_hook(CANPacket_t *to_send) {
  UNUSED(to_send);
  return true;
}

static const addr_checks* honda_body_init(uint16_t param) {
  UNUSED(param);
  honda_body_rx_checks = (addr_checks){honda_body_addr_checks, HONDA_BODY_ADDR_CHECKS_LEN};
  controls_allowed = false;
  return &honda_body_rx_checks;
}

static int honda_body_fwd_hook(int bus_num, CANPacket_t *to_fwd) {
  int addr = GET_ADDR(to_fwd);
  int bus_fwd = -1;

  // bus 0: powertrain bus (eps, adas control, etc)
  // bus 1: body bus
  // bus 2: obd2 bus
  int bus_powertrain = 0;
  int bus_body = 1;
  int bus_obd2 = 2;

  if (bus_num == bus_powertrain) {
    // Forward specific frames to B-CAN
    // TODO: add data safety where needed
    int security_msg = (addr == 0xef81218);
    int kwp_can_msg = (addr == 0x16f118f0);
    int wakeup_msg = (addr == 0x1e12ff18);
    if (security_msg || kwp_can_msg || wakeup_msg) {
      bus_fwd = bus_body;
    }

    // Forward permitted ISO-TP requests to OBD2 
    // TODO: Add the rest of the iso_tp msg spectrum
    bool isotp_tx_msg = ((addr & 0x1FFF00FF) == 0x18DA00F1);
    if (isotp_tx_msg) {
      int dat = GET_BYTES_04(to_fwd);
      // Data queries only
      // TODO: Injector kill/enable via Athena?
      if ((dat & 0xFF) == 0x22){
        bus_fwd = bus_obd2;
      }
    }
  }

  // Forward all B-CAN to OP's CAN0
  if (bus_num == bus_body) {
    bus_fwd = bus_powertrain;
  }
  
  // Forward ISO-TP replies to OP's CAN0 to query temperature, range, odometer etc.
  // TODO: Add the rest of the iso_tp msg spectrum
  if (bus_num == bus_obd2) {
    bool isotx_rx_msg = ((addr & 0x1FFFFF00) == 0x18DAF100);
    if (isotx_rx_msg){
      bus_fwd = bus_powertrain;
    }
  }

  return bus_fwd;
}

const safety_hooks honda_body_hooks = {
  .init = honda_body_init,
  .rx = honda_body_rx_hook,
  .tx = honda_body_tx_hook,
  .tx_lin = nooutput_tx_lin_hook,
  .fwd = honda_body_fwd_hook,
};
