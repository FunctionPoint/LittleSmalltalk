# Bootstrap the initial image: run "lst -v -o IMAGE SOURCE" (SOURCE: the source folder);
# lst writes the image and exits.
# Called by the baseimage target: cmake -DLST=... -DSOURCE=... -DIMAGE=... -P baseimage.cmake

execute_process(
	COMMAND "${LST}" -v -o "${IMAGE}" "${SOURCE}"
	RESULT_VARIABLE result
)

if(NOT result EQUAL 0)
	message(FATAL_ERROR "Bootstrapping ${IMAGE} from ${SOURCE} failed: ${result}")
endif()
