# ═══════════════════════════════════════════════════════════════
#  Makefile - SysWatch Pro v2.0  (MinGW-w64 / GCC)
# ═══════════════════════════════════════════════════════════════

CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c11
LDFLAGS = -lpsapi -lkernel32 -liphlpapi -lws2_32

TARGET  = SysWatchPro.exe

SRCS    = main.c \
          core/process.c \
          core/cpu.c \
          core/memory.c \
          core/network.c \
          core/anomaly.c \
          core/logger.c \
          gui/widgets.c \
          gui/hotkeys.c \
          web/httpserver.c

all: $(TARGET)
	@echo.
	@echo  >>> Build OK : $(TARGET)
	@echo  >>> Lancer   : $(TARGET)
	@echo  >>> Dashboard: http://localhost:8080
	@echo  >>> Dechiffrer logs: $(TARGET) --decrypt syswatch_forensic.log

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LDFLAGS)

clean:
	del /Q $(TARGET) 2>nul || true

.PHONY: all clean
