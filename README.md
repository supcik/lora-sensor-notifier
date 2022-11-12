# LoRa Sensor Notifier

## Activate esp-idf

```bash
. ~/esp/esp-idf/export.sh
```

## Generate nvs partition

```bash
python ${IDF_PATH}/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py generate secrets.csv data.bin 0x6000
```

## Write nvs partition

```bash
parttool.py write_partition -n nvs --input data.bin
```