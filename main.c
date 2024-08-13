#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include <sys/sysinfo.h>
#include <sys/utsname.h>

#include <linux/kernel.h>
#include <linux/unistd.h>

#define LOGO0 "._.     "
#define LOGO1 "| |     "
#define LOGO2 "| |     "
#define LOGO3 "| |     "
#define LOGO4 "| |     "
#define LOGO5 "| |____."
#define LOGO6 "|______|"

#define ATTR_PROP   "\033[1;34m"
#define RESET_PROP  "\033[0m"

#define LINEBLANK   "                                             "
#define LINE0       " " LOGO0 "    " ATTR_PROP "      " RESET_PROP " " LINEBLANK "\n"
#define LINE1       " " LOGO1 "    " ATTR_PROP "os    " RESET_PROP " " LINEBLANK "\n"
#define LINE2       " " LOGO2 "    " ATTR_PROP "host  " RESET_PROP " " LINEBLANK "\n"
#define LINE3       " " LOGO3 "    " ATTR_PROP "kernel" RESET_PROP " " LINEBLANK "\n"
#define LINE4       " " LOGO4 "    " ATTR_PROP "uptime" RESET_PROP " " LINEBLANK "\n"
#define LINE5       " " LOGO5 "    " ATTR_PROP "memory" RESET_PROP " " LINEBLANK "\n"
#define LINE6       " " LOGO6 "    " ATTR_PROP "      " RESET_PROP " " LINEBLANK "\n"

#define LINE_WIDTH  (sizeof(LINE0) - 1)
#define LINE_C      (LINE_WIDTH - (sizeof(LINEBLANK "\n") - 1))
#define IDX(LINENR) ((LINENR) * LINE_WIDTH + LINE_C)

int main()
{
    char buf[] = LINE0 LINE1 LINE2 LINE3 LINE4 LINE5 LINE6;
    char* lineptr[] = {&buf[IDX(0)], &buf[IDX(1)], &buf[IDX(2)], &buf[IDX(3)], &buf[IDX(4)], &buf[IDX(5)], &buf[IDX(6)]};

    {
        char login_buf[16];
        char hostname_buf[16];
        const int error_code0 = getlogin_r(login_buf, 16);
        const int error_code1 = gethostname(hostname_buf, 16);
        assert(error_code0 == 0);
        assert(error_code1 == 0);
        const int offset = sizeof(" " LOGO0 "    ") - 1;
        snprintf(&buf[offset], LINE_WIDTH - offset - sizeof('\n'), ATTR_PROP "%s" RESET_PROP "\033[1m" "@" ATTR_PROP "%s" RESET_PROP, login_buf, hostname_buf);
    }

    {
        struct utsname utsname;
        const int error_code = uname(&utsname);
        assert(error_code == 0);
        assert(strlen(utsname.release) <= sizeof(LINEBLANK));
        strcpy(lineptr[3], utsname.release);
    }

    {
        FILE* fp = fopen("/etc/os-release", "r");
        assert(fp);
        int c;
        while ((c = getc(fp)) != EOF && c != '\n')
            continue;
        while ((c = getc(fp)) != EOF && c != '\n')
            continue;
        while ((c = getc(fp)) != EOF && c != '"')
            continue;
        int i = 0;
        while ((c = getc(fp)) != EOF && c != '"') {
            char* ptr = lineptr[1] + i;
            if (ptr >= lineptr[2] - 1) {
                break;
            }
            *ptr = c;
            i++;
        }
        fclose(fp);
    }

    {
        FILE* fp0 = fopen("/sys/devices/virtual/dmi/id/product_name", "r");
        FILE* fp1 = fopen("/sys/devices/virtual/dmi/id/product_family", "r");
        assert(fp0);
        assert(fp1);
        char c;
        int i = 0;
        while ((c = getc(fp0)) != EOF && c != '\n') {
            char* ptr = lineptr[2] + i;
            if (ptr >= lineptr[3] - 1) {
                break;
            }
            *ptr = c;
            i++;
        }
        i++;
        while ((c = getc(fp1)) != EOF && c != '\n') {
            char* ptr = lineptr[2] + i;
            if (ptr >= lineptr[3] - 1) {
                break;
            }
            *ptr = c;
            i++;
        }
        fclose(fp0);
    }

    {
        struct sysinfo s_info;
        const int error_code = sysinfo(&s_info);
        assert(error_code == 0);
        const int min = (s_info.uptime / 60) % 60;
        const int hour = (s_info.uptime / 3600) % 24;
        sprintf(lineptr[4], "%dh %dm", hour, min);
    }

    {
        FILE* fp = fopen("/proc/meminfo", "r");
        assert(fp);
        int c;
        while ((c = getc(fp)) != EOF && !isdigit(c)) {
            continue;
        }
        int mem_total = c - '0';
        while ((c = getc(fp)) && isdigit(c)) {
            mem_total = mem_total * 10 + (c - '0');
        }
        while ((c = getc(fp)) != EOF && !isdigit(c)) {
            continue;
        }
        int mem_free = c - '0';
        while ((c = getc(fp)) && isdigit(c)) {
            mem_free = mem_free * 10 + (c - '0');
        }

        sprintf(lineptr[5], "%dM / %dM", mem_free / 1024, mem_total / 1024);
        fclose(fp);
    }

    fwrite(buf, sizeof(buf), 1, stdout);

    return 0;
}
