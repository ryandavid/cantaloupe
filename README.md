# Cantaloupe
![Cantaloupe](/images/cantaloupe.png) <br/>
Cantaloupe is a CAN bus tool targeting MacOS and the CANtact/CANable USB interface.

### Design
The core library is designed to interact with the hardware via LibUSB. Apps are designed to use the core lib to
provide high-level functionality such as bus introspection or logging.

### Prerequistes

```bash
brew install libusb
```

### Contributing
Pull requests are welcome. Please try to match the overall style.

### License
[GNU GPLv3](https://choosealicense.com/licenses/gpl-3.0/)
