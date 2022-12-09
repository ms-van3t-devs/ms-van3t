# 3GPP NR ns-3 module #

This is an [ns-3](https://www.nsnam.org "ns-3 Website") 3GPP NR module for the
simulation of NR non-standalone cellular networks. Ns-3 is used as a base,
on top of which we will add our module as plug-in (with limitations that will
be discussed below).

## Installation for an authorized developer

We try to keep in sync with the latest advancements in ns-3-dev. By the version
1.0, we have upstreamed all our patches to ns-3-dev, making our module
independent from the ns-3 version used.

### Brand new installation of ns-3-dev repository

To download a working copy of the ns-3-dev repository with the latest changes,
you can do the following:

```
$ git clone https://gitlab.com/nsnam/ns-3-dev.git
$ cd ns-3-dev
```

Provide your username and password when asked.

### Switching from CTTC-provided ns-3-dev

Before v1.0, the NR module needed a custom ns-3-dev version. For those of you
that are upgrading from v0.4 to v1.0, the steps to switch to the official
ns-3 repository are the following (without recreating the repo configuration):

```
$ git remote add nsnam https://gitlab.com/nsnam/ns-3-dev.git
$ git checkout master
$ git pull nsnam master
```

Anyway, we will make sure that the master of our custom ns-3-dev will stay
up-to-date with respect to the official ns-3-dev.

### Using an existing installation of ns-3

In case you are already using the git mirror of ns-3-dev, hosted at GitHub or
GitLab, you are already ready to go (please make sure to be up-to-date with
`git pull` in the master branch!).

### Test the installation
To test the installation, after following one of the previous point, you can do
a simple configuration and compile test (more options for that later):

```
$ ./waf configure --enable-examples --enable-tests
$ ./waf
```

A success for both previous commands indicates an overall success.

### Brand new installation of the NR module

As a precondition to the following steps, you must have a working local git
repository of ns-3-dev. If that is the case, then, your local git repo is ready
to include our nr module (only for authorized users):

```
$ cd contrib
$ git clone https://gitlab.com/cttc-lena/nr.git
$ cd ..
```

Please note that the contrib/nr directory will be listed as "Untracked files" every
time you do a `git status` command. Ignore it, as the directory lives as an
independent module. As a result, we have now two parallel repository, but one
lives inside the other.

Finally, switch to the latest release branch (fixes are included in the release
branch and not master). For example, for version 1.1 you have to `git checkout`
to the `5g-lena-v1.1.y` branch.


### Test the NR installation

Let's configure the project:

```
$ ./waf configure --enable-examples --enable-tests
```

If the NR module is recognized correctly, you should see "nr" in the list of
built modules. If that is not the case, then most probably the previous
point failed. Otherwise, you could compile it:

```
$ ./waf
```

If that command returns successfully, Welcome to the NR world !

Notice that sqlite development package and semaphore.h are required (otherwise
you will get an error, e.g: `fatal error: ns3/sqlite-output.h`). In this case
you should install libc6-dev:

```
sudo apt-get install libc6-dev
```

that will provide semaphore.h and/or sqlite:

```
apt-get install sqlite sqlite3 libsqlite3-dev
```

For more details, related to the prerequisites for ns-3 please visit: `https://www.nsnam.org/wiki/Installation#Ubuntu.2FDebian.2FMint`.
After the installation of the missing packages run again `./waf configure --enable-tests --enable-examples`.
You should see: `SQLite stats support: enabled`


## Upgrading 5G-LENA

We assume that your work lives in a separate branch, and that the 'master'
branch of the NR repository is left untouched as the first time you downloaded
it. If it is not the case, then please move all your work in a separate branch.

A vanilla 'master' branch can be updated by simply running:

```
$ cd ns-3-dev/contrib/nr    # or src/nr if the module lives under src/
$ git checkout master
$ git pull
```

At each release, we will incorporate into the master branch all the work that
is meant to be released.

## Documentation

We maintain two sources of documentation: a user manual, and the Doxygen API
documentation. The user manual describes the models and their assumptions; as
we developed the module while the standard was not fully available, some parts
are not modeling precisely the bits and the procedures indicated by the
standard. However, we tried to abstract them accurately. In the Doxygen API
documentation, you will find details about design and user usage of any class
of the module, as well as description and images for the examples and the
tests.

To build the user manual, please do:

```
$ cd doc
$ make latexpdf
```

And you fill find the PDF user manual in the directory build/latex. Please note
that you may have to install some requirements to build the documentation; you
can find the list of packages for any Ubuntu-based distribution in the file
`.gitlab-ci.yml`.

To build the doxygen documentation, please do:

```
$ python3 doc/m.css/doxygen/dox2html5.py doc/doxygen-mcss.conf --debug
```

And then you will find the doxygen documentation inside `doc/doc/html/`.
Please note that you may need to initialize the m.css submodule, and
to install some packages like python3.

## Building NR V2X code

The implementation of NR V2X is divided between ns-3 LTE (RLC and above) and
5G-LENA NR (MAC and PHY) modules, and it is contained in separate branches.
Therefore, to be able to use this code one has to use CTTC customized LTE module
of ns-3, and a specific branch in the nr module. Following are the steps to
switch to these dedicated branches. Note, before following these steps please
make sure that you have been granted access to the CTTC NR module.

### Adding V2X branches to your local nr and ns-3-dev repositories

1. Switch to the V2X branch in your local nr repository

```
$ cd contrib/nr
```
Once you are inside the nr module directory, switch to the latest V2X code
release branch. For example, for version 0.1, you have to execute one of the
following commands depending on whether you have just cloned the nr repository
for the first time or you are already working with it for some time.

**If you have just cloned the nr repository for the first time:**

```
$ git checkout 5g-lena-v2x-v0.1.y
```
**If you are already working with the nr module:**

```
$ git fetch origin 5g-lena-v2x-v0.1.y
$ git checkout 5g-lena-v2x-v0.1.y
```

2. Now, lets get back to the ns-3-dev directory to fetch the CTTC customized ns-3
   LTE module for V2X.

```
$ cd ../..
```

3. Add a new remote to the CTTC fork of ns-3-dev

*Note: Make sure to position yourself in ns-3-dev directory*

```
$ git remote add cttc-ns3-dev-fork https://gitlab.com/cttc-lena/ns-3-dev.git
```

4. Fetch the V2X branch from the CTTC fork of ns-3-dev

```
$ git fetch cttc-ns3-dev-fork v2x-lte-dev
```

5. Switch to the V2X branch in your local ns-3-dev repository

```
$ git checkout v2x-lte-dev
```

6. Building V2X code

```
$ ./waf configure --disable-python --enable-tests --enable-examples
$ ./waf build
```

## Features

To see the features, please go to the [official webpage](https://cttc-lena.gitlab.io/5g-lena-website/features/).

## Papers

An updated list of published papers that are based on the outcome of this
module is available
[here](https://cttc-lena.gitlab.io/5g-lena-website/papers/).

## Future work

## About

The Mobile Networks group in CTTC is a group of 10 highly skilled researchers, with expertise in the area of mobile and computer networks, ML/AI based network management, SDN/NFV, energy management, performance evaluation. Our work on performance evaluation started with the design and development of the LTE module of ns-3.

We are [on the web](https://cttc-lena.gitlab.io/5g-lena-website/about/).

## Authors ##

In alphabetical order:

- Zoraze Ali
- Biljana Bojovic
- Lorenza Giupponi
- Katerina Koutlia
- Sandra Lagen
- Natale Patriciello

Inspired by [mmWave module by NYU/UniPD] (https://github.com/nyuwireless-unipd/ns3-mmwave)

## License ##

This software is licensed under the terms of the GNU GPLv2, as like as ns-3.
See the LICENSE file for more details.
