# Contributing to MSXBAS2ROM

We welcome contributions from **everyone** — whether you’re a developer, a documentation enthusiast, or simply a passionate MSX user who wants to help the project grow.

This guide explains how you can contribute and what rules apply for code submissions.

---

## 📌 How You Can Contribute

### 🧑‍💻 For Users

You don’t need to be a developer to help improve **MSXBAS2ROM**! Here are some valuable ways you can contribute:

- **Report bugs or request features** by creating an [Issue](https://github.com/amaurycarvalho/msxbas2rom/issues);
- **Improve the documentation** by editing or adding pages in the [Wiki](https://github.com/amaurycarvalho/msxbas2rom/wiki);
- **[Make a donation](https://github.com/amaurycarvalho/msxbas2rom/wiki#%EF%B8%8F-support-the-project)** to support ongoing development and maintenance.

---

### 👨‍🔧 For Developers

If you want to contribute code, please follow the **Inner Source** principles:

- All changes must be submitted via **Pull Request (PR)**;
- PRs should be linked to an existing [issue](https://github.com/amaurycarvalho/msxbas2rom/issues) (or include a clear explanation of the change);
- Code should follow the project’s style guidelines and be tested before submission;
- PRs will be reviewed before merging into the `master` branch.

---

## ⚙ Developer Quick Start Guide

### 1️⃣ Clone the Repository

```bash
git clone https://github.com/amaurycarvalho/msxbas2rom.git
cd msxbas2rom
```

### 2️⃣ Create a Branch for Your Work

```bash
git checkout -b issue-nn
```

or

```bash
git checkout -b feature/my-feature-name
```

### 3️⃣ Install VSCode

Download and install [Visual Studio Code](https://code.visualstudio.com/).  
Open the project folder in VSCode.

```bash
code .
```

### 4️⃣ Install Dependencies

Depending on your platform:

- **Windows**: Install [MSYS2](https://www.msys2.org/) or MinGW for `g++`;
- **Linux/macOS**: Ensure `g++` and `make` are installed;
- Install the C++ extension in VSCode.

We use clang-format and clang-tidy in this project to enforce style and do security checks.

Also, we use [Doxygen notation](https://en.wikipedia.org/wiki/Doxygen) for inline documentation.

### 5️⃣ Compile the Project

In the terminal inside VSCode (or press CTRL+SHIFT+B):

```bash
make all
```

The compiled executable will be placed in the `bin/Release` folder.

### 6️⃣ Test Your Changes

Run in the terminal (or CTRL+SHIFT+P, "Tasks: Run Task" and "test msxbas2rom") to confirm your build is working:

```bash
./bin/Release/msxbas2rom --ver
```

Also, run unit testing:

```bash
cd test/unit
make
./test
```

Create new test scenarios if need.

### 7️⃣ Submit Your Pull Request

```bash
git add .
git commit -am "change: Description of my changes"
git push origin your-branch-name
```

Follow [Conventional](https://www.conventionalcommits.org/) commits:

- feat: for new features;
- fix: for bug fixes;
- docs: for documentation changes;
- test: for adding/modifying tests;
- refactor: for code improvements without feature changes.

Example:

`feat: add option to compile without BASIC interpreter`

Then go to your GitHub repository page and open a **Pull Request** to the `master` branch of the original project.

---

## [Inner Source](https://en.wikipedia.org/wiki/Inner_source) Principles in Action

- Transparency: All discussions, reviews, and design decisions happen in the open via GitHub PRs and Issues;
- Autonomy: You can contribute directly without waiting for “permission,” as long as you follow the rules;
- Quality Gates: Automated builds and tests ensure changes are safe;
- Documentation First: Every new feature or change must be documented before merge.

---

## 💬 Need Help?

If you run into any issues while contributing, feel free to:

- Open a [Discussion](https://github.com/amaurycarvalho/msxbas2rom/discussions);
- Contact me through GitHub Issues.

---

Let’s make **MSXBAS2ROM** better, together! 🚀
