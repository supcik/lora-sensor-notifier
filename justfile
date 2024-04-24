build_secret_partition:
    #!/bin/bash
    python ${IDF_PATH}/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py generate secrets.csv data.bin 0x6000

flash_secret_partition:
    #!/bin/bash
    parttool.py write_partition -n nvs --input data.bin
