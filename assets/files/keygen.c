#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef SERIAL_LEN
#define SERIAL_LEN 21
#endif

int to_ascii(int);
int on_bits(char *, size_t);

int main(void) {
  char *username;
  char serial[SERIAL_LEN+1];
  size_t username_len;
  int n, c;
  uint64_t u;
  char *p;

  srand(time(NULL));

  username_len = rand() % 0x10 + 5;
  if (!(username = malloc(username_len + 1))) {
    puts("malloc failed");
    exit(EXIT_FAILURE);
  }

  for (size_t i = 0; i < username_len; ++i) {
    username[i] = rand() % 2 ? 0x41: 0x61;
    username[i] = username[i] + rand() % 0x1a;
  }

  username[username_len] = 0;

  serial[0] = to_ascii(((username_len >> 4) & 15));
  serial[1] = to_ascii((username_len & 15));

  n = on_bits(username, username_len);
  serial[2] = to_ascii(((n >> 4) & 15));
  serial[3] = to_ascii((n & 15));

  serial[4] = 45;

  u = 0xb7e151628aed2a6a;
  p = (char *) &u;

  for (size_t i = 0; i < username_len; ++i) {
    c = username[i] & 0xff;
    n = 0; 
    for (size_t j = 0; j < 8; ++j) {
      n = n + ((c >> j) & 1);
    }
    for (size_t j = 0; j < n; ++j) {
      u = (u << 1) | ((u >> 63) & 1);
    }
    c = username[i] & 0xff;
    if ((n & 1)) c = (~c) & 0xff;
    *p = (*p ^ c) & 0xff;
  }

  p = (char *) &u; 

  for (size_t i = 5; i < SERIAL_LEN; i+=2) {
    serial[i]   = to_ascii(((p[(7-((i-5)>>1))] >> 4) & 15));
    serial[i+1] = to_ascii((p[(7-((i-5)>>1))] & 15));
  }

  serial[SERIAL_LEN] = 0;

  printf("%s\n%s\n", username, serial);

  free(username);

  return EXIT_SUCCESS;
}

int to_ascii(int c) {
  return c < 10 ? c + 48 : c + 55;
}

int on_bits(char *s, size_t len) {
  int n, c;
  n = 0;
  for (size_t i = 0; i < len; ++i) {
    c = s[i] & 0xff;
    for (size_t j = 0; j < 8; ++j) {
      n = n + ((c >> j) & 1);
    }
  }
  return n;
}

