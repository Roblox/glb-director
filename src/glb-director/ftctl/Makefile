all: glb-ftctl

FTCTL_SRCS = main.c
WERROR_FLAGS += -Werror

CFLAGS += -O3 -g -Wall $(WERROR_FLAGS)

APP = glb-ftctl

glb-ftctl: $(FTCTL_SRCS)
	gcc \
		$(FTCTL_SRCS) \
		$(CFLAGS) \
		-I`pwd`/..\
		-o glb-ftctl


clean:
	rm -Rf glb-ftctl
