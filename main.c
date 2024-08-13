#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>

#include <linux/kernel.h>
#include <linux/unistd.h>

#define LOGO0       "._.     "
#define LOGO1       "| |     "
#define LOGO2       "| |     "
#define LOGO3       "| |     "
#define LOGO4       "| |     "
#define LOGO5       "| |____."
#define LOGO6       "|______|"

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

    {
        char* login_buf = getenv("USER"); // unreliable, but yolo :3

        char hostname_buf[16];
        const int error_code = gethostname(hostname_buf, 16);
        assert(error_code == 0);

        const int offset = sizeof(" " LOGO0 "    ") - 1;
        const char str[] = ATTR_PROP "%s" RESET_PROP "\033[1m"
                                     "@" ATTR_PROP "%s" RESET_PROP;

        snprintf(&buf[offset], LINE_WIDTH - offset - sizeof('\n'), str, login_buf, hostname_buf);
    }

    {
        struct utsname utsname;
        const int error_code = uname(&utsname);

        assert(error_code == 0);
        assert(strlen(utsname.release) <= sizeof(LINEBLANK));

        strcpy(&buf[IDX(3)], utsname.release);
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
            char* ptr = &buf[IDX(1)] + i;
            assert(ptr < &buf[IDX(2)] - 1);
            *ptr = c;
            i++;
        }

        fclose(fp);
    }

    {
        FILE* fp = fopen("/sys/devices/virtual/dmi/id/product_family", "r");

        assert(fp);

        int c;
        int i = 0;
        while ((c = getc(fp)) != EOF && c != '\n') {
            char* ptr = &buf[IDX(2)] + i;
            assert(ptr < &buf[IDX(2)] - 1);
            *ptr = c;
            i++;
        }

        fclose(fp);
    }

    {
        FILE* fp = fopen("/proc/meminfo", "r");

        assert(fp);

        // some credit to:
        // https://github.com/makichiis/puppyfetch/blob/main/puppyfetch.c

        char param_name[64];
        long long value;
        long long mem_cached = 0;
        long long mem_sreclaimable = 0;

        while (true) {
            int res = fscanf(fp, "%s %lld", param_name, &value);
            if (res == EOF || (mem_cached != 0 && mem_sreclaimable != 0)) {
                break;
            }
            if (strcmp(param_name, "Cached:") == 0) {
                mem_cached = value;
            }
            else if (strcmp(param_name, "SReclaimable:") == 0) {
                mem_sreclaimable = value;
            }
        }

        struct sysinfo s_info;
        const int error_code = sysinfo(&s_info);

        assert(error_code == 0);

        const int mem_free =
            ((((uint64_t)s_info.totalram + (uint64_t)s_info.sharedram - s_info.freeram - s_info.bufferram) * s_info.mem_unit) / 1024) /
                1024 -
            mem_cached / 1024 - mem_sreclaimable / 1024;
        const int mem_total = (uint64_t)s_info.totalram * s_info.mem_unit / (1024 * 1024);

        sprintf(&buf[IDX(5)], "%dM / %dM", mem_free, mem_total);

        const int min = (s_info.uptime / 60) % 60;
        const int hour = (s_info.uptime / 3600) % 24;

        sprintf(&buf[IDX(4)], "%dh %dm", hour, min);
    }

    syscall(__NR_write, STDOUT_FILENO, buf, sizeof(buf) - 1);

    return 0;
}
