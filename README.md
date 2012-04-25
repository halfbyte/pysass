# Python libsass bindings

Please note that this is very much a work in progress. I have never created a python module, let alone a c extension before and I will gladly accept any help you can give me in making it better.

It is also far from being feature complete. I am currently pondering the correct API for this in python land, most
probably by trying to use it as a compiler in webassets or something like that.

As you can see right now, it currently has no tests, because it is nothing more than a proof of concept written down in less than two hours. This will change.

## Installation

The usual setup.py wrangling should do it:

```bash
$ python setup.py build
$ python setup.py install
```

(It currently generates a heap of warnings and I would be happy to know why this happens (it doesn't happen when compiling libsass with the makefile))

