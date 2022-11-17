# -----------------------------------------------------------------------------
# Copyright (c) 2018, Andes Technology Corporation
# All rights reserved.
# -----------------------------------------------------------------------------
ifneq ($(MAKECMDGOALS),clean)

PLAT_LIST := AE100 AE210P AE3XX AE250 AE350

$(if $(filter ${PLAT},${PLAT_LIST}),, \
	$(error PLAT ${PLAT} invalid or undefined, should be one of [ ${PLAT_LIST} ]))

ifneq ($(filter $(PLAT), AE350 AE250), $(PLAT))
-include ./Makefile.v3
else
-include ./Makefile.v5
endif

else

clean:
	@rm -rf build
	@rm -rf install
	
endif

