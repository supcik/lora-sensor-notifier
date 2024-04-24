idf_component_register(SRCS "notifier.cpp"
    REQUIRES "arduino" "arduino-lmic" "lorawan" "util" "cayenne-lpp"
    INCLUDE_DIRS "include")
