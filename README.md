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

