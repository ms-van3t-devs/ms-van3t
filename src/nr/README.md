# ns-3 NR module with V2X extensions

This is an [ns-3](https://www.nsnam.org "ns-3 Website") NR module for the simulation of NR V2X. 
ns-3 is used as a base, on top of which we add NR module with V2X extensions as plug-in.

## ns-3 + NR prerequisites

### ns-3 prerequisites:

Make sure to install all [ns-3 preresquisites](https://www.nsnam.org/wiki/Installation#Prerequisites).

### NR prerequisites:

Install libc6-dev (it provides `semaphore.h` header file):

```
sudo apt-get install libc6-dev
```

Install sqlite:

```
apt-get install sqlite sqlite3 libsqlite3-dev
```

Notice that ns-3 and nr prerequisites are required (otherwise you will get an error, e.g: `fatal error: ns3/sqlite-output.h`).

## ns-3 + nr installation

The implementation of NR V2X is divided between ns-3 LTE (RLC and above) and
5G-LENA NR (MAC and PHY) modules, and it is contained in separate branches.
Therefore, to be able to use this code one has to use CTTC customized LTE module
of ns-3, and a specific branch in the nr module.

###  1. Download ns-3 with extensions for V2X:

```
git clone https://gitlab.com/cttc-lena/ns-3-dev.git
cd ns-3-dev
```

### 2. Download the NR module:

```
cd contrib
git clone https://gitlab.com/cttc-lena/nr.git
```

Notice that since these are two independent git repositories, when you run 
`git status` inside of the ns-3, you will notice that the contrib/nr
directory will be listed as "Untracked files". This is normal.

### 3. Switch to the latest NR release branch with V2X extensions:

Checkout the latest NR release branch (usually the branch with the highest version 
number, to list git V2X release branches run `git branch -r --list *v2x-v*`).
For example, if `5g-lena-v2x-v0.2.y` is the latest release branch you can check it out 
in the following way:

```
cd nr
git checkout 5g-lena-v2x-v0.2.y
```

### 4. Switch to the recommended ns-3 release branch that includes V2X extensions:

Switch to ns-3 branch with V2X extensions: 

```
cd ../..
$ git checkout v2x-lte-dev
```

### 5. Check out compatible ns-3 V2X tag

To check out the correct tag, consult the following table:

| NR V2X git branch| ns-3 V2X git tag| Build system| ns-3 version|
| ------ | ------ | ------ | ------ |
| 5g-lena-v2x-v0.1.y | ns-3-dev-v2x-v0.1 | waf | ns-3.35 |
| 5g-lena-v2x-v0.2.y | ns-3-dev-v2x-v0.2 | cmake | ns-3.36 |
| to be released | to be released | cmake | ns-3.37 |

For example, for NR relase branch called 5g-lena-v2x-v0.2.y, the compatible ns-3 release tag is ns-3-dev-v2x-v0.2.
(To see the list of available ns-3 v2x release tags you can run: `git tag -l "*v2x*"`)

To check out the git tag run:

```
git checkout ns-3-dev-v2x-v0.2
```

Git will now warn you that you are in a 'detached HEAD' state. Don't worry that is OK. 

### 6. Test ns-3 + nr installation:

Let's configure the ns-3 + NR project with V2X estensions:

```
cd ../..
./ns3 configure --disable-python --enable-tests --enable-examples
```

In the output you should see: `SQLite stats support: enabled`. If that is not the case, return to "ns-3 and NR prerequisites" section, and install all prerequisites. After the installation of the missing packages run again `./ns3 configure --enable-tests --enable-examples`. 

To compile the ns-3 with NR you can run the following command:

```
./ns3 build
```

If the NR module is recognized correctly, you should see "nr" in the list of
built modules. If that is the case, _Welcome to the NR V2X world !_

## Run examples: 

To run `cttc-nr-v2x-demo-simple.cc` example from the nr/examples folder run: 

```
./ns3 run "cttc-nr-v2x-demo-simple"
```

To run `nr-v2x-west-to-east-highway.cc` example from nr/examples folder run: 

```
./ns3 run "nr-v2x-west-to-east-highway"
```

## Upgrading 5G-LENA

We assume that your work lives in a separate branch, and that the 'master' and 
'nr-v2x-dev' and release branches of the NR repository is left untouched as 
the first time you downloaded it. 
If it is not the case, then please move all your work in a separate branch.

A vanilla 'master' and 'nr-v2x-dev' branches can be updated by simply running:

```
$ cd ns-3-dev/contrib/nr    # or src/nr if the module lives under src/
$ git checkout master
$ git pull
$ git checkout nr-v2x-dev
$ git pull
```
At each release NR V2X release, we will incorporate into the nr-v2x-dev branch all the work that
is meant to be released.

### Building NR V2X documentation

To build the NR V2X documentation on your own, you can follow the
instructions from this section.

- To build the user manual, navigate to the nr folder and then:

```
cd doc
make latexpdf
```

And you will find the PDF user manual in the directory build/latex. Please note
that you may have to install some requirements to build the documentation; you
can find the list of packages for any Ubuntu-based distribution in the file
`.gitlab-ci.yml`.

- To build the doxygen documentation, please do from the nr folder:

```
git submodule sync --recursive
git submodule update --init --recursive
python3 doc/m.css/documentation/doxygen.py doc/doxygen-mcss.conf --debug
```

You will find the doxygen documentation inside `doc/doc/html/`.
Please note that you may need to initialize the m.css submodule, and
to install some packages like python3.

## Papers

An updated list of published papers that are based on the outcome of this
module is available
[here](https://cttc-lena.gitlab.io/5g-lena-website/papers/).


## Authors ##

In alphabetical order:

- Zoraze Ali
- Biljana Bojovic
- Lorenza Giupponi
- Katerina Koutlia
- Sandra Lagen
- Tom Henderson
- Natale Patriciello

Inspired by [mmWave module by NYU/UniPD] (https://github.com/nyuwireless-unipd/ns3-mmwave)

## License ##

This software is licensed under the terms of the GNU GPLv2, as like as ns-3.
See the LICENSE file for more details.
