############################################################################
#   Copyright (c) 2023 PX4 Development Team. All rights reserved.
#
# Terms and license agreement that you must read through before.
# You must copy this big 'ol thing into your file as well as a right-of
# passage and not just for legal reasons :)
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
