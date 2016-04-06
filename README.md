# Packaging the library for Arduino boards manager

1. From master branch, compress the avr folder with:

```
tar -cvjSf altair-0.2.3.tar.bz2 altair-arduinoide/avr/
```

2. Get the Sha sum and write it on package_makerlabmx_index.json

```
shasum -a 256 altair-0.2.3.tar.bz2
```

3. Get the size in bytes and put in package_makerlabmx_index.json

```
ls -l
```

# Package manager url:

http://makerlabmx.github.io/altair-arduinoide/package_makerlabmx_index.json