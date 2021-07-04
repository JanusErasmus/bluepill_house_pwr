#include <stdint.h>
#include <stdio.h>


void timeToString(uint32_t now, char * string)
{
  if(now < 60)
  {
    sprintf(string, "%ds", (int)now);
  }
  else
  {
    int min = now / 60;
    now %= 60;
    if(min < 60)
    {
      sprintf(string, "%dm %ds", min, (int)now);
    }
    else
    {
      int hour = min / 60;
      min %= 60;
      if(hour < 24)
      {
        sprintf(string, "%dh %dm %ds", hour, min, (int)now);
      }
      else
      {
        int day = hour / 24;
        hour %= 24;
        sprintf(string, "%dd %dh %dm %ds", day, hour, min ,(int)now);
      }
    }
  }
}

void
diag_vdump_buf_with_offset(
      uint8_t    *p,
      uint32_t   s,
      uint8_t    *base
      )
{
    int i, c;
    if ((uint32_t)s > (uint32_t)p) {
        s = (uint32_t)s - (uint32_t)p;
    }
    while ((int)s > 0) {
        if (base) {
            printf("%08X: ", (int)((uint32_t)p - (uint32_t)base));
        } else {
            printf("%08X: ", (int)p);
        }
        for (i = 0;  i < 16;  i++) {
            if (i < (int)s) {
                printf("%02X ", p[i] & 0xFF);
            } else {
                printf("   ");
            }
        if (i == 7) printf(" ");
        }
        printf(" |");
        for (i = 0;  i < 16;  i++) {
            if (i < (int)s) {
                c = p[i] & 0xFF;
                if ((c < 0x20) || (c >= 0x7F)) c = '.';
            } else {
                c = ' ';
            }
            printf("%c", c);
        }
        printf("|\n");
        s -= 16;
        p += 16;
    }
}

void
diag_dump_buf_with_offset(
      uint8_t    *p,
      uint32_t   s,
      uint8_t    *base
      )
{
    diag_vdump_buf_with_offset(p, s, base);
}

void diag_dump_buf(void *p, uint32_t s)
{
   diag_dump_buf_with_offset((uint8_t *)p, s, 0);
}
