/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "IVI"
 * 	found in "/home/carlosrisma/IVIM ASN1 files/asn1_IS_ISO_TS_19321_IVI.asn"
 * 	`asn1c -fincludes-quoted`
 */

#include "DTM.h"

static int
memb_syr_constraint_2(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 2000 && value <= 2127)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static int
memb_eyr_constraint_2(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 2000 && value <= 2127)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static asn_oer_constraints_t asn_OER_memb_syr_constr_3 CC_NOTUSED = {
	{ 0, 0 },
	-1};
static asn_per_constraints_t asn_PER_memb_syr_constr_3 CC_NOTUSED = {
	{ APC_CONSTRAINED | APC_EXTENSIBLE,  7,  7,  2000,  2127 }	/* (2000..2127,...) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_oer_constraints_t asn_OER_memb_eyr_constr_4 CC_NOTUSED = {
	{ 0, 0 },
	-1};
static asn_per_constraints_t asn_PER_memb_eyr_constr_4 CC_NOTUSED = {
	{ APC_CONSTRAINED | APC_EXTENSIBLE,  7,  7,  2000,  2127 }	/* (2000..2127,...) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_TYPE_member_t asn_MBR_year_2[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct DTM__year, syr),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ &asn_OER_memb_syr_constr_3, &asn_PER_memb_syr_constr_3,  memb_syr_constraint_2 },
		0, 0, /* No default value */
		"syr"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DTM__year, eyr),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ &asn_OER_memb_eyr_constr_4, &asn_PER_memb_eyr_constr_4,  memb_eyr_constraint_2 },
		0, 0, /* No default value */
		"eyr"
		},
};
static const ber_tlv_tag_t asn_DEF_year_tags_2[] = {
	(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_year_tag2el_2[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* syr */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* eyr */
};
static asn_SEQUENCE_specifics_t asn_SPC_year_specs_2 = {
	sizeof(struct DTM__year),
	offsetof(struct DTM__year, _asn_ctx),
	asn_MAP_year_tag2el_2,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* First extension addition */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_year_2 = {
	"year",
	"year",
	&asn_OP_SEQUENCE,
	asn_DEF_year_tags_2,
	sizeof(asn_DEF_year_tags_2)
		/sizeof(asn_DEF_year_tags_2[0]) - 1, /* 1 */
	asn_DEF_year_tags_2,	/* Same as above */
	sizeof(asn_DEF_year_tags_2)
		/sizeof(asn_DEF_year_tags_2[0]), /* 2 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_year_2,
	2,	/* Elements count */
	&asn_SPC_year_specs_2	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_month_day_5[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct DTM__month_day, smd),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_MonthDay,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"smd"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DTM__month_day, emd),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_MonthDay,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"emd"
		},
};
static const ber_tlv_tag_t asn_DEF_month_day_tags_5[] = {
	(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_month_day_tag2el_5[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* smd */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* emd */
};
static asn_SEQUENCE_specifics_t asn_SPC_month_day_specs_5 = {
	sizeof(struct DTM__month_day),
	offsetof(struct DTM__month_day, _asn_ctx),
	asn_MAP_month_day_tag2el_5,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* First extension addition */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_month_day_5 = {
	"month-day",
	"month-day",
	&asn_OP_SEQUENCE,
	asn_DEF_month_day_tags_5,
	sizeof(asn_DEF_month_day_tags_5)
		/sizeof(asn_DEF_month_day_tags_5[0]) - 1, /* 1 */
	asn_DEF_month_day_tags_5,	/* Same as above */
	sizeof(asn_DEF_month_day_tags_5)
		/sizeof(asn_DEF_month_day_tags_5[0]), /* 2 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_month_day_5,
	2,	/* Elements count */
	&asn_SPC_month_day_specs_5	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_hourMinutes_9[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct DTM__hourMinutes, shm),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HoursMinutes,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"shm"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DTM__hourMinutes, ehm),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HoursMinutes,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ehm"
		},
};
static const ber_tlv_tag_t asn_DEF_hourMinutes_tags_9[] = {
	(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_hourMinutes_tag2el_9[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* shm */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* ehm */
};
static asn_SEQUENCE_specifics_t asn_SPC_hourMinutes_specs_9 = {
	sizeof(struct DTM__hourMinutes),
	offsetof(struct DTM__hourMinutes, _asn_ctx),
	asn_MAP_hourMinutes_tag2el_9,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* First extension addition */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_hourMinutes_9 = {
	"hourMinutes",
	"hourMinutes",
	&asn_OP_SEQUENCE,
	asn_DEF_hourMinutes_tags_9,
	sizeof(asn_DEF_hourMinutes_tags_9)
		/sizeof(asn_DEF_hourMinutes_tags_9[0]) - 1, /* 1 */
	asn_DEF_hourMinutes_tags_9,	/* Same as above */
	sizeof(asn_DEF_hourMinutes_tags_9)
		/sizeof(asn_DEF_hourMinutes_tags_9[0]), /* 2 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_hourMinutes_9,
	2,	/* Elements count */
	&asn_SPC_hourMinutes_specs_9	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_DTM_1[] = {
	{ ATF_POINTER, 6, offsetof(struct DTM, year),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		0,
		&asn_DEF_year_2,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"year"
		},
	{ ATF_POINTER, 5, offsetof(struct DTM, month_day),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		0,
		&asn_DEF_month_day_5,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"month-day"
		},
	{ ATF_POINTER, 4, offsetof(struct DTM, pmd),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PMD,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"pmd"
		},
	{ ATF_POINTER, 3, offsetof(struct DTM, hourMinutes),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		0,
		&asn_DEF_hourMinutes_9,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"hourMinutes"
		},
	{ ATF_POINTER, 2, offsetof(struct DTM, dayOfWeek),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DayOfWeek,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"dayOfWeek"
		},
	{ ATF_POINTER, 1, offsetof(struct DTM, period),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HoursMinutes,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"period"
		},
};
static const int asn_MAP_DTM_oms_1[] = { 0, 1, 2, 3, 4, 5 };
static const ber_tlv_tag_t asn_DEF_DTM_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_DTM_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* year */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* month-day */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* pmd */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* hourMinutes */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* dayOfWeek */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 } /* period */
};
asn_SEQUENCE_specifics_t asn_SPC_DTM_specs_1 = {
	sizeof(struct DTM),
	offsetof(struct DTM, _asn_ctx),
	asn_MAP_DTM_tag2el_1,
	6,	/* Count of tags in the map */
	asn_MAP_DTM_oms_1,	/* Optional members */
	6, 0,	/* Root/Additions */
	-1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_DTM = {
	"DTM",
	"DTM",
	&asn_OP_SEQUENCE,
	asn_DEF_DTM_tags_1,
	sizeof(asn_DEF_DTM_tags_1)
		/sizeof(asn_DEF_DTM_tags_1[0]), /* 1 */
	asn_DEF_DTM_tags_1,	/* Same as above */
	sizeof(asn_DEF_DTM_tags_1)
		/sizeof(asn_DEF_DTM_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_DTM_1,
	6,	/* Elements count */
	&asn_SPC_DTM_specs_1	/* Additional specs */
};

