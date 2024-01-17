############################################################################
#   Copyright (c) 2023 PX4 Development Team. All rights reserved.
#
# Terms and license agreement copied here
#
# NOTE: flying can be dangerous, please code responsibly.
#
############################################################################

px4_add_module(
	MODULE modules__super_awesome_module
	MAIN super_awesome_module
	SRCS
		SuperAwesomeModule.cpp
		SuperAwesomeModule.hpp
	DEPENDS
		px4_work_queue
)
