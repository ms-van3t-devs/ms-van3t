# Patching scripts directory

**Normally the user is not expected to use these scripts and it is not advised to do so, unless he/she knows exactly what he/she is doing!**

The code for the C-V2X module has been patched by taking the C-V2X model by Fabian Eckermann (https://github.com/FabianEckermann/ns-3_c-v2x), based itself on the NIST D2D model, based in turn on LENA, and by adding the `cv2x_` prefix to each file/class/struct.

This should enable the full co-existence between standard LENA and the C-V2X model, enabling the simultaneous simulation of LTE and C-V2X.

In order to make this complex task easier, we developed two scripts, which are available in this directory:
- c-v2x-patch.sh, adding the prefix to all file names inside "model", "helper" and "test", patching the class names and patching wscript
- c-v2x-patch-struct.sh, patching the struct names

They are not sufficient to reach the final result of having all the code correctly patched (some further manual work is needed), but we decided to make them part of the repository as they can be useful for future reference, or to patch a future new version of the C-V2X model by Fabian Eckermann.

Just for reference, the steps we followed, with the aim of patching all the code, are the following:
- Create a clean copy of the LTE/LENA module (`ns-3.29/src/lte`) and rename the folder to `cv2x`
- Copy the two scripts into `ns-3.29/src/cv2x`
- Run `c-v2x-patch.sh`
- Run `c-v2x-patch-struct.sh`
- Perform any needed manual correction (for instance, we had to manually add the `cv2x_` prefix to some `NS_LOG_COMPONENT_DEFINE` lines, in few source files, or to fix some files in which `<something>-header.h` was erroneously renamed into `<something>.header.h`)
- (In case of any error, always restart from a clean copy of `lte`)

As stated before, normally the user should not modify this module, but just use it inside his/her vehicular applications.

-*Marco Malinverno, Politecnico di Torino*
-*Francesco Raviglione, Politecnico di Torino*
