### Install

Add the repository and key manually (recommended - review the commands before running):
```bash
curl -fsSL https://chanuka-williams.github.io/openskydimo-apt/openskydimo-archive-keyring.gpg.asc | \
sudo tee /usr/share/keyrings/openskydimo-archive-keyring.asc > /dev/null

echo "deb [signed-by=/usr/share/keyrings/openskydimo-archive-keyring.asc] \
https://chanuka-williams.github.io/openskydimo-apt stable main" | \
sudo tee /etc/apt/sources.list.d/openskydimo.list

sudo apt update
sudo apt install openskydimo
```