# LoRa Sensor Notifier

This project is a simple example of how to use a LoRa module to send sensor data to a LoRa gateway.

## Defining secrets

The project uses a secrets file to store the LoRaWAN keys. The file should be named `secrets.csv` and should be placed in the root of the project. The file should look like this:

```csv
key,type,encoding,value
storage,namespace,,
appeui,data,hex2bin,0000000000000000
deveui,data,hex2bin,70B3D5xxxxxxxxxx
appkey,data,hex2bin,xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
```

For the next parts, you need to have the `just` CLI installed. More info can be found [here](https://just.systems/).

```bash

You can then build the binary file of the secrets by running the following command:

```bash
just build_secret_partition
```

You can then flash the binary file to the ESP32 by running the following command:

```bash
just flash_secret_partition
```