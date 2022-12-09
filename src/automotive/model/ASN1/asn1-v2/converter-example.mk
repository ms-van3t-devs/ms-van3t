include Makefile.am.libasncodec

LIBS += -lm
CFLAGS += $(ASN_MODULE_CFLAGS) -DASN_PDU_COLLECTION -I.
ASN_LIBRARY ?= libasncodec.a
ASN_PROGRAM ?= converter-example
ASN_PROGRAM_SRCS ?= \
	converter-example.c\
	pdu_collection.c

all: $(ASN_PROGRAM)

$(ASN_PROGRAM): $(ASN_LIBRARY) $(ASN_PROGRAM_SRCS:.c=.o)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $(ASN_PROGRAM) $(ASN_PROGRAM_SRCS:.c=.o) $(LDFLAGS) $(ASN_LIBRARY) $(LIBS)

$(ASN_LIBRARY): $(ASN_MODULE_SRCS:.c=.o)
	$(AR) rcs $@ $(ASN_MODULE_SRCS:.c=.o)

.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(ASN_PROGRAM) $(ASN_LIBRARY)
	rm -f $(ASN_MODULE_SRCS:.c=.o) $(ASN_PROGRAM_SRCS:.c=.o)

regen: regenerate-from-asn1-source

regenerate-from-asn1-source:
	asn1c -fcompound-names ../IVIM-ASN1-files/asn1_IS_ETSI_TS_103301_IVIM_PDU_Descriptions.asn ../IVIM-ASN1-files/asn1_IS_ISO_TS_14816_AVIAEINumberingAndDataStructures.asn ../IVIM-ASN1-files/asn1_IS_ISO_TS_14823_GDD.asn ../IVIM-ASN1-files/asn1_IS_ISO_TS_14906_EfcDsrcApplication.asn ../IVIM-ASN1-files/asn1_IS_ISO_TS_14906_EfcDsrcGeneric.asn ../IVIM-ASN1-files/asn1_IS_ISO_TS_17419_CITSapplMgmtIDs.asn ../IVIM-ASN1-files/asn1_IS_ISO_TS_19091_AddGrpC.asn ../IVIM-ASN1-files/asn1_IS_ISO_TS_19091_DSRC_REGION_noCircular.asn ../IVIM-ASN1-files/asn1_IS_ISO_TS_19091_REGION.asn ../IVIM-ASN1-files/asn1_IS_ISO_TS_19321_IVI.asn ../IVIM-ASN1-files/asn1_IS_ISO_TS_24534-3_ElectronicRegistrationIdentificationVehicleDataModule.asn ../IVIM-ASN1-files/ETSI CAM v1.4.1.asn ../IVIM-ASN1-files/ETSI DENM v1.3.1.asn ../IVIM-ASN1-files/ITS-Container.asn

