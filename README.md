# OJSandbox

[![Build Status](https://travis-ci.org/taoky/OJSandbox.svg?branch=version-1)](https://travis-ci.org/taoky/OJSandbox) [![Commits since root](https://img.shields.io/github/commits-since/taoky/OJSandbox/root.svg)](https://github.com/taoky/OJSandbox/commit/)

An experimental sandbox for online judger.

---

### Setup

First run this to generate a configuration file and a script to install dependency

```
make config
```

(or `python3 setup.py` for the same)

Then, install dependencies using the generated `install.sh` script. Note: `openjdk-8-jdk` will be included if you selected to enable support for Java, which may result in some size in downloading.

```
./install.sh
```

Next, compile the backend executable

```
make
```

Now you can run the program and see the effect (may require `sudo` in the middle of execution - please grant it)

```
sudo python3 main.py
```

To cleanup the temporary files and directories created by this OJ sandbox, run the main entry with `cleanup`:

```
sudo python3 main.py cleanup
```

**Note**: As `sudo` is used when running the program, you may need it when cleaning up the qorking directory, i.e.

```
sudo make clean
```
