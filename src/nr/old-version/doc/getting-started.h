/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2018 Natale Patriciello <natale.patriciello\gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/** \page getting-started Getting started
\brief Get started with 5G-LENA in matter of minutes.

\tableofcontents

The first thing to do is take some confidence with the ns-3 environment.
We cannot really help you here, but you can find on the Web a lot of information
and tutorials. So, we will download and build the 5G-LENA project, but then
we will point out some ns-3 tutorials before entering the NR domain.

\note
Many of these instructions are copied from the README file. If you find an
inconsistency, please open a support ticket!

\section getting-started-ns3 Download the ns-3 part of the 5G-LENA project

We try to keep in sync with the latest advancements in ns-3-dev. By the version
1.0, we have upstreamed all our patches to ns-3-dev, making our module
independent from the ns-3 version used.

\note
If you don't have the permission to see the repository, it is probably due
to the fact that you did not requested it. Even though 5G LENA is GPLv2-licensed,
the access to the code is restricted.

\subsection download-ns3 Download a brand new ns-3-dev repository
To download a working copy of the ns-3-dev repository, you can do the following:

\code{.sh}
$ git clone git@gitlab.com:nsnam/ns-3-dev.git
$ cd ns-3-dev
\endcode

Provide your username and password when asked. If you don't have an account
on gitlab.com, you can use `https://gitlab.com/nsnam/ns-3-dev.git` as the
repository address.

In case you are already using the git mirror of ns-3-dev, hosted at GitHub or GitLab, you are already ready to go (please make sure to be up-to-date with
`git pull` in the master branch!).

\subsection switching-ns3 Switching from CTTC-provided ns-3-dev

Before v1.0, the NR module needed a custom ns-3-dev version. For those of you
that are upgrading from v0.4 to v1.0, the steps to switch to the official
ns-3 repository are the following (without recreating the repo configuration):

\code{.sh}
$ git remote add nsnam git@gitlab.com:nsnam/ns-3-dev.git
$ git checkout master
$ git pull nsnam master
\endcode

Anyway, we will make sure that the master of our custom ns-3-dev will stay
up-to-date with respect to the official ns-3-dev.

\subsection test-ns3 Test the installation

To test the installation, after following one of the previous point, you can do
a simple configuration and compile test (more options for that later):

\code{.sh}
$ ./waf configure --enable-examples --enable-tests
$ ./waf
\endcode

A success for both previous commands indicates an overall success.

\section getting-started-nr Download the 5G-LENA core project

As a precondition to the following steps, you must have a working local git
repository. If that is the case, then, your local git repo is ready to include
our nr module:

\code{.sh}
cd src
git clone git@gitlab.com:cttc-lena/nr.git
cd ..
\endcode

Please note that the src/nr directory will be listed as "Untracked files" every
time you do a git status command. Ignore it, as the directory lives as an
independent module. As a result, we have now two parallel repository, but one
lives inside the other. We are working to be able to put nr inside the
contrib/ directory, as per standard ns-3 rules.

To test the resulting repository, let's configure the project again:
\code{.sh}
$ ./waf configure --enable-examples --enable-tests
\endcode

If the NR module is recognized correctly, you should see "nr" in the list of
built modules. If that is not the case, then most probably the previous
point failed. Otherwise, you could compile it:

\code{sh}
$ ./waf
\endcode

If that command returns successfully, Welcome to the NR world !

\section upgrade Upgrading 5G-LENA

We assume that your work lives in a separate branch, and that the 'master'
branch of the NR repository is left untouched as the first time you downloaded
it. If it is not the case, then please move all your work in a separate branch.

A vanilla 'master' branch can be updated by simply running:

\code{.sh}
$ cd ns-3-dev/src/nr
$ git checkout master
$ git pull
\endcode

At each release, we will incorporate into the master branch all the work that
is meant to be released.

For what regards ns-3-dev (the main directory in which, under src/ or contrib/,
you saved the NR module) the story is a bit different. Since we often rewrite
its history to keep pace with ns-3-dev plus our patches to LTE that have not been
accepted in the mainline, it is possible that with a simple `git pull` it will
not upgrade correctly. What we suggest is, if the `git pull` strategy leads to
conflicts, to download again our ns-3-dev repository, following the instructions
at the beginning of this file (the repository is gitlab.com:cttc-lena/ns-3-dev.git).

\section getting-started-tutorial ns-3 tutorials

If it is the first time you work with the ns-3 environment, we recommend to take
things slowly (but steady) and going forward through simple steps.
The ns-3 documentation <https://www.nsnam.org/documentation/> is divided into
two categories: the reference manual for the ns-3 core, and a separate model
library. We suggest to read the following:

- The ns-3 core tutorial: <https://www.nsnam.org/docs/tutorial/html/index.html>
- The ns-3 core manual: <https://www.nsnam.org/docs/manual/html/index.html>
- The LTE documentation: <https://www.nsnam.org/docs/models/html/lte.html>

*/
