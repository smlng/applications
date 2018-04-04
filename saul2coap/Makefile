# name of your application
APPLICATION = saul2coap
# default riotbase
RIOTBASE ?= ../../RIOT
# If no BOARD is found in the environment, use this default:
BOARD ?= native
# some custom config
CFLAGS += -DGCOAP_REQ_WAITING_MAX=16
CFLAGS += -DGCOAP_STACK_SIZE=2048

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the minimum networking modules for IPv6
USEMODULE += gnrc_ipv6_default
# Additional networking modules that can be dropped if not needed
USEMODULE += gnrc_icmpv6_echo
# Add coap support
USEMODULE += gcoap
# Modules to include:
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
# include and auto-initialize all available sensors
USEMODULE += saul_default
USEMODULE += fmt

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
