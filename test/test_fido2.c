#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "../ctap/ctap-errors.h"
#include "../ctap/ctap-parser.h"
#include <cbor.h>
#include <crypto-util.h>
#include <emubd/lfs_emubd.h>
#include <fs.h>
#include <lfs.h>
#include <rand.h>

static void test_parse_user(void **state) {
  (void)state;

  CborParser parser;
  CborValue it;
  UserEntity user;
  uint8_t input[] = {
      0xa4, 0x62, 0x69, 0x64, 0x58, 0x20, 0x30, 0x82, 0x01, 0x93, 0x30, 0x82,
      0x01, 0x38, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x30, 0x82, 0x01, 0x93, 0x30,
      0x82, 0x01, 0x38, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x30, 0x82, 0x01, 0x93,
      0x30, 0x82, 0x64, 0x69, 0x63, 0x6f, 0x6e, 0x78, 0x2b, 0x68, 0x74, 0x74,
      0x70, 0x73, 0x3a, 0x2f, 0x2f, 0x70, 0x69, 0x63, 0x73, 0x2e, 0x65, 0x78,
      0x61, 0x6d, 0x70, 0x6c, 0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x2f, 0x30, 0x30,
      0x2f, 0x70, 0x2f, 0x61, 0x42, 0x6a, 0x6a, 0x6a, 0x70, 0x71, 0x50, 0x62,
      0x2e, 0x70, 0x6e, 0x67, 0x64, 0x6e, 0x61, 0x6d, 0x65, 0x76, 0x6a, 0x6f,
      0x68, 0x6e, 0x70, 0x73, 0x6d, 0x69, 0x74, 0x68, 0x40, 0x65, 0x78, 0x61,
      0x6d, 0x70, 0x6c, 0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x6b, 0x64, 0x69, 0x73,
      0x70, 0x6c, 0x61, 0x79, 0x4e, 0x61, 0x6d, 0x65, 0x6d, 0x4a, 0x6f, 0x68,
      0x6e, 0x20, 0x50, 0x2e, 0x20, 0x53, 0x6d, 0x69, 0x74, 0x68};
  cbor_parser_init(input, sizeof(input), CborValidateCanonicalFormat, &parser,
                   &it);
  assert_int_equal(parse_user(&user, &it), 0);
  assert_string_equal(user.name, "johnpsmith@example.com");
  assert_string_equal(user.displayName, "John P. Smith");
  assert_string_equal(user.icon, "https://pics.example.com/00/p/aBjjjpqPb.png");
}

static void test_parse_rp(void **state) {
  (void)state;

  CborParser parser;
  CborValue it;
  uint8_t input[] = {0xa2, 0x62, 0x69, 0x64, 0x6b, 0x65, 0x78, 0x61, 0x6d,
                     0x70, 0x6c, 0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x64, 0x6e,
                     0x61, 0x6d, 0x65, 0x64, 0x41, 0x63, 0x6d, 0x65};
  uint8_t output[32];
  uint8_t expected[] = {0xa3, 0x79, 0xa6, 0xf6, 0xee, 0xaf, 0xb9, 0xa5,
                        0x5e, 0x37, 0x8c, 0x11, 0x80, 0x34, 0xe2, 0x75,
                        0x1e, 0x68, 0x2f, 0xab, 0x9f, 0x2d, 0x30, 0xab,
                        0x13, 0xd2, 0x12, 0x55, 0x86, 0xce, 0x19, 0x47};
  cbor_parser_init(input, sizeof(input), CborValidateCanonicalFormat, &parser,
                   &it);
  assert_int_equal(parse_rp(output, &it), 0);
  for (int i = 0; i != 32; ++i)
    assert_int_equal(output[i], expected[i]);
}

static void test_parse_pub_key_cred_params(void **state) {
  (void)state;

  CborParser parser;
  CborValue it;
  uint8_t input[] = {0x82, 0xa2, 0x63, 0x61, 0x6c, 0x67, 0x26, 0x64, 0x74, 0x79,
                     0x70, 0x65, 0x6a, 0x70, 0x75, 0x62, 0x6C, 0x69, 0x63, 0x2D,
                     0x6B, 0x65, 0x79, 0xa2, 0x63, 0x61, 0x6c, 0x67, 0x39, 0x01,
                     0x00, 0x64, 0x74, 0x79, 0x70, 0x65, 0x6a, 0x70, 0x75, 0x62,
                     0x6C, 0x69, 0x63, 0x2D, 0x6B, 0x65, 0x79};
  cbor_parser_init(input, sizeof(input), CborValidateCanonicalFormat, &parser,
                   &it);
  assert_int_equal(parse_verify_pub_key_cred_params(&it), 0);
}

static void test_parse_verify_exclude_list(void **state) {
  (void)state;

  CborParser parser;
  CborValue it;
  uint8_t input[] = {0x82, 0xa2, 0x64, 't',  'y',  'p',  'e',  0x6a, 'p',
                     'u',  'b',  'l',  'i',  'c',  '-',  'k',  'e',  'y',
                     0x62, 'i',  'd',  0x41, '1',  0xa2, 0x64, 't',  'y',
                     'p',  'e',  0x6a, 'p',  'u',  'b',  'l',  'i',  'c',
                     '-',  'k',  'e',  'y',  0x62, 'i',  'd',  0x41, '2'};
  cbor_parser_init(input, sizeof(input), CborValidateCanonicalFormat, &parser,
                   &it);
  assert_int_equal(parse_public_key_credential_list(&it), 0);
  input[8] = 'r';
  assert_int_equal(parse_public_key_credential_list(&it),
                   CTAP2_ERR_INVALID_CREDENTIAL);
}

static void test_parse_options(void **state) {
  (void)state;

  CborParser parser;
  CborValue it;
  uint8_t input[] = {0xa1, 0x62, 'r', 'k', 0xf5}, rk = 0, up = 0, uv = 0;
  cbor_parser_init(input, sizeof(input), CborValidateCanonicalFormat, &parser,
                   &it);
  assert_int_equal(parse_options(&rk, &up, &uv, &it), 0);
  assert_int_equal(rk, 1);
  assert_int_equal(up, 0);
  assert_int_equal(uv, 0);
}

static void test_parse_cose_key(void **state) {
  (void)state;

  CborParser parser;
  CborValue it;
  uint8_t input[] = {0xA5, 0x01, 0x02, 0x03, 0x26, 0x20, 0x01, 0x21, 0x58, 0x20, 0xE8, 0x76, 0x25, 0x89, 0x6E, 0xE4,
                     0xE4, 0x6D, 0xC0, 0x32, 0x76, 0x6E, 0x80, 0x87, 0x96, 0x2F, 0x36, 0xDF, 0x9D, 0xFE, 0x8B, 0x56,
                     0x7F, 0x37, 0x63, 0x01, 0x5B, 0x19, 0x90, 0xA6, 0x0E, 0x14, 0x22, 0x58, 0x20, 0x27, 0xDE, 0x61,
                     0x2D, 0x66, 0x41, 0x8B, 0xDA, 0x19, 0x50, 0x58, 0x1E, 0xBC, 0x5C, 0x8C, 0x1D, 0xAD, 0x71, 0x0C,
                     0xB1, 0x4C, 0x22, 0xF8, 0xC9, 0x70, 0x45, 0xF4, 0x61, 0x2F, 0xB2, 0x0C, 0x91};
  uint8_t output[64];
  uint8_t expected[] = {0xE8, 0x76, 0x25, 0x89, 0x6E, 0xE4, 0xE4, 0x6D, 0xC0, 0x32, 0x76, 0x6E, 0x80, 0x87, 0x96, 0x2F,
                        0x36, 0xDF, 0x9D, 0xFE, 0x8B, 0x56, 0x7F, 0x37, 0x63, 0x01, 0x5B, 0x19, 0x90, 0xA6, 0x0E, 0x14,
                        0x27, 0xDE, 0x61, 0x2D, 0x66, 0x41, 0x8B, 0xDA, 0x19, 0x50, 0x58, 0x1E, 0xBC, 0x5C, 0x8C, 0x1D,
                        0xAD, 0x71, 0x0C, 0xB1, 0x4C, 0x22, 0xF8, 0xC9, 0x70, 0x45, 0xF4, 0x61, 0x2F, 0xB2, 0x0C, 0x91};
  cbor_parser_init(input, sizeof(input), CborValidateCanonicalFormat, &parser, &it);
  assert_int_equal(parse_cose_key(&it, output), 0);
  for (int i = 0; i != 64; ++i)
    assert_int_equal(output[i], expected[i]);
}

static void test_parse_make_credential(void **state) {
  (void)state;

  uint8_t input[] = {
      0xa5, 0x01, 0x58, 0x20, 0x68, 0x71, 0x34, 0x96, 0x82, 0x22, 0xec, 0x17,
      0x20, 0x2e, 0x42, 0x50, 0x5f, 0x8e, 0xd2, 0xb1, 0x6a, 0xe2, 0x2f, 0x16,
      0xbb, 0x05, 0xb8, 0x8c, 0x25, 0xdb, 0x9e, 0x60, 0x26, 0x45, 0xf1, 0x41,
      0x02, 0xa2, 0x62, 0x69, 0x64, 0x6b, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c,
      0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x64, 0x6e, 0x61, 0x6d, 0x65, 0x64, 0x41,
      0x63, 0x6d, 0x65, 0x03, 0xa4, 0x62, 0x69, 0x64, 0x58, 0x20, 0x30, 0x82,
      0x01, 0x93, 0x30, 0x82, 0x01, 0x38, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x30,
      0x82, 0x01, 0x93, 0x30, 0x82, 0x01, 0x38, 0xa0, 0x03, 0x02, 0x01, 0x02,
      0x30, 0x82, 0x01, 0x93, 0x30, 0x82, 0x64, 0x69, 0x63, 0x6f, 0x6e, 0x78,
      0x2b, 0x68, 0x74, 0x74, 0x70, 0x73, 0x3a, 0x2f, 0x2f, 0x70, 0x69, 0x63,
      0x73, 0x2e, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x2e, 0x63, 0x6f,
      0x6d, 0x2f, 0x30, 0x30, 0x2f, 0x70, 0x2f, 0x61, 0x42, 0x6a, 0x6a, 0x6a,
      0x70, 0x71, 0x50, 0x62, 0x2e, 0x70, 0x6e, 0x67, 0x64, 0x6e, 0x61, 0x6d,
      0x65, 0x76, 0x6a, 0x6f, 0x68, 0x6e, 0x70, 0x73, 0x6d, 0x69, 0x74, 0x68,
      0x40, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x2e, 0x63, 0x6f, 0x6d,
      0x6b, 0x64, 0x69, 0x73, 0x70, 0x6c, 0x61, 0x79, 0x4e, 0x61, 0x6d, 0x65,
      0x6d, 0x4a, 0x6f, 0x68, 0x6e, 0x20, 0x50, 0x2e, 0x20, 0x53, 0x6d, 0x69,
      0x74, 0x68, 0x04, 0x82, 0xa2, 0x63, 0x61, 0x6c, 0x67, 0x26, 0x64, 0x74,
      0x79, 0x70, 0x65, 0x6a, 0x70, 0x75, 0x62, 0x6C, 0x69, 0x63, 0x2D, 0x6B,
      0x65, 0x79, 0xa2, 0x63, 0x61, 0x6c, 0x67, 0x39, 0x01, 0x00, 0x64, 0x74,
      0x79, 0x70, 0x65, 0x6a, 0x70, 0x75, 0x62, 0x6C, 0x69, 0x63, 0x2D, 0x6B,
      0x65, 0x79, 0x07, 0xa1, 0x62, 0x72, 0x6b, 0xf5};
  CborParser parser;
  CTAP_makeCredential mc;
  assert_int_equal(parse_make_credential(&parser, &mc, input, sizeof(input)), 0);
}

static void test_parse_get_assertion(void **state) {
  (void)state;

  uint8_t input[] = {
      0xa3, 0x01, 0x6f, 0x64, 0x65, 0x6d, 0x6f, 0x2e, 0x79, 0x75, 0x62, 0x69,
      0x63, 0x6f, 0x2e, 0x63, 0x6f, 0x6d, 0x02, 0x58, 0x20, 0x8e, 0x92, 0x0a,
      0x2d, 0xc4, 0xea, 0xbe, 0xc8, 0xc2, 0x8f, 0x44, 0x8c, 0xac, 0x1d, 0x70,
      0x17, 0xcf, 0x5f, 0xf1, 0x9a, 0x5f, 0x53, 0xf3, 0xca, 0x03, 0xa4, 0x82,
      0xca, 0xcd, 0x21, 0xaf, 0x8c, 0x03, 0x81, 0xa2, 0x62, 0x69, 0x64, 0x58,
      0x40, 0x92, 0x4a, 0x84, 0xcf, 0x68, 0xca, 0xd2, 0x03, 0x90, 0x35, 0x5f,
      0x3c, 0xce, 0x8a, 0xf3, 0x9a, 0xba, 0xe8, 0x0e, 0x30, 0x48, 0x31, 0x6f,
      0x80, 0xf5, 0x0c, 0x96, 0xda, 0x7e, 0xb7, 0x2f, 0x37, 0x0b, 0xdb, 0xa6,
      0xc0, 0x4e, 0x06, 0x9c, 0x99, 0x8a, 0x19, 0x2d, 0x0a, 0x19, 0x25, 0x23,
      0x24, 0x69, 0x9d, 0x2a, 0x55, 0x4a, 0x75, 0x6e, 0xf6, 0xdb, 0x77, 0x45,
      0xf5, 0x44, 0xd2, 0x40, 0x00, 0x64, 0x74, 0x79, 0x70, 0x65, 0x6a, 0x70,
      0x75, 0x62, 0x6c, 0x69, 0x63, 0x2d, 0x6b, 0x65, 0x79};
  CborParser parser;
  CTAP_getAssertion ga;
  assert_int_equal(parse_get_assertion(&parser, &ga, input, sizeof(input)), 0);
}

static void test_ctap_make_credential(void **state) {
  (void)state;

  uint8_t input[] = {
      0x01, 0xa4, 0x01, 0x58, 0x20, 0x68, 0x71, 0x34, 0x96, 0x82, 0x22, 0xec, 0x17, 0x20, 0x2e, 0x42, 0x50, 0x5f, 0x8e,
      0xd2, 0xb1, 0x6a, 0xe2, 0x2f, 0x16, 0xbb, 0x05, 0xb8, 0x8c, 0x25, 0xdb, 0x9e, 0x60, 0x26, 0x45, 0xf1, 0x41, 0x02,
      0xa2, 0x62, 0x69, 0x64, 0x6b, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x64, 0x6e, 0x61,
      0x6d, 0x65, 0x64, 0x41, 0x63, 0x6d, 0x65, 0x03, 0xa4, 0x62, 0x69, 0x64, 0x58, 0x20, 0x30, 0x82, 0x01, 0x93, 0x30,
      0x82, 0x01, 0x38, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x30, 0x82, 0x01, 0x93, 0x30, 0x82, 0x01, 0x38, 0xa0, 0x03, 0x02,
      0x01, 0x02, 0x30, 0x82, 0x01, 0x93, 0x30, 0x82, 0x64, 0x69, 0x63, 0x6f, 0x6e, 0x78, 0x2b, 0x68, 0x74, 0x74, 0x70,
      0x73, 0x3a, 0x2f, 0x2f, 0x70, 0x69, 0x63, 0x73, 0x2e, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x2e, 0x63, 0x6f,
      0x6d, 0x2f, 0x30, 0x30, 0x2f, 0x70, 0x2f, 0x61, 0x42, 0x6a, 0x6a, 0x6a, 0x70, 0x71, 0x50, 0x62, 0x2e, 0x70, 0x6e,
      0x67, 0x64, 0x6e, 0x61, 0x6d, 0x65, 0x76, 0x6a, 0x6f, 0x68, 0x6e, 0x70, 0x73, 0x6d, 0x69, 0x74, 0x68, 0x40, 0x65,
      0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x6b, 0x64, 0x69, 0x73, 0x70, 0x6c, 0x61, 0x79, 0x4e,
      0x61, 0x6d, 0x65, 0x6d, 0x4a, 0x6f, 0x68, 0x6e, 0x20, 0x50, 0x2e, 0x20, 0x53, 0x6d, 0x69, 0x74, 0x68, 0x04, 0x82,
      0xa2, 0x63, 0x61, 0x6c, 0x67, 0x26, 0x64, 0x74, 0x79, 0x70, 0x65, 0x6a, 0x70, 0x75, 0x62, 0x6C, 0x69, 0x63, 0x2D,
      0x6B, 0x65, 0x79, 0xa2, 0x63, 0x61, 0x6c, 0x67, 0x39, 0x01, 0x00, 0x64, 0x74, 0x79, 0x70, 0x65, 0x6a, 0x70, 0x75,
      0x62, 0x6C, 0x69, 0x63, 0x2D, 0x6B, 0x65, 0x79};
  uint8_t output[1000];
  size_t output_size = sizeof(output);
  assert_int_equal(ctap_process(input, sizeof(input), output, &output_size),
                   0);
  print_hex(output, output_size);
}

static void test_ctap_get_assertion(void **state) {
  (void)state;

  uint8_t input[] = {
      0x02, 0xA3, 0x01, 0x6B, 0x65, 0x78, 0x61, 0x6D, 0x70, 0x6C, 0x65, 0x2E, 0x63, 0x6F, 0x6D, 0x02, 0x58, 0x20,
      0x68, 0x71, 0x34, 0x96, 0x82, 0x22, 0xEC, 0x17, 0x20, 0x2E, 0x42, 0x50, 0x5F, 0x8E, 0xD2, 0xB1, 0x6A, 0xE2,
      0x2F, 0x16, 0xBB, 0x05, 0xB8, 0x8C, 0x25, 0xDB, 0x9E, 0x60, 0x26, 0x45, 0xF1, 0x41, 0x03, 0x82, 0xA2, 0x62,
      0x69, 0x64, 0x58, 0x40, 0x02, 0x2A, 0xAB, 0xCF, 0x32, 0xE2, 0xA6, 0x9F, 0x52, 0x5B, 0x6F, 0xA6, 0xE2, 0xAD,
      0x68, 0x5D, 0xE7, 0xBE, 0xFD, 0x81, 0x1A, 0xAF, 0xF0, 0x94, 0xB1, 0x33, 0xF6, 0xCB, 0x5C, 0x19, 0xD1, 0xBC,
      0xA3, 0x79, 0xA6, 0xF6, 0xEE, 0xAF, 0xB9, 0xA5, 0x5E, 0x37, 0x8C, 0x11, 0x80, 0x34, 0xE2, 0x75, 0x1E, 0x68,
      0x2F, 0xAB, 0x9F, 0x2D, 0x30, 0xAB, 0x13, 0xD2, 0x12, 0x55, 0x86, 0xCE, 0x19, 0x47, 0x64, 0x74, 0x79, 0x70,
      0x65, 0x6A, 0x70, 0x75, 0x62, 0x6C, 0x69, 0x63, 0x2D, 0x6B, 0x65, 0x79, 0xA2, 0x62, 0x69, 0x64, 0x58, 0x32,
      0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
      0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
      0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x64, 0x74, 0x79, 0x70,
      0x65, 0x6A, 0x70, 0x75, 0x62, 0x6C, 0x69, 0x63, 0x2D, 0x6B, 0x65, 0x79};
  uint8_t output[1000];
  size_t output_size = sizeof(output);
  assert_int_equal(ctap_process(input, sizeof(input), output, &output_size),
                   0);
  print_hex(output, output_size);
}

static void test_ctap_client_pin(void **state) {
  (void)state;

  uint8_t input1[] = {0x06, 0xA2, 0x01, 0x01, 0x02, 0x01};  // get pin retries
  uint8_t output[1000];
  size_t output_size = sizeof(output);
  assert_int_equal(ctap_process(input1, sizeof(input1), output, &output_size),0);
  print_hex(output, output_size);

  uint8_t input2[] = {0x06, 0xA2, 0x01, 0x01, 0x02, 0x02};  // get key agreement
  assert_int_equal(ctap_process(input2, sizeof(input2), output, &output_size),0);
  print_hex(output, output_size);
}

static void test_ctap_get_info(void **state) {
  (void)state;

  uint8_t input[] = {0x04};
  uint8_t output[1000];
  size_t output_size = sizeof(output);
  assert_int_equal(ctap_process(input, sizeof(input), output, &output_size),
                   0);
  print_hex(output, output_size);
}

int main() {
  struct lfs_config cfg;
  lfs_emubd_t bd;
  memset(&cfg, 0, sizeof(cfg));
  cfg.context = &bd;
  cfg.read = &lfs_emubd_read;
  cfg.prog = &lfs_emubd_prog;
  cfg.erase = &lfs_emubd_erase;
  cfg.sync = &lfs_emubd_sync;
  cfg.read_size = 16;
  cfg.prog_size = 16;
  cfg.block_size = 512;
  cfg.block_count = 400;
  cfg.block_cycles = 50000;
  cfg.cache_size = 128;
  cfg.lookahead_size = 16;
  lfs_emubd_create(&cfg, "lfs-root");

  fs_init(&cfg);
  uint8_t buf[32];
  random_buffer(buf, 32);
  write_file("ctap_cert", NULL, 0, 0, 1);
  write_attr("ctap_cert", 0x00, buf, 32);
  write_attr("ctap_cert", 0x01, buf, 4);
  write_attr("ctap_cert", 0x02, NULL, 0);
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_parse_user),
      cmocka_unit_test(test_parse_rp),
      cmocka_unit_test(test_parse_pub_key_cred_params),
      cmocka_unit_test(test_parse_verify_exclude_list),
      cmocka_unit_test(test_parse_options),
      cmocka_unit_test(test_parse_make_credential),
      cmocka_unit_test(test_parse_get_assertion),
      cmocka_unit_test(test_parse_cose_key),
      cmocka_unit_test(test_ctap_get_info),
      cmocka_unit_test(test_ctap_make_credential),
      cmocka_unit_test(test_ctap_get_assertion),
      cmocka_unit_test(test_ctap_client_pin),
  };


  int ret = cmocka_run_group_tests(tests, NULL, NULL);

  lfs_emubd_destroy(&cfg);

  return ret;
}