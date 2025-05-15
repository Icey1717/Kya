# Kya: Dark Lineage Decompilation

This repository hosts an ongoing decompilation of *Kya: Dark Lineage*, a 2003 action-adventure platformer developed by Eden Games and published by Atari for the PlayStation 2.  
The goal is to reverse-engineer the game's original C++ source code from its compiled binaries, facilitating a deeper understanding of its mechanics, enabling modding, and preserving its legacy.

---

## 🧠 Project Overview

The decompilation effort aims to reconstruct the game's source code, providing insights into its architecture and design.  
This process involves analyzing the compiled binaries to recreate human-readable and compilable source code.

---

## 📁 Repository Structure

- `src/` – Decompiled source files  
- `port/` – Code related to rendering and running the game on PC

---

## 🛠️ Related Tools & Projects

This decompilation is part of a broader ecosystem of tools and libraries aimed at understanding and modifying *Kya: Dark Lineage*:

- [KyaBank](https://github.com/Icey1717/KyaBank): Command-line tool for listing and extracting files from `.BNK` archives used in the game  
- [edBank](https://github.com/Icey1717/edBank): Decompilation of the `edBank` library, handling bank file operations  
- [edFile](https://github.com/Icey1717/edFile): Decompilation of the `edFile` library, managing file I/O operations

For additional resources, for additional technical documentation, please visit [The Kya: Dark Lineage Files](https://kyadlfiles.github.io/).

---

## 🧩 Getting Started

**This is a work in progress, currently needs fixing as it may not run succesfully once built. It also currently only compiles for Windows.**

**Requirements**:  
- Visual Studio 2022  
- Clang builder and tools for VS2022 (from the Visual Studio Installer) 
- Vulkan SDK (including GLM Headers)

To begin working with the decompiled code:

1. **Clone the repository, including submodules:**

   ```bash
   git clone --recursive https://github.com/Icey1717/Kya.git
   ```

2. **Open the Repository in Visual Studio**

3. **Build the Project:**

   Select *x64 Debug* as the configuration, and *Kya_Debug.exe* as the target.

4. **Run and Test**

---

## 🤝 Contributing

Contributions are welcome! To participate:

- **Fork the Repository:** Create your own copy to work on  
- **Create a Branch:** Develop your feature or fix in a separate branch  
- **Submit a Pull Request:** Once ready, submit a PR for review

Please include comprehensive documentation for any new features or changes.

---

## 📜 License

This project is licensed under the GPL 3.0 license.

---

*This project is dedicated to preserving and understanding the intricacies of Kya: Dark Lineage.  
Your contributions and insights are invaluable to this endeavor.*
