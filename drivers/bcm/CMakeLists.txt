zephyr_library()

zephyr_library_sources_ifdef(CONFIG_BCM_SYS bcm_sys.c)
