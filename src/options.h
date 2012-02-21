/* File: options.h */

/* Purpose: option definitions */


/*** Option Definitions ***/

/*
 * Option indexes (offsets)
 *
 * These values are hard-coded by savefiles (and various pieces of code).
 */

#define OPT_MAX					256
#define OPT_PLAYER				192
#define OPT_BIRTH				32
#define OPT_SERVER				32

#define OPT_FLAG_BIRTH			0x01
#define OPT_FLAG_SERVER			0x02
#define OPT_FLAG_PLAYER			0x04


#define OPT_BIRTH_PAGE			6

/* Option set 0 */

#define rogue_like_commands		p_ptr->options[0]
#define quick_messages			TRUE
/* {TRUE,  0, NULL,					"Number 1" }, p_ptr->options[1] */
/* {TRUE,  0, NULL,					"Number 2" }, p_ptr->options[2] */
#define carry_query_flag		p_ptr->options[3]
#define use_old_target			p_ptr->options[4]
#define always_pickup			p_ptr->options[5]
/* {TRUE,  0, NULL,					"Number 6" }, p_ptr->options[6] */
#define depth_in_feet			p_ptr->options[7]
/* {TRUE,  0, NULL,					"Number 8" }, p_ptr->options[8] */
/* {FALSE, 0, NULL,					"Number 9" }, p_ptr->options[9] */
#define show_labels				p_ptr->options[10]
#define show_weights			TRUE
/* {TRUE,  0, NULL,					"Number 11" }, p_ptr->options[11] */
/* {TRUE,  0, NULL,					"Number 12" }, p_ptr->options[12] */
#define toggle_xp				p_ptr->options[13]
#define ring_bell				p_ptr->options[14]
#define use_color				p_ptr->options[15]
#define find_ignore_stairs		p_ptr->options[16]
#define find_ignore_doors		p_ptr->options[17]
/* {TRUE,  0, NULL,					"Number 18" }, p_ptr->options[18] */
#define find_examine			p_ptr->options[19]
/* {TRUE,  0, NULL,					"Number 20" }, p_ptr->options[20] */
#define disturb_near			p_ptr->options[21]
#define disturb_panel			TRUE
/* {TRUE,  0, NULL,					"Number 22" }, p_ptr->options[22] */
#define disturb_state			TRUE
/* {TRUE,  0, NULL,					"Number 23" }, p_ptr->options[23] */
#define disturb_minor			p_ptr->options[24]
#define disturb_other			TRUE
/* {TRUE,  0, NULL,					"Number 25" }, p_ptr->options[25] */
#define disturb_traps			p_ptr->options[26]
#define auto_more				p_ptr->options[27]
#define last_words				TRUE
/* {TRUE,  0, NULL,					"Number 28" }, p_ptr->options[28] */
#define speak_unique			TRUE
/* {TRUE,  0, NULL,					"Number 29" }, p_ptr->options[29] */
#define small_levels			svr_ptr->options[0]
#define dungeon_abyss			svr_ptr->options[1]

/* Option set 1 */

/* {TRUE,  0, NULL,					"Number 32" }, p_ptr->options[30] */
/* {TRUE,  0, NULL,					"Number 33" }, svr_ptr->options[2] */
/* {TRUE,  0, NULL,					"Number 34" }, svr_ptr->options[3] */
/* {TRUE,  0, NULL,					"Number 35" }, svr_ptr->options[4] */
/* {TRUE,  0, NULL, 				"Number 36" }, svr_ptr->options[5] */
#define expand_list				TRUE
#define destroy_batch			p_ptr->options[31]
#define view_torch_grids		p_ptr->options[32]
/* {TRUE,  0, NULL,					"Number 40" }, svr_ptr->options[7] */
#define dungeon_stair			svr_ptr->options[8]
/* {TRUE,  0, NULL,					"Number 42" }, svr_ptr->options[9] */
/* {TRUE,  0, NULL,					"Number 43" }, svr_ptr->options[10] */
/* {TRUE,  0, NULL,					"Number 44" }, svr_ptr->options[11] */
/* {TRUE,  0, NULL,					"Number 45" }, svr_ptr->options[12] */
#define smart_packs				!stupid_monsters
/* {TRUE,  0, NULL,					"Number 46" }, svr_ptr->options[13] */
/* {TRUE,  0, NULL,					"Number 47" }, svr_ptr->options[14] */
#define monster_theme			p_ptr->options[33]
/* {TRUE,  0, NULL,					"Number 49" }, p_ptr->options[34] */
/* {TRUE,  0, NULL,					"Number 50" }, p_ptr->options[35] */
/* {TRUE,  0, NULL,					"Number 51" }, p_ptr->options[36] */
#define flush_failure			p_ptr->options[37]
#define flush_disturb			p_ptr->options[38]
/* {FALSE, 0, NULL, 					"Number 54" }, p_ptr->options[39] */
#define fresh_before			p_ptr->options[40]
#define fresh_after				p_ptr->options[41]
#define emergency_stop			p_ptr->options[42]
/* {FALSE, 0, NULL,					"Number 57" }, p_ptr->options[42] */
#define compress_savefile		p_ptr->options[43]
#define hilite_player			p_ptr->options[44]
#define view_yellow_lite		p_ptr->options[45]
#define	view_bright_lite		p_ptr->options[46]
#define view_granite_lite		p_ptr->options[47]
#define	view_special_lite		p_ptr->options[48]

/* Option Set 2 */

#define	view_player_colour		p_ptr->options[49]
/* {TRUE,  0, NULL,					"Number 66" }, p_ptr->options[51] */
/* {TRUE,  0, NULL,					"Number 67" }, p_ptr->options[52] */
/* {TRUE,  0, NULL,					"Number 68" }, p_ptr->options[53] */
/* {TRUE,  0, NULL,					"Number 69" }, p_ptr->options[54] */
/* {TRUE,  0, NULL,					"Number 70" }, p_ptr->options[55] */
/* {TRUE,  0, NULL,					"Number 71" }, p_ptr->options[56] */
/* {TRUE,  0, NULL,					"Number 72" }, p_ptr->options[57] */
/* {TRUE,  0, NULL,					"Number 73" }, p_ptr->options[58] */
/* {TRUE,  0, NULL,					"Number 74" }, p_ptr->options[59] */
/* {TRUE,  0, NULL,					"Number 75" }, p_ptr->options[60] */
/* {TRUE,  0, NULL,					"Number 76" }, p_ptr->options[61] */
/* {TRUE,  0, NULL,					"Number 77" }, p_ptr->options[62] */
/* {TRUE,  0, NULL,					"Number 78" }, p_ptr->options[63] */
/* {TRUE,  0, NULL,					"Number 79" }, p_ptr->options[64] */
/* {TRUE,  0, NULL,					"Number 80" }, p_ptr->options[65] */
/* {TRUE,  0, NULL,					"Number 81" }, p_ptr->options[66] */
/* {TRUE,  0, NULL,					"Number 82" }, p_ptr->options[67] */
/* {TRUE,  0, NULL,					"Number 83" }, p_ptr->options[68] */
/* {TRUE,  0, NULL,					"Number 84" }, p_ptr->options[69] */
/* {TRUE,  0, NULL,					"Number 85" }, p_ptr->options[70] */
/* {TRUE,  0, NULL,					"Number 86" }, p_ptr->options[71] */
/* {TRUE,  0, NULL,					"Number 87" }, p_ptr->options[72] */
/* {TRUE,  0, NULL,					"Number 88" }, p_ptr->options[73] */
/* {TRUE,  0, NULL,					"Number 89" }, p_ptr->options[74] */
/* {TRUE,  0, NULL,					"Number 90" }, p_ptr->options[75] */
/* {TRUE,  0, NULL,					"Number 91" }, p_ptr->options[76] */
/* {TRUE,  0, NULL,					"Number 92" }, p_ptr->options[77] */
/* {TRUE,  0, NULL,					"Number 93" }, p_ptr->options[78] */
/* {TRUE,  0, NULL,					"Number 94" }, p_ptr->options[79] */
/* {TRUE,  0, NULL,					"Number 95" }, p_ptr->options[80] */

/* Option Set 3 */

#define study_warn				p_ptr->options[81]
/* {TRUE,  0, NULL,					"Number 97" }, p_ptr->options[82] */
/* {TRUE,  0, NULL,					"Number 98" }, p_ptr->options[83] */
/* {TRUE,  0, NULL,					"Number 99" }, p_ptr->options[84] */
/* {TRUE,  0, NULL,					"Number 100" }, p_ptr->options[85] */
/* {TRUE,  0, NULL,					"Number 101" }, p_ptr->options[86] */
/* {TRUE,  0, NULL,					"Number 102" }, p_ptr->options[87] */
/* {TRUE,  0, NULL,					"Number 103" }, p_ptr->options[88] */
/* {TRUE,  0, NULL,					"Number 104" }, p_ptr->options[89] */
/* {TRUE,  0, NULL,					"Number 105" }, p_ptr->options[90] */
/* {TRUE,  0, NULL,					"Number 106" }, p_ptr->options[91] */
/* {TRUE,  0, NULL,					"Number 107" }, p_ptr->options[92] */
/* {TRUE,  0, NULL,					"Number 108" }, p_ptr->options[93] */
/* {TRUE,  0, NULL,					"Number 109" }, p_ptr->options[94] */
/* {TRUE,  0, NULL,					"Number 110" }, p_ptr->options[95] */
/* {TRUE,  0, NULL,					"Number 111" }, p_ptr->options[96] */
/* {TRUE,  0, NULL,					"Number 112" }, p_ptr->options[97] */
/* {TRUE,  0, NULL,					"Number 113" }, p_ptr->options[98] */
/* {TRUE,  0, NULL,					"Number 114" }, p_ptr->options[99] */
/* {TRUE,  0, NULL,					"Number 115" }, p_ptr->options[100] */
/* {TRUE,  0, NULL,					"Number 116" }, p_ptr->options[101] */
/* {TRUE,  0, NULL,					"Number 117" }, p_ptr->options[102] */
/* {TRUE,  0, NULL,					"Number 118" }, p_ptr->options[103] */
/* {TRUE,  0, NULL,					"Number 119" }, p_ptr->options[104] */
/* {TRUE,  0, NULL,					"Number 120" }, p_ptr->options[105] */
/* {TRUE,  0, NULL,					"Number 121" }, p_ptr->options[106] */
/* {TRUE,  0, NULL,					"Number 122" }, p_ptr->options[107 */
/* {TRUE,  0, NULL,					"Number 123" }, p_ptr->options[108] */
/* {TRUE,  0, NULL,					"Number 124" }, p_ptr->options[109] */
/* {TRUE,  0, NULL,					"Number 125" }, p_ptr->options[110] */
/* {TRUE,  0, NULL,					"Number 126" }, p_ptr->options[111] */
/* {TRUE,  0, NULL,					"Number 127" }, p_ptr->options[112] */

/* Option Set 4 */

#define quiet_squelch			p_ptr->options[113]
/* {TRUE,  0, NULL,					"Number 129" },p_ptr->options[114] */
/* {TRUE,  0, NULL,					"Number 130" },p_ptr->options[115] */
/* {TRUE,  0, NULL,					"Number 131" },p_ptr->options[116] */
/* {TRUE,  0, NULL,					"Number 132" },p_ptr->options[117] */
/* {TRUE,  0, NULL,					"Number 133" },p_ptr->options[118] */
/* {TRUE,  0, NULL,					"Number 134" },p_ptr->options[119] */
/* {TRUE,  0, NULL,					"Number 135" },p_ptr->options[120] */
/* {TRUE,  0, NULL,					"Number 136" },p_ptr->options[121] */
/* {TRUE,  0, NULL,					"Number 137" },p_ptr->options[122] */
/* {TRUE,  0, NULL,					"Number 138" },p_ptr->options[123] */
/* {TRUE,  0, NULL,					"Number 139" },p_ptr->options[124] */
/* {TRUE,  0, NULL,					"Number 140" },p_ptr->options[125] */
/* {TRUE,  0, NULL,					"Number 141" },p_ptr->options[126] */
/* {TRUE,  0, NULL,					"Number 142" },p_ptr->options[127] */
/* {TRUE,  0, NULL,					"Number 143" },p_ptr->options[128] */
/* {TRUE,  0, NULL,					"Number 144" },p_ptr->options[129] */
/* {TRUE,  0, NULL,					"Number 145" },p_ptr->options[130] */
#define quick_destroy_bad		p_ptr->options[131]
#define quick_destroy_avg		p_ptr->options[132]
#define quick_destroy_good		p_ptr->options[133]
#define quick_destroy_all		p_ptr->options[134]
#define auto_destroy_chests		p_ptr->options[135]
#define auto_destroy_bad		p_ptr->options[136]
#define auto_destroy_weap		p_ptr->options[137]
#define auto_destroy_arm		p_ptr->options[138]
#define auto_destroy_cloak		p_ptr->options[139]
#define auto_destroy_shield		p_ptr->options[140]
#define auto_destroy_helm		p_ptr->options[141]
#define auto_destroy_gloves		p_ptr->options[142]
#define auto_destroy_boots		p_ptr->options[143]
#define auto_dump				p_ptr->options[144]


/* Option Set 5 */

/* {TRUE,  0, NULL,					"Number 160" }, p_ptr->options[145] */
#define	plain_descriptions		p_ptr->options[146]
#define stupid_monsters			p_ptr->birth[0]
/* {FALSE, 0, NULL,					"Number 165" }, p_ptr->options[147] */
#define confirm_wear			p_ptr->options[148]
/* {FALSE, 0, NULL,					"Number 165" }, p_ptr->options[149] */
#define easy_open				p_ptr->options[150]
#define easy_disarm				p_ptr->options[151]
#define easy_floor				p_ptr->options[152]
/* {TRUE,  0, NULL,					"Number 169" }, p_ptr->options[153] */
#define center_player			p_ptr->options[154]
#define avoid_center			p_ptr->options[155]
#define show_coords				p_ptr->options[156]
#define limit_messages			compress_savefile
/* {TRUE,  0, NULL,					"Number 173" }, p_ptr->options[157] */
#define check_transaction		p_ptr->options[158]
/* {TRUE,  0, NULL,					"Number 175" }, p_ptr->options[159] */
/* {TRUE,  0, NULL,					"Number 176" }, p_ptr->options[160] */
/* {TRUE,  0, NULL,					"Number 177" }, p_ptr->options[161] */
/* {TRUE,  0, NULL,					"Number 178" }, p_ptr->options[162] */
/* {TRUE,  0, NULL,					"Number 179" }, p_ptr->options[163] */
/* {TRUE,  0, NULL,					"Number 180" }, p_ptr->options[164] */
/* {TRUE,  0, NULL,					"Number 181" }, p_ptr->options[165] */
/* {TRUE,  0, NULL,					"Number 182" }, p_ptr->options[166] */
/* {TRUE,  0, NULL,					"Number 183" }, p_ptr->options[167] */
/* {TRUE,  0, NULL,					"Number 184" }, p_ptr->options[168] */
/* {TRUE,  0, NULL,					"Number 185" }, p_ptr->options[169] */
/* {TRUE,  0, NULL,					"Number 186" }, p_ptr->options[170] */
/* {TRUE,  0, NULL,					"Number 187" }, p_ptr->options[171] */
/* {TRUE,  0, NULL,					"Number 188" }, p_ptr->options[172] */
/* {TRUE,  0, NULL,					"Number 189" }, p_ptr->options[173] */
/* {TRUE,  0, NULL,					"Number 190" }, p_ptr->options[174] */
/* {TRUE,  0, NULL,					"Number 191" }, p_ptr->options[175] */

/* Option Set 6 */

#define vanilla_town			p_ptr->birth[1]
/* {TRUE,  0, NULL,					"Number 193" }, p_ptr->options[176] */
#define ironman_shops			p_ptr->birth[2]
#define ironman_small_levels    p_ptr->birth[3]
#define ironman_downward		p_ptr->birth[4]
#define cthulhu_monsters		p_ptr->birth[5]
#define amber_monsters			p_ptr->birth[6]
#define allow_randart			p_ptr->birth[7]
/* {TRUE,  0, NULL,					"Number 200" }, p_ptr->birth[8] */
/* {TRUE,  0, NULL,					"Number 201" }, p_ptr->birth[9] */
/* {TRUE,  0, NULL,					"Number 202" }, p_ptr->birth[10] */
#define munchkin_death			p_ptr->birth[11]
/* {TRUE,  0, NULL,					"Number 204" }, p_ptr->birth[12] */
/* {TRUE,  0, NULL,					"Number 205" }, p_ptr->birth[13] */
#define preserve_mode			p_ptr->birth[14]
#define autoroller				p_ptr->birth[15]
#define point_based				p_ptr->birth[16]
#define silly_monsters			p_ptr->birth[17]
#define ironman_nightmare		p_ptr->birth[18]
#define use_upkeep				FALSE  /* not allowing this option */
#define randart_rare			p_ptr->birth[19]
#define randart_weak			!randart_rare  /* one or the other. */
#define competition_mode		p_ptr->birth[20]
#define accessible_mode			p_ptr->birth[21]
/* {TRUE,  0, NULL,					"Number 215" }, p_ptr->birth[23] */
/* {TRUE,  0, NULL,					"Number 216" }, p_ptr->birth[24] */
/* {TRUE,  0, NULL,					"Number 217" }, p_ptr->birth[25] */
/* {TRUE,  0, NULL,					"Number 218" }, p_ptr->birth[26] */
/* {TRUE,  0, NULL,					"Number 219" }, p_ptr->birth[27] */
/* {TRUE,  0, NULL,					"Number 220" }, p_ptr->birth[28] */
/* {TRUE,  0, NULL,					"Number 221" }, p_ptr->birth[29] */
/* {TRUE,  0, NULL,					"Number 222" }, p_ptr->birth[30] */
/* {TRUE,  0, NULL,					"Number 223" }, p_ptr->birth[31] */

/* Option Set 7 */

/* {TRUE,  0, NULL,					"Number 224" },svr_ptr->options[15] */
#define monster_light			svr_ptr->options[16]
/* "Turn on muliplayer client - server code" , svr_ptr->options[17] */
/* {TRUE,  0, NULL,					"Number 227" },svr_ptr->options[18] */
/* {TRUE,  0, NULL,					"Number 228" },svr_ptr->options[19] */
/* {TRUE,  0, NULL,					"Number 229" },svr_ptr->options[20] */
/* {TRUE,  0, NULL,					"Number 230" },svr_ptr->options[21] */
/* {TRUE,  0, NULL,					"Number 231" },svr_ptr->options[22] */
/* {TRUE,  0, NULL,					"Number 232" },svr_ptr->options[23] */
/* {TRUE,  0, NULL,					"Number 233" },svr_ptr->options[24] */
/* {TRUE,  0, NULL,					"Number 234" },svr_ptr->options[25] */
/* {TRUE,  0, NULL,					"Number 235" },svr_ptr->options[26] */
/* {TRUE,  0, NULL,					"Number 236" },svr_ptr->options[27] */
/* {TRUE,  0, NULL,					"Number 237" },svr_ptr->options[28] */
/* {TRUE,  0, NULL,					"Number 238" },svr_ptr->options[29] */
/* {TRUE,  0, NULL,					"Number 239" },p_ptr->options[177] */
/* {TRUE,  0, NULL,					"Number 240" },p_ptr->options[178] */
/* {TRUE,  0, NULL,					"Number 241" },p_ptr->options[179] */
/* {TRUE,  0, NULL,					"Number 242" },p_ptr->options[180] */
/* {TRUE,  0, NULL,					"Number 243" },p_ptr->options[181] */
/* {TRUE,  0, NULL,					"Number 244" },p_ptr->options[182] */
/* {TRUE,  0, NULL,					"Number 245" },p_ptr->options[183] */
/* {TRUE,  0, NULL,					"Number 246" },p_ptr->options[184] */
/* {TRUE,  0, NULL,					"Number 247" },p_ptr->options[185] */
/* {TRUE,  0, NULL,					"Number 248" },p_ptr->options[186] */
/* {TRUE,  0, NULL,					"Number 249" },p_ptr->options[187] */
/* {TRUE,  0, NULL,					"Number 250" },p_ptr->options[188] */
#define auto_notes				p_ptr->options[189]
#define take_notes				TRUE
/* {TRUE,  0, NULL,					"Number 252" }, p_ptr->options[190] */
/* {TRUE,  0, NULL,					"Number 253" }, p_ptr->options[191] */
/* {TRUE,  0, NULL,					"Number 254" }, p_ptr->options[192] */
/* {TRUE,  0, NULL,					"Number 255" }, svr_ptr->options[31] */
