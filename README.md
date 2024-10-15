# R-ZWEI Kickers Codebase 2024

This is where the code of the team **R-ZWEI Kickers** lives. Its documentation can be found in the [Wiki of this repository](https://github.com/AK-Smart-Machines-HS-KL/R2K-SPL/wiki)

### Git

If you need a refreshment on your Git know-how, we recommend freshening it up with a tutorial, e.g. [this one in German](https://lerneprogrammieren.de/git/) or [this English one](https://www.w3schools.com/git/).

### B-Human Code Release

This code is based on the official 2021 B-Human code release. Documentation for it can be found in their [public wiki](https://wiki.b-human.de/coderelease2021/).

Previous code releases are tagged with "coderelease&lt;year&gt;", where &lt;year&gt; is the year in which the code was released (starting with 2013).

Please note that **before** you clone this repository on Windows, execute:

```
git config --global core.autocrlf input
git config --global core.eol lf
```

As this repository uses submodules, it must be cloned using `git clone --recursive`. Downloading the release as `zip` or `tar.gz` does not work.
