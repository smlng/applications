# name of your application
APPLICATION = ping_probe

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../RIOT

# Include networking modules
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
USEMODULE += gnrc_ipv6_default
USEMODULE += gnrc_pktdump
USEMODULE += gnrc_icmpv6_echo

# Change this to 1 to enable code in RIOT that does safety checking
DEVELHELP ?= 0

CFLAGS += -DGNRC_IPV6_NIB_NUMOF=1
CFLAGS += -DGNRC_IPV6_NIB_OFFL_NUMOF=1
# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# use the enc28j60 ethernet with the STM32 bluepill
ifneq (,$(filter bluepill,$(BOARD)))
  USEMODULE += enc28j60
  INCLUDES += -I$(CURDIR)
endif

include $(RIOTBASE)/Makefile.include
