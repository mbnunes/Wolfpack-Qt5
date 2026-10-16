# -*- coding: utf-8 -*-
#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Antigravity                    #
#   )).-' {{ ;'`   |                                            #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# .respawntowns Command                                         #
#===============================================================#

"""
	\\command respawntowns
	\\description Respawns all town/city spawnregions or a specific town.
	\\usage - <code>respawntowns [town|clear|list]</code>
"""

import wolfpack

TOWN_SPAWNREGIONS = {
	"britain": [
		"britain_vendor_1", "britain_vendor_2", "britain_vendor_3", "britain_vendor_4",
		"britain_vendor_5", "britain_vendor_6", "britain_vendor_7", "britain_vendor_8",
		"britain_vendor_9", "britain_vendor_10", "britain_vendor_11", "britain_vendor_12",
		"britain_vendor_13", "britain_vendor_14", "britain_vendor_15", "britain_vendor_16",
		"britain_vendor_17", "britain_vendor_18", "britain_vendor_19", "britain_vendor_20",
		"britain_vendor_21", "britain_vendor_22", "britain_vendor_23", "britain_vendor_24",
		"britain_vendor_25", "britain_vendor_26", "britain_vendor_27", "britain_vendor_28",
		"britain_vendor_29", "britain_vendor_30", "britain_vendor_31", "britain_vendor_32",
		"britain_vendor_33", "britain_vendor_34", "britain_vendor_35", "britain_vendor_36",
		"felucca_britain_vendor_1", "felucca_britain_vendor_2", "felucca_britain_vendor_3",
		"felucca_britain_vendor_4", "felucca_britain_vendor_5", "felucca_britain_vendor_6",
		"felucca_britain_vendor_7", "felucca_britain_vendor_8", "felucca_britain_vendor_9",
		"felucca_britain_vendor_10", "felucca_britain_vendor_11", "felucca_britain_vendor_12",
		"felucca_britain_vendor_13", "felucca_britain_vendor_14", "felucca_britain_vendor_15",
		"felucca_britain_vendor_16", "felucca_britain_vendor_17", "felucca_britain_vendor_18",
		"felucca_britain_vendor_19", "felucca_britain_vendor_20", "felucca_britain_vendor_21",
		"felucca_britain_vendor_22", "felucca_britain_vendor_23", "felucca_britain_vendor_24",
		"felucca_britain_vendor_25", "felucca_britain_vendor_26", "felucca_britain_vendor_27",
		"felucca_britain_vendor_28", "felucca_britain_vendor_29", "felucca_britain_vendor_30",
		"felucca_britain_vendor_31", "felucca_britain_vendor_32", "felucca_britain_vendor_33",
		"felucca_britain_vendor_34", "felucca_britain_vendor_35", "felucca_britain_vendor_36"
	],
	"cove": [
		"cove_vendor_1", "cove_vendor_2", "cove_vendor_3",
		"felucca_cove_vendor_1", "felucca_cove_vendor_2", "felucca_cove_vendor_3"
	],
	"jhelom": [
		"jhemlom_vendor_1", "jhemlom_vendor_2", "jhemlom_vendor_3", "jhemlom_vendor_4",
		"jhemlom_vendor_5", "jhemlom_vendor_6", "jhemlom_vendor_7", "jhemlom_vendor_8",
		"jhemlom_vendor_9", "jhemlom_vendor_10", "jhemlom_vendor_11", "jhemlom_vendor_12",
		"jhemlom_vendor_13", "jhemlom_vendor_14", "jhemlom_vendor_15", "jhemlom_vendor_16",
		"jhemlom_vendor_17", "jhemlom_vendor_18", "jhemlom_vendor_19", "jhemlom_vendor_20",
		"jhemlom_vendor_21", "jhemlom_vendor_22", "jhemlom_vendor_23", "jhemlom_vendor_24",
		"felucca_jhemlom_vendor_1", "felucca_jhemlom_vendor_2", "felucca_jhemlom_vendor_3",
		"felucca_jhemlom_vendor_4", "felucca_jhemlom_vendor_5", "felucca_jhemlom_vendor_6",
		"felucca_jhemlom_vendor_7", "felucca_jhemlom_vendor_8", "felucca_jhemlom_vendor_9",
		"felucca_jhemlom_vendor_10", "felucca_jhemlom_vendor_11", "felucca_jhemlom_vendor_12",
		"felucca_jhemlom_vendor_13", "felucca_jhemlom_vendor_14", "felucca_jhemlom_vendor_15",
		"felucca_jhemlom_vendor_16", "felucca_jhemlom_vendor_17", "felucca_jhemlom_vendor_18",
		"felucca_jhemlom_vendor_19", "felucca_jhemlom_vendor_20", "felucca_jhemlom_vendor_21",
		"felucca_jhemlom_vendor_22", "felucca_jhemlom_vendor_23", "felucca_jhemlom_vendor_24"
	],
	"minoc": [
		"minoc_vendor_1", "minoc_vendor_2", "minoc_vendor_3", "minoc_vendor_4",
		"minoc_vendor_5", "minoc_vendor_6", "minoc_vendor_7", "minoc_vendor_8",
		"minoc_vendor_9", "minoc_vendor_10", "minoc_vendor_11", "minoc_vendor_12",
		"minoc_vendor_13", "minoc_vendor_14", "minoc_vendor_15", "minoc_vendor_16",
		"felucca_minoc_vendor_1", "felucca_minoc_vendor_2", "felucca_minoc_vendor_3",
		"felucca_minoc_vendor_4", "felucca_minoc_vendor_5", "felucca_minoc_vendor_6",
		"felucca_minoc_vendor_7", "felucca_minoc_vendor_8", "felucca_minoc_vendor_9",
		"felucca_minoc_vendor_10", "felucca_minoc_vendor_11", "felucca_minoc_vendor_12",
		"felucca_minoc_vendor_13", "felucca_minoc_vendor_14", "felucca_minoc_vendor_15",
		"felucca_minoc_vendor_16"
	],
	"moonglow": [
		"moonglow_vendor_1", "moonglow_vendor_2", "moonglow_vendor_3", "moonglow_vendor_4",
		"moonglow_vendor_5", "moonglow_vendor_6", "moonglow_vendor_7", "moonglow_vendor_8",
		"moonglow_vendor_9", "moonglow_vendor_10", "moonglow_vendor_11", "moonglow_vendor_12",
		"moonglow_vendor_13", "moonglow_vendor_14", "moonglow_vendor_15", "moonglow_vendor_16",
		"moonglow_vendor_17", "moonglow_vendor_18", "moonglow_vendor_19", "moonglow_vendor_20",
		"felucca_moonglow_vendor_1", "felucca_moonglow_vendor_2", "felucca_moonglow_vendor_3",
		"felucca_moonglow_vendor_4", "felucca_moonglow_vendor_5", "felucca_moonglow_vendor_6",
		"felucca_moonglow_vendor_7", "felucca_moonglow_vendor_8", "felucca_moonglow_vendor_9",
		"felucca_moonglow_vendor_10", "felucca_moonglow_vendor_11", "felucca_moonglow_vendor_12",
		"felucca_moonglow_vendor_13", "felucca_moonglow_vendor_14", "felucca_moonglow_vendor_15",
		"felucca_moonglow_vendor_16", "felucca_moonglow_vendor_17", "felucca_moonglow_vendor_18",
		"felucca_moonglow_vendor_19", "felucca_moonglow_vendor_20"
	],
	"trinsic": [
		"trinsic_vendor_1", "trinsic_vendor_2", "trinsic_vendor_3", "trinsic_vendor_4",
		"trinsic_vendor_5", "trinsic_vendor_6", "trinsic_vendor_7", "trinsic_vendor_8",
		"trinsic_vendor_9", "trinsic_vendor_10", "trinsic_vendor_11", "trinsic_vendor_12",
		"trinsic_vendor_13", "trinsic_vendor_14", "trinsic_vendor_15", "trinsic_vendor_16",
		"trinsic_vendor_17", "trinsic_vendor_18", "trinsic_vendor_19", "trinsic_vendor_20",
		"trinsic_vendor_21",
		"felucca_trinsic_vendor_1", "felucca_trinsic_vendor_2", "felucca_trinsic_vendor_3",
		"felucca_trinsic_vendor_4", "felucca_trinsic_vendor_5", "felucca_trinsic_vendor_6",
		"felucca_trinsic_vendor_7", "felucca_trinsic_vendor_8", "felucca_trinsic_vendor_9",
		"felucca_trinsic_vendor_10", "felucca_trinsic_vendor_11", "felucca_trinsic_vendor_12",
		"felucca_trinsic_vendor_13", "felucca_trinsic_vendor_14", "felucca_trinsic_vendor_15",
		"felucca_trinsic_vendor_16", "felucca_trinsic_vendor_17", "felucca_trinsic_vendor_18",
		"felucca_trinsic_vendor_19", "felucca_trinsic_vendor_20", "felucca_trinsic_vendor_21"
	],
	"vesper": [
		"vesper_vendor_1", "vesper_vendor_2", "vesper_vendor_3", "vesper_vendor_4",
		"vesper_vendor_5", "vesper_vendor_6", "vesper_vendor_7", "vesper_vendor_8",
		"vesper_vendor_9", "vesper_vendor_10", "vesper_vendor_11", "vesper_vendor_12",
		"vesper_vendor_13", "vesper_vendor_14", "vesper_vendor_15", "vesper_vendor_16",
		"vesper_vendor_17", "vesper_vendor_18", "vesper_vendor_19", "vesper_vendor_20",
		"vesper_vendor_21", "vesper_vendor_22", "vesper_vendor_23", "vesper_vendor_24",
		"vesper_vendor_25", "vesper_vendor_26", "vesper_vendor_27", "vesper_vendor_28",
		"vesper_vendor_29", "vesper_vendor_30", "vesper_vendor_31", "vesper_vendor_32",
		"vesper_vendor_33", "vesper_vendor_34", "vesper_vendor_35", "vesper_vendor_36",
		"vesper_vendor_37", "vesper_vendor_38", "vesper_vendor_39",
		"felucca_vesper_vendor_1", "felucca_vesper_vendor_2", "felucca_vesper_vendor_3",
		"felucca_vesper_vendor_4", "felucca_vesper_vendor_5", "felucca_vesper_vendor_6",
		"felucca_vesper_vendor_7", "felucca_vesper_vendor_8", "felucca_vesper_vendor_9",
		"felucca_vesper_vendor_10", "felucca_vesper_vendor_11", "felucca_vesper_vendor_12",
		"felucca_vesper_vendor_13", "felucca_vesper_vendor_14", "felucca_vesper_vendor_15",
		"felucca_vesper_vendor_16", "felucca_vesper_vendor_17", "felucca_vesper_vendor_18",
		"felucca_vesper_vendor_19", "felucca_vesper_vendor_20", "felucca_vesper_vendor_21",
		"felucca_vesper_vendor_22", "felucca_vesper_vendor_23", "felucca_vesper_vendor_24",
		"felucca_vesper_vendor_25", "felucca_vesper_vendor_26", "felucca_vesper_vendor_27",
		"felucca_vesper_vendor_28", "felucca_vesper_vendor_29", "felucca_vesper_vendor_30",
		"felucca_vesper_vendor_31", "felucca_vesper_vendor_32", "felucca_vesper_vendor_33",
		"felucca_vesper_vendor_34", "felucca_vesper_vendor_35", "felucca_vesper_vendor_36",
		"felucca_vesper_vendor_37", "felucca_vesper_vendor_38", "felucca_vesper_vendor_39"
	],
	"yew": [
		"yew_vendor_1", "yew_vendor_2", "yew_vendor_3", "yew_vendor_4",
		"yew_vendor_5", "yew_vendor_6", "yew_vendor_7", "yew_vendor_8",
		"yew_vendor_9", "yew_vendor_10", "yew_vendor_11", "yew_vendor_12",
		"yew_vendor_13", "yew_vendor_14", "yew_vendor_15", "yew_vendor_16",
		"felucca_yew_vendor_1", "felucca_yew_vendor_2", "felucca_yew_vendor_3",
		"felucca_yew_vendor_4", "felucca_yew_vendor_5", "felucca_yew_vendor_6",
		"felucca_yew_vendor_7", "felucca_yew_vendor_8", "felucca_yew_vendor_9",
		"felucca_yew_vendor_10", "felucca_yew_vendor_11", "felucca_yew_vendor_12",
		"felucca_yew_vendor_13", "felucca_yew_vendor_14", "felucca_yew_vendor_15",
		"felucca_yew_vendor_16"
	],
	"haven": [
		"haven_vendor_1", "haven_vendor_2", "haven_vendor_3", "haven_vendor_4",
		"haven_vendor_5", "haven_vendor_6", "haven_vendor_7", "haven_vendor_8",
		"haven_vendor_9", "haven_vendor_10", "haven_vendor_11", "haven_vendor_12",
		"haven_vendor_13", "haven_vendor_14", "haven_vendor_15", "haven_vendor_16",
		"haven_vendor_17", "haven_vendor_18", "haven_vendor_19", "haven_vendor_20",
		"haven_vendor_21", "haven_vendor_22", "haven_vendor_23", "haven_vendor_24",
		"haven_vendor_25"
	],
	"bucs": [
		"bucs_vendor_1", "bucs_vendor_2", "bucs_vendor_3", "bucs_vendor_4",
		"bucs_vendor_5", "bucs_vendor_6", "bucs_vendor_7", "bucs_vendor_8",
		"felucca_bucs_vendor_1", "felucca_bucs_vendor_2", "felucca_bucs_vendor_3",
		"felucca_bucs_vendor_4", "felucca_bucs_vendor_5", "felucca_bucs_vendor_6",
		"felucca_bucs_vendor_7", "felucca_bucs_vendor_8"
	],
	"delucia": [
		"delucia_vendor_1", "delucia_vendor_2", "delucia_vendor_3", "delucia_vendor_4",
		"delucia_vendor_5", "delucia_vendor_6", "delucia_vendor_7", "delucia_vendor_8",
		"felucca_delucia_vendor_1", "felucca_delucia_vendor_2", "felucca_delucia_vendor_3",
		"felucca_delucia_vendor_4", "felucca_delucia_vendor_5", "felucca_delucia_vendor_6",
		"felucca_delucia_vendor_7", "felucca_delucia_vendor_8"
	],
	"papua": [
		"papua_vendor_1", "papua_vendor_2", "papua_vendor_3", "papua_vendor_4",
		"papua_vendor_5", "papua_vendor_6", "papua_vendor_7", "papua_vendor_8",
		"papua_vendor_9", "papua_vendor_10", "papua_vendor_11",
		"felucca_papua_vendor_1", "felucca_papua_vendor_2", "felucca_papua_vendor_3",
		"felucca_papua_vendor_4", "felucca_papua_vendor_5", "felucca_papua_vendor_6",
		"felucca_papua_vendor_7", "felucca_papua_vendor_8", "felucca_papua_vendor_9",
		"felucca_papua_vendor_10", "felucca_papua_vendor_11"
	],
	"wind": [
		"wind_vendor_1", "wind_vendor_2", "wind_vendor_3", "wind_vendor_4",
		"wind_vendor_5", "wind_vendor_6", "wind_vendor_7", "wind_vendor_8",
		"wind_vendor_9", "wind_vendor_10", "wind_vendor_11", "wind_vendor_12",
		"wind_vendor_13", "wind_vendor_14", "wind_vendor_15", "wind_vendor_16",
		"wind_vendor_17", "wind_vendor_18", "wind_vendor_19", "wind_vendor_20",
		"felucca_wind_vendor_1", "felucca_wind_vendor_2", "felucca_wind_vendor_3",
		"felucca_wind_vendor_4", "felucca_wind_vendor_5", "felucca_wind_vendor_6",
		"felucca_wind_vendor_7", "felucca_wind_vendor_8", "felucca_wind_vendor_9",
		"felucca_wind_vendor_10", "felucca_wind_vendor_11", "felucca_wind_vendor_12",
		"felucca_wind_vendor_13", "felucca_wind_vendor_14", "felucca_wind_vendor_15",
		"felucca_wind_vendor_16", "felucca_wind_vendor_17", "felucca_wind_vendor_18",
		"felucca_wind_vendor_19", "felucca_wind_vendor_20"
	],
	"serpents_hold": [
		"serpents_hold_vendor_1", "serpents_hold_vendor_2", "serpents_hold_vendor_3",
		"serpents_hold_vendor_4", "serpents_hold_vendor_5",
		"felucca_serpents_hold_vendor_1", "felucca_serpents_hold_vendor_2",
		"felucca_serpents_hold_vendor_3", "felucca_serpents_hold_vendor_4",
		"felucca_serpents_hold_vendor_5"
	],
	"skara_brae": [
		"skara_brae_vendor_1", "skara_brae_vendor_2", "skara_brae_vendor_3",
		"skara_brae_vendor_4", "skara_brae_vendor_5", "skara_brae_vendor_6",
		"skara_brae_vendor_7", "skara_brae_vendor_8", "skara_brae_vendor_9",
		"skara_brae_vendor_10", "skara_brae_vendor_11", "skara_brae_vendor_12",
		"skara_brae_vendor_13", "skara_brae_vendor_14", "skara_brae_vendor_15",
		"felucca_skara_brae_vendor_1", "felucca_skara_brae_vendor_2",
		"felucca_skara_brae_vendor_3", "felucca_skara_brae_vendor_4",
		"felucca_skara_brae_vendor_5", "felucca_skara_brae_vendor_6",
		"felucca_skara_brae_vendor_7", "felucca_skara_brae_vendor_8",
		"felucca_skara_brae_vendor_9", "felucca_skara_brae_vendor_10",
		"felucca_skara_brae_vendor_11", "felucca_skara_brae_vendor_12",
		"felucca_skara_brae_vendor_13", "felucca_skara_brae_vendor_14",
		"felucca_skara_brae_vendor_15"
	],
	"magincia": [
		"magincia_vendor_1", "magincia_vendor_2", "magincia_vendor_3", "magincia_vendor_4",
		"magincia_vendor_5", "magincia_vendor_6", "magincia_vendor_7", "magincia_vendor_8",
		"magincia_vendor_9", "magincia_vendor_10", "magincia_vendor_11", "magincia_vendor_12",
		"magincia_vendor_13", "magincia_vendor_14", "magincia_vendor_15", "magincia_vendor_16",
		"felucca_magincia_vendor_1", "felucca_magincia_vendor_2", "felucca_magincia_vendor_3",
		"felucca_magincia_vendor_4", "felucca_magincia_vendor_5", "felucca_magincia_vendor_6",
		"felucca_magincia_vendor_7", "felucca_magincia_vendor_8", "felucca_magincia_vendor_9",
		"felucca_magincia_vendor_10", "felucca_magincia_vendor_11", "felucca_magincia_vendor_12",
		"felucca_magincia_vendor_13", "felucca_magincia_vendor_14", "felucca_magincia_vendor_15",
		"felucca_magincia_vendor_16"
	],
	"nujelm": [
		"nujelm_vendor_1", "nujelm_vendor_2", "nujelm_vendor_3", "nujelm_vendor_4",
		"nujelm_vendor_5", "nujelm_vendor_6", "nujelm_vendor_7", "nujelm_vendor_8",
		"nujelm_vendor_9", "nujelm_vendor_10", "nujelm_vendor_11", "nujelm_vendor_12",
		"nujelm_vendor_13", "nujelm_vendor_14", "nujelm_vendor_15",
		"felucca_nujelm_vendor_1", "felucca_nujelm_vendor_2", "felucca_nujelm_vendor_3",
		"felucca_nujelm_vendor_4", "felucca_nujelm_vendor_5", "felucca_nujelm_vendor_6",
		"felucca_nujelm_vendor_7", "felucca_nujelm_vendor_8", "felucca_nujelm_vendor_9",
		"felucca_nujelm_vendor_10", "felucca_nujelm_vendor_11", "felucca_nujelm_vendor_12",
		"felucca_nujelm_vendor_13", "felucca_nujelm_vendor_14", "felucca_nujelm_vendor_15"
	],
	"ocllo": [
		"felucca_ocllo_vendor_1", "felucca_ocllo_vendor_2", "felucca_ocllo_vendor_3",
		"felucca_ocllo_vendor_4", "felucca_ocllo_vendor_5", "felucca_ocllo_vendor_6",
		"felucca_ocllo_vendor_7", "felucca_ocllo_vendor_8", "felucca_ocllo_vendor_9",
		"felucca_ocllo_vendor_10", "felucca_ocllo_vendor_11", "felucca_ocllo_vendor_12",
		"felucca_ocllo_vendor_13", "felucca_ocllo_vendor_14", "felucca_ocllo_vendor_15",
		"felucca_ocllo_vendor_16", "felucca_ocllo_vendor_17", "felucca_ocllo_vendor_18",
		"felucca_ocllo_vendor_19", "felucca_ocllo_vendor_20", "felucca_ocllo_vendor_21"
	],
	"luna": [
		"malas_luna_1", "malas_luna_2", "malas_luna_3", "malas_luna_4",
		"malas_luna_5", "malas_luna_6", "malas_luna_7", "malas_luna_8",
		"malas_luna_9", "malas_luna_10", "malas_luna_11", "malas_luna_12",
		"malas_luna_13", "malas_luna_14", "malas_luna_15"
	],
	"umbra": [
		"malas_umbra_1", "malas_umbra_2", "malas_umbra_3", "malas_umbra_4",
		"malas_umbra_5", "malas_umbra_6", "malas_umbra_7", "malas_umbra_8",
		"malas_umbra_9", "malas_umbra_10", "malas_umbra_11", "malas_umbra_12",
		"malas_umbra_13", "malas_umbra_14", "malas_umbra_15", "malas_umbra_16",
		"malas_umbra_17", "malas_umbra_18", "malas_umbra_19"
	],
	"zento": [
		"tokuno_vendors_0_1", "tokuno_vendors_18_12", "tokuno_vendors_29_44",
		"tokuno_vendors_14_58", "tokuno_vendors_26_62", "tokuno_vendors_3_65",
		"tokuno_townslife_0_67", "tokuno_townslife_14_68", "tokuno_townslife_8_70",
		"tokuno_vendors_9_73", "tokuno_townslife_21_84", "tokuno_townslife_23_91"
	],
	"royal_city": [
		"termur_royal_city_1", "termur_royal_city_2", "termur_royal_city_3",
		"termur_royal_city_4", "termur_royal_city_5", "termur_royal_city_6",
		"termur_royal_city_7", "termur_royal_city_8", "termur_royal_city_9",
		"termur_royal_city_10", "termur_royal_city_11", "termur_royal_city_12",
		"termur_royal_city_13"
	],
	"lakeshire": [
		"ilsh_lakeshire_1", "ilsh_lakeshire_2", "ilsh_lakeshire_3", "ilsh_lakeshire_4",
		"ilsh_lakeshire_5", "ilsh_lakeshire_6", "ilsh_lakeshire_7", "ilsh_lakeshire_8"
	],
	"mistas": ["ilsh_mistas_1", "ilsh_mistas_2"],
	"montor": [
		"ilsh_montor_1", "ilsh_montor_2", "ilsh_montor_3", "ilsh_montor_4",
		"ilsh_montor_5", "ilsh_montor_6", "ilsh_montor_7", "ilsh_montor_8"
	],
	"gargoyle_city": [
		"ilsh_gargoyle_city_1", "ilsh_gargoyle_city_2", "ilsh_gargoyle_city_3",
		"ilsh_gargoyle_city_4", "ilsh_gargoyle_city_5", "ilsh_gargoyle_city_6"
	]
}

def do_respawn_region(reg_id):
	try:
		reg = wolfpack.spawnregion(reg_id)
		if reg:
			reg.respawn()
			return 1
	except Exception:
		pass
	return 0

def do_clear_region(reg_id):
	try:
		reg = wolfpack.spawnregion(reg_id)
		if reg:
			reg.clear()
			return 1
	except Exception:
		pass
	return 0

def respawntowns(socket, command, arguments):
	args = arguments.strip().split()

	if len(args) == 0:
		# Respawn em todas as cidades
		total = 0
		for tname, sps in TOWN_SPAWNREGIONS.items():
			for reg_id in sps:
				total += do_respawn_region(reg_id)
		socket.sysmessage(f"Todas as cidades respawnadas com sucesso! ({total} regioes preenchidas)")
		return True

	first = args[0].lower()

	if first == "list":
		towns = ", ".join(sorted(TOWN_SPAWNREGIONS.keys()))
		socket.sysmessage(f"Cidades disponiveis: {towns}")
		return True

	if first == "clear":
		if len(args) > 1:
			tname = args[1].lower()
			if tname in TOWN_SPAWNREGIONS:
				cleared = 0
				for reg_id in TOWN_SPAWNREGIONS[tname]:
					cleared += do_clear_region(reg_id)
				socket.sysmessage(f"Spawns de {tname} limpos ({cleared} regioes).")
			else:
				socket.sysmessage(f"Cidade '{tname}' nao encontrada. Digite 'respawntowns list.")
		else:
			cleared = 0
			for tname, sps in TOWN_SPAWNREGIONS.items():
				for reg_id in sps:
					cleared += do_clear_region(reg_id)
			socket.sysmessage(f"Todos os spawns de cidades foram limpos ({cleared} regioes).")
		return True

	# Respawn de cidade especifica
	tname = first
	if tname in TOWN_SPAWNREGIONS:
		count = 0
		for reg_id in TOWN_SPAWNREGIONS[tname]:
			count += do_respawn_region(reg_id)
		socket.sysmessage(f"Respawn concluido em {tname} ({count} regioes preenchidas).")
		return True
	else:
		socket.sysmessage(f"Cidade '{tname}' nao encontrada. Digite 'respawntowns list para ver a lista.")
		return True

def onLoad():
	wolfpack.registercommand("respawntowns", respawntowns)
	wolfpack.registercommand("respawn", respawntowns)
