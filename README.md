# IRPAM - PAM Authentication module for Linux using IR camera

![](project_icon.webp)

IRPAM project aims to be an alternative to the Howdy project. It provides support for PAM-based authentication using the IR camera on board. Howdy pulls in various dependencies and has a script-based install process. As an alternative, IRPAM aims to be a completely packaged application, which should be faster to install and use than Howdy. We use various open-source models for face detection and embeddings generation. 

IRPAM is written in C++. We are not experts in C++, nor its various build systems - contributions are welcome and highly appreciated. IRPAM is still very much under construction, and is not built as a complete PAM module yet. However, you can contribute by cloning this repo and running:

```bash
$ git clone https://github.com/sandeshbhusal/irpam_cpp
$ cd irpam_cpp
$ cmake -S . -B build -G "Ninja" -DCMAKE_C_COMPILER=$(which clang) -DCMAKE_CXX_COMPILER=$(which clang++)
$ ninja -C build/
```

There are a couple of dependencies of this project which can be installed using corresponding commands on Fedora Linux. 
Installation instructions may differ depending on your platform.

- v4l libraries `sudo dnf install libv4l-devel`
- pam-devel libraries `sudo dnf install pam-devel`

## Using IRPam

After you've built IRPam, the built `.so` artifact is in `build/src/lib/libirpam.so`.

1. Copy the library to Pam Auth directory:

```bash
sudo cp src/lib/libirpam.so /usr/lib64/security/pam_libirpam_auth.so
```

2. Configure a `test` PAM configuration. All configurations for pam reside in `/etc/pam.d`.

```bash
echo "auth        required      pam_libirpam_auth.so" | sudo tee -a /etc/pam.d/irpam
```

3. Move the models from `src/recognition/models` to `/opt/irpam/models`

```bash
sudo mkdir -p /opt/irpam/models
sudo cp src/recognition/models/* /opt/irpam/models
```

4. Capture a bunch of images of your face and put them in the `/opt/irpam/data/` directory. This will be automated in the coming commits.

5. Test the configuration with `pamtester`. Installation directions for `pamtester` depend on your distro and are readily available online.

```bash
pamtester irpam <your-username> authenticate
```

Et voila!
