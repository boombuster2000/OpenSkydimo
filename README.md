## Ubuntu
### Install
```bash
curl -fsSL https://chanuka-williams.github.io/openskydimo-apt/add-repo.sh | sudo bash
sudo apt install openskydimo
```
Note: This adds my GPG key to your system, scoped specifically to this repository, so
you can install and update OpenSkydimo (and any future packages I publish here) via
`sudo apt update` / `sudo apt upgrade`.

Tested on: Kubuntu 26.04 LTS.
Should also work on other Debian-based distros with equally recent or newer library versions.
Not guaranteed to work on older releases (e.g. Ubuntu 20.04, Debian 11) due to glibc/libstdc++ version requirements,
will have to compile yourself.