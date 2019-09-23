#include "fabrication.h"
#include "oath.h"
#include "openpgp.h"
#include "piv.h"
#include "rand.h"
#include "u2f.h"
#include <admin.h>
#include <aes.h>
#include <apdu.h>
#include <ctap.h>
#include <emubd/lfs_emubd.h>
#include <fs.h>
#include <lfs.h>

static struct lfs_config cfg;
static lfs_emubd_t bd;

uint8_t private_key[] = {0xD9, 0x5C, 0x12, 0x15, 0xD1, 0x0A, 0xBB, 0x57, 0x91, 0xB6, 0x47,
                         0x52, 0xDF, 0x9D, 0x25, 0x3C, 0xA4, 0x17, 0x31, 0x37, 0x5D, 0x41,
                         0xCD, 0x9C, 0xD9, 0x3C, 0xDA, 0x00, 0x51, 0x36, 0xE6, 0x4E};
uint8_t cert[] = {
    0x30, 0x82, 0x01, 0xc3, 0x30, 0x82, 0x01, 0x6a, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x08, 0x1b, 0xde, 0x06, 0x7b,
    0x4c, 0xd9, 0x49, 0xe8, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x30, 0x4f, 0x31,
    0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x63, 0x6e, 0x31, 0x0d, 0x30, 0x0b, 0x06, 0x03, 0x55,
    0x04, 0x0a, 0x13, 0x04, 0x7a, 0x34, 0x79, 0x78, 0x31, 0x22, 0x30, 0x20, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x13, 0x19,
    0x41, 0x75, 0x74, 0x68, 0x65, 0x6e, 0x74, 0x69, 0x63, 0x61, 0x74, 0x6f, 0x72, 0x20, 0x41, 0x74, 0x74, 0x65, 0x73,
    0x74, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x31, 0x0d, 0x30, 0x0b, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x04, 0x7a, 0x34,
    0x79, 0x78, 0x30, 0x1e, 0x17, 0x0d, 0x31, 0x39, 0x30, 0x39, 0x32, 0x30, 0x31, 0x33, 0x31, 0x32, 0x30, 0x30, 0x5a,
    0x17, 0x0d, 0x32, 0x30, 0x30, 0x39, 0x32, 0x30, 0x31, 0x33, 0x31, 0x32, 0x30, 0x30, 0x5a, 0x30, 0x4f, 0x31, 0x0b,
    0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x63, 0x6e, 0x31, 0x0d, 0x30, 0x0b, 0x06, 0x03, 0x55, 0x04,
    0x0a, 0x13, 0x04, 0x7a, 0x34, 0x79, 0x78, 0x31, 0x22, 0x30, 0x20, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x13, 0x19, 0x41,
    0x75, 0x74, 0x68, 0x65, 0x6e, 0x74, 0x69, 0x63, 0x61, 0x74, 0x6f, 0x72, 0x20, 0x41, 0x74, 0x74, 0x65, 0x73, 0x74,
    0x61, 0x74, 0x69, 0x6f, 0x6e, 0x31, 0x0d, 0x30, 0x0b, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x04, 0x7a, 0x34, 0x79,
    0x78, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a, 0x86, 0x48,
    0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x8e, 0x6e, 0x62, 0x2a, 0xad, 0x84, 0x87, 0x00, 0xe2, 0xba,
    0x76, 0x4a, 0x0f, 0xbe, 0x68, 0xbe, 0xdb, 0xcc, 0x61, 0x2d, 0xaa, 0x11, 0x00, 0x46, 0x07, 0x16, 0xc1, 0x3c, 0x5d,
    0x96, 0x32, 0xc3, 0xae, 0x49, 0xf4, 0xe9, 0xa2, 0xdb, 0x6f, 0xd5, 0xee, 0x2b, 0x64, 0x53, 0xfa, 0x7b, 0x3d, 0x2f,
    0x1b, 0xda, 0xa7, 0xe5, 0x51, 0x6f, 0x4d, 0x53, 0x32, 0x40, 0x97, 0x10, 0xf3, 0x0e, 0x8e, 0xfc, 0xa3, 0x30, 0x30,
    0x2e, 0x30, 0x09, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x04, 0x02, 0x30, 0x00, 0x30, 0x21, 0x06, 0x0b, 0x2b, 0x06, 0x01,
    0x04, 0x01, 0x82, 0xe5, 0x1c, 0x01, 0x01, 0x04, 0x04, 0x12, 0x00, 0x00, 0x24, 0x4e, 0xb2, 0x9e, 0xe0, 0x90, 0x4e,
    0x49, 0x81, 0xfe, 0x1f, 0x20, 0xf8, 0xd3, 0xb8, 0xf4, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04,
    0x03, 0x02, 0x03, 0x47, 0x00, 0x30, 0x44, 0x02, 0x20, 0x12, 0x87, 0xb2, 0x31, 0x72, 0xf0, 0x2a, 0x97, 0x17, 0xa0,
    0xb6, 0x27, 0xda, 0x39, 0x36, 0x26, 0x4f, 0x45, 0x21, 0xe4, 0x58, 0x45, 0x52, 0x15, 0x78, 0x45, 0x99, 0xa5, 0xbe,
    0x7d, 0xfa, 0x7d, 0x02, 0x20, 0x20, 0x79, 0x69, 0x3a, 0x78, 0x31, 0xd3, 0xec, 0x3a, 0x9b, 0x17, 0x1b, 0x30, 0x47,
    0x04, 0xa8, 0x35, 0x3b, 0x5b, 0x58, 0xd5, 0x57, 0xf5, 0x77, 0x59, 0xeb, 0x7a, 0x07, 0xba, 0x0e, 0x1c, 0x67};

static void fake_u2f_personalization() {

  uint8_t c_buf[1024], r_buf[1024];
  CAPDU capdu;
  RAPDU rapdu;
  capdu.data = c_buf;
  rapdu.data = r_buf;

  apdu_fill_with_command(&capdu, "00 20 00 00 06 31 32 33 34 35 36");
  admin_process_apdu(&capdu, &rapdu);

  capdu.cla = 0x00;
  capdu.ins = ADMIN_INS_WRITE_U2F_PRIVATE_KEY;
  capdu.data = private_key;
  capdu.lc = 32;

  admin_process_apdu(&capdu, &rapdu);

  capdu.ins = ADMIN_INS_WRITE_U2F_CERT;
  capdu.data = cert;
  capdu.lc = sizeof(cert);
  admin_process_apdu(&capdu, &rapdu);
}

static void fido2_init() {
  //uint8_t buf[4] = {0};
  //if(get_file_size("ctap_cert") > 0)
  //  return;
  ctap_install(0);
  write_file("ctap_cert", cert, 0, sizeof(cert), 1);
  write_attr("ctap_cert", 0x00, private_key, sizeof(private_key));

  u2f_config(16, aes128_enc, aes128_dec);
  fake_u2f_personalization();
}


int card_fabrication_procedure() {
  memset(&cfg, 0, sizeof(cfg));
  cfg.context = &bd;
  cfg.read = &lfs_emubd_read;
  cfg.prog = &lfs_emubd_prog;
  cfg.erase = &lfs_emubd_erase;
  cfg.sync = &lfs_emubd_sync;
  cfg.read_size = 1;
  cfg.prog_size = 512;
  cfg.block_size = 512;
  cfg.block_count = 256;
  cfg.block_cycles = 50000;
  cfg.cache_size = 512;
  cfg.lookahead_size = 16;
  lfs_emubd_create(&cfg, "lfs-root");

  fs_init(&cfg);
  admin_install();
  oath_install(0);

  fido2_init();

  static uint8_t piv_buffer[2048];
  piv_config(piv_buffer, sizeof(piv_buffer));
  piv_install(0);

  openpgp_install(0);
  return 0;
}
