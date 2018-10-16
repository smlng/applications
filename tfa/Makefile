# name of your application
APPLICATION = fs1000a

# If no BOARD is found in the environment, use this default:
BOARD ?= bluepill

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../RIOT

FEATURES_REQUIRED += periph_gpio_irq

# some custom config
CFLAGS += -DGCOAP_REQ_WAITING_MAX=6
CFLAGS += -DGCOAP_STACK_SIZE=1536
CFLAGS += -DGCOAP_NON_TIMEOUT=500000
CFLAGS += -DGNRC_PKTBUF_SIZE=2048 -DGNRC_NETIF_IPV6_ADDRS_NUMOF=2 \
          -DGNRC_NETIF_IPV6_GROUPS_NUMOF=2 -DGNRC_IPV6_NIB_NUMOF=1 \
          -DGNRC_IPV6_NIB_OFFL_NUMOF=1
#CFLAGS += -DS2C_PAN=0x2424
#CFLAGS += -DS2C_CHN=18
#CFLAGS += -DS2C_COAP_SRV_PORT=5685

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules
USEMODULE += gnrc_ipv6_default
USEMODULE += gcoap
# Additional networking modules that can be dropped if not needed
#USEMODULE += gnrc_icmpv6_echo
USEMODULE += fmt
USEMODULE += xtimer

GNRC_NETIF_NUMOF := 1
USEMODULE += ethos
CFLAGS += '-DETHOS_UART=UART_DEV(1)' -DETHOS_BAUDRATE=115200 -DUSE_ETHOS_FOR_STDIO

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 0

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
