<div align="center">
  <image src="./docs/images/Black Theme.png"/>

  
</div>

# Usage example
  <p>CLI note manager with vim/neovim editor.</p>
<image src="https://github.com/TwelveFacedJanus/BSDBook/blob/main/docs/images/helpscreen.png">

## Build cli-application from source
Okay, bsdbooks written on C language that already have all OS, but for tui client u need to use some dependencies such as `ncurses`, `vim or neovim` and `make` and.
```sh
$ git clone https://github.com/TwelveFacedJanus/BSDBook.git
$ cd BSDBook
$ make && make install
```

or if u will use only server implementation, u need to build `server.c`
```sh
$ git clone https://github.com/TwelveFacedJanus/BSDBook.git
$ cd BSDBook
$ gcc src/server.c -o server -ljansson
```

## Build web ui at your server:
```sh
$ docker placeholder
```

## Tested on
- OS: FreeBSD 14.2-RELEASE-p1
- compiler: gcc13


## Contributors
CONTRIBUORS.md
