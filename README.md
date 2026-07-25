## To Start

Run `openskydimo help` to start.

## Ubuntu

### Install

Add the repository and key manually.
```bash
curl -fsSL https://chanuka-williams.github.io/openskydimo-apt/openskydimo-archive-keyring.gpg.asc | \
sudo tee /usr/share/keyrings/openskydimo-archive-keyring.asc > /dev/null

echo "deb [signed-by=/usr/share/keyrings/openskydimo-archive-keyring.asc] \
https://chanuka-williams.github.io/openskydimo-apt stable main" | \
sudo tee /etc/apt/sources.list.d/openskydimo.list

sudo apt update
sudo apt install openskydimo
```

## Arch Linux

### Install
```bash
yay -S openskydimo   # or your preferred AUR helper
```

After installing, enable and start the service:
```bash
systemctl --user enable --now openskydimo
```
