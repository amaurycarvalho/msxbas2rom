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

#### 1️⃣ Project style guidelines

We use .clang-format and .clang-tidy in this project to enforce style and do code quality checks.

Also, we use [Doxygen notation](https://en.wikipedia.org/wiki/Doxygen) for inline documentation.

#### 2️⃣ [Inner Source](https://en.wikipedia.org/wiki/Inner_source) Principles in Action

- Transparency: All discussions, reviews, and design decisions happen in the open via GitHub PRs and Issues;
- Autonomy: You can contribute directly without waiting for “permission,” as long as you follow the rules;
- Quality Gates: Automated builds and tests ensure changes are safe;
- Documentation First: Every new feature or change must be documented before merge.

---

## ⚙ Developer Quick Start Guide

### 1️⃣ Fork the MSXBAS2ROM Repository on GitHub

1. Go to the original repo:
   [https://github.com/amaurycarvalho/msxbas2rom](https://github.com/amaurycarvalho/msxbas2rom)
2. Click **Fork** (top-right corner);
3. Choose **your GitHub account** as the destination.

### 2️⃣ Clone Your Fork Repository

```bash
git clone https://github.com/YOUR_GITHUB_ACCOUNT/msxbas2rom.git
cd msxbas2rom
```

Now you have a repository where you have full write access.

### 3️⃣ Create a Branch for Your Work

Follow this branch prefixes:

- **Feature**: These branches are used for developing new features. Use the prefix feature/. For instance, feature/login-system;
- **Bugfix**: These branches are used to fix bugs in the code. Use the prefix bugfix/. For example, bugfix/header-styling;
- **Hotfix**: These branches are made directly from the production branch to fix critical bugs in the production environment. Use the prefix hotfix/. For instance, hotfix/critical-security-issue;
- **Chore/Maintenance**: These branches are used for refactoring purpose. For instance: chore/refactor-authentication-module;
- **Documentation**: These branches are used to write, update, or fix documentation eg. the README.md file. Use the prefix docs/. For instance, docs/api-endpoints;
- **Release**: These branches are used to prepare for a new production release. They allow for last-minute dotting of i’s and crossing t’s. Use the prefix release/. For example, release/v1.0.1.

Examples:

```bash
git checkout -b feature/user-authentication
git checkout -b feature/issue-nn
git checkout -b bugfix/fix-header-styling
git checkout -b bugfix/issue-nn
git checkout -b hotfix/security-patch
git checkout -b hotfix/issue-nn
git checkout -b chore/refactor-authentication-module
git checkout -b docs/update-readme
git checkout -b docs/issue-nn
git checkout -b release/v0.0.0.0
```

Also, you can switch between branches with:

```bash
git switch my-other-branch
git switch master
```

### 4️⃣ Install VSCode and Dependencies

Download and install [Visual Studio Code](https://code.visualstudio.com/).  
Open the project folder in VSCode.

```bash
code .
```

Depending on your platform:

- **Windows**: Install [MSYS2](https://www.msys2.org/) or MinGW for `g++`;
- **Linux/macOS**: Ensure `g++` and `make` are installed;
- Install the C++ extension in VSCode.

### 5️⃣ Compile the Project

In the terminal inside VSCode (or press CTRL+SHIFT+B):

```bash
make all
```

The compiled executable will be placed in the `bin/Release` folder.

### 6️⃣ Test Your Changes

Run in the terminal (or CTRL+SHIFT+P, "Tasks: Run Task" and "test msxbas2rom") to confirm your build is working:

```bash
./bin/Release/msxbas2rom -v
```

Also, run unit testing:

```bash
cd test/unit
make
./test
```

And run integration testing as well:

```bash
cd test/integration
./test.sh
```

Create new test scenarios if need.

### 7️⃣ Submit Your Pull Request

```bash
git add .
git commit -am "change: Description of my changes"
git push msxbas2rom your-branch-name
```

Follow [Conventional](https://www.conventionalcommits.org/) commits:

- feat: for new features;
- fix: for bug fixes;
- docs: for documentation changes;
- test: for adding/modifying tests;
- refactor: for code improvements without feature changes.

Example:

`feat: add option to compile without BASIC interpreter`

Then go to your GitHub fork repository page and open a **Pull Request** to the `master` branch of the original project.

---

## 💬 Need Help?

If you run into any issues while contributing, feel free to:

- Open a [Discussion](https://github.com/amaurycarvalho/msxbas2rom/discussions);
- Contact me through GitHub Issues.

---

Let’s make **MSXBAS2ROM** better, together! 🚀
