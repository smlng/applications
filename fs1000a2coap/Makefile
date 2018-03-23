# Default Makefile, for host native GNRC-based networking

# name of your application
APPLICATION = saul2coap

# If no BOARD is found in the environment, use this default:
BOARD ?= native

## Uncomment to redefine port, for example use 61616 for RFC 6282 UDP compression.
#GCOAP_PORT = 5683
#CFLAGS += -DGCOAP_PORT=$(GCOAP_PORT)

## Uncomment to redefine request token length, max 8.
#GCOAP_TOKENLEN = 2
#CFLAGS += -DGCOAP_TOKENLEN=$(GCOAP_TOKENLEN)
CFLAGS += -DGCOAP_REQ_WAITING_MAX=8
CFLAGS += -DGCOAP_NON_TIMEOUT=15000000
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
USEMODULE += gnrc_icmpv6_echo
# sensor stuff
USEMODULE += fmt
USEMODULE += fs1000a

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
