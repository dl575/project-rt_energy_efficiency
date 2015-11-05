/* Produced by CVXGEN, 2015-11-03 19:01:27 -0500.  */
/* CVXGEN is Copyright (C) 2006-2012 Jacob Mattingley, jem@cvxgen.com. */
/* The code in this file is Copyright (C) 2006-2012 Jacob Mattingley. */
/* CVXGEN, or solvers produced by CVXGEN, cannot be used for commercial */
/* applications without prior written permission from Jacob Mattingley. */

/* Filename: testsolver.c. */
/* Description: Basic test harness for solver.c. */
#include "solver.h"
Vars vars;
Params params;
Workspace work;
Settings settings;
#define NUMTESTS 0
int main(int argc, char **argv) {
  int num_iters;
#if (NUMTESTS > 0)
  int i;
  double time;
  double time_per;
#endif
  set_defaults();
  setup_indexing();
  load_default_data();
  /* Solve problem instance for the record. */
  settings.verbose = 1;
  num_iters = solve();
#ifndef ZERO_LIBRARY_MODE
#if (NUMTESTS > 0)
  /* Now solve multiple problem instances for timing purposes. */
  settings.verbose = 0;
  tic();
  for (i = 0; i < NUMTESTS; i++) {
    solve();
  }
  time = tocq();
  printf("Timed %d solves over %.3f seconds.\n", NUMTESTS, time);
  time_per = time / NUMTESTS;
  if (time_per > 1) {
    printf("Actual time taken per solve: %.3g s.\n", time_per);
  } else if (time_per > 1e-3) {
    printf("Actual time taken per solve: %.3g ms.\n", 1e3*time_per);
  } else {
    printf("Actual time taken per solve: %.3g us.\n", 1e6*time_per);
  }
#endif
#endif
  return 0;
}
void load_default_data(void) {
  params.xx[0] = 0.20319161029830202;
  params.xx[1] = 0.8325912904724193;
  params.xx[2] = -0.8363810443482227;
  params.xx[3] = 0.04331042079065206;
  params.xx[4] = 1.5717878173906188;
  params.xx[5] = 1.5851723557337523;
  params.xx[6] = -1.497658758144655;
  params.xx[7] = -1.171028487447253;
  params.xx[8] = -1.7941311867966805;
  params.xx[9] = -0.23676062539745413;
  params.xx[10] = -1.8804951564857322;
  params.xx[11] = -0.17266710242115568;
  params.xx[12] = 0.596576190459043;
  params.xx[13] = -0.8860508694080989;
  params.xx[14] = 0.7050196079205251;
  params.xx[15] = 0.3634512696654033;
  params.xx[16] = -1.9040724704913385;
  params.xx[17] = 0.23541635196352795;
  params.xx[18] = -0.9629902123701384;
  params.xx[19] = -0.3395952119597214;
  params.xx[20] = -0.865899672914725;
  params.xx[21] = 0.7725516732519853;
  params.xx[22] = -0.23818512931704205;
  params.xx[23] = -1.372529046100147;
  params.xx[24] = 0.17859607212737894;
  params.xx[25] = 1.1212590580454682;
  params.xx[26] = -0.774545870495281;
  params.xx[27] = -1.1121684642712744;
  params.xx[28] = -0.44811496977740495;
  params.xx[29] = 1.7455345994417217;
  params.xx[30] = 1.9039816898917352;
  params.xx[31] = 0.6895347036512547;
  params.xx[32] = 1.6113364341535923;
  params.xx[33] = 1.383003485172717;
  params.xx[34] = -0.48802383468444344;
  params.xx[35] = -1.631131964513103;
  params.xx[36] = 0.6136436100941447;
  params.xx[37] = 0.2313630495538037;
  params.xx[38] = -0.5537409477496875;
  params.xx[39] = -1.0997819806406723;
  params.xx[40] = -0.3739203344950055;
  params.xx[41] = -0.12423900520332376;
  params.xx[42] = -0.923057686995755;
  params.xx[43] = -0.8328289030982696;
  params.xx[44] = -0.16925440270808823;
  params.xx[45] = 1.442135651787706;
  params.xx[46] = 0.34501161787128565;
  params.xx[47] = -0.8660485502711608;
  params.xx[48] = -0.8880899735055947;
  params.xx[49] = -0.1815116979122129;
  params.xx[50] = -1.17835862158005;
  params.xx[51] = -1.1944851558277074;
  params.xx[52] = 0.05614023926976763;
  params.xx[53] = -1.6510825248767813;
  params.xx[54] = -0.06565787059365391;
  params.xx[55] = -0.5512951504486665;
  params.xx[56] = 0.8307464872626844;
  params.xx[57] = 0.9869848924080182;
  params.xx[58] = 0.7643716874230573;
  params.xx[59] = 0.7567216550196565;
  params.xx[60] = -0.5055995034042868;
  params.xx[61] = 0.6725392189410702;
  params.xx[62] = -0.6406053441727284;
  params.xx[63] = 0.29117547947550015;
  params.xx[64] = -0.6967713677405021;
  params.xx[65] = -0.21941980294587182;
  params.xx[66] = -1.753884276680243;
  params.xx[67] = -1.0292983112626475;
  params.xx[68] = 1.8864104246942706;
  params.xx[69] = -1.077663182579704;
  params.xx[70] = 0.7659100437893209;
  params.xx[71] = 0.6019074328549583;
  params.xx[72] = 0.8957565577499285;
  params.xx[73] = -0.09964555746227477;
  params.xx[74] = 0.38665509840745127;
  params.xx[75] = -1.7321223042686946;
  params.xx[76] = -1.7097514487110663;
  params.xx[77] = -1.2040958948116867;
  params.xx[78] = -1.3925560119658358;
  params.xx[79] = -1.5995826216742213;
  params.xx[80] = -1.4828245415645833;
  params.xx[81] = 0.21311092723061398;
  params.xx[82] = -1.248740700304487;
  params.xx[83] = 1.808404972124833;
  params.xx[84] = 0.7264471152297065;
  params.xx[85] = 0.16407869343908477;
  params.xx[86] = 0.8287224032315907;
  params.xx[87] = -0.9444533161899464;
  params.xx[88] = 1.7069027370149112;
  params.xx[89] = 1.3567722311998827;
  params.xx[90] = 0.9052779937121489;
  params.xx[91] = -0.07904017565835986;
  params.xx[92] = 1.3684127435065871;
  params.xx[93] = 0.979009293697437;
  params.xx[94] = 0.6413036255984501;
  params.xx[95] = 1.6559010680237511;
  params.xx[96] = 0.5346622551502991;
  params.xx[97] = -0.5362376605895625;
  params.xx[98] = 0.2113782926017822;
  params.xx[99] = -1.2144776931994525;
  params.xx[100] = -1.2317108144255875;
  params.xx[101] = 0.9026784957312834;
  params.xx[102] = 1.1397468137245244;
  params.xx[103] = 1.8883934547350631;
  params.xx[104] = 1.4038856681660068;
  params.xx[105] = 0.17437730638329096;
  params.xx[106] = -1.6408365219077408;
  params.xx[107] = -0.04450702153554875;
  params.xx[108] = 1.7117453902485025;
  params.xx[109] = 1.1504727980139053;
  params.xx[110] = -0.05962309578364744;
  params.xx[111] = -0.1788825540764547;
  params.xx[112] = -1.1280569263625857;
  params.xx[113] = -1.2911464767927057;
  params.xx[114] = -1.7055053231225696;
  params.xx[115] = 1.56957275034837;
  params.xx[116] = 0.5607064675962357;
  params.xx[117] = -1.4266707301147146;
  params.xx[118] = -0.3434923211351708;
  params.xx[119] = -1.8035643024085055;
  params.xx[120] = -1.1625066019105454;
  params.xx[121] = 0.9228324965161532;
  params.xx[122] = 0.6044910817663975;
  params.xx[123] = -0.0840868104920891;
  params.xx[124] = -0.900877978017443;
  params.xx[125] = 0.608892500264739;
  params.xx[126] = 1.8257980452695217;
  params.xx[127] = -0.25791777529922877;
  params.xx[128] = -1.7194699796493191;
  params.xx[129] = -1.7690740487081298;
  params.xx[130] = -1.6685159248097703;
  params.xx[131] = 1.8388287490128845;
  params.xx[132] = 0.16304334474597537;
  params.xx[133] = 1.3498497306788897;
  params.xx[134] = -1.3198658230514613;
  params.xx[135] = -0.9586197090843394;
  params.xx[136] = 0.7679100474913709;
  params.xx[137] = 1.5822813125679343;
  params.xx[138] = -0.6372460621593619;
  params.xx[139] = -1.741307208038867;
  params.xx[140] = 1.456478677642575;
  params.xx[141] = -0.8365102166820959;
  params.xx[142] = 0.9643296255982503;
  params.xx[143] = -1.367865381194024;
  params.xx[144] = 0.7798537405635035;
  params.xx[145] = 1.3656784761245926;
  params.xx[146] = 0.9086083149868371;
  params.xx[147] = -0.5635699005460344;
  params.xx[148] = 0.9067590059607915;
  params.xx[149] = -1.4421315032701587;
  params.xx[150] = -0.7447235390671119;
  params.xx[151] = -0.32166897326822186;
  params.xx[152] = 1.5088481557772684;
  params.xx[153] = -1.385039165715428;
  params.xx[154] = 1.5204991609972622;
  params.xx[155] = 1.1958572768832156;
  params.xx[156] = 1.8864971883119228;
  params.xx[157] = -0.5291880667861584;
  params.xx[158] = -1.1802409243688836;
  params.xx[159] = -1.037718718661604;
  params.xx[160] = 1.3114512056856835;
  params.xx[161] = 1.8609125943756615;
  params.xx[162] = 0.7952399935216938;
  params.xx[163] = -0.07001183290468038;
  params.xx[164] = -0.8518009412754686;
  params.xx[165] = 1.3347515373726386;
  params.xx[166] = 1.4887180335977037;
  params.xx[167] = -1.6314736327976336;
  params.xx[168] = -1.1362021159208933;
  params.xx[169] = 1.327044361831466;
  params.xx[170] = 1.3932155883179842;
  params.xx[171] = -0.7413880049440107;
  params.xx[172] = -0.8828216126125747;
  params.xx[173] = -0.27673991192616;
  params.xx[174] = 0.15778600105866714;
  params.xx[175] = -1.6177327399735457;
  params.xx[176] = 1.3476485548544606;
  params.xx[177] = 0.13893948140528378;
  params.xx[178] = 1.0998712601636944;
  params.xx[179] = -1.0766549376946926;
  params.xx[180] = 1.8611734044254629;
  params.xx[181] = 1.0041092292735172;
  params.xx[182] = -0.6276245424321543;
  params.xx[183] = 1.794110587839819;
  params.xx[184] = 0.8020471158650913;
  params.xx[185] = 1.362244341944948;
  params.xx[186] = -1.8180107765765245;
  params.xx[187] = -1.7774338357932473;
  params.xx[188] = 0.9709490941985153;
  params.xx[189] = -0.7812542682064318;
  params.xx[190] = 0.0671374633729811;
  params.xx[191] = -1.374950305314906;
  params.xx[192] = 1.9118096386279388;
  params.xx[193] = 0.011004190697677885;
  params.xx[194] = 1.3160043138989015;
  params.xx[195] = -1.7038488148800144;
  params.xx[196] = -0.08433819112864738;
  params.xx[197] = -1.7508820783768964;
  params.xx[198] = 1.536965724350949;
  params.xx[199] = -0.21675928514816478;
  params.xx[200] = -1.725800326952653;
  params.xx[201] = -1.6940148707361717;
  params.xx[202] = 0.15517063201268;
  params.xx[203] = -1.697734381979077;
  params.xx[204] = -1.264910727950229;
  params.xx[205] = -0.2545716633339441;
  params.xx[206] = -0.008868675926170244;
  params.xx[207] = 0.3332476609670296;
  params.xx[208] = 0.48205072561962936;
  params.xx[209] = -0.5087540014293261;
  params.xx[210] = 0.4749463319223195;
  params.xx[211] = -1.371021366459455;
  params.xx[212] = -0.8979660982652256;
  params.xx[213] = 1.194873082385242;
  params.xx[214] = -1.3876427970939353;
  params.xx[215] = -1.106708108457053;
  params.xx[216] = -1.0280872812241797;
  params.xx[217] = -0.08197078070773234;
  params.xx[218] = -1.9970179118324083;
  params.xx[219] = -1.878754557910134;
  params.xx[220] = -0.15380739340877803;
  params.xx[221] = -1.349917260533923;
  params.xx[222] = 0.7180072150931407;
  params.xx[223] = 1.1808183487065538;
  params.xx[224] = 0.31265343495084075;
  params.xx[225] = 0.7790599086928229;
  params.xx[226] = -0.4361679370644853;
  params.xx[227] = -1.8148151880282066;
  params.xx[228] = -0.24231386948140266;
  params.xx[229] = -0.5120787511622411;
  params.xx[230] = 0.3880129688013203;
  params.xx[231] = -1.4631273212038676;
  params.xx[232] = -1.0891484131126563;
  params.xx[233] = 1.2591296661091191;
  params.xx[234] = -0.9426978934391474;
  params.xx[235] = -0.358719180371347;
  params.xx[236] = 1.7438887059831263;
  params.xx[237] = -0.8977901479165817;
  params.xx[238] = -1.4188401645857445;
  params.xx[239] = 0.8080805173258092;
  params.xx[240] = 0.2682662017650985;
  params.xx[241] = 0.44637534218638786;
  params.xx[242] = -1.8318765960257055;
  params.xx[243] = -0.3309324209710929;
  params.xx[244] = -1.9829342633313622;
  params.xx[245] = -1.013858124556442;
  params.xx[246] = 0.8242247343360254;
  params.xx[247] = -1.753837136317201;
  params.xx[248] = -0.8212260055868805;
  params.xx[249] = 1.9524510112487126;
  params.xx[250] = 1.884888920907902;
  params.xx[251] = -0.0726144452811801;
  params.xx[252] = 0.9427735461129836;
  params.xx[253] = 0.5306230967445558;
  params.xx[254] = -0.1372277142250531;
  params.xx[255] = 1.4282657305652786;
  params.xx[256] = -1.309926991335284;
  params.xx[257] = 1.3137276889764422;
  params.xx[258] = -1.8317219061667278;
  params.xx[259] = 1.4678147672511939;
  params.xx[260] = 0.703986349872991;
  params.xx[261] = -0.2163435603565258;
  params.xx[262] = 0.6862809905371079;
  params.xx[263] = -0.15852598444303245;
  params.xx[264] = 1.1200128895143409;
  params.xx[265] = -1.5462236645435308;
  params.xx[266] = 0.0326297153944215;
  params.xx[267] = 1.4859581597754916;
  params.xx[268] = 1.71011710324809;
  params.xx[269] = -1.1186546738067493;
  params.xx[270] = -0.9922787897815244;
  params.xx[271] = 1.6160498864359547;
  params.xx[272] = -0.6179306451394861;
  params.xx[273] = -1.7725097038051376;
  params.xx[274] = 0.8595466884481313;
  params.xx[275] = -0.3423245633865686;
  params.xx[276] = 0.9412967499805762;
  params.xx[277] = -0.09163346622652258;
  params.xx[278] = 0.002262217745727657;
  params.xx[279] = -0.3297523583656421;
  params.xx[280] = -0.8380604158593941;
  params.xx[281] = 1.6028434695494038;
  params.xx[282] = 0.675150311940429;
  params.xx[283] = 1.1553293733718686;
  params.xx[284] = 1.5829581243724693;
  params.xx[285] = -0.9992442304425597;
  params.xx[286] = 1.6792824558896897;
  params.xx[287] = 1.4504203490342324;
  params.xx[288] = 0.02434104849994556;
  params.xx[289] = 0.27160869657612263;
  params.xx[290] = -1.5402710478528858;
  params.xx[291] = 1.0484633622310744;
  params.xx[292] = -1.3070999712627054;
  params.xx[293] = 0.13534416402363814;
  params.xx[294] = -1.4942507790851232;
  params.xx[295] = -1.708331625671371;
  params.xx[296] = 0.436109775042258;
  params.xx[297] = -0.03518748153727991;
  params.xx[298] = 0.6992397389570906;
  params.xx[299] = 1.1634167322171374;
  params.xx[300] = 1.9307499705822648;
  params.xx[301] = -1.6636772756932747;
  params.xx[302] = 0.5248484497343218;
  params.xx[303] = 0.30789958152579144;
  params.xx[304] = 0.602568707166812;
  params.xx[305] = 0.17271781925751872;
  params.xx[306] = 0.2294695501208066;
  params.xx[307] = 1.4742185345619543;
  params.xx[308] = -0.1919535345136989;
  params.xx[309] = 0.13990231452144553;
  params.xx[310] = 0.7638548150610602;
  params.xx[311] = -1.6420200344195646;
  params.xx[312] = -0.27229872445076087;
  params.xx[313] = -1.5914631171820468;
  params.xx[314] = -1.4487604283558668;
  params.xx[315] = -1.991497766136364;
  params.xx[316] = -1.1611742553535152;
  params.xx[317] = -1.133450950247063;
  params.xx[318] = 0.06497792493777155;
  params.xx[319] = 0.28083295396097263;
  params.xx[320] = 1.2958447220129887;
  params.xx[321] = -0.05315524470737154;
  params.xx[322] = 1.5658183956871667;
  params.xx[323] = -0.41975684089933685;
  params.xx[324] = 0.97844578833777;
  params.xx[325] = 0.2110290496695293;
  params.xx[326] = 0.4953003430893044;
  params.xx[327] = -0.9184320124667495;
  params.xx[328] = 1.750380031759156;
  params.xx[329] = 1.0786188614315915;
  params.xx[330] = -1.4176198837203735;
  params.xx[331] = 0.149737479778294;
  params.xx[332] = 1.9831452222223418;
  params.xx[333] = -1.8037746699794734;
  params.xx[334] = -0.7887206483295461;
  params.xx[335] = 0.9632534854086652;
  params.xx[336] = -1.8425542093895406;
  params.xx[337] = 0.986684363969033;
  params.xx[338] = 0.2936851199350441;
  params.xx[339] = 0.9268227022482662;
  params.xx[340] = 0.20333038350653299;
  params.xx[341] = 1.7576139132046351;
  params.xx[342] = -0.614393188398918;
  params.xx[343] = 0.297877839744912;
  params.xx[344] = -1.796880083990895;
  params.xx[345] = 0.21373133661742738;
  params.xx[346] = -0.32242822540825156;
  params.xx[347] = 1.9326471511608059;
  params.xx[348] = 1.7824292753481785;
  params.xx[349] = -1.4468823405675986;
  params.xx[350] = -1.8335374338761512;
  params.xx[351] = -1.5172997317243713;
  params.xx[352] = -1.229012129120719;
  params.xx[353] = 0.9046719772422094;
  params.xx[354] = 0.17591181415489432;
  params.xx[355] = 0.13970133814112584;
  params.xx[356] = -0.14185208214985234;
  params.xx[357] = -1.9732231264739348;
  params.xx[358] = -0.4301123458221334;
  params.xx[359] = 1.9957537650387742;
  params.xx[360] = 1.2811648216477893;
  params.xx[361] = 0.2914428437588219;
  params.xx[362] = -1.214148157218884;
  params.xx[363] = 1.6818776980374155;
  params.xx[364] = -0.30341101038214635;
  params.xx[365] = 0.47730909231793106;
  params.xx[366] = -1.187569373035299;
  params.xx[367] = -0.6877370247915531;
  params.xx[368] = -0.6201861482616171;
  params.xx[369] = -0.4209925183921568;
  params.xx[370] = -1.9110724537712471;
  params.xx[371] = 0.6413882087807936;
  params.xx[372] = -1.3200399280087032;
  params.xx[373] = 0.41320105301312626;
  params.xx[374] = 0.4783213861392275;
  params.xx[375] = 0.7916189857293743;
  params.xx[376] = -0.8322752558146558;
  params.xx[377] = -0.8318720537426154;
  params.xx[378] = 1.0221179076113445;
  params.xx[379] = -0.4471032189262627;
  params.xx[380] = -1.3901469561676985;
  params.xx[381] = 1.6210596051208572;
  params.xx[382] = -1.9476687601912737;
  params.xx[383] = 1.5459376306231292;
  params.xx[384] = -0.830972896191656;
  params.xx[385] = -0.47269983955176276;
  params.xx[386] = 1.913620609584223;
  params.xx[387] = -0.25329703423935124;
  params.xx[388] = 0.8635279149674653;
  params.xx[389] = -0.35046893227111564;
  params.xx[390] = 1.6541432486772365;
  params.xx[391] = 0.8779619968413503;
  params.xx[392] = -0.07723284625844862;
  params.xx[393] = -1.6631134040635196;
  params.xx[394] = -0.54546452868516;
  params.xx[395] = -0.03757319061095998;
  params.xx[396] = -0.864543266194465;
  params.xx[397] = 0.13856203767859343;
  params.xx[398] = -1.1613957272733684;
  params.xx[399] = -0.022681697832835024;
  params.yy[0] = 0.11202078062843634;
  params.yy[1] = 0.6934385624164641;
  params.yy[2] = 0.9814633803279791;
  params.yy[3] = 0.9198949681022897;
  params.yy[4] = -0.3035363988458051;
  params.yy[5] = -0.1761906755724203;
  params.yy[6] = 1.4940284058791686;
  params.yy[7] = -0.5488483097174393;
  params.yy[8] = 0.9521313238305416;
  params.yy[9] = 1.9762689267600413;
  params.yy[10] = 1.6992335341478482;
  params.yy[11] = 0.1969474711697119;
  params.yy[12] = -0.7795544525014559;
  params.yy[13] = 0.4892505434034007;
  params.yy[14] = 0.7372066729248594;
  params.yy[15] = 0.10784901966517557;
  params.yy[16] = -0.6340934767066218;
  params.yy[17] = -0.17829371464242083;
  params.yy[18] = -1.6728370279392784;
  params.yy[19] = -0.8348711800042916;
}
