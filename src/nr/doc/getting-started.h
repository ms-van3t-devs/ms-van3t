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

In order to start with the NR module you should have some confidence with
the ns-3 environment. You can find on the ns-3 web site all the necessary information.

In the following instructions, we will download and build the 5G-LENA project,
and we will point out to ns-3 and 5G-LENA documentation to help you enter
the ns-3 and the NR world.

\section getting-started-ns3 Download the ns-3 part of the 5G-LENA project

\subsection download-ns3 Download a brand new ns-3-dev repository
To download a working copy of the ns-3-dev repository, you can do the following:

\code{.sh}
$ git clone git@gitlab.com:nsnam/ns-3-dev.git
$ cd ns-3-dev
\endcode

We try to keep in sync with the latest advancements in ns-3-dev. By the version
1.0, we have upstreamed all our patches to ns-3-dev, making our module
independent from the ns-3 version used. However, since the version 1.3, we
recommend the specific ns-3 release branch for each NR release, i.e., the
ns-3 release branch with which are carried out all CI/CD tests for the specific
NR release. Hence, at this step, you should check in RELEASE_NOTES.md of the NR module
which is the recommended ns-3 release to use for the specific NR release,
and then you can switch to the corresponding ns-3 release branch, e.g., in the
following way:

```
$ git checkout ns-3.36

```

You can replace "36" with the specific release that you want to use. If the
recommended ns-3 release is not available yet (such in the case that NR is
released before the recommended ns-3 release), then you can use ns-3 master
until ns-3 recommended release is ready.


\subsection test-ns3 Test the installation

To test the installation, after following the previous steps, you can do
a simple configuration and compilation test:

\code{.sh}
$ ./ns3 configure --enable-examples --enable-tests
$ ./ns3 build
\endcode

A success for both previous commands indicates an overall success and you are
ready to install the NR module.

\section getting-started-nr Download the 5G-LENA core project

As a precondition to the following steps, you must have a working local ns-3 git
repository as explained in the previous steps.

Clone the NR module:

\code{.sh}
cd src
git clone https://gitlab.com/cttc-lena/nr.git
\endcode

At this step, you should switch to the latest NR release branch.
For example, to switch to the NR version 2.2 you should do the following:

\code{.sh}
cd nr
git checkout 5g-lena-v2.2.y
\endcode

Please note that the src/nr directory will be listed as "Untracked files" every
time you do a git status command. Ignore it, as the directory lives as an
independent module. This is normal, since you have two parallel git repositories,
that of ns-3 and of nr. Notice that you can install the nr module also inside
the contrib/ directory, as per standard ns-3 instructions.

To test the resulting repository, go back to the root ns-3 directory and
configure the ns-3 project and build it again:
\code{.sh}
$ cd ../../
$ ./ns3 configure --enable-examples --enable-tests
$ ./ns3 build
\endcode

If the NR module is recognized correctly, you should see "nr" in the list of
built modules. If that is the case, Welcome to the NR world !!! :-)

Now, you can run some of NR examples (see nr/examples folder):

\code{.sh}
$ ./ns3 run cttc-nr-mimo-demo
\endcode

For more examples see nr/examples folder.

\section contributing Contributing to 5G-LENA

We would be very happy if you would contribute to 5G-LENA!

If you do some of the following with 5G-LENA:

  - find and solve some bug,
  - add some new parameter,
  - create a completely and different example or test,
  - parametrize existing piece of code,
  - develop a completely new feature,
  - extend the tracing system through files or the databases,
  - improve visualization of the scenario through the python scripts
  - or whatever else,

and you would like to contribute that to the open source NR module,
please create a merge request (MR) or contact us to guide you through the
contributing process.


\section getting-started-tutorial Get familiar with ns-3 and the NR module

We recommend you to first get familiar with ns-3 and then with the NR module.

\subsection getting-started-ns-3 ns-3 tutorials

If it is the first time you work with the ns-3 environment, we recommend to take
things slowly (but steady) and going forward through simple steps.
The ns-3 documentation <https://www.nsnam.org/documentation/> is divided into
three categories tutorial, manual and documentation (describing models):

- The ns-3 tutorial: <https://www.nsnam.org/docs/tutorial/html/index.html>
- The ns-3 manual: <https://www.nsnam.org/docs/manual/html/index.html>
- The LTE documentation: <https://www.nsnam.org/docs/models/html/lte.html>

\subsection getting-started-nr-doc NR documentation

The NR documentation is divided into two categories:

- The NR module general documentation describing models: <https://cttc-lena.gitlab.io/nr/nrmodule.pdf>
- The NR doxygen documentation (very detailed code documentation): <https://cttc-lena.gitlab.io/nr/html/>


The publications related to the NR module and its extensions are also very
valuable source of information. The list of the publications can be found here:
<https://5g-lena.cttc.es/papers/>.


\section upgrade Upgrading 5G-LENA

We assume that your work lives in a separate branch, and that the 'master'
branch of the NR repository is left untouched as the first time you downloaded
it. If it is not the case, then please move all your work in a separate branch.

A clean 'master' branch can be updated by simply running:

\code{.sh}
$ cd ns-3-dev/src/nr
$ git pull
\endcode

Then checkout the latest NR release branch (e.g., `git checkout 5g-lena-v2.2.y`).
With each release, we will also incorporate into the master branch all the work
that is meant to be released.

For what regards ns-3-dev run `git pull` and switch to its release branch that is
indicated in the NR RELEASE_NOTES.md for the corresponding NR release.

\note
Many of these instructions are copied from the NR README.md file. If you find an
inconsistency, please open a support ticket!

*/
