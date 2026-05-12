/************** Begin file fts3_unicode2.c ***********************************/
/*
** 2012-05-25
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
*/

/*
** DO NOT EDIT THIS MACHINE GENERATED FILE.
*/

#ifndef SQLITE_DISABLE_FTS3_UNICODE
#if defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)

/* #include <assert.h> */

/*
** Return true if the argument corresponds to a unicode codepoint
** classified as either a letter or a number. Otherwise false.
**
** The results are undefined if the value passed to this function
** is less than zero.
*/
SQLITE_PRIVATE int sqlite3FtsUnicodeIsalnum(int c){
  /* Each unsigned integer in the following array corresponds to a contiguous
  ** range of unicode codepoints that are not either letters or numbers (i.e.
  ** codepoints for which this function should return 0).
  **
  ** The most significant 22 bits in each 32-bit value contain the first
  ** codepoint in the range. The least significant 10 bits are used to store
  ** the size of the range (always at least 1). In other words, the value
  ** ((C<<22) + N) represents a range of N codepoints starting with codepoint
  ** C. It is not possible to represent a range larger than 1023 codepoints
  ** using this format.
  */
  static const unsigned int aEntry[] = {
    0x00000030, 0x0000E807, 0x00016C06, 0x0001EC2F, 0x0002AC07,
    0x0002D001, 0x0002D803, 0x0002EC01, 0x0002FC01, 0x00035C01,
    0x0003DC01, 0x000B0804, 0x000B480E, 0x000B9407, 0x000BB401,
    0x000BBC81, 0x000DD401, 0x000DF801, 0x000E1002, 0x000E1C01,
    0x000FD801, 0x00120808, 0x00156806, 0x00162402, 0x00163C01,
    0x00164437, 0x0017CC02, 0x00180005, 0x00181816, 0x00187802,
    0x00192C15, 0x0019A804, 0x0019C001, 0x001B5001, 0x001B580F,
    0x001B9C07, 0x001BF402, 0x001C000E, 0x001C3C01, 0x001C4401,
    0x001CC01B, 0x001E980B, 0x001FAC09, 0x001FD804, 0x00205804,
    0x00206C09, 0x00209403, 0x0020A405, 0x0020C00F, 0x00216403,
    0x00217801, 0x0023901B, 0x00240004, 0x0024E803, 0x0024F812,
    0x00254407, 0x00258804, 0x0025C001, 0x00260403, 0x0026F001,
    0x0026F807, 0x00271C02, 0x00272C03, 0x00275C01, 0x00278802,
    0x0027C802, 0x0027E802, 0x00280403, 0x0028F001, 0x0028F805,
    0x00291C02, 0x00292C03, 0x00294401, 0x0029C002, 0x0029D401,
    0x002A0403, 0x002AF001, 0x002AF808, 0x002B1C03, 0x002B2C03,
    0x002B8802, 0x002BC002, 0x002C0403, 0x002CF001, 0x002CF807,
    0x002D1C02, 0x002D2C03, 0x002D5802, 0x002D8802, 0x002DC001,
    0x002E0801, 0x002EF805, 0x002F1803, 0x002F2804, 0x002F5C01,
    0x002FCC08, 0x00300403, 0x0030F807, 0x00311803, 0x00312804,
    0x00315402, 0x00318802, 0x0031FC01, 0x00320802, 0x0032F001,
    0x0032F807, 0x00331803, 0x00332804, 0x00335402, 0x00338802,
    0x00340802, 0x0034F807, 0x00351803, 0x00352804, 0x00355C01,
    0x00358802, 0x0035E401, 0x00360802, 0x00372801, 0x00373C06,
    0x00375801, 0x00376008, 0x0037C803, 0x0038C401, 0x0038D007,
    0x0038FC01, 0x00391C09, 0x00396802, 0x003AC401, 0x003AD006,
    0x003AEC02, 0x003B2006, 0x003C041F, 0x003CD00C, 0x003DC417,
    0x003E340B, 0x003E6424, 0x003EF80F, 0x003F380D, 0x0040AC14,
    0x00412806, 0x00415804, 0x00417803, 0x00418803, 0x00419C07,
    0x0041C404, 0x0042080C, 0x00423C01, 0x00426806, 0x0043EC01,
    0x004D740C, 0x004E400A, 0x00500001, 0x0059B402, 0x005A0001,
    0x005A6C02, 0x005BAC03, 0x005C4803, 0x005CC805, 0x005D4802,
    0x005DC802, 0x005ED023, 0x005F6004, 0x005F7401, 0x0060000F,
    0x0062A401, 0x0064800C, 0x0064C00C, 0x00650001, 0x00651002,
    0x0066C011, 0x00672002, 0x00677822, 0x00685C05, 0x00687802,
    0x0069540A, 0x0069801D, 0x0069FC01, 0x006A8007, 0x006AA006,
    0x006C0005, 0x006CD011, 0x006D6823, 0x006E0003, 0x006E840D,
    0x006F980E, 0x006FF004, 0x00709014, 0x0070EC05, 0x0071F802,
    0x00730008, 0x00734019, 0x0073B401, 0x0073C803, 0x00770027,
    0x0077F004, 0x007EF401, 0x007EFC03, 0x007F3403, 0x007F7403,
    0x007FB403, 0x007FF402, 0x00800065, 0x0081A806, 0x0081E805,
    0x00822805, 0x0082801A, 0x00834021, 0x00840002, 0x00840C04,
    0x00842002, 0x00845001, 0x00845803, 0x00847806, 0x00849401,
    0x00849C01, 0x0084A401, 0x0084B801, 0x0084E802, 0x00850005,
    0x00852804, 0x00853C01, 0x00864264, 0x00900027, 0x0091000B,
    0x0092704E, 0x00940200, 0x009C0475, 0x009E53B9, 0x00AD400A,
    0x00B39406, 0x00B3BC03, 0x00B3E404, 0x00B3F802, 0x00B5C001,
    0x00B5FC01, 0x00B7804F, 0x00B8C00C, 0x00BA001A, 0x00BA6C59,
    0x00BC00D6, 0x00BFC00C, 0x00C00005, 0x00C02019, 0x00C0A807,
    0x00C0D802, 0x00C0F403, 0x00C26404, 0x00C28001, 0x00C3EC01,
    0x00C64002, 0x00C6580A, 0x00C70024, 0x00C8001F, 0x00C8A81E,
    0x00C94001, 0x00C98020, 0x00CA2827, 0x00CB003F, 0x00CC0100,
    0x01370040, 0x02924037, 0x0293F802, 0x02983403, 0x0299BC10,
    0x029A7C01, 0x029BC008, 0x029C0017, 0x029C8002, 0x029E2402,
    0x02A00801, 0x02A01801, 0x02A02C01, 0x02A08C09, 0x02A0D804,
    0x02A1D004, 0x02A20002, 0x02A2D011, 0x02A33802, 0x02A38012,
    0x02A3E003, 0x02A4980A, 0x02A51C0D, 0x02A57C01, 0x02A60004,
    0x02A6CC1B, 0x02A77802, 0x02A8A40E, 0x02A90C01, 0x02A93002,
    0x02A97004, 0x02A9DC03, 0x02A9EC01, 0x02AAC001, 0x02AAC803,
    0x02AADC02, 0x02AAF802, 0x02AB0401, 0x02AB7802, 0x02ABAC07,
    0x02ABD402, 0x02AF8C0B, 0x03600001, 0x036DFC02, 0x036FFC02,
    0x037FFC01, 0x03EC7801, 0x03ECA401, 0x03EEC810, 0x03F4F802,
    0x03F7F002, 0x03F8001A, 0x03F88007, 0x03F8C023, 0x03F95013,
    0x03F9A004, 0x03FBFC01, 0x03FC040F, 0x03FC6807, 0x03FCEC06,
    0x03FD6C0B, 0x03FF8007, 0x03FFA007, 0x03FFE405, 0x04040003,
    0x0404DC09, 0x0405E411, 0x0406400C, 0x0407402E, 0x040E7C01,
    0x040F4001, 0x04215C01, 0x04247C01, 0x0424FC01, 0x04280403,
    0x04281402, 0x04283004, 0x0428E003, 0x0428FC01, 0x04294009,
    0x0429FC01, 0x042CE407, 0x04400003, 0x0440E016, 0x04420003,
    0x0442C012, 0x04440003, 0x04449C0E, 0x04450004, 0x04460003,
    0x0446CC0E, 0x04471404, 0x045AAC0D, 0x0491C004, 0x05BD442E,
    0x05BE3C04, 0x074000F6, 0x07440027, 0x0744A4B5, 0x07480046,
    0x074C0057, 0x075B0401, 0x075B6C01, 0x075BEC01, 0x075C5401,
    0x075CD401, 0x075D3C01, 0x075DBC01, 0x075E2401, 0x075EA401,
    0x075F0C01, 0x07BBC002, 0x07C0002C, 0x07C0C064, 0x07C2800F,
    0x07C2C40E, 0x07C3040F, 0x07C3440F, 0x07C4401F, 0x07C4C03C,
    0x07C5C02B, 0x07C7981D, 0x07C8402B, 0x07C90009, 0x07C94002,
    0x07CC0021, 0x07CCC006, 0x07CCDC46, 0x07CE0014, 0x07CE8025,
    0x07CF1805, 0x07CF8011, 0x07D0003F, 0x07D10001, 0x07D108B6,
    0x07D3E404, 0x07D4003E, 0x07D50004, 0x07D54018, 0x07D7EC46,
    0x07D9140B, 0x07DA0046, 0x07DC0074, 0x38000401, 0x38008060,
    0x380400F0,
  };
  static const unsigned int aAscii[4] = {
    0xFFFFFFFF, 0xFC00FFFF, 0xF8000001, 0xF8000001,
  };

  if( (unsigned int)c<128 ){
    return ( (aAscii[c >> 5] & ((unsigned int)1 << (c & 0x001F)))==0 );
  }else if( (unsigned int)c<(1<<22) ){
    unsigned int key = (((unsigned int)c)<<10) | 0x000003FF;
    int iRes = 0;
    int iHi = sizeof(aEntry)/sizeof(aEntry[0]) - 1;
    int iLo = 0;
    while( iHi>=iLo ){
      int iTest = (iHi + iLo) / 2;
      if( key >= aEntry[iTest] ){
        iRes = iTest;
        iLo = iTest+1;
      }else{
        iHi = iTest-1;
      }
    }
    assert( aEntry[0]<key );
    assert( key>=aEntry[iRes] );
    return (((unsigned int)c) >= ((aEntry[iRes]>>10) + (aEntry[iRes]&0x3FF)));
  }
  return 1;
}


/*
** If the argument is a codepoint corresponding to a lowercase letter
** in the ASCII range with a diacritic added, return the codepoint
** of the ASCII letter only. For example, if passed 235 - "LATIN
** SMALL LETTER E WITH DIAERESIS" - return 65 ("LATIN SMALL LETTER
** E"). The resuls of passing a codepoint that corresponds to an
** uppercase letter are undefined.
*/
static int remove_diacritic(int c, int bComplex){
  unsigned short aDia[] = {
        0,  1797,  1848,  1859,  1891,  1928,  1940,  1995,
     2024,  2040,  2060,  2110,  2168,  2206,  2264,  2286,
     2344,  2383,  2472,  2488,  2516,  2596,  2668,  2732,
     2782,  2842,  2894,  2954,  2984,  3000,  3028,  3336,
     3456,  3696,  3712,  3728,  3744,  3766,  3832,  3896,
     3912,  3928,  3944,  3968,  4008,  4040,  4056,  4106,
     4138,  4170,  4202,  4234,  4266,  4296,  4312,  4344,
     4408,  4424,  4442,  4472,  4488,  4504,  6148,  6198,
     6264,  6280,  6360,  6429,  6505,  6529, 61448, 61468,
    61512, 61534, 61592, 61610, 61642, 61672, 61688, 61704,
    61726, 61784, 61800, 61816, 61836, 61880, 61896, 61914,
    61948, 61998, 62062, 62122, 62154, 62184, 62200, 62218,
    62252, 62302, 62364, 62410, 62442, 62478, 62536, 62554,
    62584, 62604, 62640, 62648, 62656, 62664, 62730, 62766,
    62830, 62890, 62924, 62974, 63032, 63050, 63082, 63118,
    63182, 63242, 63274, 63310, 63368, 63390,
  };
#define HIBIT ((unsigned char)0x80)
  unsigned char aChar[] = {
    '\0',      'a',       'c',       'e',       'i',       'n',
    'o',       'u',       'y',       'y',       'a',       'c',
    'd',       'e',       'e',       'g',       'h',       'i',
    'j',       'k',       'l',       'n',       'o',       'r',
    's',       't',       'u',       'u',       'w',       'y',
    'z',       'o',       'u',       'a',       'i',       'o',
    'u',       'u'|HIBIT, 'a'|HIBIT, 'g',       'k',       'o',
    'o'|HIBIT, 'j',       'g',       'n',       'a'|HIBIT, 'a',
    'e',       'i',       'o',       'r',       'u',       's',
    't',       'h',       'a',       'e',       'o'|HIBIT, 'o',
    'o'|HIBIT, 'y',       '\0',      '\0',      '\0',      '\0',
    '\0',      '\0',      '\0',      '\0',      'a',       'b',
    'c'|HIBIT, 'd',       'd',       'e'|HIBIT, 'e',       'e'|HIBIT,
    'f',       'g',       'h',       'h',       'i',       'i'|HIBIT,
    'k',       'l',       'l'|HIBIT, 'l',       'm',       'n',
    'o'|HIBIT, 'p',       'r',       'r'|HIBIT, 'r',       's',
    's'|HIBIT, 't',       'u',       'u'|HIBIT, 'v',       'w',
    'w',       'x',       'y',       'z',       'h',       't',
    'w',       'y',       'a',       'a'|HIBIT, 'a'|HIBIT, 'a'|HIBIT,
    'e',       'e'|HIBIT, 'e'|HIBIT, 'i',       'o',       'o'|HIBIT,
    'o'|HIBIT, 'o'|HIBIT, 'u',       'u'|HIBIT, 'u'|HIBIT, 'y',
  };

  unsigned int key = (((unsigned int)c)<<3) | 0x00000007;
  int iRes = 0;
  int iHi = sizeof(aDia)/sizeof(aDia[0]) - 1;
  int iLo = 0;
  while( iHi>=iLo ){
    int iTest = (iHi + iLo) / 2;
    if( key >= aDia[iTest] ){
      iRes = iTest;
      iLo = iTest+1;
    }else{
      iHi = iTest-1;
    }
  }
  assert( key>=aDia[iRes] );
  if( bComplex==0 && (aChar[iRes] & 0x80) ) return c;
  return (c > (aDia[iRes]>>3) + (aDia[iRes]&0x07)) ? c : ((int)aChar[iRes] & 0x7F);
}


/*
** Return true if the argument interpreted as a unicode codepoint
** is a diacritical modifier character.
*/
SQLITE_PRIVATE int sqlite3FtsUnicodeIsdiacritic(int c){
  unsigned int mask0 = 0x08029FDF;
  unsigned int mask1 = 0x000361F8;
  if( c<768 || c>817 ) return 0;
  return (c < 768+32) ?
      (mask0 & ((unsigned int)1 << (c-768))) :
      (mask1 & ((unsigned int)1 << (c-768-32)));
}


/*
** Interpret the argument as a unicode codepoint. If the codepoint
** is an upper case character that has a lower case equivalent,
** return the codepoint corresponding to the lower case version.
** Otherwise, return a copy of the argument.
**
** The results are undefined if the value passed to this function
** is less than zero.
*/
SQLITE_PRIVATE int sqlite3FtsUnicodeFold(int c, int eRemoveDiacritic){
  /* Each entry in the following array defines a rule for folding a range
  ** of codepoints to lower case. The rule applies to a range of nRange
  ** codepoints starting at codepoint iCode.
  **
  ** If the least significant bit in flags is clear, then the rule applies
  ** to all nRange codepoints (i.e. all nRange codepoints are upper case and
  ** need to be folded). Or, if it is set, then the rule only applies to
  ** every second codepoint in the range, starting with codepoint C.
  **
  ** The 7 most significant bits in flags are an index into the aiOff[]
  ** array. If a specific codepoint C does require folding, then its lower
  ** case equivalent is ((C + aiOff[flags>>1]) & 0xFFFF).
  **
  ** The contents of this array are generated by parsing the CaseFolding.txt
  ** file distributed as part of the "Unicode Character Database". See
  ** http://www.unicode.org for details.
  */
  static const struct TableEntry {
    unsigned short iCode;
    unsigned char flags;
    unsigned char nRange;
  } aEntry[] = {
    {65, 14, 26},          {181, 64, 1},          {192, 14, 23},
    {216, 14, 7},          {256, 1, 48},          {306, 1, 6},
    {313, 1, 16},          {330, 1, 46},          {376, 116, 1},
    {377, 1, 6},           {383, 104, 1},         {385, 50, 1},
    {386, 1, 4},           {390, 44, 1},          {391, 0, 1},
    {393, 42, 2},          {395, 0, 1},           {398, 32, 1},
    {399, 38, 1},          {400, 40, 1},          {401, 0, 1},
    {403, 42, 1},          {404, 46, 1},          {406, 52, 1},
    {407, 48, 1},          {408, 0, 1},           {412, 52, 1},
    {413, 54, 1},          {415, 56, 1},          {416, 1, 6},
    {422, 60, 1},          {423, 0, 1},           {425, 60, 1},
    {428, 0, 1},           {430, 60, 1},          {431, 0, 1},
    {433, 58, 2},          {435, 1, 4},           {439, 62, 1},
    {440, 0, 1},           {444, 0, 1},           {452, 2, 1},
    {453, 0, 1},           {455, 2, 1},           {456, 0, 1},
    {458, 2, 1},           {459, 1, 18},          {478, 1, 18},
    {497, 2, 1},           {498, 1, 4},           {502, 122, 1},
    {503, 134, 1},         {504, 1, 40},          {544, 110, 1},
    {546, 1, 18},          {570, 70, 1},          {571, 0, 1},
    {573, 108, 1},         {574, 68, 1},          {577, 0, 1},
    {579, 106, 1},         {580, 28, 1},          {581, 30, 1},
    {582, 1, 10},          {837, 36, 1},          {880, 1, 4},
    {886, 0, 1},           {902, 18, 1},          {904, 16, 3},
    {908, 26, 1},          {910, 24, 2},          {913, 14, 17},
    {931, 14, 9},          {962, 0, 1},           {975, 4, 1},
    {976, 140, 1},         {977, 142, 1},         {981, 146, 1},
    {982, 144, 1},         {984, 1, 24},          {1008, 136, 1},
    {1009, 138, 1},        {1012, 130, 1},        {1013, 128, 1},
    {1015, 0, 1},          {1017, 152, 1},        {1018, 0, 1},
    {1021, 110, 3},        {1024, 34, 16},        {1040, 14, 32},
    {1120, 1, 34},         {1162, 1, 54},         {1216, 6, 1},
    {1217, 1, 14},         {1232, 1, 88},         {1329, 22, 38},
    {4256, 66, 38},        {4295, 66, 1},         {4301, 66, 1},
    {7680, 1, 150},        {7835, 132, 1},        {7838, 96, 1},
    {7840, 1, 96},         {7944, 150, 8},        {7960, 150, 6},
    {7976, 150, 8},        {7992, 150, 8},        {8008, 150, 6},
    {8025, 151, 8},        {8040, 150, 8},        {8072, 150, 8},
    {8088, 150, 8},        {8104, 150, 8},        {8120, 150, 2},
    {8122, 126, 2},        {8124, 148, 1},        {8126, 100, 1},
    {8136, 124, 4},        {8140, 148, 1},        {8152, 150, 2},
    {8154, 120, 2},        {8168, 150, 2},        {8170, 118, 2},
    {8172, 152, 1},        {8184, 112, 2},        {8186, 114, 2},
    {8188, 148, 1},        {8486, 98, 1},         {8490, 92, 1},
    {8491, 94, 1},         {8498, 12, 1},         {8544, 8, 16},
    {8579, 0, 1},          {9398, 10, 26},        {11264, 22, 47},
    {11360, 0, 1},         {11362, 88, 1},        {11363, 102, 1},
    {11364, 90, 1},        {11367, 1, 6},         {11373, 84, 1},
    {11374, 86, 1},        {11375, 80, 1},        {11376, 82, 1},
    {11378, 0, 1},         {11381, 0, 1},         {11390, 78, 2},
    {11392, 1, 100},       {11499, 1, 4},         {11506, 0, 1},
    {42560, 1, 46},        {42624, 1, 24},        {42786, 1, 14},
    {42802, 1, 62},        {42873, 1, 4},         {42877, 76, 1},
    {42878, 1, 10},        {42891, 0, 1},         {42893, 74, 1},
    {42896, 1, 4},         {42912, 1, 10},        {42922, 72, 1},
    {65313, 14, 26},
  };
  static const unsigned short aiOff[] = {
   1,     2,     8,     15,    16,    26,    28,    32,
   37,    38,    40,    48,    63,    64,    69,    71,
   79,    80,    116,   202,   203,   205,   206,   207,
   209,   210,   211,   213,   214,   217,   218,   219,
   775,   7264,  10792, 10795, 23228, 23256, 30204, 54721,
   54753, 54754, 54756, 54787, 54793, 54809, 57153, 57274,
   57921, 58019, 58363, 61722, 65268, 65341, 65373, 65406,
   65408, 65410, 65415, 65424, 65436, 65439, 65450, 65462,
   65472, 65476, 65478, 65480, 65482, 65488, 65506, 65511,
   65514, 65521, 65527, 65528, 65529,
  };

  int ret = c;

  assert( sizeof(unsigned short)==2 && sizeof(unsigned char)==1 );

  if( c<128 ){
    if( c>='A' && c<='Z' ) ret = c + ('a' - 'A');
  }else if( c<65536 ){
    const struct TableEntry *p;
    int iHi = sizeof(aEntry)/sizeof(aEntry[0]) - 1;
    int iLo = 0;
    int iRes = -1;

    assert( c>aEntry[0].iCode );
    while( iHi>=iLo ){
      int iTest = (iHi + iLo) / 2;
      int cmp = (c - aEntry[iTest].iCode);
      if( cmp>=0 ){
        iRes = iTest;
        iLo = iTest+1;
      }else{
        iHi = iTest-1;
      }
    }

    assert( iRes>=0 && c>=aEntry[iRes].iCode );
    p = &aEntry[iRes];
    if( c<(p->iCode + p->nRange) && 0==(0x01 & p->flags & (p->iCode ^ c)) ){
      ret = (c + (aiOff[p->flags>>1])) & 0x0000FFFF;
      assert( ret>0 );
    }

    if( eRemoveDiacritic ){
      ret = remove_diacritic(ret, eRemoveDiacritic==2);
    }
  }

  else if( c>=66560 && c<66600 ){
    ret = c + 40;
  }

  return ret;
}
#endif /* defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4) */
#endif /* !defined(SQLITE_DISABLE_FTS3_UNICODE) */

/************** End of fts3_unicode2.c ***************************************/
/************** Begin file json.c ********************************************/
/*
** 2015-08-12
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** SQLite JSON functions.
**
** This file began as an extension in ext/misc/json1.c in 2015.  That
** extension proved so useful that it has now been moved into the core.
**
** The original design stored all JSON as pure text, canonical RFC-8259.
** Support for JSON-5 extensions was added with version 3.42.0 (2023-05-16).
** All generated JSON text still conforms strictly to RFC-8259, but text
** with JSON-5 extensions is accepted as input.
**
** Beginning with version 3.45.0 (circa 2024-01-01), these routines also
** accept BLOB values that have JSON encoded using a binary representation
** called "JSONB".  The name JSONB comes from PostgreSQL, however the on-disk
** format for SQLite-JSONB is completely different and incompatible with
** PostgreSQL-JSONB.
**
** Decoding and interpreting JSONB is still O(N) where N is the size of
** the input, the same as text JSON.  However, the constant of proportionality
** for JSONB is much smaller due to faster parsing.  The size of each
** element in JSONB is encoded in its header, so there is no need to search
** for delimiters using persnickety syntax rules.  JSONB seems to be about
** 3x faster than text JSON as a result.  JSONB is also tends to be slightly
** smaller than text JSON, by 5% or 10%, but there are corner cases where
** JSONB can be slightly larger.  So you are not far mistaken to say that
** a JSONB blob is the same size as the equivalent RFC-8259 text.
**
**
** THE JSONB ENCODING:
**
** Every JSON element is encoded in JSONB as a header and a payload.
** The header is between 1 and 9 bytes in size.  The payload is zero
** or more bytes.
**
** The lower 4 bits of the first byte of the header determines the
** element type:
**
**    0:   NULL
**    1:   TRUE
**    2:   FALSE
**    3:   INT        -- RFC-8259 integer literal
**    4:   INT5       -- JSON5 integer literal
**    5:   FLOAT      -- RFC-8259 floating point literal
**    6:   FLOAT5     -- JSON5 floating point literal
**    7:   TEXT       -- Text literal acceptable to both SQL and JSON
**    8:   TEXTJ      -- Text containing RFC-8259 escapes
**    9:   TEXT5      -- Text containing JSON5 and/or RFC-8259 escapes
**   10:   TEXTRAW    -- Text containing unescaped syntax characters
**   11:   ARRAY
**   12:   OBJECT
**
** The other three possible values (13-15) are reserved for future
** enhancements.
**
** The upper 4 bits of the first byte determine the size of the header
** and sometimes also the size of the payload.  If X is the first byte
** of the element and if X>>4 is between 0 and 11, then the payload
** will be that many bytes in size and the header is exactly one byte
** in size.  Other four values for X>>4 (12-15) indicate that the header
** is more than one byte in size and that the payload size is determined
** by the remainder of the header, interpreted as a unsigned big-endian
** integer.
**
**   Value of X>>4         Size integer        Total header size
**   -------------     --------------------    -----------------
**        12           1 byte (0-255)                2
**        13           2 byte (0-65535)              3
**        14           4 byte (0-4294967295)         5
**        15           8 byte (0-1.8e19)             9
**
** The payload size need not be expressed in its minimal form.  For example,
** if the payload size is 10, the size can be expressed in any of 5 different
** ways: (1) (X>>4)==10, (2) (X>>4)==12 following by one 0x0a byte,
** (3) (X>>4)==13 followed by 0x00 and 0x0a, (4) (X>>4)==14 followed by
** 0x00 0x00 0x00 0x0a, or (5) (X>>4)==15 followed by 7 bytes of 0x00 and
** a single byte of 0x0a.  The shorter forms are preferred, of course, but
** sometimes when generating JSONB, the payload size is not known in advance
** and it is convenient to reserve sufficient header space to cover the
** largest possible payload size and then come back later and patch up
** the size when it becomes known, resulting in a non-minimal encoding.
**
** The value (X>>4)==15 is not actually used in the current implementation
** (as SQLite is currently unable to handle BLOBs larger than about 2GB)
** but is included in the design to allow for future enhancements.
**
** The payload follows the header.  NULL, TRUE, and FALSE have no payload and
** their payload size must always be zero.  The payload for INT, INT5,
** FLOAT, FLOAT5, TEXT, TEXTJ, TEXT5, and TEXTROW is text.  Note that the
** "..." or '...' delimiters are omitted from the various text encodings.
** The payload for ARRAY and OBJECT is a list of additional elements that
** are the content for the array or object.  The payload for an OBJECT
** must be an even number of elements.  The first element of each pair is
** the label and must be of type TEXT, TEXTJ, TEXT5, or TEXTRAW.
**
** A valid JSONB blob consists of a single element, as described above.
** Usually this will be an ARRAY or OBJECT element which has many more
** elements as its content.  But the overall blob is just a single element.
**
** Input validation for JSONB blobs simply checks that the element type
** code is between 0 and 12 and that the total size of the element
** (header plus payload) is the same as the size of the BLOB.  If those
** checks are true, the BLOB is assumed to be JSONB and processing continues.
** Errors are only raised if some other miscoding is discovered during
** processing.
**
** Additional information can be found in the doc/jsonb.md file of the
** canonical SQLite source tree.
*/
#ifndef SQLITE_OMIT_JSON
/* #include "sqliteInt.h" */

/* JSONB element types
*/
#define JSONB_NULL     0   /* "null" */
#define JSONB_TRUE     1   /* "true" */
#define JSONB_FALSE    2   /* "false" */
#define JSONB_INT      3   /* integer acceptable to JSON and SQL */
#define JSONB_INT5     4   /* integer in 0x000 notation */
#define JSONB_FLOAT    5   /* float acceptable to JSON and SQL */
#define JSONB_FLOAT5   6   /* float with JSON5 extensions */
#define JSONB_TEXT     7   /* Text compatible with both JSON and SQL */
#define JSONB_TEXTJ    8   /* Text with JSON escapes */
#define JSONB_TEXT5    9   /* Text with JSON-5 escape */
#define JSONB_TEXTRAW 10   /* SQL text that needs escaping for JSON */
#define JSONB_ARRAY   11   /* An array */
#define JSONB_OBJECT  12   /* An object */

/* Human-readable names for the JSONB values.  The index for each
** string must correspond to the JSONB_* integer above.
*/
static const char * const jsonbType[] = {
  "null", "true", "false", "integer", "integer",
  "real", "real", "text",  "text",    "text",
  "text", "array", "object", "", "", "", ""
};

/*
** Growing our own isspace() routine this way is twice as fast as
** the library isspace() function, resulting in a 7% overall performance
** increase for the text-JSON parser.  (Ubuntu14.10 gcc 4.8.4 x64 with -Os).
*/
static const char jsonIsSpace[] = {
#ifdef SQLITE_ASCII
/*0  1  2  3  4  5  6  7   8  9  a  b  c  d  e  f  */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 1, 1, 0, 0, 1, 0, 0,  /* 0 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 1 */
  1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 2 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 3 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 4 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 5 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 6 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 7 */

  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 8 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 9 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* a */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* b */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* c */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* d */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* e */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* f */
#endif
#ifdef SQLITE_EBCDIC
/*0  1  2  3  4  5  6  7   8  9  a  b  c  d  e  f  */
  0, 0, 0, 0, 0, 1, 0, 0,  0, 0, 0, 0, 0, 1, 0, 0,  /* 0 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 1 */
  0, 0, 0, 0, 0, 1, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 2 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 3 */
  1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 4 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 5 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 6 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 7 */

  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 8 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 9 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* a */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* b */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* c */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* d */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* e */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* f */
#endif

};
#define jsonIsspace(x) (jsonIsSpace[(unsigned char)x])

/*
** The set of all space characters recognized by jsonIsspace().
** Useful as the second argument to strspn().
*/
#ifdef SQLITE_ASCII
static const char jsonSpaces[] = "\011\012\015\040";
#endif
#ifdef SQLITE_EBCDIC
static const char jsonSpaces[] = "\005\045\015\100";
#endif


/*
** Characters that are special to JSON.  Control characters,
** '"' and '\\' and '\''.  Actually, '\'' is not special to
** canonical JSON, but it is special in JSON-5, so we include
** it in the set of special characters.
*/
static const char jsonIsOk[256] = {
#ifdef SQLITE_ASCII
/*0  1  2  3  4  5  6  7   8  9  a  b  c  d  e  f  */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 0 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 1 */
  1, 1, 0, 1, 1, 1, 1, 0,  1, 1, 1, 1, 1, 1, 1, 1,  /* 2 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* 3 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* 4 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 0, 1, 1, 1,  /* 5 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* 6 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* 7 */

  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* 8 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* 9 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* a */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* b */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* c */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* d */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* e */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1   /* f */
#endif
#ifdef SQLITE_EBCDIC
/*0  1  2  3  4  5  6  7   8  9  a  b  c  d  e  f  */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 0 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 1 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 2 */
  1, 1, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 0,  /* 3 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* 4 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* 5 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* 6 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 0, 1, 0,  /* 7 */

  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* 8 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* 9 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* a */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* b */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* c */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* d */
  0, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* e */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1   /* f */
#endif
};

/* Objects */
typedef struct JsonCache JsonCache;
typedef struct JsonString JsonString;
typedef struct JsonParse JsonParse;

/*
** Magic number used for the JSON parse cache in sqlite3_get_auxdata()
*/
#define JSON_CACHE_ID    (-429938)  /* Cache entry */
#define JSON_CACHE_SIZE  4          /* Max number of cache entries */

/*
** jsonUnescapeOneChar() returns this invalid code point if it encounters
** a syntax error.
*/
#define JSON_INVALID_CHAR 0x99999

/* A cache mapping JSON text into JSONB blobs.
**
** Each cache entry is a JsonParse object with the following restrictions:
**
**    *   The bReadOnly flag must be set
**
**    *   The aBlob[] array must be owned by the JsonParse object.  In other
**        words, nBlobAlloc must be non-zero.
**
**    *   eEdit and delta must be zero.
**
**    *   zJson must be an RCStr.  In other words bJsonIsRCStr must be true.
*/
struct JsonCache {
  sqlite3 *db;                    /* Database connection */
  int nUsed;                      /* Number of active entries in the cache */
  JsonParse *a[JSON_CACHE_SIZE];  /* One line for each cache entry */
};

/* An instance of this object represents a JSON string
** under construction.  Really, this is a generic string accumulator
** that can be and is used to create strings other than JSON.
**
** If the generated string is longer than will fit into the zSpace[] buffer,
** then it will be an RCStr string.  This aids with caching of large
** JSON strings.
*/
struct JsonString {
  sqlite3_context *pCtx;   /* Function context - put error messages here */
  char *zBuf;              /* Append JSON content here */
  u64 nAlloc;              /* Bytes of storage available in zBuf[] */
  u64 nUsed;               /* Bytes of zBuf[] currently used */
  u8 bStatic;              /* True if zBuf is static space */
  u8 eErr;                 /* True if an error has been encountered */
  char zSpace[100];        /* Initial static space */
};

/* Allowed values for JsonString.eErr */
#define JSTRING_OOM         0x01   /* Out of memory */
#define JSTRING_MALFORMED   0x02   /* Malformed JSONB */
#define JSTRING_TOODEEP     0x04   /* JSON nested too deep */
#define JSTRING_ERR         0x08   /* Error already sent to sqlite3_result */

/* The "subtype" set for text JSON values passed through using
** sqlite3_result_subtype() and sqlite3_value_subtype().
*/
#define JSON_SUBTYPE  74    /* Ascii for "J" */

/*
** Bit values for the flags passed into various SQL function implementations
** via the sqlite3_user_data() value.
*/
#define JSON_JSON      0x01        /* Result is always JSON */
#define JSON_SQL       0x02        /* Result is always SQL */
#define JSON_ABPATH    0x03        /* Allow abbreviated JSON path specs */
#define JSON_ISSET     0x04        /* json_set(), not json_insert() */
#define JSON_AINS      0x08        /* json_array_insert(), not json_insert() */
#define JSON_BLOB      0x10        /* Use the BLOB output format */

#define JSON_INSERT_TYPE(X)  (((X)&0xC)>>2)


/* A parsed JSON value.  Lifecycle:
**
**   1.  JSON comes in and is parsed into a JSONB value in aBlob.  The
**       original text is stored in zJson.  This step is skipped if the
**       input is JSONB instead of text JSON.
**
**   2.  The aBlob[] array is searched using the JSON path notation, if needed.
**
**   3.  Zero or more changes are made to aBlob[] (via json_remove() or
**       json_replace() or json_patch() or similar).
**
**   4.  New JSON text is generated from the aBlob[] for output.  This step
**       is skipped if the function is one of the jsonb_* functions that
**       returns JSONB instead of text JSON.
*/
struct JsonParse {
  u8 *aBlob;         /* JSONB representation of JSON value */
  u32 nBlob;         /* Bytes of aBlob[] actually used */
  u32 nBlobAlloc;    /* Bytes allocated to aBlob[].  0 if aBlob is external */
  char *zJson;       /* Json text used for parsing */
  sqlite3 *db;       /* The database connection to which this object belongs */
  int nJson;         /* Length of the zJson string in bytes */
  u32 nJPRef;        /* Number of references to this object */
  u32 iErr;          /* Error location in zJson[] */
  u16 iDepth;        /* Nesting depth */
  u8 nErr;           /* Number of errors seen */
  u8 oom;            /* Set to true if out of memory */
  u8 bJsonIsRCStr;   /* True if zJson is an RCStr */
  u8 hasNonstd;      /* True if input uses non-standard features like JSON5 */
  u8 bReadOnly;      /* Do not modify. */
  /* Search and edit information.  See jsonLookupStep() */
  u8 eEdit;          /* Edit operation to apply */
  int delta;         /* Size change due to the edit */
  u32 nIns;          /* Number of bytes to insert */
  u32 iLabel;        /* Location of label if search landed on an object value */
  u8 *aIns;          /* Content to be inserted */
};

/* Allowed values for JsonParse.eEdit */
#define JEDIT_DEL   1   /* Delete if exists */
#define JEDIT_REPL  2   /* Overwrite if exists */
#define JEDIT_INS   3   /* Insert if not exists */
#define JEDIT_SET   4   /* Insert or overwrite */
#define JEDIT_AINS  5   /* array_insert() */

/*
** Maximum nesting depth of JSON for this implementation.
**
** This limit is needed to avoid a stack overflow in the recursive
** descent parser.  A depth of 1000 is far deeper than any sane JSON
** should go.  Historical note: This limit was 2000 prior to version 3.42.0
*/
#ifndef SQLITE_JSON_MAX_DEPTH
# define JSON_MAX_DEPTH  1000
#else
# define JSON_MAX_DEPTH SQLITE_JSON_MAX_DEPTH
#endif

/*
** Allowed values for the flgs argument to jsonParseFuncArg();
*/
#define JSON_EDITABLE  0x01   /* Generate a writable JsonParse object */
#define JSON_KEEPERROR 0x02   /* Return non-NULL even if there is an error */

/**************************************************************************
** Forward references
**************************************************************************/
static void jsonReturnStringAsBlob(JsonString*);
static int jsonArgIsJsonb(sqlite3_value *pJson, JsonParse *p);
static u32 jsonTranslateBlobToText(JsonParse*,u32,JsonString*);
static void jsonReturnParse(sqlite3_context*,JsonParse*);
static JsonParse *jsonParseFuncArg(sqlite3_context*,sqlite3_value*,u32);
static void jsonParseFree(JsonParse*);
static u32 jsonbPayloadSize(const JsonParse*, u32, u32*);
static u32 jsonUnescapeOneChar(const char*, u32, u32*);

/**************************************************************************
** Utility routines for dealing with JsonCache objects
**************************************************************************/

/*
** Free a JsonCache object.
*/
static void jsonCacheDelete(JsonCache *p){
  int i;
  for(i=0; i<p->nUsed; i++){
    jsonParseFree(p->a[i]);
  }
  sqlite3DbFree(p->db, p);
}
static void jsonCacheDeleteGeneric(void *p){
  jsonCacheDelete((JsonCache*)p);
}

/*
** Insert a new entry into the cache.  If the cache is full, expel
** the least recently used entry.  Return SQLITE_OK on success or a
** result code otherwise.
**
** Cache entries are stored in age order, oldest first.
*/
static int jsonCacheInsert(
  sqlite3_context *ctx,   /* The SQL statement context holding the cache */
  JsonParse *pParse       /* The parse object to be added to the cache */
){
  JsonCache *p;

  assert( pParse->zJson!=0 );
  assert( pParse->bJsonIsRCStr );
  assert( pParse->delta==0 );
  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  if( p==0 ){
    sqlite3 *db = sqlite3_context_db_handle(ctx);
    p = sqlite3DbMallocZero(db, sizeof(*p));
    if( p==0 ) return SQLITE_NOMEM;
    p->db = db;
    sqlite3_set_auxdata(ctx, JSON_CACHE_ID, p, jsonCacheDeleteGeneric);
    p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
    if( p==0 ) return SQLITE_NOMEM;
  }
  if( p->nUsed >= JSON_CACHE_SIZE ){
    jsonParseFree(p->a[0]);
    memmove(p->a, &p->a[1], (JSON_CACHE_SIZE-1)*sizeof(p->a[0]));
    p->nUsed = JSON_CACHE_SIZE-1;
  }
  assert( pParse->nBlobAlloc>0 );
  pParse->eEdit = 0;
  pParse->nJPRef++;
  pParse->bReadOnly = 1;
  p->a[p->nUsed] = pParse;
  p->nUsed++;
  return SQLITE_OK;
}

/*
** Search for a cached translation the json text supplied by pArg.  Return
** the JsonParse object if found.  Return NULL if not found.
**
** When a match if found, the matching entry is moved to become the
** most-recently used entry if it isn't so already.
**
** The JsonParse object returned still belongs to the Cache and might
** be deleted at any moment.  If the caller wants the JsonParse to
** linger, it needs to increment the nPJRef reference counter.
*/
static JsonParse *jsonCacheSearch(
  sqlite3_context *ctx,    /* The SQL statement context holding the cache */
  sqlite3_value *pArg      /* Function argument containing SQL text */
){
  JsonCache *p;
  int i;
  const char *zJson;
  int nJson;

  if( sqlite3_value_type(pArg)!=SQLITE_TEXT ){
    return 0;
  }
  zJson = (const char*)sqlite3_value_text(pArg);
  if( zJson==0 ) return 0;
  nJson = sqlite3_value_bytes(pArg);

  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  if( p==0 ){
    return 0;
  }
  for(i=0; i<p->nUsed; i++){
    if( p->a[i]->zJson==zJson ) break;
  }
  if( i>=p->nUsed ){
    for(i=0; i<p->nUsed; i++){
      if( p->a[i]->nJson!=nJson ) continue;
      if( memcmp(p->a[i]->zJson, zJson, nJson)==0 ) break;
    }
  }
  if( i<p->nUsed ){
    if( i<p->nUsed-1 ){
      /* Make the matching entry the most recently used entry */
      JsonParse *tmp = p->a[i];
      memmove(&p->a[i], &p->a[i+1], (p->nUsed-i-1)*sizeof(tmp));
      p->a[p->nUsed-1] = tmp;
      i = p->nUsed - 1;
    }
    assert( p->a[i]->delta==0 );
    return p->a[i];
  }else{
    return 0;
  }
}

/**************************************************************************
** Utility routines for dealing with JsonString objects
**************************************************************************/

/* Turn uninitialized bulk memory into a valid JsonString object
** holding a zero-length string.
*/
static void jsonStringZero(JsonString *p){
  p->zBuf = p->zSpace;
  p->nAlloc = sizeof(p->zSpace);
  p->nUsed = 0;
  p->bStatic = 1;
}

/* Initialize the JsonString object
*/
static void jsonStringInit(JsonString *p, sqlite3_context *pCtx){
  p->pCtx = pCtx;
  p->eErr = 0;
  jsonStringZero(p);
}

/* Free all allocated memory and reset the JsonString object back to its
** initial state.
*/
static void jsonStringReset(JsonString *p){
  if( !p->bStatic ) sqlite3RCStrUnref(p->zBuf);
  jsonStringZero(p);
}

/* Report an out-of-memory (OOM) condition
*/
static void jsonStringOom(JsonString *p){
  p->eErr |= JSTRING_OOM;
  if( p->pCtx ) sqlite3_result_error_nomem(p->pCtx);
  jsonStringReset(p);
}

/* Report JSON nested too deep
*/
static void jsonStringTooDeep(JsonString *p){
  p->eErr |= JSTRING_TOODEEP;
  assert( p->pCtx!=0 );
  sqlite3_result_error(p->pCtx, "JSON nested too deep", -1);
  jsonStringReset(p);
}

/* Enlarge pJson->zBuf so that it can hold at least N more bytes.
** Return zero on success.  Return non-zero on an OOM error
*/
static int jsonStringGrow(JsonString *p, u32 N){
  u64 nTotal = N<p->nAlloc ? p->nAlloc*2 : p->nAlloc+N+10;
  char *zNew;
  if( p->bStatic ){
    if( p->eErr ) return 1;
    zNew = sqlite3RCStrNew(nTotal);
    if( zNew==0 ){
      jsonStringOom(p);
      return SQLITE_NOMEM;
    }
    memcpy(zNew, p->zBuf, (size_t)p->nUsed);
    p->zBuf = zNew;
    p->bStatic = 0;
  }else{
    p->zBuf = sqlite3RCStrResize(p->zBuf, nTotal);
    if( p->zBuf==0 ){
      p->eErr |= JSTRING_OOM;
      jsonStringZero(p);
      return SQLITE_NOMEM;
    }
  }
  p->nAlloc = nTotal;
  return SQLITE_OK;
}

/* Append N bytes from zIn onto the end of the JsonString string.
*/
static SQLITE_NOINLINE void jsonStringExpandAndAppend(
  JsonString *p,
  const char *zIn,
  u32 N
){
  assert( N>0 );
  if( jsonStringGrow(p,N) ) return;
  memcpy(p->zBuf+p->nUsed, zIn, N);
  p->nUsed += N;
}
static void jsonAppendRaw(JsonString *p, const char *zIn, u32 N){
  if( N==0 ) return;
  if( N+p->nUsed >= p->nAlloc ){
    jsonStringExpandAndAppend(p,zIn,N);
  }else{
    memcpy(p->zBuf+p->nUsed, zIn, N);
    p->nUsed += N;
  }
}
static void jsonAppendRawNZ(JsonString *p, const char *zIn, u32 N){
  assert( N>0 );
  if( N+p->nUsed >= p->nAlloc ){
    jsonStringExpandAndAppend(p,zIn,N);
  }else{
    memcpy(p->zBuf+p->nUsed, zIn, N);
    p->nUsed += N;
  }
}

/* Append formatted text (not to exceed N bytes) to the JsonString.
*/
static void jsonPrintf(int N, JsonString *p, const char *zFormat, ...){
  va_list ap;
  if( (p->nUsed + N >= p->nAlloc) && jsonStringGrow(p, N) ) return;
  va_start(ap, zFormat);
  sqlite3_vsnprintf(N, p->zBuf+p->nUsed, zFormat, ap);
  va_end(ap);
  p->nUsed += (int)strlen(p->zBuf+p->nUsed);
}

/* Append a single character
*/
static SQLITE_NOINLINE void jsonAppendCharExpand(JsonString *p, char c){
  if( jsonStringGrow(p,1) ) return;
  p->zBuf[p->nUsed++] = c;
}
static void jsonAppendChar(JsonString *p, char c){
  if( p->nUsed>=p->nAlloc ){
    jsonAppendCharExpand(p,c);
  }else{
    p->zBuf[p->nUsed++] = c;
  }
}

/* Remove a single character from the end of the string
*/
static void jsonStringTrimOneChar(JsonString *p){
  if( p->eErr==0 ){
    assert( p->nUsed>0 );
    p->nUsed--;
  }
}


/* Make sure there is a zero terminator on p->zBuf[]
**
** Return true on success.  Return false if an OOM prevents this
** from happening.
*/
static int jsonStringTerminate(JsonString *p){
  jsonAppendChar(p, 0);
  jsonStringTrimOneChar(p);
  return p->eErr==0;
}

/* Append a comma separator to the output buffer, if the previous
** character is not '[' or '{'.
*/
static void jsonAppendSeparator(JsonString *p){
  char c;
  if( p->nUsed==0 ) return;
  c = p->zBuf[p->nUsed-1];
  if( c=='[' || c=='{' ) return;
  jsonAppendChar(p, ',');
}

/* c is a control character.  Append the canonical JSON representation
** of that control character to p.
**
** This routine assumes that the output buffer has already been enlarged
** sufficiently to hold the worst-case encoding plus a nul terminator.
*/
static void jsonAppendControlChar(JsonString *p, u8 c){
  static const char aSpecial[] = {
     0, 0, 0, 0, 0, 0, 0, 0, 'b', 't', 'n', 0, 'f', 'r', 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,   0,   0,   0, 0,   0,   0, 0, 0
  };
  assert( sizeof(aSpecial)==32 );
  assert( aSpecial['\b']=='b' );
  assert( aSpecial['\f']=='f' );
  assert( aSpecial['\n']=='n' );
  assert( aSpecial['\r']=='r' );
  assert( aSpecial['\t']=='t' );
  assert( c>=0 && c<sizeof(aSpecial) );
  assert( p->nUsed+7 <= p->nAlloc );
  if( aSpecial[c] ){
    p->zBuf[p->nUsed] = '\\';
    p->zBuf[p->nUsed+1] = aSpecial[c];
    p->nUsed += 2;
  }else{
    p->zBuf[p->nUsed] = '\\';
    p->zBuf[p->nUsed+1] = 'u';
    p->zBuf[p->nUsed+2] = '0';
    p->zBuf[p->nUsed+3] = '0';
    p->zBuf[p->nUsed+4] = "0123456789abcdef"[c>>4];
    p->zBuf[p->nUsed+5] = "0123456789abcdef"[c&0xf];
    p->nUsed += 6;
  }
}

/* Append the N-byte string in zIn to the end of the JsonString string
** under construction.  Enclose the string in double-quotes ("...") and
** escape any double-quotes or backslash characters contained within the
** string.
**
** This routine is a high-runner.  There is a measurable performance
** increase associated with unwinding the jsonIsOk[] loop.
*/
static void jsonAppendString(JsonString *p, const char *zIn, u32 N){
  u32 k;
  u8 c;
  const u8 *z = (const u8*)zIn;
  if( z==0 ) return;
  if( (N+p->nUsed+2 >= p->nAlloc) && jsonStringGrow(p,N+2)!=0 ) return;
  p->zBuf[p->nUsed++] = '"';
  while( 1 /*exit-by-break*/ ){
    k = 0;
    /* The following while() is the 4-way unwound equivalent of
    **
    **     while( k<N && jsonIsOk[z[k]] ){ k++; }
    */
    while( 1 /* Exit by break */ ){
      if( k+3>=N ){
        while( k<N && jsonIsOk[z[k]] ){ k++; }
        break;
      }
      if( !jsonIsOk[z[k]] ){
        break;
      }
      if( !jsonIsOk[z[k+1]] ){
        k += 1;
        break;
      }
      if( !jsonIsOk[z[k+2]] ){
        k += 2;
        break;
      }
      if( !jsonIsOk[z[k+3]] ){
        k += 3;
        break;
      }else{
        k += 4;
      }
    }
    if( k>=N ){
      if( k>0 ){
        memcpy(&p->zBuf[p->nUsed], z, k);
        p->nUsed += k;
      }
      break;
    }
    if( k>0 ){
      memcpy(&p->zBuf[p->nUsed], z, k);
      p->nUsed += k;
      z += k;
      N -= k;
    }
    c = z[0];
    if( c=='"' || c=='\\' ){
      if( (p->nUsed+N+3 > p->nAlloc) && jsonStringGrow(p,N+3)!=0 ) return;
      p->zBuf[p->nUsed++] = '\\';
      p->zBuf[p->nUsed++] = c;
    }else if( c=='\'' ){
      p->zBuf[p->nUsed++] = c;
    }else{
      if( (p->nUsed+N+7 > p->nAlloc) && jsonStringGrow(p,N+7)!=0 ) return;
      jsonAppendControlChar(p, c);
    }
    z++;
    N--;
  }
  p->zBuf[p->nUsed++] = '"';
  assert( p->nUsed<p->nAlloc );
}

/*
** Append an sqlite3_value (such as a function parameter) to the JSON
** string under construction in p.
*/
static void jsonAppendSqlValue(
  JsonString *p,                 /* Append to this JSON string */
  sqlite3_value *pValue          /* Value to append */
){
  switch( sqlite3_value_type(pValue) ){
    case SQLITE_NULL: {
      jsonAppendRawNZ(p, "null", 4);
      break;
    }
    case SQLITE_FLOAT: {
      jsonPrintf(100, p, "%!0.15g", sqlite3_value_double(pValue));
      break;
    }
    case SQLITE_INTEGER: {
      const char *z = (const char*)sqlite3_value_text(pValue);
      u32 n = (u32)sqlite3_value_bytes(pValue);
      jsonAppendRaw(p, z, n);
      break;
    }
    case SQLITE_TEXT: {
      const char *z = (const char*)sqlite3_value_text(pValue);
      u32 n = (u32)sqlite3_value_bytes(pValue);
      if( sqlite3_value_subtype(pValue)==JSON_SUBTYPE ){
        jsonAppendRaw(p, z, n);
      }else{
        jsonAppendString(p, z, n);
      }
      break;
    }
    default: {
      JsonParse px;
      memset(&px, 0, sizeof(px));
      if( jsonArgIsJsonb(pValue, &px) ){
        jsonTranslateBlobToText(&px, 0, p);
      }else if( p->eErr==0 ){
        sqlite3_result_error(p->pCtx, "JSON cannot hold BLOB values", -1);
        p->eErr = JSTRING_ERR;
        jsonStringReset(p);
      }
      break;
    }
  }
}

/* Make the text in p (which is probably a generated JSON text string)
** the result of the SQL function.
**
** The JsonString is reset.
**
** If pParse and ctx are both non-NULL, then the SQL string in p is
** loaded into the zJson field of the pParse object as a RCStr and the
** pParse is added to the cache.
*/
static void jsonReturnString(
  JsonString *p,            /* String to return */
  JsonParse *pParse,        /* JSONB source or NULL */
  sqlite3_context *ctx      /* Where to cache */
){
  assert( (pParse!=0)==(ctx!=0) );
  assert( ctx==0 || ctx==p->pCtx );
  jsonStringTerminate(p);
  if( p->eErr==0 ){
    int flags = SQLITE_PTR_TO_INT(sqlite3_user_data(p->pCtx));
    if( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(p);
    }else if( p->bStatic ){
      sqlite3_result_text64(p->pCtx, p->zBuf, p->nUsed,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
    }else{
      if( pParse && pParse->bJsonIsRCStr==0 && pParse->nBlobAlloc>0 ){
        int rc;
        pParse->zJson = sqlite3RCStrRef(p->zBuf);
        pParse->nJson = p->nUsed;
        pParse->bJsonIsRCStr = 1;
        rc = jsonCacheInsert(ctx, pParse);
        if( rc==SQLITE_NOMEM ){
          sqlite3_result_error_nomem(ctx);
          jsonStringReset(p);
          return;
        }
      }
      sqlite3_result_text64(p->pCtx, sqlite3RCStrRef(p->zBuf), p->nUsed,
                            sqlite3RCStrUnref,
                            SQLITE_UTF8);
    }
  }else if( p->eErr & JSTRING_OOM ){
    sqlite3_result_error_nomem(p->pCtx);
  }else if( p->eErr & JSTRING_TOODEEP ){
    /* error already in p->pCtx */
  }else if( p->eErr & JSTRING_MALFORMED ){
    sqlite3_result_error(p->pCtx, "malformed JSON", -1);
  }
  jsonStringReset(p);
}

/**************************************************************************
** Utility routines for dealing with JsonParse objects
**************************************************************************/

/*
** Reclaim all memory allocated by a JsonParse object.  But do not
** delete the JsonParse object itself.
*/
static void jsonParseReset(JsonParse *pParse){
  assert( pParse->nJPRef<=1 );
  if( pParse->bJsonIsRCStr ){
    sqlite3RCStrUnref(pParse->zJson);
    pParse->zJson = 0;
    pParse->nJson = 0;
    pParse->bJsonIsRCStr = 0;
  }
  if( pParse->nBlobAlloc ){
    sqlite3DbFree(pParse->db, pParse->aBlob);
    pParse->aBlob = 0;
    pParse->nBlob = 0;
    pParse->nBlobAlloc = 0;
  }
}

/*
** Decrement the reference count on the JsonParse object.  When the
** count reaches zero, free the object.
*/
static void jsonParseFree(JsonParse *pParse){
  if( pParse ){
    if( pParse->nJPRef>1 ){
      pParse->nJPRef--;
    }else{
      jsonParseReset(pParse);
      sqlite3DbFree(pParse->db, pParse);
    }
  }
}

/**************************************************************************
** Utility routines for the JSON text parser
**************************************************************************/

/*
** Translate a single byte of Hex into an integer.
** This routine only gives a correct answer if h really is a valid hexadecimal
** character:  0..9a..fA..F.  But unlike sqlite3HexToInt(), it does not
** assert() if the digit is not hex.
*/
static u8 jsonHexToInt(int h){
#ifdef SQLITE_ASCII
  h += 9*(1&(h>>6));
#endif
#ifdef SQLITE_EBCDIC
  h += 9*(1&~(h>>4));
#endif
  return (u8)(h & 0xf);
}

/*
** Convert a 4-byte hex string into an integer
*/
static u32 jsonHexToInt4(const char *z){
  u32 v;
  v = (jsonHexToInt(z[0])<<12)
    + (jsonHexToInt(z[1])<<8)
    + (jsonHexToInt(z[2])<<4)
    + jsonHexToInt(z[3]);
  return v;
}

/*
** Return true if z[] begins with 2 (or more) hexadecimal digits
*/
static int jsonIs2Hex(const char *z){
  return sqlite3Isxdigit(z[0]) && sqlite3Isxdigit(z[1]);
}

/*
** Return true if z[] begins with 4 (or more) hexadecimal digits
*/
static int jsonIs4Hex(const char *z){
  return jsonIs2Hex(z) && jsonIs2Hex(&z[2]);
}

/*
** Return the number of bytes of JSON5 whitespace at the beginning of
** the input string z[].
**
** JSON5 whitespace consists of any of the following characters:
**
**    Unicode  UTF-8         Name
**    U+0009   09            horizontal tab
**    U+000a   0a            line feed
**    U+000b   0b            vertical tab
**    U+000c   0c            form feed
**    U+000d   0d            carriage return
**    U+0020   20            space
**    U+00a0   c2 a0         non-breaking space
**    U+1680   e1 9a 80      ogham space mark
**    U+2000   e2 80 80      en quad
**    U+2001   e2 80 81      em quad
**    U+2002   e2 80 82      en space
**    U+2003   e2 80 83      em space
**    U+2004   e2 80 84      three-per-em space
**    U+2005   e2 80 85      four-per-em space
**    U+2006   e2 80 86      six-per-em space
**    U+2007   e2 80 87      figure space
**    U+2008   e2 80 88      punctuation space
**    U+2009   e2 80 89      thin space
**    U+200a   e2 80 8a      hair space
**    U+2028   e2 80 a8      line separator
**    U+2029   e2 80 a9      paragraph separator
**    U+202f   e2 80 af      narrow no-break space (NNBSP)
**    U+205f   e2 81 9f      medium mathematical space (MMSP)
**    U+3000   e3 80 80      ideographical space
**    U+FEFF   ef bb bf      byte order mark
**
** In addition, comments between '/', '*' and '*', '/' and
** from '/', '/' to end-of-line are also considered to be whitespace.
*/
static int json5Whitespace(const char *zIn){
  int n = 0;
  const u8 *z = (u8*)zIn;
  while( 1 /*exit by "goto whitespace_done"*/ ){
    switch( z[n] ){
      case 0x09:
      case 0x0a:
      case 0x0b:
      case 0x0c:
      case 0x0d:
      case 0x20: {
        n++;
        break;
      }
      case '/': {
        if( z[n+1]=='*' && z[n+2]!=0 ){
          int j;
          for(j=n+3; z[j]!='/' || z[j-1]!='*'; j++){
            if( z[j]==0 ) goto whitespace_done;
          }
          n = j+1;
          break;
        }else if( z[n+1]=='/' ){
          int j;
          char c;
          for(j=n+2; (c = z[j])!=0; j++){
            if( c=='\n' || c=='\r' ) break;
            if( 0xe2==(u8)c && 0x80==(u8)z[j+1]
             && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2])
            ){
              j += 2;
              break;
            }
          }
          n = j;
          if( z[n] ) n++;
          break;
        }
        goto whitespace_done;
      }
      case 0xc2: {
        if( z[n+1]==0xa0 ){
          n += 2;
          break;
        }
        goto whitespace_done;
      }
      case 0xe1: {
        if( z[n+1]==0x9a && z[n+2]==0x80 ){
          n += 3;
          break;
        }
        goto whitespace_done;
      }
      case 0xe2: {
        if( z[n+1]==0x80 ){
          u8 c = z[n+2];
          if( c<0x80 ) goto whitespace_done;
          if( c<=0x8a || c==0xa8 || c==0xa9 || c==0xaf ){
            n += 3;
            break;
          }
        }else if( z[n+1]==0x81 && z[n+2]==0x9f ){
          n += 3;
          break;
        }
        goto whitespace_done;
      }
      case 0xe3: {
        if( z[n+1]==0x80 && z[n+2]==0x80 ){
          n += 3;
          break;
        }
        goto whitespace_done;
      }
      case 0xef: {
        if( z[n+1]==0xbb && z[n+2]==0xbf ){
          n += 3;
          break;
        }
        goto whitespace_done;
      }
      default: {
        goto whitespace_done;
      }
    }
  }
  whitespace_done:
  return n;
}

/*
** Extra floating-point literals to allow in JSON.
*/
static const struct NanInfName {
  char c1;
  char c2;
  char n;
  char eType;
  char nRepl;
  char *zMatch;
  char *zRepl;
} aNanInfName[] = {
  { 'i', 'I', 3, JSONB_FLOAT, 7, "inf", "9.0e999" },
  { 'i', 'I', 8, JSONB_FLOAT, 7, "infinity", "9.0e999" },
  { 'n', 'N', 3, JSONB_NULL, 4, "NaN", "null" },
  { 'q', 'Q', 4, JSONB_NULL, 4, "QNaN", "null" },
  { 's', 'S', 4, JSONB_NULL, 4, "SNaN", "null" },
};


/*
** Report the wrong number of arguments for json_insert(), json_replace()
** or json_set().
*/
static void jsonWrongNumArgs(
  sqlite3_context *pCtx,
  const char *zFuncName
){
  char *zMsg = sqlite3_mprintf("json_%s() needs an odd number of arguments",
                               zFuncName);
  sqlite3_result_error(pCtx, zMsg, -1);
  sqlite3_free(zMsg);
}

/****************************************************************************
** Utility routines for dealing with the binary BLOB representation of JSON
****************************************************************************/

/*
** Expand pParse->aBlob so that it holds at least N bytes.
**
** Return the number of errors.
*/
static int jsonBlobExpand(JsonParse *pParse, u32 N){
  u8 *aNew;
  u64 t;
  assert( N>pParse->nBlobAlloc );
  if( pParse->nBlobAlloc==0 ){
    t = 100;
  }else{
    t = pParse->nBlobAlloc*2;
  }
  if( t<N ) t = N+100;
  aNew = sqlite3DbRealloc(pParse->db, pParse->aBlob, t);
  if( aNew==0 ){ pParse->oom = 1; return 1; }
  assert( t<0x7fffffff );
  pParse->aBlob = aNew;
  pParse->nBlobAlloc = (u32)t;
  return 0;
}

/*
** If pParse->aBlob is not previously editable (because it is taken
** from sqlite3_value_blob(), as indicated by the fact that
** pParse->nBlobAlloc==0 and pParse->nBlob>0) then make it editable
** by making a copy into space obtained from malloc.
**
** Return true on success.  Return false on OOM.
*/
static int jsonBlobMakeEditable(JsonParse *pParse, u32 nExtra){
  u8 *aOld;
  u32 nSize;
  assert( !pParse->bReadOnly );
  if( pParse->oom ) return 0;
  if( pParse->nBlobAlloc>0 ) return 1;
  aOld = pParse->aBlob;
  nSize = pParse->nBlob + nExtra;
  pParse->aBlob = 0;
  if( jsonBlobExpand(pParse, nSize) ){
    return 0;
  }
  assert( pParse->nBlobAlloc >= pParse->nBlob + nExtra );
  memcpy(pParse->aBlob, aOld, pParse->nBlob);
  return 1;
}

/* Expand pParse->aBlob and append one bytes.
*/
static SQLITE_NOINLINE void jsonBlobExpandAndAppendOneByte(
  JsonParse *pParse,
  u8 c
){
  jsonBlobExpand(pParse, pParse->nBlob+1);
  if( pParse->oom==0 ){
    assert( pParse->nBlob+1<=pParse->nBlobAlloc );
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Append a single character.
*/
static void jsonBlobAppendOneByte(JsonParse *pParse, u8 c){
  if( pParse->nBlob >= pParse->nBlobAlloc ){
    jsonBlobExpandAndAppendOneByte(pParse, c);
  }else{
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Slow version of jsonBlobAppendNode() that first resizes the
** pParse->aBlob structure.
*/
static void jsonBlobAppendNode(JsonParse*,u8,u64,const void*);
static SQLITE_NOINLINE void jsonBlobExpandAndAppendNode(
  JsonParse *pParse,
  u8 eType,
  u64 szPayload,
  const void *aPayload
){
  if( jsonBlobExpand(pParse, pParse->nBlob+szPayload+9) ) return;
  jsonBlobAppendNode(pParse, eType, szPayload, aPayload);
}


/* Append a node type byte together with the payload size and
** possibly also the payload.
**
** If aPayload is not NULL, then it is a pointer to the payload which
** is also appended.  If aPayload is NULL, the pParse->aBlob[] array
** is resized (if necessary) so that it is big enough to hold the
** payload, but the payload is not appended and pParse->nBlob is left
** pointing to where the first byte of payload will eventually be.
*/
static void jsonBlobAppendNode(
  JsonParse *pParse,          /* The JsonParse object under construction */
  u8 eType,                   /* Node type.  One of JSONB_* */
  u64 szPayload,              /* Number of bytes of payload */
  const void *aPayload        /* The payload.  Might be NULL */
){
  u8 *a;
  if( pParse->nBlob+szPayload+9 > pParse->nBlobAlloc ){
    jsonBlobExpandAndAppendNode(pParse,eType,szPayload,aPayload);
    return;
  }
  assert( pParse->aBlob!=0 );
  a = &pParse->aBlob[pParse->nBlob];
  if( szPayload<=11 ){
    a[0] = eType | (szPayload<<4);
    pParse->nBlob += 1;
  }else if( szPayload<=0xff ){
    a[0] = eType | 0xc0;
    a[1] = szPayload & 0xff;
    pParse->nBlob += 2;
  }else if( szPayload<=0xffff ){
    a[0] = eType | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
    pParse->nBlob += 3;
  }else{
    a[0] = eType | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
    pParse->nBlob += 5;
  }
  if( aPayload ){
    pParse->nBlob += szPayload;
    memcpy(&pParse->aBlob[pParse->nBlob-szPayload], aPayload, szPayload);
  }
}

/* Change the payload size for the node at index i to be szPayload.
*/
static int jsonBlobChangePayloadSize(
  JsonParse *pParse,
  u32 i,
  u32 szPayload
){
  u8 *a;
  u8 szType;
  u8 nExtra;
  u8 nNeeded;
  int delta;
  if( pParse->oom ) return 0;
  a = &pParse->aBlob[i];
  szType = a[0]>>4;
  if( szType<=11 ){
    nExtra = 0;
  }else if( szType==12 ){
    nExtra = 1;
  }else if( szType==13 ){
    nExtra = 2;
  }else if( szType==14 ){
    nExtra = 4;
  }else{
    nExtra = 8;
  }
  if( szPayload<=11 ){
    nNeeded = 0;
  }else if( szPayload<=0xff ){
    nNeeded = 1;
  }else if( szPayload<=0xffff ){
    nNeeded = 2;
  }else{
    nNeeded = 4;
  }
  delta = nNeeded - nExtra;
  if( delta ){
    u32 newSize = pParse->nBlob + delta;
    if( delta>0 ){
      if( newSize>pParse->nBlobAlloc && jsonBlobExpand(pParse, newSize) ){
        return 0;  /* OOM error.  Error state recorded in pParse->oom. */
      }
      a = &pParse->aBlob[i];
      memmove(&a[1+delta], &a[1], pParse->nBlob - (i+1));
    }else{
      memmove(&a[1], &a[1-delta], pParse->nBlob - (i+1-delta));
    }
    pParse->nBlob = newSize;
  }
  if( nNeeded==0 ){
    a[0] = (a[0] & 0x0f) | (szPayload<<4);
  }else if( nNeeded==1 ){
    a[0] = (a[0] & 0x0f) | 0xc0;
    a[1] = szPayload & 0xff;
  }else if( nNeeded==2 ){
    a[0] = (a[0] & 0x0f) | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
  }else{
    a[0] = (a[0] & 0x0f) | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
  }
  return delta;
}

/*
** If z[0] is 'u' and is followed by exactly 4 hexadecimal character,
** then set *pOp to JSONB_TEXTJ and return true.  If not, do not make
** any changes to *pOp and return false.
*/
static int jsonIs4HexB(const char *z, int *pOp){
  if( z[0]!='u' ) return 0;
  if( !jsonIs4Hex(&z[1]) ) return 0;
  *pOp = JSONB_TEXTJ;
  return 1;
}

/*
** Check a single element of the JSONB in pParse for validity.
**
** The element to be checked starts at offset i and must end at on the
** last byte before iEnd.
**
** Return 0 if everything is correct.  Return the 1-based byte offset of the
** error if a problem is detected.  (In other words, if the error is at offset
** 0, return 1).
*/
static u32 jsonbValidityCheck(
  const JsonParse *pParse,    /* Input JSONB.  Only aBlob and nBlob are used */
  u32 i,                      /* Start of element as pParse->aBlob[i] */
  u32 iEnd,                   /* One more than the last byte of the element */
  u32 iDepth                  /* Current nesting depth */
){
  u32 n, sz, j, k;
  const u8 *z;
  u8 x;
  if( iDepth>JSON_MAX_DEPTH ) return i+1;
  sz = 0;
  n = jsonbPayloadSize(pParse, i, &sz);
  if( NEVER(n==0) ) return i+1;          /* Checked by caller */
  if( NEVER(i+n+sz!=iEnd) ) return i+1;  /* Checked by caller */
  z = pParse->aBlob;
  x = z[i] & 0x0f;
  switch( x ){
    case JSONB_NULL:
    case JSONB_TRUE:
    case JSONB_FALSE: {
      return n+sz==1 ? 0 : i+1;
    }
    case JSONB_INT: {
      if( sz<1 ) return i+1;
      j = i+n;
      if( z[j]=='-' ){
        j++;
        if( sz<2 ) return i+1;
      }
      k = i+n+sz;
      while( j<k ){
        if( sqlite3Isdigit(z[j]) ){
          j++;
        }else{
          return j+1;
        }
      }
      return 0;
    }
    case JSONB_INT5: {
      if( sz<3 ) return i+1;
      j = i+n;
      if( z[j]=='-' ){
        if( sz<4 ) return i+1;
        j++;
      }
      if( z[j]!='0' ) return i+1;
      if( z[j+1]!='x' && z[j+1]!='X' ) return j+2;
      j += 2;
      k = i+n+sz;
      while( j<k ){
        if( sqlite3Isxdigit(z[j]) ){
          j++;
        }else{
          return j+1;
        }
      }
      return 0;
    }
    case JSONB_FLOAT:
    case JSONB_FLOAT5: {
      u8 seen = 0;   /* 0: initial.  1: '.' seen  2: 'e' seen */
      if( sz<2 ) return i+1;
      j = i+n;
      k = j+sz;
      if( z[j]=='-' ){
        j++;
        if( sz<3 ) return i+1;
      }
      if( z[j]=='.' ){
        if( x==JSONB_FLOAT ) return j+1;
        if( !sqlite3Isdigit(z[j+1]) ) return j+1;
        j += 2;
        seen = 1;
      }else if( z[j]=='0' && x==JSONB_FLOAT ){
        if( j+3>k ) return j+1;
        if( z[j+1]!='.' && z[j+1]!='e' && z[j+1]!='E' ) return j+1;
        j++;
      }
      for(; j<k; j++){
        if( sqlite3Isdigit(z[j]) ) continue;
        if( z[j]=='.' ){
          if( seen>0 ) return j+1;
          if( x==JSONB_FLOAT && (j==k-1 || !sqlite3Isdigit(z[j+1])) ){
            return j+1;
          }
          seen = 1;
          continue;
        }
        if( z[j]=='e' || z[j]=='E' ){
          if( seen==2 ) return j+1;
          if( j==k-1 ) return j+1;
          if( z[j+1]=='+' || z[j+1]=='-' ){
            j++;
            if( j==k-1 ) return j+1;
          }
          seen = 2;
          continue;
        }
        return j+1;
      }
      if( seen==0 ) return i+1;
      return 0;
    }
    case JSONB_TEXT: {
      j = i+n;
      k = j+sz;
      while( j<k ){
        if( !jsonIsOk[z[j]] && z[j]!='\'' ) return j+1;
        j++;
      }
      return 0;
    }
    case JSONB_TEXTJ:
    case JSONB_TEXT5: {
      j = i+n;
      k = j+sz;
      while( j<k ){
        if( !jsonIsOk[z[j]] && z[j]!='\'' ){
          if( z[j]=='"' ){
            if( x==JSONB_TEXTJ ) return j+1;
          }else if( z[j]<=0x1f ){
            /* Control characters in JSON5 string literals are ok */
            if( x==JSONB_TEXTJ ) return j+1;
          }else if( NEVER(z[j]!='\\') || j+1>=k ){
            return j+1;
          }else if( strchr("\"\\/bfnrt",z[j+1])!=0 ){
            j++;
          }else if( z[j+1]=='u' ){
            if( j+5>=k ) return j+1;
            if( !jsonIs4Hex((const char*)&z[j+2]) ) return j+1;
            j++;
          }else if( x!=JSONB_TEXT5 ){
            return j+1;
          }else{
            u32 c = 0;
            u32 szC = jsonUnescapeOneChar((const char*)&z[j], k-j, &c);
            if( c==JSON_INVALID_CHAR ) return j+1;
            j += szC - 1;
          }
        }
        j++;
      }
      return 0;
    }
    case JSONB_TEXTRAW: {
      return 0;
    }
    case JSONB_ARRAY: {
      u32 sub;
      j = i+n;
      k = j+sz;
      while( j<k ){
        sz = 0;
        n = jsonbPayloadSize(pParse, j, &sz);
        if( n==0 ) return j+1;
        if( j+n+sz>k ) return j+1;
        sub = jsonbValidityCheck(pParse, j, j+n+sz, iDepth+1);
        if( sub ) return sub;
        j += n + sz;
      }
      assert( j==k );
      return 0;
    }
    case JSONB_OBJECT: {
      u32 cnt = 0;
      u32 sub;
      j = i+n;
      k = j+sz;
      while( j<k ){
        sz = 0;
        n = jsonbPayloadSize(pParse, j, &sz);
        if( n==0 ) return j+1;
        if( j+n+sz>k ) return j+1;
        if( (cnt & 1)==0 ){
          x = z[j] & 0x0f;
          if( x<JSONB_TEXT || x>JSONB_TEXTRAW ) return j+1;
        }
        sub = jsonbValidityCheck(pParse, j, j+n+sz, iDepth+1);
        if( sub ) return sub;
        cnt++;
        j += n + sz;
      }
      assert( j==k );
      if( (cnt & 1)!=0 ) return j+1;
      return 0;
    }
    default: {
      return i+1;
    }
  }
}

/*
** Translate a single element of JSON text at pParse->zJson[i] into
** its equivalent binary JSONB representation.  Append the translation into
** pParse->aBlob[] beginning at pParse->nBlob.  The size of
** pParse->aBlob[] is increased as necessary.
**
** Return the index of the first character past the end of the element parsed,
** or one of the following special result codes:
**
**      0    End of input
**     -1    Syntax error or OOM
**     -2    '}' seen   \
**     -3    ']' seen    \___  For these returns, pParse->iErr is set to
**     -4    ',' seen    /     the index in zJson[] of the seen character
**     -5    ':' seen   /
*/
static int jsonTranslateTextToBlob(JsonParse *pParse, u32 i){
  char c;
  u32 j;
  u32 iThis, iStart;
  int x;
  u8 t;
  const char *z = pParse->zJson;
json_parse_restart:
  switch( (u8)z[i] ){
  case '{': {
    /* Parse object */
    iThis = pParse->nBlob;
    jsonBlobAppendNode(pParse, JSONB_OBJECT, pParse->nJson-i, 0);
    if( ++pParse->iDepth > JSON_MAX_DEPTH ){
      pParse->iErr = i;
      return -1;
    }
    iStart = pParse->nBlob;
    for(j=i+1;;j++){
      u32 iBlob = pParse->nBlob;
      x = jsonTranslateTextToBlob(pParse, j);
      if( x<=0 ){
        int op;
        if( x==(-2) ){
          j = pParse->iErr;
          if( pParse->nBlob!=(u32)iStart ) pParse->hasNonstd = 1;
          break;
        }
        j += json5Whitespace(&z[j]);
        op = JSONB_TEXT;
        if( sqlite3JsonId1(z[j])
         || (z[j]=='\\' && jsonIs4HexB(&z[j+1], &op))
        ){
          int k = j+1;
          while( (sqlite3JsonId2(z[k]) && json5Whitespace(&z[k])==0)
            || (z[k]=='\\' && jsonIs4HexB(&z[k+1], &op))
          ){
            k++;
          }
          assert( iBlob==pParse->nBlob );
          jsonBlobAppendNode(pParse, op, k-j, &z[j]);
          pParse->hasNonstd = 1;
          x = k;
        }else{
          if( x!=-1 ) pParse->iErr = j;
          return -1;
        }
      }
      if( pParse->oom ) return -1;
      t = pParse->aBlob[iBlob] & 0x0f;
      if( t<JSONB_TEXT || t>JSONB_TEXTRAW ){
        pParse->iErr = j;
        return -1;
      }
      j = x;
      if( z[j]==':' ){
        j++;
      }else{
        if( jsonIsspace(z[j]) ){
          /* strspn() is not helpful here */
          do{ j++; }while( jsonIsspace(z[j]) );
          if( z[j]==':' ){
            j++;
            goto parse_object_value;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        if( x!=(-5) ){
          if( x!=(-1) ) pParse->iErr = j;
          return -1;
        }
        j = pParse->iErr+1;
      }
    parse_object_value:
      x = jsonTranslateTextToBlob(pParse, j);
      if( x<=0 ){
        if( x!=(-1) ) pParse->iErr = j;
        return -1;
      }
      j = x;
      if( z[j]==',' ){
        continue;
      }else if( z[j]=='}' ){
        break;
      }else{
        if( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          if( z[j]==',' ){
            continue;
          }else if( z[j]=='}' ){
            break;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        if( x==(-4) ){
          j = pParse->iErr;
          continue;
        }
        if( x==(-2) ){
          j = pParse->iErr;
          break;
        }
      }
      pParse->iErr = j;
      return -1;
    }
    jsonBlobChangePayloadSize(pParse, iThis, pParse->nBlob - iStart);
    pParse->iDepth--;
    return j+1;
  }
  case '[': {
    /* Parse array */
    iThis = pParse->nBlob;
    assert( i<=(u32)pParse->nJson );
    jsonBlobAppendNode(pParse, JSONB_ARRAY, pParse->nJson - i, 0);
    iStart = pParse->nBlob;
    if( pParse->oom ) return -1;
    if( ++pParse->iDepth > JSON_MAX_DEPTH ){
      pParse->iErr = i;
      return -1;
    }
    for(j=i+1;;j++){
      x = jsonTranslateTextToBlob(pParse, j);
      if( x<=0 ){
        if( x==(-3) ){
          j = pParse->iErr;
          if( pParse->nBlob!=iStart ) pParse->hasNonstd = 1;
          break;
        }
        if( x!=(-1) ) pParse->iErr = j;
        return -1;
      }
      j = x;
      if( z[j]==',' ){
        continue;
      }else if( z[j]==']' ){
        break;
      }else{
        if( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          if( z[j]==',' ){
            continue;
          }else if( z[j]==']' ){
            break;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        if( x==(-4) ){
          j = pParse->iErr;
          continue;
        }
        if( x==(-3) ){
          j = pParse->iErr;
          break;
        }
      }
      pParse->iErr = j;
      return -1;
    }
    jsonBlobChangePayloadSize(pParse, iThis, pParse->nBlob - iStart);
    pParse->iDepth--;
    return j+1;
  }
  case '\'': {
    u8 opcode;
    char cDelim;
    pParse->hasNonstd = 1;
    opcode = JSONB_TEXT;
    goto parse_string;
  case '"':
    /* Parse string */
    opcode = JSONB_TEXT;
  parse_string:
    cDelim = z[i];
    j = i+1;
    while( 1 /*exit-by-break*/ ){
      if( jsonIsOk[(u8)z[j]] ){
        if( !jsonIsOk[(u8)z[j+1]] ){
          j += 1;
        }else if( !jsonIsOk[(u8)z[j+2]] ){
          j += 2;
        }else{
          j += 3;
          continue;
        }
      }
      c = z[j];
      if( c==cDelim ){
        break;
      }else if( c=='\\' ){
        c = z[++j];
        if( c=='"' || c=='\\' || c=='/' || c=='b' || c=='f'
           || c=='n' || c=='r' || c=='t'
           || (c=='u' && jsonIs4Hex(&z[j+1])) ){
          if( opcode==JSONB_TEXT ) opcode = JSONB_TEXTJ;
        }else if( c=='\'' ||  c=='v' || c=='\n'
#ifdef SQLITE_BUG_COMPATIBLE_20250510
           || (c=='0')                            /* Legacy bug compatible */
#else
           || (c=='0' && !sqlite3Isdigit(z[j+1])) /* Correct implementation */
#endif
           || (0xe2==(u8)c && 0x80==(u8)z[j+1]
                && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2]))
           || (c=='x' && jsonIs2Hex(&z[j+1])) ){
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }else if( c=='\r' ){
          if( z[j+1]=='\n' ) j++;
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }else{
          pParse->iErr = j;
          return -1;
        }
      }else if( c<=0x1f ){
        if( c==0 ){
          pParse->iErr = j;
          return -1;
        }
        /* Control characters are not allowed in canonical JSON string
        ** literals, but are allowed in JSON5 string literals. */
        opcode = JSONB_TEXT5;
        pParse->hasNonstd = 1;
      }else if( c=='"' ){
        opcode = JSONB_TEXT5;
      }
      j++;
    }
    jsonBlobAppendNode(pParse, opcode, j-1-i, &z[i+1]);
    return j+1;
  }
  case 't': {
    if( strncmp(z+i,"true",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_TRUE);
      return i+4;
    }
    pParse->iErr = i;
    return -1;
  }
  case 'f': {
    if( strncmp(z+i,"false",5)==0 && !sqlite3Isalnum(z[i+5]) ){
      jsonBlobAppendOneByte(pParse, JSONB_FALSE);
      return i+5;
    }
    pParse->iErr = i;
    return -1;
  }
  case '+': {
    u8 seenE;
    pParse->hasNonstd = 1;
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
    goto parse_number;
  case '.':
    if( sqlite3Isdigit(z[i+1]) ){
      pParse->hasNonstd = 1;
      t = 0x03;          /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
      seenE = 0;
      goto parse_number_2;
    }
    pParse->iErr = i;
    return -1;
  case '-':
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    /* Parse number */
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
  parse_number:
    seenE = 0;
    assert( '-' < '0' );
    assert( '+' < '0' );
    assert( '.' < '0' );
    c = z[i];

    if( c<='0' ){
      if( c=='0' ){
        if( (z[i+1]=='x' || z[i+1]=='X') && sqlite3Isxdigit(z[i+2]) ){
          assert( t==0x00 );
          pParse->hasNonstd = 1;
          t = 0x01;
          for(j=i+3; sqlite3Isxdigit(z[j]); j++){}
          goto parse_number_finish;
        }else if( sqlite3Isdigit(z[i+1]) ){
          pParse->iErr = i+1;
          return -1;
        }
      }else{
        if( !sqlite3Isdigit(z[i+1]) ){
          /* JSON5 allows for "+Infinity" and "-Infinity" using exactly
          ** that case.  SQLite also allows these in any case and it allows
          ** "+inf" and "-inf". */
          if( (z[i+1]=='I' || z[i+1]=='i')
           && sqlite3StrNICmp(&z[i+1], "inf",3)==0
          ){
            pParse->hasNonstd = 1;
            if( z[i]=='-' ){
              jsonBlobAppendNode(pParse, JSONB_FLOAT, 6, "-9e999");
            }else{
              jsonBlobAppendNode(pParse, JSONB_FLOAT, 5, "9e999");
            }
            return i + (sqlite3StrNICmp(&z[i+4],"inity",5)==0 ? 9 : 4);
          }
          if( z[i+1]=='.' ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            goto parse_number_2;
          }
          pParse->iErr = i;
          return -1;
        }
        if( z[i+1]=='0' ){
          if( sqlite3Isdigit(z[i+2]) ){
            pParse->iErr = i+1;
            return -1;
          }else if( (z[i+2]=='x' || z[i+2]=='X') && sqlite3Isxdigit(z[i+3]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            for(j=i+4; sqlite3Isxdigit(z[j]); j++){}
            goto parse_number_finish;
          }
        }
      }
    }

  parse_number_2:
    for(j=i+1;; j++){
      c = z[j];
      if( sqlite3Isdigit(c) ) continue;
      if( c=='.' ){
        if( (t & 0x02)!=0 ){
          pParse->iErr = j;
          return -1;
        }
        t |= 0x02;
        continue;
      }
      if( c=='e' || c=='E' ){
        if( z[j-1]<'0' ){
          if( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
          }else{
            pParse->iErr = j;
            return -1;
          }
        }
        if( seenE ){
          pParse->iErr = j;
          return -1;
        }
        t |= 0x02;
        seenE = 1;
        c = z[j+1];
        if( c=='+' || c=='-' ){
          j++;
          c = z[j+1];
        }
        if( c<'0' || c>'9' ){
          pParse->iErr = j;
          return -1;
        }
        continue;
      }
      break;
    }
    if( z[j-1]<'0' ){
      if( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
        pParse->hasNonstd = 1;
        t |= 0x01;
      }else{
        pParse->iErr = j;
        return -1;
      }
    }
  parse_number_finish:
    assert( JSONB_INT+0x01==JSONB_INT5 );
    assert( JSONB_FLOAT+0x01==JSONB_FLOAT5 );
    assert( JSONB_INT+0x02==JSONB_FLOAT );
    if( z[i]=='+' ) i++;
    jsonBlobAppendNode(pParse, JSONB_INT+t, j-i, &z[i]);
    return j;
  }
  case '}': {
    pParse->iErr = i;
    return -2;  /* End of {...} */
  }
  case ']': {
    pParse->iErr = i;
    return -3;  /* End of [...] */
  }
  case ',': {
    pParse->iErr = i;
    return -4;  /* List separator */
  }
  case ':': {
    pParse->iErr = i;
    return -5;  /* Object label/value separator */
  }
  case 0: {
    return 0;   /* End of file */
  }
  case 0x09:
  case 0x0a:
  case 0x0d:
  case 0x20: {
    i += 1 + (u32)strspn(&z[i+1], jsonSpaces);
    goto json_parse_restart;
  }
  case 0x0b:
  case 0x0c:
  case '/':
  case 0xc2:
  case 0xe1:
  case 0xe2:
  case 0xe3:
  case 0xef: {
    j = json5Whitespace(&z[i]);
    if( j>0 ){
      i += j;
      pParse->hasNonstd = 1;
      goto json_parse_restart;
    }
    pParse->iErr = i;
    return -1;
  }
  case 'n': {
    if( strncmp(z+i,"null",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_NULL);
      return i+4;
    }
    /* fall-through into the default case that checks for NaN */
    /* no break */ deliberate_fall_through
  }
  default: {
    u32 k;
    int nn;
    c = z[i];
    for(k=0; k<sizeof(aNanInfName)/sizeof(aNanInfName[0]); k++){
      if( c!=aNanInfName[k].c1 && c!=aNanInfName[k].c2 ) continue;
      nn = aNanInfName[k].n;
      if( sqlite3StrNICmp(&z[i], aNanInfName[k].zMatch, nn)!=0 ){
        continue;
      }
      if( sqlite3Isalnum(z[i+nn]) ) continue;
      if( aNanInfName[k].eType==JSONB_FLOAT ){
        jsonBlobAppendNode(pParse, JSONB_FLOAT, 5, "9e999");
      }else{
        jsonBlobAppendOneByte(pParse, JSONB_NULL);
      }
      pParse->hasNonstd = 1;
      return i + nn;
    }
    pParse->iErr = i;
    return -1;  /* Syntax error */
  }
  } /* End switch(z[i]) */
}


/*
** Parse a complete JSON string.  Return 0 on success or non-zero if there
** are any errors.  If an error occurs, free all memory held by pParse,
** but not pParse itself.
**
** pParse must be initialized to an empty parse object prior to calling
** this routine.
*/
static int jsonConvertTextToBlob(
  JsonParse *pParse,           /* Initialize and fill this JsonParse object */
  sqlite3_context *pCtx        /* Report errors here */
){
  int i;
  const char *zJson = pParse->zJson;
  i = jsonTranslateTextToBlob(pParse, 0);
  if( pParse->oom ) i = -1;
  if( i>0 ){
#ifdef SQLITE_DEBUG
    assert( pParse->iDepth==0 );
    if( sqlite3Config.bJsonSelfcheck ){
      assert( jsonbValidityCheck(pParse, 0, pParse->nBlob, 0)==0 );
    }
#endif
    while( jsonIsspace(zJson[i]) ) i++;
    if( zJson[i] ){
      i += json5Whitespace(&zJson[i]);
      if( zJson[i] ){
        if( pCtx ) sqlite3_result_error(pCtx, "malformed JSON", -1);
        jsonParseReset(pParse);
        return 1;
      }
      pParse->hasNonstd = 1;
    }
  }
  if( i<=0 ){
    if( pCtx!=0 ){
      if( pParse->oom ){
        sqlite3_result_error_nomem(pCtx);
      }else{
        sqlite3_result_error(pCtx, "malformed JSON", -1);
      }
    }
    jsonParseReset(pParse);
    return 1;
  }
  return 0;
}

/*
** The input string pStr is a well-formed JSON text string.  Convert
** this into the JSONB format and make it the return value of the
** SQL function.
*/
static void jsonReturnStringAsBlob(JsonString *pStr){
  JsonParse px;
  assert( pStr->eErr==0 );
  memset(&px, 0, sizeof(px));
  px.zJson = pStr->zBuf;
  px.nJson = pStr->nUsed;
  px.db = sqlite3_context_db_handle(pStr->pCtx);
  (void)jsonTranslateTextToBlob(&px, 0);
  if( px.oom ){
    sqlite3DbFree(px.db, px.aBlob);
    sqlite3_result_error_nomem(pStr->pCtx);
  }else{
    assert( px.nBlobAlloc>0 );
    assert( !px.bReadOnly );
    sqlite3_result_blob(pStr->pCtx, px.aBlob, px.nBlob, SQLITE_DYNAMIC);
  }
}

/* The byte at index i is a node type-code.  This routine
** determines the payload size for that node and writes that
** payload size in to *pSz.  It returns the offset from i to the
** beginning of the payload.  Return 0 on error.
*/
static u32 jsonbPayloadSize(const JsonParse *pParse, u32 i, u32 *pSz){
  u8 x;
  u32 sz;
  u32 n;
  assert( i<=pParse->nBlob );
  x = pParse->aBlob[i]>>4;
  if( x<=11 ){
    sz = x;
    n = 1;
  }else if( x==12 ){
    if( i+1>=pParse->nBlob ){
      *pSz = 0;
      return 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }else if( x==13 ){
    if( i+2>=pParse->nBlob ){
      *pSz = 0;
      return 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }else if( x==14 ){
    if( i+4>=pParse->nBlob ){
      *pSz = 0;
      return 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }else{
    if( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      return 0;
    }
    sz = ((u32)pParse->aBlob[i+5]<<24) + (pParse->aBlob[i+6]<<16) +
         (pParse->aBlob[i+7]<<8) + pParse->aBlob[i+8];
    n = 9;
  }
  if( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    return 0;
  }
  *pSz = sz;
  return n;
}


/*
** Translate the binary JSONB representation of JSON beginning at
** pParse->aBlob[i] into a JSON text string.  Append the JSON
** text onto the end of pOut.  Return the index in pParse->aBlob[]
** of the first byte past the end of the element that is translated.
**
** If an error is detected in the BLOB input, the pOut->eErr flag
** might get set to JSTRING_MALFORMED.  But not all BLOB input errors
** are detected.  So a malformed JSONB input might either result
** in an error, or in incorrect JSON.
**
** The pOut->eErr JSTRING_OOM flag is set on a OOM.
*/
static u32 jsonTranslateBlobToText(
  JsonParse *pParse,             /* the complete parse of the JSON */
  u32 i,                         /* Start rendering at this index */
  JsonString *pOut               /* Write JSON here */
){
  u32 sz, n, j, iEnd;

  n = jsonbPayloadSize(pParse, i, &sz);
  if( n==0 ){
    pOut->eErr |= JSTRING_MALFORMED;
    return pParse->nBlob+1;
  }
  switch( pParse->aBlob[i] & 0x0f ){
    case JSONB_NULL: {
      jsonAppendRawNZ(pOut, "null", 4);
      return i+1;
    }
    case JSONB_TRUE: {
      jsonAppendRawNZ(pOut, "true", 4);
      return i+1;
    }
    case JSONB_FALSE: {
      jsonAppendRawNZ(pOut, "false", 5);
      return i+1;
    }
    case JSONB_INT:
    case JSONB_FLOAT: {
      if( sz==0 ) goto malformed_jsonb;
      jsonAppendRaw(pOut, (const char*)&pParse->aBlob[i+n], sz);
      break;
    }
    case JSONB_INT5: {  /* Integer literal in hexadecimal notation */
      u32 k = 2;
      sqlite3_uint64 u = 0;
      const char *zIn = (const char*)&pParse->aBlob[i+n];
      int bOverflow = 0;
      if( sz==0 ) goto malformed_jsonb;
      if( zIn[0]=='-' ){
        jsonAppendChar(pOut, '-');
        k++;
      }else if( zIn[0]=='+' ){
        k++;
      }
      for(; k<sz; k++){
        if( !sqlite3Isxdigit(zIn[k]) ){
          pOut->eErr |= JSTRING_MALFORMED;
          break;
        }else if( (u>>60)!=0 ){
          bOverflow = 1;
        }else{
          u = u*16 + sqlite3HexToInt(zIn[k]);
        }
      }
      jsonPrintf(100,pOut,bOverflow?"9.0e999":"%llu", u);
      break;
    }
    case JSONB_FLOAT5: { /* Float literal missing digits beside "." */
      u32 k = 0;
      const char *zIn = (const char*)&pParse->aBlob[i+n];
      if( sz==0 ) goto malformed_jsonb;
      if( zIn[0]=='-' ){
        jsonAppendChar(pOut, '-');
        k++;
      }
      if( zIn[k]=='.' ){
        jsonAppendChar(pOut, '0');
      }
      for(; k<sz; k++){
        jsonAppendChar(pOut, zIn[k]);
        if( zIn[k]=='.' && (k+1==sz || !sqlite3Isdigit(zIn[k+1])) ){
          jsonAppendChar(pOut, '0');
        }
      }
      break;
    }
    case JSONB_TEXT:
    case JSONB_TEXTJ: {
      if( pOut->nUsed+sz+2<=pOut->nAlloc || jsonStringGrow(pOut, sz+2)==0 ){
        pOut->zBuf[pOut->nUsed] = '"';
        memcpy(pOut->zBuf+pOut->nUsed+1,(const char*)&pParse->aBlob[i+n],sz);
        pOut->zBuf[pOut->nUsed+sz+1] = '"';
        pOut->nUsed += sz+2;
      }
      break;
    }
    case JSONB_TEXT5: {
      const char *zIn;
      u32 k;
      u32 sz2 = sz;
      zIn = (const char*)&pParse->aBlob[i+n];
      jsonAppendChar(pOut, '"');
      while( sz2>0 ){
        for(k=0; k<sz2 && (jsonIsOk[(u8)zIn[k]] || zIn[k]=='\''); k++){}
        if( k>0 ){
          jsonAppendRawNZ(pOut, zIn, k);
          if( k>=sz2 ){
            break;
          }
          zIn += k;
          sz2 -= k;
        }
        if( zIn[0]=='"' ){
          jsonAppendRawNZ(pOut, "\\\"", 2);
          zIn++;
          sz2--;
          continue;
        }
        if( zIn[0]<=0x1f ){
          if( pOut->nUsed+7>pOut->nAlloc && jsonStringGrow(pOut,7) ) break;
          jsonAppendControlChar(pOut, zIn[0]);
          zIn++;
          sz2--;
          continue;
        }
        assert( zIn[0]=='\\' );
        assert( sz2>=1 );
        if( sz2<2 ){
          pOut->eErr |= JSTRING_MALFORMED;
          break;
        }
        switch( (u8)zIn[1] ){
          case '\'':
            jsonAppendChar(pOut, '\'');
            break;
          case 'v':
            jsonAppendRawNZ(pOut, "\\u000b", 6);
            break;
          case 'x':
            if( sz2<4 ){
              pOut->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              break;
            }
            jsonAppendRawNZ(pOut, "\\u00", 4);
            jsonAppendRawNZ(pOut, &zIn[2], 2);
            zIn += 2;
            sz2 -= 2;
            break;
          case '0':
            jsonAppendRawNZ(pOut, "\\u0000", 6);
            break;
          case '\r':
            if( sz2>2 && zIn[2]=='\n' ){
              zIn++;
              sz2--;
            }
            break;
          case '\n':
            break;
          case 0xe2:
            /* '\' followed by either U+2028 or U+2029 is ignored as
            ** whitespace.  Not that in UTF8, U+2028 is 0xe2 0x80 0x29.
            ** U+2029 is the same except for the last byte */
            if( sz2<4
             || 0x80!=(u8)zIn[2]
             || (0xa8!=(u8)zIn[3] && 0xa9!=(u8)zIn[3])
            ){
              pOut->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              break;
            }
            zIn += 2;
            sz2 -= 2;
            break;
          default:
            jsonAppendRawNZ(pOut, zIn, 2);
            break;
        }
        assert( sz2>=2 );
        zIn += 2;
        sz2 -= 2;
      }
      jsonAppendChar(pOut, '"');
      break;
    }
    case JSONB_TEXTRAW: {
      jsonAppendString(pOut, (const char*)&pParse->aBlob[i+n], sz);
      break;
    }
    case JSONB_ARRAY: {
      jsonAppendChar(pOut, '[');
      j = i+n;
      iEnd = j+sz;
      if( ++pParse->iDepth > JSON_MAX_DEPTH ){
        jsonStringTooDeep(pOut);
      }
      while( j<iEnd && pOut->eErr==0 ){
        j = jsonTranslateBlobToText(pParse, j, pOut);
        jsonAppendChar(pOut, ',');
      }
      pParse->iDepth--;
      if( j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
      if( sz>0 ) jsonStringTrimOneChar(pOut);
      jsonAppendChar(pOut, ']');
      break;
    }
    case JSONB_OBJECT: {
      int x = 0;
      jsonAppendChar(pOut, '{');
      j = i+n;
      iEnd = j+sz;
      if( ++pParse->iDepth > JSON_MAX_DEPTH ){
        jsonStringTooDeep(pOut);
      }
      while( j<iEnd && pOut->eErr==0 ){
        j = jsonTranslateBlobToText(pParse, j, pOut);
        jsonAppendChar(pOut, (x++ & 1) ? ',' : ':');
      }
      pParse->iDepth--;
      if( (x & 1)!=0 || j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
      if( sz>0 ) jsonStringTrimOneChar(pOut);
      jsonAppendChar(pOut, '}');
      break;
    }

    default: {
      malformed_jsonb:
      pOut->eErr |= JSTRING_MALFORMED;
      break;
    }
  }
  return i+n+sz;
}

/* Context for recursion of json_pretty()
*/
typedef struct JsonPretty JsonPretty;
struct JsonPretty {
  JsonParse *pParse;        /* The BLOB being rendered */
  JsonString *pOut;         /* Generate pretty output into this string */
  const char *zIndent;      /* Use this text for indentation */
  u32 szIndent;             /* Bytes in zIndent[] */
  u32 nIndent;              /* Current level of indentation */
};

/* Append indentation to the pretty JSON under construction */
static void jsonPrettyIndent(JsonPretty *pPretty){
  u32 jj;
  for(jj=0; jj<pPretty->nIndent; jj++){
    jsonAppendRaw(pPretty->pOut, pPretty->zIndent, pPretty->szIndent);
  }
}

/*
** Translate the binary JSONB representation of JSON beginning at
** pParse->aBlob[i] into a JSON text string.  Append the JSON
** text onto the end of pOut.  Return the index in pParse->aBlob[]
** of the first byte past the end of the element that is translated.
**
** This is a variant of jsonTranslateBlobToText() that "pretty-prints"
** the output.  Extra whitespace is inserted to make the JSON easier
** for humans to read.
**
** If an error is detected in the BLOB input, the pOut->eErr flag
** might get set to JSTRING_MALFORMED.  But not all BLOB input errors
** are detected.  So a malformed JSONB input might either result
** in an error, or in incorrect JSON.
**
** The pOut->eErr JSTRING_OOM flag is set on a OOM.
*/
static u32 jsonTranslateBlobToPrettyText(
  JsonPretty *pPretty,       /* Pretty-printing context */
  u32 i                      /* Start rendering at this index */
){
  u32 sz, n, j, iEnd;
  JsonParse *pParse = pPretty->pParse;
  JsonString *pOut = pPretty->pOut;
  n = jsonbPayloadSize(pParse, i, &sz);
  if( n==0 ){
    pOut->eErr |= JSTRING_MALFORMED;
    return pParse->nBlob+1;
  }
  switch( pParse->aBlob[i] & 0x0f ){
    case JSONB_ARRAY: {
      j = i+n;
      iEnd = j+sz;
      jsonAppendChar(pOut, '[');
      if( j<iEnd ){
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent++;
        if( pPretty->nIndent >= JSON_MAX_DEPTH ){
          jsonStringTooDeep(pOut);
        }
        while( pOut->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = jsonTranslateBlobToPrettyText(pPretty, j);
          if( j>=iEnd ) break;
          jsonAppendRawNZ(pOut, ",\n", 2);
        }
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(pOut, ']');
      i = iEnd;
      break;
    }
    case JSONB_OBJECT: {
      j = i+n;
      iEnd = j+sz;
      jsonAppendChar(pOut, '{');
      if( j<iEnd ){
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent++;
        if( pPretty->nIndent >= JSON_MAX_DEPTH ){
          jsonStringTooDeep(pOut);
        }
        pParse->iDepth = pPretty->nIndent;
        while( pOut->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = jsonTranslateBlobToText(pParse, j, pOut);
          if( j>iEnd ){
            pOut->eErr |= JSTRING_MALFORMED;
            break;
          }
          jsonAppendRawNZ(pOut, ": ", 2);
          j = jsonTranslateBlobToPrettyText(pPretty, j);
          if( j>=iEnd ) break;
          jsonAppendRawNZ(pOut, ",\n", 2);
        }
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(pOut, '}');
      i = iEnd;
      break;
    }
    default: {
      i = jsonTranslateBlobToText(pParse, i, pOut);
      break;
    }
  }
  return i;
}

/*
** Given that a JSONB_ARRAY object starts at offset i, return
** the number of entries in that array.
*/
static u32 jsonbArrayCount(JsonParse *pParse, u32 iRoot){
  u32 n, sz, i, iEnd;
  u32 k = 0;
  n = jsonbPayloadSize(pParse, iRoot, &sz);
  iEnd = iRoot+n+sz;
  for(i=iRoot+n; n>0 && i<iEnd; i+=sz+n, k++){
    n = jsonbPayloadSize(pParse, i, &sz);
  }
  return k;
}

/*
** Edit the payload size of the element at iRoot by the amount in
** pParse->delta.
*/
static void jsonAfterEditSizeAdjust(JsonParse *pParse, u32 iRoot){
  u32 sz = 0;
  u32 nBlob;
  assert( pParse->delta!=0 );
  assert( pParse->nBlobAlloc >= pParse->nBlob );
  nBlob = pParse->nBlob;
  pParse->nBlob = pParse->nBlobAlloc;
  (void)jsonbPayloadSize(pParse, iRoot, &sz);
  pParse->nBlob = nBlob;
  sz += pParse->delta;
  pParse->delta += jsonBlobChangePayloadSize(pParse, iRoot, sz);
}

/*
** If the JSONB at aIns[0..nIns-1] can be expanded (by denormalizing the
** size field) by d bytes, then write the expansion into aOut[] and
** return true.  In this way, an overwrite happens without changing the
** size of the JSONB, which reduces memcpy() operations and also make it
** faster and easier to update the B-Tree entry that contains the JSONB
** in the database.
**
** If the expansion of aIns[] by d bytes cannot be (easily) accomplished
** then return false.
**
** The d parameter is guaranteed to be between 1 and 8.
**
** This routine is an optimization.  A correct answer is obtained if it
** always leaves the output unchanged and returns false.
*/
static int jsonBlobOverwrite(
  u8 *aOut,                 /* Overwrite here */
  const u8 *aIns,           /* New content */
  u32 nIns,                 /* Bytes of new content */
  u32 d                     /* Need to expand new content by this much */
){
  u32 szPayload;       /* Bytes of payload */
  u32 i;               /* New header size, after expansion & a loop counter */
  u8 szHdr;            /* Size of header before expansion */

  /* Lookup table for finding the upper 4 bits of the first byte of the
  ** expanded aIns[], based on the size of the expanded aIns[] header:
  **
  **                             2     3  4     5  6  7  8     9 */
  static const u8 aType[] = { 0xc0, 0xd0, 0, 0xe0, 0, 0, 0, 0xf0 };

  if( (aIns[0]&0x0f)<=2 ) return 0;    /* Cannot enlarge NULL, true, false */
  switch( aIns[0]>>4 ){
    default: {                         /* aIns[] header size 1 */
      if( ((1<<d)&0x116)==0 ) return 0;  /* d must be 1, 2, 4, or 8 */
      i = d + 1;                         /* New hdr sz: 2, 3, 5, or 9 */
      szHdr = 1;
      break;
    }
    case 12: {                         /* aIns[] header size is 2 */
      if( ((1<<d)&0x8a)==0) return 0;    /* d must be 1, 3, or 7 */
      i = d + 2;                         /* New hdr sz: 2, 5, or 9 */
      szHdr = 2;
      break;
    }
    case 13: {                         /* aIns[] header size is 3 */
      if( d!=2 && d!=6 ) return 0;       /* d must be 2 or 6 */
      i = d + 3;                         /* New hdr sz: 5 or 9 */
      szHdr = 3;
      break;
    }
    case 14: {                         /* aIns[] header size is 5 */
      if( d!=4 ) return 0;               /* d must be 4 */
      i = 9;                             /* New hdr sz: 9 */
      szHdr = 5;
      break;
    }
    case 15: {                         /* aIns[] header size is 9 */
      return 0;                          /* No solution */
    }
  }
  assert( i>=2 && i<=9 && aType[i-2]!=0 );
  aOut[0] = (aIns[0] & 0x0f) | aType[i-2];
  memcpy(&aOut[i], &aIns[szHdr], nIns-szHdr);
  szPayload = nIns - szHdr;
  while( 1/*edit-by-break*/ ){
    i--;
    aOut[i] = szPayload & 0xff;
    if( i==1 ) break;
    szPayload >>= 8;
  }
  assert( (szPayload>>8)==0 );
  return 1;
}

/*
** Modify the JSONB blob at pParse->aBlob by removing nDel bytes of
** content beginning at iDel, and replacing them with nIns bytes of
** content given by aIns.
**
** nDel may be zero, in which case no bytes are removed.  But iDel is
** still important as new bytes will be insert beginning at iDel.
**
** aIns may be zero, in which case space is created to hold nIns bytes
** beginning at iDel, but that space is uninitialized.
**
** Set pParse->oom if an OOM occurs.
*/
static void jsonBlobEdit(
  JsonParse *pParse,     /* The JSONB to be modified is in pParse->aBlob */
  u32 iDel,              /* First byte to be removed */
  u32 nDel,              /* Number of bytes to remove */
  const u8 *aIns,        /* Content to insert */
  u32 nIns               /* Bytes of content to insert */
){
  i64 d = (i64)nIns - (i64)nDel;
  assert( pParse->nBlob >= (u64)iDel + (u64)nDel );
  if( d<0 && d>=(-8) && aIns!=0
   && jsonBlobOverwrite(&pParse->aBlob[iDel], aIns, nIns, (int)-d)
  ){
    return;
  }
  if( d!=0 ){
    if( pParse->nBlob + d > pParse->nBlobAlloc ){
      jsonBlobExpand(pParse, pParse->nBlob+d);
      if( pParse->oom ) return;
    }
    memmove(&pParse->aBlob[iDel+nIns],
            &pParse->aBlob[iDel+nDel],
            pParse->nBlob - (iDel+nDel));
    pParse->nBlob += d;
    pParse->delta += d;
  }
  if( nIns && aIns ){
    memcpy(&pParse->aBlob[iDel], aIns, nIns);
  }
}

/*
** Return the number of escaped newlines to be ignored.
** An escaped newline is a one of the following byte sequences:
**
**    0x5c 0x0a
**    0x5c 0x0d
**    0x5c 0x0d 0x0a
**    0x5c 0xe2 0x80 0xa8
**    0x5c 0xe2 0x80 0xa9
*/
static u32 jsonBytesToBypass(const char *z, u32 n){
  u32 i = 0;
  while( i+1<n ){
    if( z[i]!='\\' ) return i;
    if( z[i+1]=='\n' ){
      i += 2;
      continue;
    }
    if( z[i+1]=='\r' ){
      if( i+2<n && z[i+2]=='\n' ){
        i += 3;
      }else{
        i += 2;
      }
      continue;
    }
    if( 0xe2==(u8)z[i+1]
     && i+3<n
     && 0x80==(u8)z[i+2]
     && (0xa8==(u8)z[i+3] || 0xa9==(u8)z[i+3])
    ){
      i += 4;
      continue;
    }
    break;
  }
  return i;
}

/*
** Input z[0..n] defines JSON escape sequence including the leading '\\'.
** Decode that escape sequence into a single character.  Write that
** character into *piOut.  Return the number of bytes in the escape sequence.
**
** If there is a syntax error of some kind (for example too few characters
** after the '\\' to complete the encoding) then *piOut is set to
** JSON_INVALID_CHAR.
*/
static u32 jsonUnescapeOneChar(const char *z, u32 n, u32 *piOut){
  assert( n>0 );
  assert( z[0]=='\\' );
  if( n<2 ){
    *piOut = JSON_INVALID_CHAR;
    return n;
  }
  switch( (u8)z[1] ){
    case 'u': {
      u32 v, vlo;
      if( n<6 ){
        *piOut = JSON_INVALID_CHAR;
        return n;
      }
      v = jsonHexToInt4(&z[2]);
      if( (v & 0xfc00)==0xd800
       && n>=12
       && z[6]=='\\'
       && z[7]=='u'
       && ((vlo = jsonHexToInt4(&z[8]))&0xfc00)==0xdc00
      ){
        *piOut = ((v&0x3ff)<<10) + (vlo&0x3ff) + 0x10000;
        return 12;
      }else{
        *piOut = v;
        return 6;
      }
    }
    case 'b': {   *piOut = '\b';  return 2; }
    case 'f': {   *piOut = '\f';  return 2; }
    case 'n': {   *piOut = '\n';  return 2; }
    case 'r': {   *piOut = '\r';  return 2; }
    case 't': {   *piOut = '\t';  return 2; }
    case 'v': {   *piOut = '\v';  return 2; }
    case '0': {
      /* JSON5 requires that the \0 escape not be followed by a digit.
      ** But SQLite did not enforce this restriction in versions 3.42.0
      ** through 3.49.2.  That was a bug.  But some applications might have
      ** come to depend on that bug.  Use the SQLITE_BUG_COMPATIBLE_20250510
      ** option to restore the old buggy behavior. */
#ifdef SQLITE_BUG_COMPATIBLE_20250510
      /* Legacy bug-compatible behavior */
      *piOut = 0;
#else
      /* Correct behavior */
      *piOut = (n>2 && sqlite3Isdigit(z[2])) ? JSON_INVALID_CHAR : 0;
#endif
      return 2;
    }
    case '\'':
    case '"':
    case '/':
    case '\\':{   *piOut = z[1];  return 2; }
    case 'x': {
      if( n<4 ){
        *piOut = JSON_INVALID_CHAR;
        return n;
      }
      *piOut = (jsonHexToInt(z[2])<<4) | jsonHexToInt(z[3]);
      return 4;
    }
    case 0xe2:
    case '\r':
    case '\n': {
      u32 nSkip = jsonBytesToBypass(z, n);
      if( nSkip==0 ){
        *piOut = JSON_INVALID_CHAR;
        return n;
      }else if( nSkip==n ){
        *piOut = 0;
        return n;
      }else if( z[nSkip]=='\\' ){
        return nSkip + jsonUnescapeOneChar(&z[nSkip], n-nSkip, piOut);
      }else{
        int sz = sqlite3Utf8ReadLimited((u8*)&z[nSkip], n-nSkip, piOut);
        return nSkip + sz;
      }
    }
    default: {
      *piOut = JSON_INVALID_CHAR;
      return 2;
    }
  }
}


/*
** Compare two object labels.  Return 1 if they are equal and
** 0 if they differ.
**
** In this version, we know that one or the other or both of the
** two comparands contains an escape sequence.
*/
static SQLITE_NOINLINE int jsonLabelCompareEscaped(
  const char *zLeft,          /* The left label */
  u32 nLeft,                  /* Size of the left label in bytes */
  int rawLeft,                /* True if zLeft contains no escapes */
  const char *zRight,         /* The right label */
  u32 nRight,                 /* Size of the right label in bytes */
  int rawRight                /* True if zRight is escape-free */
){
  u32 cLeft, cRight;
  assert( rawLeft==0 || rawRight==0 );
  while( 1 /*exit-by-return*/ ){
    if( nLeft==0 ){
      cLeft = 0;
    }else if( rawLeft || zLeft[0]!='\\' ){
      cLeft = ((u8*)zLeft)[0];
      if( cLeft>=0xc0 ){
        int sz = sqlite3Utf8ReadLimited((u8*)zLeft, nLeft, &cLeft);
        zLeft += sz;
        nLeft -= sz;
      }else{
        zLeft++;
        nLeft--;
      }
    }else{
      u32 n = jsonUnescapeOneChar(zLeft, nLeft, &cLeft);
      zLeft += n;
      assert( n<=nLeft );
      nLeft -= n;
    }
    if( nRight==0 ){
      cRight = 0;
    }else if( rawRight || zRight[0]!='\\' ){
      cRight = ((u8*)zRight)[0];
      if( cRight>=0xc0 ){
        int sz = sqlite3Utf8ReadLimited((u8*)zRight, nRight, &cRight);
        zRight += sz;
        nRight -= sz;
      }else{
        zRight++;
        nRight--;
      }
    }else{
      u32 n = jsonUnescapeOneChar(zRight, nRight, &cRight);
      zRight += n;
      assert( n<=nRight );
      nRight -= n;
    }
    if( cLeft!=cRight ) return 0;
    if( cLeft==0 ) return 1;
  }
}

/*
** Compare two object labels.  Return 1 if they are equal and
** 0 if they differ.  Return -1 if an OOM occurs.
*/
static int jsonLabelCompare(
  const char *zLeft,          /* The left label */
  u32 nLeft,                  /* Size of the left label in bytes */
  int rawLeft,                /* True if zLeft contains no escapes */
  const char *zRight,         /* The right label */
  u32 nRight,                 /* Size of the right label in bytes */
  int rawRight                /* True if zRight is escape-free */
){
  if( rawLeft && rawRight ){
    /* Simpliest case:  Neither label contains escapes.  A simple
    ** memcmp() is sufficient. */
    if( nLeft!=nRight ) return 0;
    return memcmp(zLeft, zRight, nLeft)==0;
  }else{
    return jsonLabelCompareEscaped(zLeft, nLeft, rawLeft,
                                   zRight, nRight, rawRight);
  }
}

/*
** Error returns from jsonLookupStep()
*/
#define JSON_LOOKUP_ERROR      0xffffffff
#define JSON_LOOKUP_NOTFOUND   0xfffffffe
#define JSON_LOOKUP_NOTARRAY   0xfffffffd
#define JSON_LOOKUP_TOODEEP    0xfffffffc
#define JSON_LOOKUP_PATHERROR  0xfffffffb
#define JSON_LOOKUP_ISERROR(x) ((x)>=JSON_LOOKUP_PATHERROR)

/* Forward declaration */
static u32 jsonLookupStep(JsonParse*,u32,const char*,u32);


/* This helper routine for jsonLookupStep() populates pIns with
** binary data that is to be inserted into pParse.
**
** In the common case, pIns just points to pParse->aIns and pParse->nIns.
** But if the zPath of the original edit operation includes path elements
** that go deeper, additional substructure must be created.
**
** For example:
**
**     json_insert('{}', '$.a.b.c', 123);
**
** The search stops at '$.a'  But additional substructure must be
** created for the ".b.c" part of the patch so that the final result
** is:  {"a":{"b":{"c"::123}}}.  This routine populates pIns with
** the binary equivalent of {"b":{"c":123}} so that it can be inserted.
**
** The caller is responsible for resetting pIns when it has finished
** using the substructure.
*/
static u32 jsonCreateEditSubstructure(
  JsonParse *pParse,  /* The original JSONB that is being edited */
  JsonParse *pIns,    /* Populate this with the blob data to insert */
  const char *zTail   /* Tail of the path that determines substructure */
){
  static const u8 emptyObject[] = { JSONB_ARRAY, JSONB_OBJECT };
  int rc;
  memset(pIns, 0, sizeof(*pIns));
  pIns->db = pParse->db;
  if( zTail[0]==0 ){
    /* No substructure.  Just insert what is given in pParse. */
    pIns->aBlob = pParse->aIns;
    pIns->nBlob = pParse->nIns;
    rc = 0;
  }else{
    /* Construct the binary substructure */
    pIns->nBlob = 1;
    pIns->aBlob = (u8*)&emptyObject[zTail[0]=='.'];
    pIns->eEdit = pParse->eEdit;
    pIns->nIns = pParse->nIns;
    pIns->aIns = pParse->aIns;
    pIns->iDepth = pParse->iDepth+1;
    if( pIns->iDepth >= JSON_MAX_DEPTH ){
      return JSON_LOOKUP_TOODEEP;
    }
    rc = jsonLookupStep(pIns, 0, zTail, 0);
    pParse->iDepth--;
    pParse->oom |= pIns->oom;
  }
  return rc;  /* Error code only */
}

/*
** Search along zPath to find the Json element specified.  Return an
** index into pParse->aBlob[] for the start of that element's value.
**
** If the value found by this routine is the value half of label/value pair
** within an object, then set pPath->iLabel to the start of the corresponding
** label, before returning.
**
** Return one of the JSON_LOOKUP error codes if problems are seen.
**
** This routine will also modify the blob.  If pParse->eEdit is one of
** JEDIT_DEL, JEDIT_REPL, JEDIT_INS, JEDIT_SET, or JEDIT_AINS, then changes
** might be made to the selected value. If an edit is performed, then the
** return value does not necessarily point to the select element. If an edit
** is performed, the return value is only useful for detecting error
** conditions.
*/
static u32 jsonLookupStep(
  JsonParse *pParse,      /* The JSON to search */
  u32 iRoot,              /* Begin the search at this element of aBlob[] */
  const char *zPath,      /* The path to search */
  u32 iLabel              /* Label if iRoot is a value of in an object */
){
  u32 i, j, k, nKey, sz, n, iEnd, rc;
  const char *zKey;
  u8 x;

  if( zPath[0]==0 ){
    if( pParse->eEdit && jsonBlobMakeEditable(pParse, pParse->nIns) ){
      n = jsonbPayloadSize(pParse, iRoot, &sz);
      sz += n;
      if( pParse->eEdit==JEDIT_DEL ){
        if( iLabel>0 ){
          sz += iRoot - iLabel;
          iRoot = iLabel;
        }
        jsonBlobEdit(pParse, iRoot, sz, 0, 0);
      }else if( pParse->eEdit==JEDIT_INS ){
        /* Already exists, so json_insert() is a no-op */
      }else if( pParse->eEdit==JEDIT_AINS ){
        /* json_array_insert() */
        if( zPath[-1]!=']' ){
          return JSON_LOOKUP_NOTARRAY;
        }else{
          jsonBlobEdit(pParse, iRoot, 0, pParse->aIns, pParse->nIns);
        }
      }else{
        /* json_set() or json_replace() */
        jsonBlobEdit(pParse, iRoot, sz, pParse->aIns, pParse->nIns);
      }
    }
    pParse->iLabel = iLabel;
    return iRoot;
  }
  if( zPath[0]=='.' ){
    int rawKey = 1;
    x = pParse->aBlob[iRoot];
    zPath++;
    if( zPath[0]=='"' ){
      zKey = zPath + 1;
      for(i=1; zPath[i] && zPath[i]!='"'; i++){
        if( zPath[i]=='\\' && zPath[i+1]!=0 ) i++;
      }
      nKey = i-1;
      if( zPath[i] ){
        i++;
      }else{
        return JSON_LOOKUP_PATHERROR;
      }
      testcase( nKey==0 );
      rawKey = memchr(zKey, '\\', nKey)==0;
    }else{
      zKey = zPath;
      for(i=0; zPath[i] && zPath[i]!='.' && zPath[i]!='['; i++){}
      nKey = i;
      if( nKey==0 ){
        return JSON_LOOKUP_PATHERROR;
      }
    }
    if( (x & 0x0f)!=JSONB_OBJECT ) return JSON_LOOKUP_NOTFOUND;
    n = jsonbPayloadSize(pParse, iRoot, &sz);
    j = iRoot + n;  /* j is the index of a label */
    iEnd = j+sz;
    while( j<iEnd ){
      int rawLabel;
      const char *zLabel;
      x = pParse->aBlob[j] & 0x0f;
      if( x<JSONB_TEXT || x>JSONB_TEXTRAW ) return JSON_LOOKUP_ERROR;
      n = jsonbPayloadSize(pParse, j, &sz);
      if( n==0 ) return JSON_LOOKUP_ERROR;
      k = j+n;  /* k is the index of the label text */
      if( k+sz>=iEnd ) return JSON_LOOKUP_ERROR;
      zLabel = (const char*)&pParse->aBlob[k];
      rawLabel = x==JSONB_TEXT || x==JSONB_TEXTRAW;
      if( jsonLabelCompare(zKey, nKey, rawKey, zLabel, sz, rawLabel) ){
        u32 v = k+sz;  /* v is the index of the value */
        if( ((pParse->aBlob[v])&0x0f)>JSONB_OBJECT ) return JSON_LOOKUP_ERROR;
        n = jsonbPayloadSize(pParse, v, &sz);
        if( n==0 || v+n+sz>iEnd ) return JSON_LOOKUP_ERROR;
        assert( j>0 );
        if( ++pParse->iDepth >= JSON_MAX_DEPTH ){
          return JSON_LOOKUP_TOODEEP;
        }
        rc = jsonLookupStep(pParse, v, &zPath[i], j);
        pParse->iDepth--;
        if( pParse->delta ) jsonAfterEditSizeAdjust(pParse, iRoot);
        return rc;
      }
      j = k+sz;
      if( ((pParse->aBlob[j])&0x0f)>JSONB_OBJECT ) return JSON_LOOKUP_ERROR;
      n = jsonbPayloadSize(pParse, j, &sz);
      if( n==0 ) return JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    if( j>iEnd ) return JSON_LOOKUP_ERROR;
    if( pParse->eEdit>=JEDIT_INS ){
      u32 nIns;          /* Total bytes to insert (label+value) */
      JsonParse v;       /* BLOB encoding of the value to be inserted */
      JsonParse ix;      /* Header of the label to be inserted */
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_SET );
      testcase( pParse->eEdit==JEDIT_AINS );
      if( pParse->eEdit==JEDIT_AINS && sqlite3_strglob("*]",&zPath[i])!=0 ){
        return JSON_LOOKUP_NOTARRAY;
      }
      memset(&ix, 0, sizeof(ix));
      ix.db = pParse->db;
      jsonBlobAppendNode(&ix, rawKey?JSONB_TEXTRAW:JSONB_TEXT5, nKey, 0);
      pParse->oom |= ix.oom;
      rc = jsonCreateEditSubstructure(pParse, &v, &zPath[i]);
      if( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, ix.nBlob+nKey+v.nBlob)
      ){
        assert( !pParse->oom );
        nIns = ix.nBlob + nKey + v.nBlob;
        jsonBlobEdit(pParse, j, 0, 0, nIns);
        if( !pParse->oom ){
          assert( pParse->aBlob!=0 ); /* Because pParse->oom!=0 */
          assert( ix.aBlob!=0 );      /* Because pPasre->oom!=0 */
          memcpy(&pParse->aBlob[j], ix.aBlob, ix.nBlob);
          k = j + ix.nBlob;
          memcpy(&pParse->aBlob[k], zKey, nKey);
          k += nKey;
          memcpy(&pParse->aBlob[k], v.aBlob, v.nBlob);
          if( ALWAYS(pParse->delta) ) jsonAfterEditSizeAdjust(pParse, iRoot);
        }
      }
      jsonParseReset(&v);
      jsonParseReset(&ix);
      return rc;
    }
  }else if( zPath[0]=='[' ){
    u64 kk = 0;
    x = pParse->aBlob[iRoot] & 0x0f;
    if( x!=JSONB_ARRAY )  return JSON_LOOKUP_NOTFOUND;
    n = jsonbPayloadSize(pParse, iRoot, &sz);
    i = 1;
    while( sqlite3Isdigit(zPath[i]) ){
      if( kk<0xffffffff ) kk = kk*10 + zPath[i] - '0';
      /*     ^^^^^^^^^^--- Allow kk to be bigger than any JSON array so that
      ** we get NOTFOUND instead of PATHERROR, without overflowing kk. */
      i++;
    }
    if( i<2 || zPath[i]!=']' ){
      if( zPath[1]=='#' ){
        kk = jsonbArrayCount(pParse, iRoot);
        i = 2;
        if( zPath[2]=='-' && sqlite3Isdigit(zPath[3]) ){
          u64 nn = 0;
          i = 3;
          do{
            if( nn<0xffffffff ) nn = nn*10 + zPath[i] - '0';
            /*     ^^^^^^^^^^--- Allow nn to be bigger than any JSON array to
            ** get NOTFOUND instead of PATHERROR, without overflowing nn. */
            i++;
          }while( sqlite3Isdigit(zPath[i]) );
          if( nn>kk ) return JSON_LOOKUP_NOTFOUND;
          kk -= nn;
        }
        if( zPath[i]!=']' ){
          return JSON_LOOKUP_PATHERROR;
        }
      }else{
        return JSON_LOOKUP_PATHERROR;
      }
    }
    j = iRoot+n;
    iEnd = j+sz;
    while( j<iEnd ){
      if( kk==0 ){
        if( ++pParse->iDepth >= JSON_MAX_DEPTH ){
          return JSON_LOOKUP_TOODEEP;
        }
        rc = jsonLookupStep(pParse, j, &zPath[i+1], 0);
        pParse->iDepth--;
        if( pParse->delta ) jsonAfterEditSizeAdjust(pParse, iRoot);
        return rc;
      }
      kk--;
      n = jsonbPayloadSize(pParse, j, &sz);
      if( n==0 ) return JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    if( j>iEnd ) return JSON_LOOKUP_ERROR;
    if( kk>0 ) return JSON_LOOKUP_NOTFOUND;
    if( pParse->eEdit>=JEDIT_INS ){
      JsonParse v;
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_AINS );
      testcase( pParse->eEdit==JEDIT_SET );
      rc = jsonCreateEditSubstructure(pParse, &v, &zPath[i+1]);
      if( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, v.nBlob)
      ){
        assert( !pParse->oom );
        jsonBlobEdit(pParse, j, 0, v.aBlob, v.nBlob);
      }
      jsonParseReset(&v);
      if( pParse->delta ) jsonAfterEditSizeAdjust(pParse, iRoot);
      return rc;
    }
  }else{
    return JSON_LOOKUP_PATHERROR;
  }
  return JSON_LOOKUP_NOTFOUND;
}

/*
** Convert a JSON BLOB into text and make that text the return value
** of an SQL function.
*/
static void jsonReturnTextJsonFromBlob(
  sqlite3_context *ctx,
  const u8 *aBlob,
  u32 nBlob
){
  JsonParse x;
  JsonString s;

  if( NEVER(aBlob==0) ) return;
  memset(&x, 0, sizeof(x));
  x.aBlob = (u8*)aBlob;
  x.nBlob = nBlob;
  jsonStringInit(&s, ctx);
  jsonTranslateBlobToText(&x, 0, &s);
  jsonReturnString(&s, 0, 0);
}


/*
** Return the value of the BLOB node at index i.
**
** If the value is a primitive, return it as an SQL value.
** If the value is an array or object, return it as either
** JSON text or the BLOB encoding, depending on the eMode flag
** as follows:
**
**     eMode==0     JSONB if the JSON_B flag is set in userdata or
**                  text if the JSON_B flag is omitted from userdata.
**
**     eMode==1     Text
**
**     eMode==2     JSONB
*/
static void jsonReturnFromBlob(
  JsonParse *pParse,          /* Complete JSON parse tree */
  u32 i,                      /* Index of the node */
  sqlite3_context *pCtx,      /* Return value for this function */
  int eMode                   /* Format of return: text of JSONB */
){
  u32 n, sz;
  int rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);

  assert( eMode>=0 && eMode<=2 );
  n = jsonbPayloadSize(pParse, i, &sz);
  if( n==0 ){
    sqlite3_result_error(pCtx, "malformed JSON", -1);
    return;
  }
  switch( pParse->aBlob[i] & 0x0f ){
    case JSONB_NULL: {
      if( sz ) goto returnfromblob_malformed;
      sqlite3_result_null(pCtx);
      break;
    }
    case JSONB_TRUE: {
      if( sz ) goto returnfromblob_malformed;
      sqlite3_result_int(pCtx, 1);
      break;
    }
    case JSONB_FALSE: {
      if( sz ) goto returnfromblob_malformed;
      sqlite3_result_int(pCtx, 0);
      break;
    }
    case JSONB_INT5:
    case JSONB_INT: {
      sqlite3_int64 iRes = 0;
      char *z;
      int bNeg = 0;
      char x;
      if( sz==0 ) goto returnfromblob_malformed;
      x = (char)pParse->aBlob[i+n];
      if( x=='-' ){
        if( sz<2 ) goto returnfromblob_malformed;
        n++;
        sz--;
        bNeg = 1;
      }
      z = sqlite3DbStrNDup(db, (const char*)&pParse->aBlob[i+n], (int)sz);
      if( z==0 ) goto returnfromblob_oom;
      rc = sqlite3DecOrHexToI64(z, &iRes);
      sqlite3DbFree(db, z);
      if( rc==0 ){
        if( iRes<0 ){
          /* A hexadecimal literal with 16 significant digits and with the
          ** high-order bit set is a negative integer in SQLite (and hence
          ** iRes comes back as negative) but should be interpreted as a
          ** positive value if it occurs within JSON.  The value is too
          ** large to appear as an SQLite integer so it must be converted
          ** into floating point. */
          double r;
          r = (double)*(sqlite3_uint64*)&iRes;
          sqlite3_result_double(pCtx, bNeg ? -r : r);
        }else{
          sqlite3_result_int64(pCtx, bNeg ? -iRes : iRes);
        }
      }else if( rc==3 && bNeg ){
        sqlite3_result_int64(pCtx, SMALLEST_INT64);
      }else if( rc==1 ){
        goto returnfromblob_malformed;
      }else{
        if( bNeg ){ n--; sz++; }
        goto to_double;
      }
      break;
    }
    case JSONB_FLOAT5:
    case JSONB_FLOAT: {
      double r;
      char *z;
      if( sz==0 ) goto returnfromblob_malformed;
    to_double:
      z = sqlite3DbStrNDup(db, (const char*)&pParse->aBlob[i+n], (int)sz);
      if( z==0 ) goto returnfromblob_oom;
      rc = sqlite3AtoF(z, &r);
      sqlite3DbFree(db, z);
      if( rc<=0 ) goto returnfromblob_malformed;
      sqlite3_result_double(pCtx, r);
      break;
    }
    case JSONB_TEXTRAW:
    case JSONB_TEXT: {
      sqlite3_result_text(pCtx, (char*)&pParse->aBlob[i+n], sz,
                          SQLITE_TRANSIENT);
      break;
    }
    case JSONB_TEXT5:
    case JSONB_TEXTJ: {
      /* Translate JSON formatted string into raw text */
      u32 iIn, iOut;
      const char *z;
      char *zOut;
      u32 nOut = sz;
      z = (const char*)&pParse->aBlob[i+n];
      zOut = sqlite3DbMallocRaw(db, ((u64)nOut)+1);
      if( zOut==0 ) goto returnfromblob_oom;
      for(iIn=iOut=0; iIn<sz; iIn++){
        char c = z[iIn];
        if( c=='\\' ){
          u32 v;
          u32 szEscape = jsonUnescapeOneChar(&z[iIn], sz-iIn, &v);
          if( v<=0x7f ){
            zOut[iOut++] = (char)v;
          }else if( v<=0x7ff ){
            assert( szEscape>=2 );
            zOut[iOut++] = (char)(0xc0 | (v>>6));
            zOut[iOut++] = 0x80 | (v&0x3f);
          }else if( v<0x10000 ){
            assert( szEscape>=3 );
            zOut[iOut++] = 0xe0 | (v>>12);
            zOut[iOut++] = 0x80 | ((v>>6)&0x3f);
            zOut[iOut++] = 0x80 | (v&0x3f);
          }else if( v==JSON_INVALID_CHAR ){
            /* Silently ignore illegal unicode */
          }else{
            assert( szEscape>=4 );
            zOut[iOut++] = 0xf0 | (v>>18);
            zOut[iOut++] = 0x80 | ((v>>12)&0x3f);
            zOut[iOut++] = 0x80 | ((v>>6)&0x3f);
            zOut[iOut++] = 0x80 | (v&0x3f);
          }
          iIn += szEscape - 1;
        }else{
          zOut[iOut++] = c;
        }
      } /* end for() */
      assert( iOut<=nOut );
      zOut[iOut] = 0;
      sqlite3_result_text(pCtx, zOut, iOut, SQLITE_DYNAMIC);
      break;
    }
    case JSONB_ARRAY:
    case JSONB_OBJECT: {
      if( eMode==0 ){
        if( (SQLITE_PTR_TO_INT(sqlite3_user_data(pCtx)) & JSON_BLOB)!=0 ){
          eMode = 2;
        }else{
          eMode = 1;
        }
      }
      if( eMode==2 ){
        sqlite3_result_blob(pCtx, &pParse->aBlob[i], sz+n, SQLITE_TRANSIENT);
      }else{
        jsonReturnTextJsonFromBlob(pCtx, &pParse->aBlob[i], sz+n);
      }
      break;
    }
    default: {
      goto returnfromblob_malformed;
    }
  }
  return;

returnfromblob_oom:
  sqlite3_result_error_nomem(pCtx);
  return;

returnfromblob_malformed:
  sqlite3_result_error(pCtx, "malformed JSON", -1);
  return;
}

/*
** pArg is a function argument that might be an SQL value or a JSON
** value.  Figure out what it is and encode it as a JSONB blob.
** Return the results in pParse.
**
** pParse is uninitialized upon entry.  This routine will handle the
** initialization of pParse.  The result will be contained in
** pParse->aBlob and pParse->nBlob.  pParse->aBlob might be dynamically
** allocated (if pParse->nBlobAlloc is greater than zero) in which case
** the caller is responsible for freeing the space allocated to pParse->aBlob
** when it has finished with it.  Or pParse->aBlob might be a static string
** or a value obtained from sqlite3_value_blob(pArg).
**
** If the argument is a BLOB that is clearly not a JSONB, then this
** function might set an error message in ctx and return non-zero.
** It might also set an error message and return non-zero on an OOM error.
*/
static int jsonFunctionArgToBlob(
  sqlite3_context *ctx,
  sqlite3_value *pArg,
  JsonParse *pParse
){
  int eType = sqlite3_value_type(pArg);
  static u8 aNull[] = { 0x00 };
  memset(pParse, 0, sizeof(pParse[0]));
  pParse->db = sqlite3_context_db_handle(ctx);
  switch( eType ){
    default: {
      pParse->aBlob = aNull;
      pParse->nBlob = 1;
      return 0;
    }
    case SQLITE_BLOB: {
      if( !jsonArgIsJsonb(pArg, pParse) ){
        sqlite3_result_error(ctx, "JSON cannot hold BLOB values", -1);
        return 1;
      }
      break;
    }
    case SQLITE_TEXT: {
      const char *zJson = (const char*)sqlite3_value_text(pArg);
      int nJson = sqlite3_value_bytes(pArg);
      if( zJson==0 ) return 1;
      if( sqlite3_value_subtype(pArg)==JSON_SUBTYPE ){
        pParse->zJson = (char*)zJson;
        pParse->nJson = nJson;
        if( jsonConvertTextToBlob(pParse, ctx) ){
          sqlite3_result_error(ctx, "malformed JSON", -1);
          sqlite3DbFree(pParse->db, pParse->aBlob);
          memset(pParse, 0, sizeof(pParse[0]));
          return 1;
        }
      }else{
        jsonBlobAppendNode(pParse, JSONB_TEXTRAW, nJson, zJson);
      }
      break;
    }
    case SQLITE_FLOAT: {
      double r = sqlite3_value_double(pArg);
      if( NEVER(sqlite3IsNaN(r)) ){
        jsonBlobAppendNode(pParse, JSONB_NULL, 0, 0);
      }else{
        int n = sqlite3_value_bytes(pArg);
        const char *z = (const char*)sqlite3_value_text(pArg);
        if( z==0 ) return 1;
        if( z[0]=='I' ){
          jsonBlobAppendNode(pParse, JSONB_FLOAT, 5, "9e999");
        }else if( z[0]=='-' && z[1]=='I' ){
          jsonBlobAppendNode(pParse, JSONB_FLOAT, 6, "-9e999");
        }else{
          jsonBlobAppendNode(pParse, JSONB_FLOAT, n, z);
        }
      }
      break;
    }
    case SQLITE_INTEGER: {
      int n = sqlite3_value_bytes(pArg);
      const char *z = (const char*)sqlite3_value_text(pArg);
      if( z==0 ) return 1;
      jsonBlobAppendNode(pParse, JSONB_INT, n, z);
      break;
    }
  }
  if( pParse->oom ){
    sqlite3_result_error_nomem(ctx);
    return 1;
  }else{
    return 0;
  }
}

/*
** Generate a path error.
**
** The specifics of the error are determined by the rc argument.
**
**          rc                        error
**  -----------------       ----------------------
**  JSON_LOOKUP_ARRAY       "not an array"
**  JSON_LOOKUP_TOODEEP     "JSON nested too deep"
**  JSON_LOOKUP_ERROR       "malformed JSON"
**  otherwise...            "bad JSON path"
**
** If ctx is not NULL then push the error message into ctx and return NULL.
** If ctx is NULL, then return the text of the error message.
*/
static char *jsonBadPathError(
  sqlite3_context *ctx,     /* The function call containing the error */
  const char *zPath,        /* The path with the problem */
  int rc                    /* Maybe JSON_LOOKUP_NOTARRAY */
){
  char *zMsg;
  if( rc==(int)JSON_LOOKUP_NOTARRAY ){
    zMsg = sqlite3_mprintf("not an array element: %Q", zPath);
  }else if( rc==(int)JSON_LOOKUP_ERROR ){
    zMsg = sqlite3_mprintf("malformed JSON");
  }else if( rc==(int)JSON_LOOKUP_TOODEEP ){
    zMsg = sqlite3_mprintf("JSON path too deep");
  }else{
    zMsg = sqlite3_mprintf("bad JSON path: %Q", zPath);
  }
  if( ctx==0 ) return zMsg;
  if( zMsg ){
    sqlite3_result_error(ctx, zMsg, -1);
    sqlite3_free(zMsg);
  }else{
    sqlite3_result_error_nomem(ctx);
  }
  return 0;
}

/* argv[0] is a BLOB that seems likely to be a JSONB.  Subsequent
** arguments come in pairs where each pair contains a JSON path and
** content to insert or set at that patch.  Do the updates
** and return the result.
**
** The specific operation is determined by eEdit, which can be one
** of JEDIT_INS, JEDIT_REPL, JEDIT_SET, or JEDIT_AINS.
*/
static void jsonInsertIntoBlob(
  sqlite3_context *ctx,
  int argc,
  sqlite3_value **argv,
  int eEdit                /* JEDIT_INS, JEDIT_REPL, JEDIT_SET, JEDIT_AINS */
){
  int i;
  u32 rc = 0;
  const char *zPath = 0;
  int flgs;
  JsonParse *p;
  JsonParse ax;

  assert( (argc&1)==1 );
  flgs = argc==1 ? 0 : JSON_EDITABLE;
  p = jsonParseFuncArg(ctx, argv[0], flgs);
  if( p==0 ) return;
  for(i=1; i<argc-1; i+=2){
    if( sqlite3_value_type(argv[i])==SQLITE_NULL ) continue;
    zPath = (const char*)sqlite3_value_text(argv[i]);
    if( zPath==0 ){
      sqlite3_result_error_nomem(ctx);
      jsonParseFree(p);
      return;
    }
    if( zPath[0]!='$' ) goto jsonInsertIntoBlob_patherror;
    if( jsonFunctionArgToBlob(ctx, argv[i+1], &ax) ){
      jsonParseReset(&ax);
      jsonParseFree(p);
      return;
    }
    if( zPath[1]==0 ){
      if( eEdit==JEDIT_REPL || eEdit==JEDIT_SET ){
        jsonBlobEdit(p, 0, p->nBlob, ax.aBlob, ax.nBlob);
      }
      rc = 0;
   }else{
      p->eEdit = eEdit;
      p->nIns = ax.nBlob;
      p->aIns = ax.aBlob;
      p->delta = 0;
      p->iDepth = 0;
      rc = jsonLookupStep(p, 0, zPath+1, 0);
    }
    jsonParseReset(&ax);
    if( rc==JSON_LOOKUP_NOTFOUND ) continue;
    if( JSON_LOOKUP_ISERROR(rc) ) goto jsonInsertIntoBlob_patherror;
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  return;

jsonInsertIntoBlob_patherror:
  jsonParseFree(p);
  jsonBadPathError(ctx, zPath, rc);
  return;
}

/*
** If pArg is a blob that seems like a JSONB blob, then initialize
** p to point to that JSONB and return TRUE.  If pArg does not seem like
** a JSONB blob, then return FALSE.
**
** For small BLOBs (having no more than 7 bytes of payload) a full
** validity check is done.  So for small BLOBs this routine only returns
** true if the value is guaranteed to be a valid JSONB.  For larger BLOBs
** (8 byte or more of payload) only the size of the outermost element is
** checked to verify that the BLOB is superficially valid JSONB.
**
** A full JSONB validation is done on smaller BLOBs because those BLOBs might
** also be text JSON that has been incorrectly cast into a BLOB.
** (See tag-20240123-a and https://sqlite.org/forum/forumpost/012136abd5)
** If the BLOB is 9 bytes are larger, then it is not possible for the
** superficial size check done here to pass if the input is really text
** JSON so we do not need to look deeper in that case.
**
** Why we only need to do full JSONB validation for smaller BLOBs:
**
** The first byte of valid JSON text must be one of: '{', '[', '"', ' ', '\n',
** '\r', '\t', '-', or a digit '0' through '9'.  Of these, only a subset
** can also be the first byte of JSONB:  '{', '[', and digits '3'
** through '9'.  In every one of those cases, the payload size is 7 bytes
** or less.  So if we do full JSONB validation for every BLOB where the
** payload is less than 7 bytes, we will never get a false positive for
** JSONB on an input that is really text JSON.
*/
static int jsonArgIsJsonb(sqlite3_value *pArg, JsonParse *p){
  u32 n, sz = 0;
  u8 c;
  if( sqlite3_value_type(pArg)!=SQLITE_BLOB ) return 0;
  p->aBlob = (u8*)sqlite3_value_blob(pArg);
  p->nBlob = (u32)sqlite3_value_bytes(pArg);
  if( p->nBlob>0
   && ALWAYS(p->aBlob!=0)
   && ((c = p->aBlob[0]) & 0x0f)<=JSONB_OBJECT
   && (n = jsonbPayloadSize(p, 0, &sz))>0
   && sz+n==p->nBlob
   && ((c & 0x0f)>JSONB_FALSE || sz==0)
   && (sz>7
      || (c!=0x7b && c!=0x5b && !sqlite3Isdigit(c))
      || jsonbValidityCheck(p, 0, p->nBlob, 1)==0)
  ){
    return 1;
  }
  p->aBlob = 0;
  p->nBlob = 0;
  return 0;
}

/*
** Generate a JsonParse object, containing valid JSONB in aBlob and nBlob,
** from the SQL function argument pArg.  Return a pointer to the new
** JsonParse object.
**
** Ownership of the new JsonParse object is passed to the caller.  The
** caller should invoke jsonParseFree() on the return value when it
** has finished using it.
**
** If any errors are detected, an appropriate error messages is set
** using sqlite3_result_error() or the equivalent and this routine
** returns NULL.  This routine also returns NULL if the pArg argument
** is an SQL NULL value, but no error message is set in that case.  This
** is so that SQL functions that are given NULL arguments will return
** a NULL value.
*/
static JsonParse *jsonParseFuncArg(
  sqlite3_context *ctx,
  sqlite3_value *pArg,
  u32 flgs
){
  int eType;                   /* Datatype of pArg */
  JsonParse *p = 0;            /* Value to be returned */
  JsonParse *pFromCache = 0;   /* Value taken from cache */
  sqlite3 *db;                 /* The database connection */

  assert( ctx!=0 );
  eType = sqlite3_value_type(pArg);
  if( eType==SQLITE_NULL ){
    return 0;
  }
  pFromCache = jsonCacheSearch(ctx, pArg);
  if( pFromCache ){
    pFromCache->nJPRef++;
    if( (flgs & JSON_EDITABLE)==0 ){
      return pFromCache;
    }
  }
  db = sqlite3_context_db_handle(ctx);
rebuild_from_cache:
  p = sqlite3DbMallocZero(db, sizeof(*p));
  if( p==0 ) goto json_pfa_oom;
  memset(p, 0, sizeof(*p));
  p->db = db;
  p->nJPRef = 1;
  if( pFromCache!=0 ){
    u32 nBlob = pFromCache->nBlob;
    p->aBlob = sqlite3DbMallocRaw(db, nBlob);
    if( p->aBlob==0 ) goto json_pfa_oom;
    memcpy(p->aBlob, pFromCache->aBlob, nBlob);
    p->nBlobAlloc = p->nBlob = nBlob;
    p->hasNonstd = pFromCache->hasNonstd;
    jsonParseFree(pFromCache);
    return p;
  }
  if( eType==SQLITE_BLOB ){
    if( jsonArgIsJsonb(pArg,p) ){
      if( (flgs & JSON_EDITABLE)!=0 && jsonBlobMakeEditable(p, 0)==0 ){
        goto json_pfa_oom;
      }
      return p;
    }
    /* If the blob is not valid JSONB, fall through into trying to cast
    ** the blob into text which is then interpreted as JSON.  (tag-20240123-a)
    **
    ** This goes against all historical documentation about how the SQLite
    ** JSON functions were suppose to work.  From the beginning, blob was
    ** reserved for expansion and a blob value should have raised an error.
    ** But it did not, due to a bug.  And many applications came to depend
    ** upon this buggy behavior, especially when using the CLI and reading
    ** JSON text using readfile(), which returns a blob.  For this reason
    ** we will continue to support the bug moving forward.
    ** See for example https://sqlite.org/forum/forumpost/012136abd5292b8d
    */
  }
  p->zJson = (char*)sqlite3_value_text(pArg);
  p->nJson = sqlite3_value_bytes(pArg);
  if( db->mallocFailed ) goto json_pfa_oom;
  if( p->nJson==0 ) goto json_pfa_malformed;
  assert( p->zJson!=0 );
  if( jsonConvertTextToBlob(p, (flgs & JSON_KEEPERROR) ? 0 : ctx) ){
    if( flgs & JSON_KEEPERROR ){
      p->nErr = 1;
      return p;
    }else{
      jsonParseFree(p);
      return 0;
    }
  }else{
    int isRCStr = sqlite3ValueIsOfClass(pArg, sqlite3RCStrUnref);
    int rc;
    if( !isRCStr ){
      char *zNew = sqlite3RCStrNew( p->nJson );
      if( zNew==0 ) goto json_pfa_oom;
      memcpy(zNew, p->zJson, p->nJson);
      p->zJson = zNew;
      p->zJson[p->nJson] = 0;
    }else{
      sqlite3RCStrRef(p->zJson);
    }
    p->bJsonIsRCStr = 1;
    rc = jsonCacheInsert(ctx, p);
    if( rc==SQLITE_NOMEM ) goto json_pfa_oom;
    if( flgs & JSON_EDITABLE ){
      pFromCache = p;
      p = 0;
      goto rebuild_from_cache;
    }
  }
  return p;

json_pfa_malformed:
  if( flgs & JSON_KEEPERROR ){
    p->nErr = 1;
    return p;
  }else{
    jsonParseFree(p);
    sqlite3_result_error(ctx, "malformed JSON", -1);
    return 0;
  }

json_pfa_oom:
  jsonParseFree(pFromCache);
  jsonParseFree(p);
  sqlite3_result_error_nomem(ctx);
  return 0;
}

/*
** Make the return value of a JSON function either the raw JSONB blob
** or make it JSON text, depending on whether the JSON_BLOB flag is
** set on the function.
*/
static void jsonReturnParse(
  sqlite3_context *ctx,
  JsonParse *p
){
  int flgs;
  if( p->oom ){
    sqlite3_result_error_nomem(ctx);
    return;
  }
  flgs = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  if( flgs & JSON_BLOB ){
    if( p->nBlobAlloc>0 && !p->bReadOnly ){
      sqlite3_result_blob(ctx, p->aBlob, p->nBlob, SQLITE_DYNAMIC);
      p->nBlobAlloc = 0;
    }else{
      sqlite3_result_blob(ctx, p->aBlob, p->nBlob, SQLITE_TRANSIENT);
    }
  }else{
    JsonString s;
    jsonStringInit(&s, ctx);
    p->delta = 0;
    jsonTranslateBlobToText(p, 0, &s);
    jsonReturnString(&s, p, ctx);
    sqlite3_result_subtype(ctx, JSON_SUBTYPE);
  }
}

/****************************************************************************
** SQL functions used for testing and debugging
****************************************************************************/

#if SQLITE_DEBUG
/*
** Decode JSONB bytes in aBlob[] starting at iStart through but not
** including iEnd.  Indent the
** content by nIndent spaces.
*/
static void jsonDebugPrintBlob(
  JsonParse *pParse, /* JSON content */
  u32 iStart,        /* Start rendering here */
  u32 iEnd,          /* Do not render this byte or any byte after this one */
  int nIndent,       /* Indent by this many spaces */
  sqlite3_str *pOut  /* Generate output into this sqlite3_str object */
){
  while( iStart<iEnd ){
    u32 i, n, nn, sz = 0;
    int showContent = 1;
    u8 x = pParse->aBlob[iStart] & 0x0f;
    u32 savedNBlob = pParse->nBlob;
    sqlite3_str_appendf(pOut, "%5d:%*s", iStart, nIndent, "");
    if( pParse->nBlobAlloc>pParse->nBlob ){
      pParse->nBlob = pParse->nBlobAlloc;
    }
    nn = n = jsonbPayloadSize(pParse, iStart, &sz);
    if( nn==0 ) nn = 1;
    if( sz>0 && x<JSONB_ARRAY ){
      nn += sz;
    }
    for(i=0; i<nn; i++){
      sqlite3_str_appendf(pOut, " %02x", pParse->aBlob[iStart+i]);
    }
    if( n==0 ){
      sqlite3_str_appendf(pOut, "   ERROR invalid node size\n");
      iStart = n==0 ? iStart+1 : iEnd;
      continue;
    }
    pParse->nBlob = savedNBlob;
    if( iStart+n+sz>iEnd ){
      iEnd = iStart+n+sz;
      if( iEnd>pParse->nBlob ){
        if( pParse->nBlobAlloc>0 && iEnd>pParse->nBlobAlloc ){
          iEnd = pParse->nBlobAlloc;
        }else{
          iEnd = pParse->nBlob;
        }
      }
    }
    sqlite3_str_appendall(pOut,"  <-- ");
    switch( x ){
      case JSONB_NULL:     sqlite3_str_appendall(pOut,"null"); break;
      case JSONB_TRUE:     sqlite3_str_appendall(pOut,"true"); break;
      case JSONB_FALSE:    sqlite3_str_appendall(pOut,"false"); break;
      case JSONB_INT:      sqlite3_str_appendall(pOut,"int"); break;
      case JSONB_INT5:     sqlite3_str_appendall(pOut,"int5"); break;
      case JSONB_FLOAT:    sqlite3_str_appendall(pOut,"float"); break;
      case JSONB_FLOAT5:   sqlite3_str_appendall(pOut,"float5"); break;
      case JSONB_TEXT:     sqlite3_str_appendall(pOut,"text"); break;
      case JSONB_TEXTJ:    sqlite3_str_appendall(pOut,"textj"); break;
      case JSONB_TEXT5:    sqlite3_str_appendall(pOut,"text5"); break;
      case JSONB_TEXTRAW:  sqlite3_str_appendall(pOut,"textraw"); break;
      case JSONB_ARRAY: {
        sqlite3_str_appendf(pOut,"array, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, iStart+n, iStart+n+sz, nIndent+2, pOut);
        showContent = 0;
        break;
      }
      case JSONB_OBJECT: {
        sqlite3_str_appendf(pOut, "object, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, iStart+n, iStart+n+sz, nIndent+2, pOut);
        showContent = 0;
        break;
      }
      default: {
        sqlite3_str_appendall(pOut, "ERROR: unknown node type\n");
        showContent = 0;
        break;
      }
    }
    if( showContent ){
      if( sz==0 && x<=JSONB_FALSE ){
        sqlite3_str_append(pOut, "\n", 1);
      }else{
        u32 j;
        sqlite3_str_appendall(pOut, ": \"");
        for(j=iStart+n; j<iStart+n+sz; j++){
          u8 c = pParse->aBlob[j];
          if( c<0x20 || c>=0x7f ) c = '.';
          sqlite3_str_append(pOut, (char*)&c, 1);
        }
        sqlite3_str_append(pOut, "\"\n", 2);
      }
    }
    iStart += n + sz;
  }
}
static void jsonShowParse(JsonParse *pParse){
  sqlite3_str out;
  char zBuf[1000];
  if( pParse==0 ){
    printf("NULL pointer\n");
    return;
  }else{
    printf("nBlobAlloc = %u\n", pParse->nBlobAlloc);
    printf("nBlob = %u\n", pParse->nBlob);
    printf("delta = %d\n", pParse->delta);
    if( pParse->nBlob==0 ) return;
    printf("content (bytes 0..%u):\n", pParse->nBlob-1);
  }
  sqlite3StrAccumInit(&out, 0, zBuf, sizeof(zBuf), 1000000);
  jsonDebugPrintBlob(pParse, 0, pParse->nBlob, 0, &out);
  printf("%s", sqlite3_str_value(&out));
  sqlite3_str_reset(&out);
}
#endif /* SQLITE_DEBUG */

#ifdef SQLITE_DEBUG
/*
** SQL function:   json_parse(JSON)
**
** Parse JSON using jsonParseFuncArg().  Return text that is a
** human-readable dump of the binary JSONB for the input parameter.
*/
static void jsonParseFunc(
  sqlite3_context *ctx,
  int argc,
  sqlite3_value **argv
){
  JsonParse *p;        /* The parse */
  sqlite3_str out;

  assert( argc>=1 );
  sqlite3StrAccumInit(&out, 0, 0, 0, 1000000);
  p = jsonParseFuncArg(ctx, argv[0], 0);
  if( p==0 ) return;
  if( argc==1 ){
    jsonDebugPrintBlob(p, 0, p->nBlob, 0, &out);
    sqlite3_result_text64(ctx,out.zText,out.nChar,SQLITE_TRANSIENT,SQLITE_UTF8);
  }else{
    jsonShowParse(p);
  }
  jsonParseFree(p);
  sqlite3_str_reset(&out);
}
#endif /* SQLITE_DEBUG */

/****************************************************************************
** Scalar SQL function implementations
****************************************************************************/

/*
** Implementation of the json_quote(VALUE) function.  Return a JSON value
** corresponding to the SQL value input.  Mostly this means putting
** double-quotes around strings and returning the unquoted string "null"
** when given a NULL input.
*/
static void jsonQuoteFunc(
  sqlite3_context *ctx,
  int argc,
  sqlite3_value **argv
){
  JsonString jx;
  UNUSED_PARAMETER(argc);

  jsonStringInit(&jx, ctx);
  jsonAppendSqlValue(&jx, argv[0]);
  jsonReturnString(&jx, 0, 0);
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}

/*
** Implementation of the json_array(VALUE,...) function.  Return a JSON
** array that contains all values given in arguments.  Or if any argument
** is a BLOB, throw an error.
*/
static void jsonArrayFunc(
  sqlite3_context *ctx,
  int argc,
  sqlite3_value **argv
){
  int i;
  JsonString jx;

  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '[');
  for(i=0; i<argc; i++){
    jsonAppendSeparator(&jx);
    jsonAppendSqlValue(&jx, argv[i]);
  }
  jsonAppendChar(&jx, ']');
  jsonReturnString(&jx, 0, 0);
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}

/*
** json_array_length(JSON)
** json_array_length(JSON, PATH)
**
** Return the number of elements in the top-level JSON array.
** Return 0 if the input is not a well-formed JSON array.
*/
static void jsonArrayLengthFunc(
  sqlite3_context *ctx,
  int argc,
  sqlite3_value **argv
){
  JsonParse *p;          /* The parse */
  sqlite3_int64 cnt = 0;
  u32 i;
  u8 eErr = 0;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  if( p==0 ) return;
  if( argc==2 ){
    const char *zPath = (const char*)sqlite3_value_text(argv[1]);
    if( zPath==0 ){
      jsonParseFree(p);
      return;
    }
    i = jsonLookupStep(p, 0, zPath[0]=='$' ? zPath+1 : "@", 0);
    if( JSON_LOOKUP_ISERROR(i) ){
      if( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }else{
        jsonBadPathError(ctx, zPath, i);
      }
      eErr = 1;
      i = 0;
    }
  }else{
    i = 0;
  }
  if( (p->aBlob[i] & 0x0f)==JSONB_ARRAY ){
    cnt = jsonbArrayCount(p, i);
  }
  if( !eErr ) sqlite3_result_int64(ctx, cnt);
  jsonParseFree(p);
}

/* True if the string is all alphanumerics and underscores */
static int jsonAllAlphanum(const char *z, int n){
  int i;
  for(i=0; i<n && (sqlite3Isalnum(z[i]) || z[i]=='_'); i++){}
  return i==n;
}

/*
** json_extract(JSON, PATH, ...)
** "->"(JSON,PATH)
** "->>"(JSON,PATH)
**
** Return the element described by PATH.  Return NULL if that PATH element
** is not found.
**
** If JSON_JSON is set or if more that one PATH argument is supplied then
** always return a JSON representation of the result.  If JSON_SQL is set,
** then always return an SQL representation of the result.  If neither flag
** is present and argc==2, then return JSON for objects and arrays and SQL
** for all other values.
**
** When multiple PATH arguments are supplied, the result is a JSON array
** containing the result of each PATH.
**
** Abbreviated JSON path expressions are allows if JSON_ABPATH, for
** compatibility with PG.
*/
static void jsonExtractFunc(
  sqlite3_context *ctx,
  int argc,
  sqlite3_value **argv
){
  JsonParse *p = 0;      /* The parse */
  int flags;             /* Flags associated with the function */
  int i;                 /* Loop counter */
  JsonString jx;         /* String for array result */

  if( argc<2 ) return;
  p = jsonParseFuncArg(ctx, argv[0], 0);
  if( p==0 ) return;
  flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  jsonStringInit(&jx, ctx);
  if( argc>2 ){
    jsonAppendChar(&jx, '[');
  }
  for(i=1; i<argc; i++){
    /* With a single PATH argument */
    const char *zPath = (const char*)sqlite3_value_text(argv[i]);
    int nPath;
    u32 j;
    if( zPath==0 ) goto json_extract_error;
    nPath = sqlite3Strlen30(zPath);
    if( zPath[0]=='$' ){
      j = jsonLookupStep(p, 0, zPath+1, 0);
    }else if( (flags & JSON_ABPATH) ){
      /* The -> and ->> operators accept abbreviated PATH arguments.  This
      ** is mostly for compatibility with PostgreSQL, but also for
      ** convenience.
      **
      **     NUMBER   ==>  $[NUMBER]     // PG compatible
      **     LABEL    ==>  $.LABEL       // PG compatible
      **     [NUMBER] ==>  $[NUMBER]     // Not PG.  Purely for convenience
      **
      ** Updated 2024-05-27:  If the NUMBER is negative, then PG counts from
      ** the right of the array.  Hence for negative NUMBER:
      **
      **     NUMBER   ==>  $[#NUMBER]    // PG compatible
      */
      jsonStringInit(&jx, ctx);
      if( sqlite3_value_type(argv[i])==SQLITE_INTEGER ){
        jsonAppendRawNZ(&jx, "[", 1);
        if( zPath[0]=='-' ) jsonAppendRawNZ(&jx,"#",1);
        jsonAppendRaw(&jx, zPath, nPath);
        jsonAppendRawNZ(&jx, "]", 2);
      }else if( jsonAllAlphanum(zPath, nPath) ){
        jsonAppendRawNZ(&jx, ".", 1);
        jsonAppendRaw(&jx, zPath, nPath);
      }else if( zPath[0]=='[' && nPath>=3 && zPath[nPath-1]==']' ){
        jsonAppendRaw(&jx, zPath, nPath);
      }else{
        jsonAppendRawNZ(&jx, ".\"", 2);
        jsonAppendRaw(&jx, zPath, nPath);
        jsonAppendRawNZ(&jx, "\"", 1);
      }
      jsonStringTerminate(&jx);
      j = jsonLookupStep(p, 0, jx.zBuf, 0);
      jsonStringReset(&jx);
    }else{
      jsonBadPathError(ctx, zPath, 0);
      goto json_extract_error;
    }
    if( j<p->nBlob ){
      if( argc==2 ){
        if( flags & JSON_JSON ){
          jsonStringInit(&jx, ctx);
          jsonTranslateBlobToText(p, j, &jx);
          jsonReturnString(&jx, 0, 0);
          jsonStringReset(&jx);
          assert( (flags & JSON_BLOB)==0 );
          sqlite3_result_subtype(ctx, JSON_SUBTYPE);
        }else{
          jsonReturnFromBlob(p, j, ctx, 0);
          if( (flags & (JSON_SQL|JSON_BLOB))==0
           && (p->aBlob[j]&0x0f)>=JSONB_ARRAY
          ){
            sqlite3_result_subtype(ctx, JSON_SUBTYPE);
          }
        }
      }else{
        jsonAppendSeparator(&jx);
        jsonTranslateBlobToText(p, j, &jx);
      }
    }else if( j==JSON_LOOKUP_NOTFOUND ){
      if( argc==2 ){
        goto json_extract_error;  /* Return NULL if not found */
      }else{
        jsonAppendSeparator(&jx);
        jsonAppendRawNZ(&jx, "null", 4);
      }
    }else{
      jsonBadPathError(ctx, zPath, j);
      goto json_extract_error;
    }
  }
  if( argc>2 ){
    jsonAppendChar(&jx, ']');
    jsonReturnString(&jx, 0, 0);
    if( (flags & JSON_BLOB)==0 ){
      sqlite3_result_subtype(ctx, JSON_SUBTYPE);
    }
  }
json_extract_error:
  jsonStringReset(&jx);
  jsonParseFree(p);
  return;
}

/*
** Return codes for jsonMergePatch()
*/
#define JSON_MERGE_OK          0     /* Success */
#define JSON_MERGE_BADTARGET   1     /* Malformed TARGET blob */
#define JSON_MERGE_BADPATCH    2     /* Malformed PATCH blob */
#define JSON_MERGE_OOM         3     /* Out-of-memory condition */
#define JSON_MERGE_TOODEEP     4     /* Nested too deep */

/*
** RFC-7396 MergePatch for two JSONB blobs.
**
** pTarget is the target. pPatch is the patch.  The target is updated
** in place.  The patch is read-only.
**
** The original RFC-7396 algorithm is this:
**
**   define MergePatch(Target, Patch):
**     if Patch is an Object:
**       if Target is not an Object:
**         Target = {} # Ignore the contents and set it to an empty Object
**     for each Name/Value pair in Patch:
**         if Value is null:
**           if Name exists in Target:
**             remove the Name/Value pair from Target
**         else:
**           Target[Name] = MergePatch(Target[Name], Value)
**       return Target
**     else:
**       return Patch
**
** Here is an equivalent algorithm restructured to show the actual
** implementation:
**
** 01   define MergePatch(Target, Patch):
** 02      if Patch is not an Object:
** 03         return Patch
** 04      else: // if Patch is an Object
** 05         if Target is not an Object:
** 06            Target = {}
** 07      for each Name/Value pair in Patch:
** 08         if Name exists in Target:
** 09            if Value is null:
** 10               remove the Name/Value pair from Target
** 11            else
** 12               Target[name] = MergePatch(Target[Name], Value)
** 13         else if Value is not NULL:
** 14            if Value is not an Object:
** 15               Target[name] = Value
** 16            else:
** 17               Target[name] = MergePatch('{}',value)
** 18      return Target
**  |
**  ^---- Line numbers referenced in comments in the implementation
*/
static int jsonMergePatch(
  JsonParse *pTarget,      /* The JSON parser that contains the TARGET */
  u32 iTarget,             /* Index of TARGET in pTarget->aBlob[] */
  const JsonParse *pPatch, /* The PATCH */
  u32 iPatch,              /* Index of PATCH in pPatch->aBlob[] */
  u32 iDepth               /* Nesting depth */
){
  u8 x;             /* Type of a single node */
  u32 n, sz=0;      /* Return values from jsonbPayloadSize() */
  u32 iTCursor;     /* Cursor position while scanning the target object */
  u32 iTStart;      /* First label in the target object */
  u32 iTEndBE;      /* Original first byte past end of target, before edit */
  u32 iTEnd;        /* Current first byte past end of target */
  u8 eTLabel;       /* Node type of the target label */
  u32 iTLabel = 0;  /* Index of the label */
  u32 nTLabel = 0;  /* Header size in bytes for the target label */
  u32 szTLabel = 0; /* Size of the target label payload */
  u32 iTValue = 0;  /* Index of the target value */
  u32 nTValue = 0;  /* Header size of the target value */
  u32 szTValue = 0; /* Payload size for the target value */

  u32 iPCursor;     /* Cursor position while scanning the patch */
  u32 iPEnd;        /* First byte past the end of the patch */
  u8 ePLabel;       /* Node type of the patch label */
  u32 iPLabel;      /* Start of patch label */
  u32 nPLabel;      /* Size of header on the patch label */
  u32 szPLabel;     /* Payload size of the patch label */
  u32 iPValue;      /* Start of patch value */
  u32 nPValue;      /* Header size for the patch value */
  u32 szPValue;     /* Payload size of the patch value */

  assert( iTarget>=0 && iTarget<pTarget->nBlob );
  assert( iPatch>=0 && iPatch<pPatch->nBlob );
  x = pPatch->aBlob[iPatch] & 0x0f;
  if( x!=JSONB_OBJECT ){  /* Algorithm line 02 */
    u32 szPatch;        /* Total size of the patch, header+payload */
    u32 szTarget;       /* Total size of the target, header+payload */
    n = jsonbPayloadSize(pPatch, iPatch, &sz);
    szPatch = n+sz;
    sz = 0;
    n = jsonbPayloadSize(pTarget, iTarget, &sz);
    szTarget = n+sz;
    jsonBlobEdit(pTarget, iTarget, szTarget, pPatch->aBlob+iPatch, szPatch);
    return pTarget->oom ? JSON_MERGE_OOM : JSON_MERGE_OK;  /* Line 03 */
  }
  x = pTarget->aBlob[iTarget] & 0x0f;
  if( x!=JSONB_OBJECT ){  /* Algorithm line 05 */
    n = jsonbPayloadSize(pTarget, iTarget, &sz);
    jsonBlobEdit(pTarget, iTarget+n, sz, 0, 0);
    x = pTarget->aBlob[iTarget];
    pTarget->aBlob[iTarget] = (x & 0xf0) | JSONB_OBJECT;
  }
  n = jsonbPayloadSize(pPatch, iPatch, &sz);
  if( NEVER(n==0) ) return JSON_MERGE_BADPATCH;
  iPCursor = iPatch+n;
  iPEnd = iPCursor+sz;
  n = jsonbPayloadSize(pTarget, iTarget, &sz);
  if( NEVER(n==0) ) return JSON_MERGE_BADTARGET;
  iTStart = iTarget+n;
  iTEndBE = iTStart+sz;

  while( iPCursor<iPEnd ){  /* Algorithm line 07 */
    iPLabel = iPCursor;
    ePLabel = pPatch->aBlob[iPCursor] & 0x0f;
    if( ePLabel<JSONB_TEXT || ePLabel>JSONB_TEXTRAW ){
      return JSON_MERGE_BADPATCH;
    }
    nPLabel = jsonbPayloadSize(pPatch, iPCursor, &szPLabel);
    if( nPLabel==0 ) return JSON_MERGE_BADPATCH;
    iPValue = iPCursor + nPLabel + szPLabel;
    if( iPValue>=iPEnd ) return JSON_MERGE_BADPATCH;
    nPValue = jsonbPayloadSize(pPatch, iPValue, &szPValue);
    if( nPValue==0 ) return JSON_MERGE_BADPATCH;
    iPCursor = iPValue + nPValue + szPValue;
    if( iPCursor>iPEnd ) return JSON_MERGE_BADPATCH;

    iTCursor = iTStart;
    iTEnd = iTEndBE + pTarget->delta;
    while( iTCursor<iTEnd ){
      int isEqual;   /* true if the patch and target labels match */
      iTLabel = iTCursor;
      eTLabel = pTarget->aBlob[iTCursor] & 0x0f;
      if( eTLabel<JSONB_TEXT || eTLabel>JSONB_TEXTRAW ){
        return JSON_MERGE_BADTARGET;
      }
      nTLabel = jsonbPayloadSize(pTarget, iTCursor, &szTLabel);
      if( nTLabel==0 ) return JSON_MERGE_BADTARGET;
      iTValue = iTLabel + nTLabel + szTLabel;
      if( iTValue>=iTEnd ) return JSON_MERGE_BADTARGET;
      nTValue = jsonbPayloadSize(pTarget, iTValue, &szTValue);
      if( nTValue==0 ) return JSON_MERGE_BADTARGET;
      if( iTValue + nTValue + szTValue > iTEnd ) return JSON_MERGE_BADTARGET;
      isEqual = jsonLabelCompare(
                   (const char*)&pPatch->aBlob[iPLabel+nPLabel],
                   szPLabel,
                   (ePLabel==JSONB_TEXT || ePLabel==JSONB_TEXTRAW),
                   (const char*)&pTarget->aBlob[iTLabel+nTLabel],
                   szTLabel,
                   (eTLabel==JSONB_TEXT || eTLabel==JSONB_TEXTRAW));
      if( isEqual ) break;
      iTCursor = iTValue + nTValue + szTValue;
    }
    x = pPatch->aBlob[iPValue] & 0x0f;
    if( iTCursor<iTEnd ){
      /* A match was found.  Algorithm line 08 */
      if( x==0 ){
        /* Patch value is NULL.  Algorithm line 09 */
        jsonBlobEdit(pTarget, iTLabel, nTLabel+szTLabel+nTValue+szTValue, 0,0);
        /*  vvvvvv----- No OOM on a delete-only edit */
        if( NEVER(pTarget->oom) ) return JSON_MERGE_OOM;
      }else{
        /* Algorithm line 12 */
        int rc, savedDelta = pTarget->delta;
        pTarget->delta = 0;
        if( iDepth>=JSON_MAX_DEPTH ) return JSON_MERGE_TOODEEP;
        rc = jsonMergePatch(pTarget, iTValue, pPatch, iPValue, iDepth+1);
        if( rc ) return rc;
        pTarget->delta += savedDelta;
      }
    }else if( x>0 ){  /* Algorithm line 13 */
      /* No match and patch value is not NULL */
      u32 szNew = szPLabel+nPLabel;
      if( (pPatch->aBlob[iPValue] & 0x0f)!=JSONB_OBJECT ){  /* Line 14 */
        jsonBlobEdit(pTarget, iTEnd, 0, 0, szPValue+nPValue+szNew);
        if( pTarget->oom ) return JSON_MERGE_OOM;
        memcpy(&pTarget->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        memcpy(&pTarget->aBlob[iTEnd+szNew],
               &pPatch->aBlob[iPValue], szPValue+nPValue);
      }else{
        int rc, savedDelta;
        jsonBlobEdit(pTarget, iTEnd, 0, 0, szNew+1);
        if( pTarget->oom ) return JSON_MERGE_OOM;
        memcpy(&pTarget->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        pTarget->aBlob[iTEnd+szNew] = 0x00;
        savedDelta = pTarget->delta;
        pTarget->delta = 0;
        if( iDepth>=JSON_MAX_DEPTH ) return JSON_MERGE_TOODEEP;
        rc = jsonMergePatch(pTarget, iTEnd+szNew,pPatch,iPValue,iDepth+1);
        if( rc ) return rc;
        pTarget->delta += savedDelta;
      }
    }
  }
  if( pTarget->delta ) jsonAfterEditSizeAdjust(pTarget, iTarget);
  return pTarget->oom ? JSON_MERGE_OOM : JSON_MERGE_OK;
}


/*
** Implementation of the json_mergepatch(JSON1,JSON2) function.  Return a JSON
** object that is the result of running the RFC 7396 MergePatch() algorithm
** on the two arguments.
*/
static void jsonPatchFunc(
  sqlite3_context *ctx,
  int argc,
  sqlite3_value **argv
){
  JsonParse *pTarget;    /* The TARGET */
  JsonParse *pPatch;     /* The PATCH */
  int rc;                /* Result code */

  UNUSED_PARAMETER(argc);
  assert( argc==2 );
  pTarget = jsonParseFuncArg(ctx, argv[0], JSON_EDITABLE);
  if( pTarget==0 ) return;
  pPatch = jsonParseFuncArg(ctx, argv[1], 0);
  if( pPatch ){
    rc = jsonMergePatch(pTarget, 0, pPatch, 0, 0);
    if( rc==JSON_MERGE_OK ){
      jsonReturnParse(ctx, pTarget);
    }else if( rc==JSON_MERGE_OOM ){
      sqlite3_result_error_nomem(ctx);
    }else if( rc==JSON_MERGE_TOODEEP ){
      sqlite3_result_error(ctx, "JSON nested too deep", -1);
    }else{
      sqlite3_result_error(ctx, "malformed JSON", -1);
    }
    jsonParseFree(pPatch);
  }
  jsonParseFree(pTarget);
}


/*
** Implementation of the json_object(NAME,VALUE,...) function.  Return a JSON
** object that contains all name/value given in arguments.  Or if any name
** is not a string or if any value is a BLOB, throw an error.
*/
static void jsonObjectFunc(
  sqlite3_context *ctx,
  int argc,
  sqlite3_value **argv
){
  int i;
  JsonString jx;
  const char *z;
  u32 n;

  if( argc&1 ){
    sqlite3_result_error(ctx, "json_object() requires an even number "
                                  "of arguments", -1);
    return;
  }
  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '{');
  for(i=0; i<argc; i+=2){
    if( sqlite3_value_type(argv[i])!=SQLITE_TEXT ){
      sqlite3_result_error(ctx, "json_object() labels must be TEXT", -1);
      jsonStringReset(&jx);
      return;
    }
    jsonAppendSeparator(&jx);
    z = (const char*)sqlite3_value_text(argv[i]);
    n = sqlite3_value_bytes(argv[i]);
    jsonAppendString(&jx, z, n);
    jsonAppendChar(&jx, ':');
    jsonAppendSqlValue(&jx, argv[i+1]);
  }
  jsonAppendChar(&jx, '}');
  jsonReturnString(&jx, 0, 0);
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}


/*
** json_remove(JSON, PATH, ...)
**
** Remove the named elements from JSON and return the result.  malformed
** JSON or PATH arguments result in an error.
*/
static void jsonRemoveFunc(
  sqlite3_context *ctx,
  int argc,
  sqlite3_value **argv
){
  JsonParse *p;          /* The parse */
  const char *zPath = 0; /* Path of element to be removed */
  int i;                 /* Loop counter */
  u32 rc;                /* Subroutine return code */

  if( argc<1 ) return;
  p = jsonParseFuncArg(ctx, argv[0], argc>1 ? JSON_EDITABLE : 0);
  if( p==0 ) return;
  for(i=1; i<argc; i++){
    zPath = (const char*)sqlite3_value_text(argv[i]);
    if( zPath==0 ){
      goto json_remove_done;
    }
    if( zPath[0]!='$' ){
      goto json_remove_patherror;
    }
    if( zPath[1]==0 ){
      /* json_remove(j,'$') returns NULL */
      goto json_remove_done;
    }
    p->eEdit = JEDIT_DEL;
    p->delta = 0;
    rc = jsonLookupStep(p, 0, zPath+1, 0);
    if( JSON_LOOKUP_ISERROR(rc) ){
      if( rc==JSON_LOOKUP_NOTFOUND ){
        continue;  /* No-op */
      }else{
        jsonBadPathError(ctx, zPath, rc);
      }
      goto json_remove_done;
    }
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  return;

json_remove_patherror:
  jsonBadPathError(ctx, zPath, 0);

json_remove_done:
  jsonParseFree(p);
  return;
}

/*
** json_replace(JSON, PATH, VALUE, ...)
**
** Replace the value at PATH with VALUE.  If PATH does not already exist,
** this routine is a no-op.  If JSON or PATH is malformed, throw an error.
*/
static void jsonReplaceFunc(
  sqlite3_context *ctx,
  int argc,
  sqlite3_value **argv
){
  if( argc<1 ) return;
  if( (argc&1)==0 ) {
    jsonWrongNumArgs(ctx, "replace");
    return;
  }
  jsonInsertIntoBlob(ctx, argc, argv, JEDIT_REPL);
}


/*
** json_set(JSON, PATH, VALUE, ...)
**
** Set the value at PATH to VALUE.  Create the PATH if it does not already
** exist.  Overwrite existing values that do exist.
** If JSON or PATH is malformed, throw an error.
**
** json_insert(JSON, PATH, VALUE, ...)
**
** Create PATH and initialize it to VALUE.  If PATH already exists, this
** routine is a no-op.  If JSON or PATH is malformed, throw an error.
*/
static void jsonSetFunc(
  sqlite3_context *ctx,
  int argc,
  sqlite3_value **argv
){
  int flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  int eInsType = JSON_INSERT_TYPE(flags);
  static const char *azInsType[] = { "insert", "set", "array_insert" };
  static const u8 aEditType[] = { JEDIT_INS, JEDIT_SET, JEDIT_AINS };

  if( argc<1 ) return;
  assert( eInsType>=0 && eInsType<=2 );
  if( (argc&1)==0 ) {
    jsonWrongNumArgs(ctx, azInsType[eInsType]);
    return;
  }
  jsonInsertIntoBlob(ctx, argc, argv, aEditType[eInsType]);
}

/*
** json_type(JSON)
** json_type(JSON, PATH)
**
** Return the top-level "type" of a JSON string.  json_type() raises an
** error if either the JSON or PATH inputs are not well-formed.
*/
static void jsonTypeFunc(
  sqlite3_context *ctx,
  int argc,
  sqlite3_value **argv
){
  JsonParse *p;          /* The parse */
  const char *zPath = 0;
  u32 i;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  if( p==0 ) return;
  if( argc==2 ){
    zPath = (const char*)sqlite3_value_text(argv[1]);
    if( zPath==0 ) goto json_type_done;
    if( zPath[0]!='$' ){
      jsonBadPathError(ctx, zPath, 0);
      goto json_type_done;
    }
    i = jsonLookupStep(p, 0, zPath+1, 0);
    if( JSON_LOOKUP_ISERROR(i) ){
      if( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }else{
        jsonBadPathError(ctx, zPath, i);
      }
      goto json_type_done;
    }
  }else{
    i = 0;
  }
  sqlite3_result_text(ctx, jsonbType[p->aBlob[i]&0x0f], -1, SQLITE_STATIC);
json_type_done:
  jsonParseFree(p);
}

/*
** json_pretty(JSON)
** json_pretty(JSON, INDENT)
**
** Return text that is a pretty-printed rendering of the input JSON.
** If the argument is not valid JSON, return NULL.
**
** The INDENT argument is text that is used for indentation.  If omitted,
** it defaults to four spaces (the same as PostgreSQL).
*/
static void jsonPrettyFunc(
  sqlite3_context *ctx,
  int argc,
  sqlite3_value **argv
){
  JsonString s;          /* The output string */
  JsonPretty x;          /* Pretty printing context */

  memset(&x, 0, sizeof(x));
  x.pParse = jsonParseFuncArg(ctx, argv[0], 0);
  if( x.pParse==0 ) return;
  x.pOut = &s;
  jsonStringInit(&s, ctx);
  if( argc==1 || (x.zIndent = (const char*)sqlite3_value_text(argv[1]))==0 ){
    x.zIndent = "    ";
    x.szIndent = 4;
  }else{
    x.szIndent = (u32)strlen(x.zIndent);
  }
  jsonTranslateBlobToPrettyText(&x, 0);
  jsonReturnString(&s, 0, 0);
  jsonParseFree(x.pParse);
}

/*
** json_valid(JSON)
** json_valid(JSON, FLAGS)
**
** Check the JSON argument to see if it is well-formed.  The FLAGS argument
** encodes the various constraints on what is meant by "well-formed":
**
**     0x01      Canonical RFC-8259 JSON text
**     0x02      JSON text with optional JSON-5 extensions
**     0x04      Superficially appears to be JSONB
**     0x08      Strictly well-formed JSONB
**
** If the FLAGS argument is omitted, it defaults to 1.  Useful values for
** FLAGS include:
**
**    1          Strict canonical JSON text
**    2          JSON text perhaps with JSON-5 extensions
**    4          Superficially appears to be JSONB
**    5          Canonical JSON text or superficial JSONB
**    6          JSON-5 text or superficial JSONB
**    8          Strict JSONB
**    9          Canonical JSON text or strict JSONB
**    10         JSON-5 text or strict JSONB
**
** Other flag combinations are redundant.  For example, every canonical
** JSON text is also well-formed JSON-5 text, so FLAG values 2 and 3
** are the same.  Similarly, any input that passes a strict JSONB validation
** will also pass the superficial validation so 12 through 15 are the same
** as 8 through 11 respectively.
**
** This routine runs in linear time to validate text and when doing strict
** JSONB validation.  Superficial JSONB validation is constant time,
** assuming the BLOB is already in memory.  The performance advantage
** of superficial JSONB validation is why that option is provided.
** Application developers can choose to do fast superficial validation or
** slower strict validation, according to their specific needs.
**
** Only the lower four bits of the FLAGS argument are currently used.
** Higher bits are reserved for future expansion.   To facilitate
** compatibility, the current implementation raises an error if any bit
** in FLAGS is set other than the lower four bits.
**
** The original circa 2015 implementation of the JSON routines in
** SQLite only supported canonical RFC-8259 JSON text and the json_valid()
** function only accepted one argument.  That is why the default value
** for the FLAGS argument is 1, since FLAGS=1 causes this routine to only
** recognize canonical RFC-8259 JSON text as valid.  The extra FLAGS
** argument was added when the JSON routines were extended to support
** JSON5-like extensions and binary JSONB stored in BLOBs.
**
** Return Values:
**
**   *   Raise an error if FLAGS is outside the range of 1 to 15.
**   *   Return NULL if the input is NULL
**   *   Return 1 if the input is well-formed.
**   *   Return 0 if the input is not well-formed.
*/
static void jsonValidFunc(
  sqlite3_context *ctx,
  int argc,
  sqlite3_value **argv
){
  JsonParse *p;          /* The parse */
  u8 flags = 1;
  u8 res = 0;
  if( argc==2 ){
    i64 f = sqlite3_value_int64(argv[1]);
    if( f<1 || f>15 ){
      sqlite3_result_error(ctx, "FLAGS parameter to json_valid() must be"
                                " between 1 and 15", -1);
      return;
    }
    flags = f & 0x0f;
  }
  switch( sqlite3_value_type(argv[0]) ){
    case SQLITE_NULL: {
#ifdef SQLITE_LEGACY_JSON_VALID
      /* Incorrect legacy behavior was to return FALSE for a NULL input */
      sqlite3_result_int(ctx, 0);
#endif
      return;
    }
    case SQLITE_BLOB: {
      JsonParse py;
      memset(&py, 0, sizeof(py));
      if( jsonArgIsJsonb(argv[0], &py) ){
        if( flags & 0x04 ){
          /* Superficial checking only - accomplished by the
          ** jsonArgIsJsonb() call above. */
          res = 1;
        }else if( flags & 0x08 ){
          /* Strict checking.  Check by translating BLOB->TEXT->BLOB.  If
          ** no errors occur, call that a "strict check". */
          res = 0==jsonbValidityCheck(&py, 0, py.nBlob, 1);
        }
        break;
      }
      /* Fall through into interpreting the input as text.  See note
      ** above at tag-20240123-a. */
      /* no break */ deliberate_fall_through
    }
    default: {
      JsonParse px;
      if( (flags & 0x3)==0 ) break;
      memset(&px, 0, sizeof(px));

      p = jsonParseFuncArg(ctx, argv[0], JSON_KEEPERROR);
      if( p ){
        if( p->oom ){
          sqlite3_result_error_nomem(ctx);
        }else if( p->nErr ){
          /* no-op */
        }else if( (flags & 0x02)!=0 || p->hasNonstd==0 ){
          res = 1;
        }
        jsonParseFree(p);
      }else{
        sqlite3_result_error_nomem(ctx);
      }
      break;
    }
  }
  sqlite3_result_int(ctx, res);
}

/*
** json_error_position(JSON)
**
** If the argument is NULL, return NULL
**
** If the argument is BLOB, do a full validity check and return non-zero
** if the check fails.  The return value is the approximate 1-based offset
** to the byte of the element that contains the first error.
**
** Otherwise interpret the argument is TEXT (even if it is numeric) and
** return the 1-based character position for where the parser first recognized
** that the input was not valid JSON, or return 0 if the input text looks
** ok.  JSON-5 extensions are accepted.
*/
static void jsonErrorFunc(
  sqlite3_context *ctx,
  int argc,
  sqlite3_value **argv
){
  i64 iErrPos = 0;       /* Error position to be returned */
  JsonParse s;

  assert( argc==1 );
  UNUSED_PARAMETER(argc);
  memset(&s, 0, sizeof(s));
  s.db = sqlite3_context_db_handle(ctx);
  if( jsonArgIsJsonb(argv[0], &s) ){
    iErrPos = (i64)jsonbValidityCheck(&s, 0, s.nBlob, 1);
  }else{
    s.zJson = (char*)sqlite3_value_text(argv[0]);
    if( s.zJson==0 ) return;  /* NULL input or OOM */
    s.nJson = sqlite3_value_bytes(argv[0]);
    if( jsonConvertTextToBlob(&s,0) ){
      if( s.oom ){
        iErrPos = -1;
      }else{
        /* Convert byte-offset s.iErr into a character offset */
        u32 k;
        assert( s.zJson!=0 );  /* Because s.oom is false */
        for(k=0; k<s.iErr && ALWAYS(s.zJson[k]); k++){
          if( (s.zJson[k] & 0xc0)!=0x80 ) iErrPos++;
        }
        iErrPos++;
      }
    }
  }
  jsonParseReset(&s);
  if( iErrPos<0 ){
    sqlite3_result_error_nomem(ctx);
  }else{
    sqlite3_result_int64(ctx, iErrPos);
  }
}

/****************************************************************************
** Aggregate SQL function implementations
****************************************************************************/
/*
** json_group_array(VALUE)
**
** Return a JSON array composed of all values in the aggregate.
*/
static void jsonArrayStep(
  sqlite3_context *ctx,
  int argc,
  sqlite3_value **argv
){
  JsonString *pStr;
  UNUSED_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, sizeof(*pStr));
  if( pStr ){
    if( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '[');
    }else if( pStr->nUsed>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    jsonAppendSqlValue(pStr, argv[0]);
  }
}
static void jsonArrayCompute(sqlite3_context *ctx, int isFinal){
  JsonString *pStr;
  int flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  if( pStr ){
    pStr->pCtx = ctx;
    jsonAppendRawNZ(pStr, "]", 2);
    jsonStringTrimOneChar(pStr);
    if( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      return;
    }else if( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      if( isFinal ){
        if( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }else{
        jsonStringTrimOneChar(pStr);
      }
      return;
    }else if( isFinal ){
      sqlite3_result_text(ctx, pStr->zBuf, (int)pStr->nUsed,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                              sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }else{
      sqlite3_result_text(ctx, pStr->zBuf, (int)pStr->nUsed, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pStr);
    }
  }else if( flags & JSON_BLOB ){
    static const u8 emptyArray = 0x0b;
    sqlite3_result_blob(ctx, &emptyArray, 1, SQLITE_STATIC);
  }else{
    sqlite3_result_text(ctx, "[]", 2, SQLITE_STATIC);
  }
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}
static void jsonArrayValue(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 0);
}
static void jsonArrayFinal(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 1);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** This method works for both json_group_array() and json_group_object().
** It works by removing the first element of the group by searching forward
** to the first comma (",") that is not within a string and deleting all
** text through that comma.
*/
static void jsonGroupInverse(
  sqlite3_context *ctx,
  int argc,
  sqlite3_value **argv
){
  unsigned int i;
  int inStr = 0;
  int nNest = 0;
  char *z;
  char c;
  JsonString *pStr;
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
#ifdef NEVER
  /* pStr is always non-NULL since jsonArrayStep() or jsonObjectStep() will
  ** always have been called to initialize it */
  if( NEVER(!pStr) ) return;
#endif
  z = pStr->zBuf;
  for(i=1; i<pStr->nUsed && ((c = z[i])!=',' || inStr || nNest); i++){
    if( c=='"' ){
      inStr = !inStr;
    }else if( c=='\\' ){
      i++;
    }else if( !inStr ){
      if( c=='{' || c=='[' ) nNest++;
      if( c=='}' || c==']' ) nNest--;
    }
  }
  if( i<pStr->nUsed ){
    pStr->nUsed -= i;
    memmove(&z[1], &z[i+1], (size_t)pStr->nUsed-1);
    z[pStr->nUsed] = 0;
  }else{
    pStr->nUsed = 1;
  }
}
#else
# define jsonGroupInverse 0
#endif


/*
** json_group_obj(NAME,VALUE)
**
** Return a JSON object composed of all names and values in the aggregate.
*/
static void jsonObjectStep(
  sqlite3_context *ctx,
  int argc,
  sqlite3_value **argv
){
  JsonString *pStr;
  const char *z;
  u32 n;
  UNUSED_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, sizeof(*pStr));
  if( pStr ){
    z = (const char*)sqlite3_value_text(argv[0]);
    n = sqlite3Strlen30(z);
    if( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '{');
    }else if( pStr->nUsed>1 && z!=0 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    if( z!=0 ){
      jsonAppendString(pStr, z, n);
      jsonAppendChar(pStr, ':');
      jsonAppendSqlValue(pStr, argv[1]);
    }
  }
}
static void jsonObjectCompute(sqlite3_context *ctx, int isFinal){
  JsonString *pStr;
  int flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  if( pStr ){
    jsonAppendRawNZ(pStr, "}", 2);
    jsonStringTrimOneChar(pStr);
    pStr->pCtx = ctx;
    if( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      return;
    }else if( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      if( isFinal ){
        if( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }else{
        jsonStringTrimOneChar(pStr);
      }
      return;
    }else if( isFinal ){
      sqlite3_result_text(ctx, pStr->zBuf, (int)pStr->nUsed,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                          sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }else{
      sqlite3_result_text(ctx, pStr->zBuf, (int)pStr->nUsed, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pStr);
    }
  }else if( flags & JSON_BLOB ){
    static const unsigned char emptyObject = 0x0c;
    sqlite3_result_blob(ctx, &emptyObject, 1, SQLITE_STATIC);
  }else{
    sqlite3_result_text(ctx, "{}", 2, SQLITE_STATIC);
  }
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}
static void jsonObjectValue(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 0);
}
static void jsonObjectFinal(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 1);
}



#ifndef SQLITE_OMIT_VIRTUALTABLE
/****************************************************************************
** The json_each virtual table
****************************************************************************/
typedef struct JsonParent JsonParent;
struct JsonParent {
  u32 iHead;                 /* Start of object or array */
  u32 iValue;                /* Start of the value */
  u32 iEnd;                  /* First byte past the end */
  u32 nPath;                 /* Length of path */
  i64 iKey;                  /* Key for JSONB_ARRAY */
};

typedef struct JsonEachCursor JsonEachCursor;
struct JsonEachCursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  u32 iRowid;                /* The rowid */
  u32 i;                     /* Index in sParse.aBlob[] of current row */
  u32 iEnd;                  /* EOF when i equals or exceeds this value */
  u32 nRoot;                 /* Size of the root path in bytes */
  u8 eType;                  /* Type of the container for element i */
  u8 bRecursive;             /* True for json_tree().  False for json_each() */
  u8 eMode;                  /* 1 for json_each().  2 for jsonb_each() */
  u32 nParent;               /* Current nesting depth */
  u32 nParentAlloc;          /* Space allocated for aParent[] */
  JsonParent *aParent;       /* Parent elements of i */
  sqlite3 *db;               /* Database connection */
  JsonString path;           /* Current path */
  JsonParse sParse;          /* Parse of the input JSON */
};
typedef struct JsonEachConnection JsonEachConnection;
struct JsonEachConnection {
  sqlite3_vtab base;         /* Base class - must be first */
  sqlite3 *db;               /* Database connection */
  u8 eMode;                  /* 1 for json_each().  2 for jsonb_each() */
  u8 bRecursive;             /* True for json_tree().  False for json_each() */
};


/* Constructor for the json_each virtual table */
static int jsonEachConnect(
  sqlite3 *db,
  void *pAux,
  int argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  JsonEachConnection *pNew;
  int rc;

/* Column numbers */
#define JEACH_KEY     0
#define JEACH_VALUE   1
#define JEACH_TYPE    2
#define JEACH_ATOM    3
#define JEACH_ID      4
#define JEACH_PARENT  5
#define JEACH_FULLKEY 6
#define JEACH_PATH    7
/* The xBestIndex method assumes that the JSON and ROOT columns are
** the last two columns in the table.  Should this ever changes, be
** sure to update the xBestIndex method. */
#define JEACH_JSON    8
#define JEACH_ROOT    9

  UNUSED_PARAMETER(pzErr);
  UNUSED_PARAMETER(argv);
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(pAux);
  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(key,value,type,atom,id,parent,fullkey,path,"
                    "json HIDDEN,root HIDDEN)");
  if( rc==SQLITE_OK ){
    pNew = (JsonEachConnection*)sqlite3DbMallocZero(db, sizeof(*pNew));
    *ppVtab = (sqlite3_vtab*)pNew;
    if( pNew==0 ) return SQLITE_NOMEM;
    sqlite3_vtab_config(db, SQLITE_VTAB_INNOCUOUS);
    pNew->db = db;
    pNew->eMode = argv[0][4]=='b' ? 2 : 1;
    pNew->bRecursive = argv[0][4+pNew->eMode]=='t';
  }
  return rc;
}

/* destructor for json_each virtual table */
static int jsonEachDisconnect(sqlite3_vtab *pVtab){
  JsonEachConnection *p = (JsonEachConnection*)pVtab;
  sqlite3DbFree(p->db, pVtab);
  return SQLITE_OK;
}

/* constructor for a JsonEachCursor object for json_each()/json_tree(). */
static int jsonEachOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  JsonEachConnection *pVtab = (JsonEachConnection*)p;
  JsonEachCursor *pCur;

  UNUSED_PARAMETER(p);
  pCur = sqlite3DbMallocZero(pVtab->db, sizeof(*pCur));
  if( pCur==0 ) return SQLITE_NOMEM;
  pCur->db = pVtab->db;
  pCur->eMode = pVtab->eMode;
  pCur->bRecursive = pVtab->bRecursive;
  jsonStringZero(&pCur->path);
  *ppCursor = &pCur->base;
  return SQLITE_OK;
}

/* Reset a JsonEachCursor back to its original state.  Free any memory
** held. */
static void jsonEachCursorReset(JsonEachCursor *p){
  jsonParseReset(&p->sParse);
  jsonStringReset(&p->path);
  sqlite3DbFree(p->db, p->aParent);
  p->iRowid = 0;
  p->i = 0;
  p->aParent = 0;
  p->nParent = 0;
  p->nParentAlloc = 0;
  p->iEnd = 0;
  p->eType = 0;
}

/* Destructor for a jsonEachCursor object */
static int jsonEachClose(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  jsonEachCursorReset(p);

  sqlite3DbFree(p->db, cur);
  return SQLITE_OK;
}

/* Return TRUE if the jsonEachCursor object has been advanced off the end
** of the JSON object */
static int jsonEachEof(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  return p->i >= p->iEnd;
}

/*
** If the cursor is currently pointing at the label of a object entry,
** then return the index of the value.  For all other cases, return the
** current pointer position, which is the value.
*/
static int jsonSkipLabel(JsonEachCursor *p){
  if( p->eType==JSONB_OBJECT ){
    u32 sz = 0;
    u32 n = jsonbPayloadSize(&p->sParse, p->i, &sz);
    return p->i + n + sz;
  }else{
    return p->i;
  }
}

/*
** Append the path name for the current element.
*/
static void jsonAppendPathName(JsonEachCursor *p){
  assert( p->nParent>0 );
  assert( p->eType==JSONB_ARRAY || p->eType==JSONB_OBJECT );
  if( p->eType==JSONB_ARRAY ){
    jsonPrintf(30, &p->path, "[%lld]", p->aParent[p->nParent-1].iKey);
  }else{
    u32 n, sz = 0, k, i;
    const char *z;
    int needQuote = 0;
    n = jsonbPayloadSize(&p->sParse, p->i, &sz);
    k = p->i + n;
    z = (const char*)&p->sParse.aBlob[k];
    if( sz==0 || !sqlite3Isalpha(z[0]) ){
      needQuote = 1;
    }else{
      for(i=0; i<sz; i++){
        if( !sqlite3Isalnum(z[i]) ){
          needQuote = 1;
          break;
        }
      }
    }
    if( needQuote ){
      jsonPrintf(sz+4,&p->path,".\"%.*s\"", sz, z);
    }else{
      jsonPrintf(sz+2,&p->path,".%.*s", sz, z);
    }
  }
}

/* Advance the cursor to the next element for json_tree() */
static int jsonEachNext(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  int rc = SQLITE_OK;
  if( p->bRecursive ){
    u8 x;
    u8 levelChange = 0;
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    x = p->sParse.aBlob[i] & 0x0f;
    n = jsonbPayloadSize(&p->sParse, i, &sz);
    if( x==JSONB_OBJECT || x==JSONB_ARRAY ){
      JsonParent *pParent;
      if( p->nParent>=p->nParentAlloc ){
        JsonParent *pNew;
        u64 nNew;
        nNew = p->nParentAlloc*2 + 3;
        pNew = sqlite3DbRealloc(p->db, p->aParent, sizeof(JsonParent)*nNew);
        if( pNew==0 ) return SQLITE_NOMEM;
        p->nParentAlloc = (u32)nNew;
        p->aParent = pNew;
      }
      levelChange = 1;
      pParent = &p->aParent[p->nParent];
      pParent->iHead = p->i;
      pParent->iValue = i;
      pParent->iEnd = i + n + sz;
      pParent->iKey = -1;
      pParent->nPath = (u32)p->path.nUsed;
      if( p->eType && p->nParent ){
        jsonAppendPathName(p);
        if( p->path.eErr ) rc = SQLITE_NOMEM;
      }
      p->nParent++;
      p->i = i + n;
    }else{
      p->i = i + n + sz;
    }
    while( p->nParent>0 && p->i >= p->aParent[p->nParent-1].iEnd ){
      p->nParent--;
      p->path.nUsed = p->aParent[p->nParent].nPath;
      levelChange = 1;
    }
    if( levelChange ){
      if( p->nParent>0 ){
        JsonParent *pParent = &p->aParent[p->nParent-1];
        u32 iVal = pParent->iValue;
        p->eType = p->sParse.aBlob[iVal] & 0x0f;
      }else{
        p->eType = 0;
      }
    }
  }else{
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    n = jsonbPayloadSize(&p->sParse, i, &sz);
    p->i = i + n + sz;
  }
  if( p->eType==JSONB_ARRAY && p->nParent ){
    p->aParent[p->nParent-1].iKey++;
  }
  p->iRowid++;
  return rc;
}

/* Length of the path for rowid==0 in bRecursive mode.
*/
static int jsonEachPathLength(JsonEachCursor *p){
  u32 n = p->path.nUsed;
  char *z = p->path.zBuf;
  if( p->iRowid==0 && p->bRecursive && n>=2 ){
    while( n>1 ){
      n--;
      if( z[n]=='[' || z[n]=='.' ){
        u32 x, sz = 0;
        char cSaved = z[n];
        z[n] = 0;
        assert( p->sParse.eEdit==0 );
        x = jsonLookupStep(&p->sParse, 0, z+1, 0);
        z[n] = cSaved;
        if( JSON_LOOKUP_ISERROR(x) ) continue;
        if( x + jsonbPayloadSize(&p->sParse, x, &sz) == p->i ) break;
      }
    }
  }
  return n;
}

/* Return the value of a column */
static int jsonEachColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  int iColumn                 /* Which column to return */
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  switch( iColumn ){
    case JEACH_KEY: {
      if( p->nParent==0 ){
        u32 n, j;
        if( p->nRoot==1 ) break;
        j = jsonEachPathLength(p);
        n = p->nRoot - j;
        if( n==0 ){
          break;
        }else if( p->path.zBuf[j]=='[' ){
          i64 x;
          sqlite3Atoi64(&p->path.zBuf[j+1], &x, n-1, SQLITE_UTF8);
          sqlite3_result_int64(ctx, x);
        }else if( p->path.zBuf[j+1]=='"' ){
          sqlite3_result_text(ctx, &p->path.zBuf[j+2], n-3, SQLITE_TRANSIENT);
        }else{
          sqlite3_result_text(ctx, &p->path.zBuf[j+1], n-1, SQLITE_TRANSIENT);
        }
        break;
      }
      if( p->eType==JSONB_OBJECT ){
        jsonReturnFromBlob(&p->sParse, p->i, ctx, 1);
      }else{
        assert( p->eType==JSONB_ARRAY );
        sqlite3_result_int64(ctx, p->aParent[p->nParent-1].iKey);
      }
      break;
    }
    case JEACH_VALUE: {
      u32 i = jsonSkipLabel(p);
      jsonReturnFromBlob(&p->sParse, i, ctx, p->eMode);
      if( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY ){
        sqlite3_result_subtype(ctx, JSON_SUBTYPE);
      }
      break;
    }
    case JEACH_TYPE: {
      u32 i = jsonSkipLabel(p);
      u8 eType = p->sParse.aBlob[i] & 0x0f;
      sqlite3_result_text(ctx, jsonbType[eType], -1, SQLITE_STATIC);
      break;
    }
    case JEACH_ATOM: {
      u32 i = jsonSkipLabel(p);
      if( (p->sParse.aBlob[i] & 0x0f)<JSONB_ARRAY ){
        jsonReturnFromBlob(&p->sParse, i, ctx, 1);
      }
      break;
    }
    case JEACH_ID: {
      sqlite3_result_int64(ctx, (sqlite3_int64)p->i);
      break;
    }
    case JEACH_PARENT: {
      if( p->nParent>0 && p->bRecursive ){
        sqlite3_result_int64(ctx, p->aParent[p->nParent-1].iHead);
      }
      break;
    }
    case JEACH_FULLKEY: {
      u64 nBase = p->path.nUsed;
      if( p->nParent ) jsonAppendPathName(p);
      sqlite3_result_text64(ctx, p->path.zBuf, p->path.nUsed,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      p->path.nUsed = nBase;
      break;
    }
    case JEACH_PATH: {
      u32 n = jsonEachPathLength(p);
      sqlite3_result_text64(ctx, p->path.zBuf, n,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      break;
    }
    default: {
      sqlite3_result_text(ctx, p->path.zBuf, p->nRoot, SQLITE_STATIC);
      break;
    }
    case JEACH_JSON: {
      if( p->sParse.zJson==0 ){
        sqlite3_result_blob(ctx, p->sParse.aBlob, p->sParse.nBlob,
                            SQLITE_TRANSIENT);
      }else{
        sqlite3_result_text(ctx, p->sParse.zJson, -1, SQLITE_TRANSIENT);
      }
      break;
    }
  }
  return SQLITE_OK;
}

/* Return the current rowid value */
static int jsonEachRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  *pRowid = p->iRowid;
  return SQLITE_OK;
}

/* The query strategy is to look for an equality constraint on the json
** column.  Without such a constraint, the table cannot operate.  idxNum is
** 1 if the constraint is found, 3 if the constraint and zRoot are found,
** and 0 otherwise.
*/
static int jsonEachBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  int i;                     /* Loop counter or computed array index */
  int aIdx[2];               /* Index of constraints for JSON and ROOT */
  int unusableMask = 0;      /* Mask of unusable JSON and ROOT constraints */
  int idxMask = 0;           /* Mask of usable == constraints JSON and ROOT */
  const struct sqlite3_index_constraint *pConstraint;

  /* This implementation assumes that JSON and ROOT are the last two
  ** columns in the table */
  assert( JEACH_ROOT == JEACH_JSON+1 );
  UNUSED_PARAMETER(tab);
  aIdx[0] = aIdx[1] = -1;
  pConstraint = pIdxInfo->aConstraint;
  for(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    int iCol;
    int iMask;
    if( pConstraint->iColumn < JEACH_JSON ) continue;
    iCol = pConstraint->iColumn - JEACH_JSON;
    assert( iCol==0 || iCol==1 );
    testcase( iCol==0 );
    iMask = 1 << iCol;
    if( pConstraint->usable==0 ){
      unusableMask |= iMask;
    }else if( pConstraint->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      aIdx[iCol] = i;
      idxMask |= iMask;
    }
  }
  if( pIdxInfo->nOrderBy>0
   && pIdxInfo->aOrderBy[0].iColumn<0
   && pIdxInfo->aOrderBy[0].desc==0
  ){
    pIdxInfo->orderByConsumed = 1;
  }

  if( (unusableMask & ~idxMask)!=0 ){
    /* If there are any unusable constraints on JSON or ROOT, then reject
    ** this entire plan */
    return SQLITE_CONSTRAINT;
  }
  if( aIdx[0]<0 ){
    /* No JSON input.  Leave estimatedCost at the huge value that it was
    ** initialized to to discourage the query planner from selecting this
    ** plan. */
    pIdxInfo->idxNum = 0;
  }else{
    pIdxInfo->estimatedCost = 1.0;
    i = aIdx[0];
    pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    if( aIdx[1]<0 ){
      pIdxInfo->idxNum = 1;  /* Only JSON supplied.  Plan 1 */
    }else{
      i = aIdx[1];
      pIdxInfo->aConstraintUsage[i].argvIndex = 2;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idxNum = 3;  /* Both JSON and ROOT are supplied.  Plan 3 */
    }
  }
  return SQLITE_OK;
}

/* Start a search on a new JSON string */
static int jsonEachFilter(
  sqlite3_vtab_cursor *cur,
  int idxNum, const char *idxStr,
  int argc, sqlite3_value **argv
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  const char *zRoot = 0;
  u32 i, n, sz;

  UNUSED_PARAMETER(idxStr);
  UNUSED_PARAMETER(argc);
  jsonEachCursorReset(p);
  if( idxNum==0 ) return SQLITE_OK;
  memset(&p->sParse, 0, sizeof(p->sParse));
  p->sParse.nJPRef = 1;
  p->sParse.db = p->db;
  if( jsonArgIsJsonb(argv[0], &p->sParse) ){
    /* We have JSONB */
  }else{
    p->sParse.zJson = (char*)sqlite3_value_text(argv[0]);
    p->sParse.nJson = sqlite3_value_bytes(argv[0]);
    if( p->sParse.zJson==0 ){
      p->i = p->iEnd = 0;
      return SQLITE_OK;
    }
    if( jsonConvertTextToBlob(&p->sParse, 0) ){
      if( p->sParse.oom ){
        return SQLITE_NOMEM;
      }
      goto json_each_malformed_input;
    }
  }
  if( idxNum==3 ){
    zRoot = (const char*)sqlite3_value_text(argv[1]);
    if( zRoot==0 ) return SQLITE_OK;
    if( zRoot[0]!='$' ){
      sqlite3_free(cur->pVtab->zErrMsg);
      cur->pVtab->zErrMsg = jsonBadPathError(0, zRoot, 0);
      jsonEachCursorReset(p);
      return cur->pVtab->zErrMsg ? SQLITE_ERROR : SQLITE_NOMEM;
    }
    p->nRoot = sqlite3Strlen30(zRoot);
    if( zRoot[1]==0 ){
      i = p->i = 0;
      p->eType = 0;
    }else{
      i = jsonLookupStep(&p->sParse, 0, zRoot+1, 0);
      if( JSON_LOOKUP_ISERROR(i) ){
        if( i==JSON_LOOKUP_NOTFOUND ){
          p->i = 0;
          p->eType = 0;
          p->iEnd = 0;
          return SQLITE_OK;
        }
        sqlite3_free(cur->pVtab->zErrMsg);
        cur->pVtab->zErrMsg = jsonBadPathError(0, zRoot, 0);
        jsonEachCursorReset(p);
        return cur->pVtab->zErrMsg ? SQLITE_ERROR : SQLITE_NOMEM;
      }
      if( p->sParse.iLabel ){
        p->i = p->sParse.iLabel;
        p->eType = JSONB_OBJECT;
      }else{
        p->i = i;
        p->eType = JSONB_ARRAY;
      }
    }
    jsonAppendRaw(&p->path, zRoot, p->nRoot);
  }else{
    i = p->i = 0;
    p->eType = 0;
    p->nRoot = 1;
    jsonAppendRaw(&p->path, "$", 1);
  }
  p->nParent = 0;
  n = jsonbPayloadSize(&p->sParse, i, &sz);
  p->iEnd = i+n+sz;
  if( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY && !p->bRecursive ){
    p->i = i + n;
    p->eType = p->sParse.aBlob[i] & 0x0f;
    p->aParent = sqlite3DbMallocZero(p->db, sizeof(JsonParent));
    if( p->aParent==0 ) return SQLITE_NOMEM;
    p->nParent = 1;
    p->nParentAlloc = 1;
    p->aParent[0].iKey = 0;
    p->aParent[0].iEnd = p->iEnd;
    p->aParent[0].iHead = p->i;
    p->aParent[0].iValue = i;
  }
  return SQLITE_OK;

json_each_malformed_input:
  sqlite3_free(cur->pVtab->zErrMsg);
  cur->pVtab->zErrMsg = sqlite3_mprintf("malformed JSON");
  jsonEachCursorReset(p);
  return cur->pVtab->zErrMsg ? SQLITE_ERROR : SQLITE_NOMEM;
}

/* The methods of the json_each virtual table */
static sqlite3_module jsonEachModule = {
  0,                         /* iVersion */
  0,                         /* xCreate */
  jsonEachConnect,           /* xConnect */
  jsonEachBestIndex,         /* xBestIndex */
  jsonEachDisconnect,        /* xDisconnect */
  0,                         /* xDestroy */
  jsonEachOpen,              /* xOpen - open a cursor */
  jsonEachClose,             /* xClose - close a cursor */
  jsonEachFilter,            /* xFilter - configure scan constraints */
  jsonEachNext,              /* xNext - advance a cursor */
  jsonEachEof,               /* xEof - check for end of scan */
  jsonEachColumn,            /* xColumn - read data */
  jsonEachRowid,             /* xRowid - read data */
  0,                         /* xUpdate */
  0,                         /* xBegin */
  0,                         /* xSync */
  0,                         /* xCommit */
  0,                         /* xRollback */
  0,                         /* xFindMethod */
  0,                         /* xRename */
  0,                         /* xSavepoint */
  0,                         /* xRelease */
  0,                         /* xRollbackTo */
  0,                         /* xShadowName */
  0                          /* xIntegrity */
};
#endif /* SQLITE_OMIT_VIRTUALTABLE */
#endif /* !defined(SQLITE_OMIT_JSON) */

/*
** Register JSON functions.
*/
SQLITE_PRIVATE void sqlite3RegisterJsonFunctions(void){
#ifndef SQLITE_OMIT_JSON
  static FuncDef aJsonFunc[] = {
    /*   sqlite3_result_subtype() ----,  ,--- sqlite3_value_subtype()       */
    /*                                |  |                                  */
    /*             Uses cache ------, |  | ,---- Returns JSONB              */
    /*                              | |  | |                                */
    /*     Number of arguments ---, | |  | | ,--- Flags                     */
    /*                            | | |  | | |                              */
    JFUNCTION(json,               1,1,1, 0,0,0,          jsonRemoveFunc),
    JFUNCTION(jsonb,              1,1,0, 0,1,0,          jsonRemoveFunc),
    JFUNCTION(json_array,        -1,0,1, 1,0,0,          jsonArrayFunc),
    JFUNCTION(jsonb_array,       -1,0,1, 1,1,0,          jsonArrayFunc),
    JFUNCTION(json_array_insert, -1,1,1, 1,0,JSON_AINS,  jsonSetFunc),
    JFUNCTION(jsonb_array_insert,-1,1,0, 1,1,JSON_AINS,  jsonSetFunc),
    JFUNCTION(json_array_length,  1,1,0, 0,0,0,          jsonArrayLengthFunc),
    JFUNCTION(json_array_length,  2,1,0, 0,0,0,          jsonArrayLengthFunc),
    JFUNCTION(json_error_position,1,1,0, 0,0,0,          jsonErrorFunc),
    JFUNCTION(json_extract,      -1,1,1, 0,0,0,          jsonExtractFunc),
    JFUNCTION(jsonb_extract,     -1,1,0, 0,1,0,          jsonExtractFunc),
    JFUNCTION(->,                 2,1,1, 0,0,JSON_JSON,  jsonExtractFunc),
    JFUNCTION(->>,                2,1,0, 0,0,JSON_SQL,   jsonExtractFunc),
    JFUNCTION(json_insert,       -1,1,1, 1,0,0,          jsonSetFunc),
    JFUNCTION(jsonb_insert,      -1,1,0, 1,1,0,          jsonSetFunc),
    JFUNCTION(json_object,       -1,0,1, 1,0,0,          jsonObjectFunc),
    JFUNCTION(jsonb_object,      -1,0,1, 1,1,0,          jsonObjectFunc),
    JFUNCTION(json_patch,         2,1,1, 0,0,0,          jsonPatchFunc),
    JFUNCTION(jsonb_patch,        2,1,0, 0,1,0,          jsonPatchFunc),
    JFUNCTION(json_pretty,        1,1,0, 0,0,0,          jsonPrettyFunc),
    JFUNCTION(json_pretty,        2,1,0, 0,0,0,          jsonPrettyFunc),
    JFUNCTION(json_quote,         1,0,1, 1,0,0,          jsonQuoteFunc),
    JFUNCTION(json_remove,       -1,1,1, 0,0,0,          jsonRemoveFunc),
    JFUNCTION(jsonb_remove,      -1,1,0, 0,1,0,          jsonRemoveFunc),
    JFUNCTION(json_replace,      -1,1,1, 1,0,0,          jsonReplaceFunc),
    JFUNCTION(jsonb_replace,     -1,1,0, 1,1,0,          jsonReplaceFunc),
    JFUNCTION(json_set,          -1,1,1, 1,0,JSON_ISSET, jsonSetFunc),
    JFUNCTION(jsonb_set,         -1,1,0, 1,1,JSON_ISSET, jsonSetFunc),
    JFUNCTION(json_type,          1,1,0, 0,0,0,          jsonTypeFunc),
    JFUNCTION(json_type,          2,1,0, 0,0,0,          jsonTypeFunc),
    JFUNCTION(json_valid,         1,1,0, 0,0,0,          jsonValidFunc),
    JFUNCTION(json_valid,         2,1,0, 0,0,0,          jsonValidFunc),
#if SQLITE_DEBUG
    JFUNCTION(json_parse,         1,1,0, 0,0,0,          jsonParseFunc),
#endif
    WAGGREGATE(json_group_array,  1, 0, 0,
       jsonArrayStep, jsonArrayFinal, jsonArrayValue, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_group_array, 1, JSON_BLOB, 0,
       jsonArrayStep, jsonArrayFinal, jsonArrayValue, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(json_group_object, 2, 0, 0,
       jsonObjectStep, jsonObjectFinal, jsonObjectValue, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_group_object,2, JSON_BLOB, 0,
       jsonObjectStep, jsonObjectFinal, jsonObjectValue, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC)
  };
  sqlite3InsertBuiltinFuncs(aJsonFunc, ArraySize(aJsonFunc));
#endif
}

#if  !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON)
/*
** Register the JSON table-valued function named zName and return a
** pointer to its Module object.  Return NULL if something goes wrong.
*/
SQLITE_PRIVATE Module *sqlite3JsonVtabRegister(sqlite3 *db, const char *zName){
  unsigned int i;
  static const char *azModule[] = {
    "json_each", "json_tree", "jsonb_each", "jsonb_tree"
  };
  assert( sqlite3HashFind(&db->aModule, zName)==0 );
  for(i=0; i<sizeof(azModule)/sizeof(azModule[0]); i++){
    if( sqlite3StrICmp(azModule[i],zName)==0 ){
      return sqlite3VtabCreateModule(db, azModule[i], &jsonEachModule, 0, 0);
    }
  }
  return 0;
}
#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON) */

/************** End of json.c ************************************************/
/************** Begin file rtree.c *******************************************/
/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains code for implementations of the r-tree and r*-tree
** algorithms packaged as an SQLite virtual table module.
*/

/*
** Database Format of R-Tree Tables
** --------------------------------
**
** The data structure for a single virtual r-tree table is stored in three
** native SQLite tables declared as follows. In each case, the '%' character
** in the table name is replaced with the user-supplied name of the r-tree
** table.
**
**   CREATE TABLE %_node(nodeno INTEGER PRIMARY KEY, data BLOB)
**   CREATE TABLE %_parent(nodeno INTEGER PRIMARY KEY, parentnode INTEGER)
**   CREATE TABLE %_rowid(rowid INTEGER PRIMARY KEY, nodeno INTEGER, ...)
**
** The data for each node of the r-tree structure is stored in the %_node
** table. For each node that is not the root node of the r-tree, there is
** an entry in the %_parent table associating the node with its parent.
** And for each row of data in the table, there is an entry in the %_rowid
** table that maps from the entries rowid to the id of the node that it
** is stored on.  If the r-tree contains auxiliary columns, those are stored
** on the end of the %_rowid table.
**
** The root node of an r-tree always exists, even if the r-tree table is
** empty. The nodeno of the root node is always 1. All other nodes in the
** table must be the same size as the root node. The content of each node
** is formatted as follows:
**
**   1. If the node is the root node (node 1), then the first 2 bytes
**      of the node contain the tree depth as a big-endian integer.
**      For non-root nodes, the first 2 bytes are left unused.
**
**   2. The next 2 bytes contain the number of entries currently
**      stored in the node.
**
**   3. The remainder of the node contains the node entries. Each entry
**      consists of a single 8-byte integer followed by an even number
**      of 4-byte coordinates. For leaf nodes the integer is the rowid
**      of a record. For internal nodes it is the node number of a
**      child page.
*/

#if !defined(SQLITE_CORE) \
  || (defined(SQLITE_ENABLE_RTREE) && !defined(SQLITE_OMIT_VIRTUALTABLE))

#ifndef SQLITE_CORE
/*   #include "sqlite3ext.h" */
  SQLITE_EXTENSION_INIT1
#else
/*   #include "sqlite3.h" */
#endif
SQLITE_PRIVATE sqlite3_int64 sqlite3GetToken(const unsigned char*,int*); /* In SQLite core */

/* #include <stddef.h> */

/*
** If building separately, we will need some setup that is normally
** found in sqliteInt.h
*/
#if !defined(SQLITE_AMALGAMATION)
#include "sqlite3rtree.h"
typedef sqlite3_int64 i64;
typedef sqlite3_uint64 u64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
#if !defined(NDEBUG) && !defined(SQLITE_DEBUG)
# define NDEBUG 1
#endif
#if defined(NDEBUG) && defined(SQLITE_DEBUG)
# undef NDEBUG
#endif
#if defined(SQLITE_COVERAGE_TEST) || defined(SQLITE_MUTATION_TEST)
# define SQLITE_OMIT_AUXILIARY_SAFETY_CHECKS 1
#endif
#if defined(SQLITE_OMIT_AUXILIARY_SAFETY_CHECKS)
# define ALWAYS(X)      (1)
# define NEVER(X)       (0)
#elif !defined(NDEBUG)
# define ALWAYS(X)      ((X)?1:(assert(0),0))
# define NEVER(X)       ((X)?(assert(0),1):0)
#else
# define ALWAYS(X)      (X)
# define NEVER(X)       (X)
#endif
#ifndef offsetof
# define offsetof(ST,M) ((size_t)((char*)&((ST*)0)->M - (char*)0))
#endif
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
# define FLEXARRAY
#else
# define FLEXARRAY 1
#endif
#endif /* !defined(SQLITE_AMALGAMATION) */

/* Macro to check for 4-byte alignment.  Only used inside of assert() */
#ifdef SQLITE_DEBUG
# define FOUR_BYTE_ALIGNED(X)  ((((char*)(X) - (char*)0) & 3)==0)
#endif

/* #include <string.h> */
/* #include <stdio.h> */
/* #include <assert.h> */
/* #include <stdlib.h> */

/*  The following macro is used to suppress compiler warnings.
*/
#ifndef UNUSED_PARAMETER
# define UNUSED_PARAMETER(x) (void)(x)
#endif

typedef struct Rtree Rtree;
typedef struct RtreeCursor RtreeCursor;
typedef struct RtreeNode RtreeNode;
typedef struct RtreeCell RtreeCell;
typedef struct RtreeConstraint RtreeConstraint;
typedef struct RtreeMatchArg RtreeMatchArg;
typedef struct RtreeGeomCallback RtreeGeomCallback;
typedef union RtreeCoord RtreeCoord;
typedef struct RtreeSearchPoint RtreeSearchPoint;

/* The rtree may have between 1 and RTREE_MAX_DIMENSIONS dimensions. */
#define RTREE_MAX_DIMENSIONS 5

/* Maximum number of auxiliary columns */
#define RTREE_MAX_AUX_COLUMN 100

/* Size of hash table Rtree.aHash. This hash table is not expected to
** ever contain very many entries, so a fixed number of buckets is
** used.
*/
#define HASHSIZE 97

/* The xBestIndex method of this virtual table requires an estimate of
** the number of rows in the virtual table to calculate the costs of
** various strategies. If possible, this estimate is loaded from the
** sqlite_stat1 table (with RTREE_MIN_ROWEST as a hard-coded minimum).
** Otherwise, if no sqlite_stat1 entry is available, use
** RTREE_DEFAULT_ROWEST.
*/
#define RTREE_DEFAULT_ROWEST 1048576
#define RTREE_MIN_ROWEST         100

/*
** An rtree virtual-table object.
*/
struct Rtree {
  sqlite3_vtab base;          /* Base class.  Must be first */
  sqlite3 *db;                /* Host database connection */
  int iNodeSize;              /* Size in bytes of each node in the node table */
  u8 nDim;                    /* Number of dimensions */
  u8 nDim2;                   /* Twice the number of dimensions */
  u8 eCoordType;              /* RTREE_COORD_REAL32 or RTREE_COORD_INT32 */
  u8 nBytesPerCell;           /* Bytes consumed per cell */
  u8 inWrTrans;               /* True if inside write transaction */
  u8 nAux;                    /* # of auxiliary columns in %_rowid */
#ifdef SQLITE_ENABLE_GEOPOLY
  u8 nAuxNotNull;             /* Number of initial not-null aux columns */
#endif
#ifdef SQLITE_DEBUG
  u8 bCorrupt;                /* Shadow table corruption detected */
#endif
  int iDepth;                 /* Current depth of the r-tree structure */
  char *zDb;                  /* Name of database containing r-tree table */
  char *zName;                /* Name of r-tree table */
  char *zNodeName;            /* Name of the %_node table */
  u32 nBusy;                  /* Current number of users of this structure */
  i64 nRowEst;                /* Estimated number of rows in this table */
  u32 nCursor;                /* Number of open cursors */
  u32 nNodeRef;               /* Number RtreeNodes with positive nRef */
  char *zReadAuxSql;          /* SQL for statement to read aux data */

  /* List of nodes removed during a CondenseTree operation. List is
  ** linked together via the pointer normally used for hash chains -
  ** RtreeNode.pNext. RtreeNode.iNode stores the depth of the sub-tree
  ** headed by the node (leaf nodes have RtreeNode.iNode==0).
  */
  RtreeNode *pDeleted;

  /* Blob I/O on xxx_node */
  sqlite3_blob *pNodeBlob;

  /* Statements to read/write/delete a record from xxx_node */
  sqlite3_stmt *pWriteNode;
  sqlite3_stmt *pDeleteNode;

  /* Statements to read/write/delete a record from xxx_rowid */
  sqlite3_stmt *pReadRowid;
  sqlite3_stmt *pWriteRowid;
  sqlite3_stmt *pDeleteRowid;

  /* Statements to read/write/delete a record from xxx_parent */
  sqlite3_stmt *pReadParent;
  sqlite3_stmt *pWriteParent;
  sqlite3_stmt *pDeleteParent;

  /* Statement for writing to the "aux:" fields, if there are any */
  sqlite3_stmt *pWriteAux;

  RtreeNode *aHash[HASHSIZE]; /* Hash table of in-memory nodes. */
};

/* Possible values for Rtree.eCoordType: */
#define RTREE_COORD_REAL32 0
#define RTREE_COORD_INT32  1

/*
** If SQLITE_RTREE_INT_ONLY is defined, then this virtual table will
** only deal with integer coordinates.  No floating point operations
** will be done.
*/
#ifdef SQLITE_RTREE_INT_ONLY
  typedef sqlite3_int64 RtreeDValue;       /* High accuracy coordinate */
  typedef int RtreeValue;                  /* Low accuracy coordinate */
# define RTREE_ZERO 0
#else
  typedef double RtreeDValue;              /* High accuracy coordinate */
  typedef float RtreeValue;                /* Low accuracy coordinate */
# define RTREE_ZERO 0.0
#endif

/*
** Set the Rtree.bCorrupt flag
*/
#ifdef SQLITE_DEBUG
# define RTREE_IS_CORRUPT(X) ((X)->bCorrupt = 1)
#else
# define RTREE_IS_CORRUPT(X)
#endif

/*
** When doing a search of an r-tree, instances of the following structure
** record intermediate results from the tree walk.
**
** The id is always a node-id.  For iLevel>=1 the id is the node-id of
** the node that the RtreeSearchPoint represents.  When iLevel==0, however,
** the id is of the parent node and the cell that RtreeSearchPoint
** represents is the iCell-th entry in the parent node.
*/
struct RtreeSearchPoint {
  RtreeDValue rScore;    /* The score for this node.  Smallest goes first. */
  sqlite3_int64 id;      /* Node ID */
  u8 iLevel;             /* 0=entries.  1=leaf node.  2+ for higher */
  u8 eWithin;            /* PARTLY_WITHIN or FULLY_WITHIN */
  u8 iCell;              /* Cell index within the node */
};

/*
** The minimum number of cells allowed for a node is a third of the
** maximum. In Gutman's notation:
**
**     m = M/3
**
** If an R*-tree "Reinsert" operation is required, the same number of
** cells are removed from the overfull node and reinserted into the tree.
*/
#define RTREE_MINCELLS(p) ((((p)->iNodeSize-4)/(p)->nBytesPerCell)/3)
#define RTREE_REINSERT(p) RTREE_MINCELLS(p)
#define RTREE_MAXCELLS 51

/*
** The smallest possible node-size is (512-64)==448 bytes. And the largest
** supported cell size is 48 bytes (8 byte rowid + ten 4 byte coordinates).
** Therefore all non-root nodes must contain at least 3 entries. Since
** 3^40 is greater than 2^64, an r-tree structure always has a depth of
** 40 or less.
*/
#define RTREE_MAX_DEPTH 40


/*
** Number of entries in the cursor RtreeNode cache.  The first entry is
** used to cache the RtreeNode for RtreeCursor.sPoint.  The remaining
** entries cache the RtreeNode for the first elements of the priority queue.
*/
#define RTREE_CACHE_SZ  5

/*
** An rtree cursor object.
*/
struct RtreeCursor {
  sqlite3_vtab_cursor base;         /* Base class.  Must be first */
  u8 atEOF;                         /* True if at end of search */
  u8 bPoint;                        /* True if sPoint is valid */
  u8 bAuxValid;                     /* True if pReadAux is valid */
  int iStrategy;                    /* Copy of idxNum search parameter */
  int nConstraint;                  /* Number of entries in aConstraint */
  RtreeConstraint *aConstraint;     /* Search constraints. */
  int nPointAlloc;                  /* Number of slots allocated for aPoint[] */
  int nPoint;                       /* Number of slots used in aPoint[] */
  int mxLevel;                      /* iLevel value for root of the tree */
  RtreeSearchPoint *aPoint;         /* Priority queue for search points */
  sqlite3_stmt *pReadAux;           /* Statement to read aux-data */
  RtreeSearchPoint sPoint;          /* Cached next search point */
  RtreeNode *aNode[RTREE_CACHE_SZ]; /* Rtree node cache */
  u32 anQueue[RTREE_MAX_DEPTH+1];   /* Number of queued entries by iLevel */
};

/* Return the Rtree of a RtreeCursor */
#define RTREE_OF_CURSOR(X)   ((Rtree*)((X)->base.pVtab))

/*
** A coordinate can be either a floating point number or a integer.  All
** coordinates within a single R-Tree are always of the same time.
*/
union RtreeCoord {
  RtreeValue f;      /* Floating point value */
  int i;             /* Integer value */
  u32 u;             /* Unsigned for byte-order conversions */
};

/*
** The argument is an RtreeCoord. Return the value stored within the RtreeCoord
** formatted as a RtreeDValue (double or int64). This macro assumes that local
** variable pRtree points to the Rtree structure associated with the
** RtreeCoord.
*/
#ifdef SQLITE_RTREE_INT_ONLY
# define DCOORD(coord) ((RtreeDValue)coord.i)
#else
# define DCOORD(coord) (                           \
    (pRtree->eCoordType==RTREE_COORD_REAL32) ?      \
      ((double)coord.f) :                           \
      ((double)coord.i)                             \
  )
#endif

/*
** A search constraint.
*/
struct RtreeConstraint {
  int iCoord;                     /* Index of constrained coordinate */
  int op;                         /* Constraining operation */
  union {
    RtreeDValue rValue;             /* Constraint value. */
    int (*xGeom)(sqlite3_rtree_geometry*,int,RtreeDValue*,int*);
    int (*xQueryFunc)(sqlite3_rtree_query_info*);
  } u;
  sqlite3_rtree_query_info *pInfo;  /* xGeom and xQueryFunc argument */
};

/* Possible values for RtreeConstraint.op */
#define RTREE_EQ    0x41  /* A */
#define RTREE_LE    0x42  /* B */
#define RTREE_LT    0x43  /* C */
#define RTREE_GE    0x44  /* D */
#define RTREE_GT    0x45  /* E */
#define RTREE_MATCH 0x46  /* F: Old-style sqlite3_rtree_geometry_callback() */
#define RTREE_QUERY 0x47  /* G: New-style sqlite3_rtree_query_callback() */

/* Special operators available only on cursors.  Needs to be consecutive
** with the normal values above, but must be less than RTREE_MATCH.  These
** are used in the cursor for contraints such as x=NULL (RTREE_FALSE) or
** x<'xyz' (RTREE_TRUE) */
#define RTREE_TRUE  0x3f  /* ? */
#define RTREE_FALSE 0x40  /* @ */

/*
** An rtree structure node.
*/
struct RtreeNode {
  RtreeNode *pParent;         /* Parent node */
  i64 iNode;                  /* The node number */
  int nRef;                   /* Number of references to this node */
  int isDirty;                /* True if the node needs to be written to disk */
  u8 *zData;                  /* Content of the node, as should be on disk */
  RtreeNode *pNext;           /* Next node in this hash collision chain */
};

/* Return the number of cells in a node  */
#define NCELL(pNode) readInt16(&(pNode)->zData[2])

/*
** A single cell from a node, deserialized
*/
struct RtreeCell {
  i64 iRowid;                                 /* Node or entry ID */
  RtreeCoord aCoord[RTREE_MAX_DIMENSIONS*2];  /* Bounding box coordinates */
};


/*
** This object becomes the sqlite3_user_data() for the SQL functions
** that are created by sqlite3_rtree_geometry_callback() and
** sqlite3_rtree_query_callback() and which appear on the right of MATCH
** operators in order to constrain a search.
**
** xGeom and xQueryFunc are the callback functions.  Exactly one of
** xGeom and xQueryFunc fields is non-NULL, depending on whether the
** SQL function was created using sqlite3_rtree_geometry_callback() or
** sqlite3_rtree_query_callback().
**
** This object is deleted automatically by the destructor mechanism in
** sqlite3_create_function_v2().
*/
struct RtreeGeomCallback {
  int (*xGeom)(sqlite3_rtree_geometry*, int, RtreeDValue*, int*);
  int (*xQueryFunc)(sqlite3_rtree_query_info*);
  void (*xDestructor)(void*);
  void *pContext;
};

/*
** An instance of this structure (in the form of a BLOB) is returned by
** the SQL functions that sqlite3_rtree_geometry_callback() and
** sqlite3_rtree_query_callback() create, and is read as the right-hand
** operand to the MATCH operator of an R-Tree.
*/
struct RtreeMatchArg {
  u32 iSize;                  /* Size of this object */
  RtreeGeomCallback cb;       /* Info about the callback functions */
  int nParam;                 /* Number of parameters to the SQL function */
  sqlite3_value **apSqlParam; /* Original SQL parameter values */
  RtreeDValue aParam[FLEXARRAY]; /* Values for parameters to the SQL function */
};

/* Size of an RtreeMatchArg object with N parameters */
#define SZ_RTREEMATCHARG(N)  \
        (offsetof(RtreeMatchArg,aParam)+(N)*sizeof(RtreeDValue))

#ifndef MAX
# define MAX(x,y) ((x) < (y) ? (y) : (x))
#endif
#ifndef MIN
# define MIN(x,y) ((x) > (y) ? (y) : (x))
#endif

/* What version of GCC is being used.  0 means GCC is not being used .
** Note that the GCC_VERSION macro will also be set correctly when using
** clang, since clang works hard to be gcc compatible.  So the gcc
** optimizations will also work when compiling with clang.
*/
#ifndef GCC_VERSION
#if defined(__GNUC__) && !defined(SQLITE_DISABLE_INTRINSIC)
# define GCC_VERSION (__GNUC__*1000000+__GNUC_MINOR__*1000+__GNUC_PATCHLEVEL__)
#else
# define GCC_VERSION 0
#endif
#endif

/* The testcase() macro should already be defined in the amalgamation.  If
** it is not, make it a no-op.
*/
#ifndef SQLITE_AMALGAMATION
# if defined(SQLITE_COVERAGE_TEST) || defined(SQLITE_DEBUG)
    unsigned int sqlite3RtreeTestcase = 0;
#   define testcase(X)  if( X ){ sqlite3RtreeTestcase += __LINE__; }
# else
#   define testcase(X)
# endif
#endif

/*
** Make sure that the compiler intrinsics we desire are enabled when
** compiling with an appropriate version of MSVC unless prevented by
** the SQLITE_DISABLE_INTRINSIC define.
*/
#if !defined(SQLITE_DISABLE_INTRINSIC)
#  if defined(_MSC_VER) && _MSC_VER>=1400
#    if !defined(_WIN32_WCE)
/* #      include <intrin.h> */
#      pragma intrinsic(_byteswap_ulong)
#      pragma intrinsic(_byteswap_uint64)
#    else
/* #      include <cmnintrin.h> */
#    endif
#  endif
#endif

/*
** Macros to determine whether the machine is big or little endian,
** and whether or not that determination is run-time or compile-time.
**
** For best performance, an attempt is made to guess at the byte-order
** using C-preprocessor macros.  If that is unsuccessful, or if
** -DSQLITE_RUNTIME_BYTEORDER=1 is set, then byte-order is determined
** at run-time.
*/
#ifndef SQLITE_BYTEORDER /* Replicate changes at tag-20230904a */
# if defined(__BYTE_ORDER__) && __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
#   define SQLITE_BYTEORDER 4321
# elif defined(__BYTE_ORDER__) && __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
#   define SQLITE_BYTEORDER 1234
# elif defined(__BIG_ENDIAN__) && __BIG_ENDIAN__==1
#   define SQLITE_BYTEORDER 4321
# elif defined(i386)    || defined(__i386__)      || defined(_M_IX86) ||    \
     defined(__x86_64)  || defined(__x86_64__)    || defined(_M_X64)  ||    \
     defined(_M_AMD64)  || defined(_M_ARM)        || defined(__x86)   ||    \
     defined(__ARMEL__) || defined(__AARCH64EL__) || defined(_M_ARM64)
#   define SQLITE_BYTEORDER 1234
# elif defined(sparc)   || defined(__ARMEB__)     || defined(__AARCH64EB__)
#   define SQLITE_BYTEORDER 4321
# else
#   define SQLITE_BYTEORDER 0
# endif
#endif


/* What version of MSVC is being used.  0 means MSVC is not being used */
#ifndef MSVC_VERSION
#if defined(_MSC_VER) && !defined(SQLITE_DISABLE_INTRINSIC)
# define MSVC_VERSION _MSC_VER
#else
# define MSVC_VERSION 0
#endif
#endif

/*
** Functions to deserialize a 16 bit integer, 32 bit real number and
** 64 bit integer. The deserialized value is returned.
*/
static int readInt16(u8 *p){
  return (p[0]<<8) + p[1];
}
static void readCoord(u8 *p, RtreeCoord *pCoord){
  assert( FOUR_BYTE_ALIGNED(p) );
#if SQLITE_BYTEORDER==1234 && MSVC_VERSION>=1300
  pCoord->u = _byteswap_ulong(*(u32*)p);
#elif SQLITE_BYTEORDER==1234 && GCC_VERSION>=4003000
  pCoord->u = __builtin_bswap32(*(u32*)p);
#elif SQLITE_BYTEORDER==4321
  pCoord->u = *(u32*)p;
#else
  pCoord->u = (
    (((u32)p[0]) << 24) +
    (((u32)p[1]) << 16) +
    (((u32)p[2]) <<  8) +
    (((u32)p[3]) <<  0)
  );
#endif
}
static i64 readInt64(u8 *p){
#if SQLITE_BYTEORDER==1234 && MSVC_VERSION>=1300
  u64 x;
  memcpy(&x, p, 8);
  return (i64)_byteswap_uint64(x);
#elif SQLITE_BYTEORDER==1234 && GCC_VERSION>=4003000
  u64 x;
  memcpy(&x, p, 8);
  return (i64)__builtin_bswap64(x);
#elif SQLITE_BYTEORDER==4321
  i64 x;
  memcpy(&x, p, 8);
  return x;
#else
  return (i64)(
    (((u64)p[0]) << 56) +
    (((u64)p[1]) << 48) +
    (((u64)p[2]) << 40) +
    (((u64)p[3]) << 32) +
    (((u64)p[4]) << 24) +
    (((u64)p[5]) << 16) +
    (((u64)p[6]) <<  8) +
    (((u64)p[7]) <<  0)
  );
#endif
}

/*
** Functions to serialize a 16 bit integer, 32 bit real number and
** 64 bit integer. The value returned is the number of bytes written
** to the argument buffer (always 2, 4 and 8 respectively).
*/
static void writeInt16(u8 *p, int i){
  p[0] = (i>> 8)&0xFF;
  p[1] = (i>> 0)&0xFF;
}
static int writeCoord(u8 *p, RtreeCoord *pCoord){
  u32 i;
  assert( FOUR_BYTE_ALIGNED(p) );
  assert( sizeof(RtreeCoord)==4 );
  assert( sizeof(u32)==4 );
#if SQLITE_BYTEORDER==1234 && GCC_VERSION>=4003000
  i = __builtin_bswap32(pCoord->u);
  memcpy(p, &i, 4);
#elif SQLITE_BYTEORDER==1234 && MSVC_VERSION>=1300
  i = _byteswap_ulong(pCoord->u);
  memcpy(p, &i, 4);
#elif SQLITE_BYTEORDER==4321
  i = pCoord->u;
  memcpy(p, &i, 4);
#else
  i = pCoord->u;
  p[0] = (i>>24)&0xFF;
  p[1] = (i>>16)&0xFF;
  p[2] = (i>> 8)&0xFF;
  p[3] = (i>> 0)&0xFF;
#endif
  return 4;
}
static int writeInt64(u8 *p, i64 i){
#if SQLITE_BYTEORDER==1234 && GCC_VERSION>=4003000
  i = (i64)__builtin_bswap64((u64)i);
  memcpy(p, &i, 8);
#elif SQLITE_BYTEORDER==1234 && MSVC_VERSION>=1300
  i = (i64)_byteswap_uint64((u64)i);
  memcpy(p, &i, 8);
#elif SQLITE_BYTEORDER==4321
  memcpy(p, &i, 8);
#else
  p[0] = (i>>56)&0xFF;
  p[1] = (i>>48)&0xFF;
  p[2] = (i>>40)&0xFF;
  p[3] = (i>>32)&0xFF;
  p[4] = (i>>24)&0xFF;
  p[5] = (i>>16)&0xFF;
  p[6] = (i>> 8)&0xFF;
  p[7] = (i>> 0)&0xFF;
#endif
  return 8;
}

/*
** Increment the reference count of node p.
*/
static void nodeReference(RtreeNode *p){
  if( p ){
    assert( p->nRef>0 );
    p->nRef++;
  }
}

/*
** Clear the content of node p (set all bytes to 0x00).
*/
static void nodeZero(Rtree *pRtree, RtreeNode *p){
  memset(&p->zData[2], 0, pRtree->iNodeSize-2);
  p->isDirty = 1;
}

/*
** Given a node number iNode, return the corresponding key to use
** in the Rtree.aHash table.
*/
static unsigned int nodeHash(i64 iNode){
  return ((unsigned)iNode) % HASHSIZE;
}

/*
** Search the node hash table for node iNode. If found, return a pointer
** to it. Otherwise, return 0.
*/
static RtreeNode *nodeHashLookup(Rtree *pRtree, i64 iNode){
  RtreeNode *p;
  for(p=pRtree->aHash[nodeHash(iNode)]; p && p->iNode!=iNode; p=p->pNext);
  return p;
}

/*
** Add node pNode to the node hash table.
*/
static void nodeHashInsert(Rtree *pRtree, RtreeNode *pNode){
  int iHash;
  assert( pNode->pNext==0 );
  iHash = nodeHash(pNode->iNode);
  pNode->pNext = pRtree->aHash[iHash];
  pRtree->aHash[iHash] = pNode;
}

/*
** Remove node pNode from the node hash table.
*/
static void nodeHashDelete(Rtree *pRtree, RtreeNode *pNode){
  RtreeNode **pp;
  if( pNode->iNode!=0 ){
    pp = &pRtree->aHash[nodeHash(pNode->iNode)];
    for( ; (*pp)!=pNode; pp = &(*pp)->pNext){ assert(*pp); }
    *pp = pNode->pNext;
    pNode->pNext = 0;
  }
}

/*
** Allocate and return new r-tree node. Initially, (RtreeNode.iNode==0),
** indicating that node has not yet been assigned a node number. It is
** assigned a node number when nodeWrite() is called to write the
** node contents out to the database.
*/
static RtreeNode *nodeNew(Rtree *pRtree, RtreeNode *pParent){
  RtreeNode *pNode;
  pNode = (RtreeNode *)sqlite3_malloc64(sizeof(RtreeNode) + pRtree->iNodeSize);
  if( pNode ){
    memset(pNode, 0, sizeof(RtreeNode) + pRtree->iNodeSize);
    pNode->zData = (u8 *)&pNode[1];
    pNode->nRef = 1;
    pRtree->nNodeRef++;
    pNode->pParent = pParent;
    pNode->isDirty = 1;
    nodeReference(pParent);
  }
  return pNode;
}

/*
** Clear the Rtree.pNodeBlob object
*/
static void nodeBlobReset(Rtree *pRtree){
  sqlite3_blob *pBlob = pRtree->pNodeBlob;
  pRtree->pNodeBlob = 0;
  sqlite3_blob_close(pBlob);
}

/*
** Obtain a reference to an r-tree node.
*/
static int nodeAcquire(
  Rtree *pRtree,             /* R-tree structure */
  i64 iNode,                 /* Node number to load */
  RtreeNode *pParent,        /* Either the parent node or NULL */
  RtreeNode **ppNode         /* OUT: Acquired node */
){
  int rc = SQLITE_OK;
  RtreeNode *pNode = 0;

  /* Check if the requested node is already in the hash table. If so,
  ** increase its reference count and return it.
  */
  if( (pNode = nodeHashLookup(pRtree, iNode))!=0 ){
    if( pParent && ALWAYS(pParent!=pNode->pParent) ){
      RTREE_IS_CORRUPT(pRtree);
      return SQLITE_CORRUPT_VTAB;
    }
    pNode->nRef++;
    *ppNode = pNode;
    return SQLITE_OK;
  }

  if( pRtree->pNodeBlob ){
    sqlite3_blob *pBlob = pRtree->pNodeBlob;
    pRtree->pNodeBlob = 0;
    rc = sqlite3_blob_reopen(pBlob, iNode);
    pRtree->pNodeBlob = pBlob;
    if( rc ){
      nodeBlobReset(pRtree);
      if( rc==SQLITE_NOMEM ) return SQLITE_NOMEM;
    }
  }
  if( pRtree->pNodeBlob==0 ){
    rc = sqlite3_blob_open(pRtree->db, pRtree->zDb, pRtree->zNodeName,
                           "data", iNode, 0,
                           &pRtree->pNodeBlob);
  }
  if( rc ){
    *ppNode = 0;
    /* If unable to open an sqlite3_blob on the desired row, that can only
    ** be because the shadow tables hold erroneous data. */
    if( rc==SQLITE_ERROR ){
      rc = SQLITE_CORRUPT_VTAB;
      RTREE_IS_CORRUPT(pRtree);
    }
  }else if( pRtree->iNodeSize==sqlite3_blob_bytes(pRtree->pNodeBlob) ){
    pNode = (RtreeNode *)sqlite3_malloc64(sizeof(RtreeNode)+pRtree->iNodeSize);
    if( !pNode ){
      rc = SQLITE_NOMEM;
    }else{
      pNode->pParent = pParent;
      pNode->zData = (u8 *)&pNode[1];
      pNode->nRef = 1;
      pRtree->nNodeRef++;
      pNode->iNode = iNode;
      pNode->isDirty = 0;
      pNode->pNext = 0;
      rc = sqlite3_blob_read(pRtree->pNodeBlob, pNode->zData,
                             pRtree->iNodeSize, 0);
    }
  }

  /* If the root node was just loaded, set pRtree->iDepth to the height
  ** of the r-tree structure. A height of zero means all data is stored on
  ** the root node. A height of one means the children of the root node
  ** are the leaves, and so on. If the depth as specified on the root node
  ** is greater than RTREE_MAX_DEPTH, the r-tree structure must be corrupt.
  */
  if( rc==SQLITE_OK && pNode && iNode==1 ){
    pRtree->iDepth = readInt16(pNode->zData);
    if( pRtree->iDepth>RTREE_MAX_DEPTH ){
      rc = SQLITE_CORRUPT_VTAB;
      RTREE_IS_CORRUPT(pRtree);
    }
  }

  /* If no error has occurred so far, check if the "number of entries"
  ** field on the node is too large. If so, set the return code to
  ** SQLITE_CORRUPT_VTAB.
  */
  if( pNode && rc==SQLITE_OK ){
    if( NCELL(pNode)>((pRtree->iNodeSize-4)/pRtree->nBytesPerCell) ){
      rc = SQLITE_CORRUPT_VTAB;
      RTREE_IS_CORRUPT(pRtree);
    }
  }

  if( rc==SQLITE_OK ){
    if( pNode!=0 ){
      nodeReference(pParent);
      nodeHashInsert(pRtree, pNode);
    }else{
      rc = SQLITE_CORRUPT_VTAB;
      RTREE_IS_CORRUPT(pRtree);
    }
    *ppNode = pNode;
  }else{
    nodeBlobReset(pRtree);
    if( pNode ){
      pRtree->nNodeRef--;
      sqlite3_free(pNode);
    }
    *ppNode = 0;
  }

  return rc;
}

/*
** Overwrite cell iCell of node pNode with the contents of pCell.
*/
static void nodeOverwriteCell(
  Rtree *pRtree,             /* The overall R-Tree */
  RtreeNode *pNode,          /* The node into which the cell is to be written */
  RtreeCell *pCell,          /* The cell to write */
  int iCell                  /* Index into pNode into which pCell is written */
){
  int ii;
  u8 *p = &pNode->zData[4 + pRtree->nBytesPerCell*iCell];
  p += writeInt64(p, pCell->iRowid);
  for(ii=0; ii<pRtree->nDim2; ii++){
    p += writeCoord(p, &pCell->aCoord[ii]);
  }
  pNode->isDirty = 1;
}

/*
** Remove the cell with index iCell from node pNode.
*/
static void nodeDeleteCell(Rtree *pRtree, RtreeNode *pNode, int iCell){
  u8 *pDst = &pNode->zData[4 + pRtree->nBytesPerCell*iCell];
  u8 *pSrc = &pDst[pRtree->nBytesPerCell];
  int nByte = (NCELL(pNode) - iCell - 1) * pRtree->nBytesPerCell;
  memmove(pDst, pSrc, nByte);
  writeInt16(&pNode->zData[2], NCELL(pNode)-1);
  pNode->isDirty = 1;
}

/*
** Insert the contents of cell pCell into node pNode. If the insert
** is successful, return SQLITE_OK.
**
** If there is not enough free space in pNode, return SQLITE_FULL.
*/
static int nodeInsertCell(
  Rtree *pRtree,                /* The overall R-Tree */
  RtreeNode *pNode,             /* Write new cell into this node */
  RtreeCell *pCell              /* The cell to be inserted */
){
  int nCell;                    /* Current number of cells in pNode */
  int nMaxCell;                 /* Maximum number of cells for pNode */

  nMaxCell = (pRtree->iNodeSize-4)/pRtree->nBytesPerCell;
  nCell = NCELL(pNode);

  assert( nCell<=nMaxCell );
  if( nCell<nMaxCell ){
    nodeOverwriteCell(pRtree, pNode, pCell, nCell);
    writeInt16(&pNode->zData[2], nCell+1);
    pNode->isDirty = 1;
  }

  return (nCell==nMaxCell);
}

/*
** If the node is dirty, write it out to the database.
*/
static int nodeWrite(Rtree *pRtree, RtreeNode *pNode){
  int rc = SQLITE_OK;
  if( pNode->isDirty ){
    sqlite3_stmt *p = pRtree->pWriteNode;
    if( pNode->iNode ){
      sqlite3_bind_int64(p, 1, pNode->iNode);
    }else{
      sqlite3_bind_null(p, 1);
    }
    sqlite3_bind_blob(p, 2, pNode->zData, pRtree->iNodeSize, SQLITE_STATIC);
    sqlite3_step(p);
    pNode->isDirty = 0;
    rc = sqlite3_reset(p);
    sqlite3_bind_null(p, 2);
    if( pNode->iNode==0 && rc==SQLITE_OK ){
      pNode->iNode = sqlite3_last_insert_rowid(pRtree->db);
      nodeHashInsert(pRtree, pNode);
    }
  }
  return rc;
}

/*
** Release a reference to a node. If the node is dirty and the reference
** count drops to zero, the node data is written to the database.
*/
static int nodeRelease(Rtree *pRtree, RtreeNode *pNode){
  int rc = SQLITE_OK;
  if( pNode ){
    assert( pNode->nRef>0 );
    assert( pRtree->nNodeRef>0 );
    pNode->nRef--;
    if( pNode->nRef==0 ){
      pRtree->nNodeRef--;
      if( pNode->iNode==1 ){
        pRtree->iDepth = -1;
      }
      if( pNode->pParent ){
        rc = nodeRelease(pRtree, pNode->pParent);
      }
      if( rc==SQLITE_OK ){
        rc = nodeWrite(pRtree, pNode);
      }
      nodeHashDelete(pRtree, pNode);
      sqlite3_free(pNode);
    }
  }
  return rc;
}

/*
** Return the 64-bit integer value associated with cell iCell of
** node pNode. If pNode is a leaf node, this is a rowid. If it is
** an internal node, then the 64-bit integer is a child page number.
*/
static i64 nodeGetRowid(
  Rtree *pRtree,       /* The overall R-Tree */
  RtreeNode *pNode,    /* The node from which to extract the ID */
  int iCell            /* The cell index from which to extract the ID */
){
  assert( iCell<NCELL(pNode) );
  return readInt64(&pNode->zData[4 + pRtree->nBytesPerCell*iCell]);
}

/*
** Return coordinate iCoord from cell iCell in node pNode.
*/
static void nodeGetCoord(
  Rtree *pRtree,               /* The overall R-Tree */
  RtreeNode *pNode,            /* The node from which to extract a coordinate */
  int iCell,                   /* The index of the cell within the node */
  int iCoord,                  /* Which coordinate to extract */
  RtreeCoord *pCoord           /* OUT: Space to write result to */
){
  assert( iCell<NCELL(pNode) );
  readCoord(&pNode->zData[12 + pRtree->nBytesPerCell*iCell + 4*iCoord], pCoord);
}

/*
** Deserialize cell iCell of node pNode. Populate the structure pointed
** to by pCell with the results.
*/
static void nodeGetCell(
  Rtree *pRtree,               /* The overall R-Tree */
  RtreeNode *pNode,            /* The node containing the cell to be read */
  int iCell,                   /* Index of the cell within the node */
  RtreeCell *pCell             /* OUT: Write the cell contents here */
){
  u8 *pData;
  RtreeCoord *pCoord;
  int ii = 0;
  pCell->iRowid = nodeGetRowid(pRtree, pNode, iCell);
  pData = pNode->zData + (12 + pRtree->nBytesPerCell*iCell);
  pCoord = pCell->aCoord;
  do{
    readCoord(pData, &pCoord[ii]);
    readCoord(pData+4, &pCoord[ii+1]);
    pData += 8;
    ii += 2;
  }while( ii<pRtree->nDim2 );
}


/* Forward declaration for the function that does the work of
** the virtual table module xCreate() and xConnect() methods.
*/
static int rtreeInit(
  sqlite3 *, void *, int, const char *const*, sqlite3_vtab **, char **, int
);

/*
** Rtree virtual table module xCreate method.
*/
static int rtreeCreate(
  sqlite3 *db,
  void *pAux,
  int argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  return rtreeInit(db, pAux, argc, argv, ppVtab, pzErr, 1);
}

/*
** Rtree virtual table module xConnect method.
*/
static int rtreeConnect(
  sqlite3 *db,
  void *pAux,
  int argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  return rtreeInit(db, pAux, argc, argv, ppVtab, pzErr, 0);
}

/*
** Increment the r-tree reference count.
*/
static void rtreeReference(Rtree *pRtree){
  pRtree->nBusy++;
}

/*
** Decrement the r-tree reference count. When the reference count reaches
** zero the structure is deleted.
*/
static void rtreeRelease(Rtree *pRtree){
  pRtree->nBusy--;
  if( pRtree->nBusy==0 ){
    pRtree->inWrTrans = 0;
    assert( pRtree->nCursor==0 );
    nodeBlobReset(pRtree);
    if( pRtree->nNodeRef ){
      int i;
      assert( pRtree->bCorrupt );
      for(i=0; i<HASHSIZE; i++){
        while( pRtree->aHash[i] ){
          RtreeNode *pNext = pRtree->aHash[i]->pNext;
          sqlite3_free(pRtree->aHash[i]);
          pRtree->aHash[i] = pNext;
        }
      }
    }
    sqlite3_finalize(pRtree->pWriteNode);
    sqlite3_finalize(pRtree->pDeleteNode);
    sqlite3_finalize(pRtree->pReadRowid);
    sqlite3_finalize(pRtree->pWriteRowid);
    sqlite3_finalize(pRtree->pDeleteRowid);
    sqlite3_finalize(pRtree->pReadParent);
    sqlite3_finalize(pRtree->pWriteParent);
    sqlite3_finalize(pRtree->pDeleteParent);
    sqlite3_finalize(pRtree->pWriteAux);
    sqlite3_free(pRtree->zReadAuxSql);
    sqlite3_free(pRtree);
  }
}

/*
** Rtree virtual table module xDisconnect method.
*/
static int rtreeDisconnect(sqlite3_vtab *pVtab){
  rtreeRelease((Rtree *)pVtab);
  return SQLITE_OK;
}

/*
** Rtree virtual table module xDestroy method.
*/
static int rtreeDestroy(sqlite3_vtab *pVtab){
  Rtree *pRtree = (Rtree *)pVtab;
  int rc;
  char *zCreate = sqlite3_mprintf(
    "DROP TABLE '%q'.'%q_node';"
    "DROP TABLE '%q'.'%q_rowid';"
    "DROP TABLE '%q'.'%q_parent';",
    pRtree->zDb, pRtree->zName,
    pRtree->zDb, pRtree->zName,
    pRtree->zDb, pRtree->zName
  );
  if( !zCreate ){
    rc = SQLITE_NOMEM;
  }else{
    nodeBlobReset(pRtree);
    rc = sqlite3_exec(pRtree->db, zCreate, 0, 0, 0);
    sqlite3_free(zCreate);
  }
  if( rc==SQLITE_OK ){
    rtreeRelease(pRtree);
  }

  return rc;
}

/*
** Rtree virtual table module xOpen method.
*/
static int rtreeOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  int rc = SQLITE_NOMEM;
  Rtree *pRtree = (Rtree *)pVTab;
  RtreeCursor *pCsr;

  pCsr = (RtreeCursor *)sqlite3_malloc64(sizeof(RtreeCursor));
  if( pCsr ){
    memset(pCsr, 0, sizeof(RtreeCursor));
    pCsr->base.pVtab = pVTab;
    rc = SQLITE_OK;
    pRtree->nCursor++;
  }
  *ppCursor = (sqlite3_vtab_cursor *)pCsr;

  return rc;
}


/*
** Reset a cursor back to its initial state.
*/
static void resetCursor(RtreeCursor *pCsr){
  Rtree *pRtree = (Rtree *)(pCsr->base.pVtab);
  int ii;
  sqlite3_stmt *pStmt;
  if( pCsr->aConstraint ){
    int i;                        /* Used to iterate through constraint array */
    for(i=0; i<pCsr->nConstraint; i++){
      sqlite3_rtree_query_info *pInfo = pCsr->aConstraint[i].pInfo;
      if( pInfo ){
        if( pInfo->xDelUser ) pInfo->xDelUser(pInfo->pUser);
        sqlite3_free(pInfo);
      }
    }
    sqlite3_free(pCsr->aConstraint);
    pCsr->aConstraint = 0;
  }
  for(ii=0; ii<RTREE_CACHE_SZ; ii++) nodeRelease(pRtree, pCsr->aNode[ii]);
  sqlite3_free(pCsr->aPoint);
  pStmt = pCsr->pReadAux;
  memset(pCsr, 0, sizeof(RtreeCursor));
  pCsr->base.pVtab = (sqlite3_vtab*)pRtree;
  pCsr->pReadAux = pStmt;

  /* The following will only fail if the previous sqlite3_step() call failed,
  ** in which case the error has already been caught. This statement never
  ** encounters an error within an sqlite3_column_xxx() function, as it
  ** calls sqlite3_column_value(), which does not use malloc(). So it is safe
  ** to ignore the error code here.  */
  sqlite3_reset(pStmt);
}

/*
** Rtree virtual table module xClose method.
*/
static int rtreeClose(sqlite3_vtab_cursor *cur){
  Rtree *pRtree = (Rtree *)(cur->pVtab);
  RtreeCursor *pCsr = (RtreeCursor *)cur;
  assert( pRtree->nCursor>0 );
  resetCursor(pCsr);
  sqlite3_finalize(pCsr->pReadAux);
  sqlite3_free(pCsr);
  pRtree->nCursor--;
  if( pRtree->nCursor==0 && pRtree->inWrTrans==0 ){
    nodeBlobReset(pRtree);
  }
  return SQLITE_OK;
}

/*
** Rtree virtual table module xEof method.
**
** Return non-zero if the cursor does not currently point to a valid
** record (i.e if the scan has finished), or zero otherwise.
*/
static int rtreeEof(sqlite3_vtab_cursor *cur){
  RtreeCursor *pCsr = (RtreeCursor *)cur;
  return pCsr->atEOF;
}

/*
** Convert raw bits from the on-disk RTree record into a coordinate value.
** The on-disk format is big-endian and needs to be converted for little-
** endian platforms.  The on-disk record stores integer coordinates if
** eInt is true and it stores 32-bit floating point records if eInt is
** false.  a[] is the four bytes of the on-disk record to be decoded.
** Store the results in "r".
**
** There are five versions of this macro.  The last one is generic.  The
** other four are various architectures-specific optimizations.
*/
#if SQLITE_BYTEORDER==1234 && MSVC_VERSION>=1300
#define RTREE_DECODE_COORD(eInt, a, r) {                        \
    RtreeCoord c;    /* Coordinate decoded */                   \
    c.u = _byteswap_ulong(*(u32*)a);                            \
    r = eInt ? (sqlite3_rtree_dbl)c.i : (sqlite3_rtree_dbl)c.f; \
}
#elif SQLITE_BYTEORDER==1234 && GCC_VERSION>=4003000
#define RTREE_DECODE_COORD(eInt, a, r) {                        \
    RtreeCoord c;    /* Coordinate decoded */                   \
    c.u = __builtin_bswap32(*(u32*)a);                          \
    r = eInt ? (sqlite3_rtree_dbl)c.i : (sqlite3_rtree_dbl)c.f; \
}
#elif SQLITE_BYTEORDER==1234
#define RTREE_DECODE_COORD(eInt, a, r) {                        \
    RtreeCoord c;    /* Coordinate decoded */                   \
    memcpy(&c.u,a,4);                                           \
    c.u = ((c.u>>24)&0xff)|((c.u>>8)&0xff00)|                   \
          ((c.u&0xff)<<24)|((c.u&0xff00)<<8);                   \
    r = eInt ? (sqlite3_rtree_dbl)c.i : (sqlite3_rtree_dbl)c.f; \
}
#elif SQLITE_BYTEORDER==4321
#define RTREE_DECODE_COORD(eInt, a, r) {                        \
    RtreeCoord c;    /* Coordinate decoded */                   \
    memcpy(&c.u,a,4);                                           \
    r = eInt ? (sqlite3_rtree_dbl)c.i : (sqlite3_rtree_dbl)c.f; \
}
#else
#define RTREE_DECODE_COORD(eInt, a, r) {                        \
    RtreeCoord c;    /* Coordinate decoded */                   \
    c.u = ((u32)a[0]<<24) + ((u32)a[1]<<16)                     \
           +((u32)a[2]<<8) + a[3];                              \
    r = eInt ? (sqlite3_rtree_dbl)c.i : (sqlite3_rtree_dbl)c.f; \
}
#endif

/*
** Check the RTree node or entry given by pCellData and p against the MATCH
** constraint pConstraint.
*/
static int rtreeCallbackConstraint(
  RtreeConstraint *pConstraint,  /* The constraint to test */
  int eInt,                      /* True if RTree holding integer coordinates */
  u8 *pCellData,                 /* Raw cell content */
  RtreeSearchPoint *pSearch,     /* Container of this cell */
  sqlite3_rtree_dbl *prScore,    /* OUT: score for the cell */
  int *peWithin                  /* OUT: visibility of the cell */
){
  sqlite3_rtree_query_info *pInfo = pConstraint->pInfo; /* Callback info */
  int nCoord = pInfo->nCoord;                           /* No. of coordinates */
  int rc;                                             /* Callback return code */
  RtreeCoord c;                                       /* Translator union */
  sqlite3_rtree_dbl aCoord[RTREE_MAX_DIMENSIONS*2];   /* Decoded coordinates */

  assert( pConstraint->op==RTREE_MATCH || pConstraint->op==RTREE_QUERY );
  assert( nCoord==2 || nCoord==4 || nCoord==6 || nCoord==8 || nCoord==10 );

  if( pConstraint->op==RTREE_QUERY && pSearch->iLevel==1 ){
    pInfo->iRowid = readInt64(pCellData);
  }
  pCellData += 8;
#ifndef SQLITE_RTREE_INT_ONLY
  if( eInt==0 ){
    switch( nCoord ){
      case 10:  readCoord(pCellData+36, &c); aCoord[9] = c.f;
                readCoord(pCellData+32, &c); aCoord[8] = c.f;
      case 8:   readCoord(pCellData+28, &c); aCoord[7] = c.f;
                readCoord(pCellData+24, &c); aCoord[6] = c.f;
      case 6:   readCoord(pCellData+20, &c); aCoord[5] = c.f;
                readCoord(pCellData+16, &c); aCoord[4] = c.f;
      case 4:   readCoord(pCellData+12, &c); aCoord[3] = c.f;
                readCoord(pCellData+8,  &c); aCoord[2] = c.f;
      default:  readCoord(pCellData+4,  &c); aCoord[1] = c.f;
                readCoord(pCellData,    &c); aCoord[0] = c.f;
    }
  }else
#endif
  {
    switch( nCoord ){
      case 10:  readCoord(pCellData+36, &c); aCoord[9] = c.i;
                readCoord(pCellData+32, &c); aCoord[8] = c.i;
      case 8:   readCoord(pCellData+28, &c); aCoord[7] = c.i;
                readCoord(pCellData+24, &c); aCoord[6] = c.i;
      case 6:   readCoord(pCellData+20, &c); aCoord[5] = c.i;
                readCoord(pCellData+16, &c); aCoord[4] = c.i;
      case 4:   readCoord(pCellData+12, &c); aCoord[3] = c.i;
                readCoord(pCellData+8,  &c); aCoord[2] = c.i;
      default:  readCoord(pCellData+4,  &c); aCoord[1] = c.i;
                readCoord(pCellData,    &c); aCoord[0] = c.i;
    }
  }
  if( pConstraint->op==RTREE_MATCH ){
    int eWithin = 0;
    rc = pConstraint->u.xGeom((sqlite3_rtree_geometry*)pInfo,
                              nCoord, aCoord, &eWithin);
    if( eWithin==0 ) *peWithin = NOT_WITHIN;
    *prScore = RTREE_ZERO;
  }else{
    pInfo->aCoord = aCoord;
    pInfo->iLevel = pSearch->iLevel - 1;
    pInfo->rScore = pInfo->rParentScore = pSearch->rScore;
    pInfo->eWithin = pInfo->eParentWithin = pSearch->eWithin;
    rc = pConstraint->u.xQueryFunc(pInfo);
    if( pInfo->eWithin<*peWithin ) *peWithin = pInfo->eWithin;
    if( pInfo->rScore<*prScore || *prScore<RTREE_ZERO ){
      *prScore = pInfo->rScore;
    }
  }
  return rc;
}

/*
** Check the internal RTree node given by pCellData against constraint p.
** If this constraint cannot be satisfied by any child within the node,
** set *peWithin to NOT_WITHIN.
*/
static void rtreeNonleafConstraint(
  RtreeConstraint *p,        /* The constraint to test */
  int eInt,                  /* True if RTree holds integer coordinates */
  u8 *pCellData,             /* Raw cell content as appears on disk */
  int *peWithin              /* Adjust downward, as appropriate */
){
  sqlite3_rtree_dbl val;     /* Coordinate value convert to a double */

  /* p->iCoord might point to either a lower or upper bound coordinate
  ** in a coordinate pair.  But make pCellData point to the lower bound.
  */
  pCellData += 8 + 4*(p->iCoord&0xfe);

  assert(p->op==RTREE_LE || p->op==RTREE_LT || p->op==RTREE_GE
      || p->op==RTREE_GT || p->op==RTREE_EQ || p->op==RTREE_TRUE
      || p->op==RTREE_FALSE );
  assert( FOUR_BYTE_ALIGNED(pCellData) );
  switch( p->op ){
    case RTREE_TRUE:  return;   /* Always satisfied */
    case RTREE_FALSE: break;    /* Never satisfied */
    case RTREE_EQ:
      RTREE_DECODE_COORD(eInt, pCellData, val);
      /* val now holds the lower bound of the coordinate pair */
      if( p->u.rValue>=val ){
        pCellData += 4;
        RTREE_DECODE_COORD(eInt, pCellData, val);
        /* val now holds the upper bound of the coordinate pair */
        if( p->u.rValue<=val ) return;
      }
      break;
    case RTREE_LE:
    case RTREE_LT:
      RTREE_DECODE_COORD(eInt, pCellData, val);
      /* val now holds the lower bound of the coordinate pair */
      if( p->u.rValue>=val ) return;
      break;

    default:
      pCellData += 4;
      RTREE_DECODE_COORD(eInt, pCellData, val);
      /* val now holds the upper bound of the coordinate pair */
      if( p->u.rValue<=val ) return;
      break;
  }
  *peWithin = NOT_WITHIN;
}

/*
** Check the leaf RTree cell given by pCellData against constraint p.
** If this constraint is not satisfied, set *peWithin to NOT_WITHIN.
** If the constraint is satisfied, leave *peWithin unchanged.
**
** The constraint is of the form:  xN op $val
**
** The op is given by p->op.  The xN is p->iCoord-th coordinate in
** pCellData.  $val is given by p->u.rValue.
*/
static void rtreeLeafConstraint(
  RtreeConstraint *p,        /* The constraint to test */
  int eInt,                  /* True if RTree holds integer coordinates */
  u8 *pCellData,             /* Raw cell content as appears on disk */
  int *peWithin              /* Adjust downward, as appropriate */
){
  RtreeDValue xN;      /* Coordinate value converted to a double */

  assert(p->op==RTREE_LE || p->op==RTREE_LT || p->op==RTREE_GE
      || p->op==RTREE_GT || p->op==RTREE_EQ || p->op==RTREE_TRUE
      || p->op==RTREE_FALSE );
  pCellData += 8 + p->iCoord*4;
  assert( FOUR_BYTE_ALIGNED(pCellData) );
  RTREE_DECODE_COORD(eInt, pCellData, xN);
  switch( p->op ){
    case RTREE_TRUE:  return;   /* Always satisfied */
    case RTREE_FALSE: break;    /* Never satisfied */
    case RTREE_LE:    if( xN <= p->u.rValue ) return;  break;
    case RTREE_LT:    if( xN <  p->u.rValue ) return;  break;
    case RTREE_GE:    if( xN >= p->u.rValue ) return;  break;
    case RTREE_GT:    if( xN >  p->u.rValue ) return;  break;
    default:          if( xN == p->u.rValue ) return;  break;
  }
  *peWithin = NOT_WITHIN;
}

/*
** One of the cells in node pNode is guaranteed to have a 64-bit
** integer value equal to iRowid. Return the index of this cell.
*/
static int nodeRowidIndex(
  Rtree *pRtree,
  RtreeNode *pNode,
  i64 iRowid,
  int *piIndex
){
  int ii;
  int nCell = NCELL(pNode);
  assert( nCell<200 );
  for(ii=0; ii<nCell; ii++){
    if( nodeGetRowid(pRtree, pNode, ii)==iRowid ){
      *piIndex = ii;
      return SQLITE_OK;
    }
  }
  RTREE_IS_CORRUPT(pRtree);
  return SQLITE_CORRUPT_VTAB;
}

/*
** Return the index of the cell containing a pointer to node pNode
** in its parent. If pNode is the root node, return -1.
*/
static int nodeParentIndex(Rtree *pRtree, RtreeNode *pNode, int *piIndex){
  RtreeNode *pParent = pNode->pParent;
  if( ALWAYS(pParent) ){
    return nodeRowidIndex(pRtree, pParent, pNode->iNode, piIndex);
  }else{
    *piIndex = -1;
    return SQLITE_OK;
  }
}

/*
** Compare two search points.  Return negative, zero, or positive if the first
** is less than, equal to, or greater than the second.
**
** The rScore is the primary key.  Smaller rScore values come first.
** If the rScore is a tie, then use iLevel as the tie breaker with smaller
** iLevel values coming first.  In this way, if rScore is the same for all
** SearchPoints, then iLevel becomes the deciding factor and the result
** is a depth-first search, which is the desired default behavior.
*/
static int rtreeSearchPointCompare(
  const RtreeSearchPoint *pA,
  const RtreeSearchPoint *pB
){
  if( pA->rScore<pB->rScore ) return -1;
  if( pA->rScore>pB->rScore ) return +1;
  if( pA->iLevel<pB->iLevel ) return -1;
  if( pA->iLevel>pB->iLevel ) return +1;
  return 0;
}

/*
** Interchange two search points in a cursor.
*/
static void rtreeSearchPointSwap(RtreeCursor *p, int i, int j){
  RtreeSearchPoint t = p->aPoint[i];
  assert( i<j );
  p->aPoint[i] = p->aPoint[j];
  p->aPoint[j] = t;
  i++; j++;
  if( i<RTREE_CACHE_SZ ){
    if( j>=RTREE_CACHE_SZ ){
      nodeRelease(RTREE_OF_CURSOR(p), p->aNode[i]);
      p->aNode[i] = 0;
    }else{
      RtreeNode *pTemp = p->aNode[i];
      p->aNode[i] = p->aNode[j];
      p->aNode[j] = pTemp;
    }
  }
}

/*
** Return the search point with the lowest current score.
*/
static RtreeSearchPoint *rtreeSearchPointFirst(RtreeCursor *pCur){
  return pCur->bPoint ? &pCur->sPoint : pCur->nPoint ? pCur->aPoint : 0;
}

/*
** Get the RtreeNode for the search point with the lowest score.
*/
static RtreeNode *rtreeNodeOfFirstSearchPoint(RtreeCursor *pCur, int *pRC){
  sqlite3_int64 id;
  int ii = 1 - pCur->bPoint;
  assert( ii==0 || ii==1 );
  assert( pCur->bPoint || pCur->nPoint );
  if( pCur->aNode[ii]==0 ){
    assert( pRC!=0 );
    id = ii ? pCur->aPoint[0].id : pCur->sPoint.id;
    *pRC = nodeAcquire(RTREE_OF_CURSOR(pCur), id, 0, &pCur->aNode[ii]);
  }
  return pCur->aNode[ii];
}

/*
** Push a new element onto the priority queue
*/
static RtreeSearchPoint *rtreeEnqueue(
  RtreeCursor *pCur,    /* The cursor */
  RtreeDValue rScore,   /* Score for the new search point */
  u8 iLevel             /* Level for the new search point */
){
  int i, j;
  RtreeSearchPoint *pNew;
  if( pCur->nPoint>=pCur->nPointAlloc ){
    int nNew = pCur->nPointAlloc*2 + 8;
    pNew = sqlite3_realloc64(pCur->aPoint, nNew*sizeof(pCur->aPoint[0]));
    if( pNew==0 ) return 0;
    pCur->aPoint = pNew;
    pCur->nPointAlloc = nNew;
  }
  i = pCur->nPoint++;
  pNew = pCur->aPoint + i;
  pNew->rScore = rScore;
  pNew->iLevel = iLevel;
  assert( iLevel<=RTREE_MAX_DEPTH );
  while( i>0 ){
    RtreeSearchPoint *pParent;
    j = (i-1)/2;
    pParent = pCur->aPoint + j;
    if( rtreeSearchPointCompare(pNew, pParent)>=0 ) break;
    rtreeSearchPointSwap(pCur, j, i);
    i = j;
    pNew = pParent;
  }
  return pNew;
}

/*
** Allocate a new RtreeSearchPoint and return a pointer to it.  Return
** NULL if malloc fails.
*/
static RtreeSearchPoint *rtreeSearchPointNew(
  RtreeCursor *pCur,    /* The cursor */
  RtreeDValue rScore,   /* Score for the new search point */
  u8 iLevel             /* Level for the new search point */
){
  RtreeSearchPoint *pNew, *pFirst;
  pFirst = rtreeSearchPointFirst(pCur);
  pCur->anQueue[iLevel]++;
  if( pFirst==0
   || pFirst->rScore>rScore
   || (pFirst->rScore==rScore && pFirst->iLevel>iLevel)
  ){
    if( pCur->bPoint ){
      int ii;
      pNew = rtreeEnqueue(pCur, rScore, iLevel);
      if( pNew==0 ) return 0;
      ii = (int)(pNew - pCur->aPoint) + 1;
      assert( ii==1 );
      if( ALWAYS(ii<RTREE_CACHE_SZ) ){
        assert( pCur->aNode[ii]==0 );
        pCur->aNode[ii] = pCur->aNode[0];
      }else{
        nodeRelease(RTREE_OF_CURSOR(pCur), pCur->aNode[0]);
      }
      pCur->aNode[0] = 0;
      *pNew = pCur->sPoint;
    }
    pCur->sPoint.rScore = rScore;
    pCur->sPoint.iLevel = iLevel;
    pCur->bPoint = 1;
    return &pCur->sPoint;
  }else{
    return rtreeEnqueue(pCur, rScore, iLevel);
  }
}

#if 0
/* Tracing routines for the RtreeSearchPoint queue */
static void tracePoint(RtreeSearchPoint *p, int idx, RtreeCursor *pCur){
  if( idx<0 ){ printf(" s"); }else{ printf("%2d", idx); }
  printf(" %d.%05lld.%02d %g %d",
    p->iLevel, p->id, p->iCell, p->rScore, p->eWithin
  );
  idx++;
  if( idx<RTREE_CACHE_SZ ){
    printf(" %p\n", pCur->aNode[idx]);
  }else{
    printf("\n");
  }
}
static void traceQueue(RtreeCursor *pCur, const char *zPrefix){
  int ii;
  printf("=== %9s ", zPrefix);
  if( pCur->bPoint ){
    tracePoint(&pCur->sPoint, -1, pCur);
  }
  for(ii=0; ii<pCur->nPoint; ii++){
    if( ii>0 || pCur->bPoint ) printf("              ");
    tracePoint(&pCur->aPoint[ii], ii, pCur);
  }
}
# define RTREE_QUEUE_TRACE(A,B) traceQueue(A,B)
#else
# define RTREE_QUEUE_TRACE(A,B)   /* no-op */
#endif

/* Remove the search point with the lowest current score.
*/
static void rtreeSearchPointPop(RtreeCursor *p){
  int i, j, k, n;
  i = 1 - p->bPoint;
  assert( i==0 || i==1 );
  if( p->aNode[i] ){
    nodeRelease(RTREE_OF_CURSOR(p), p->aNode[i]);
    p->aNode[i] = 0;
  }
  if( p->bPoint ){
    p->anQueue[p->sPoint.iLevel]--;
    p->bPoint = 0;
  }else if( ALWAYS(p->nPoint) ){
    p->anQueue[p->aPoint[0].iLevel]--;
    n = --p->nPoint;
    p->aPoint[0] = p->aPoint[n];
    if( n<RTREE_CACHE_SZ-1 ){
      p->aNode[1] = p->aNode[n+1];
      p->aNode[n+1] = 0;
    }
    i = 0;
    while( (j = i*2+1)<n ){
      k = j+1;
      if( k<n && rtreeSearchPointCompare(&p->aPoint[k], &p->aPoint[j])<0 ){
        if( rtreeSearchPointCompare(&p->aPoint[k], &p->aPoint[i])<0 ){
          rtreeSearchPointSwap(p, i, k);
          i = k;
        }else{
          break;
        }
      }else{
        if( rtreeSearchPointCompare(&p->aPoint[j], &p->aPoint[i])<0 ){
          rtreeSearchPointSwap(p, i, j);
          i = j;
        }else{
          break;
        }
      }
    }
  }
}


/*
** Continue the search on cursor pCur until the front of the queue
** contains an entry suitable for returning as a result-set row,
** or until the RtreeSearchPoint queue is empty, indicating that the
** query has completed.
*/
static int rtreeStepToLeaf(RtreeCursor *pCur){
  RtreeSearchPoint *p;
  Rtree *pRtree = RTREE_OF_CURSOR(pCur);
  RtreeNode *pNode;
  int eWithin;
  int rc = SQLITE_OK;
  int nCell;
  int nConstraint = pCur->nConstraint;
  int ii;
  int eInt;
  RtreeSearchPoint x;

  eInt = pRtree->eCoordType==RTREE_COORD_INT32;
  while( (p = rtreeSearchPointFirst(pCur))!=0 && p->iLevel>0 ){
    u8 *pCellData;
    pNode = rtreeNodeOfFirstSearchPoint(pCur, &rc);
    if( rc ) return rc;
    nCell = NCELL(pNode);
    assert( nCell<200 );
    pCellData = pNode->zData + (4+pRtree->nBytesPerCell*p->iCell);
    while( p->iCell<nCell ){
      sqlite3_rtree_dbl rScore = (sqlite3_rtree_dbl)-1;
      eWithin = FULLY_WITHIN;
      for(ii=0; ii<nConstraint; ii++){
        RtreeConstraint *pConstraint = pCur->aConstraint + ii;
        if( pConstraint->op>=RTREE_MATCH ){
          rc = rtreeCallbackConstraint(pConstraint, eInt, pCellData, p,
                                       &rScore, &eWithin);
          if( rc ) return rc;
        }else if( p->iLevel==1 ){
          rtreeLeafConstraint(pConstraint, eInt, pCellData, &eWithin);
        }else{
          rtreeNonleafConstraint(pConstraint, eInt, pCellData, &eWithin);
        }
        if( eWithin==NOT_WITHIN ){
          p->iCell++;
          pCellData += pRtree->nBytesPerCell;
          break;
        }
      }
      if( eWithin==NOT_WITHIN ) continue;
      p->iCell++;
      x.iLevel = p->iLevel - 1;
      if( x.iLevel ){
        x.id = readInt64(pCellData);
        for(ii=0; ii<pCur->nPoint; ii++){
          if( pCur->aPoint[ii].id==x.id ){
            RTREE_IS_CORRUPT(pRtree);
            return SQLITE_CORRUPT_VTAB;
          }
        }
        x.iCell = 0;
      }else{
        x.id = p->id;
        x.iCell = p->iCell - 1;
      }
      if( p->iCell>=nCell ){
        RTREE_QUEUE_TRACE(pCur, "POP-S:");
        rtreeSearchPointPop(pCur);
      }
      if( rScore<RTREE_ZERO ) rScore = RTREE_ZERO;
      p = rtreeSearchPointNew(pCur, rScore, x.iLevel);
      if( p==0 ) return SQLITE_NOMEM;
      p->eWithin = (u8)eWithin;
      p->id = x.id;
      p->iCell = x.iCell;
      RTREE_QUEUE_TRACE(pCur, "PUSH-S:");
      break;
    }
    if( p->iCell>=nCell ){
      RTREE_QUEUE_TRACE(pCur, "POP-Se:");
      rtreeSearchPointPop(pCur);
    }
  }
  pCur->atEOF = p==0;
  return SQLITE_OK;
}

/*
** Rtree virtual table module xNext method.
*/
static int rtreeNext(sqlite3_vtab_cursor *pVtabCursor){
  RtreeCursor *pCsr = (RtreeCursor *)pVtabCursor;
  int rc = SQLITE_OK;

  /* Move to the next entry that matches the configured constraints. */
  RTREE_QUEUE_TRACE(pCsr, "POP-Nx:");
  if( pCsr->bAuxValid ){
    pCsr->bAuxValid = 0;
    sqlite3_reset(pCsr->pReadAux);
  }
  rtreeSearchPointPop(pCsr);
  rc = rtreeStepToLeaf(pCsr);
  return rc;
}

/*
** Rtree virtual table module xRowid method.
*/
static int rtreeRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *pRowid){
  RtreeCursor *pCsr = (RtreeCursor *)pVtabCursor;
  RtreeSearchPoint *p = rtreeSearchPointFirst(pCsr);
  int rc = SQLITE_OK;
  RtreeNode *pNode = rtreeNodeOfFirstSearchPoint(pCsr, &rc);
  if( rc==SQLITE_OK && ALWAYS(p) ){
    if( p->iCell>=NCELL(pNode) ){
      rc = SQLITE_ABORT;
    }else{
      *pRowid = nodeGetRowid(RTREE_OF_CURSOR(pCsr), pNode, p->iCell);
    }
  }
  return rc;
}

/*
** Rtree virtual table module xColumn method.
*/
static int rtreeColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, int i){
  Rtree *pRtree = (Rtree *)cur->pVtab;
  RtreeCursor *pCsr = (RtreeCursor *)cur;
  RtreeSearchPoint *p = rtreeSearchPointFirst(pCsr);
  RtreeCoord c;
  int rc = SQLITE_OK;
  RtreeNode *pNode = rtreeNodeOfFirstSearchPoint(pCsr, &rc);

  if( rc ) return rc;
  if( NEVER(p==0) ) return SQLITE_OK;
  if( p->iCell>=NCELL(pNode) ) return SQLITE_ABORT;
  if( i==0 ){
    sqlite3_result_int64(ctx, nodeGetRowid(pRtree, pNode, p->iCell));
  }else if( i<=pRtree->nDim2 ){
    nodeGetCoord(pRtree, pNode, p->iCell, i-1, &c);
#ifndef SQLITE_RTREE_INT_ONLY
    if( pRtree->eCoordType==RTREE_COORD_REAL32 ){
      sqlite3_result_double(ctx, c.f);
    }else
#endif
    {
      assert( pRtree->eCoordType==RTREE_COORD_INT32 );
      sqlite3_result_int(ctx, c.i);
    }
  }else{
    if( !pCsr->bAuxValid ){
      if( pCsr->pReadAux==0 ){
        rc = sqlite3_prepare_v3(pRtree->db, pRtree->zReadAuxSql, -1, 0,
                                &pCsr->pReadAux, 0);
        if( rc ) return rc;
      }
      sqlite3_bind_int64(pCsr->pReadAux, 1,
          nodeGetRowid(pRtree, pNode, p->iCell));
      rc = sqlite3_step(pCsr->pReadAux);
      if( rc==SQLITE_ROW ){
        pCsr->bAuxValid = 1;
      }else{
        sqlite3_reset(pCsr->pReadAux);
        if( rc==SQLITE_DONE ) rc = SQLITE_OK;
        return rc;
      }
    }
    sqlite3_result_value(ctx,
         sqlite3_column_value(pCsr->pReadAux, i - pRtree->nDim2 + 1));
  }
  return SQLITE_OK;
}

/*
** Use nodeAcquire() to obtain the leaf node containing the record with
** rowid iRowid. If successful, set *ppLeaf to point to the node and
** return SQLITE_OK. If there is no such record in the table, set
** *ppLeaf to 0 and return SQLITE_OK. If an error occurs, set *ppLeaf
** to zero and return an SQLite error code.
*/
static int findLeafNode(
  Rtree *pRtree,              /* RTree to search */
  i64 iRowid,                 /* The rowid searching for */
  RtreeNode **ppLeaf,         /* Write the node here */
  sqlite3_int64 *piNode       /* Write the node-id here */
){
  int rc;
  *ppLeaf = 0;
  sqlite3_bind_int64(pRtree->pReadRowid, 1, iRowid);
  if( sqlite3_step(pRtree->pReadRowid)==SQLITE_ROW ){
    i64 iNode = sqlite3_column_int64(pRtree->pReadRowid, 0);
    if( piNode ) *piNode = iNode;
    rc = nodeAcquire(pRtree, iNode, 0, ppLeaf);
    sqlite3_reset(pRtree->pReadRowid);
  }else{
    rc = sqlite3_reset(pRtree->pReadRowid);
  }
  return rc;
}

/*
** This function is called to configure the RtreeConstraint object passed
** as the second argument for a MATCH constraint. The value passed as the
** first argument to this function is the right-hand operand to the MATCH
** operator.
*/
static int deserializeGeometry(sqlite3_value *pValue, RtreeConstraint *pCons){
  RtreeMatchArg *pBlob, *pSrc;       /* BLOB returned by geometry function */
  sqlite3_rtree_query_info *pInfo;   /* Callback information */

  pSrc = sqlite3_value_pointer(pValue, "RtreeMatchArg");
  if( pSrc==0 ) return SQLITE_ERROR;
  pInfo = (sqlite3_rtree_query_info*)
                sqlite3_malloc64( sizeof(*pInfo)+pSrc->iSize );
  if( !pInfo ) return SQLITE_NOMEM;
  memset(pInfo, 0, sizeof(*pInfo));
  pBlob = (RtreeMatchArg*)&pInfo[1];
  memcpy(pBlob, pSrc, pSrc->iSize);
  pInfo->pContext = pBlob->cb.pContext;
  pInfo->nParam = pBlob->nParam;
  pInfo->aParam = pBlob->aParam;
  pInfo->apSqlParam = pBlob->apSqlParam;

  if( pBlob->cb.xGeom ){
    pCons->u.xGeom = pBlob->cb.xGeom;
  }else{
    pCons->op = RTREE_QUERY;
    pCons->u.xQueryFunc = pBlob->cb.xQueryFunc;
  }
  pCons->pInfo = pInfo;
  return SQLITE_OK;
}

SQLITE_PRIVATE int sqlite3IntFloatCompare(i64,double);

/*
** Rtree virtual table module xFilter method.
*/
static int rtreeFilter(
  sqlite3_vtab_cursor *pVtabCursor,
  int idxNum, const char *idxStr,
  int argc, sqlite3_value **argv
){
  Rtree *pRtree = (Rtree *)pVtabCursor->pVtab;
  RtreeCursor *pCsr = (RtreeCursor *)pVtabCursor;
  RtreeNode *pRoot = 0;
  int ii;
  int rc = SQLITE_OK;
  int iCell = 0;

  rtreeReference(pRtree);

  /* Reset the cursor to the same state as rtreeOpen() leaves it in. */
  resetCursor(pCsr);

  pCsr->iStrategy = idxNum;
  if( idxNum==1 ){
    /* Special case - lookup by rowid. */
    RtreeNode *pLeaf;        /* Leaf on which the required cell resides */
    RtreeSearchPoint *p;     /* Search point for the leaf */
    i64 iRowid = sqlite3_value_int64(argv[0]);
    i64 iNode = 0;
    int eType = sqlite3_value_numeric_type(argv[0]);
    if( eType==SQLITE_INTEGER
     || (eType==SQLITE_FLOAT
         && 0==sqlite3IntFloatCompare(iRowid,sqlite3_value_double(argv[0])))
    ){
      rc = findLeafNode(pRtree, iRowid, &pLeaf, &iNode);
    }else{
      rc = SQLITE_OK;
      pLeaf = 0;
    }
    if( rc==SQLITE_OK && pLeaf!=0 ){
      p = rtreeSearchPointNew(pCsr, RTREE_ZERO, 0);
      assert( p!=0 );  /* Always returns pCsr->sPoint */
      pCsr->aNode[0] = pLeaf;
      p->id = iNode;
      p->eWithin = PARTLY_WITHIN;
      rc = nodeRowidIndex(pRtree, pLeaf, iRowid, &iCell);
      p->iCell = (u8)iCell;
      RTREE_QUEUE_TRACE(pCsr, "PUSH-F1:");
    }else{
      pCsr->atEOF = 1;
    }
  }else{
    /* Normal case - r-tree scan. Set up the RtreeCursor.aConstraint array
    ** with the configured constraints.
    */
    rc = nodeAcquire(pRtree, 1, 0, &pRoot);
    if( rc==SQLITE_OK && argc>0 ){
      pCsr->aConstraint = sqlite3_malloc64(sizeof(RtreeConstraint)*argc);
      pCsr->nConstraint = argc;
      if( !pCsr->aConstraint ){
        rc = SQLITE_NOMEM;
      }else{
        memset(pCsr->aConstraint, 0, sizeof(RtreeConstraint)*argc);
        memset(pCsr->anQueue, 0, sizeof(u32)*(pRtree->iDepth + 1));
        assert( (idxStr==0 && argc==0)
                || (idxStr && (int)strlen(idxStr)==argc*2) );
        for(ii=0; ii<argc; ii++){
          RtreeConstraint *p = &pCsr->aConstraint[ii];
          int eType = sqlite3_value_numeric_type(argv[ii]);
          p->op = idxStr[ii*2];
          p->iCoord = idxStr[ii*2+1]-'0';
          if( p->op>=RTREE_MATCH ){
            /* A MATCH operator. The right-hand-side must be a blob that
            ** can be cast into an RtreeMatchArg object. One created using
            ** an sqlite3_rtree_geometry_callback() SQL user function.
            */
            rc = deserializeGeometry(argv[ii], p);
            if( rc!=SQLITE_OK ){
              break;
            }
            p->pInfo->nCoord = pRtree->nDim2;
            p->pInfo->anQueue = pCsr->anQueue;
            p->pInfo->mxLevel = pRtree->iDepth + 1;
          }else if( eType==SQLITE_INTEGER ){
            sqlite3_int64 iVal = sqlite3_value_int64(argv[ii]);
#ifdef SQLITE_RTREE_INT_ONLY
            p->u.rValue = iVal;
#else
            p->u.rValue = (double)iVal;
            if( iVal>=((sqlite3_int64)1)<<48
             || iVal<=-(((sqlite3_int64)1)<<48)
            ){
              if( p->op==RTREE_LT ) p->op = RTREE_LE;
              if( p->op==RTREE_GT ) p->op = RTREE_GE;
            }
#endif
          }else if( eType==SQLITE_FLOAT ){
#ifdef SQLITE_RTREE_INT_ONLY
            p->u.rValue = sqlite3_value_int64(argv[ii]);
#else
            p->u.rValue = sqlite3_value_double(argv[ii]);
#endif
          }else{
            p->u.rValue = RTREE_ZERO;
            if( eType==SQLITE_NULL ){
              p->op = RTREE_FALSE;
            }else if( p->op==RTREE_LT || p->op==RTREE_LE ){
              p->op = RTREE_TRUE;
            }else{
              p->op = RTREE_FALSE;
            }
          }
        }
      }
    }
    if( rc==SQLITE_OK ){
      RtreeSearchPoint *pNew;
      assert( pCsr->bPoint==0 );  /* Due to the resetCursor() call above */
      pNew = rtreeSearchPointNew(pCsr, RTREE_ZERO, (u8)(pRtree->iDepth+1));
      if( NEVER(pNew==0) ){       /* Because pCsr->bPoint was FALSE */
        return SQLITE_NOMEM;
      }
      pNew->id = 1;
      pNew->iCell = 0;
      pNew->eWithin = PARTLY_WITHIN;
      assert( pCsr->bPoint==1 );
      pCsr->aNode[0] = pRoot;
      pRoot = 0;
      RTREE_QUEUE_TRACE(pCsr, "PUSH-Fm:");
      rc = rtreeStepToLeaf(pCsr);
    }
  }

  nodeRelease(pRtree, pRoot);
  rtreeRelease(pRtree);
  return rc;
}

/*
** Rtree virtual table module xBestIndex method. There are three
** table scan strategies to choose from (in order from most to
** least desirable):
**
**   idxNum     idxStr        Strategy
**   ------------------------------------------------
**     1        Unused        Direct lookup by rowid.
**     2        See below     R-tree query or full-table scan.
**   ------------------------------------------------
**
** If strategy 1 is used, then idxStr is not meaningful. If strategy
** 2 is used, idxStr is formatted to contain 2 bytes for each
** constraint used. The first two bytes of idxStr correspond to
** the constraint in sqlite3_index_info.aConstraintUsage[] with
** (argvIndex==1) etc.
**
** The first of each pair of bytes in idxStr identifies the constraint
** operator as follows:
**
**   Operator    Byte Value
**   ----------------------
**      =        0x41 ('A')
**     <=        0x42 ('B')
**      <        0x43 ('C')
**     >=        0x44 ('D')
**      >        0x45 ('E')
**   MATCH       0x46 ('F')
**   ----------------------
**
** The second of each pair of bytes identifies the coordinate column
** to which the constraint applies. The leftmost coordinate column
** is 'a', the second from the left 'b' etc.
*/
static int rtreeBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  Rtree *pRtree = (Rtree*)tab;
  int rc = SQLITE_OK;
  int ii;
  int bMatch = 0;                 /* True if there exists a MATCH constraint */
  i64 nRow;                       /* Estimated rows returned by this scan */

  int iIdx = 0;
  char zIdxStr[RTREE_MAX_DIMENSIONS*8+1];
  memset(zIdxStr, 0, sizeof(zIdxStr));

  /* Check if there exists a MATCH constraint - even an unusable one. If there
  ** is, do not consider the lookup-by-rowid plan as using such a plan would
  ** require the VDBE to evaluate the MATCH constraint, which is not currently
  ** possible. */
  for(ii=0; ii<pIdxInfo->nConstraint; ii++){
    if( pIdxInfo->aConstraint[ii].op==SQLITE_INDEX_CONSTRAINT_MATCH ){
      bMatch = 1;
    }
  }

  assert( pIdxInfo->idxStr==0 );
  for(ii=0; ii<pIdxInfo->nConstraint && iIdx<(int)(sizeof(zIdxStr)-1); ii++){
    struct sqlite3_index_constraint *p = &pIdxInfo->aConstraint[ii];

    if( bMatch==0 && p->usable
     && p->iColumn<=0 && p->op==SQLITE_INDEX_CONSTRAINT_EQ
    ){
      /* We have an equality constraint on the rowid. Use strategy 1. */
      int jj;
      for(jj=0; jj<ii; jj++){
        pIdxInfo->aConstraintUsage[jj].argvIndex = 0;
        pIdxInfo->aConstraintUsage[jj].omit = 0;
      }
      pIdxInfo->idxNum = 1;
      pIdxInfo->aConstraintUsage[ii].argvIndex = 1;
      pIdxInfo->aConstraintUsage[jj].omit = 1;

      /* This strategy involves a two rowid lookups on an B-Tree structures
      ** and then a linear search of an R-Tree node. This should be
      ** considered almost as quick as a direct rowid lookup (for which
      ** sqlite uses an internal cost of 0.0). It is expected to return
      ** a single row.
      */
      pIdxInfo->estimatedCost = 30.0;
      pIdxInfo->estimatedRows = 1;
      pIdxInfo->idxFlags = SQLITE_INDEX_SCAN_UNIQUE;
      return SQLITE_OK;
    }

    if( p->usable
    && ((p->iColumn>0 && p->iColumn<=pRtree->nDim2)
        || p->op==SQLITE_INDEX_CONSTRAINT_MATCH)
    ){
      u8 op;
      u8 doOmit = 1;
      switch( p->op ){
        case SQLITE_INDEX_CONSTRAINT_EQ:    op = RTREE_EQ;    doOmit = 0; break;
        case SQLITE_INDEX_CONSTRAINT_GT:    op = RTREE_GT;    doOmit = 0; break;
        case SQLITE_INDEX_CONSTRAINT_LE:    op = RTREE_LE;    break;
        case SQLITE_INDEX_CONSTRAINT_LT:    op = RTREE_LT;    doOmit = 0; break;
        case SQLITE_INDEX_CONSTRAINT_GE:    op = RTREE_GE;    break;
        case SQLITE_INDEX_CONSTRAINT_MATCH: op = RTREE_MATCH; break;
        default:                            op = 0;           break;
      }
      if( op ){
        zIdxStr[iIdx++] = op;
        zIdxStr[iIdx++] = (char)(p->iColumn - 1 + '0');
        pIdxInfo->aConstraintUsage[ii].argvIndex = (iIdx/2);
        pIdxInfo->aConstraintUsage[ii].omit = doOmit;
      }
    }
  }

  pIdxInfo->idxNum = 2;
  pIdxInfo->needToFreeIdxStr = 1;
  if( iIdx>0 ){
    pIdxInfo->idxStr = sqlite3_malloc( iIdx+1 );
    if( pIdxInfo->idxStr==0 ){
      return SQLITE_NOMEM;
    }
    memcpy(pIdxInfo->idxStr, zIdxStr, iIdx+1);
  }

  nRow = pRtree->nRowEst >> (iIdx/2);
  pIdxInfo->estimatedCost = (double)6.0 * (double)nRow;
  pIdxInfo->estimatedRows = nRow;

  return rc;
}

/*
** Return the N-dimensional volume of the cell stored in *p.
*/
static RtreeDValue cellArea(Rtree *pRtree, RtreeCell *p){
  RtreeDValue area = (RtreeDValue)1;
  assert( pRtree->nDim>=1 && pRtree->nDim<=5 );
#ifndef SQLITE_RTREE_INT_ONLY
  if( pRtree->eCoordType==RTREE_COORD_REAL32 ){
    switch( pRtree->nDim ){
      case 5:  area  = p->aCoord[9].f - p->aCoord[8].f;
      case 4:  area *= p->aCoord[7].f - p->aCoord[6].f;
      case 3:  area *= p->aCoord[5].f - p->aCoord[4].f;
      case 2:  area *= p->aCoord[3].f - p->aCoord[2].f;
      default: area *= p->aCoord[1].f - p->aCoord[0].f;
    }
  }else
#endif
  {
    switch( pRtree->nDim ){
      case 5:  area  = (i64)p->aCoord[9].i - (i64)p->aCoord[8].i;
      case 4:  area *= (i64)p->aCoord[7].i - (i64)p->aCoord[6].i;
      case 3:  area *= (i64)p->aCoord[5].i - (i64)p->aCoord[4].i;
      case 2:  area *= (i64)p->aCoord[3].i - (i64)p->aCoord[2].i;
      default: area *= (i64)p->aCoord[1].i - (i64)p->aCoord[0].i;
    }
  }
  return area;
}

/*
** Return the margin length of cell p. The margin length is the sum
** of the objects size in each dimension.
*/
static RtreeDValue cellMargin(Rtree *pRtree, RtreeCell *p){
  RtreeDValue margin = 0;
  int ii = pRtree->nDim2 - 2;
  do{
    margin += (DCOORD(p->aCoord[ii+1]) - DCOORD(p->aCoord[ii]));
    ii -= 2;
  }while( ii>=0 );
  return margin;
}

/*
** Store the union of cells p1 and p2 in p1.
*/
static void cellUnion(Rtree *pRtree, RtreeCell *p1, RtreeCell *p2){
  int ii = 0;
  if( pRtree->eCoordType==RTREE_COORD_REAL32 ){
    do{
      p1->aCoord[ii].f = MIN(p1->aCoord[ii].f, p2->aCoord[ii].f);
      p1->aCoord[ii+1].f = MAX(p1->aCoord[ii+1].f, p2->aCoord[ii+1].f);
      ii += 2;
    }while( ii<pRtree->nDim2 );
  }else{
    do{
      p1->aCoord[ii].i = MIN(p1->aCoord[ii].i, p2->aCoord[ii].i);
      p1->aCoord[ii+1].i = MAX(p1->aCoord[ii+1].i, p2->aCoord[ii+1].i);
      ii += 2;
    }while( ii<pRtree->nDim2 );
  }
}

/*
** Return true if the area covered by p2 is a subset of the area covered
** by p1. False otherwise.
*/
static int cellContains(Rtree *pRtree, RtreeCell *p1, RtreeCell *p2){
  int ii;
  if( pRtree->eCoordType==RTREE_COORD_INT32 ){
    for(ii=0; ii<pRtree->nDim2; ii+=2){
      RtreeCoord *a1 = &p1->aCoord[ii];
      RtreeCoord *a2 = &p2->aCoord[ii];
      if( a2[0].i<a1[0].i || a2[1].i>a1[1].i ) return 0;
    }
  }else{
    for(ii=0; ii<pRtree->nDim2; ii+=2){
      RtreeCoord *a1 = &p1->aCoord[ii];
      RtreeCoord *a2 = &p2->aCoord[ii];
      if( a2[0].f<a1[0].f || a2[1].f>a1[1].f ) return 0;
    }
  }
  return 1;
}

static RtreeDValue cellOverlap(
  Rtree *pRtree,
  RtreeCell *p,
  RtreeCell *aCell,
  int nCell
){
  int ii;
  RtreeDValue overlap = RTREE_ZERO;
  for(ii=0; ii<nCell; ii++){
    int jj;
    RtreeDValue o = (RtreeDValue)1;
    for(jj=0; jj<pRtree->nDim2; jj+=2){
      RtreeDValue x1, x2;
      x1 = MAX(DCOORD(p->aCoord[jj]), DCOORD(aCell[ii].aCoord[jj]));
      x2 = MIN(DCOORD(p->aCoord[jj+1]), DCOORD(aCell[ii].aCoord[jj+1]));
      if( x2<x1 ){
        o = (RtreeDValue)0;
        break;
      }else{
        o = o * (x2-x1);
      }
    }
    overlap += o;
  }
  return overlap;
}


/*
** This function implements the ChooseLeaf algorithm from Gutman[84].
** ChooseSubTree in r*tree terminology.
*/
static int ChooseLeaf(
  Rtree *pRtree,               /* Rtree table */
  RtreeCell *pCell,            /* Cell to insert into rtree */
  int iHeight,                 /* Height of sub-tree rooted at pCell */
  RtreeNode **ppLeaf           /* OUT: Selected leaf page */
){
  int rc;
  int ii;
  RtreeNode *pNode = 0;
  rc = nodeAcquire(pRtree, 1, 0, &pNode);

  for(ii=0; rc==SQLITE_OK && ii<(pRtree->iDepth-iHeight); ii++){
    int iCell;
    sqlite3_int64 iBest = 0;
    int bFound = 0;
    RtreeDValue fMinGrowth = RTREE_ZERO;
    RtreeDValue fMinArea = RTREE_ZERO;
    int nCell = NCELL(pNode);
    RtreeNode *pChild = 0;

    /* First check to see if there is are any cells in pNode that completely
    ** contains pCell.  If two or more cells in pNode completely contain pCell
    ** then pick the smallest.
    */
    for(iCell=0; iCell<nCell; iCell++){
      RtreeCell cell;
      nodeGetCell(pRtree, pNode, iCell, &cell);
      if( cellContains(pRtree, &cell, pCell) ){
        RtreeDValue area = cellArea(pRtree, &cell);
        if( bFound==0 || area<fMinArea ){
          iBest = cell.iRowid;
          fMinArea = area;
          bFound = 1;
        }
      }
    }
    if( !bFound ){
      /* No cells of pNode will completely contain pCell.  So pick the
      ** cell of pNode that grows by the least amount when pCell is added.
      ** Break ties by selecting the smaller cell.
      */
      for(iCell=0; iCell<nCell; iCell++){
        RtreeCell cell;
        RtreeDValue growth;
        RtreeDValue area;
        nodeGetCell(pRtree, pNode, iCell, &cell);
        area = cellArea(pRtree, &cell);
        cellUnion(pRtree, &cell, pCell);
        growth = cellArea(pRtree, &cell)-area;
        if( iCell==0
         || growth<fMinGrowth
         || (growth==fMinGrowth && area<fMinArea)
        ){
          fMinGrowth = growth;
          fMinArea = area;
          iBest = cell.iRowid;
        }
      }
    }

    rc = nodeAcquire(pRtree, iBest, pNode, &pChild);
    nodeRelease(pRtree, pNode);
    pNode = pChild;
  }

  *ppLeaf = pNode;
  return rc;
}

/*
** A cell with the same content as pCell has just been inserted into
** the node pNode. This function updates the bounding box cells in
** all ancestor elements.
*/
static int AdjustTree(
  Rtree *pRtree,                    /* Rtree table */
  RtreeNode *pNode,                 /* Adjust ancestry of this node. */
  RtreeCell *pCell                  /* This cell was just inserted */
){
  RtreeNode *p = pNode;
  int cnt = 0;
  int rc;
  while( p->pParent ){
    RtreeNode *pParent = p->pParent;
    RtreeCell cell;
    int iCell;

    cnt++;
    if( cnt>100 ){
      RTREE_IS_CORRUPT(pRtree);
      return SQLITE_CORRUPT_VTAB;
    }
    rc = nodeParentIndex(pRtree, p, &iCell);
    if( NEVER(rc!=SQLITE_OK) ){
      RTREE_IS_CORRUPT(pRtree);
      return SQLITE_CORRUPT_VTAB;
    }

    nodeGetCell(pRtree, pParent, iCell, &cell);
    if( !cellContains(pRtree, &cell, pCell) ){
      cellUnion(pRtree, &cell, pCell);
      nodeOverwriteCell(pRtree, pParent, &cell, iCell);
    }

    p = pParent;
  }
  return SQLITE_OK;
}

/*
** Write mapping (iRowid->iNode) to the <rtree>_rowid table.
*/
static int rowidWrite(Rtree *pRtree, sqlite3_int64 iRowid, sqlite3_int64 iNode){
  sqlite3_bind_int64(pRtree->pWriteRowid, 1, iRowid);
  sqlite3_bind_int64(pRtree->pWriteRowid, 2, iNode);
  sqlite3_step(pRtree->pWriteRowid);
  return sqlite3_reset(pRtree->pWriteRowid);
}

/*
** Write mapping (iNode->iPar) to the <rtree>_parent table.
*/
static int parentWrite(Rtree *pRtree, sqlite3_int64 iNode, sqlite3_int64 iPar){
  sqlite3_bind_int64(pRtree->pWriteParent, 1, iNode);
  sqlite3_bind_int64(pRtree->pWriteParent, 2, iPar);
  sqlite3_step(pRtree->pWriteParent);
  return sqlite3_reset(pRtree->pWriteParent);
}

static int rtreeInsertCell(Rtree *, RtreeNode *, RtreeCell *, int);



/*
** Arguments aIdx, aCell and aSpare all point to arrays of size
** nIdx. The aIdx array contains the set of integers from 0 to
** (nIdx-1) in no particular order. This function sorts the values
** in aIdx according to dimension iDim of the cells in aCell. The
** minimum value of dimension iDim is considered first, the
** maximum used to break ties.
**
** The aSpare array is used as temporary working space by the
** sorting algorithm.
*/
static void SortByDimension(
  Rtree *pRtree,
  int *aIdx,
  int nIdx,
  int iDim,
  RtreeCell *aCell,
  int *aSpare
){
  if( nIdx>1 ){

    int iLeft = 0;
    int iRight = 0;

    int nLeft = nIdx/2;
    int nRight = nIdx-nLeft;
    int *aLeft = aIdx;
    int *aRight = &aIdx[nLeft];

    SortByDimension(pRtree, aLeft, nLeft, iDim, aCell, aSpare);
    SortByDimension(pRtree, aRight, nRight, iDim, aCell, aSpare);

    memcpy(aSpare, aLeft, sizeof(int)*nLeft);
    aLeft = aSpare;
    while( iLeft<nLeft || iRight<nRight ){
      RtreeDValue xleft1 = DCOORD(aCell[aLeft[iLeft]].aCoord[iDim*2]);
      RtreeDValue xleft2 = DCOORD(aCell[aLeft[iLeft]].aCoord[iDim*2+1]);
      RtreeDValue xright1 = DCOORD(aCell[aRight[iRight]].aCoord[iDim*2]);
      RtreeDValue xright2 = DCOORD(aCell[aRight[iRight]].aCoord[iDim*2+1]);
      if( (iLeft!=nLeft) && ((iRight==nRight)
       || (xleft1<xright1)
       || (xleft1==xright1 && xleft2<xright2)
      )){
        aIdx[iLeft+iRight] = aLeft[iLeft];
        iLeft++;
      }else{
        aIdx[iLeft+iRight] = aRight[iRight];
        iRight++;
      }
    }

#if 0
    /* Check that the sort worked */
    {
      int jj;
      for(jj=1; jj<nIdx; jj++){
        RtreeDValue xleft1 = aCell[aIdx[jj-1]].aCoord[iDim*2];
        RtreeDValue xleft2 = aCell[aIdx[jj-1]].aCoord[iDim*2+1];
        RtreeDValue xright1 = aCell[aIdx[jj]].aCoord[iDim*2];
        RtreeDValue xright2 = aCell[aIdx[jj]].aCoord[iDim*2+1];
        assert( xleft1<=xright1 && (xleft1<xright1 || xleft2<=xright2) );
      }
    }
#endif
  }
}

/*
** Implementation of the R*-tree variant of SplitNode from Beckman[1990].
*/
static int splitNodeStartree(
  Rtree *pRtree,
  RtreeCell *aCell,
  int nCell,
  RtreeNode *pLeft,
  RtreeNode *pRight,
  RtreeCell *pBboxLeft,
  RtreeCell *pBboxRight
){
  int **aaSorted;
  int *aSpare;
  int ii;

  int iBestDim = 0;
  int iBestSplit = 0;
  RtreeDValue fBestMargin = RTREE_ZERO;

  sqlite3_int64 nByte = (pRtree->nDim+1)*(sizeof(int*)+nCell*sizeof(int));

  aaSorted = (int **)sqlite3_malloc64(nByte);
  if( !aaSorted ){
    return SQLITE_NOMEM;
  }

  aSpare = &((int *)&aaSorted[pRtree->nDim])[pRtree->nDim*nCell];
  memset(aaSorted, 0, nByte);
  for(ii=0; ii<pRtree->nDim; ii++){
    int jj;
    aaSorted[ii] = &((int *)&aaSorted[pRtree->nDim])[ii*nCell];
    for(jj=0; jj<nCell; jj++){
      aaSorted[ii][jj] = jj;
    }
    SortByDimension(pRtree, aaSorted[ii], nCell, ii, aCell, aSpare);
  }

  for(ii=0; ii<pRtree->nDim; ii++){
    RtreeDValue margin = RTREE_ZERO;
    RtreeDValue fBestOverlap = RTREE_ZERO;
    RtreeDValue fBestArea = RTREE_ZERO;
    int iBestLeft = 0;
    int nLeft;

    for(
      nLeft=RTREE_MINCELLS(pRtree);
      nLeft<=(nCell-RTREE_MINCELLS(pRtree));
      nLeft++
    ){
      RtreeCell left;
      RtreeCell right;
      int kk;
      RtreeDValue overlap;
      RtreeDValue area;

      memcpy(&left, &aCell[aaSorted[ii][0]], sizeof(RtreeCell));
      memcpy(&right, &aCell[aaSorted[ii][nCell-1]], sizeof(RtreeCell));
      for(kk=1; kk<(nCell-1); kk++){
        if( kk<nLeft ){
          cellUnion(pRtree, &left, &aCell[aaSorted[ii][kk]]);
        }else{
          cellUnion(pRtree, &right, &aCell[aaSorted[ii][kk]]);
        }
      }
      margin += cellMargin(pRtree, &left);
      margin += cellMargin(pRtree, &right);
      overlap = cellOverlap(pRtree, &left, &right, 1);
      area = cellArea(pRtree, &left) + cellArea(pRtree, &right);
      if( (nLeft==RTREE_MINCELLS(pRtree))
       || (overlap<fBestOverlap)
       || (overlap==fBestOverlap && area<fBestArea)
      ){
        iBestLeft = nLeft;
        fBestOverlap = overlap;
        fBestArea = area;
      }
    }

    if( ii==0 || margin<fBestMargin ){
      iBestDim = ii;
      fBestMargin = margin;
      iBestSplit = iBestLeft;
    }
  }

  memcpy(pBboxLeft, &aCell[aaSorted[iBestDim][0]], sizeof(RtreeCell));
  memcpy(pBboxRight, &aCell[aaSorted[iBestDim][iBestSplit]], sizeof(RtreeCell));
  for(ii=0; ii<nCell; ii++){
    RtreeNode *pTarget = (ii<iBestSplit)?pLeft:pRight;
    RtreeCell *pBbox = (ii<iBestSplit)?pBboxLeft:pBboxRight;
    RtreeCell *pCell = &aCell[aaSorted[iBestDim][ii]];
    nodeInsertCell(pRtree, pTarget, pCell);
    cellUnion(pRtree, pBbox, pCell);
  }

  sqlite3_free(aaSorted);
  return SQLITE_OK;
}


static int updateMapping(
  Rtree *pRtree,
  i64 iRowid,
  RtreeNode *pNode,
  int iHeight
){
  int (*xSetMapping)(Rtree *, sqlite3_int64, sqlite3_int64);
  xSetMapping = ((iHeight==0)?rowidWrite:parentWrite);
  if( iHeight>0 ){
    RtreeNode *pChild = nodeHashLookup(pRtree, iRowid);
    RtreeNode *p;
    for(p=pNode; p; p=p->pParent){
      if( p==pChild ) return SQLITE_CORRUPT_VTAB;
    }
    if( pChild ){
      nodeRelease(pRtree, pChild->pParent);
      nodeReference(pNode);
      pChild->pParent = pNode;
    }
  }
  if( NEVER(pNode==0) ) return SQLITE_ERROR;
  return xSetMapping(pRtree, iRowid, pNode->iNode);
}

static int SplitNode(
  Rtree *pRtree,
  RtreeNode *pNode,
  RtreeCell *pCell,
  int iHeight
){
  int i;
  int newCellIsRight = 0;

  int rc = SQLITE_OK;
  int nCell = NCELL(pNode);
  RtreeCell *aCell;
  int *aiUsed;

  RtreeNode *pLeft = 0;
  RtreeNode *pRight = 0;

  RtreeCell leftbbox;
  RtreeCell rightbbox;

  /* Allocate an array and populate it with a copy of pCell and
  ** all cells from node pLeft. Then zero the original node.
  */
  aCell = sqlite3_malloc64((sizeof(RtreeCell)+sizeof(int))*(nCell+1));
  if( !aCell ){
    rc = SQLITE_NOMEM;
    goto splitnode_out;
  }
  aiUsed = (int *)&aCell[nCell+1];
  memset(aiUsed, 0, sizeof(int)*(nCell+1));
  for(i=0; i<nCell; i++){
    nodeGetCell(pRtree, pNode, i, &aCell[i]);
  }
  nodeZero(pRtree, pNode);
  memcpy(&aCell[nCell], pCell, sizeof(RtreeCell));
  nCell++;

  if( pNode->iNode==1 ){
    pRight = nodeNew(pRtree, pNode);
    pLeft = nodeNew(pRtree, pNode);
    pRtree->iDepth++;
    pNode->isDirty = 1;
    writeInt16(pNode->zData, pRtree->iDepth);
  }else{
    pLeft = pNode;
    pRight = nodeNew(pRtree, pLeft->pParent);
    pLeft->nRef++;
  }

  if( !pLeft || !pRight ){
    rc = SQLITE_NOMEM;
    goto splitnode_out;
  }

  memset(pLeft->zData, 0, pRtree->iNodeSize);
  memset(pRight->zData, 0, pRtree->iNodeSize);

  rc = splitNodeStartree(pRtree, aCell, nCell, pLeft, pRight,
                         &leftbbox, &rightbbox);
  if( rc!=SQLITE_OK ){
    goto splitnode_out;
  }

  /* Ensure both child nodes have node numbers assigned to them by calling
  ** nodeWrite(). Node pRight always needs a node number, as it was created
  ** by nodeNew() above. But node pLeft sometimes already has a node number.
  ** In this case avoid the all to nodeWrite().
  */
  if( SQLITE_OK!=(rc = nodeWrite(pRtree, pRight))
   || (0==pLeft->iNode && SQLITE_OK!=(rc = nodeWrite(pRtree, pLeft)))
  ){
    goto splitnode_out;
  }

  rightbbox.iRowid = pRight->iNode;
  leftbbox.iRowid = pLeft->iNode;

  if( pNode->iNode==1 ){
    rc = rtreeInsertCell(pRtree, pLeft->pParent, &leftbbox, iHeight+1);
    if( rc!=SQLITE_OK ){
      goto splitnode_out;
    }
  }else{
    RtreeNode *pParent = pLeft->pParent;
    int iCell;
    rc = nodeParentIndex(pRtree, pLeft, &iCell);
    if( ALWAYS(rc==SQLITE_OK) ){
      nodeOverwriteCell(pRtree, pParent, &leftbbox, iCell);
      rc = AdjustTree(pRtree, pParent, &leftbbox);
      assert( rc==SQLITE_OK );
    }
    if( NEVER(rc!=SQLITE_OK) ){
      goto splitnode_out;
    }
  }
  if( (rc = rtreeInsertCell(pRtree, pRight->pParent, &rightbbox, iHeight+1)) ){
    goto splitnode_out;
  }

  for(i=0; i<NCELL(pRight); i++){
    i64 iRowid = nodeGetRowid(pRtree, pRight, i);
    rc = updateMapping(pRtree, iRowid, pRight, iHeight);
    if( iRowid==pCell->iRowid ){
      newCellIsRight = 1;
    }
    if( rc!=SQLITE_OK ){
      goto splitnode_out;
    }
  }
  if( pNode->iNode==1 ){
    for(i=0; i<NCELL(pLeft); i++){
      i64 iRowid = nodeGetRowid(pRtree, pLeft, i);
      rc = updateMapping(pRtree, iRowid, pLeft, iHeight);
      if( rc!=SQLITE_OK ){
        goto splitnode_out;
      }
    }
  }else if( newCellIsRight==0 ){
    rc = updateMapping(pRtree, pCell->iRowid, pLeft, iHeight);
  }

splitnode_out:
  nodeRelease(pRtree, pRight);
  nodeRelease(pRtree, pLeft);
  sqlite3_free(aCell);
  return rc;
}

/*
** If node pLeaf is not the root of the r-tree and its pParent pointer is
** still NULL, load all ancestor nodes of pLeaf into memory and populate
** the pLeaf->pParent chain all the way up to the root node.
**
** This operation is required when a row is deleted (or updated - an update
** is implemented as a delete followed by an insert). SQLite provides the
** rowid of the row to delete, which can be used to find the leaf on which
** the entry resides (argument pLeaf). Once the leaf is located, this
** function is called to determine its ancestry.
*/
static int fixLeafParent(Rtree *pRtree, RtreeNode *pLeaf){
  int rc = SQLITE_OK;
  RtreeNode *pChild = pLeaf;
  while( rc==SQLITE_OK && pChild->iNode!=1 && pChild->pParent==0 ){
    int rc2 = SQLITE_OK;          /* sqlite3_reset() return code */
    sqlite3_bind_int64(pRtree->pReadParent, 1, pChild->iNode);
    rc = sqlite3_step(pRtree->pReadParent);
    if( rc==SQLITE_ROW ){
      RtreeNode *pTest;           /* Used to test for reference loops */
      i64 iNode;                  /* Node number of parent node */

      /* Before setting pChild->pParent, test that we are not creating a
      ** loop of references (as we would if, say, pChild==pParent). We don't
      ** want to do this as it leads to a memory leak when trying to delete
      ** the referenced counted node structures.
      */
      iNode = sqlite3_column_int64(pRtree->pReadParent, 0);
      for(pTest=pLeaf; pTest && pTest->iNode!=iNode; pTest=pTest->pParent);
      if( pTest==0 ){
        rc2 = nodeAcquire(pRtree, iNode, 0, &pChild->pParent);
      }
    }
    rc = sqlite3_reset(pRtree->pReadParent);
    if( rc==SQLITE_OK ) rc = rc2;
    if( rc==SQLITE_OK && !pChild->pParent ){
      RTREE_IS_CORRUPT(pRtree);
      rc = SQLITE_CORRUPT_VTAB;
    }
    pChild = pChild->pParent;
  }
  return rc;
}

static int deleteCell(Rtree *, RtreeNode *, int, int);

static int removeNode(Rtree *pRtree, RtreeNode *pNode, int iHeight){
  int rc;
  int rc2;
  RtreeNode *pParent = 0;
  int iCell;

  assert( pNode->nRef==1 );

  /* Remove the entry in the parent cell. */
  rc = nodeParentIndex(pRtree, pNode, &iCell);
  if( rc==SQLITE_OK ){
    pParent = pNode->pParent;
    pNode->pParent = 0;
    rc = deleteCell(pRtree, pParent, iCell, iHeight+1);
    testcase( rc!=SQLITE_OK );
  }
  rc2 = nodeRelease(pRtree, pParent);
  if( rc==SQLITE_OK ){
    rc = rc2;
  }
  if( rc!=SQLITE_OK ){
    return rc;
  }

  /* Remove the xxx_node entry. */
  sqlite3_bind_int64(pRtree->pDeleteNode, 1, pNode->iNode);
  sqlite3_step(pRtree->pDeleteNode);
  if( SQLITE_OK!=(rc = sqlite3_reset(pRtree->pDeleteNode)) ){
    return rc;
  }

  /* Remove the xxx_parent entry. */
  sqlite3_bind_int64(pRtree->pDeleteParent, 1, pNode->iNode);
  sqlite3_step(pRtree->pDeleteParent);
  if( SQLITE_OK!=(rc = sqlite3_reset(pRtree->pDeleteParent)) ){
    return rc;
  }

  /* Remove the node from the in-memory hash table and link it into
  ** the Rtree.pDeleted list. Its contents will be re-inserted later on.
  */
  nodeHashDelete(pRtree, pNode);
  pNode->iNode = iHeight;
  pNode->pNext = pRtree->pDeleted;
  pNode->nRef++;
  pRtree->pDeleted = pNode;

  return SQLITE_OK;
}

static int fixBoundingBox(Rtree *pRtree, RtreeNode *pNode){
  RtreeNode *pParent = pNode->pParent;
  int rc = SQLITE_OK;
  if( pParent ){
    int ii;
    int nCell = NCELL(pNode);
    RtreeCell box;                            /* Bounding box for pNode */
    nodeGetCell(pRtree, pNode, 0, &box);
    for(ii=1; ii<nCell; ii++){
      RtreeCell cell;
      nodeGetCell(pRtree, pNode, ii, &cell);
      cellUnion(pRtree, &box, &cell);
    }
    box.iRowid = pNode->iNode;
    rc = nodeParentIndex(pRtree, pNode, &ii);
    if( rc==SQLITE_OK ){
      nodeOverwriteCell(pRtree, pParent, &box, ii);
      rc = fixBoundingBox(pRtree, pParent);
    }
  }
  return rc;
}

/*
** Delete the cell at index iCell of node pNode. After removing the
** cell, adjust the r-tree data structure if required.
*/
static int deleteCell(Rtree *pRtree, RtreeNode *pNode, int iCell, int iHeight){
  RtreeNode *pParent;
  int rc;

  if( SQLITE_OK!=(rc = fixLeafParent(pRtree, pNode)) ){
    return rc;
  }

  /* Remove the cell from the node. This call just moves bytes around
  ** the in-memory node image, so it cannot fail.
  */
  nodeDeleteCell(pRtree, pNode, iCell);

  /* If the node is not the tree root and now has less than the minimum
  ** number of cells, remove it from the tree. Otherwise, update the
  ** cell in the parent node so that it tightly contains the updated
  ** node.
  */
  pParent = pNode->pParent;
  assert( pParent || pNode->iNode==1 );
  if( pParent ){
    if( NCELL(pNode)<RTREE_MINCELLS(pRtree) ){
      rc = removeNode(pRtree, pNode, iHeight);
    }else{
      rc = fixBoundingBox(pRtree, pNode);
    }
  }

  return rc;
}

/*
** Insert cell pCell into node pNode. Node pNode is the head of a
** subtree iHeight high (leaf nodes have iHeight==0).
*/
static int rtreeInsertCell(
  Rtree *pRtree,
  RtreeNode *pNode,
  RtreeCell *pCell,
  int iHeight
){
  int rc = SQLITE_OK;
  if( iHeight>0 ){
    RtreeNode *pChild = nodeHashLookup(pRtree, pCell->iRowid);
    if( pChild ){
      nodeRelease(pRtree, pChild->pParent);
      nodeReference(pNode);
      pChild->pParent = pNode;
    }
  }
  if( nodeInsertCell(pRtree, pNode, pCell) ){
    rc = SplitNode(pRtree, pNode, pCell, iHeight);
  }else{
    rc = AdjustTree(pRtree, pNode, pCell);
    if( rc==SQLITE_OK ){
      if( iHeight==0 ){
        rc = rowidWrite(pRtree, pCell->iRowid, pNode->iNode);
      }else{
        rc = parentWrite(pRtree, pCell->iRowid, pNode->iNode);
      }
    }
  }
  return rc;
}

static int reinsertNodeContent(Rtree *pRtree, RtreeNode *pNode){
  int ii;
  int rc = SQLITE_OK;
  int nCell = NCELL(pNode);

  for(ii=0; rc==SQLITE_OK && ii<nCell; ii++){
    RtreeNode *pInsert;
    RtreeCell cell;
    nodeGetCell(pRtree, pNode, ii, &cell);

    /* Find a node to store this cell in. pNode->iNode currently contains
    ** the height of the sub-tree headed by the cell.
    */
    rc = ChooseLeaf(pRtree, &cell, (int)pNode->iNode, &pInsert);
    if( rc==SQLITE_OK ){
      int rc2;
      rc = rtreeInsertCell(pRtree, pInsert, &cell, (int)pNode->iNode);
      rc2 = nodeRelease(pRtree, pInsert);
      if( rc==SQLITE_OK ){
        rc = rc2;
      }
    }
  }
  return rc;
}

/*
** Select a currently unused rowid for a new r-tree record.
*/
static int rtreeNewRowid(Rtree *pRtree, i64 *piRowid){
  int rc;
  sqlite3_bind_null(pRtree->pWriteRowid, 1);
  sqlite3_bind_null(pRtree->pWriteRowid, 2);
  sqlite3_step(pRtree->pWriteRowid);
  rc = sqlite3_reset(pRtree->pWriteRowid);
  *piRowid = sqlite3_last_insert_rowid(pRtree->db);
  return rc;
}

/*
** Remove the entry with rowid=iDelete from the r-tree structure.
*/
static int rtreeDeleteRowid(Rtree *pRtree, sqlite3_int64 iDelete){
  int rc;                         /* Return code */
  RtreeNode *pLeaf = 0;           /* Leaf node containing record iDelete */
  int iCell;                      /* Index of iDelete cell in pLeaf */
  RtreeNode *pRoot = 0;           /* Root node of rtree structure */


  /* Obtain a reference to the root node to initialize Rtree.iDepth */
  rc = nodeAcquire(pRtree, 1, 0, &pRoot);

  /* Obtain a reference to the leaf node that contains the entry
  ** about to be deleted.
  */
  if( rc==SQLITE_OK ){
    rc = findLeafNode(pRtree, iDelete, &pLeaf, 0);
  }

#ifdef CORRUPT_DB
  assert( pLeaf!=0 || rc!=SQLITE_OK || CORRUPT_DB );
#endif

  /* Delete the cell in question from the leaf node. */
  if( rc==SQLITE_OK && pLeaf ){
    int rc2;
    rc = nodeRowidIndex(pRtree, pLeaf, iDelete, &iCell);
    if( rc==SQLITE_OK ){
      rc = deleteCell(pRtree, pLeaf, iCell, 0);
    }
    rc2 = nodeRelease(pRtree, pLeaf);
    if( rc==SQLITE_OK ){
      rc = rc2;
    }
  }

  /* Delete the corresponding entry in the <rtree>_rowid table. */
  if( rc==SQLITE_OK ){
    sqlite3_bind_int64(pRtree->pDeleteRowid, 1, iDelete);
    sqlite3_step(pRtree->pDeleteRowid);
    rc = sqlite3_reset(pRtree->pDeleteRowid);
  }

  /* Check if the root node now has exactly one child. If so, remove
  ** it, schedule the contents of the child for reinsertion and
  ** reduce the tree height by one.
  **
  ** This is equivalent to copying the contents of the child into
  ** the root node (the operation that Gutman's paper says to perform
  ** in this scenario).
  */
  if( rc==SQLITE_OK && pRtree->iDepth>0 && NCELL(pRoot)==1 ){
    int rc2;
    RtreeNode *pChild = 0;
    i64 iChild = nodeGetRowid(pRtree, pRoot, 0);
    rc = nodeAcquire(pRtree, iChild, pRoot, &pChild);  /* tag-20210916a */
    if( rc==SQLITE_OK ){
      rc = removeNode(pRtree, pChild, pRtree->iDepth-1);
    }
    rc2 = nodeRelease(pRtree, pChild);
    if( rc==SQLITE_OK ) rc = rc2;
    if( rc==SQLITE_OK ){
      pRtree->iDepth--;
      writeInt16(pRoot->zData, pRtree->iDepth);
      pRoot->isDirty = 1;
    }
  }

  /* Re-insert the contents of any underfull nodes removed from the tree. */
  for(pLeaf=pRtree->pDeleted; pLeaf; pLeaf=pRtree->pDeleted){
    if( rc==SQLITE_OK ){
      rc = reinsertNodeContent(pRtree, pLeaf);
    }
    pRtree->pDeleted = pLeaf->pNext;
    pRtree->nNodeRef--;
    sqlite3_free(pLeaf);
  }

  /* Release the reference to the root node. */
  if( rc==SQLITE_OK ){
    rc = nodeRelease(pRtree, pRoot);
  }else{
    nodeRelease(pRtree, pRoot);
  }

  return rc;
}

/*
** Rounding constants for float->double conversion.
*/
#define RNDTOWARDS  (1.0 - 1.0/8388608.0)  /* Round towards zero */
#define RNDAWAY     (1.0 + 1.0/8388608.0)  /* Round away from zero */

#if !defined(SQLITE_RTREE_INT_ONLY)
/*
** Convert an sqlite3_value into an RtreeValue (presumably a float)
** while taking care to round toward negative or positive, respectively.
*/
static RtreeValue rtreeValueDown(sqlite3_value *v){
  double d = sqlite3_value_double(v);
  float f = (float)d;
  if( f>d ){
    f = (float)(d*(d<0 ? RNDAWAY : RNDTOWARDS));
  }
  return f;
}
static RtreeValue rtreeValueUp(sqlite3_value *v){
  double d = sqlite3_value_double(v);
  float f = (float)d;
  if( f<d ){
    f = (float)(d*(d<0 ? RNDTOWARDS : RNDAWAY));
  }
  return f;
}
#endif /* !defined(SQLITE_RTREE_INT_ONLY) */

/*
** A constraint has failed while inserting a row into an rtree table.
** Assuming no OOM error occurs, this function sets the error message
** (at pRtree->base.zErrMsg) to an appropriate value and returns
** SQLITE_CONSTRAINT.
**
** Parameter iCol is the index of the leftmost column involved in the
** constraint failure. If it is 0, then the constraint that failed is
** the unique constraint on the id column. Otherwise, it is the rtree
** (c1<=c2) constraint on columns iCol and iCol+1 that has failed.
**
** If an OOM occurs, SQLITE_NOMEM is returned instead of SQLITE_CONSTRAINT.
*/
static int rtreeConstraintError(Rtree *pRtree, int iCol){
  sqlite3_stmt *pStmt = 0;
  char *zSql;
  int rc;

  assert( iCol==0 || iCol%2 );
  zSql = sqlite3_mprintf("SELECT * FROM %Q.%Q", pRtree->zDb, pRtree->zName);
  if( zSql ){
    rc = sqlite3_prepare_v2(pRtree->db, zSql, -1, &pStmt, 0);
  }else{
    rc = SQLITE_NOMEM;
  }
  sqlite3_free(zSql);

  if( rc==SQLITE_OK ){
    if( iCol==0 ){
      const char *zCol = sqlite3_column_name(pStmt, 0);
      pRtree->base.zErrMsg = sqlite3_mprintf(
          "UNIQUE constraint failed: %s.%s", pRtree->zName, zCol
      );
    }else{
      const char *zCol1 = sqlite3_column_name(pStmt, iCol);
      const char *zCol2 = sqlite3_column_name(pStmt, iCol+1);
      pRtree->base.zErrMsg = sqlite3_mprintf(
          "rtree constraint failed: %s.(%s<=%s)", pRtree->zName, zCol1, zCol2
      );
    }
  }

  sqlite3_finalize(pStmt);
  return (rc==SQLITE_OK ? SQLITE_CONSTRAINT : rc);
}



/*
** The xUpdate method for rtree module virtual tables.
*/
static int rtreeUpdate(
  sqlite3_vtab *pVtab,
  int nData,
  sqlite3_value **aData,
  sqlite_int64 *pRowid
){
  Rtree *pRtree = (Rtree *)pVtab;
  int rc = SQLITE_OK;
  RtreeCell cell;                 /* New cell to insert if nData>1 */
  int bHaveRowid = 0;             /* Set to 1 after new rowid is determined */

  if( pRtree->nNodeRef ){
    /* Unable to write to the btree while another cursor is reading from it,
    ** since the write might do a rebalance which would disrupt the read
    ** cursor. */
    return SQLITE_LOCKED_VTAB;
  }
  rtreeReference(pRtree);
  assert(nData>=1);

  memset(&cell, 0, sizeof(cell));

  /* Constraint handling. A write operation on an r-tree table may return
  ** SQLITE_CONSTRAINT for two reasons:
  **
  **   1. A duplicate rowid value, or
  **   2. The supplied data violates the "x2>=x1" constraint.
  **
  ** In the first case, if the conflict-handling mode is REPLACE, then
  ** the conflicting row can be removed before proceeding. In the second
  ** case, SQLITE_CONSTRAINT must be returned regardless of the
  ** conflict-handling mode specified by the user.
  */
  if( nData>1 ){
    int ii;
    int nn = nData - 4;

    if( nn > pRtree->nDim2 ) nn = pRtree->nDim2;
    /* Populate the cell.aCoord[] array. The first coordinate is aData[3].
    **
    ** NB: nData can only be less than nDim*2+3 if the rtree is mis-declared
    ** with "column" that are interpreted as table constraints.
    ** Example:  CREATE VIRTUAL TABLE bad USING rtree(x,y,CHECK(y>5));
    ** This problem was discovered after years of use, so we silently ignore
    ** these kinds of misdeclared tables to avoid breaking any legacy.
    */

#ifndef SQLITE_RTREE_INT_ONLY
    if( pRtree->eCoordType==RTREE_COORD_REAL32 ){
      for(ii=0; ii<nn; ii+=2){
        cell.aCoord[ii].f = rtreeValueDown(aData[ii+3]);
        cell.aCoord[ii+1].f = rtreeValueUp(aData[ii+4]);
        if( cell.aCoord[ii].f>cell.aCoord[ii+1].f ){
          rc = rtreeConstraintError(pRtree, ii+1);
          goto constraint;
        }
      }
    }else
#endif
    {
      for(ii=0; ii<nn; ii+=2){
        cell.aCoord[ii].i = sqlite3_value_int(aData[ii+3]);
        cell.aCoord[ii+1].i = sqlite3_value_int(aData[ii+4]);
        if( cell.aCoord[ii].i>cell.aCoord[ii+1].i ){
          rc = rtreeConstraintError(pRtree, ii+1);
          goto constraint;
        }
      }
    }

    /* If a rowid value was supplied, check if it is already present in
    ** the table. If so, the constraint has failed. */
    if( sqlite3_value_type(aData[2])!=SQLITE_NULL ){
      cell.iRowid = sqlite3_value_int64(aData[2]);
      if( sqlite3_value_type(aData[0])==SQLITE_NULL
       || sqlite3_value_int64(aData[0])!=cell.iRowid
      ){
        int steprc;
        sqlite3_bind_int64(pRtree->pReadRowid, 1, cell.iRowid);
        steprc = sqlite3_step(pRtree->pReadRowid);
        rc = sqlite3_reset(pRtree->pReadRowid);
        if( SQLITE_ROW==steprc ){
          if( sqlite3_vtab_on_conflict(pRtree->db)==SQLITE_REPLACE ){
            rc = rtreeDeleteRowid(pRtree, cell.iRowid);
          }else{
            rc = rtreeConstraintError(pRtree, 0);
            goto constraint;
          }
        }
      }
      bHaveRowid = 1;
    }
  }

  /* If aData[0] is not an SQL NULL value, it is the rowid of a
  ** record to delete from the r-tree table. The following block does
  ** just that.
  */
  if( sqlite3_value_type(aData[0])!=SQLITE_NULL ){
    rc = rtreeDeleteRowid(pRtree, sqlite3_value_int64(aData[0]));
  }

  /* If the aData[] array contains more than one element, elements
  ** (aData[2]..aData[argc-1]) contain a new record to insert into
  ** the r-tree structure.
  */
  if( rc==SQLITE_OK && nData>1 ){
    /* Insert the new record into the r-tree */
    RtreeNode *pLeaf = 0;

    /* Figure out the rowid of the new row. */
    if( bHaveRowid==0 ){
      rc = rtreeNewRowid(pRtree, &cell.iRowid);
    }
    *pRowid = cell.iRowid;

    if( rc==SQLITE_OK ){
      rc = ChooseLeaf(pRtree, &cell, 0, &pLeaf);
    }
    if( rc==SQLITE_OK ){
      int rc2;
      rc = rtreeInsertCell(pRtree, pLeaf, &cell, 0);
      rc2 = nodeRelease(pRtree, pLeaf);
      if( rc==SQLITE_OK ){
        rc = rc2;
      }
    }
    if( rc==SQLITE_OK && pRtree->nAux ){
      sqlite3_stmt *pUp = pRtree->pWriteAux;
      int jj;
      sqlite3_bind_int64(pUp, 1, *pRowid);
      for(jj=0; jj<pRtree->nAux; jj++){
        sqlite3_bind_value(pUp, jj+2, aData[pRtree->nDim2+3+jj]);
      }
      sqlite3_step(pUp);
      rc = sqlite3_reset(pUp);
    }
  }

constraint:
  rtreeRelease(pRtree);
  return rc;
}

/*
** Called when a transaction starts.
*/
static int rtreeBeginTransaction(sqlite3_vtab *pVtab){
  Rtree *pRtree = (Rtree *)pVtab;
  assert( pRtree->inWrTrans==0 );
  pRtree->inWrTrans = 1;
  return SQLITE_OK;
}

/*
** Called when a transaction completes (either by COMMIT or ROLLBACK).
** The sqlite3_blob object should be released at this point.
*/
static int rtreeEndTransaction(sqlite3_vtab *pVtab){
  Rtree *pRtree = (Rtree *)pVtab;
  pRtree->inWrTrans = 0;
  nodeBlobReset(pRtree);
  return SQLITE_OK;
}
static int rtreeRollback(sqlite3_vtab *pVtab){
  return rtreeEndTransaction(pVtab);
}

/*
** The xRename method for rtree module virtual tables.
*/
static int rtreeRename(sqlite3_vtab *pVtab, const char *zNewName){
  Rtree *pRtree = (Rtree *)pVtab;
  int rc = SQLITE_NOMEM;
  char *zSql = sqlite3_mprintf(
    "ALTER TABLE %Q.'%q_node'   RENAME TO \"%w_node\";"
    "ALTER TABLE %Q.'%q_parent' RENAME TO \"%w_parent\";"
    "ALTER TABLE %Q.'%q_rowid'  RENAME TO \"%w_rowid\";"
    , pRtree->zDb, pRtree->zName, zNewName
    , pRtree->zDb, pRtree->zName, zNewName
    , pRtree->zDb, pRtree->zName, zNewName
  );
  if( zSql ){
    nodeBlobReset(pRtree);
    rc = sqlite3_exec(pRtree->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }
  return rc;
}

/*
** The xSavepoint method.
**
** This module does not need to do anything to support savepoints. However,
** it uses this hook to close any open blob handle. This is done because a
** DROP TABLE command - which fortunately always opens a savepoint - cannot
** succeed if there are any open blob handles. i.e. if the blob handle were
** not closed here, the following would fail:
**
**   BEGIN;
**     INSERT INTO rtree...
**     DROP TABLE <tablename>;    -- Would fail with SQLITE_LOCKED
**   COMMIT;
*/
static int rtreeSavepoint(sqlite3_vtab *pVtab, int iSavepoint){
  Rtree *pRtree = (Rtree *)pVtab;
  u8 iwt = pRtree->inWrTrans;
  UNUSED_PARAMETER(iSavepoint);
  pRtree->inWrTrans = 0;
  nodeBlobReset(pRtree);
  pRtree->inWrTrans = iwt;
  return SQLITE_OK;
}

/*
** This function populates the pRtree->nRowEst variable with an estimate
** of the number of rows in the virtual table. If possible, this is based
** on sqlite_stat1 data. Otherwise, use RTREE_DEFAULT_ROWEST.
*/
static int rtreeQueryStat1(sqlite3 *db, Rtree *pRtree){
  const char *zFmt = "SELECT stat FROM %Q.sqlite_stat1 WHERE tbl = '%q_rowid'";
  char *zSql;
  sqlite3_stmt *p;
  int rc;
  i64 nRow = RTREE_MIN_ROWEST;

  rc = sqlite3_table_column_metadata(
      db, pRtree->zDb, "sqlite_stat1",0,0,0,0,0,0
  );
  if( rc!=SQLITE_OK ){
    pRtree->nRowEst = RTREE_DEFAULT_ROWEST;
    return rc==SQLITE_ERROR ? SQLITE_OK : rc;
  }
  zSql = sqlite3_mprintf(zFmt, pRtree->zDb, pRtree->zName);
  if( zSql==0 ){
    rc = SQLITE_NOMEM;
  }else{
    rc = sqlite3_prepare_v2(db, zSql, -1, &p, 0);
    if( rc==SQLITE_OK ){
      if( sqlite3_step(p)==SQLITE_ROW ) nRow = sqlite3_column_int64(p, 0);
      rc = sqlite3_finalize(p);
    }
    sqlite3_free(zSql);
  }
  pRtree->nRowEst = MAX(nRow, RTREE_MIN_ROWEST);
  return rc;
}


/*
** Return true if zName is the extension on one of the shadow tables used
** by this module.
*/
static int rtreeShadowName(const char *zName){
  static const char *azName[] = {
    "node", "parent", "rowid"
  };
  unsigned int i;
  for(i=0; i<sizeof(azName)/sizeof(azName[0]); i++){
    if( sqlite3_stricmp(zName, azName[i])==0 ) return 1;
  }
  return 0;
}

/* Forward declaration */
static int rtreeIntegrity(sqlite3_vtab*, const char*, const char*, int, char**);

static sqlite3_module rtreeModule = {
  4,                          /* iVersion */
  rtreeCreate,                /* xCreate - create a table */
  rtreeConnect,               /* xConnect - connect to an existing table */
  rtreeBestIndex,             /* xBestIndex - Determine search strategy */
  rtreeDisconnect,            /* xDisconnect - Disconnect from a table */
  rtreeDestroy,               /* xDestroy - Drop a table */
  rtreeOpen,                  /* xOpen - open a cursor */
  rtreeClose,                 /* xClose - close a cursor */
  rtreeFilter,                /* xFilter - configure scan constraints */
  rtreeNext,                  /* xNext - advance a cursor */
  rtreeEof,                   /* xEof */
  rtreeColumn,                /* xColumn - read data */
  rtreeRowid,                 /* xRowid - read data */
  rtreeUpdate,                /* xUpdate - write data */
  rtreeBeginTransaction,      /* xBegin - begin transaction */
  rtreeEndTransaction,        /* xSync - sync transaction */
  rtreeEndTransaction,        /* xCommit - commit transaction */
  rtreeRollback,              /* xRollback - rollback transaction */
  0,                          /* xFindFunction - function overloading */
  rtreeRename,                /* xRename - rename the table */
  rtreeSavepoint,             /* xSavepoint */
  0,                          /* xRelease */
  0,                          /* xRollbackTo */
  rtreeShadowName,            /* xShadowName */
  rtreeIntegrity              /* xIntegrity */
};

static int rtreeSqlInit(
  Rtree *pRtree,
  sqlite3 *db,
  const char *zDb,
  const char *zPrefix,
  int isCreate
){
  int rc = SQLITE_OK;

  #define N_STATEMENT 8
  static const char *azSql[N_STATEMENT] = {
    /* Write the xxx_node table */
    "INSERT OR REPLACE INTO '%q'.'%q_node' VALUES(?1, ?2)",
    "DELETE FROM '%q'.'%q_node' WHERE nodeno = ?1",

    /* Read and write the xxx_rowid table */
    "SELECT nodeno FROM '%q'.'%q_rowid' WHERE rowid = ?1",
    "INSERT OR REPLACE INTO '%q'.'%q_rowid' VALUES(?1, ?2)",
    "DELETE FROM '%q'.'%q_rowid' WHERE rowid = ?1",

    /* Read and write the xxx_parent table */
    "SELECT parentnode FROM '%q'.'%q_parent' WHERE nodeno = ?1",
    "INSERT OR REPLACE INTO '%q'.'%q_parent' VALUES(?1, ?2)",
    "DELETE FROM '%q'.'%q_parent' WHERE nodeno = ?1"
  };
  sqlite3_stmt **appStmt[N_STATEMENT];
  int i;
  const int f = SQLITE_PREPARE_PERSISTENT|SQLITE_PREPARE_NO_VTAB;

  pRtree->db = db;

  if( isCreate ){
    char *zCreate;
    sqlite3_str *p = sqlite3_str_new(db);
    int ii;
    sqlite3_str_appendf(p,
       "CREATE TABLE \"%w\".\"%w_rowid\"(rowid INTEGER PRIMARY KEY,nodeno",
       zDb, zPrefix);
    for(ii=0; ii<pRtree->nAux; ii++){
      sqlite3_str_appendf(p,",a%d",ii);
    }
    sqlite3_str_appendf(p,
      ");CREATE TABLE \"%w\".\"%w_node\"(nodeno INTEGER PRIMARY KEY,data);",
      zDb, zPrefix);
    sqlite3_str_appendf(p,
    "CREATE TABLE \"%w\".\"%w_parent\"(nodeno INTEGER PRIMARY KEY,parentnode);",
      zDb, zPrefix);
    sqlite3_str_appendf(p,
       "INSERT INTO \"%w\".\"%w_node\"VALUES(1,zeroblob(%d))",
       zDb, zPrefix, pRtree->iNodeSize);
    zCreate = sqlite3_str_finish(p);
    if( !zCreate ){
      return SQLITE_NOMEM;
    }
    rc = sqlite3_exec(db, zCreate, 0, 0, 0);
    sqlite3_free(zCreate);
    if( rc!=SQLITE_OK ){
      return rc;
    }
  }

  appStmt[0] = &pRtree->pWriteNode;
  appStmt[1] = &pRtree->pDeleteNode;
  appStmt[2] = &pRtree->pReadRowid;
  appStmt[3] = &pRtree->pWriteRowid;
  appStmt[4] = &pRtree->pDeleteRowid;
  appStmt[5] = &pRtree->pReadParent;
  appStmt[6] = &pRtree->pWriteParent;
  appStmt[7] = &pRtree->pDeleteParent;

  rc = rtreeQueryStat1(db, pRtree);
  for(i=0; i<N_STATEMENT && rc==SQLITE_OK; i++){
    char *zSql;
    const char *zFormat;
    if( i!=3 || pRtree->nAux==0 ){
       zFormat = azSql[i];
    }else {
       /* An UPSERT is very slightly slower than REPLACE, but it is needed
       ** if there are auxiliary columns */
       zFormat = "INSERT INTO\"%w\".\"%w_rowid\"(rowid,nodeno)VALUES(?1,?2)"
                  "ON CONFLICT(rowid)DO UPDATE SET nodeno=excluded.nodeno";
    }
    zSql = sqlite3_mprintf(zFormat, zDb, zPrefix);
    if( zSql ){
      rc = sqlite3_prepare_v3(db, zSql, -1, f, appStmt[i], 0);
    }else{
      rc = SQLITE_NOMEM;
    }
    sqlite3_free(zSql);
  }
  if( pRtree->nAux && rc!=SQLITE_NOMEM ){
    pRtree->zReadAuxSql = sqlite3_mprintf(
       "SELECT * FROM \"%w\".\"%w_rowid\" WHERE rowid=?1",
       zDb, zPrefix);
    if( pRtree->zReadAuxSql==0 ){
      rc = SQLITE_NOMEM;
    }else{
      sqlite3_str *p = sqlite3_str_new(db);
      int ii;
      char *zSql;
      sqlite3_str_appendf(p, "UPDATE \"%w\".\"%w_rowid\"SET ", zDb, zPrefix);
      for(ii=0; ii<pRtree->nAux; ii++){
        if( ii ) sqlite3_str_append(p, ",", 1);
#ifdef SQLITE_ENABLE_GEOPOLY
        if( ii<pRtree->nAuxNotNull ){
          sqlite3_str_appendf(p,"a%d=coalesce(?%d,a%d)",ii,ii+2,ii);
        }else
#endif
        {
          sqlite3_str_appendf(p,"a%d=?%d",ii,ii+2);
        }
      }
      sqlite3_str_appendf(p, " WHERE rowid=?1");
      zSql = sqlite3_str_finish(p);
      if( zSql==0 ){
        rc = SQLITE_NOMEM;
      }else{
        rc = sqlite3_prepare_v3(db, zSql, -1, f, &pRtree->pWriteAux, 0);
        sqlite3_free(zSql);
      }
    }
  }

  return rc;
}

/*
** The second argument to this function contains the text of an SQL statement
** that returns a single integer value. The statement is compiled and executed
** using database connection db. If successful, the integer value returned
** is written to *piVal and SQLITE_OK returned. Otherwise, an SQLite error
** code is returned and the value of *piVal after returning is not defined.
*/
static int getIntFromStmt(sqlite3 *db, const char *zSql, int *piVal){
  int rc = SQLITE_NOMEM;
  if( zSql ){
    sqlite3_stmt *pStmt = 0;
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    if( rc==SQLITE_OK ){
      if( SQLITE_ROW==sqlite3_step(pStmt) ){
        *piVal = sqlite3_column_int(pStmt, 0);
      }
      rc = sqlite3_finalize(pStmt);
    }
  }
  return rc;
}

/*
** This function is called from within the xConnect() or xCreate() method to
** determine the node-size used by the rtree table being created or connected
** to. If successful, pRtree->iNodeSize is populated and SQLITE_OK returned.
** Otherwise, an SQLite error code is returned.
**
** If this function is being called as part of an xConnect(), then the rtree
** table already exists. In this case the node-size is determined by inspecting
** the root node of the tree.
**
** Otherwise, for an xCreate(), use 64 bytes less than the database page-size.
** This ensures that each node is stored on a single database page. If the
** database page-size is so large that more than RTREE_MAXCELLS entries
** would fit in a single node, use a smaller node-size.
*/
static int getNodeSize(
  sqlite3 *db,                    /* Database handle */
  Rtree *pRtree,                  /* Rtree handle */
  int isCreate,                   /* True for xCreate, false for xConnect */
  char **pzErr                    /* OUT: Error message, if any */
){
  int rc;
  char *zSql;
  if( isCreate ){
    int iPageSize = 0;
    zSql = sqlite3_mprintf("PRAGMA %Q.page_size", pRtree->zDb);
    rc = getIntFromStmt(db, zSql, &iPageSize);
    if( rc==SQLITE_OK ){
      pRtree->iNodeSize = iPageSize-64;
      if( (4+pRtree->nBytesPerCell*RTREE_MAXCELLS)<pRtree->iNodeSize ){
        pRtree->iNodeSize = 4+pRtree->nBytesPerCell*RTREE_MAXCELLS;
      }
    }else{
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }
  }else{
    zSql = sqlite3_mprintf(
        "SELECT length(data) FROM '%q'.'%q_node' WHERE nodeno = 1",
        pRtree->zDb, pRtree->zName
    );
    rc = getIntFromStmt(db, zSql, &pRtree->iNodeSize);
    if( rc!=SQLITE_OK ){
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }else if( pRtree->iNodeSize<(512-64) ){
      rc = SQLITE_CORRUPT_VTAB;
      RTREE_IS_CORRUPT(pRtree);
      *pzErr = sqlite3_mprintf("undersize RTree blobs in \"%q_node\"",
                               pRtree->zName);
    }
  }

  sqlite3_free(zSql);
  return rc;
}

/*
** Return the length of a token
*/
static int rtreeTokenLength(const char *z){
  int dummy = 0;
  return sqlite3GetToken((const unsigned char*)z,&dummy);
}

/*
** This function is the implementation of both the xConnect and xCreate
** methods of the r-tree virtual table.
**
**   argv[0]   -> module name
**   argv[1]   -> database name
**   argv[2]   -> table name
**   argv[...] -> column names...
*/
static int rtreeInit(
  sqlite3 *db,                        /* Database connection */
  void *pAux,                         /* One of the RTREE_COORD_* constants */
  int argc, const char *const*argv,   /* Parameters to CREATE TABLE statement */
  sqlite3_vtab **ppVtab,              /* OUT: New virtual table */
  char **pzErr,                       /* OUT: Error message, if any */
  int isCreate                        /* True for xCreate, false for xConnect */
){
  int rc = SQLITE_OK;
  Rtree *pRtree;
  int nDb;              /* Length of string argv[1] */
  int nName;            /* Length of string argv[2] */
  int eCoordType = (pAux ? RTREE_COORD_INT32 : RTREE_COORD_REAL32);
  sqlite3_str *pSql;
  char *zSql;
  int ii = 4;
  int iErr;

  const char *aErrMsg[] = {
    0,                                                    /* 0 */
    "Wrong number of columns for an rtree table",         /* 1 */
    "Too few columns for an rtree table",                 /* 2 */
    "Too many columns for an rtree table",                /* 3 */
    "Auxiliary rtree columns must be last"                /* 4 */
  };

  assert( RTREE_MAX_AUX_COLUMN<256 ); /* Aux columns counted by a u8 */
  if( argc<6 || argc>RTREE_MAX_AUX_COLUMN+3 ){
    *pzErr = sqlite3_mprintf("%s", aErrMsg[2 + (argc>=6)]);
    return SQLITE_ERROR;
  }

  sqlite3_vtab_config(db, SQLITE_VTAB_CONSTRAINT_SUPPORT, 1);
  sqlite3_vtab_config(db, SQLITE_VTAB_INNOCUOUS);


  /* Allocate the sqlite3_vtab structure */
  nDb = (int)strlen(argv[1]);
  nName = (int)strlen(argv[2]);
  pRtree = (Rtree *)sqlite3_malloc64(sizeof(Rtree)+nDb+nName*2+8);
  if( !pRtree ){
    return SQLITE_NOMEM;
  }
  memset(pRtree, 0, sizeof(Rtree)+nDb+nName*2+8);
  pRtree->nBusy = 1;
  pRtree->base.pModule = &rtreeModule;
  pRtree->zDb = (char *)&pRtree[1];
  pRtree->zName = &pRtree->zDb[nDb+1];
  pRtree->zNodeName = &pRtree->zName[nName+1];
  pRtree->eCoordType = (u8)eCoordType;
  memcpy(pRtree->zDb, argv[1], nDb);
  memcpy(pRtree->zName, argv[2], nName);
  memcpy(pRtree->zNodeName, argv[2], nName);
  memcpy(&pRtree->zNodeName[nName], "_node", 6);


  /* Create/Connect to the underlying relational database schema. If
  ** that is successful, call sqlite3_declare_vtab() to configure
  ** the r-tree table schema.
  */
  pSql = sqlite3_str_new(db);
  sqlite3_str_appendf(pSql, "CREATE TABLE x(%.*s INT",
                      rtreeTokenLength(argv[3]), argv[3]);
  for(ii=4; ii<argc; ii++){
    const char *zArg = argv[ii];
    if( zArg[0]=='+' ){
      pRtree->nAux++;
      sqlite3_str_appendf(pSql, ",%.*s", rtreeTokenLength(zArg+1), zArg+1);
    }else if( pRtree->nAux>0 ){
      break;
    }else{
      static const char *azFormat[] = {",%.*s REAL", ",%.*s INT"};
      pRtree->nDim2++;
      sqlite3_str_appendf(pSql, azFormat[eCoordType],
                          rtreeTokenLength(zArg), zArg);
    }
  }
  sqlite3_str_appendf(pSql, ");");
  zSql = sqlite3_str_finish(pSql);
  if( !zSql ){
    rc = SQLITE_NOMEM;
  }else if( ii<argc ){
    *pzErr = sqlite3_mprintf("%s", aErrMsg[4]);
    rc = SQLITE_ERROR;
  }else if( SQLITE_OK!=(rc = sqlite3_declare_vtab(db, zSql)) ){
    *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
  }
  sqlite3_free(zSql);
  if( rc ) goto rtreeInit_fail;
  pRtree->nDim = pRtree->nDim2/2;
  if( pRtree->nDim<1 ){
    iErr = 2;
  }else if( pRtree->nDim2>RTREE_MAX_DIMENSIONS*2 ){
    iErr = 3;
  }else if( pRtree->nDim2 % 2 ){
    iErr = 1;
  }else{
    iErr = 0;
  }
  if( iErr ){
    *pzErr = sqlite3_mprintf("%s", aErrMsg[iErr]);
    goto rtreeInit_fail;
  }
  pRtree->nBytesPerCell = 8 + pRtree->nDim2*4;

  /* Figure out the node size to use. */
  rc = getNodeSize(db, pRtree, isCreate, pzErr);
  if( rc ) goto rtreeInit_fail;
  rc = rtreeSqlInit(pRtree, db, argv[1], argv[2], isCreate);
  if( rc ){
    *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    goto rtreeInit_fail;
  }

  *ppVtab = (sqlite3_vtab *)pRtree;
  return SQLITE_OK;

rtreeInit_fail:
  if( rc==SQLITE_OK ) rc = SQLITE_ERROR;
  assert( *ppVtab==0 );
  assert( pRtree->nBusy==1 );
  rtreeRelease(pRtree);
  return rc;
}


/*
** Implementation of a scalar function that decodes r-tree nodes to
** human readable strings. This can be used for debugging and analysis.
**
** The scalar function takes two arguments: (1) the number of dimensions
** to the rtree (between 1 and 5, inclusive) and (2) a blob of data containing
** an r-tree node.  For a two-dimensional r-tree structure called "rt", to
** deserialize all nodes, a statement like:
**
**   SELECT rtreenode(2, data) FROM rt_node;
**
** The human readable string takes the form of a Tcl list with one
** entry for each cell in the r-tree node. Each entry is itself a
** list, containing the 8-byte rowid/pageno followed by the
** <num-dimension>*2 coordinates.
*/
static void rtreenode(sqlite3_context *ctx, int nArg, sqlite3_value **apArg){
  RtreeNode node;
  Rtree tree;
  int ii;
  int nData;
  int errCode;
  sqlite3_str *pOut;

  UNUSED_PARAMETER(nArg);
  memset(&node, 0, sizeof(RtreeNode));
  memset(&tree, 0, sizeof(Rtree));
  tree.nDim = (u8)sqlite3_value_int(apArg[0]);
  if( tree.nDim<1 || tree.nDim>5 ) return;
  tree.nDim2 = tree.nDim*2;
  tree.nBytesPerCell = 8 + 8 * tree.nDim;
  node.zData = (u8 *)sqlite3_value_blob(apArg[1]);
  if( node.zData==0 ) return;
  nData = sqlite3_value_bytes(apArg[1]);
  if( nData<4 ) return;
  if( nData<4+NCELL(&node)*tree.nBytesPerCell ) return;

  pOut = sqlite3_str_new(0);
  for(ii=0; ii<NCELL(&node); ii++){
    RtreeCell cell;
    int jj;

    nodeGetCell(&tree, &node, ii, &cell);
    if( ii>0 ) sqlite3_str_append(pOut, " ", 1);
    sqlite3_str_appendf(pOut, "{%lld", cell.iRowid);
    for(jj=0; jj<tree.nDim2; jj++){
#ifndef SQLITE_RTREE_INT_ONLY
      sqlite3_str_appendf(pOut, " %g", (double)cell.aCoord[jj].f);
#else
      sqlite3_str_appendf(pOut, " %d", cell.aCoord[jj].i);
#endif
    }
    sqlite3_str_append(pOut, "}", 1);
  }
  errCode = sqlite3_str_errcode(pOut);
  sqlite3_result_error_code(ctx, errCode);
  sqlite3_result_text(ctx, sqlite3_str_finish(pOut), -1, sqlite3_free);
}

/* This routine implements an SQL function that returns the "depth" parameter
** from the front of a blob that is an r-tree node.  For example:
**
**     SELECT rtreedepth(data) FROM rt_node WHERE nodeno=1;
**
** The depth value is 0 for all nodes other than the root node, and the root
** node always has nodeno=1, so the example above is the primary use for this
** routine.  This routine is intended for testing and analysis only.
*/
static void rtreedepth(sqlite3_context *ctx, int nArg, sqlite3_value **apArg){
  UNUSED_PARAMETER(nArg);
  if( sqlite3_value_type(apArg[0])!=SQLITE_BLOB
   || sqlite3_value_bytes(apArg[0])<2

  ){
    sqlite3_result_error(ctx, "Invalid argument to rtreedepth()", -1);
  }else{
    u8 *zBlob = (u8 *)sqlite3_value_blob(apArg[0]);
    if( zBlob ){
      sqlite3_result_int(ctx, readInt16(zBlob));
    }else{
      sqlite3_result_error_nomem(ctx);
    }
  }
}

/*
** Context object passed between the various routines that make up the
** implementation of integrity-check function rtreecheck().
*/
typedef struct RtreeCheck RtreeCheck;
struct RtreeCheck {
  sqlite3 *db;                    /* Database handle */
  const char *zDb;                /* Database containing rtree table */
  const char *zTab;               /* Name of rtree table */
  int bInt;                       /* True for rtree_i32 table */
  int nDim;                       /* Number of dimensions for this rtree tbl */
  sqlite3_stmt *pGetNode;         /* Statement used to retrieve nodes */
  sqlite3_stmt *aCheckMapping[2]; /* Statements to query %_parent/%_rowid */
  int nLeaf;                      /* Number of leaf cells in table */
  int nNonLeaf;                   /* Number of non-leaf cells in table */
  int rc;                         /* Return code */
  char *zReport;                  /* Message to report */
  int nErr;                       /* Number of lines in zReport */
};

#define RTREE_CHECK_MAX_ERROR 100

/*
** Reset SQL statement pStmt. If the sqlite3_reset() call returns an error,
** and RtreeCheck.rc==SQLITE_OK, set RtreeCheck.rc to the error code.
*/
static void rtreeCheckReset(RtreeCheck *pCheck, sqlite3_stmt *pStmt){
  int rc = sqlite3_reset(pStmt);
  if( pCheck->rc==SQLITE_OK ) pCheck->rc = rc;
}

/*
** The second and subsequent arguments to this function are a format string
** and printf style arguments. This function formats the string and attempts
** to compile it as an SQL statement.
**
** If successful, a pointer to the new SQL statement is returned. Otherwise,
** NULL is returned and an error code left in RtreeCheck.rc.
*/
static sqlite3_stmt *rtreeCheckPrepare(
  RtreeCheck *pCheck,             /* RtreeCheck object */
  const char *zFmt, ...           /* Format string and trailing args */
){
  va_list ap;
  char *z;
  sqlite3_stmt *pRet = 0;

  va_start(ap, zFmt);
  z = sqlite3_vmprintf(zFmt, ap);

  if( pCheck->rc==SQLITE_OK ){
    if( z==0 ){
      pCheck->rc = SQLITE_NOMEM;
    }else{
      pCheck->rc = sqlite3_prepare_v2(pCheck->db, z, -1, &pRet, 0);
    }
  }

  sqlite3_free(z);
  va_end(ap);
  return pRet;
}

/*
** The second and subsequent arguments to this function are a printf()
** style format string and arguments. This function formats the string and
** appends it to the report being accumulated in pCheck.
*/
static void rtreeCheckAppendMsg(RtreeCheck *pCheck, const char *zFmt, ...){
  va_list ap;
  va_start(ap, zFmt);
  if( pCheck->rc==SQLITE_OK && pCheck->nErr<RTREE_CHECK_MAX_ERROR ){
    char *z = sqlite3_vmprintf(zFmt, ap);
    if( z==0 ){
      pCheck->rc = SQLITE_NOMEM;
    }else{
      pCheck->zReport = sqlite3_mprintf("%z%s%z",
          pCheck->zReport, (pCheck->zReport ? "\n" : ""), z
      );
      if( pCheck->zReport==0 ){
        pCheck->rc = SQLITE_NOMEM;
      }
    }
    pCheck->nErr++;
  }
  va_end(ap);
}

/*
** This function is a no-op if there is already an error code stored
** in the RtreeCheck object indicated by the first argument. NULL is
** returned in this case.
**
** Otherwise, the contents of rtree table node iNode are loaded from
** the database and copied into a buffer obtained from sqlite3_malloc().
** If no error occurs, a pointer to the buffer is returned and (*pnNode)
** is set to the size of the buffer in bytes.
**
** Or, if an error does occur, NULL is returned and an error code left
** in the RtreeCheck object. The final value of *pnNode is undefined in
** this case.
*/
static u8 *rtreeCheckGetNode(RtreeCheck *pCheck, i64 iNode, int *pnNode){
  u8 *pRet = 0;                   /* Return value */

  if( pCheck->rc==SQLITE_OK && pCheck->pGetNode==0 ){
    pCheck->pGetNode = rtreeCheckPrepare(pCheck,
        "SELECT data FROM %Q.'%q_node' WHERE nodeno=?",
        pCheck->zDb, pCheck->zTab
    );
  }

  if( pCheck->rc==SQLITE_OK ){
    sqlite3_bind_int64(pCheck->pGetNode, 1, iNode);
    if( sqlite3_step(pCheck->pGetNode)==SQLITE_ROW ){
      int nNode = sqlite3_column_bytes(pCheck->pGetNode, 0);
      const u8 *pNode = (const u8*)sqlite3_column_blob(pCheck->pGetNode, 0);
      pRet = sqlite3_malloc64(nNode);
      if( pRet==0 ){
        pCheck->rc = SQLITE_NOMEM;
      }else{
        memcpy(pRet, pNode, nNode);
        *pnNode = nNode;
      }
    }
    rtreeCheckReset(pCheck, pCheck->pGetNode);
    if( pCheck->rc==SQLITE_OK && pRet==0 ){
      rtreeCheckAppendMsg(pCheck, "Node %lld missing from database", iNode);
    }
  }

  return pRet;
}

/*
** This function is used to check that the %_parent (if bLeaf==0) or %_rowid
** (if bLeaf==1) table contains a specified entry. The schemas of the
** two tables are:
**
**   CREATE TABLE %_parent(nodeno INTEGER PRIMARY KEY, parentnode INTEGER)
**   CREATE TABLE %_rowid(rowid INTEGER PRIMARY KEY, nodeno INTEGER, ...)
**
** In both cases, this function checks that there exists an entry with
** IPK value iKey and the second column set to iVal.
**
*/
static void rtreeCheckMapping(
  RtreeCheck *pCheck,             /* RtreeCheck object */
  int bLeaf,                      /* True for a leaf cell, false for interior */
  i64 iKey,                       /* Key for mapping */
  i64 iVal                        /* Expected value for mapping */
){
  int rc;
  sqlite3_stmt *pStmt;
  const char *azSql[2] = {
    "SELECT parentnode FROM %Q.'%q_parent' WHERE nodeno=?1",
    "SELECT nodeno FROM %Q.'%q_rowid' WHERE rowid=?1"
  };

  assert( bLeaf==0 || bLeaf==1 );
  if( pCheck->aCheckMapping[bLeaf]==0 ){
    pCheck->aCheckMapping[bLeaf] = rtreeCheckPrepare(pCheck,
        azSql[bLeaf], pCheck->zDb, pCheck->zTab
    );
  }
  if( pCheck->rc!=SQLITE_OK ) return;

  pStmt = pCheck->aCheckMapping[bLeaf];
  sqlite3_bind_int64(pStmt, 1, iKey);
  rc = sqlite3_step(pStmt);
  if( rc==SQLITE_DONE ){
    rtreeCheckAppendMsg(pCheck, "Mapping (%lld -> %lld) missing from %s table",
        iKey, iVal, (bLeaf ? "%_rowid" : "%_parent")
    );
  }else if( rc==SQLITE_ROW ){
    i64 ii = sqlite3_column_int64(pStmt, 0);
    if( ii!=iVal ){
      rtreeCheckAppendMsg(pCheck,
          "Found (%lld -> %lld) in %s table, expected (%lld -> %lld)",
          iKey, ii, (bLeaf ? "%_rowid" : "%_parent"), iKey, iVal
      );
    }
  }
  rtreeCheckReset(pCheck, pStmt);
}

/*
** Argument pCell points to an array of coordinates stored on an rtree page.
** This function checks that the coordinates are internally consistent (no
** x1>x2 conditions) and adds an error message to the RtreeCheck object
** if they are not.
**
** Additionally, if pParent is not NULL, then it is assumed to point to
** the array of coordinates on the parent page that bound the page
** containing pCell. In this case it is also verified that the two
** sets of coordinates are mutually consistent and an error message added
** to the RtreeCheck object if they are not.
*/
static void rtreeCheckCellCoord(
  RtreeCheck *pCheck,
  i64 iNode,                      /* Node id to use in error messages */
  int iCell,                      /* Cell number to use in error messages */
  u8 *pCell,                      /* Pointer to cell coordinates */
  u8 *pParent                     /* Pointer to parent coordinates */
){
  RtreeCoord c1, c2;
  RtreeCoord p1, p2;
  int i;

  for(i=0; i<pCheck->nDim; i++){
    readCoord(&pCell[4*2*i], &c1);
    readCoord(&pCell[4*(2*i + 1)], &c2);

    /* printf("%e, %e\n", c1.u.f, c2.u.f); */
    if( pCheck->bInt ? c1.i>c2.i : c1.f>c2.f ){
      rtreeCheckAppendMsg(pCheck,
          "Dimension %d of cell %d on node %lld is corrupt", i, iCell, iNode
      );
    }

    if( pParent ){
      readCoord(&pParent[4*2*i], &p1);
      readCoord(&pParent[4*(2*i + 1)], &p2);

      if( (pCheck->bInt ? c1.i<p1.i : c1.f<p1.f)
       || (pCheck->bInt ? c2.i>p2.i : c2.f>p2.f)
      ){
        rtreeCheckAppendMsg(pCheck,
            "Dimension %d of cell %d on node %lld is corrupt relative to parent"
            , i, iCell, iNode
        );
      }
    }
  }
}

/*
** Run rtreecheck() checks on node iNode, which is at depth iDepth within
** the r-tree structure. Argument aParent points to the array of coordinates
** that bound node iNode on the parent node.
**
** If any problems are discovered, an error message is appended to the
** report accumulated in the RtreeCheck object.
*/
static void rtreeCheckNode(
  RtreeCheck *pCheck,
  int iDepth,                     /* Depth of iNode (0==leaf) */
  u8 *aParent,                    /* Buffer containing parent coords */
  i64 iNode                       /* Node to check */
){
  u8 *aNode = 0;
  int nNode = 0;

  assert( iNode==1 || aParent!=0 );
  assert( pCheck->nDim>0 );

  aNode = rtreeCheckGetNode(pCheck, iNode, &nNode);
  if( aNode ){
    if( nNode<4 ){
      rtreeCheckAppendMsg(pCheck,
          "Node %lld is too small (%d bytes)", iNode, nNode
      );
    }else{
      int nCell;                  /* Number of cells on page */
      int i;                      /* Used to iterate through cells */
      if( aParent==0 ){
        iDepth = readInt16(aNode);
        if( iDepth>RTREE_MAX_DEPTH ){
          rtreeCheckAppendMsg(pCheck, "Rtree depth out of range (%d)", iDepth);
          sqlite3_free(aNode);
          return;
        }
      }
      nCell = readInt16(&aNode[2]);
      if( (4 + nCell*(8 + pCheck->nDim*2*4))>nNode ){
        rtreeCheckAppendMsg(pCheck,
            "Node %lld is too small for cell count of %d (%d bytes)",
            iNode, nCell, nNode
        );
      }else{
        for(i=0; i<nCell; i++){
          u8 *pCell = &aNode[4 + i*(8 + pCheck->nDim*2*4)];
          i64 iVal = readInt64(pCell);
          rtreeCheckCellCoord(pCheck, iNode, i, &pCell[8], aParent);

          if( iDepth>0 ){
            rtreeCheckMapping(pCheck, 0, iVal, iNode);
            rtreeCheckNode(pCheck, iDepth-1, &pCell[8], iVal);
            pCheck->nNonLeaf++;
          }else{
            rtreeCheckMapping(pCheck, 1, iVal, iNode);
            pCheck->nLeaf++;
          }
        }
      }
    }
    sqlite3_free(aNode);
  }
}

/*
** The second argument to this function must be either "_rowid" or
** "_parent". This function checks that the number of entries in the
** %_rowid or %_parent table is exactly nExpect. If not, it adds
** an error message to the report in the RtreeCheck object indicated
** by the first argument.
*/
static void rtreeCheckCount(RtreeCheck *pCheck, const char *zTbl, i64 nExpect){
  if( pCheck->rc==SQLITE_OK ){
    sqlite3_stmt *pCount;
    pCount = rtreeCheckPrepare(pCheck, "SELECT count(*) FROM %Q.'%q%s'",
        pCheck->zDb, pCheck->zTab, zTbl
    );
    if( pCount ){
      if( sqlite3_step(pCount)==SQLITE_ROW ){
        i64 nActual = sqlite3_column_int64(pCount, 0);
        if( nActual!=nExpect ){
          rtreeCheckAppendMsg(pCheck, "Wrong number of entries in %%%s table"
              " - expected %lld, actual %lld" , zTbl, nExpect, nActual
          );
        }
      }
      pCheck->rc = sqlite3_finalize(pCount);
    }
  }
}

/*
** This function does the bulk of the work for the rtree integrity-check.
** It is called by rtreecheck(), which is the SQL function implementation.
*/
static int rtreeCheckTable(
  sqlite3 *db,                    /* Database handle to access db through */
  const char *zDb,                /* Name of db ("main", "temp" etc.) */
  const char *zTab,               /* Name of rtree table to check */
  char **pzReport                 /* OUT: sqlite3_malloc'd report text */
){
  RtreeCheck check;               /* Common context for various routines */
  sqlite3_stmt *pStmt = 0;        /* Used to find column count of rtree table */
  int nAux = 0;                   /* Number of extra columns. */

  /* Initialize the context object */
  memset(&check, 0, sizeof(check));
  check.db = db;
  check.zDb = zDb;
  check.zTab = zTab;

  /* Find the number of auxiliary columns */
  pStmt = rtreeCheckPrepare(&check, "SELECT * FROM %Q.'%q_rowid'", zDb, zTab);
  if( pStmt ){
    nAux = sqlite3_column_count(pStmt) - 2;
    sqlite3_finalize(pStmt);
  }else
  if( check.rc!=SQLITE_NOMEM ){
    check.rc = SQLITE_OK;
  }

  /* Find number of dimensions in the rtree table. */
  pStmt = rtreeCheckPrepare(&check, "SELECT * FROM %Q.%Q", zDb, zTab);
  if( pStmt ){
    int rc;
    check.nDim = (sqlite3_column_count(pStmt) - 1 - nAux) / 2;
    if( check.nDim<1 ){
      rtreeCheckAppendMsg(&check, "Schema corrupt or not an rtree");
    }else if( SQLITE_ROW==sqlite3_step(pStmt) ){
      check.bInt = (sqlite3_column_type(pStmt, 1)==SQLITE_INTEGER);
    }
    rc = sqlite3_finalize(pStmt);
    if( rc!=SQLITE_CORRUPT ) check.rc = rc;
  }

  /* Do the actual integrity-check */
  if( check.nDim>=1 ){
    if( check.rc==SQLITE_OK ){
      rtreeCheckNode(&check, 0, 0, 1);
    }
    rtreeCheckCount(&check, "_rowid", check.nLeaf);
    rtreeCheckCount(&check, "_parent", check.nNonLeaf);
  }

  /* Finalize SQL statements used by the integrity-check */
  sqlite3_finalize(check.pGetNode);
  sqlite3_finalize(check.aCheckMapping[0]);
  sqlite3_finalize(check.aCheckMapping[1]);

  *pzReport = check.zReport;
  return check.rc;
}

/*
** Implementation of the xIntegrity method for Rtree.
*/
static int rtreeIntegrity(
  sqlite3_vtab *pVtab,   /* The virtual table to check */
  const char *zSchema,   /* Schema in which the virtual table lives */
  const char *zName,     /* Name of the virtual table */
  int isQuick,           /* True for a quick_check */
  char **pzErr           /* Write results here */
){
  Rtree *pRtree = (Rtree*)pVtab;
  int rc;
  assert( pzErr!=0 && *pzErr==0 );
  UNUSED_PARAMETER(zSchema);
  UNUSED_PARAMETER(zName);
  UNUSED_PARAMETER(isQuick);
  rc = rtreeCheckTable(pRtree->db, pRtree->zDb, pRtree->zName, pzErr);
  if( rc==SQLITE_OK && *pzErr ){
    *pzErr = sqlite3_mprintf("In RTree %s.%s:\n%z",
                 pRtree->zDb, pRtree->zName, *pzErr);
    if( (*pzErr)==0 ) rc = SQLITE_NOMEM;
  }
  return rc;
}

/*
** Usage:
**
**   rtreecheck(<rtree-table>);
**   rtreecheck(<database>, <rtree-table>);
**
** Invoking this SQL function runs an integrity-check on the named rtree
** table. The integrity-check verifies the following:
**
**   1. For each cell in the r-tree structure (%_node table), that:
**
**       a) for each dimension, (coord1 <= coord2).
**
**       b) unless the cell is on the root node, that the cell is bounded
**          by the parent cell on the parent node.
**
**       c) for leaf nodes, that there is an entry in the %_rowid
**          table corresponding to the cell's rowid value that
**          points to the correct node.
**
**       d) for cells on non-leaf nodes, that there is an entry in the
**          %_parent table mapping from the cell's child node to the
**          node that it resides on.
**
**   2. That there are the same number of entries in the %_rowid table
**      as there are leaf cells in the r-tree structure, and that there
**      is a leaf cell that corresponds to each entry in the %_rowid table.
**
**   3. That there are the same number of entries in the %_parent table
**      as there are non-leaf cells in the r-tree structure, and that
**      there is a non-leaf cell that corresponds to each entry in the
**      %_parent table.
*/
static void rtreecheck(
  sqlite3_context *ctx,
  int nArg,
  sqlite3_value **apArg
){
  if( nArg!=1 && nArg!=2 ){
    sqlite3_result_error(ctx,
        "wrong number of arguments to function rtreecheck()", -1
    );
  }else{
    int rc;
    char *zReport = 0;
    const char *zDb = (const char*)sqlite3_value_text(apArg[0]);
    const char *zTab;
    if( nArg==1 ){
      zTab = zDb;
      zDb = "main";
    }else{
      zTab = (const char*)sqlite3_value_text(apArg[1]);
    }
    rc = rtreeCheckTable(sqlite3_context_db_handle(ctx), zDb, zTab, &zReport);
    if( rc==SQLITE_OK ){
      sqlite3_result_text(ctx, zReport ? zReport : "ok", -1, SQLITE_TRANSIENT);
    }else{
      sqlite3_result_error_code(ctx, rc);
    }
    sqlite3_free(zReport);
  }
}

/* Conditionally include the geopoly code */
#ifdef SQLITE_ENABLE_GEOPOLY
/************** Include geopoly.c in the middle of rtree.c *******************/
/************** Begin file geopoly.c *****************************************/
/*
** 2018-05-25
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file implements an alternative R-Tree virtual table that
** uses polygons to express the boundaries of 2-dimensional objects.
**
** This file is #include-ed onto the end of "rtree.c" so that it has
** access to all of the R-Tree internals.
*/
/* #include <stdlib.h> */

/* Enable -DGEOPOLY_ENABLE_DEBUG for debugging facilities */
#ifdef GEOPOLY_ENABLE_DEBUG
  static int geo_debug = 0;
# define GEODEBUG(X) if(geo_debug)printf X
#else
# define GEODEBUG(X)
#endif

/* Character class routines */
#ifdef sqlite3Isdigit
   /* Use the SQLite core versions if this routine is part of the
   ** SQLite amalgamation */
#  define safe_isdigit(x)  sqlite3Isdigit(x)
#  define safe_isalnum(x)  sqlite3Isalnum(x)
#  define safe_isxdigit(x) sqlite3Isxdigit(x)
#else
   /* Use the standard library for separate compilation */
#include <ctype.h>  /* amalgamator: keep */
#  define safe_isdigit(x)  isdigit((unsigned char)(x))
#  define safe_isalnum(x)  isalnum((unsigned char)(x))
#  define safe_isxdigit(x) isxdigit((unsigned char)(x))
#endif

#ifndef JSON_NULL   /* The following stuff repeats things found in json1 */
/*
** Growing our own isspace() routine this way is twice as fast as
** the library isspace() function.
*/
static const char geopolyIsSpace[] = {
  0, 0, 0, 0, 0, 0, 0, 0,     0, 1, 1, 0, 0, 1, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
};
#define fast_isspace(x) (geopolyIsSpace[(unsigned char)x])
#endif /* JSON NULL - back to original code */

/* Compiler and version */
#ifndef GCC_VERSION
#if defined(__GNUC__) && !defined(SQLITE_DISABLE_INTRINSIC)
# define GCC_VERSION (__GNUC__*1000000+__GNUC_MINOR__*1000+__GNUC_PATCHLEVEL__)
#else
# define GCC_VERSION 0
#endif
#endif
#ifndef MSVC_VERSION
#if defined(_MSC_VER) && !defined(SQLITE_DISABLE_INTRINSIC)
# define MSVC_VERSION _MSC_VER
#else
# define MSVC_VERSION 0
#endif
#endif

/* Datatype for coordinates
*/
typedef float GeoCoord;

/*
** Internal representation of a polygon.
**
** The polygon consists of a sequence of vertexes.  There is a line
** segment between each pair of vertexes, and one final segment from
** the last vertex back to the first.  (This differs from the GeoJSON
** standard in which the final vertex is a repeat of the first.)
**
** The polygon follows the right-hand rule.  The area to the right of
** each segment is "outside" and the area to the left is "inside".
**
** The on-disk representation consists of a 4-byte header followed by
** the values.  The 4-byte header is:
**
**      encoding    (1 byte)   0=big-endian, 1=little-endian
**      nvertex     (3 bytes)  Number of vertexes as a big-endian integer
**
** Enough space is allocated for 4 coordinates, to work around over-zealous
** warnings coming from some compiler (notably, clang). In reality, the size
** of each GeoPoly memory allocate is adjusted as necessary so that the
** GeoPoly.a[] array at the end is the appropriate size.
*/
typedef struct GeoPoly GeoPoly;
struct GeoPoly {
  int nVertex;          /* Number of vertexes */
  unsigned char hdr[4]; /* Header for on-disk representation */
  GeoCoord a[8];        /* 2*nVertex values. X (longitude) first, then Y */
};

/* The size of a memory allocation needed for a GeoPoly object sufficient
** to hold N coordinate pairs.
*/
#define GEOPOLY_SZ(N)  (sizeof(GeoPoly) + sizeof(GeoCoord)*2*((N)-4))

/* Macros to access coordinates of a GeoPoly.
** We have to use these macros, rather than just say p->a[i] in order
** to silence (incorrect) UBSAN warnings if the array index is too large.
*/
#define GeoX(P,I)  (((GeoCoord*)(P)->a)[(I)*2])
#define GeoY(P,I)  (((GeoCoord*)(P)->a)[(I)*2+1])


/*
** State of a parse of a GeoJSON input.
*/
typedef struct GeoParse GeoParse;
struct GeoParse {
  const unsigned char *z;   /* Unparsed input */
  int nVertex;              /* Number of vertexes in a[] */
  int nAlloc;               /* Space allocated to a[] */
  int nErr;                 /* Number of errors encountered */
  GeoCoord *a;          /* Array of vertexes.  From sqlite3_malloc64() */
};

/* Do a 4-byte byte swap */
static void geopolySwab32(unsigned char *a){
  unsigned char t = a[0];
  a[0] = a[3];
  a[3] = t;
  t = a[1];
  a[1] = a[2];
  a[2] = t;
}

/* Skip whitespace.  Return the next non-whitespace character. */
static char geopolySkipSpace(GeoParse *p){
  while( fast_isspace(p->z[0]) ) p->z++;
  return p->z[0];
}

/* Parse out a number.  Write the value into *pVal if pVal!=0.
** return non-zero on success and zero if the next token is not a number.
*/
static int geopolyParseNumber(GeoParse *p, GeoCoord *pVal){
  char c = geopolySkipSpace(p);
  const unsigned char *z = p->z;
  int j = 0;
  int seenDP = 0;
  int seenE = 0;
  if( c=='-' ){
    j = 1;
    c = z[j];
  }
  if( c=='0' && z[j+1]>='0' && z[j+1]<='9' ) return 0;
  for(;; j++){
    c = z[j];
    if( safe_isdigit(c) ) continue;
    if( c=='.' ){
      if( z[j-1]=='-' ) return 0;
      if( seenDP ) return 0;
      seenDP = 1;
      continue;
    }
    if( c=='e' || c=='E' ){
      if( z[j-1]<'0' ) return 0;
      if( seenE ) return -1;
      seenDP = seenE = 1;
      c = z[j+1];
      if( c=='+' || c=='-' ){
        j++;
        c = z[j+1];
      }
      if( c<'0' || c>'9' ) return 0;
      continue;
    }
    break;
  }
  if( z[j-1]<'0' ) return 0;
  if( pVal ){
#ifdef SQLITE_AMALGAMATION
     /* The sqlite3AtoF() routine is much much faster than atof(), if it
     ** is available */
     double r;
     (void)sqlite3AtoF((const char*)p->z, &r);
     *pVal = r;
#else
     *pVal = (GeoCoord)atof((const char*)p->z);
#endif
  }
  p->z += j;
  return 1;
}

/*
** If the input is a well-formed JSON array of coordinates with at least
** four coordinates and where each coordinate is itself a two-value array,
** then convert the JSON into a GeoPoly object and return a pointer to
** that object.
**
** If any error occurs, return NULL.
*/
static GeoPoly *geopolyParseJson(const unsigned char *z, int *pRc){
  GeoParse s;
  int rc = SQLITE_OK;
  memset(&s, 0, sizeof(s));
  s.z = z;
  if( geopolySkipSpace(&s)=='[' ){
    s.z++;
    while( geopolySkipSpace(&s)=='[' ){
      int ii = 0;
      char c;
      s.z++;
      if( s.nVertex>=s.nAlloc ){
        GeoCoord *aNew;
        s.nAlloc = s.nAlloc*2 + 16;
        aNew = sqlite3_realloc64(s.a, s.nAlloc*sizeof(GeoCoord)*2 );
        if( aNew==0 ){
          rc = SQLITE_NOMEM;
          s.nErr++;
          break;
        }
        s.a = aNew;
      }
      while( geopolyParseNumber(&s, ii<=1 ? &s.a[s.nVertex*2+ii] : 0) ){
        ii++;
        if( ii==2 ) s.nVertex++;
        c = geopolySkipSpace(&s);
        s.z++;
        if( c==',' ) continue;
        if( c==']' && ii>=2 ) break;
        s.nErr++;
        rc = SQLITE_ERROR;
        goto parse_json_err;
      }
      if( geopolySkipSpace(&s)==',' ){
        s.z++;
        continue;
      }
      break;
    }
    if( geopolySkipSpace(&s)==']'
     && s.nVertex>=4
     && s.a[0]==s.a[s.nVertex*2-2]
     && s.a[1]==s.a[s.nVertex*2-1]
     && (s.z++, geopolySkipSpace(&s)==0)
    ){
      GeoPoly *pOut;
      int x = 1;
      s.nVertex--;  /* Remove the redundant vertex at the end */
      pOut = sqlite3_malloc64( GEOPOLY_SZ((sqlite3_int64)s.nVertex) );
      x = 1;
      if( pOut==0 ) goto parse_json_err;
      pOut->nVertex = s.nVertex;
      memcpy(pOut->a, s.a, s.nVertex*2*sizeof(GeoCoord));
      pOut->hdr[0] = *(unsigned char*)&x;
      pOut->hdr[1] = (s.nVertex>>16)&0xff;
      pOut->hdr[2] = (s.nVertex>>8)&0xff;
      pOut->hdr[3] = s.nVertex&0xff;
      sqlite3_free(s.a);
      if( pRc ) *pRc = SQLITE_OK;
      return pOut;
    }else{
      s.nErr++;
      rc = SQLITE_ERROR;
    }
  }
parse_json_err:
  if( pRc ) *pRc = rc;
  sqlite3_free(s.a);
  return 0;
}

/*
** Given a function parameter, try to interpret it as a polygon, either
** in the binary format or JSON text.  Compute a GeoPoly object and
** return a pointer to that object.  Or if the input is not a well-formed
** polygon, put an error message in sqlite3_context and return NULL.
*/
static GeoPoly *geopolyFuncParam(
  sqlite3_context *pCtx,      /* Context for error messages */
  sqlite3_value *pVal,        /* The value to decode */
  int *pRc                    /* Write error here */
){
  GeoPoly *p = 0;
  int nByte;
  testcase( pCtx==0 );
  if( sqlite3_value_type(pVal)==SQLITE_BLOB
   && (nByte = sqlite3_value_bytes(pVal))>=(int)(4+6*sizeof(GeoCoord))
  ){
    const unsigned char *a = sqlite3_value_blob(pVal);
    int nVertex;
    if( a==0 ){
      if( pCtx ) sqlite3_result_error_nomem(pCtx);
      return 0;
    }
    nVertex = (a[1]<<16) + (a[2]<<8) + a[3];
    if( (a[0]==0 || a[0]==1)
     && (nVertex*2*sizeof(GeoCoord) + 4)==(unsigned int)nByte
    ){
      p = sqlite3_malloc64( sizeof(*p) + (nVertex-1)*2*sizeof(GeoCoord) );
      if( p==0 ){
        if( pRc ) *pRc = SQLITE_NOMEM;
        if( pCtx ) sqlite3_result_error_nomem(pCtx);
      }else{
        int x = 1;
        p->nVertex = nVertex;
        memcpy(p->hdr, a, nByte);
        if( a[0] != *(unsigned char*)&x ){
          int ii;
          for(ii=0; ii<nVertex; ii++){
            geopolySwab32((unsigned char*)&GeoX(p,ii));
            geopolySwab32((unsigned char*)&GeoY(p,ii));
          }
          p->hdr[0] ^= 1;
        }
      }
    }
    if( pRc ) *pRc = SQLITE_OK;
    return p;
  }else if( sqlite3_value_type(pVal)==SQLITE_TEXT ){
    const unsigned char *zJson = sqlite3_value_text(pVal);
    if( zJson==0 ){
      if( pRc ) *pRc = SQLITE_NOMEM;
      return 0;
    }
    return geopolyParseJson(zJson, pRc);
  }else{
    if( pRc ) *pRc = SQLITE_ERROR;
    return 0;
  }
}

/*
** Implementation of the geopoly_blob(X) function.
**
** If the input is a well-formed Geopoly BLOB or JSON string
** then return the BLOB representation of the polygon.  Otherwise
** return NULL.
*/
static void geopolyBlobFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  GeoPoly *p = geopolyFuncParam(context, argv[0], 0);
  (void)argc;
  if( p ){
    sqlite3_result_blob(context, p->hdr,
       4+8*p->nVertex, SQLITE_TRANSIENT);
    sqlite3_free(p);
  }
}

/*
** SQL function:     geopoly_json(X)
**
** Interpret X as a polygon and render it as a JSON array
** of coordinates.  Or, if X is not a valid polygon, return NULL.
*/
static void geopolyJsonFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  GeoPoly *p = geopolyFuncParam(context, argv[0], 0);
  (void)argc;
  if( p ){
    sqlite3 *db = sqlite3_context_db_handle(context);
    sqlite3_str *x = sqlite3_str_new(db);
    int i;
    sqlite3_str_append(x, "[", 1);
    for(i=0; i<p->nVertex; i++){
      sqlite3_str_appendf(x, "[%!g,%!g],", GeoX(p,i), GeoY(p,i));
    }
    sqlite3_str_appendf(x, "[%!g,%!g]]", GeoX(p,0), GeoY(p,0));
    sqlite3_result_text(context, sqlite3_str_finish(x), -1, sqlite3_free);
    sqlite3_free(p);
  }
}

/*
** SQL function:     geopoly_svg(X, ....)
**
** Interpret X as a polygon and render it as a SVG <polyline>.
** Additional arguments are added as attributes to the <polyline>.
*/
static void geopolySvgFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  GeoPoly *p;
  if( argc<1 ) return;
  p = geopolyFuncParam(context, argv[0], 0);
  if( p ){
    sqlite3 *db = sqlite3_context_db_handle(context);
    sqlite3_str *x = sqlite3_str_new(db);
    int i;
    char cSep = '\'';
    sqlite3_str_appendf(x, "<polyline points=");
    for(i=0; i<p->nVertex; i++){
      sqlite3_str_appendf(x, "%c%g,%g", cSep, GeoX(p,i), GeoY(p,i));
      cSep = ' ';
    }
    sqlite3_str_appendf(x, " %g,%g'", GeoX(p,0), GeoY(p,0));
    for(i=1; i<argc; i++){
      const char *z = (const char*)sqlite3_value_text(argv[i]);
      if( z && z[0] ){
        sqlite3_str_appendf(x, " %s", z);
      }
    }
    sqlite3_str_appendf(x, "></polyline>");
    sqlite3_result_text(context, sqlite3_str_finish(x), -1, sqlite3_free);
    sqlite3_free(p);
  }
}

/*
** SQL Function:      geopoly_xform(poly, A, B, C, D, E, F)
**
** Transform and/or translate a polygon as follows:
**
**      x1 = A*x0 + B*y0 + E
**      y1 = C*x0 + D*y0 + F
**
** For a translation:
**
**      geopoly_xform(poly, 1, 0, 0, 1, x-offset, y-offset)
**
** Rotate by R around the point (0,0):
**
**      geopoly_xform(poly, cos(R), sin(R), -sin(R), cos(R), 0, 0)
*/
static void geopolyXformFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  GeoPoly *p = geopolyFuncParam(context, argv[0], 0);
  double A = sqlite3_value_double(argv[1]);
  double B = sqlite3_value_double(argv[2]);
  double C = sqlite3_value_double(argv[3]);
  double D = sqlite3_value_double(argv[4]);
  double E = sqlite3_value_double(argv[5]);
  double F = sqlite3_value_double(argv[6]);
  GeoCoord x1, y1, x0, y0;
  int ii;
  (void)argc;
  if( p ){
    for(ii=0; ii<p->nVertex; ii++){
      x0 = GeoX(p,ii);
      y0 = GeoY(p,ii);
      x1 = (GeoCoord)(A*x0 + B*y0 + E);
      y1 = (GeoCoord)(C*x0 + D*y0 + F);
      GeoX(p,ii) = x1;
      GeoY(p,ii) = y1;
    }
    sqlite3_result_blob(context, p->hdr,
       4+8*p->nVertex, SQLITE_TRANSIENT);
    sqlite3_free(p);
  }
}

/*
** Compute the area enclosed by the polygon.
**
** This routine can also be used to detect polygons that rotate in
** the wrong direction.  Polygons are suppose to be counter-clockwise (CCW).
** This routine returns a negative value for clockwise (CW) polygons.
*/
static double geopolyArea(GeoPoly *p){
  double rArea = 0.0;
  int ii;
  for(ii=0; ii<p->nVertex-1; ii++){
    rArea += (GeoX(p,ii) - GeoX(p,ii+1))           /* (x0 - x1) */
              * (GeoY(p,ii) + GeoY(p,ii+1))        /* (y0 + y1) */
              * 0.5;
  }
  rArea += (GeoX(p,ii) - GeoX(p,0))                /* (xN - x0) */
           * (GeoY(p,ii) + GeoY(p,0))              /* (yN + y0) */
           * 0.5;
  return rArea;
}

/*
** Implementation of the geopoly_area(X) function.
**
** If the input is a well-formed Geopoly BLOB then return the area
** enclosed by the polygon.  If the polygon circulates clockwise instead
** of counterclockwise (as it should) then return the negative of the
** enclosed area.  Otherwise return NULL.
*/
static void geopolyAreaFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  GeoPoly *p = geopolyFuncParam(context, argv[0], 0);
  (void)argc;
  if( p ){
    sqlite3_result_double(context, geopolyArea(p));
    sqlite3_free(p);
  }
}

/*
** Implementation of the geopoly_ccw(X) function.
**
** If the rotation of polygon X is clockwise (incorrect) instead of
** counter-clockwise (the correct winding order according to RFC7946)
** then reverse the order of the vertexes in polygon X.
**
** In other words, this routine returns a CCW polygon regardless of the
** winding order of its input.
**
** Use this routine to sanitize historical inputs that that sometimes
** contain polygons that wind in the wrong direction.
*/
static void geopolyCcwFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  GeoPoly *p = geopolyFuncParam(context, argv[0], 0);
  (void)argc;
  if( p ){
    if( geopolyArea(p)<0.0 ){
      int ii, jj;
      for(ii=1, jj=p->nVertex-1; ii<jj; ii++, jj--){
        GeoCoord t = GeoX(p,ii);
        GeoX(p,ii) = GeoX(p,jj);
        GeoX(p,jj) = t;
        t = GeoY(p,ii);
        GeoY(p,ii) = GeoY(p,jj);
        GeoY(p,jj) = t;
      }
    }
    sqlite3_result_blob(context, p->hdr,
       4+8*p->nVertex, SQLITE_TRANSIENT);
    sqlite3_free(p);
  }
}

#define GEOPOLY_PI 3.1415926535897932385

/* Fast approximation for sine(X) for X between -0.5*pi and 2*pi
*/
static double geopolySine(double r){
  assert( r>=-0.5*GEOPOLY_PI && r<=2.0*GEOPOLY_PI );
  if( r>=1.5*GEOPOLY_PI ){
    r -= 2.0*GEOPOLY_PI;
  }
  if( r>=0.5*GEOPOLY_PI ){
    return -geopolySine(r-GEOPOLY_PI);
  }else{
    double r2 = r*r;
    double r3 = r2*r;
    double r5 = r3*r2;
    return 0.9996949*r - 0.1656700*r3 + 0.0075134*r5;
  }
}

/*
** Function:   geopoly_regular(X,Y,R,N)
**
** Construct a simple, convex, regular polygon centered at X, Y
** with circumradius R and with N sides.
*/
static void geopolyRegularFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  double x = sqlite3_value_double(argv[0]);
  double y = sqlite3_value_double(argv[1]);
  double r = sqlite3_value_double(argv[2]);
  int n = sqlite3_value_int(argv[3]);
  int i;
  GeoPoly *p;
  (void)argc;

  if( n<3 || r<=0.0 ) return;
  if( n>1000 ) n = 1000;
  p = sqlite3_malloc64( sizeof(*p) + (n-1)*2*sizeof(GeoCoord) );
  if( p==0 ){
    sqlite3_result_error_nomem(context);
    return;
  }
  i = 1;
  p->hdr[0] = *(unsigned char*)&i;
  p->hdr[1] = 0;
  p->hdr[2] = (n>>8)&0xff;
  p->hdr[3] = n&0xff;
  for(i=0; i<n; i++){
    double rAngle = 2.0*GEOPOLY_PI*i/n;
    GeoX(p,i) = x - r*geopolySine(rAngle-0.5*GEOPOLY_PI);
    GeoY(p,i) = y + r*geopolySine(rAngle);
  }
  sqlite3_result_blob(context, p->hdr, 4+8*n, SQLITE_TRANSIENT);
  sqlite3_free(p);
}

/*
** If pPoly is a polygon, compute its bounding box. Then:
**
**    (1) if aCoord!=0 store the bounding box in aCoord, returning NULL
**    (2) otherwise, compute a GeoPoly for the bounding box and return the
**        new GeoPoly
**
** If pPoly is NULL but aCoord is not NULL, then compute a new GeoPoly from
** the bounding box in aCoord and return a pointer to that GeoPoly.
*/
static GeoPoly *geopolyBBox(
  sqlite3_context *context,   /* For recording the error */
  sqlite3_value *pPoly,       /* The polygon */
  RtreeCoord *aCoord,         /* Results here */
  int *pRc                    /* Error code here */
){
  GeoPoly *pOut = 0;
  GeoPoly *p;
  float mnX, mxX, mnY, mxY;
  if( pPoly==0 && aCoord!=0 ){
    p = 0;
    mnX = aCoord[0].f;
    mxX = aCoord[1].f;
    mnY = aCoord[2].f;
    mxY = aCoord[3].f;
    goto geopolyBboxFill;
  }else{
    p = geopolyFuncParam(context, pPoly, pRc);
  }
  if( p ){
    int ii;
    mnX = mxX = GeoX(p,0);
    mnY = mxY = GeoY(p,0);
    for(ii=1; ii<p->nVertex; ii++){
      double r = GeoX(p,ii);
      if( r<mnX ) mnX = (float)r;
      else if( r>mxX ) mxX = (float)r;
      r = GeoY(p,ii);
      if( r<mnY ) mnY = (float)r;
      else if( r>mxY ) mxY = (float)r;
    }
    if( pRc ) *pRc = SQLITE_OK;
    if( aCoord==0 ){
      geopolyBboxFill:
      pOut = sqlite3_realloc64(p, GEOPOLY_SZ(4));
      if( pOut==0 ){
        sqlite3_free(p);
        if( context ) sqlite3_result_error_nomem(context);
        if( pRc ) *pRc = SQLITE_NOMEM;
        return 0;
      }
      pOut->nVertex = 4;
      ii = 1;
      pOut->hdr[0] = *(unsigned char*)&ii;
      pOut->hdr[1] = 0;
      pOut->hdr[2] = 0;
      pOut->hdr[3] = 4;
      GeoX(pOut,0) = mnX;
      GeoY(pOut,0) = mnY;
      GeoX(pOut,1) = mxX;
      GeoY(pOut,1) = mnY;
      GeoX(pOut,2) = mxX;
      GeoY(pOut,2) = mxY;
      GeoX(pOut,3) = mnX;
      GeoY(pOut,3) = mxY;
    }else{
      sqlite3_free(p);
      aCoord[0].f = mnX;
      aCoord[1].f = mxX;
      aCoord[2].f = mnY;
      aCoord[3].f = mxY;
    }
  }else if( aCoord ){
    memset(aCoord, 0, sizeof(RtreeCoord)*4);
  }
  return pOut;
}

/*
** Implementation of the geopoly_bbox(X) SQL function.
*/
static void geopolyBBoxFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  GeoPoly *p = geopolyBBox(context, argv[0], 0, 0);
  (void)argc;
  if( p ){
    sqlite3_result_blob(context, p->hdr,
       4+8*p->nVertex, SQLITE_TRANSIENT);
    sqlite3_free(p);
  }
}

/*
** State vector for the geopoly_group_bbox() aggregate function.
*/
typedef struct GeoBBox GeoBBox;
struct GeoBBox {
  int isInit;
  RtreeCoord a[4];
};


/*
** Implementation of the geopoly_group_bbox(X) aggregate SQL function.
*/
static void geopolyBBoxStep(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  RtreeCoord a[4];
  int rc = SQLITE_OK;
  (void)argc;
  (void)geopolyBBox(context, argv[0], a, &rc);
  if( rc==SQLITE_OK ){
    GeoBBox *pBBox;
    pBBox = (GeoBBox*)sqlite3_aggregate_context(context, sizeof(*pBBox));
    if( pBBox==0 ) return;
    if( pBBox->isInit==0 ){
      pBBox->isInit = 1;
      memcpy(pBBox->a, a, sizeof(RtreeCoord)*4);
    }else{
      if( a[0].f < pBBox->a[0].f ) pBBox->a[0] = a[0];
      if( a[1].f > pBBox->a[1].f ) pBBox->a[1] = a[1];
      if( a[2].f < pBBox->a[2].f ) pBBox->a[2] = a[2];
      if( a[3].f > pBBox->a[3].f ) pBBox->a[3] = a[3];
    }
  }
}
static void geopolyBBoxFinal(
  sqlite3_context *context
){
  GeoPoly *p;
  GeoBBox *pBBox;
  pBBox = (GeoBBox*)sqlite3_aggregate_context(context, 0);
  if( pBBox==0 ) return;
  p = geopolyBBox(context, 0, pBBox->a, 0);
  if( p ){
    sqlite3_result_blob(context, p->hdr,
       4+8*p->nVertex, SQLITE_TRANSIENT);
    sqlite3_free(p);
  }
}


/*
** Determine if point (x0,y0) is beneath line segment (x1,y1)->(x2,y2).
** Returns:
**
**    +2  x0,y0 is on the line segment
**
**    +1  x0,y0 is beneath line segment
**
**    0   x0,y0 is not on or beneath the line segment or the line segment
**        is vertical and x0,y0 is not on the line segment
**
** The left-most coordinate min(x1,x2) is not considered to be part of
** the line segment for the purposes of this analysis.
*/
static int pointBeneathLine(
  double x0, double y0,
  double x1, double y1,
  double x2, double y2
){
  double y;
  if( x0==x1 && y0==y1 ) return 2;
  if( x1<x2 ){
    if( x0<=x1 || x0>x2 ) return 0;
  }else if( x1>x2 ){
    if( x0<=x2 || x0>x1 ) return 0;
  }else{
    /* Vertical line segment */
    if( x0!=x1 ) return 0;
    if( y0<y1 && y0<y2 ) return 0;
    if( y0>y1 && y0>y2 ) return 0;
    return 2;
  }
  y = y1 + (y2-y1)*(x0-x1)/(x2-x1);
  if( y0==y ) return 2;
  if( y0<y ) return 1;
  return 0;
}

/*
** SQL function:    geopoly_contains_point(P,X,Y)
**
** Return +2 if point X,Y is within polygon P.
** Return +1 if point X,Y is on the polygon boundary.
** Return 0 if point X,Y is outside the polygon
*/
static void geopolyContainsPointFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  GeoPoly *p1 = geopolyFuncParam(context, argv[0], 0);
  double x0 = sqlite3_value_double(argv[1]);
  double y0 = sqlite3_value_double(argv[2]);
  int v = 0;
  int cnt = 0;
  int ii;
  (void)argc;

  if( p1==0 ) return;
  for(ii=0; ii<p1->nVertex-1; ii++){
    v = pointBeneathLine(x0,y0,GeoX(p1,ii), GeoY(p1,ii),
                               GeoX(p1,ii+1),GeoY(p1,ii+1));
    if( v==2 ) break;
    cnt += v;
  }
  if( v!=2 ){
    v = pointBeneathLine(x0,y0,GeoX(p1,ii), GeoY(p1,ii),
                               GeoX(p1,0),  GeoY(p1,0));
  }
  if( v==2 ){
    sqlite3_result_int(context, 1);
  }else if( ((v+cnt)&1)==0 ){
    sqlite3_result_int(context, 0);
  }else{
    sqlite3_result_int(context, 2);
  }
  sqlite3_free(p1);
}

/* Forward declaration */
static int geopolyOverlap(GeoPoly *p1, GeoPoly *p2);

/*
** SQL function:    geopoly_within(P1,P2)
**
** Return +2 if P1 and P2 are the same polygon
** Return +1 if P2 is contained within P1
** Return 0 if any part of P2 is on the outside of P1
**
*/
static void geopolyWithinFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  GeoPoly *p1 = geopolyFuncParam(context, argv[0], 0);
  GeoPoly *p2 = geopolyFuncParam(context, argv[1], 0);
  (void)argc;
  if( p1 && p2 ){
    int x = geopolyOverlap(p1, p2);
    if( x<0 ){
      sqlite3_result_error_nomem(context);
    }else{
      sqlite3_result_int(context, x==2 ? 1 : x==4 ? 2 : 0);
    }
  }
  sqlite3_free(p1);
  sqlite3_free(p2);
}

/* Objects used by the overlap algorithm. */
typedef struct GeoEvent GeoEvent;
typedef struct GeoSegment GeoSegment;
typedef struct GeoOverlap GeoOverlap;
struct GeoEvent {
  double x;              /* X coordinate at which event occurs */
  int eType;             /* 0 for ADD, 1 for REMOVE */
  GeoSegment *pSeg;      /* The segment to be added or removed */
  GeoEvent *pNext;       /* Next event in the sorted list */
};
struct GeoSegment {
  double C, B;           /* y = C*x + B */
  double y;              /* Current y value */
  float y0;              /* Initial y value */
  unsigned char side;    /* 1 for p1, 2 for p2 */
  unsigned int idx;      /* Which segment within the side */
  GeoSegment *pNext;     /* Next segment in a list sorted by y */
};
struct GeoOverlap {
  GeoEvent *aEvent;          /* Array of all events */
  GeoSegment *aSegment;      /* Array of all segments */
  int nEvent;                /* Number of events */
  int nSegment;              /* Number of segments */
};

/*
** Add a single segment and its associated events.
*/
static void geopolyAddOneSegment(
  GeoOverlap *p,
  GeoCoord x0,
  GeoCoord y0,
  GeoCoord x1,
  GeoCoord y1,
  unsigned char side,
  unsigned int idx
){
  GeoSegment *pSeg;
  GeoEvent *pEvent;
  if( x0==x1 ) return;  /* Ignore vertical segments */
  if( x0>x1 ){
    GeoCoord t = x0;
    x0 = x1;
    x1 = t;
    t = y0;
    y0 = y1;
    y1 = t;
  }
  pSeg = p->aSegment + p->nSegment;
  p->nSegment++;
  pSeg->C = (y1-y0)/(x1-x0);
  pSeg->B = y1 - x1*pSeg->C;
  pSeg->y0 = y0;
  pSeg->side = side;
  pSeg->idx = idx;
  pEvent = p->aEvent + p->nEvent;
  p->nEvent++;
  pEvent->x = x0;
  pEvent->eType = 0;
  pEvent->pSeg = pSeg;
  pEvent = p->aEvent + p->nEvent;
  p->nEvent++;
  pEvent->x = x1;
  pEvent->eType = 1;
  pEvent->pSeg = pSeg;
}



/*
** Insert all segments and events for polygon pPoly.
*/
static void geopolyAddSegments(
  GeoOverlap *p,          /* Add segments to this Overlap object */
  GeoPoly *pPoly,         /* Take all segments from this polygon */
  unsigned char side      /* The side of pPoly */
){
  unsigned int i;
  GeoCoord *x;
  for(i=0; i<(unsigned)pPoly->nVertex-1; i++){
    x = &GeoX(pPoly,i);
    geopolyAddOneSegment(p, x[0], x[1], x[2], x[3], side, i);
  }
  x = &GeoX(pPoly,i);
  geopolyAddOneSegment(p, x[0], x[1], pPoly->a[0], pPoly->a[1], side, i);
}

/*
** Merge two lists of sorted events by X coordinate
*/
static GeoEvent *geopolyEventMerge(GeoEvent *pLeft, GeoEvent *pRight){
  GeoEvent head, *pLast;
  head.pNext = 0;
  pLast = &head;
  while( pRight && pLeft ){
    if( pRight->x <= pLeft->x ){
      pLast->pNext = pRight;
      pLast = pRight;
      pRight = pRight->pNext;
    }else{
      pLast->pNext = pLeft;
      pLast = pLeft;
      pLeft = pLeft->pNext;
    }
  }
  pLast->pNext = pRight ? pRight : pLeft;
  return head.pNext;
}

/*
** Sort an array of nEvent event objects into a list.
*/
static GeoEvent *geopolySortEventsByX(GeoEvent *aEvent, int nEvent){
  int mx = 0;
  int i, j;
  GeoEvent *p;
  GeoEvent *a[50];
  for(i=0; i<nEvent; i++){
    p = &aEvent[i];
    p->pNext = 0;
    for(j=0; j<mx && a[j]; j++){
      p = geopolyEventMerge(a[j], p);
      a[j] = 0;
    }
    a[j] = p;
    if( j>=mx ) mx = j+1;
  }
  p = 0;
  for(i=0; i<mx; i++){
    p = geopolyEventMerge(a[i], p);
  }
  return p;
}

/*
** Merge two lists of sorted segments by Y, and then by C.
*/
static GeoSegment *geopolySegmentMerge(GeoSegment *pLeft, GeoSegment *pRight){
  GeoSegment head, *pLast;
  head.pNext = 0;
  pLast = &head;
  while( pRight && pLeft ){
    double r = pRight->y - pLeft->y;
    if( r==0.0 ) r = pRight->C - pLeft->C;
    if( r<0.0 ){
      pLast->pNext = pRight;
      pLast = pRight;
      pRight = pRight->pNext;
    }else{
      pLast->pNext = pLeft;
      pLast = pLeft;
      pLeft = pLeft->pNext;
    }
  }
  pLast->pNext = pRight ? pRight : pLeft;
  return head.pNext;
}

/*
** Sort a list of GeoSegments in order of increasing Y and in the event of
** a tie, increasing C (slope).
*/
static GeoSegment *geopolySortSegmentsByYAndC(GeoSegment *pList){
  int mx = 0;
  int i;
  GeoSegment *p;
  GeoSegment *a[50];
  while( pList ){
    p = pList;
    pList = pList->pNext;
    p->pNext = 0;
    for(i=0; i<mx && a[i]; i++){
      p = geopolySegmentMerge(a[i], p);
      a[i] = 0;
    }
    a[i] = p;
    if( i>=mx ) mx = i+1;
  }
  p = 0;
  for(i=0; i<mx; i++){
    p = geopolySegmentMerge(a[i], p);
  }
  return p;
}

/*
** Determine the overlap between two polygons
*/
static int geopolyOverlap(GeoPoly *p1, GeoPoly *p2){
  sqlite3_int64 nVertex = p1->nVertex + p2->nVertex + 2;
  GeoOverlap *p;
  sqlite3_int64 nByte;
  GeoEvent *pThisEvent;
  double rX;
  int rc = 0;
  int needSort = 0;
  GeoSegment *pActive = 0;
  GeoSegment *pSeg;
  unsigned char aOverlap[4];

  nByte = sizeof(GeoEvent)*nVertex*2
           + sizeof(GeoSegment)*nVertex
           + sizeof(GeoOverlap);
  p = sqlite3_malloc64( nByte );
  if( p==0 ) return -1;
  p->aEvent = (GeoEvent*)&p[1];
  p->aSegment = (GeoSegment*)&p->aEvent[nVertex*2];
  p->nEvent = p->nSegment = 0;
  geopolyAddSegments(p, p1, 1);
  geopolyAddSegments(p, p2, 2);
  pThisEvent = geopolySortEventsByX(p->aEvent, p->nEvent);
  rX = pThisEvent && pThisEvent->x==0.0 ? -1.0 : 0.0;
  memset(aOverlap, 0, sizeof(aOverlap));
  while( pThisEvent ){
    if( pThisEvent->x!=rX ){
      GeoSegment *pPrev = 0;
      int iMask = 0;
      GEODEBUG(("Distinct X: %g\n", pThisEvent->x));
      rX = pThisEvent->x;
      if( needSort ){
        GEODEBUG(("SORT\n"));
        pActive = geopolySortSegmentsByYAndC(pActive);
        needSort = 0;
      }
      for(pSeg=pActive; pSeg; pSeg=pSeg->pNext){
        if( pPrev ){
          if( pPrev->y!=pSeg->y ){
            GEODEBUG(("MASK: %d\n", iMask));
            aOverlap[iMask] = 1;
          }
        }
        iMask ^= pSeg->side;
        pPrev = pSeg;
      }
      pPrev = 0;
      for(pSeg=pActive; pSeg; pSeg=pSeg->pNext){
        double y = pSeg->C*rX + pSeg->B;
        GEODEBUG(("Segment %d.%d %g->%g\n", pSeg->side, pSeg->idx, pSeg->y, y));
        pSeg->y = y;
        if( pPrev ){
          if( pPrev->y>pSeg->y && pPrev->side!=pSeg->side ){
            rc = 1;
            GEODEBUG(("Crossing: %d.%d and %d.%d\n",
                    pPrev->side, pPrev->idx,
                    pSeg->side, pSeg->idx));
            goto geopolyOverlapDone;
          }else if( pPrev->y!=pSeg->y ){
            GEODEBUG(("MASK: %d\n", iMask));
            aOverlap[iMask] = 1;
          }
        }
        iMask ^= pSeg->side;
        pPrev = pSeg;
      }
    }
    GEODEBUG(("%s %d.%d C=%g B=%g\n",
      pThisEvent->eType ? "RM " : "ADD",
      pThisEvent->pSeg->side, pThisEvent->pSeg->idx,
      pThisEvent->pSeg->C,
      pThisEvent->pSeg->B));
    if( pThisEvent->eType==0 ){
      /* Add a segment */
      pSeg = pThisEvent->pSeg;
      pSeg->y = pSeg->y0;
      pSeg->pNext = pActive;
      pActive = pSeg;
      needSort = 1;
    }else{
      /* Remove a segment */
      if( pActive==pThisEvent->pSeg ){
        pActive = ALWAYS(pActive) ? pActive->pNext : 0;
      }else{
        for(pSeg=pActive; pSeg; pSeg=pSeg->pNext){
          if( pSeg->pNext==pThisEvent->pSeg ){
            pSeg->pNext = ALWAYS(pSeg->pNext) ? pSeg->pNext->pNext : 0;
            break;
          }
        }
      }
    }
    pThisEvent = pThisEvent->pNext;
  }
  if( aOverlap[3]==0 ){
    rc = 0;
  }else if( aOverlap[1]!=0 && aOverlap[2]==0 ){
    rc = 3;
  }else if( aOverlap[1]==0 && aOverlap[2]!=0 ){
    rc = 2;
  }else if( aOverlap[1]==0 && aOverlap[2]==0 ){
    rc = 4;
  }else{
    rc = 1;
  }

geopolyOverlapDone:
  sqlite3_free(p);
  return rc;
}

/*
** SQL function:    geopoly_overlap(P1,P2)
**
** Determine whether or not P1 and P2 overlap. Return value:
**
**   0     The two polygons are disjoint
**   1     They overlap
**   2     P1 is completely contained within P2
**   3     P2 is completely contained within P1
**   4     P1 and P2 are the same polygon
**   NULL  Either P1 or P2 or both are not valid polygons
*/
static void geopolyOverlapFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  GeoPoly *p1 = geopolyFuncParam(context, argv[0], 0);
  GeoPoly *p2 = geopolyFuncParam(context, argv[1], 0);
  (void)argc;
  if( p1 && p2 ){
    int x = geopolyOverlap(p1, p2);
    if( x<0 ){
      sqlite3_result_error_nomem(context);
    }else{
      sqlite3_result_int(context, x);
    }
  }
  sqlite3_free(p1);
  sqlite3_free(p2);
}

/*
** Enable or disable debugging output
*/
static void geopolyDebugFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  (void)context;
  (void)argc;
#ifdef GEOPOLY_ENABLE_DEBUG
  geo_debug = sqlite3_value_int(argv[0]);
#else
  (void)argv;
#endif
}

/*
** This function is the implementation of both the xConnect and xCreate
** methods of the geopoly virtual table.
**
**   argv[0]   -> module name
**   argv[1]   -> database name
**   argv[2]   -> table name
**   argv[...] -> column names...
*/
static int geopolyInit(
  sqlite3 *db,                        /* Database connection */
  void *pAux,                         /* One of the RTREE_COORD_* constants */
  int argc, const char *const*argv,   /* Parameters to CREATE TABLE statement */
  sqlite3_vtab **ppVtab,              /* OUT: New virtual table */
  char **pzErr,                       /* OUT: Error message, if any */
  int isCreate                        /* True for xCreate, false for xConnect */
){
  int rc = SQLITE_OK;
  Rtree *pRtree;
  sqlite3_int64 nDb;              /* Length of string argv[1] */
  sqlite3_int64 nName;            /* Length of string argv[2] */
  sqlite3_str *pSql;
  char *zSql;
  int ii;
  (void)pAux;

  sqlite3_vtab_config(db, SQLITE_VTAB_CONSTRAINT_SUPPORT, 1);
  sqlite3_vtab_config(db, SQLITE_VTAB_INNOCUOUS);

  /* Allocate the sqlite3_vtab structure */
  nDb = strlen(argv[1]);
  nName = strlen(argv[2]);
  pRtree = (Rtree *)sqlite3_malloc64(sizeof(Rtree)+nDb+nName*2+8);
  if( !pRtree ){
    return SQLITE_NOMEM;
  }
  memset(pRtree, 0, sizeof(Rtree)+nDb+nName*2+8);
  pRtree->nBusy = 1;
  pRtree->base.pModule = &rtreeModule;
  pRtree->zDb = (char *)&pRtree[1];
  pRtree->zName = &pRtree->zDb[nDb+1];
  pRtree->zNodeName = &pRtree->zName[nName+1];
  pRtree->eCoordType = RTREE_COORD_REAL32;
  pRtree->nDim = 2;
  pRtree->nDim2 = 4;
  memcpy(pRtree->zDb, argv[1], nDb);
  memcpy(pRtree->zName, argv[2], nName);
  memcpy(pRtree->zNodeName, argv[2], nName);
  memcpy(&pRtree->zNodeName[nName], "_node", 6);


  /* Create/Connect to the underlying relational database schema. If
  ** that is successful, call sqlite3_declare_vtab() to configure
  ** the r-tree table schema.
  */
  pSql = sqlite3_str_new(db);
  sqlite3_str_appendf(pSql, "CREATE TABLE x(_shape");
  pRtree->nAux = 1;         /* Add one for _shape */
  pRtree->nAuxNotNull = 1;  /* The _shape column is always not-null */
  for(ii=3; ii<argc; ii++){
    pRtree->nAux++;
    sqlite3_str_appendf(pSql, ",%s", argv[ii]);
  }
  sqlite3_str_appendf(pSql, ");");
  zSql = sqlite3_str_finish(pSql);
  if( !zSql ){
    rc = SQLITE_NOMEM;
  }else if( SQLITE_OK!=(rc = sqlite3_declare_vtab(db, zSql)) ){
    *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
  }
  sqlite3_free(zSql);
  if( rc ) goto geopolyInit_fail;
  pRtree->nBytesPerCell = 8 + pRtree->nDim2*4;

  /* Figure out the node size to use. */
  rc = getNodeSize(db, pRtree, isCreate, pzErr);
  if( rc ) goto geopolyInit_fail;
  rc = rtreeSqlInit(pRtree, db, argv[1], argv[2], isCreate);
  if( rc ){
    *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    goto geopolyInit_fail;
  }

  *ppVtab = (sqlite3_vtab *)pRtree;
  return SQLITE_OK;

geopolyInit_fail:
  if( rc==SQLITE_OK ) rc = SQLITE_ERROR;
  assert( *ppVtab==0 );
  assert( pRtree->nBusy==1 );
  rtreeRelease(pRtree);
  return rc;
}


/*
** GEOPOLY virtual table module xCreate method.
*/
static int geopolyCreate(
  sqlite3 *db,
  void *pAux,
  int argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  return geopolyInit(db, pAux, argc, argv, ppVtab, pzErr, 1);
}

/*
** GEOPOLY virtual table module xConnect method.
*/
static int geopolyConnect(
  sqlite3 *db,
  void *pAux,
  int argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  return geopolyInit(db, pAux, argc, argv, ppVtab, pzErr, 0);
}


/*
** GEOPOLY virtual table module xFilter method.
**
** Query plans:
**
**      1         rowid lookup
**      2         search for objects overlapping the same bounding box
**                that contains polygon argv[0]
**      3         search for objects overlapping the same bounding box
**                that contains polygon argv[0]
**      4         full table scan
*/
static int geopolyFilter(
  sqlite3_vtab_cursor *pVtabCursor,     /* The cursor to initialize */
  int idxNum,                           /* Query plan */
  const char *idxStr,                   /* Not Used */
  int argc, sqlite3_value **argv        /* Parameters to the query plan */
){
  Rtree *pRtree = (Rtree *)pVtabCursor->pVtab;
  RtreeCursor *pCsr = (RtreeCursor *)pVtabCursor;
  RtreeNode *pRoot = 0;
  int rc = SQLITE_OK;
  int iCell = 0;
  (void)idxStr;

  rtreeReference(pRtree);

  /* Reset the cursor to the same state as rtreeOpen() leaves it in. */
  resetCursor(pCsr);

  pCsr->iStrategy = idxNum;
  if( idxNum==1 ){
    /* Special case - lookup by rowid. */
    RtreeNode *pLeaf;        /* Leaf on which the required cell resides */
    RtreeSearchPoint *p;     /* Search point for the leaf */
    i64 iRowid = sqlite3_value_int64(argv[0]);
    i64 iNode = 0;
    rc = findLeafNode(pRtree, iRowid, &pLeaf, &iNode);
    if( rc==SQLITE_OK && pLeaf!=0 ){
      p = rtreeSearchPointNew(pCsr, RTREE_ZERO, 0);
      assert( p!=0 );  /* Always returns pCsr->sPoint */
      pCsr->aNode[0] = pLeaf;
      p->id = iNode;
      p->eWithin = PARTLY_WITHIN;
      rc = nodeRowidIndex(pRtree, pLeaf, iRowid, &iCell);
      p->iCell = (u8)iCell;
      RTREE_QUEUE_TRACE(pCsr, "PUSH-F1:");
    }else{
      pCsr->atEOF = 1;
    }
  }else{
    /* Normal case - r-tree scan. Set up the RtreeCursor.aConstraint array
    ** with the configured constraints.
    */
    rc = nodeAcquire(pRtree, 1, 0, &pRoot);
    if( rc==SQLITE_OK && idxNum<=3 ){
      RtreeCoord bbox[4];
      RtreeConstraint *p;
      assert( argc==1 );
      assert( argv[0]!=0 );
      geopolyBBox(0, argv[0], bbox, &rc);
      if( rc ){
        goto geopoly_filter_end;
      }
      pCsr->aConstraint = p = sqlite3_malloc(sizeof(RtreeConstraint)*4);
      pCsr->nConstraint = 4;
      if( p==0 ){
        rc = SQLITE_NOMEM;
      }else{
        memset(pCsr->aConstraint, 0, sizeof(RtreeConstraint)*4);
        memset(pCsr->anQueue, 0, sizeof(u32)*(pRtree->iDepth + 1));
        if( idxNum==2 ){
          /* Overlap query */
          p->op = 'B';
          p->iCoord = 0;
          p->u.rValue = bbox[1].f;
          p++;
          p->op = 'D';
          p->iCoord = 1;
          p->u.rValue = bbox[0].f;
          p++;
          p->op = 'B';
          p->iCoord = 2;
          p->u.rValue = bbox[3].f;
          p++;
          p->op = 'D';
          p->iCoord = 3;
          p->u.rValue = bbox[2].f;
        }else{
          /* Within query */
          p->op = 'D';
          p->iCoord = 0;
          p->u.rValue = bbox[0].f;
          p++;
          p->op = 'B';
          p->iCoord = 1;
          p->u.rValue = bbox[1].f;
          p++;
          p->op = 'D';
          p->iCoord = 2;
          p->u.rValue = bbox[2].f;
          p++;
          p->op = 'B';
          p->iCoord = 3;
          p->u.rValue = bbox[3].f;
        }
      }
    }
    if( rc==SQLITE_OK ){
      RtreeSearchPoint *pNew;
      pNew = rtreeSearchPointNew(pCsr, RTREE_ZERO, (u8)(pRtree->iDepth+1));
      if( pNew==0 ){
        rc = SQLITE_NOMEM;
        goto geopoly_filter_end;
      }
      pNew->id = 1;
      pNew->iCell = 0;
      pNew->eWithin = PARTLY_WITHIN;
      assert( pCsr->bPoint==1 );
      pCsr->aNode[0] = pRoot;
      pRoot = 0;
      RTREE_QUEUE_TRACE(pCsr, "PUSH-Fm:");
      rc = rtreeStepToLeaf(pCsr);
    }
  }

geopoly_filter_end:
  nodeRelease(pRtree, pRoot);
  rtreeRelease(pRtree);
  return rc;
}

/*
** Rtree virtual table module xBestIndex method. There are three
** table scan strategies to choose from (in order from most to
** least desirable):
**
**   idxNum     idxStr        Strategy
**   ------------------------------------------------
**     1        "rowid"       Direct lookup by rowid.
**     2        "rtree"       R-tree overlap query using geopoly_overlap()
**     3        "rtree"       R-tree within query using geopoly_within()
**     4        "fullscan"    full-table scan.
**   ------------------------------------------------
*/
static int geopolyBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  int ii;
  int iRowidTerm = -1;
  int iFuncTerm = -1;
  int idxNum = 0;
  (void)tab;

  for(ii=0; ii<pIdxInfo->nConstraint; ii++){
    struct sqlite3_index_constraint *p = &pIdxInfo->aConstraint[ii];
    if( !p->usable ) continue;
    if( p->iColumn<0 && p->op==SQLITE_INDEX_CONSTRAINT_EQ  ){
      iRowidTerm = ii;
      break;
    }
    if( p->iColumn==0 && p->op>=SQLITE_INDEX_CONSTRAINT_FUNCTION ){
      /* p->op==SQLITE_INDEX_CONSTRAINT_FUNCTION for geopoly_overlap()
      ** p->op==(SQLITE_INDEX_CONTRAINT_FUNCTION+1) for geopoly_within().
      ** See geopolyFindFunction() */
      iFuncTerm = ii;
      idxNum = p->op - SQLITE_INDEX_CONSTRAINT_FUNCTION + 2;
    }
  }

  if( iRowidTerm>=0 ){
    pIdxInfo->idxNum = 1;
    pIdxInfo->idxStr = "rowid";
    pIdxInfo->aConstraintUsage[iRowidTerm].argvIndex = 1;
    pIdxInfo->aConstraintUsage[iRowidTerm].omit = 1;
    pIdxInfo->estimatedCost = 30.0;
    pIdxInfo->estimatedRows = 1;
    pIdxInfo->idxFlags = SQLITE_INDEX_SCAN_UNIQUE;
    return SQLITE_OK;
  }
  if( iFuncTerm>=0 ){
    pIdxInfo->idxNum = idxNum;
    pIdxInfo->idxStr = "rtree";
    pIdxInfo->aConstraintUsage[iFuncTerm].argvIndex = 1;
    pIdxInfo->aConstraintUsage[iFuncTerm].omit = 0;
    pIdxInfo->estimatedCost = 300.0;
    pIdxInfo->estimatedRows = 10;
    return SQLITE_OK;
  }
  pIdxInfo->idxNum = 4;
  pIdxInfo->idxStr = "fullscan";
  pIdxInfo->estimatedCost = 3000000.0;
  pIdxInfo->estimatedRows = 100000;
  return SQLITE_OK;
}


/*
** GEOPOLY virtual table module xColumn method.
*/
static int geopolyColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, int i){
  Rtree *pRtree = (Rtree *)cur->pVtab;
  RtreeCursor *pCsr = (RtreeCursor *)cur;
  RtreeSearchPoint *p = rtreeSearchPointFirst(pCsr);
  int rc = SQLITE_OK;
  RtreeNode *pNode = rtreeNodeOfFirstSearchPoint(pCsr, &rc);

  if( rc ) return rc;
  if( p==0 ) return SQLITE_OK;
  if( i==0 && sqlite3_vtab_nochange(ctx) ) return SQLITE_OK;
  if( i<=pRtree->nAux ){
    if( !pCsr->bAuxValid ){
      if( pCsr->pReadAux==0 ){
        rc = sqlite3_prepare_v3(pRtree->db, pRtree->zReadAuxSql, -1, 0,
                                &pCsr->pReadAux, 0);
        if( rc ) return rc;
      }
      sqlite3_bind_int64(pCsr->pReadAux, 1,
          nodeGetRowid(pRtree, pNode, p->iCell));
      rc = sqlite3_step(pCsr->pReadAux);
      if( rc==SQLITE_ROW ){
        pCsr->bAuxValid = 1;
      }else{
        sqlite3_reset(pCsr->pReadAux);
        if( rc==SQLITE_DONE ) rc = SQLITE_OK;
        return rc;
      }
    }
    sqlite3_result_value(ctx, sqlite3_column_value(pCsr->pReadAux, i+2));
  }
  return SQLITE_OK;
}


/*
** The xUpdate method for GEOPOLY module virtual tables.
**
** For DELETE:
**
**     argv[0] = the rowid to be deleted
**
** For INSERT:
**
**     argv[0] = SQL NULL
**     argv[1] = rowid to insert, or an SQL NULL to select automatically
**     argv[2] = _shape column
**     argv[3] = first application-defined column....
**
** For UPDATE:
**
**     argv[0] = rowid to modify.  Never NULL
**     argv[1] = rowid after the change.  Never NULL
**     argv[2] = new value for _shape
**     argv[3] = new value for first application-defined column....
*/
static int geopolyUpdate(
  sqlite3_vtab *pVtab,
  int nData,
  sqlite3_value **aData,
  sqlite_int64 *pRowid
){
  Rtree *pRtree = (Rtree *)pVtab;
  int rc = SQLITE_OK;
  RtreeCell cell;                 /* New cell to insert if nData>1 */
  i64 oldRowid;                   /* The old rowid */
  int oldRowidValid;              /* True if oldRowid is valid */
  i64 newRowid;                   /* The new rowid */
  int newRowidValid;              /* True if newRowid is valid */
  int coordChange = 0;            /* Change in coordinates */

  if( pRtree->nNodeRef ){
    /* Unable to write to the btree while another cursor is reading from it,
    ** since the write might do a rebalance which would disrupt the read
    ** cursor. */
    return SQLITE_LOCKED_VTAB;
  }
  rtreeReference(pRtree);
  assert(nData>=1);

  oldRowidValid = sqlite3_value_type(aData[0])!=SQLITE_NULL;;
  oldRowid = oldRowidValid ? sqlite3_value_int64(aData[0]) : 0;
  newRowidValid = nData>1 && sqlite3_value_type(aData[1])!=SQLITE_NULL;
  newRowid = newRowidValid ? sqlite3_value_int64(aData[1]) : 0;
  cell.iRowid = newRowid;

  if( nData>1                                 /* not a DELETE */
   && (!oldRowidValid                         /* INSERT */
        || !sqlite3_value_nochange(aData[2])  /* UPDATE _shape */
        || oldRowid!=newRowid)                /* Rowid change */
  ){
    assert( aData[2]!=0 );
    geopolyBBox(0, aData[2], cell.aCoord, &rc);
    if( rc ){
      if( rc==SQLITE_ERROR ){
        pVtab->zErrMsg =
          sqlite3_mprintf("_shape does not contain a valid polygon");
      }
      goto geopoly_update_end;
    }
    coordChange = 1;

    /* If a rowid value was supplied, check if it is already present in
    ** the table. If so, the constraint has failed. */
    if( newRowidValid && (!oldRowidValid || oldRowid!=newRowid) ){
      int steprc;
      sqlite3_bind_int64(pRtree->pReadRowid, 1, cell.iRowid);
      steprc = sqlite3_step(pRtree->pReadRowid);
      rc = sqlite3_reset(pRtree->pReadRowid);
      if( SQLITE_ROW==steprc ){
        if( sqlite3_vtab_on_conflict(pRtree->db)==SQLITE_REPLACE ){
          rc = rtreeDeleteRowid(pRtree, cell.iRowid);
        }else{
          rc = rtreeConstraintError(pRtree, 0);
        }
      }
    }
  }

  /* If aData[0] is not an SQL NULL value, it is the rowid of a
  ** record to delete from the r-tree table. The following block does
  ** just that.
  */
  if( rc==SQLITE_OK && (nData==1 || (coordChange && oldRowidValid)) ){
    rc = rtreeDeleteRowid(pRtree, oldRowid);
  }

  /* If the aData[] array contains more than one element, elements
  ** (aData[2]..aData[argc-1]) contain a new record to insert into
  ** the r-tree structure.
  */
  if( rc==SQLITE_OK && nData>1 && coordChange ){
    /* Insert the new record into the r-tree */
    RtreeNode *pLeaf = 0;
    if( !newRowidValid ){
      rc = rtreeNewRowid(pRtree, &cell.iRowid);
    }
    *pRowid = cell.iRowid;
    if( rc==SQLITE_OK ){
      rc = ChooseLeaf(pRtree, &cell, 0, &pLeaf);
    }
    if( rc==SQLITE_OK ){
      int rc2;
      rc = rtreeInsertCell(pRtree, pLeaf, &cell, 0);
      rc2 = nodeRelease(pRtree, pLeaf);
      if( rc==SQLITE_OK ){
        rc = rc2;
      }
    }
  }

  /* Change the data */
  if( rc==SQLITE_OK && nData>1 ){
    sqlite3_stmt *pUp = pRtree->pWriteAux;
    int jj;
    int nChange = 0;
    sqlite3_bind_int64(pUp, 1, cell.iRowid);
    assert( pRtree->nAux>=1 );
    if( sqlite3_value_nochange(aData[2]) ){
      sqlite3_bind_null(pUp, 2);
    }else{
      GeoPoly *p = 0;
      if( sqlite3_value_type(aData[2])==SQLITE_TEXT
       && (p = geopolyFuncParam(0, aData[2], &rc))!=0
       && rc==SQLITE_OK
      ){
        sqlite3_bind_blob(pUp, 2, p->hdr, 4+8*p->nVertex, SQLITE_TRANSIENT);
      }else{
        sqlite3_bind_value(pUp, 2, aData[2]);
      }
      sqlite3_free(p);
      nChange = 1;
    }
    for(jj=1; jj<nData-2; jj++){
      nChange++;
      sqlite3_bind_value(pUp, jj+2, aData[jj+2]);
    }
    if( nChange ){
      sqlite3_step(pUp);
      rc = sqlite3_reset(pUp);
    }
  }

geopoly_update_end:
  rtreeRelease(pRtree);
  return rc;
}

/*
** Report that geopoly_overlap() is an overloaded function suitable
** for use in xBestIndex.
*/
static int geopolyFindFunction(
  sqlite3_vtab *pVtab,
  int nArg,
  const char *zName,
  void (**pxFunc)(sqlite3_context*,int,sqlite3_value**),
  void **ppArg
){
  (void)pVtab;
  (void)nArg;
  if( sqlite3_stricmp(zName, "geopoly_overlap")==0 ){
    *pxFunc = geopolyOverlapFunc;
    *ppArg = 0;
    return SQLITE_INDEX_CONSTRAINT_FUNCTION;
  }
  if( sqlite3_stricmp(zName, "geopoly_within")==0 ){
    *pxFunc = geopolyWithinFunc;
    *ppArg = 0;
    return SQLITE_INDEX_CONSTRAINT_FUNCTION+1;
  }
  return 0;
}


static sqlite3_module geopolyModule = {
  3,                          /* iVersion */
  geopolyCreate,              /* xCreate - create a table */
  geopolyConnect,             /* xConnect - connect to an existing table */
  geopolyBestIndex,           /* xBestIndex - Determine search strategy */
  rtreeDisconnect,            /* xDisconnect - Disconnect from a table */
  rtreeDestroy,               /* xDestroy - Drop a table */
  rtreeOpen,                  /* xOpen - open a cursor */
  rtreeClose,                 /* xClose - close a cursor */
  geopolyFilter,              /* xFilter - configure scan constraints */
  rtreeNext,                  /* xNext - advance a cursor */
  rtreeEof,                   /* xEof */
  geopolyColumn,              /* xColumn - read data */
  rtreeRowid,                 /* xRowid - read data */
  geopolyUpdate,              /* xUpdate - write data */
  rtreeBeginTransaction,      /* xBegin - begin transaction */
  rtreeEndTransaction,        /* xSync - sync transaction */
  rtreeEndTransaction,        /* xCommit - commit transaction */
  rtreeEndTransaction,        /* xRollback - rollback transaction */
  geopolyFindFunction,        /* xFindFunction - function overloading */
  rtreeRename,                /* xRename - rename the table */
  rtreeSavepoint,             /* xSavepoint */
  0,                          /* xRelease */
  0,                          /* xRollbackTo */
  rtreeShadowName,            /* xShadowName */
  rtreeIntegrity              /* xIntegrity */
};

static int sqlite3_geopoly_init(sqlite3 *db){
  int rc = SQLITE_OK;
  static const struct {
    void (*xFunc)(sqlite3_context*,int,sqlite3_value**);
    signed char nArg;
    unsigned char bPure;
    const char *zName;
  } aFunc[] = {
     { geopolyAreaFunc,          1, 1,    "geopoly_area"             },
     { geopolyBlobFunc,          1, 1,    "geopoly_blob"             },
     { geopolyJsonFunc,          1, 1,    "geopoly_json"             },
     { geopolySvgFunc,          -1, 1,    "geopoly_svg"              },
     { geopolyWithinFunc,        2, 1,    "geopoly_within"           },
     { geopolyContainsPointFunc, 3, 1,    "geopoly_contains_point"   },
     { geopolyOverlapFunc,       2, 1,    "geopoly_overlap"          },
     { geopolyDebugFunc,         1, 0,    "geopoly_debug"            },
     { geopolyBBoxFunc,          1, 1,    "geopoly_bbox"             },
     { geopolyXformFunc,         7, 1,    "geopoly_xform"            },
     { geopolyRegularFunc,       4, 1,    "geopoly_regular"          },
     { geopolyCcwFunc,           1, 1,    "geopoly_ccw"              },
  };
  static const struct {
    void (*xStep)(sqlite3_context*,int,sqlite3_value**);
    void (*xFinal)(sqlite3_context*);
    const char *zName;
  } aAgg[] = {
     { geopolyBBoxStep, geopolyBBoxFinal, "geopoly_group_bbox"    },
  };
  unsigned int i;
  for(i=0; i<sizeof(aFunc)/sizeof(aFunc[0]) && rc==SQLITE_OK; i++){
    int enc;
    if( aFunc[i].bPure ){
      enc = SQLITE_UTF8|SQLITE_DETERMINISTIC|SQLITE_INNOCUOUS;
    }else{
      enc = SQLITE_UTF8|SQLITE_DIRECTONLY;
    }
    rc = sqlite3_create_function(db, aFunc[i].zName, aFunc[i].nArg,
                                 enc, 0,
                                 aFunc[i].xFunc, 0, 0);
  }
  for(i=0; i<sizeof(aAgg)/sizeof(aAgg[0]) && rc==SQLITE_OK; i++){
    rc = sqlite3_create_function(db, aAgg[i].zName, 1,
              SQLITE_UTF8|SQLITE_DETERMINISTIC|SQLITE_INNOCUOUS, 0,
              0, aAgg[i].xStep, aAgg[i].xFinal);
  }
  if( rc==SQLITE_OK ){
    rc = sqlite3_create_module_v2(db, "geopoly", &geopolyModule, 0, 0);
  }
  return rc;
}

/************** End of geopoly.c *********************************************/
/************** Continuing where we left off in rtree.c **********************/
#endif

/*
** Register the r-tree module with database handle db. This creates the
** virtual table module "rtree" and the debugging/analysis scalar
** function "rtreenode".
*/
SQLITE_PRIVATE int sqlite3RtreeInit(sqlite3 *db){
  const int utf8 = SQLITE_UTF8;
  int rc;

  rc = sqlite3_create_function(db, "rtreenode", 2, utf8, 0, rtreenode, 0, 0);
  if( rc==SQLITE_OK ){
    rc = sqlite3_create_function(db, "rtreedepth", 1, utf8, 0,rtreedepth, 0, 0);
  }
  if( rc==SQLITE_OK ){
    rc = sqlite3_create_function(db, "rtreecheck", -1, utf8, 0,rtreecheck, 0,0);
  }
  if( rc==SQLITE_OK ){
#ifdef SQLITE_RTREE_INT_ONLY
    void *c = (void *)RTREE_COORD_INT32;
#else
    void *c = (void *)RTREE_COORD_REAL32;
#endif
    rc = sqlite3_create_module_v2(db, "rtree", &rtreeModule, c, 0);
  }
  if( rc==SQLITE_OK ){
    void *c = (void *)RTREE_COORD_INT32;
    rc = sqlite3_create_module_v2(db, "rtree_i32", &rtreeModule, c, 0);
  }
#ifdef SQLITE_ENABLE_GEOPOLY
  if( rc==SQLITE_OK ){
    rc = sqlite3_geopoly_init(db);
  }
#endif

  return rc;
}

/*
** This routine deletes the RtreeGeomCallback object that was attached
** one of the SQL functions create by sqlite3_rtree_geometry_callback()
** or sqlite3_rtree_query_callback().  In other words, this routine is the
** destructor for an RtreeGeomCallback objecct.  This routine is called when
** the corresponding SQL function is deleted.
*/
static void rtreeFreeCallback(void *p){
  RtreeGeomCallback *pInfo = (RtreeGeomCallback*)p;
  if( pInfo->xDestructor ) pInfo->xDestructor(pInfo->pContext);
  sqlite3_free(p);
}

/*
** This routine frees the BLOB that is returned by geomCallback().
*/
static void rtreeMatchArgFree(void *pArg){
  int i;
  RtreeMatchArg *p = (RtreeMatchArg*)pArg;
  for(i=0; i<p->nParam; i++){
    sqlite3_value_free(p->apSqlParam[i]);
  }
  sqlite3_free(p);
}

/*
** Each call to sqlite3_rtree_geometry_callback() or
** sqlite3_rtree_query_callback() creates an ordinary SQLite
** scalar function that is implemented by this routine.
**
** All this function does is construct an RtreeMatchArg object that
** contains the geometry-checking callback routines and a list of
** parameters to this function, then return that RtreeMatchArg object
** as a BLOB.
**
** The R-Tree MATCH operator will read the returned BLOB, deserialize
** the RtreeMatchArg object, and use the RtreeMatchArg object to figure
** out which elements of the R-Tree should be returned by the query.
*/
static void geomCallback(sqlite3_context *ctx, int nArg, sqlite3_value **aArg){
  RtreeGeomCallback *pGeomCtx = (RtreeGeomCallback *)sqlite3_user_data(ctx);
  RtreeMatchArg *pBlob;
  sqlite3_int64 nBlob;
  int memErr = 0;

  nBlob = SZ_RTREEMATCHARG(nArg) + nArg*sizeof(sqlite3_value*);
  pBlob = (RtreeMatchArg *)sqlite3_malloc64(nBlob);
  if( !pBlob ){
    sqlite3_result_error_nomem(ctx);
  }else{
    int i;
    pBlob->iSize = nBlob;
    pBlob->cb = pGeomCtx[0];
    pBlob->apSqlParam = (sqlite3_value**)&pBlob->aParam[nArg];
    pBlob->nParam = nArg;
    for(i=0; i<nArg; i++){
      pBlob->apSqlParam[i] = sqlite3_value_dup(aArg[i]);
      if( pBlob->apSqlParam[i]==0 ) memErr = 1;
#ifdef SQLITE_RTREE_INT_ONLY
      pBlob->aParam[i] = sqlite3_value_int64(aArg[i]);
#else
      pBlob->aParam[i] = sqlite3_value_double(aArg[i]);
#endif
    }
    if( memErr ){
      sqlite3_result_error_nomem(ctx);
      rtreeMatchArgFree(pBlob);
    }else{
      sqlite3_result_pointer(ctx, pBlob, "RtreeMatchArg", rtreeMatchArgFree);
    }
  }
}

/*
** Register a new geometry function for use with the r-tree MATCH operator.
*/
SQLITE_API int sqlite3_rtree_geometry_callback(
  sqlite3 *db,                  /* Register SQL function on this connection */
  const char *zGeom,            /* Name of the new SQL function */
  int (*xGeom)(sqlite3_rtree_geometry*,int,RtreeDValue*,int*), /* Callback */
  void *pContext                /* Extra data associated with the callback */
){
  RtreeGeomCallback *pGeomCtx;      /* Context object for new user-function */

  /* Allocate and populate the context object. */
  pGeomCtx = (RtreeGeomCallback *)sqlite3_malloc(sizeof(RtreeGeomCallback));
  if( !pGeomCtx ) return SQLITE_NOMEM;
  pGeomCtx->xGeom = xGeom;
  pGeomCtx->xQueryFunc = 0;
  pGeomCtx->xDestructor = 0;
  pGeomCtx->pContext = pContext;
  return sqlite3_create_function_v2(db, zGeom, -1, SQLITE_ANY,
      (void *)pGeomCtx, geomCallback, 0, 0, rtreeFreeCallback
  );
}

/*
** Register a new 2nd-generation geometry function for use with the
** r-tree MATCH operator.
*/
SQLITE_API int sqlite3_rtree_query_callback(
  sqlite3 *db,                 /* Register SQL function on this connection */
  const char *zQueryFunc,      /* Name of new SQL function */
  int (*xQueryFunc)(sqlite3_rtree_query_info*), /* Callback */
  void *pContext,              /* Extra data passed into the callback */
  void (*xDestructor)(void*)   /* Destructor for the extra data */
){
  RtreeGeomCallback *pGeomCtx;      /* Context object for new user-function */

  /* Allocate and populate the context object. */
  pGeomCtx = (RtreeGeomCallback *)sqlite3_malloc(sizeof(RtreeGeomCallback));
  if( !pGeomCtx ){
    if( xDestructor ) xDestructor(pContext);
    return SQLITE_NOMEM;
  }
  pGeomCtx->xGeom = 0;
  pGeomCtx->xQueryFunc = xQueryFunc;
  pGeomCtx->xDestructor = xDestructor;
  pGeomCtx->pContext = pContext;
  return sqlite3_create_function_v2(db, zQueryFunc, -1, SQLITE_ANY,
      (void *)pGeomCtx, geomCallback, 0, 0, rtreeFreeCallback
  );
}

#ifndef SQLITE_CORE
#ifdef _WIN32
__declspec(dllexport)
#endif
SQLITE_API int sqlite3_rtree_init(
  sqlite3 *db,
  char **pzErrMsg,
  const sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi)
  return sqlite3RtreeInit(db);
}
#endif

#endif

/************** End of rtree.c ***********************************************/
/************** Begin file icu.c *********************************************/
/*
** 2007 May 6
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** $Id: icu.c,v 1.7 2007/12/13 21:54:11 drh Exp $
**
** This file implements an integration between the ICU library
** ("International Components for Unicode", an open-source library
** for handling unicode data) and SQLite. The integration uses
** ICU to provide the following to SQLite:
**
**   * An implementation of the SQL regexp() function (and hence REGEXP
**     operator) using the ICU uregex_XX() APIs.
**
**   * Implementations of the SQL scalar upper() and lower() functions
**     for case mapping.
**
**   * Integration of ICU and SQLite collation sequences.
**
**   * An implementation of the LIKE operator that uses ICU to
**     provide case-independent matching.
*/

#if !defined(SQLITE_CORE)                  \
 || defined(SQLITE_ENABLE_ICU)             \
 || defined(SQLITE_ENABLE_ICU_COLLATIONS)

/* Include ICU headers */
#include <unicode/utypes.h>
#include <unicode/uregex.h>
#include <unicode/ustring.h>
#include <unicode/ucol.h>

/* #include <assert.h> */

#ifndef SQLITE_CORE
/*   #include "sqlite3ext.h" */
  SQLITE_EXTENSION_INIT1
#else
/*   #include "sqlite3.h" */
#endif

/*
** This function is called when an ICU function called from within
** the implementation of an SQL scalar function returns an error.
**
** The scalar function context passed as the first argument is
** loaded with an error message based on the following two args.
*/
static void icuFunctionError(
  sqlite3_context *pCtx,       /* SQLite scalar function context */
  const char *zName,           /* Name of ICU function that failed */
  UErrorCode e                 /* Error code returned by ICU function */
){
  char zBuf[128];
  sqlite3_snprintf(128, zBuf, "ICU error: %s(): %s", zName, u_errorName(e));
  zBuf[127] = '\0';
  sqlite3_result_error(pCtx, zBuf, -1);
}

#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_ICU)

/*
** Maximum length (in bytes) of the pattern in a LIKE or GLOB
** operator.
*/
#ifndef SQLITE_MAX_LIKE_PATTERN_LENGTH
# define SQLITE_MAX_LIKE_PATTERN_LENGTH 50000
#endif

/*
** Version of sqlite3_free() that is always a function, never a macro.
*/
static void xFree(void *p){
  sqlite3_free(p);
}

/*
** This lookup table is used to help decode the first byte of
** a multi-byte UTF8 character. It is copied here from SQLite source
** code file utf8.c.
*/
static const unsigned char icuUtf8Trans1[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x00, 0x00,
};

#define SQLITE_ICU_READ_UTF8(zIn, c)                       \
  c = *(zIn++);                                            \
  if( c>=0xc0 ){                                           \
    c = icuUtf8Trans1[c-0xc0];                             \
    while( (*zIn & 0xc0)==0x80 ){                          \
      c = (c<<6) + (0x3f & *(zIn++));                      \
    }                                                      \
  }

#define SQLITE_ICU_SKIP_UTF8(zIn)                          \
  assert( *zIn );                                          \
  if( *(zIn++)>=0xc0 ){                                    \
    while( (*zIn & 0xc0)==0x80 ){zIn++;}                   \
  }


/*
** Compare two UTF-8 strings for equality where the first string is
** a "LIKE" expression. Return true (1) if they are the same and
** false (0) if they are different.
*/
static int icuLikeCompare(
  const uint8_t *zPattern,   /* LIKE pattern */
  const uint8_t *zString,    /* The UTF-8 string to compare against */
  const UChar32 uEsc         /* The escape character */
){
  static const uint32_t MATCH_ONE = (uint32_t)'_';
  static const uint32_t MATCH_ALL = (uint32_t)'%';

  int prevEscape = 0;     /* True if the previous character was uEsc */

  while( 1 ){

    /* Read (and consume) the next character from the input pattern. */
    uint32_t uPattern;
    SQLITE_ICU_READ_UTF8(zPattern, uPattern);
    if( uPattern==0 ) break;

    /* There are now 4 possibilities:
    **
    **     1. uPattern is an unescaped match-all character "%",
    **     2. uPattern is an unescaped match-one character "_",
    **     3. uPattern is an unescaped escape character, or
    **     4. uPattern is to be handled as an ordinary character
    */
    if( uPattern==MATCH_ALL && !prevEscape && uPattern!=(uint32_t)uEsc ){
      /* Case 1. */
      uint8_t c;

      /* Skip any MATCH_ALL or MATCH_ONE characters that follow a
      ** MATCH_ALL. For each MATCH_ONE, skip one character in the
      ** test string.
      */
      while( (c=*zPattern) == MATCH_ALL || c == MATCH_ONE ){
        if( c==MATCH_ONE ){
          if( *zString==0 ) return 0;
          SQLITE_ICU_SKIP_UTF8(zString);
        }
        zPattern++;
      }

      if( *zPattern==0 ) return 1;

      while( *zString ){
        if( icuLikeCompare(zPattern, zString, uEsc) ){
          return 1;
        }
        SQLITE_ICU_SKIP_UTF8(zString);
      }
      return 0;

    }else if( uPattern==MATCH_ONE && !prevEscape && uPattern!=(uint32_t)uEsc ){
      /* Case 2. */
      if( *zString==0 ) return 0;
      SQLITE_ICU_SKIP_UTF8(zString);

    }else if( uPattern==(uint32_t)uEsc && !prevEscape ){
      /* Case 3. */
      prevEscape = 1;

    }else{
      /* Case 4. */
      uint32_t uString;
      SQLITE_ICU_READ_UTF8(zString, uString);
      uString = (uint32_t)u_foldCase((UChar32)uString, U_FOLD_CASE_DEFAULT);
      uPattern = (uint32_t)u_foldCase((UChar32)uPattern, U_FOLD_CASE_DEFAULT);
      if( uString!=uPattern ){
        return 0;
      }
      prevEscape = 0;
    }
  }

  return *zString==0;
}

/*
** Implementation of the like() SQL function.  This function implements
** the build-in LIKE operator.  The first argument to the function is the
** pattern and the second argument is the string.  So, the SQL statements:
**
**       A LIKE B
**
** is implemented as like(B, A). If there is an escape character E,
**
**       A LIKE B ESCAPE E
**
** is mapped to like(B, A, E).
*/
static void icuLikeFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  const unsigned char *zA = sqlite3_value_text(argv[0]);
  const unsigned char *zB = sqlite3_value_text(argv[1]);
  UChar32 uEsc = 0;

  /* Limit the length of the LIKE or GLOB pattern to avoid problems
  ** of deep recursion and N*N behavior in patternCompare().
  */
  if( sqlite3_value_bytes(argv[0])>SQLITE_MAX_LIKE_PATTERN_LENGTH ){
    sqlite3_result_error(context, "LIKE or GLOB pattern too complex", -1);
    return;
  }


  if( argc==3 ){
    /* The escape character string must consist of a single UTF-8 character.
    ** Otherwise, return an error.
    */
    int nE= sqlite3_value_bytes(argv[2]);
    const unsigned char *zE = sqlite3_value_text(argv[2]);
    int i = 0;
    if( zE==0 ) return;
    U8_NEXT(zE, i, nE, uEsc);
    if( i!=nE){
      sqlite3_result_error(context,
          "ESCAPE expression must be a single character", -1);
      return;
    }
  }

  if( zA && zB ){
    sqlite3_result_int(context, icuLikeCompare(zA, zB, uEsc));
  }
}

/*
** Function to delete compiled regexp objects. Registered as
** a destructor function with sqlite3_set_auxdata().
*/
static void icuRegexpDelete(void *p){
  URegularExpression *pExpr = (URegularExpression *)p;
  uregex_close(pExpr);
}

/*
** Implementation of SQLite REGEXP operator. This scalar function takes
** two arguments. The first is a regular expression pattern to compile
** the second is a string to match against that pattern. If either
** argument is an SQL NULL, then NULL Is returned. Otherwise, the result
** is 1 if the string matches the pattern, or 0 otherwise.
**
** SQLite maps the regexp() function to the regexp() operator such
** that the following two are equivalent:
**
**     zString REGEXP zPattern
**     regexp(zPattern, zString)
**
** Uses the following ICU regexp APIs:
**
**     uregex_open()
**     uregex_matches()
**     uregex_close()
*/
static void icuRegexpFunc(sqlite3_context *p, int nArg, sqlite3_value **apArg){
  UErrorCode status = U_ZERO_ERROR;
  URegularExpression *pExpr;
  UBool res;
  const UChar *zString = sqlite3_value_text16(apArg[1]);

  (void)nArg;  /* Unused parameter */

  /* If the left hand side of the regexp operator is NULL,
  ** then the result is also NULL.
  */
  if( !zString ){
    return;
  }

  pExpr = sqlite3_get_auxdata(p, 0);
  if( !pExpr ){
    const UChar *zPattern = sqlite3_value_text16(apArg[0]);
    if( !zPattern ){
      return;
    }
    pExpr = uregex_open(zPattern, -1, 0, 0, &status);

    if( U_SUCCESS(status) ){
      sqlite3_set_auxdata(p, 0, pExpr, icuRegexpDelete);
      pExpr = sqlite3_get_auxdata(p, 0);
    }
    if( !pExpr ){
      icuFunctionError(p, "uregex_open", status);
      return;
    }
  }

  /* Configure the text that the regular expression operates on. */
  uregex_setText(pExpr, zString, -1, &status);
  if( !U_SUCCESS(status) ){
    icuFunctionError(p, "uregex_setText", status);
    return;
  }

  /* Attempt the match */
  res = uregex_matches(pExpr, 0, &status);
  if( !U_SUCCESS(status) ){
    icuFunctionError(p, "uregex_matches", status);
    return;
  }

  /* Set the text that the regular expression operates on to a NULL
  ** pointer. This is not really necessary, but it is tidier than
  ** leaving the regular expression object configured with an invalid
  ** pointer after this function returns.
  */
  uregex_setText(pExpr, 0, 0, &status);

  /* Return 1 or 0. */
  sqlite3_result_int(p, res ? 1 : 0);
}

/*
** Implementations of scalar functions for case mapping - upper() and
** lower(). Function upper() converts its input to upper-case (ABC).
** Function lower() converts to lower-case (abc).
**
** ICU provides two types of case mapping, "general" case mapping and
** "language specific". Refer to ICU documentation for the differences
** between the two.
**
** To utilise "general" case mapping, the upper() or lower() scalar
** functions are invoked with one argument:
**
**     upper('ABC') -> 'abc'
**     lower('abc') -> 'ABC'
**
** To access ICU "language specific" case mapping, upper() or lower()
** should be invoked with two arguments. The second argument is the name
** of the locale to use. Passing an empty string ("") or SQL NULL value
** as the second argument is the same as invoking the 1 argument version
** of upper() or lower().
**
**     lower('I', 'en_us') -> 'i'
**     lower('I', 'tr_tr') -> '\u131' (small dotless i)
**
** http://www.icu-project.org/userguide/posix.html#case_mappings
*/
static void icuCaseFunc16(sqlite3_context *p, int nArg, sqlite3_value **apArg){
  const UChar *zInput;            /* Pointer to input string */
  UChar *zOutput = 0;             /* Pointer to output buffer */
  int nInput;                     /* Size of utf-16 input string in bytes */
  int nOut;                       /* Size of output buffer in bytes */
  int cnt;
  int bToUpper;                   /* True for toupper(), false for tolower() */
  UErrorCode status;
  const char *zLocale = 0;

  assert(nArg==1 || nArg==2);
  bToUpper = (sqlite3_user_data(p)!=0);
  if( nArg==2 ){
    zLocale = (const char *)sqlite3_value_text(apArg[1]);
  }

  zInput = sqlite3_value_text16(apArg[0]);
  if( !zInput ){
    return;
  }
  nOut = nInput = sqlite3_value_bytes16(apArg[0]);
  if( nOut==0 ){
    sqlite3_result_text16(p, "", 0, SQLITE_STATIC);
    return;
  }

  for(cnt=0; cnt<2; cnt++){
    UChar *zNew = sqlite3_realloc(zOutput, nOut);
    if( zNew==0 ){
      sqlite3_free(zOutput);
      sqlite3_result_error_nomem(p);
      return;
    }
    zOutput = zNew;
    status = U_ZERO_ERROR;
    if( bToUpper ){
      nOut = 2*u_strToUpper(zOutput,nOut/2,zInput,nInput/2,zLocale,&status);
    }else{
      nOut = 2*u_strToLower(zOutput,nOut/2,zInput,nInput/2,zLocale,&status);
    }

    if( U_SUCCESS(status) ){
      sqlite3_result_text16(p, zOutput, nOut, xFree);
    }else if( status==U_BUFFER_OVERFLOW_ERROR ){
      assert( cnt==0 );
      continue;
    }else{
      icuFunctionError(p, bToUpper ? "u_strToUpper" : "u_strToLower", status);
    }
    return;
  }
  assert( 0 );     /* Unreachable */
}

#endif /* !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_ICU) */

/*
** Collation sequence destructor function. The pCtx argument points to
** a UCollator structure previously allocated using ucol_open().
*/
static void icuCollationDel(void *pCtx){
  UCollator *p = (UCollator *)pCtx;
  ucol_close(p);
}

/*
** Collation sequence comparison function. The pCtx argument points to
** a UCollator structure previously allocated using ucol_open().
*/
static int icuCollationColl(
  void *pCtx,
  int nLeft,
  const void *zLeft,
  int nRight,
  const void *zRight
){
  UCollationResult res;
  UCollator *p = (UCollator *)pCtx;
  res = ucol_strcoll(p, (UChar *)zLeft, nLeft/2, (UChar *)zRight, nRight/2);
  switch( res ){
    case UCOL_LESS:    return -1;
    case UCOL_GREATER: return +1;
    case UCOL_EQUAL:   return 0;
  }
  assert(!"Unexpected return value from ucol_strcoll()");
  return 0;
}

/*
** Implementation of the scalar function icu_load_collation().
**
** This scalar function is used to add ICU collation based collation
** types to an SQLite database connection. It is intended to be called
** as follows:
**
**     SELECT icu_load_collation(<locale>, <collation-name>);
**
** Where <locale> is a string containing an ICU locale identifier (i.e.
** "en_AU", "tr_TR" etc.) and <collation-name> is the name of the
** collation sequence to create.
*/
static void icuLoadCollation(
  sqlite3_context *p,
  int nArg,
  sqlite3_value **apArg
){
  sqlite3 *db = (sqlite3 *)sqlite3_user_data(p);
  UErrorCode status = U_ZERO_ERROR;
  const char *zLocale;      /* Locale identifier - (eg. "jp_JP") */
  const char *zName;        /* SQL Collation sequence name (eg. "japanese") */
  UCollator *pUCollator;    /* ICU library collation object */
  int rc;                   /* Return code from sqlite3_create_collation_x() */

  assert(nArg==2 || nArg==3);
  (void)nArg; /* Unused parameter */
  zLocale = (const char *)sqlite3_value_text(apArg[0]);
  zName = (const char *)sqlite3_value_text(apArg[1]);

  if( !zLocale || !zName ){
    return;
  }

  pUCollator = ucol_open(zLocale, &status);
  if( !U_SUCCESS(status) ){
    icuFunctionError(p, "ucol_open", status);
    return;
  }
  assert(p);
  if(nArg==3){
    const char *zOption = (const char*)sqlite3_value_text(apArg[2]);
    static const struct {
       const char *zName;
       UColAttributeValue val;
    } aStrength[] = {
      {  "PRIMARY",      UCOL_PRIMARY           },
      {  "SECONDARY",    UCOL_SECONDARY         },
      {  "TERTIARY",     UCOL_TERTIARY          },
      {  "DEFAULT",      UCOL_DEFAULT_STRENGTH  },
      {  "QUARTERNARY",  UCOL_QUATERNARY        },
      {  "IDENTICAL",    UCOL_IDENTICAL         },
    };
    unsigned int i;
    for(i=0; i<sizeof(aStrength)/sizeof(aStrength[0]); i++){
      if( sqlite3_stricmp(zOption,aStrength[i].zName)==0 ){
        ucol_setStrength(pUCollator, aStrength[i].val);
        break;
      }
    }
    if( i>=sizeof(aStrength)/sizeof(aStrength[0]) ){
      sqlite3_str *pStr = sqlite3_str_new(sqlite3_context_db_handle(p));
      sqlite3_str_appendf(pStr,
         "unknown collation strength \"%s\" - should be one of:",
         zOption);
      for(i=0; i<sizeof(aStrength)/sizeof(aStrength[0]); i++){
         sqlite3_str_appendf(pStr, " %s", aStrength[i].zName);
      }
      sqlite3_result_error(p, sqlite3_str_value(pStr), -1);
      sqlite3_free(sqlite3_str_finish(pStr));
      return;
    }
  }
  rc = sqlite3_create_collation_v2(db, zName, SQLITE_UTF16, (void *)pUCollator,
      icuCollationColl, icuCollationDel
  );
  if( rc!=SQLITE_OK ){
    ucol_close(pUCollator);
    sqlite3_result_error(p, "Error registering collation function", -1);
  }
}

/*
** Register the ICU extension functions with database db.
*/
SQLITE_PRIVATE int sqlite3IcuInit(sqlite3 *db){
# define SQLITEICU_EXTRAFLAGS (SQLITE_DETERMINISTIC|SQLITE_INNOCUOUS)
  static const struct IcuScalar {
    const char *zName;                        /* Function name */
    unsigned char nArg;                       /* Number of arguments */
    unsigned int enc;                         /* Optimal text encoding */
    unsigned char iContext;                   /* sqlite3_user_data() context */
    void (*xFunc)(sqlite3_context*,int,sqlite3_value**);
  } scalars[] = {
    {"icu_load_collation",2,SQLITE_UTF8|SQLITE_DIRECTONLY,1, icuLoadCollation},
    {"icu_load_collation",3,SQLITE_UTF8|SQLITE_DIRECTONLY,1, icuLoadCollation},
#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_ICU)
    {"regexp", 2, SQLITE_ANY|SQLITEICU_EXTRAFLAGS,         0, icuRegexpFunc},
    {"lower",  1, SQLITE_UTF16|SQLITEICU_EXTRAFLAGS,       0, icuCaseFunc16},
    {"lower",  2, SQLITE_UTF16|SQLITEICU_EXTRAFLAGS,       0, icuCaseFunc16},
    {"upper",  1, SQLITE_UTF16|SQLITEICU_EXTRAFLAGS,       1, icuCaseFunc16},
    {"upper",  2, SQLITE_UTF16|SQLITEICU_EXTRAFLAGS,       1, icuCaseFunc16},
    {"lower",  1, SQLITE_UTF8|SQLITEICU_EXTRAFLAGS,        0, icuCaseFunc16},
    {"lower",  2, SQLITE_UTF8|SQLITEICU_EXTRAFLAGS,        0, icuCaseFunc16},
    {"upper",  1, SQLITE_UTF8|SQLITEICU_EXTRAFLAGS,        1, icuCaseFunc16},
    {"upper",  2, SQLITE_UTF8|SQLITEICU_EXTRAFLAGS,        1, icuCaseFunc16},
    {"like",   2, SQLITE_UTF8|SQLITEICU_EXTRAFLAGS,        0, icuLikeFunc},
    {"like",   3, SQLITE_UTF8|SQLITEICU_EXTRAFLAGS,        0, icuLikeFunc},
#endif /* !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_ICU) */
  };
  int rc = SQLITE_OK;
  int i;

  for(i=0; rc==SQLITE_OK && i<(int)(sizeof(scalars)/sizeof(scalars[0])); i++){
    const struct IcuScalar *p = &scalars[i];
    rc = sqlite3_create_function(
        db, p->zName, p->nArg, p->enc,
        p->iContext ? (void*)db : (void*)0,
        p->xFunc, 0, 0
    );
  }

  return rc;
}

#ifndef SQLITE_CORE
#ifdef _WIN32
__declspec(dllexport)
#endif
SQLITE_API int sqlite3_icu_init(
  sqlite3 *db,
  char **pzErrMsg,
  const sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi)
  return sqlite3IcuInit(db);
}
#endif

#endif

/************** End of icu.c *************************************************/
/************** Begin file fts3_icu.c ****************************************/
/*
** 2007 June 22
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file implements a tokenizer for fts3 based on the ICU library.
*/
/* #include "fts3Int.h" */
#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3)
#ifdef SQLITE_ENABLE_ICU

/* #include <assert.h> */
/* #include <string.h> */
/* #include "fts3_tokenizer.h" */

#include <unicode/ubrk.h>
/* #include <unicode/ucol.h> */
/* #include <unicode/ustring.h> */
#include <unicode/utf16.h>

typedef struct IcuTokenizer IcuTokenizer;
typedef struct IcuCursor IcuCursor;

struct IcuTokenizer {
  sqlite3_tokenizer base;
  char *zLocale;
};

struct IcuCursor {
  sqlite3_tokenizer_cursor base;

  UBreakIterator *pIter;      /* ICU break-iterator object */
  int nChar;                  /* Number of UChar elements in pInput */
  UChar *aChar;               /* Copy of input using utf-16 encoding */
  int *aOffset;               /* Offsets of each character in utf-8 input */

  int nBuffer;
  char *zBuffer;

  int iToken;
};

/*
** Create a new tokenizer instance.
*/
static int icuCreate(
  int argc,                            /* Number of entries in argv[] */
  const char * const *argv,            /* Tokenizer creation arguments */
  sqlite3_tokenizer **ppTokenizer      /* OUT: Created tokenizer */
){
  IcuTokenizer *p;
  int n = 0;

  if( argc>0 ){
    n = strlen(argv[0])+1;
  }
  p = (IcuTokenizer *)sqlite3_malloc64(sizeof(IcuTokenizer)+n);
  if( !p ){
    return SQLITE_NOMEM;
  }
  memset(p, 0, sizeof(IcuTokenizer));

  if( n ){
    p->zLocale = (char *)&p[1];
    memcpy(p->zLocale, argv[0], n);
  }

  *ppTokenizer = (sqlite3_tokenizer *)p;

  return SQLITE_OK;
}

/*
** Destroy a tokenizer
*/
static int icuDestroy(sqlite3_tokenizer *pTokenizer){
  IcuTokenizer *p = (IcuTokenizer *)pTokenizer;
  sqlite3_free(p);
  return SQLITE_OK;
}

/*
** Prepare to begin tokenizing a particular string.  The input
** string to be tokenized is pInput[0..nBytes-1].  A cursor
** used to incrementally tokenize this string is returned in
** *ppCursor.
*/
static int icuOpen(
  sqlite3_tokenizer *pTokenizer,         /* The tokenizer */
  const char *zInput,                    /* Input string */
  int nInput,                            /* Length of zInput in bytes */
  sqlite3_tokenizer_cursor **ppCursor    /* OUT: Tokenization cursor */
){
  IcuTokenizer *p = (IcuTokenizer *)pTokenizer;
  IcuCursor *pCsr;

  const int32_t opt = U_FOLD_CASE_DEFAULT;
  UErrorCode status = U_ZERO_ERROR;
  int nChar;

  UChar32 c;
  int iInput = 0;
  int iOut = 0;

  *ppCursor = 0;

  if( zInput==0 ){
    nInput = 0;
    zInput = "";
  }else if( nInput<0 ){
    nInput = strlen(zInput);
  }
  nChar = nInput+1;
  pCsr = (IcuCursor *)sqlite3_malloc64(
      sizeof(IcuCursor) +                /* IcuCursor */
      ((nChar+3)&~3) * sizeof(UChar) +   /* IcuCursor.aChar[] */
      (nChar+1) * sizeof(int)            /* IcuCursor.aOffset[] */
  );
  if( !pCsr ){
    return SQLITE_NOMEM;
  }
  memset(pCsr, 0, sizeof(IcuCursor));
  pCsr->aChar = (UChar *)&pCsr[1];
  pCsr->aOffset = (int *)&pCsr->aChar[(nChar+3)&~3];

  pCsr->aOffset[iOut] = iInput;
  U8_NEXT(zInput, iInput, nInput, c);
  while( c>0 ){
    int isError = 0;
    c = u_foldCase(c, opt);
    U16_APPEND(pCsr->aChar, iOut, nChar, c, isError);
    if( isError ){
      sqlite3_free(pCsr);
      return SQLITE_ERROR;
    }
    pCsr->aOffset[iOut] = iInput;

    if( iInput<nInput ){
      U8_NEXT(zInput, iInput, nInput, c);
    }else{
      c = 0;
    }
  }

  pCsr->pIter = ubrk_open(UBRK_WORD, p->zLocale, pCsr->aChar, iOut, &status);
  if( !U_SUCCESS(status) ){
    sqlite3_free(pCsr);
    return SQLITE_ERROR;
  }
  pCsr->nChar = iOut;

  ubrk_first(pCsr->pIter);
  *ppCursor = (sqlite3_tokenizer_cursor *)pCsr;
  return SQLITE_OK;
}

/*
** Close a tokenization cursor previously opened by a call to icuOpen().
*/
static int icuClose(sqlite3_tokenizer_cursor *pCursor){
  IcuCursor *pCsr = (IcuCursor *)pCursor;
  ubrk_close(pCsr->pIter);
  sqlite3_free(pCsr->zBuffer);
  sqlite3_free(pCsr);
  return SQLITE_OK;
}

/*
** Extract the next token from a tokenization cursor.
*/
static int icuNext(
  sqlite3_tokenizer_cursor *pCursor,  /* Cursor returned by simpleOpen */
  const char **ppToken,               /* OUT: *ppToken is the token text */
  int *pnBytes,                       /* OUT: Number of bytes in token */
  int *piStartOffset,                 /* OUT: Starting offset of token */
  int *piEndOffset,                   /* OUT: Ending offset of token */
  int *piPosition                     /* OUT: Position integer of token */
){
  IcuCursor *pCsr = (IcuCursor *)pCursor;

  int iStart = 0;
  int iEnd = 0;
  int nByte = 0;

  while( iStart==iEnd ){
    UChar32 c;

    iStart = ubrk_current(pCsr->pIter);
    iEnd = ubrk_next(pCsr->pIter);
    if( iEnd==UBRK_DONE ){
      return SQLITE_DONE;
    }

    while( iStart<iEnd ){
      int iWhite = iStart;
      U16_NEXT(pCsr->aChar, iWhite, pCsr->nChar, c);
      if( u_isspace(c) ){
        iStart = iWhite;
      }else{
        break;
      }
    }
    assert(iStart<=iEnd);
  }

  do {
    UErrorCode status = U_ZERO_ERROR;
    if( nByte ){
      char *zNew = sqlite3_realloc(pCsr->zBuffer, nByte);
      if( !zNew ){
        return SQLITE_NOMEM;
      }
      pCsr->zBuffer = zNew;
      pCsr->nBuffer = nByte;
    }

    u_strToUTF8(
        pCsr->zBuffer, pCsr->nBuffer, &nByte,    /* Output vars */
        &pCsr->aChar[iStart], iEnd-iStart,       /* Input vars */
        &status                                  /* Output success/failure */
    );
  } while( nByte>pCsr->nBuffer );

  *ppToken = pCsr->zBuffer;
  *pnBytes = nByte;
  *piStartOffset = pCsr->aOffset[iStart];
  *piEndOffset = pCsr->aOffset[iEnd];
  *piPosition = pCsr->iToken++;

  return SQLITE_OK;
}

/*
** The set of routines that implement the simple tokenizer
*/
static const sqlite3_tokenizer_module icuTokenizerModule = {
  0,                           /* iVersion    */
  icuCreate,                   /* xCreate     */
  icuDestroy,                  /* xCreate     */
  icuOpen,                     /* xOpen       */
  icuClose,                    /* xClose      */
  icuNext,                     /* xNext       */
  0,                           /* xLanguageid */
};

/*
** Set *ppModule to point at the implementation of the ICU tokenizer.
*/
SQLITE_PRIVATE void sqlite3Fts3IcuTokenizerModule(
  sqlite3_tokenizer_module const**ppModule
){
  *ppModule = &icuTokenizerModule;
}

#endif /* defined(SQLITE_ENABLE_ICU) */
#endif /* !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3) */

/************** End of fts3_icu.c ********************************************/
/************** Begin file sqlite3rbu.c **************************************/
/*
** 2014 August 30
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
**
** OVERVIEW
**
**  The RBU extension requires that the RBU update be packaged as an
**  SQLite database. The tables it expects to find are described in
**  sqlite3rbu.h.  Essentially, for each table xyz in the target database
**  that the user wishes to write to, a corresponding data_xyz table is
**  created in the RBU database and populated with one row for each row to
**  update, insert or delete from the target table.
**
**  The update proceeds in three stages:
**
**  1) The database is updated. The modified database pages are written
**     to a *-oal file. A *-oal file is just like a *-wal file, except
**     that it is named "<database>-oal" instead of "<database>-wal".
**     Because regular SQLite clients do not look for file named
**     "<database>-oal", they go on using the original database in
**     rollback mode while the *-oal file is being generated.
**
**     During this stage RBU does not update the database by writing
**     directly to the target tables. Instead it creates "imposter"
**     tables using the SQLITE_TESTCTRL_IMPOSTER interface that it uses
**     to update each b-tree individually. All updates required by each
**     b-tree are completed before moving on to the next, and all
**     updates are done in sorted key order.
**
**  2) The "<database>-oal" file is moved to the equivalent "<database>-wal"
**     location using a call to rename(2). Before doing this the RBU
**     module takes an EXCLUSIVE lock on the database file, ensuring
**     that there are no other active readers.
**
**     Once the EXCLUSIVE lock is released, any other database readers
**     detect the new *-wal file and read the database in wal mode. At
**     this point they see the new version of the database - including
**     the updates made as part of the RBU update.
**
**  3) The new *-wal file is checkpointed. This proceeds in the same way
**     as a regular database checkpoint, except that a single frame is
**     checkpointed each time sqlite3rbu_step() is called. If the RBU
**     handle is closed before the entire *-wal file is checkpointed,
**     the checkpoint progress is saved in the RBU database and the
**     checkpoint can be resumed by another RBU client at some point in
**     the future.
**
** POTENTIAL PROBLEMS
**
**  The rename() call might not be portable. And RBU is not currently
**  syncing the directory after renaming the file.
**
**  When state is saved, any commit to the *-oal file and the commit to
**  the RBU update database are not atomic. So if the power fails at the
**  wrong moment they might get out of sync. As the main database will be
**  committed before the RBU update database this will likely either just
**  pass unnoticed, or result in SQLITE_CONSTRAINT errors (due to UNIQUE
**  constraint violations).
**
**  If some client does modify the target database mid RBU update, or some
**  other error occurs, the RBU extension will keep throwing errors. It's
**  not really clear how to get out of this state. The system could just
**  by delete the RBU update database and *-oal file and have the device
**  download the update again and start over.
**
**  At present, for an UPDATE, both the new.* and old.* records are
**  collected in the rbu_xyz table. And for both UPDATEs and DELETEs all
**  fields are collected.  This means we're probably writing a lot more
**  data to disk when saving the state of an ongoing update to the RBU
**  update database than is strictly necessary.
**
*/

/* #include <assert.h> */
/* #include <string.h> */
/* #include <stdio.h> */

/* #include "sqlite3.h" */

#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_RBU)
/************** Include sqlite3rbu.h in the middle of sqlite3rbu.c ***********/
/************** Begin file sqlite3rbu.h **************************************/
/*
** 2014 August 30
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file contains the public interface for the RBU extension.
*/

/*
** SUMMARY
**
** Writing a transaction containing a large number of operations on
** b-tree indexes that are collectively larger than the available cache
** memory can be very inefficient.
**
** The problem is that in order to update a b-tree, the leaf page (at least)
** containing the entry being inserted or deleted must be modified. If the
** working set of leaves is larger than the available cache memory, then a
** single leaf that is modified more than once as part of the transaction
** may be loaded from or written to the persistent media multiple times.
** Additionally, because the index updates are likely to be applied in
** random order, access to pages within the database is also likely to be in
** random order, which is itself quite inefficient.
**
** One way to improve the situation is to sort the operations on each index
** by index key before applying them to the b-tree. This leads to an IO
** pattern that resembles a single linear scan through the index b-tree,
** and all but guarantees each modified leaf page is loaded and stored
** exactly once. SQLite uses this trick to improve the performance of
** CREATE INDEX commands. This extension allows it to be used to improve
** the performance of large transactions on existing databases.
**
** Additionally, this extension allows the work involved in writing the
** large transaction to be broken down into sub-transactions performed
** sequentially by separate processes. This is useful if the system cannot
** guarantee that a single update process will run for long enough to apply
** the entire update, for example because the update is being applied on a
** mobile device that is frequently rebooted. Even after the writer process
** has committed one or more sub-transactions, other database clients continue
** to read from the original database snapshot. In other words, partially
** applied transactions are not visible to other clients.
**
** "RBU" stands for "Resumable Bulk Update". As in a large database update
** transmitted via a wireless network to a mobile device. A transaction
** applied using this extension is hence referred to as an "RBU update".
**
**
** LIMITATIONS
**
** An "RBU update" transaction is subject to the following limitations:
**
**   * The transaction must consist of INSERT, UPDATE and DELETE operations
**     only.
**
**   * INSERT statements may not use any default values.
**
**   * UPDATE and DELETE statements must identify their target rows by
**     non-NULL PRIMARY KEY values. Rows with NULL values stored in PRIMARY
**     KEY fields may not be updated or deleted. If the table being written
**     has no PRIMARY KEY, affected rows must be identified by rowid.
**
**   * UPDATE statements may not modify PRIMARY KEY columns.
**
**   * No triggers will be fired.
**
**   * No foreign key violations are detected or reported.
**
**   * CHECK constraints are not enforced.
**
**   * No constraint handling mode except for "OR ROLLBACK" is supported.
**
**
** PREPARATION
**
** An "RBU update" is stored as a separate SQLite database. A database
** containing an RBU update is an "RBU database". For each table in the
** target database to be updated, the RBU database should contain a table
** named "data_<target name>" containing the same set of columns as the
** target table, and one more - "rbu_control". The data_% table should
** have no PRIMARY KEY or UNIQUE constraints, but each column should have
** the same type as the corresponding column in the target database.
** The "rbu_control" column should have no type at all. For example, if
** the target database contains:
**
**   CREATE TABLE t1(a INTEGER PRIMARY KEY, b TEXT, c UNIQUE);
**
** Then the RBU database should contain:
**
**   CREATE TABLE data_t1(a INTEGER, b TEXT, c, rbu_control);
**
** The order of the columns in the data_% table does not matter.
**
** Instead of a regular table, the RBU database may also contain virtual
** tables or views named using the data_<target> naming scheme.
**
** Instead of the plain data_<target> naming scheme, RBU database tables
** may also be named data<integer>_<target>, where <integer> is any sequence
** of zero or more numeric characters (0-9). This can be significant because
** tables within the RBU database are always processed in order sorted by
** name. By judicious selection of the <integer> portion of the names
** of the RBU tables the user can therefore control the order in which they
** are processed. This can be useful, for example, to ensure that "external
** content" FTS4 tables are updated before their underlying content tables.
**
** If the target database table is a virtual table or a table that has no
** PRIMARY KEY declaration, the data_% table must also contain a column
** named "rbu_rowid". This column is mapped to the table's implicit primary
** key column - "rowid". Virtual tables for which the "rowid" column does
** not function like a primary key value cannot be updated using RBU. For
** example, if the target db contains either of the following:
**
**   CREATE VIRTUAL TABLE x1 USING fts3(a, b);
**   CREATE TABLE x1(a, b)
**
** then the RBU database should contain:
**
**   CREATE TABLE data_x1(a, b, rbu_rowid, rbu_control);
**
** All non-hidden columns (i.e. all columns matched by "SELECT *") of the
** target table must be present in the input table. For virtual tables,
** hidden columns are optional - they are updated by RBU if present in
** the input table, or not otherwise. For example, to write to an fts4
** table with a hidden languageid column such as:
**
**   CREATE VIRTUAL TABLE ft1 USING fts4(a, b, languageid='langid');
**
** Either of the following input table schemas may be used:
**
**   CREATE TABLE data_ft1(a, b, langid, rbu_rowid, rbu_control);
**   CREATE TABLE data_ft1(a, b, rbu_rowid, rbu_control);
**
** For each row to INSERT into the target database as part of the RBU
** update, the corresponding data_% table should contain a single record
** with the "rbu_control" column set to contain integer value 0. The
** other columns should be set to the values that make up the new record
** to insert.
**
** If the target database table has an INTEGER PRIMARY KEY, it is not
** possible to insert a NULL value into the IPK column. Attempting to
** do so results in an SQLITE_MISMATCH error.
**
** For each row to DELETE from the target database as part of the RBU
** update, the corresponding data_% table should contain a single record
** with the "rbu_control" column set to contain integer value 1. The
** real primary key values of the row to delete should be stored in the
** corresponding columns of the data_% table. The values stored in the
** other columns are not used.
**
** For each row to UPDATE from the target database as part of the RBU
** update, the corresponding data_% table should contain a single record
** with the "rbu_control" column set to contain a value of type text.
** The real primary key values identifying the row to update should be
** stored in the corresponding columns of the data_% table row, as should
** the new values of all columns being update. The text value in the
** "rbu_control" column must contain the same number of characters as
** there are columns in the target database table, and must consist entirely
** of 'x' and '.' characters (or in some special cases 'd' - see below). For
** each column that is being updated, the corresponding character is set to
** 'x'. For those that remain as they are, the corresponding character of the
** rbu_control value should be set to '.'. For example, given the tables
** above, the update statement:
**
**   UPDATE t1 SET c = 'usa' WHERE a = 4;
**
** is represented by the data_t1 row created by:
**
**   INSERT INTO data_t1(a, b, c, rbu_control) VALUES(4, NULL, 'usa', '..x');
**
** Instead of an 'x' character, characters of the rbu_control value specified
** for UPDATEs may also be set to 'd'. In this case, instead of updating the
** target table with the value stored in the corresponding data_% column, the
** user-defined SQL function "rbu_delta()" is invoked and the result stored in
** the target table column. rbu_delta() is invoked with two arguments - the
** original value currently stored in the target table column and the
** value specified in the data_xxx table.
**
** For example, this row:
**
**   INSERT INTO data_t1(a, b, c, rbu_control) VALUES(4, NULL, 'usa', '..d');
**
** is similar to an UPDATE statement such as:
**
**   UPDATE t1 SET c = rbu_delta(c, 'usa') WHERE a = 4;
**
** Finally, if an 'f' character appears in place of a 'd' or 's' in an
** ota_control string, the contents of the data_xxx table column is assumed
** to be a "fossil delta" - a patch to be applied to a blob value in the
** format used by the fossil source-code management system. In this case
** the existing value within the target database table must be of type BLOB.
** It is replaced by the result of applying the specified fossil delta to
** itself.
**
** If the target database table is a virtual table or a table with no PRIMARY
** KEY, the rbu_control value should not include a character corresponding
** to the rbu_rowid value. For example, this:
**
**   INSERT INTO data_ft1(a, b, rbu_rowid, rbu_control)
**       VALUES(NULL, 'usa', 12, '.x');
**
** causes a result similar to:
**
**   UPDATE ft1 SET b = 'usa' WHERE rowid = 12;
**
** The data_xxx tables themselves should have no PRIMARY KEY declarations.
** However, RBU is more efficient if reading the rows in from each data_xxx
** table in "rowid" order is roughly the same as reading them sorted by
** the PRIMARY KEY of the corresponding target database table. In other
** words, rows should be sorted using the destination table PRIMARY KEY
** fields before they are inserted into the data_xxx tables.
**
** USAGE
**
** The API declared below allows an application to apply an RBU update
** stored on disk to an existing target database. Essentially, the
** application:
**
**     1) Opens an RBU handle using the sqlite3rbu_open() function.
**
**     2) Registers any required virtual table modules with the database
**        handle returned by sqlite3rbu_db(). Also, if required, register
**        the rbu_delta() implementation.
**
**     3) Calls the sqlite3rbu_step() function one or more times on
**        the new handle. Each call to sqlite3rbu_step() performs a single
**        b-tree operation, so thousands of calls may be required to apply
**        a complete update.
**
**     4) Calls sqlite3rbu_close() to close the RBU update handle. If
**        sqlite3rbu_step() has been called enough times to completely
**        apply the update to the target database, then the RBU database
**        is marked as fully applied. Otherwise, the state of the RBU
**        update application is saved in the RBU database for later
**        resumption.
**
** See comments below for more detail on APIs.
**
** If an update is only partially applied to the target database by the
** time sqlite3rbu_close() is called, various state information is saved
** within the RBU database. This allows subsequent processes to automatically
** resume the RBU update from where it left off.
**
** To remove all RBU extension state information, returning an RBU database
** to its original contents, it is sufficient to drop all tables that begin
** with the prefix "rbu_"
**
** DATABASE LOCKING
**
** An RBU update may not be applied to a database in WAL mode. Attempting
** to do so is an error (SQLITE_ERROR).
**
** While an RBU handle is open, a SHARED lock may be held on the target
** database file. This means it is possible for other clients to read the
** database, but not to write it.
**
** If an RBU update is started and then suspended before it is completed,
** then an external client writes to the database, then attempting to resume
** the suspended RBU update is also an error (SQLITE_BUSY).
*/

#ifndef _SQLITE3RBU_H
#define _SQLITE3RBU_H

/* #include "sqlite3.h"              ** Required for error code definitions ** */

#if 0
extern "C" {
#endif

typedef struct sqlite3rbu sqlite3rbu;

/*
** Open an RBU handle.
**
** Argument zTarget is the path to the target database. Argument zRbu is
** the path to the RBU database. Each call to this function must be matched
** by a call to sqlite3rbu_close(). When opening the databases, RBU passes
** the SQLITE_CONFIG_URI flag to sqlite3_open_v2(). So if either zTarget
** or zRbu begin with "file:", it will be interpreted as an SQLite
** database URI, not a regular file name.
**
** If the zState argument is passed a NULL value, the RBU extension stores
** the current state of the update (how many rows have been updated, which
** indexes are yet to be updated etc.) within the RBU database itself. This
** can be convenient, as it means that the RBU application does not need to
** organize removing a separate state file after the update is concluded.
** Or, if zState is non-NULL, it must be a path to a database file in which
** the RBU extension can store the state of the update.
**
** When resuming an RBU update, the zState argument must be passed the same
** value as when the RBU update was started.
**
** Once the RBU update is finished, the RBU extension does not
** automatically remove any zState database file, even if it created it.
**
** By default, RBU uses the default VFS to access the files on disk. To
** use a VFS other than the default, an SQLite "file:" URI containing a
** "vfs=..." option may be passed as the zTarget option.
**
** IMPORTANT NOTE FOR ZIPVFS USERS: The RBU extension works with all of
** SQLite's built-in VFSs, including the multiplexor VFS. However it does
** not work out of the box with zipvfs. Refer to the comment describing
** the zipvfs_create_vfs() API below for details on using RBU with zipvfs.
*/
SQLITE_API sqlite3rbu *sqlite3rbu_open(
  const char *zTarget,
  const char *zRbu,
  const char *zState
);

/*
** Open an RBU handle to perform an RBU vacuum on database file zTarget.
** An RBU vacuum is similar to SQLite's built-in VACUUM command, except
** that it can be suspended and resumed like an RBU update.
**
** The second argument to this function identifies a database in which
** to store the state of the RBU vacuum operation if it is suspended. The
** first time sqlite3rbu_vacuum() is called, to start an RBU vacuum
** operation, the state database should either not exist or be empty
** (contain no tables). If an RBU vacuum is suspended by calling
** sqlite3rbu_close() on the RBU handle before sqlite3rbu_step() has
** returned SQLITE_DONE, the vacuum state is stored in the state database.
** The vacuum can be resumed by calling this function to open a new RBU
** handle specifying the same target and state databases.
**
** If the second argument passed to this function is NULL, then the
** name of the state database is "<database>-vacuum", where <database>
** is the name of the target database file. In this case, on UNIX, if the
** state database is not already present in the file-system, it is created
** with the same permissions as the target db is made.
**
** With an RBU vacuum, it is an SQLITE_MISUSE error if the name of the
** state database ends with "-vactmp". This name is reserved for internal
** use.
**
** This function does not delete the state database after an RBU vacuum
** is completed, even if it created it. However, if the call to
** sqlite3rbu_close() returns any value other than SQLITE_OK, the contents
** of the state tables within the state database are zeroed. This way,
** the next call to sqlite3rbu_vacuum() opens a handle that starts a
** new RBU vacuum operation.
**
** As with sqlite3rbu_open(), Zipvfs users should refer to the comment
** describing the sqlite3rbu_create_vfs() API function below for
** a description of the complications associated with using RBU with
** zipvfs databases.
*/
SQLITE_API sqlite3rbu *sqlite3rbu_vacuum(
  const char *zTarget,
  const char *zState
);

/*
** Configure a limit for the amount of temp space that may be used by
** the RBU handle passed as the first argument. The new limit is specified
** in bytes by the second parameter. If it is positive, the limit is updated.
** If the second parameter to this function is passed zero, then the limit
** is removed entirely. If the second parameter is negative, the limit is
** not modified (this is useful for querying the current limit).
**
** In all cases the returned value is the current limit in bytes (zero
** indicates unlimited).
**
** If the temp space limit is exceeded during operation, an SQLITE_FULL
** error is returned.
*/
SQLITE_API sqlite3_int64 sqlite3rbu_temp_size_limit(sqlite3rbu*, sqlite3_int64);

/*
** Return the current amount of temp file space, in bytes, currently used by
** the RBU handle passed as the only argument.
*/
SQLITE_API sqlite3_int64 sqlite3rbu_temp_size(sqlite3rbu*);

/*
** Internally, each RBU connection uses a separate SQLite database
** connection to access the target and rbu update databases. This
** API allows the application direct access to these database handles.
**
** The first argument passed to this function must be a valid, open, RBU
** handle. The second argument should be passed zero to access the target
** database handle, or non-zero to access the rbu update database handle.
** Accessing the underlying database handles may be useful in the
** following scenarios:
**
**   * If any target tables are virtual tables, it may be necessary to
**     call sqlite3_create_module() on the target database handle to
**     register the required virtual table implementations.
**
**   * If the data_xxx tables in the RBU source database are virtual
**     tables, the application may need to call sqlite3_create_module() on
**     the rbu update db handle to any required virtual table
**     implementations.
**
**   * If the application uses the "rbu_delta()" feature described above,
**     it must use sqlite3_create_function() or similar to register the
**     rbu_delta() implementation with the target database handle.
**
** If an error has occurred, either while opening or stepping the RBU object,
** this function may return NULL. The error code and message may be collected
** when sqlite3rbu_close() is called.
**
** Database handles returned by this function remain valid until the next
** call to any sqlite3rbu_xxx() function other than sqlite3rbu_db().
*/
SQLITE_API sqlite3 *sqlite3rbu_db(sqlite3rbu*, int bRbu);

/*
** Do some work towards applying the RBU update to the target db.
**
** Return SQLITE_DONE if the update has been completely applied, or
** SQLITE_OK if no error occurs but there remains work to do to apply
** the RBU update. If an error does occur, some other error code is
** returned.
**
** Once a call to sqlite3rbu_step() has returned a value other than
** SQLITE_OK, all subsequent calls on the same RBU handle are no-ops
** that immediately return the same value.
*/
SQLITE_API int sqlite3rbu_step(sqlite3rbu *pRbu);

/*
** Force RBU to save its state to disk.
**
** If a power failure or application crash occurs during an update, following
** system recovery RBU may resume the update from the point at which the state
** was last saved. In other words, from the most recent successful call to
** sqlite3rbu_close() or this function.
**
** SQLITE_OK is returned if successful, or an SQLite error code otherwise.
*/
SQLITE_API int sqlite3rbu_savestate(sqlite3rbu *pRbu);

/*
** Close an RBU handle.
**
** If the RBU update has been completely applied, mark the RBU database
** as fully applied. Otherwise, assuming no error has occurred, save the
** current state of the RBU update application to the RBU database.
**
** If an error has already occurred as part of an sqlite3rbu_step()
** or sqlite3rbu_open() call, or if one occurs within this function, an
** SQLite error code is returned. Additionally, if pzErrmsg is not NULL,
** *pzErrmsg may be set to point to a buffer containing a utf-8 formatted
** English language error message. It is the responsibility of the caller to
** eventually free any such buffer using sqlite3_free().
**
** Otherwise, if no error occurs, this function returns SQLITE_OK if the
** update has been partially applied, or SQLITE_DONE if it has been
** completely applied.
*/
SQLITE_API int sqlite3rbu_close(sqlite3rbu *pRbu, char **pzErrmsg);

/*
** Return the total number of key-value operations (inserts, deletes or
** updates) that have been performed on the target database since the
** current RBU update was started.
*/
SQLITE_API sqlite3_int64 sqlite3rbu_progress(sqlite3rbu *pRbu);

/*
** Obtain permyriadage (permyriadage is to 10000 as percentage is to 100)
** progress indications for the two stages of an RBU update. This API may
** be useful for driving GUI progress indicators and similar.
**
** An RBU update is divided into two stages:
**
**   * Stage 1, in which changes are accumulated in an oal/wal file, and
**   * Stage 2, in which the contents of the wal file are copied into the
**     main database.
**
** The update is visible to non-RBU clients during stage 2. During stage 1
** non-RBU reader clients may see the original database.
**
** If this API is called during stage 2 of the update, output variable
** (*pnOne) is set to 10000 to indicate that stage 1 has finished and (*pnTwo)
** to a value between 0 and 10000 to indicate the permyriadage progress of
** stage 2. A value of 5000 indicates that stage 2 is half finished,
** 9000 indicates that it is 90% finished, and so on.
**
** If this API is called during stage 1 of the update, output variable
** (*pnTwo) is set to 0 to indicate that stage 2 has not yet started. The
** value to which (*pnOne) is set depends on whether or not the RBU
** database contains an "rbu_count" table. The rbu_count table, if it
** exists, must contain the same columns as the following:
**
**   CREATE TABLE rbu_count(tbl TEXT PRIMARY KEY, cnt INTEGER) WITHOUT ROWID;
**
** There must be one row in the table for each source (data_xxx) table within
** the RBU database. The 'tbl' column should contain the name of the source
** table. The 'cnt' column should contain the number of rows within the
** source table.
**
** If the rbu_count table is present and populated correctly and this
** API is called during stage 1, the *pnOne output variable is set to the
** permyriadage progress of the same stage. If the rbu_count table does
** not exist, then (*pnOne) is set to -1 during stage 1. If the rbu_count
** table exists but is not correctly populated, the value of the *pnOne
** output variable during stage 1 is undefined.
*/
SQLITE_API void sqlite3rbu_bp_progress(sqlite3rbu *pRbu, int *pnOne, int*pnTwo);

/*
** Obtain an indication as to the current stage of an RBU update or vacuum.
** This function always returns one of the SQLITE_RBU_STATE_XXX constants
** defined in this file. Return values should be interpreted as follows:
**
** SQLITE_RBU_STATE_OAL:
**   RBU is currently building a *-oal file. The next call to sqlite3rbu_step()
**   may either add further data to the *-oal file, or compute data that will
**   be added by a subsequent call.
**
** SQLITE_RBU_STATE_MOVE:
**   RBU has finished building the *-oal file. The next call to sqlite3rbu_step()
**   will move the *-oal file to the equivalent *-wal path. If the current
**   operation is an RBU update, then the updated version of the database
**   file will become visible to ordinary SQLite clients following the next
**   call to sqlite3rbu_step().
**
** SQLITE_RBU_STATE_CHECKPOINT:
**   RBU is currently performing an incremental checkpoint. The next call to
**   sqlite3rbu_step() will copy a page of data from the *-wal file into
**   the target database file.
**
** SQLITE_RBU_STATE_DONE:
**   The RBU operation has finished. Any subsequent calls to sqlite3rbu_step()
**   will immediately return SQLITE_DONE.
**
** SQLITE_RBU_STATE_ERROR:
**   An error has occurred. Any subsequent calls to sqlite3rbu_step() will
**   immediately return the SQLite error code associated with the error.
*/
#define SQLITE_RBU_STATE_OAL        1
#define SQLITE_RBU_STATE_MOVE       2
#define SQLITE_RBU_STATE_CHECKPOINT 3
#define SQLITE_RBU_STATE_DONE       4
#define SQLITE_RBU_STATE_ERROR      5

SQLITE_API int sqlite3rbu_state(sqlite3rbu *pRbu);

/*
** As part of applying an RBU update or performing an RBU vacuum operation,
** the system must at one point move the *-oal file to the equivalent *-wal
** path. Normally, it does this by invoking POSIX function rename(2) directly.
** Except on WINCE platforms, where it uses win32 API MoveFileW(). This
** function may be used to register a callback that the RBU module will invoke
** instead of one of these APIs.
**
** If a callback is registered with an RBU handle, it invokes it instead
** of rename(2) when it needs to move a file within the file-system. The
** first argument passed to the xRename() callback is a copy of the second
** argument (pArg) passed to this function. The second is the full path
** to the file to move and the third the full path to which it should be
** moved. The callback function should return SQLITE_OK to indicate
** success. If an error occurs, it should return an SQLite error code.
** In this case the RBU operation will be abandoned and the error returned
** to the RBU user.
**
** Passing a NULL pointer in place of the xRename argument to this function
** restores the default behaviour.
*/
SQLITE_API void sqlite3rbu_rename_handler(
  sqlite3rbu *pRbu,
  void *pArg,
  int (*xRename)(void *pArg, const char *zOld, const char *zNew)
);


/*
** Create an RBU VFS named zName that accesses the underlying file-system
** via existing VFS zParent. Or, if the zParent parameter is passed NULL,
** then the new RBU VFS uses the default system VFS to access the file-system.
** The new object is registered as a non-default VFS with SQLite before
** returning.
**
** Part of the RBU implementation uses a custom VFS object. Usually, this
** object is created and deleted automatically by RBU.
**
** The exception is for applications that also use zipvfs. In this case,
** the custom VFS must be explicitly created by the user before the RBU
** handle is opened. The RBU VFS should be installed so that the zipvfs
** VFS uses the RBU VFS, which in turn uses any other VFS layers in use
** (for example multiplexor) to access the file-system. For example,
** to assemble an RBU enabled VFS stack that uses both zipvfs and
** multiplexor (error checking omitted):
**
**     // Create a VFS named "multiplex" (not the default).
**     sqlite3_multiplex_initialize(0, 0);
**
**     // Create an rbu VFS named "rbu" that uses multiplexor. If the
**     // second argument were replaced with NULL, the "rbu" VFS would
**     // access the file-system via the system default VFS, bypassing the
**     // multiplexor.
**     sqlite3rbu_create_vfs("rbu", "multiplex");
**
**     // Create a zipvfs VFS named "zipvfs" that uses rbu.
**     zipvfs_create_vfs_v3("zipvfs", "rbu", 0, xCompressorAlgorithmDetector);
**
**     // Make zipvfs the default VFS.
**     sqlite3_vfs_register(sqlite3_vfs_find("zipvfs"), 1);
**
** Because the default VFS created above includes a RBU functionality, it
** may be used by RBU clients. Attempting to use RBU with a zipvfs VFS stack
** that does not include the RBU layer results in an error.
**
** The overhead of adding the "rbu" VFS to the system is negligible for
** non-RBU users. There is no harm in an application accessing the
** file-system via "rbu" all the time, even if it only uses RBU functionality
** occasionally.
*/
SQLITE_API int sqlite3rbu_create_vfs(const char *zName, const char *zParent);

/*
** Deregister and destroy an RBU vfs created by an earlier call to
** sqlite3rbu_create_vfs().
**
** VFS objects are not reference counted. If a VFS object is destroyed
** before all database handles that use it have been closed, the results
** are undefined.
*/
SQLITE_API void sqlite3rbu_destroy_vfs(const char *zName);

#if 0
}  /* end of the 'extern "C"' block */
#endif

#endif /* _SQLITE3RBU_H */

/************** End of sqlite3rbu.h ******************************************/
/************** Continuing where we left off in sqlite3rbu.c *****************/

#if defined(_WIN32_WCE)
/* #include "windows.h" */
#endif

/* Maximum number of prepared UPDATE statements held by this module */
#define SQLITE_RBU_UPDATE_CACHESIZE 16

/* Delta checksums disabled by default.  Compile with -DRBU_ENABLE_DELTA_CKSUM
** to enable checksum verification.
*/
#ifndef RBU_ENABLE_DELTA_CKSUM
# define RBU_ENABLE_DELTA_CKSUM 0
#endif

/*
** Swap two objects of type TYPE.
*/
#if !defined(SQLITE_AMALGAMATION)
# define SWAP(TYPE,A,B) {TYPE t=A; A=B; B=t;}
#endif

/*
** Name of the URI option that causes RBU to take an exclusive lock as
** part of the incremental checkpoint operation.
*/
#define RBU_EXCLUSIVE_CHECKPOINT "rbu_exclusive_checkpoint"


/*
** The rbu_state table is used to save the state of a partially applied
** update so that it can be resumed later. The table consists of integer
** keys mapped to values as follows:
**
** RBU_STATE_STAGE:
**   May be set to integer values 1, 2, 4 or 5. As follows:
**       1: the *-rbu file is currently under construction.
**       2: the *-rbu file has been constructed, but not yet moved
**          to the *-wal path.
**       4: the checkpoint is underway.
**       5: the rbu update has been checkpointed.
**
** RBU_STATE_TBL:
**   Only valid if STAGE==1. The target database name of the table
**   currently being written.
**
** RBU_STATE_IDX:
**   Only valid if STAGE==1. The target database name of the index
**   currently being written, or NULL if the main table is currently being
**   updated.
**
** RBU_STATE_ROW:
**   Only valid if STAGE==1. Number of rows already processed for the current
**   table/index.
**
** RBU_STATE_PROGRESS:
**   Trbul number of sqlite3rbu_step() calls made so far as part of this
**   rbu update.
**
** RBU_STATE_CKPT:
**   Valid if STAGE==4. The 64-bit checksum associated with the wal-index
**   header created by recovering the *-wal file. This is used to detect
**   cases when another client appends frames to the *-wal file in the
**   middle of an incremental checkpoint (an incremental checkpoint cannot
**   be continued if this happens).
**
** RBU_STATE_COOKIE:
**   Valid if STAGE==1. The current change-counter cookie value in the
**   target db file.
**
** RBU_STATE_OALSZ:
**   Valid if STAGE==1. The size in bytes of the *-oal file.
**
** RBU_STATE_DATATBL:
**   Only valid if STAGE==1. The RBU database name of the table
**   currently being read.
*/
#define RBU_STATE_STAGE        1
#define RBU_STATE_TBL          2
#define RBU_STATE_IDX          3
#define RBU_STATE_ROW          4
#define RBU_STATE_PROGRESS     5
#define RBU_STATE_CKPT         6
#define RBU_STATE_COOKIE       7
#define RBU_STATE_OALSZ        8
#define RBU_STATE_PHASEONESTEP 9
#define RBU_STATE_DATATBL     10

#define RBU_STAGE_OAL         1
#define RBU_STAGE_MOVE        2
#define RBU_STAGE_CAPTURE     3
#define RBU_STAGE_CKPT        4
#define RBU_STAGE_DONE        5


#define RBU_CREATE_STATE \
  "CREATE TABLE IF NOT EXISTS %s.rbu_state(k INTEGER PRIMARY KEY, v)"

typedef struct RbuFrame RbuFrame;
typedef struct RbuObjIter RbuObjIter;
typedef struct RbuState RbuState;
typedef struct RbuSpan RbuSpan;
typedef struct rbu_vfs rbu_vfs;
typedef struct rbu_file rbu_file;
typedef struct RbuUpdateStmt RbuUpdateStmt;

#if !defined(SQLITE_AMALGAMATION)
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef sqlite3_int64 i64;
typedef sqlite3_uint64 u64;
#endif

/*
** These values must match the values defined in wal.c for the equivalent
** locks. These are not magic numbers as they are part of the SQLite file
** format.
*/
#define WAL_LOCK_WRITE  0
#define WAL_LOCK_CKPT   1
#define WAL_LOCK_READ0  3

#define SQLITE_FCNTL_RBUCNT    5149216

/*
** A structure to store values read from the rbu_state table in memory.
*/
struct RbuState {
  int eStage;
  char *zTbl;
  char *zDataTbl;
  char *zIdx;
  i64 iWalCksum;
  int nRow;
  i64 nProgress;
  u32 iCookie;
  i64 iOalSz;
  i64 nPhaseOneStep;
};

struct RbuUpdateStmt {
  char *zMask;                    /* Copy of update mask used with pUpdate */
  sqlite3_stmt *pUpdate;          /* Last update statement (or NULL) */
  RbuUpdateStmt *pNext;
};

struct RbuSpan {
  const char *zSpan;
  int nSpan;
};

/*
** An iterator of this type is used to iterate through all objects in
** the target database that require updating. For each such table, the
** iterator visits, in order:
**
**     * the table itself,
**     * each index of the table (zero or more points to visit), and
**     * a special "cleanup table" state.
**
** abIndexed:
**   If the table has no indexes on it, abIndexed is set to NULL. Otherwise,
**   it points to an array of flags nTblCol elements in size. The flag is
**   set for each column that is either a part of the PK or a part of an
**   index. Or clear otherwise.
**
**   If there are one or more partial indexes on the table, all fields of
**   this array set set to 1. This is because in that case, the module has
**   no way to tell which fields will be required to add and remove entries
**   from the partial indexes.
**
*/
struct RbuObjIter {
  sqlite3_stmt *pTblIter;         /* Iterate through tables */
  sqlite3_stmt *pIdxIter;         /* Index iterator */
  int nTblCol;                    /* Size of azTblCol[] array */
  char **azTblCol;                /* Array of unquoted target column names */
  char **azTblType;               /* Array of target column types */
  int *aiSrcOrder;                /* src table col -> target table col */
  u8 *abTblPk;                    /* Array of flags, set on target PK columns */
  u8 *abNotNull;                  /* Array of flags, set on NOT NULL columns */
  u8 *abIndexed;                  /* Array of flags, set on indexed & PK cols */
  int eType;                      /* Table type - an RBU_PK_XXX value */

  /* Output variables. zTbl==0 implies EOF. */
  int bCleanup;                   /* True in "cleanup" state */
  const char *zTbl;               /* Name of target db table */
  const char *zDataTbl;           /* Name of rbu db table (or null) */
  const char *zIdx;               /* Name of target db index (or null) */
  int iTnum;                      /* Root page of current object */
  int iPkTnum;                    /* If eType==EXTERNAL, root of PK index */
  int bUnique;                    /* Current index is unique */
  int nIndex;                     /* Number of aux. indexes on table zTbl */

  /* Statements created by rbuObjIterPrepareAll() */
  int nCol;                       /* Number of columns in current object */
  sqlite3_stmt *pSelect;          /* Source data */
  sqlite3_stmt *pInsert;          /* Statement for INSERT operations */
  sqlite3_stmt *pDelete;          /* Statement for DELETE ops */
  sqlite3_stmt *pTmpInsert;       /* Insert into rbu_tmp_$zDataTbl */
  int nIdxCol;
  RbuSpan *aIdxCol;
  char *zIdxSql;

  /* Last UPDATE used (for PK b-tree updates only), or NULL. */
  RbuUpdateStmt *pRbuUpdate;
};

/*
** Values for RbuObjIter.eType
**
**     0: Table does not exist (error)
**     1: Table has an implicit rowid.
**     2: Table has an explicit IPK column.
**     3: Table has an external PK index.
**     4: Table is WITHOUT ROWID.
**     5: Table is a virtual table.
*/
#define RBU_PK_NOTABLE        0
#define RBU_PK_NONE           1
#define RBU_PK_IPK            2
#define RBU_PK_EXTERNAL       3
#define RBU_PK_WITHOUT_ROWID  4
#define RBU_PK_VTAB           5


/*
** Within the RBU_STAGE_OAL stage, each call to sqlite3rbu_step() performs
** one of the following operations.
*/
#define RBU_INSERT     1          /* Insert on a main table b-tree */
#define RBU_DELETE     2          /* Delete a row from a main table b-tree */
#define RBU_REPLACE    3          /* Delete and then insert a row */
#define RBU_IDX_DELETE 4          /* Delete a row from an aux. index b-tree */
#define RBU_IDX_INSERT 5          /* Insert on an aux. index b-tree */

#define RBU_UPDATE     6          /* Update a row in a main table b-tree */

/*
** A single step of an incremental checkpoint - frame iWalFrame of the wal
** file should be copied to page iDbPage of the database file.
*/
struct RbuFrame {
  u32 iDbPage;
  u32 iWalFrame;
};

#ifndef UNUSED_PARAMETER
/*
** The following macros are used to suppress compiler warnings and to
** make it clear to human readers when a function parameter is deliberately
** left unused within the body of a function. This usually happens when
** a function is called via a function pointer. For example the
** implementation of an SQL aggregate step callback may not use the
** parameter indicating the number of arguments passed to the aggregate,
** if it knows that this is enforced elsewhere.
**
** When a function parameter is not used at all within the body of a function,
** it is generally named "NotUsed" or "NotUsed2" to make things even clearer.
** However, these macros may also be used to suppress warnings related to
** parameters that may or may not be used depending on compilation options.
** For example those parameters only used in assert() statements. In these
** cases the parameters are named as per the usual conventions.
*/
#define UNUSED_PARAMETER(x) (void)(x)
#define UNUSED_PARAMETER2(x,y) UNUSED_PARAMETER(x),UNUSED_PARAMETER(y)
#endif

/*
** RBU handle.
**
** nPhaseOneStep:
**   If the RBU database contains an rbu_count table, this value is set to
**   a running estimate of the number of b-tree operations required to
**   finish populating the *-oal file. This allows the sqlite3_bp_progress()
**   API to calculate the permyriadage progress of populating the *-oal file
**   using the formula:
**
**     permyriadage = (10000 * nProgress) / nPhaseOneStep
**
**   nPhaseOneStep is initialized to the sum of:
**
**     nRow * (nIndex + 1)
**
**   for all source tables in the RBU database, where nRow is the number
**   of rows in the source table and nIndex the number of indexes on the
**   corresponding target database table.
**
**   This estimate is accurate if the RBU update consists entirely of
**   INSERT operations. However, it is inaccurate if:
**
**     * the RBU update contains any UPDATE operations. If the PK specified
**       for an UPDATE operation does not exist in the target table, then
**       no b-tree operations are required on index b-trees. Or if the
**       specified PK does exist, then (nIndex*2) such operations are
**       required (one delete and one insert on each index b-tree).
**
**     * the RBU update contains any DELETE operations for which the specified
**       PK does not exist. In this case no operations are required on index
**       b-trees.
**
**     * the RBU update contains REPLACE operations. These are similar to
**       UPDATE operations.
**
**   nPhaseOneStep is updated to account for the conditions above during the
**   first pass of each source table. The updated nPhaseOneStep value is
**   stored in the rbu_state table if the RBU update is suspended.
*/
struct sqlite3rbu {
  int eStage;                     /* Value of RBU_STATE_STAGE field */
  sqlite3 *dbMain;                /* target database handle */
  sqlite3 *dbRbu;                 /* rbu database handle */
  char *zTarget;                  /* Path to target db */
  char *zRbu;                     /* Path to rbu db */
  char *zState;                   /* Path to state db (or NULL if zRbu) */
  char zStateDb[5];               /* Db name for state ("stat" or "main") */
  int rc;                         /* Value returned by last rbu_step() call */
  char *zErrmsg;                  /* Error message if rc!=SQLITE_OK */
  int nStep;                      /* Rows processed for current object */
  sqlite3_int64 nProgress;        /* Rows processed for all objects */
  RbuObjIter objiter;             /* Iterator for skipping through tbl/idx */
  const char *zVfsName;           /* Name of automatically created rbu vfs */
  rbu_file *pTargetFd;            /* File handle open on target db */
  int nPagePerSector;             /* Pages per sector for pTargetFd */
  i64 iOalSz;
  i64 nPhaseOneStep;
  void *pRenameArg;
  int (*xRename)(void*, const char*, const char*);

  /* The following state variables are used as part of the incremental
  ** checkpoint stage (eStage==RBU_STAGE_CKPT). See comments surrounding
  ** function rbuSetupCheckpoint() for details.  */
  u32 iMaxFrame;                  /* Largest iWalFrame value in aFrame[] */
  u32 mLock;
  int nFrame;                     /* Entries in aFrame[] array */
  int nFrameAlloc;                /* Allocated size of aFrame[] array */
  RbuFrame *aFrame;
  int pgsz;
  u8 *aBuf;
  i64 iWalCksum;
  i64 szTemp;                     /* Current size of all temp files in use */
  i64 szTempLimit;                /* Total size limit for temp files */

  /* Used in RBU vacuum mode only */
  int nRbu;                       /* Number of RBU VFS in the stack */
  rbu_file *pRbuFd;               /* Fd for main db of dbRbu */
};

/*
** An rbu VFS is implemented using an instance of this structure.
**
** Variable pRbu is only non-NULL for automatically created RBU VFS objects.
** It is NULL for RBU VFS objects created explicitly using
** sqlite3rbu_create_vfs(). It is used to track the total amount of temp
** space used by the RBU handle.
*/
struct rbu_vfs {
  sqlite3_vfs base;               /* rbu VFS shim methods */
  sqlite3_vfs *pRealVfs;          /* Underlying VFS */
  sqlite3_mutex *mutex;           /* Mutex to protect pMain */
  sqlite3rbu *pRbu;               /* Owner RBU object */
  rbu_file *pMain;                /* List of main db files */
  rbu_file *pMainRbu;             /* List of main db files with pRbu!=0 */
};

/*
** Each file opened by an rbu VFS is represented by an instance of
** the following structure.
**
** If this is a temporary file (pRbu!=0 && flags&DELETE_ON_CLOSE), variable
** "sz" is set to the current size of the database file.
*/
struct rbu_file {
  sqlite3_file base;              /* sqlite3_file methods */
  sqlite3_file *pReal;            /* Underlying file handle */
  rbu_vfs *pRbuVfs;               /* Pointer to the rbu_vfs object */
  sqlite3rbu *pRbu;               /* Pointer to rbu object (rbu target only) */
  i64 sz;                         /* Size of file in bytes (temp only) */

  int openFlags;                  /* Flags this file was opened with */
  u32 iCookie;                    /* Cookie value for main db files */
  u8 iWriteVer;                   /* "write-version" value for main db files */
  u8 bNolock;                     /* True to fail EXCLUSIVE locks */

  int nShm;                       /* Number of entries in apShm[] array */
  char **apShm;                   /* Array of mmap'd *-shm regions */
  char *zDel;                     /* Delete this when closing file */

  const char *zWal;               /* Wal filename for this main db file */
  rbu_file *pWalFd;               /* Wal file descriptor for this main db */
  rbu_file *pMainNext;            /* Next MAIN_DB file */
  rbu_file *pMainRbuNext;         /* Next MAIN_DB file with pRbu!=0 */
};

/*
** True for an RBU vacuum handle, or false otherwise.
*/
#define rbuIsVacuum(p) ((p)->zTarget==0)


/*************************************************************************
** The following three functions, found below:
**
**   rbuDeltaGetInt()
**   rbuDeltaChecksum()
**   rbuDeltaApply()
**
** are lifted from the fossil source code (http://fossil-scm.org). They
** are used to implement the scalar SQL function rbu_fossil_delta().
*/

/*
** Read bytes from *pz and convert them into a positive integer.  When
** finished, leave *pz pointing to the first character past the end of
** the integer.  The *pLen parameter holds the length of the string
** in *pz and is decremented once for each character in the integer.
*/
static unsigned int rbuDeltaGetInt(const char **pz, int *pLen){
  static const signed char zValue[] = {
    -1, -1, -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1, -1, -1, -1, -1,
     0,  1,  2,  3,  4,  5,  6,  7,    8,  9, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, 16,   17, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 27, 28, 29, 30, 31, 32,   33, 34, 35, -1, -1, -1, -1, 36,
    -1, 37, 38, 39, 40, 41, 42, 43,   44, 45, 46, 47, 48, 49, 50, 51,
    52, 53, 54, 55, 56, 57, 58, 59,   60, 61, 62, -1, -1, -1, 63, -1,
  };
  unsigned int v = 0;
  int c;
  unsigned char *z = (unsigned char*)*pz;
  unsigned char *zStart = z;
  while( (c = zValue[0x7f&*(z++)])>=0 ){
     v = (v<<6) + c;
  }
  z--;
  *pLen -= (int)(z - zStart);
  *pz = (char*)z;
  return v;
}

#if RBU_ENABLE_DELTA_CKSUM
/*
** Compute a 32-bit checksum on the N-byte buffer.  Return the result.
*/
static unsigned int rbuDeltaChecksum(const char *zIn, size_t N){
  const unsigned char *z = (const unsigned char *)zIn;
  unsigned sum0 = 0;
  unsigned sum1 = 0;
  unsigned sum2 = 0;
  unsigned sum3 = 0;
  while(N >= 16){
    sum0 += ((unsigned)z[0] + z[4] + z[8] + z[12]);
    sum1 += ((unsigned)z[1] + z[5] + z[9] + z[13]);
    sum2 += ((unsigned)z[2] + z[6] + z[10]+ z[14]);
    sum3 += ((unsigned)z[3] + z[7] + z[11]+ z[15]);
    z += 16;
    N -= 16;
  }
  while(N >= 4){
    sum0 += z[0];
    sum1 += z[1];
    sum2 += z[2];
    sum3 += z[3];
    z += 4;
    N -= 4;
  }
  sum3 += (sum2 << 8) + (sum1 << 16) + (sum0 << 24);
  switch(N){
    case 3:   sum3 += (z[2] << 8);
    case 2:   sum3 += (z[1] << 16);
    case 1:   sum3 += (z[0] << 24);
    default:  ;
  }
  return sum3;
}
#endif

/*
** Apply a delta.
**
** The output buffer should be big enough to hold the whole output
** file and a NUL terminator at the end.  The delta_output_size()
** routine will determine this size for you.
**
** The delta string should be null-terminated.  But the delta string
** may contain embedded NUL characters (if the input and output are
** binary files) so we also have to pass in the length of the delta in
** the lenDelta parameter.
**
** This function returns the size of the output file in bytes (excluding
** the final NUL terminator character).  Except, if the delta string is
** malformed or intended for use with a source file other than zSrc,
** then this routine returns -1.
**
** Refer to the delta_create() documentation above for a description
** of the delta file format.
*/
static int rbuDeltaApply(
  const char *zSrc,      /* The source or pattern file */
  int lenSrc,            /* Length of the source file */
  const char *zDelta,    /* Delta to apply to the pattern */
  int lenDelta,          /* Length of the delta */
  char *zOut             /* Write the output into this preallocated buffer */
){
  unsigned int limit;
  unsigned int total = 0;
#if RBU_ENABLE_DELTA_CKSUM
  char *zOrigOut = zOut;
#endif

  limit = rbuDeltaGetInt(&zDelta, &lenDelta);
  if( *zDelta!='\n' ){
    /* ERROR: size integer not terminated by "\n" */
    return -1;
  }
  zDelta++; lenDelta--;
  while( *zDelta && lenDelta>0 ){
    unsigned int cnt, ofst;
    cnt = rbuDeltaGetInt(&zDelta, &lenDelta);
    switch( zDelta[0] ){
      case '@': {
        zDelta++; lenDelta--;
        ofst = rbuDeltaGetInt(&zDelta, &lenDelta);
        if( lenDelta>0 && zDelta[0]!=',' ){
          /* ERROR: copy command not terminated by ',' */
          return -1;
        }
        zDelta++; lenDelta--;
        total += cnt;
        if( total>limit ){
          /* ERROR: copy exceeds output file size */
          return -1;
        }
        if( (u64)ofst+(u64)cnt > (u64)lenSrc ){
          /* ERROR: copy extends past end of input */
          return -1;
        }
        memcpy(zOut, &zSrc[ofst], cnt);
        zOut += cnt;
        break;
      }
      case ':': {
        zDelta++; lenDelta--;
        total += cnt;
        if( total>limit ){
          /* ERROR:  insert command gives an output larger than predicted */
          return -1;
        }
        if( (int)cnt>lenDelta ){
          /* ERROR: insert count exceeds size of delta */
          return -1;
        }
        memcpy(zOut, zDelta, cnt);
        zOut += cnt;
        zDelta += cnt;
        lenDelta -= cnt;
        break;
      }
      case ';': {
        zDelta++; lenDelta--;
        zOut[0] = 0;
#if RBU_ENABLE_DELTA_CKSUM
        if( cnt!=rbuDeltaChecksum(zOrigOut, total) ){
          /* ERROR:  bad checksum */
          return -1;
        }
#endif
        if( total!=limit ){
          /* ERROR: generated size does not match predicted size */
          return -1;
        }
        return total;
      }
      default: {
        /* ERROR: unknown delta operator */
        return -1;
      }
    }
  }
  /* ERROR: unterminated delta */
  return -1;
}

static int rbuDeltaOutputSize(const char *zDelta, int lenDelta){
  int size;
  size = rbuDeltaGetInt(&zDelta, &lenDelta);
  if( *zDelta!='\n' ){
    /* ERROR: size integer not terminated by "\n" */
    return -1;
  }
  return size;
}

/*
** End of code taken from fossil.
*************************************************************************/

/*
** Implementation of SQL scalar function rbu_fossil_delta().
**
** This function applies a fossil delta patch to a blob. Exactly two
** arguments must be passed to this function. The first is the blob to
** patch and the second the patch to apply. If no error occurs, this
** function returns the patched blob.
*/
static void rbuFossilDeltaFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  const char *aDelta;
  int nDelta;
  const char *aOrig;
  int nOrig;

  int nOut;
  int nOut2;
  char *aOut;

  assert( argc==2 );
  UNUSED_PARAMETER(argc);

  nOrig = sqlite3_value_bytes(argv[0]);
  aOrig = (const char*)sqlite3_value_blob(argv[0]);
  nDelta = sqlite3_value_bytes(argv[1]);
  aDelta = (const char*)sqlite3_value_blob(argv[1]);

  /* Figure out the size of the output */
  nOut = rbuDeltaOutputSize(aDelta, nDelta);
  if( nOut<0 ){
    sqlite3_result_error(context, "corrupt fossil delta", -1);
    return;
  }

  aOut = sqlite3_malloc(nOut+1);
  if( aOut==0 ){
    sqlite3_result_error_nomem(context);
  }else{
    nOut2 = rbuDeltaApply(aOrig, nOrig, aDelta, nDelta, aOut);
    if( nOut2!=nOut ){
      sqlite3_free(aOut);
      sqlite3_result_error(context, "corrupt fossil delta", -1);
    }else{
      sqlite3_result_blob(context, aOut, nOut, sqlite3_free);
    }
  }
}


/*
** Prepare the SQL statement in buffer zSql against database handle db.
** If successful, set *ppStmt to point to the new statement and return
** SQLITE_OK.
**
** Otherwise, if an error does occur, set *ppStmt to NULL and return
** an SQLite error code. Additionally, set output variable *pzErrmsg to
** point to a buffer containing an error message. It is the responsibility
** of the caller to (eventually) free this buffer using sqlite3_free().
*/
static int prepareAndCollectError(
  sqlite3 *db,
  sqlite3_stmt **ppStmt,
  char **pzErrmsg,
  const char *zSql
){
  int rc = sqlite3_prepare_v2(db, zSql, -1, ppStmt, 0);
  if( rc!=SQLITE_OK ){
    *pzErrmsg = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    *ppStmt = 0;
  }
  return rc;
}

/*
** Reset the SQL statement passed as the first argument. Return a copy
** of the value returned by sqlite3_reset().
**
** If an error has occurred, then set *pzErrmsg to point to a buffer
** containing an error message. It is the responsibility of the caller
** to eventually free this buffer using sqlite3_free().
*/
static int resetAndCollectError(sqlite3_stmt *pStmt, char **pzErrmsg){
  int rc = sqlite3_reset(pStmt);
  if( rc!=SQLITE_OK ){
    *pzErrmsg = sqlite3_mprintf("%s", sqlite3_errmsg(sqlite3_db_handle(pStmt)));
  }
  return rc;
}

/*
** Unless it is NULL, argument zSql points to a buffer allocated using
** sqlite3_malloc containing an SQL statement. This function prepares the SQL
** statement against database db and frees the buffer. If statement
** compilation is successful, *ppStmt is set to point to the new statement
** handle and SQLITE_OK is returned.
**
** Otherwise, if an error occurs, *ppStmt is set to NULL and an error code
** returned. In this case, *pzErrmsg may also be set to point to an error
** message. It is the responsibility of the caller to free this error message
** buffer using sqlite3_free().
**
** If argument zSql is NULL, this function assumes that an OOM has occurred.
** In this case SQLITE_NOMEM is returned and *ppStmt set to NULL.
*/
static int prepareFreeAndCollectError(
  sqlite3 *db,
  sqlite3_stmt **ppStmt,
  char **pzErrmsg,
  char *zSql
){
  int rc;
  assert( *pzErrmsg==0 );
  if( zSql==0 ){
    rc = SQLITE_NOMEM;
    *ppStmt = 0;
  }else{
    rc = prepareAndCollectError(db, ppStmt, pzErrmsg, zSql);
    sqlite3_free(zSql);
  }
  return rc;
}

/*
** Free the RbuObjIter.azTblCol[] and RbuObjIter.abTblPk[] arrays allocated
** by an earlier call to rbuObjIterCacheTableInfo().
*/
static void rbuObjIterFreeCols(RbuObjIter *pIter){
  int i;
  for(i=0; i<pIter->nTblCol; i++){
    sqlite3_free(pIter->azTblCol[i]);
    sqlite3_free(pIter->azTblType[i]);
  }
  sqlite3_free(pIter->azTblCol);
  pIter->azTblCol = 0;
  pIter->azTblType = 0;
  pIter->aiSrcOrder = 0;
  pIter->abTblPk = 0;
  pIter->abNotNull = 0;
  pIter->nTblCol = 0;
  pIter->eType = 0;               /* Invalid value */
}

/*
** Finalize all statements and free all allocations that are specific to
** the current object (table/index pair).
*/
static void rbuObjIterClearStatements(RbuObjIter *pIter){
  RbuUpdateStmt *pUp;

  sqlite3_finalize(pIter->pSelect);
  sqlite3_finalize(pIter->pInsert);
  sqlite3_finalize(pIter->pDelete);
  sqlite3_finalize(pIter->pTmpInsert);
  pUp = pIter->pRbuUpdate;
  while( pUp ){
    RbuUpdateStmt *pTmp = pUp->pNext;
    sqlite3_finalize(pUp->pUpdate);
    sqlite3_free(pUp);
    pUp = pTmp;
  }
  sqlite3_free(pIter->aIdxCol);
  sqlite3_free(pIter->zIdxSql);

  pIter->pSelect = 0;
  pIter->pInsert = 0;
  pIter->pDelete = 0;
  pIter->pRbuUpdate = 0;
  pIter->pTmpInsert = 0;
  pIter->nCol = 0;
  pIter->nIdxCol = 0;
  pIter->aIdxCol = 0;
  pIter->zIdxSql = 0;
}

/*
** Clean up any resources allocated as part of the iterator object passed
** as the only argument.
*/
static void rbuObjIterFinalize(RbuObjIter *pIter){
  rbuObjIterClearStatements(pIter);
  sqlite3_finalize(pIter->pTblIter);
  sqlite3_finalize(pIter->pIdxIter);
  rbuObjIterFreeCols(pIter);
  memset(pIter, 0, sizeof(RbuObjIter));
}

/*
** Advance the iterator to the next position.
**
** If no error occurs, SQLITE_OK is returned and the iterator is left
** pointing to the next entry. Otherwise, an error code and message is
** left in the RBU handle passed as the first argument. A copy of the
** error code is returned.
*/
static int rbuObjIterNext(sqlite3rbu *p, RbuObjIter *pIter){
  int rc = p->rc;
  if( rc==SQLITE_OK ){

    /* Free any SQLite statements used while processing the previous object */
    rbuObjIterClearStatements(pIter);
    if( pIter->zIdx==0 ){
      rc = sqlite3_exec(p->dbMain,
          "DROP TRIGGER IF EXISTS temp.rbu_insert_tr;"
          "DROP TRIGGER IF EXISTS temp.rbu_update1_tr;"
          "DROP TRIGGER IF EXISTS temp.rbu_update2_tr;"
          "DROP TRIGGER IF EXISTS temp.rbu_delete_tr;"
          , 0, 0, &p->zErrmsg
      );
    }

    if( rc==SQLITE_OK ){
      if( pIter->bCleanup ){
        rbuObjIterFreeCols(pIter);
        pIter->bCleanup = 0;
        rc = sqlite3_step(pIter->pTblIter);
        if( rc!=SQLITE_ROW ){
          rc = resetAndCollectError(pIter->pTblIter, &p->zErrmsg);
          pIter->zTbl = 0;
          pIter->zDataTbl = 0;
        }else{
          pIter->zTbl = (const char*)sqlite3_column_text(pIter->pTblIter, 0);
          pIter->zDataTbl = (const char*)sqlite3_column_text(pIter->pTblIter,1);
          rc = (pIter->zDataTbl && pIter->zTbl) ? SQLITE_OK : SQLITE_NOMEM;
        }
      }else{
        if( pIter->zIdx==0 ){
          sqlite3_stmt *pIdx = pIter->pIdxIter;
          rc = sqlite3_bind_text(pIdx, 1, pIter->zTbl, -1, SQLITE_STATIC);
        }
        if( rc==SQLITE_OK ){
          rc = sqlite3_step(pIter->pIdxIter);
          if( rc!=SQLITE_ROW ){
            rc = resetAndCollectError(pIter->pIdxIter, &p->zErrmsg);
            pIter->bCleanup = 1;
            pIter->zIdx = 0;
          }else{
            pIter->zIdx = (const char*)sqlite3_column_text(pIter->pIdxIter, 0);
            pIter->iTnum = sqlite3_column_int(pIter->pIdxIter, 1);
            pIter->bUnique = sqlite3_column_int(pIter->pIdxIter, 2);
            rc = pIter->zIdx ? SQLITE_OK : SQLITE_NOMEM;
          }
        }
      }
    }
  }

  if( rc!=SQLITE_OK ){
    rbuObjIterFinalize(pIter);
    p->rc = rc;
  }
  return rc;
}


/*
** The implementation of the rbu_target_name() SQL function. This function
** accepts one or two arguments. The first argument is the name of a table -
** the name of a table in the RBU database.  The second, if it is present, is 1
** for a view or 0 for a table.
**
** For a non-vacuum RBU handle, if the table name matches the pattern:
**
**     data[0-9]_<name>
**
** where <name> is any sequence of 1 or more characters, <name> is returned.
** Otherwise, if the only argument does not match the above pattern, an SQL
** NULL is returned.
**
**     "data_t1"     -> "t1"
**     "data0123_t2" -> "t2"
**     "dataAB_t3"   -> NULL
**
** For an rbu vacuum handle, a copy of the first argument is returned if
** the second argument is either missing or 0 (not a view).
*/
static void rbuTargetNameFunc(
  sqlite3_context *pCtx,
  int argc,
  sqlite3_value **argv
){
  sqlite3rbu *p = sqlite3_user_data(pCtx);
  const char *zIn;
  assert( argc==1 || argc==2 );

  zIn = (const char*)sqlite3_value_text(argv[0]);
  if( zIn ){
    if( rbuIsVacuum(p) ){
      assert( argc==2 || argc==1 );
      if( argc==1 || 0==sqlite3_value_int(argv[1]) ){
        sqlite3_result_text(pCtx, zIn, -1, SQLITE_STATIC);
      }
    }else{
      if( strlen(zIn)>4 && memcmp("data", zIn, 4)==0 ){
        int i;
        for(i=4; zIn[i]>='0' && zIn[i]<='9'; i++);
        if( zIn[i]=='_' && zIn[i+1] ){
          sqlite3_result_text(pCtx, &zIn[i+1], -1, SQLITE_STATIC);
        }
      }
    }
  }
}

/*
** Initialize the iterator structure passed as the second argument.
**
** If no error occurs, SQLITE_OK is returned and the iterator is left
** pointing to the first entry. Otherwise, an error code and message is
** left in the RBU handle passed as the first argument. A copy of the
** error code is returned.
*/
static int rbuObjIterFirst(sqlite3rbu *p, RbuObjIter *pIter){
  int rc;
  memset(pIter, 0, sizeof(RbuObjIter));

  rc = prepareFreeAndCollectError(p->dbRbu, &pIter->pTblIter, &p->zErrmsg,
    sqlite3_mprintf(
      "SELECT rbu_target_name(name, type='view') AS target, name "
      "FROM sqlite_schema "
      "WHERE type IN ('table', 'view') AND target IS NOT NULL "
      " %s "
      "ORDER BY name"
  , rbuIsVacuum(p) ? "AND rootpage!=0 AND rootpage IS NOT NULL" : ""));

  if( rc==SQLITE_OK ){
    rc = prepareAndCollectError(p->dbMain, &pIter->pIdxIter, &p->zErrmsg,
        "SELECT name, rootpage, sql IS NULL OR substr(8, 6)=='UNIQUE' "
        "  FROM main.sqlite_schema "
        "  WHERE type='index' AND tbl_name = ?"
    );
  }

  pIter->bCleanup = 1;
  p->rc = rc;
  return rbuObjIterNext(p, pIter);
}

/*
** This is a wrapper around "sqlite3_mprintf(zFmt, ...)". If an OOM occurs,
** an error code is stored in the RBU handle passed as the first argument.
**
** If an error has already occurred (p->rc is already set to something other
** than SQLITE_OK), then this function returns NULL without modifying the
** stored error code. In this case it still calls sqlite3_free() on any
** printf() parameters associated with %z conversions.
*/
static char *rbuMPrintf(sqlite3rbu *p, const char *zFmt, ...){
  char *zSql = 0;
  va_list ap;
  va_start(ap, zFmt);
  zSql = sqlite3_vmprintf(zFmt, ap);
  if( p->rc==SQLITE_OK ){
    if( zSql==0 ) p->rc = SQLITE_NOMEM;
  }else{
    sqlite3_free(zSql);
    zSql = 0;
  }
  va_end(ap);
  return zSql;
}

/*
** Argument zFmt is a sqlite3_mprintf() style format string. The trailing
** arguments are the usual subsitution values. This function performs
** the printf() style substitutions and executes the result as an SQL
** statement on the RBU handles database.
**
** If an error occurs, an error code and error message is stored in the
** RBU handle. If an error has already occurred when this function is
** called, it is a no-op.
*/
static int rbuMPrintfExec(sqlite3rbu *p, sqlite3 *db, const char *zFmt, ...){
  va_list ap;
  char *zSql;
  va_start(ap, zFmt);
  zSql = sqlite3_vmprintf(zFmt, ap);
  if( p->rc==SQLITE_OK ){
    if( zSql==0 ){
      p->rc = SQLITE_NOMEM;
    }else{
      p->rc = sqlite3_exec(db, zSql, 0, 0, &p->zErrmsg);
    }
  }
  sqlite3_free(zSql);
  va_end(ap);
  return p->rc;
}

/*
** Attempt to allocate and return a pointer to a zeroed block of nByte
** bytes.
**
** If an error (i.e. an OOM condition) occurs, return NULL and leave an
** error code in the rbu handle passed as the first argument. Or, if an
** error has already occurred when this function is called, return NULL
** immediately without attempting the allocation or modifying the stored
** error code.
*/
static void *rbuMalloc(sqlite3rbu *p, sqlite3_int64 nByte){
  void *pRet = 0;
  if( p->rc==SQLITE_OK ){
    assert( nByte>0 );
    pRet = sqlite3_malloc64(nByte);
    if( pRet==0 ){
      p->rc = SQLITE_NOMEM;
    }else{
      memset(pRet, 0, nByte);
    }
  }
  return pRet;
}


/*
** Allocate and zero the pIter->azTblCol[] and abTblPk[] arrays so that
** there is room for at least nCol elements. If an OOM occurs, store an
** error code in the RBU handle passed as the first argument.
*/
static void rbuAllocateIterArrays(sqlite3rbu *p, RbuObjIter *pIter, int nCol){
  sqlite3_int64 nByte = (2*sizeof(char*) + sizeof(int) + 3*sizeof(u8)) * nCol;
  char **azNew;

  azNew = (char**)rbuMalloc(p, nByte);
  if( azNew ){
    pIter->azTblCol = azNew;
    pIter->azTblType = &azNew[nCol];
    pIter->aiSrcOrder = (int*)&pIter->azTblType[nCol];
    pIter->abTblPk = (u8*)&pIter->aiSrcOrder[nCol];
    pIter->abNotNull = (u8*)&pIter->abTblPk[nCol];
    pIter->abIndexed = (u8*)&pIter->abNotNull[nCol];
  }
}

/*
** The first argument must be a nul-terminated string. This function
** returns a copy of the string in memory obtained from sqlite3_malloc().
** It is the responsibility of the caller to eventually free this memory
** using sqlite3_free().
**
** If an OOM condition is encountered when attempting to allocate memory,
** output variable (*pRc) is set to SQLITE_NOMEM before returning. Otherwise,
** if the allocation succeeds, (*pRc) is left unchanged.
*/
static char *rbuStrndup(const char *zStr, int *pRc){
  char *zRet = 0;

  if( *pRc==SQLITE_OK ){
    if( zStr ){
      size_t nCopy = strlen(zStr) + 1;
      zRet = (char*)sqlite3_malloc64(nCopy);
      if( zRet ){
        memcpy(zRet, zStr, nCopy);
      }else{
        *pRc = SQLITE_NOMEM;
      }
    }
  }

  return zRet;
}

/*
** Finalize the statement passed as the second argument.
**
** If the sqlite3_finalize() call indicates that an error occurs, and the
** rbu handle error code is not already set, set the error code and error
** message accordingly.
*/
static void rbuFinalize(sqlite3rbu *p, sqlite3_stmt *pStmt){
  sqlite3 *db = sqlite3_db_handle(pStmt);
  int rc = sqlite3_finalize(pStmt);
  if( p->rc==SQLITE_OK && rc!=SQLITE_OK ){
    p->rc = rc;
    p->zErrmsg = sqlite3_mprintf("%s", sqlite3_errmsg(db));
  }
}

/* Determine the type of a table.
**
**   peType is of type (int*), a pointer to an output parameter of type
**   (int). This call sets the output parameter as follows, depending
**   on the type of the table specified by parameters dbName and zTbl.
**
**     RBU_PK_NOTABLE:       No such table.
**     RBU_PK_NONE:          Table has an implicit rowid.
**     RBU_PK_IPK:           Table has an explicit IPK column.
**     RBU_PK_EXTERNAL:      Table has an external PK index.
**     RBU_PK_WITHOUT_ROWID: Table is WITHOUT ROWID.
**     RBU_PK_VTAB:          Table is a virtual table.
**
**   Argument *piPk is also of type (int*), and also points to an output
**   parameter. Unless the table has an external primary key index
**   (i.e. unless *peType is set to 3), then *piPk is set to zero. Or,
**   if the table does have an external primary key index, then *piPk
**   is set to the root page number of the primary key index before
**   returning.
**
** ALGORITHM:
**
**   if( no entry exists in sqlite_schema ){
**     return RBU_PK_NOTABLE
**   }else if( sql for the entry starts with "CREATE VIRTUAL" ){
**     return RBU_PK_VTAB
**   }else if( "PRAGMA index_list()" for the table contains a "pk" index ){
**     if( the index that is the pk exists in sqlite_schema ){
**       *piPK = rootpage of that index.
**       return RBU_PK_EXTERNAL
**     }else{
**       return RBU_PK_WITHOUT_ROWID
**     }
**   }else if( "PRAGMA table_info()" lists one or more "pk" columns ){
**     return RBU_PK_IPK
**   }else{
**     return RBU_PK_NONE
**   }
*/
static void rbuTableType(
  sqlite3rbu *p,
  const char *zTab,
  int *peType,
  int *piTnum,
  int *piPk
){
  /*
  ** 0) SELECT count(*) FROM sqlite_schema where name=%Q AND IsVirtual(%Q)
  ** 1) PRAGMA index_list = ?
  ** 2) SELECT count(*) FROM sqlite_schema where name=%Q
  ** 3) PRAGMA table_info = ?
  */
  sqlite3_stmt *aStmt[4] = {0, 0, 0, 0};

  *peType = RBU_PK_NOTABLE;
  *piPk = 0;

  assert( p->rc==SQLITE_OK );
  p->rc = prepareFreeAndCollectError(p->dbMain, &aStmt[0], &p->zErrmsg,
    sqlite3_mprintf(
          "SELECT "
          " (sql COLLATE nocase BETWEEN 'CREATE VIRTUAL' AND 'CREATE VIRTUAM'),"
          " rootpage"
          "  FROM sqlite_schema"
          " WHERE name=%Q", zTab
  ));
  if( p->rc!=SQLITE_OK || sqlite3_step(aStmt[0])!=SQLITE_ROW ){
    /* Either an error, or no such table. */
    goto rbuTableType_end;
  }
  if( sqlite3_column_int(aStmt[0], 0) ){
    *peType = RBU_PK_VTAB;                     /* virtual table */
    goto rbuTableType_end;
  }
  *piTnum = sqlite3_column_int(aStmt[0], 1);

  p->rc = prepareFreeAndCollectError(p->dbMain, &aStmt[1], &p->zErrmsg,
    sqlite3_mprintf("PRAGMA index_list=%Q",zTab)
  );
  if( p->rc ) goto rbuTableType_end;
  while( sqlite3_step(aStmt[1])==SQLITE_ROW ){
    const u8 *zOrig = sqlite3_column_text(aStmt[1], 3);
    const u8 *zIdx = sqlite3_column_text(aStmt[1], 1);
    if( zOrig && zIdx && zOrig[0]=='p' ){
      p->rc = prepareFreeAndCollectError(p->dbMain, &aStmt[2], &p->zErrmsg,
          sqlite3_mprintf(
            "SELECT rootpage FROM sqlite_schema WHERE name = %Q", zIdx
      ));
      if( p->rc==SQLITE_OK ){
        if( sqlite3_step(aStmt[2])==SQLITE_ROW ){
          *piPk = sqlite3_column_int(aStmt[2], 0);
          *peType = RBU_PK_EXTERNAL;
        }else{
          *peType = RBU_PK_WITHOUT_ROWID;
        }
      }
      goto rbuTableType_end;
    }
  }

  p->rc = prepareFreeAndCollectError(p->dbMain, &aStmt[3], &p->zErrmsg,
    sqlite3_mprintf("PRAGMA table_info=%Q",zTab)
  );
  if( p->rc==SQLITE_OK ){
    while( sqlite3_step(aStmt[3])==SQLITE_ROW ){
      if( sqlite3_column_int(aStmt[3],5)>0 ){
        *peType = RBU_PK_IPK;                /* explicit IPK column */
        goto rbuTableType_end;
      }
    }
    *peType = RBU_PK_NONE;
  }

rbuTableType_end: {
    unsigned int i;
    for(i=0; i<sizeof(aStmt)/sizeof(aStmt[0]); i++){
      rbuFinalize(p, aStmt[i]);
    }
  }
}

/*
** This is a helper function for rbuObjIterCacheTableInfo(). It populates
** the pIter->abIndexed[] array.
*/
static void rbuObjIterCacheIndexedCols(sqlite3rbu *p, RbuObjIter *pIter){
  sqlite3_stmt *pList = 0;
  int bIndex = 0;

  if( p->rc==SQLITE_OK ){
    memcpy(pIter->abIndexed, pIter->abTblPk, sizeof(u8)*pIter->nTblCol);
    p->rc = prepareFreeAndCollectError(p->dbMain, &pList, &p->zErrmsg,
        sqlite3_mprintf("PRAGMA main.index_list = %Q", pIter->zTbl)
    );
  }

  pIter->nIndex = 0;
  while( p->rc==SQLITE_OK && SQLITE_ROW==sqlite3_step(pList) ){
    const char *zIdx = (const char*)sqlite3_column_text(pList, 1);
    int bPartial = sqlite3_column_int(pList, 4);
    sqlite3_stmt *pXInfo = 0;
    if( zIdx==0 ) break;
    if( bPartial ){
      memset(pIter->abIndexed, 0x01, sizeof(u8)*pIter->nTblCol);
    }
    p->rc = prepareFreeAndCollectError(p->dbMain, &pXInfo, &p->zErrmsg,
        sqlite3_mprintf("PRAGMA main.index_xinfo = %Q", zIdx)
    );
    while( p->rc==SQLITE_OK && SQLITE_ROW==sqlite3_step(pXInfo) ){
      int iCid = sqlite3_column_int(pXInfo, 1);
      if( iCid>=0 ) pIter->abIndexed[iCid] = 1;
      if( iCid==-2 ){
        memset(pIter->abIndexed, 0x01, sizeof(u8)*pIter->nTblCol);
      }
    }
    rbuFinalize(p, pXInfo);
    bIndex = 1;
    pIter->nIndex++;
  }

  if( pIter->eType==RBU_PK_WITHOUT_ROWID ){
    /* "PRAGMA index_list" includes the main PK b-tree */
    pIter->nIndex--;
  }

  rbuFinalize(p, pList);
  if( bIndex==0 ) pIter->abIndexed = 0;
}


/*
** If they are not already populated, populate the pIter->azTblCol[],
** pIter->abTblPk[], pIter->nTblCol and pIter->bRowid variables according to
** the table (not index) that the iterator currently points to.
**
** Return SQLITE_OK if successful, or an SQLite error code otherwise. If
** an error does occur, an error code and error message are also left in
** the RBU handle.
*/
static int rbuObjIterCacheTableInfo(sqlite3rbu *p, RbuObjIter *pIter){
  if( pIter->azTblCol==0 ){
    sqlite3_stmt *pStmt = 0;
    int nCol = 0;
    int i;                        /* for() loop iterator variable */
    int bRbuRowid = 0;            /* If input table has column "rbu_rowid" */
    int iOrder = 0;
    int iTnum = 0;

    /* Figure out the type of table this step will deal with. */
    assert( pIter->eType==0 );
    rbuTableType(p, pIter->zTbl, &pIter->eType, &iTnum, &pIter->iPkTnum);
    if( p->rc==SQLITE_OK && pIter->eType==RBU_PK_NOTABLE ){
      p->rc = SQLITE_ERROR;
      p->zErrmsg = sqlite3_mprintf("no such table: %s", pIter->zTbl);
    }
    if( p->rc ) return p->rc;
    if( pIter->zIdx==0 ) pIter->iTnum = iTnum;

    assert( pIter->eType==RBU_PK_NONE || pIter->eType==RBU_PK_IPK
         || pIter->eType==RBU_PK_EXTERNAL || pIter->eType==RBU_PK_WITHOUT_ROWID
         || pIter->eType==RBU_PK_VTAB
    );

    /* Populate the azTblCol[] and nTblCol variables based on the columns
    ** of the input table. Ignore any input table columns that begin with
    ** "rbu_".  */
    p->rc = prepareFreeAndCollectError(p->dbRbu, &pStmt, &p->zErrmsg,
        sqlite3_mprintf("SELECT * FROM '%q'", pIter->zDataTbl)
    );
    if( p->rc==SQLITE_OK ){
      nCol = sqlite3_column_count(pStmt);
      rbuAllocateIterArrays(p, pIter, nCol);
    }
    for(i=0; p->rc==SQLITE_OK && i<nCol; i++){
      const char *zName = (const char*)sqlite3_column_name(pStmt, i);
      if( sqlite3_strnicmp("rbu_", zName, 4) ){
        char *zCopy = rbuStrndup(zName, &p->rc);
        pIter->aiSrcOrder[pIter->nTblCol] = pIter->nTblCol;
        pIter->azTblCol[pIter->nTblCol++] = zCopy;
      }
      else if( 0==sqlite3_stricmp("rbu_rowid", zName) ){
        bRbuRowid = 1;
      }
    }
    sqlite3_finalize(pStmt);
    pStmt = 0;

    if( p->rc==SQLITE_OK
     && rbuIsVacuum(p)==0
     && bRbuRowid!=(pIter->eType==RBU_PK_VTAB || pIter->eType==RBU_PK_NONE)
    ){
      p->rc = SQLITE_ERROR;
      p->zErrmsg = sqlite3_mprintf(
          "table %q %s rbu_rowid column", pIter->zDataTbl,
          (bRbuRowid ? "may not have" : "requires")
      );
    }

    /* Check that all non-HIDDEN columns in the destination table are also
    ** present in the input table. Populate the abTblPk[], azTblType[] and
    ** aiTblOrder[] arrays at the same time.  */
    if( p->rc==SQLITE_OK ){
      p->rc = prepareFreeAndCollectError(p->dbMain, &pStmt, &p->zErrmsg,
          sqlite3_mprintf("PRAGMA table_info(%Q)", pIter->zTbl)
      );
    }
    while( p->rc==SQLITE_OK && SQLITE_ROW==sqlite3_step(pStmt) ){
      const char *zName = (const char*)sqlite3_column_text(pStmt, 1);
      if( zName==0 ) break;  /* An OOM - finalize() below returns S_NOMEM */
      for(i=iOrder; i<pIter->nTblCol; i++){
        if( 0==strcmp(zName, pIter->azTblCol[i]) ) break;
      }
      if( i==pIter->nTblCol ){
        p->rc = SQLITE_ERROR;
        p->zErrmsg = sqlite3_mprintf("column missing from %q: %s",
            pIter->zDataTbl, zName
        );
      }else{
        int iPk = sqlite3_column_int(pStmt, 5);
        int bNotNull = sqlite3_column_int(pStmt, 3);
        const char *zType = (const char*)sqlite3_column_text(pStmt, 2);

        if( i!=iOrder ){
          SWAP(int, pIter->aiSrcOrder[i], pIter->aiSrcOrder[iOrder]);
          SWAP(char*, pIter->azTblCol[i], pIter->azTblCol[iOrder]);
        }

        pIter->azTblType[iOrder] = rbuStrndup(zType, &p->rc);
        assert( iPk>=0 );
        pIter->abTblPk[iOrder] = (u8)iPk;
        pIter->abNotNull[iOrder] = (u8)bNotNull || (iPk!=0);
        iOrder++;
      }
    }

    rbuFinalize(p, pStmt);
    rbuObjIterCacheIndexedCols(p, pIter);
    assert( pIter->eType!=RBU_PK_VTAB || pIter->abIndexed==0 );
    assert( pIter->eType!=RBU_PK_VTAB || pIter->nIndex==0 );
  }

  return p->rc;
}

/*
** This function constructs and returns a pointer to a nul-terminated
** string containing some SQL clause or list based on one or more of the
** column names currently stored in the pIter->azTblCol[] array.
*/
static char *rbuObjIterGetCollist(
  sqlite3rbu *p,                  /* RBU object */
  RbuObjIter *pIter               /* Object iterator for column names */
){
  char *zList = 0;
  const char *zSep = "";
  int i;
  for(i=0; i<pIter->nTblCol; i++){
    const char *z = pIter->azTblCol[i];
    zList = rbuMPrintf(p, "%z%s\"%w\"", zList, zSep, z);
    zSep = ", ";
  }
  return zList;
}

/*
** Return a comma separated list of the quoted PRIMARY KEY column names,
** in order, for the current table. Before each column name, add the text
** zPre. After each column name, add the zPost text. Use zSeparator as
** the separator text (usually ", ").
*/
static char *rbuObjIterGetPkList(
  sqlite3rbu *p,                  /* RBU object */
  RbuObjIter *pIter,              /* Object iterator for column names */
  const char *zPre,               /* Before each quoted column name */
  const char *zSeparator,         /* Separator to use between columns */
  const char *zPost               /* After each quoted column name */
){
  int iPk = 1;
  char *zRet = 0;
  const char *zSep = "";
  while( 1 ){
    int i;
    for(i=0; i<pIter->nTblCol; i++){
      if( (int)pIter->abTblPk[i]==iPk ){
        const char *zCol = pIter->azTblCol[i];
        zRet = rbuMPrintf(p, "%z%s%s\"%w\"%s", zRet, zSep, zPre, zCol, zPost);
        zSep = zSeparator;
        break;
      }
    }
    if( i==pIter->nTblCol ) break;
    iPk++;
  }
  return zRet;
}

/*
** This function is called as part of restarting an RBU vacuum within
** stage 1 of the process (while the *-oal file is being built) while
** updating a table (not an index). The table may be a rowid table or
** a WITHOUT ROWID table. It queries the target database to find the
** largest key that has already been written to the target table and
** constructs a WHERE clause that can be used to extract the remaining
** rows from the source table. For a rowid table, the WHERE clause
** is of the form:
**
**     "WHERE _rowid_ > ?"
**
** and for WITHOUT ROWID tables:
**
**     "WHERE (key1, key2) > (?, ?)"
**
** Instead of "?" placeholders, the actual WHERE clauses created by
** this function contain literal SQL values.
*/
static char *rbuVacuumTableStart(
  sqlite3rbu *p,                  /* RBU handle */
  RbuObjIter *pIter,              /* RBU iterator object */
  int bRowid,                     /* True for a rowid table */
  const char *zWrite              /* Target table name prefix */
){
  sqlite3_stmt *pMax = 0;
  char *zRet = 0;
  if( bRowid ){
    p->rc = prepareFreeAndCollectError(p->dbMain, &pMax, &p->zErrmsg,
        sqlite3_mprintf(
          "SELECT max(_rowid_) FROM \"%s%w\"", zWrite, pIter->zTbl
        )
    );
    if( p->rc==SQLITE_OK && SQLITE_ROW==sqlite3_step(pMax) ){
      sqlite3_int64 iMax = sqlite3_column_int64(pMax, 0);
      zRet = rbuMPrintf(p, " WHERE _rowid_ > %lld ", iMax);
    }
    rbuFinalize(p, pMax);
  }else{
    char *zOrder = rbuObjIterGetPkList(p, pIter, "", ", ", " DESC");
    char *zSelect = rbuObjIterGetPkList(p, pIter, "quote(", "||','||", ")");
    char *zList = rbuObjIterGetPkList(p, pIter, "", ", ", "");

    if( p->rc==SQLITE_OK ){
      p->rc = prepareFreeAndCollectError(p->dbMain, &pMax, &p->zErrmsg,
          sqlite3_mprintf(
            "SELECT %s FROM \"%s%w\" ORDER BY %s LIMIT 1",
                zSelect, zWrite, pIter->zTbl, zOrder
          )
      );
      if( p->rc==SQLITE_OK && SQLITE_ROW==sqlite3_step(pMax) ){
        const char *zVal = (const char*)sqlite3_column_text(pMax, 0);
        zRet = rbuMPrintf(p, " WHERE (%s) > (%s) ", zList, zVal);
      }
      rbuFinalize(p, pMax);
    }

    sqlite3_free(zOrder);
    sqlite3_free(zSelect);
    sqlite3_free(zList);
  }
  return zRet;
}

/*
** This function is called as part of restating an RBU vacuum when the
** current operation is writing content to an index. If possible, it
** queries the target index b-tree for the largest key already written to
** it, then composes and returns an expression that can be used in a WHERE
** clause to select the remaining required rows from the source table.
** It is only possible to return such an expression if:
**
**   * The index contains no DESC columns, and
**   * The last key written to the index before the operation was
**     suspended does not contain any NULL values.
**
** The expression is of the form:
**
**   (index-field1, index-field2, ...) > (?, ?, ...)
**
** except that the "?" placeholders are replaced with literal values.
**
** If the expression cannot be created, NULL is returned. In this case,
** the caller has to use an OFFSET clause to extract only the required
** rows from the sourct table, just as it does for an RBU update operation.
*/
static char *rbuVacuumIndexStart(
  sqlite3rbu *p,                  /* RBU handle */
  RbuObjIter *pIter               /* RBU iterator object */
){
  char *zOrder = 0;
  char *zLhs = 0;
  char *zSelect = 0;
  char *zVector = 0;
  char *zRet = 0;
  int bFailed = 0;
  const char *zSep = "";
  int iCol = 0;
  sqlite3_stmt *pXInfo = 0;

  p->rc = prepareFreeAndCollectError(p->dbMain, &pXInfo, &p->zErrmsg,
      sqlite3_mprintf("PRAGMA main.index_xinfo = %Q", pIter->zIdx)
  );
  while( p->rc==SQLITE_OK && SQLITE_ROW==sqlite3_step(pXInfo) ){
    int iCid = sqlite3_column_int(pXInfo, 1);
    const char *zCollate = (const char*)sqlite3_column_text(pXInfo, 4);
    const char *zCol;
    if( sqlite3_column_int(pXInfo, 3) ){
      bFailed = 1;
      break;
    }

    if( iCid<0 ){
      if( pIter->eType==RBU_PK_IPK ){
        int i;
        for(i=0; pIter->abTblPk[i]==0; i++);
        assert( i<pIter->nTblCol );
        zCol = pIter->azTblCol[i];
      }else{
        zCol = "_rowid_";
      }
    }else{
      zCol = pIter->azTblCol[iCid];
    }

    zLhs = rbuMPrintf(p, "%z%s \"%w\" COLLATE %Q",
        zLhs, zSep, zCol, zCollate
        );
    zOrder = rbuMPrintf(p, "%z%s \"rbu_imp_%d%w\" COLLATE %Q DESC",
        zOrder, zSep, iCol, zCol, zCollate
        );
    zSelect = rbuMPrintf(p, "%z%s quote(\"rbu_imp_%d%w\")",
        zSelect, zSep, iCol, zCol
        );
    zSep = ", ";
    iCol++;
  }
  rbuFinalize(p, pXInfo);
  if( bFailed ) goto index_start_out;

  if( p->rc==SQLITE_OK ){
    sqlite3_stmt *pSel = 0;

    p->rc = prepareFreeAndCollectError(p->dbMain, &pSel, &p->zErrmsg,
        sqlite3_mprintf("SELECT %s FROM \"rbu_imp_%w\" ORDER BY %s LIMIT 1",
          zSelect, pIter->zTbl, zOrder
        )
    );
    if( p->rc==SQLITE_OK && SQLITE_ROW==sqlite3_step(pSel) ){
      zSep = "";
      for(iCol=0; iCol<pIter->nCol; iCol++){
        const char *zQuoted = (const char*)sqlite3_column_text(pSel, iCol);
        if( zQuoted==0 ){
          p->rc = SQLITE_NOMEM;
        }else if( zQuoted[0]=='N' ){
          bFailed = 1;
          break;
        }
        zVector = rbuMPrintf(p, "%z%s%s", zVector, zSep, zQuoted);
        zSep = ", ";
      }

      if( !bFailed ){
        zRet = rbuMPrintf(p, "(%s) > (%s)", zLhs, zVector);
      }
    }
    rbuFinalize(p, pSel);
  }

 index_start_out:
  sqlite3_free(zOrder);
  sqlite3_free(zSelect);
  sqlite3_free(zVector);
  sqlite3_free(zLhs);
  return zRet;
}

/*
** This function is used to create a SELECT list (the list of SQL
** expressions that follows a SELECT keyword) for a SELECT statement
** used to read from an data_xxx or rbu_tmp_xxx table while updating the
** index object currently indicated by the iterator object passed as the
** second argument. A "PRAGMA index_xinfo = <idxname>" statement is used
** to obtain the required information.
**
** If the index is of the following form:
**
**   CREATE INDEX i1 ON t1(c, b COLLATE nocase);
**
** and "t1" is a table with an explicit INTEGER PRIMARY KEY column
** "ipk", the returned string is:
**
**   "`c` COLLATE 'BINARY', `b` COLLATE 'NOCASE', `ipk` COLLATE 'BINARY'"
**
** As well as the returned string, three other malloc'd strings are
** returned via output parameters. As follows:
**
**   pzImposterCols: ...
**   pzImposterPk: ...
**   pzWhere: ...
*/
static char *rbuObjIterGetIndexCols(
  sqlite3rbu *p,                  /* RBU object */
  RbuObjIter *pIter,              /* Object iterator for column names */
  char **pzImposterCols,          /* OUT: Columns for imposter table */
  char **pzImposterPk,            /* OUT: Imposter PK clause */
  char **pzWhere,                 /* OUT: WHERE clause */
  int *pnBind                     /* OUT: Trbul number of columns */
){
  int rc = p->rc;                 /* Error code */
  int rc2;                        /* sqlite3_finalize() return code */
  char *zRet = 0;                 /* String to return */
  char *zImpCols = 0;             /* String to return via *pzImposterCols */
  char *zImpPK = 0;               /* String to return via *pzImposterPK */
  char *zWhere = 0;               /* String to return via *pzWhere */
  int nBind = 0;                  /* Value to return via *pnBind */
  const char *zCom = "";          /* Set to ", " later on */
  const char *zAnd = "";          /* Set to " AND " later on */
  sqlite3_stmt *pXInfo = 0;       /* PRAGMA index_xinfo = ? */

  if( rc==SQLITE_OK ){
    assert( p->zErrmsg==0 );
    rc = prepareFreeAndCollectError(p->dbMain, &pXInfo, &p->zErrmsg,
        sqlite3_mprintf("PRAGMA main.index_xinfo = %Q", pIter->zIdx)
    );
  }

  while( rc==SQLITE_OK && SQLITE_ROW==sqlite3_step(pXInfo) ){
    int iCid = sqlite3_column_int(pXInfo, 1);
    int bDesc = sqlite3_column_int(pXInfo, 3);
    const char *zCollate = (const char*)sqlite3_column_text(pXInfo, 4);
    const char *zCol = 0;
    const char *zType;

    if( iCid==-2 ){
      int iSeq = sqlite3_column_int(pXInfo, 0);
      zRet = sqlite3_mprintf("%z%s(%.*s) COLLATE %Q", zRet, zCom,
          pIter->aIdxCol[iSeq].nSpan, pIter->aIdxCol[iSeq].zSpan, zCollate
      );
      zType = "";
    }else {
      if( iCid<0 ){
        /* An integer primary key. If the table has an explicit IPK, use
        ** its name. Otherwise, use "rbu_rowid".  */
        if( pIter->eType==RBU_PK_IPK ){
          int i;
          for(i=0; pIter->abTblPk[i]==0; i++);
          assert( i<pIter->nTblCol );
          zCol = pIter->azTblCol[i];
        }else if( rbuIsVacuum(p) ){
          zCol = "_rowid_";
        }else{
          zCol = "rbu_rowid";
        }
        zType = "INTEGER";
      }else{
        zCol = pIter->azTblCol[iCid];
        zType = pIter->azTblType[iCid];
      }
      zRet = sqlite3_mprintf("%z%s\"%w\" COLLATE %Q", zRet, zCom,zCol,zCollate);
    }

    if( pIter->bUnique==0 || sqlite3_column_int(pXInfo, 5) ){
      const char *zOrder = (bDesc ? " DESC" : "");
      zImpPK = sqlite3_mprintf("%z%s\"rbu_imp_%d%w\"%s",
          zImpPK, zCom, nBind, zCol, zOrder
      );
    }
    zImpCols = sqlite3_mprintf("%z%s\"rbu_imp_%d%w\" %s COLLATE %Q",
        zImpCols, zCom, nBind, zCol, zType, zCollate
    );
    zWhere = sqlite3_mprintf(
        "%z%s\"rbu_imp_%d%w\" IS ?", zWhere, zAnd, nBind, zCol
    );
    if( zRet==0 || zImpPK==0 || zImpCols==0 || zWhere==0 ) rc = SQLITE_NOMEM;
    zCom = ", ";
    zAnd = " AND ";
    nBind++;
  }

  rc2 = sqlite3_finalize(pXInfo);
  if( rc==SQLITE_OK ) rc = rc2;

  if( rc!=SQLITE_OK ){
    sqlite3_free(zRet);
    sqlite3_free(zImpCols);
    sqlite3_free(zImpPK);
    sqlite3_free(zWhere);
    zRet = 0;
    zImpCols = 0;
    zImpPK = 0;
    zWhere = 0;
    p->rc = rc;
  }

  *pzImposterCols = zImpCols;
  *pzImposterPk = zImpPK;
  *pzWhere = zWhere;
  *pnBind = nBind;
  return zRet;
}

/*
** Assuming the current table columns are "a", "b" and "c", and the zObj
** paramter is passed "old", return a string of the form:
**
**     "old.a, old.b, old.b"
**
** With the column names escaped.
**
** For tables with implicit rowids - RBU_PK_EXTERNAL and RBU_PK_NONE, append
** the text ", old._rowid_" to the returned value.
*/
static char *rbuObjIterGetOldlist(
  sqlite3rbu *p,
  RbuObjIter *pIter,
  const char *zObj
){
  char *zList = 0;
  if( p->rc==SQLITE_OK && pIter->abIndexed ){
    const char *zS = "";
    int i;
    for(i=0; i<pIter->nTblCol; i++){
      if( pIter->abIndexed[i] ){
        const char *zCol = pIter->azTblCol[i];
        zList = sqlite3_mprintf("%z%s%s.\"%w\"", zList, zS, zObj, zCol);
      }else{
        zList = sqlite3_mprintf("%z%sNULL", zList, zS);
      }
      zS = ", ";
      if( zList==0 ){
        p->rc = SQLITE_NOMEM;
        break;
      }
    }

    /* For a table with implicit rowids, append "old._rowid_" to the list. */
    if( pIter->eType==RBU_PK_EXTERNAL || pIter->eType==RBU_PK_NONE ){
      zList = rbuMPrintf(p, "%z, %s._rowid_", zList, zObj);
    }
  }
  return zList;
}

/*
** Return an expression that can be used in a WHERE clause to match the
** primary key of the current table. For example, if the table is:
**
**   CREATE TABLE t1(a, b, c, PRIMARY KEY(b, c));
**
** Return the string:
**
**   "b = ?1 AND c = ?2"
*/
static char *rbuObjIterGetWhere(
  sqlite3rbu *p,
  RbuObjIter *pIter
){
  char *zList = 0;
  if( pIter->eType==RBU_PK_VTAB || pIter->eType==RBU_PK_NONE ){
    zList = rbuMPrintf(p, "_rowid_ = ?%d", pIter->nTblCol+1);
  }else if( pIter->eType==RBU_PK_EXTERNAL ){
    const char *zSep = "";
    int i;
    for(i=0; i<pIter->nTblCol; i++){
      if( pIter->abTblPk[i] ){
        zList = rbuMPrintf(p, "%z%sc%d=?%d", zList, zSep, i, i+1);
        zSep = " AND ";
      }
    }
    zList = rbuMPrintf(p,
        "_rowid_ = (SELECT id FROM rbu_imposter2 WHERE %z)", zList
    );

  }else{
    const char *zSep = "";
    int i;
    for(i=0; i<pIter->nTblCol; i++){
      if( pIter->abTblPk[i] ){
        const char *zCol = pIter->azTblCol[i];
        zList = rbuMPrintf(p, "%z%s\"%w\"=?%d", zList, zSep, zCol, i+1);
        zSep = " AND ";
      }
    }
  }
  return zList;
}

/*
** The SELECT statement iterating through the keys for the current object
** (p->objiter.pSelect) currently points to a valid row. However, there
** is something wrong with the rbu_control value in the rbu_control value
** stored in the (p->nCol+1)'th column. Set the error code and error message
** of the RBU handle to something reflecting this.
*/
static void rbuBadControlError(sqlite3rbu *p){
  p->rc = SQLITE_ERROR;
  p->zErrmsg = sqlite3_mprintf("invalid rbu_control value");
}


/*
** Return a nul-terminated string containing the comma separated list of
** assignments that should be included following the "SET" keyword of
** an UPDATE statement used to update the table object that the iterator
** passed as the second argument currently points to if the rbu_control
** column of the data_xxx table entry is set to zMask.
**
** The memory for the returned string is obtained from sqlite3_malloc().
** It is the responsibility of the caller to eventually free it using
** sqlite3_free().
**
** If an OOM error is encountered when allocating space for the new
** string, an error code is left in the rbu handle passed as the first
** argument and NULL is returned. Or, if an error has already occurred
** when this function is called, NULL is returned immediately, without
** attempting the allocation or modifying the stored error code.
*/
static char *rbuObjIterGetSetlist(
  sqlite3rbu *p,
  RbuObjIter *pIter,
  const char *zMask
){
  char *zList = 0;
  if( p->rc==SQLITE_OK ){
    int i;

    if( (int)strlen(zMask)!=pIter->nTblCol ){
      rbuBadControlError(p);
    }else{
      const char *zSep = "";
      for(i=0; i<pIter->nTblCol; i++){
        char c = zMask[pIter->aiSrcOrder[i]];
        if( c=='x' ){
          zList = rbuMPrintf(p, "%z%s\"%w\"=?%d",
              zList, zSep, pIter->azTblCol[i], i+1
          );
          zSep = ", ";
        }
        else if( c=='d' ){
          zList = rbuMPrintf(p, "%z%s\"%w\"=rbu_delta(\"%w\", ?%d)",
              zList, zSep, pIter->azTblCol[i], pIter->azTblCol[i], i+1
          );
          zSep = ", ";
        }
        else if( c=='f' ){
          zList = rbuMPrintf(p, "%z%s\"%w\"=rbu_fossil_delta(\"%w\", ?%d)",
              zList, zSep, pIter->azTblCol[i], pIter->azTblCol[i], i+1
          );
          zSep = ", ";
        }
      }
    }
  }
  return zList;
}

/*
** Return a nul-terminated string consisting of nByte comma separated
** "?" expressions. For example, if nByte is 3, return a pointer to
** a buffer containing the string "?,?,?".
**
** The memory for the returned string is obtained from sqlite3_malloc().
** It is the responsibility of the caller to eventually free it using
** sqlite3_free().
**
** If an OOM error is encountered when allocating space for the new
** string, an error code is left in the rbu handle passed as the first
** argument and NULL is returned. Or, if an error has already occurred
** when this function is called, NULL is returned immediately, without
** attempting the allocation or modifying the stored error code.
*/
static char *rbuObjIterGetBindlist(sqlite3rbu *p, int nBind){
  char *zRet = 0;
  sqlite3_int64 nByte = 2*(sqlite3_int64)nBind + 1;

  zRet = (char*)rbuMalloc(p, nByte);
  if( zRet ){
    int i;
    for(i=0; i<nBind; i++){
      zRet[i*2] = '?';
      zRet[i*2+1] = (i+1==nBind) ? '\0' : ',';
    }
  }
  return zRet;
}

/*
** The iterator currently points to a table (not index) of type
** RBU_PK_WITHOUT_ROWID. This function creates the PRIMARY KEY
** declaration for the corresponding imposter table. For example,
** if the iterator points to a table created as:
**
**   CREATE TABLE t1(a, b, c, PRIMARY KEY(b, a DESC)) WITHOUT ROWID
**
** this function returns:
**
**   PRIMARY KEY("b", "a" DESC)
*/
static char *rbuWithoutRowidPK(sqlite3rbu *p, RbuObjIter *pIter){
  char *z = 0;
  assert( pIter->zIdx==0 );
  if( p->rc==SQLITE_OK ){
    const char *zSep = "PRIMARY KEY(";
    sqlite3_stmt *pXList = 0;     /* PRAGMA index_list = (pIter->zTbl) */
    sqlite3_stmt *pXInfo = 0;     /* PRAGMA index_xinfo = <pk-index> */

    p->rc = prepareFreeAndCollectError(p->dbMain, &pXList, &p->zErrmsg,
        sqlite3_mprintf("PRAGMA main.index_list = %Q", pIter->zTbl)
    );
    while( p->rc==SQLITE_OK && SQLITE_ROW==sqlite3_step(pXList) ){
      const char *zOrig = (const char*)sqlite3_column_text(pXList,3);
      if( zOrig && strcmp(zOrig, "pk")==0 ){
        const char *zIdx = (const char*)sqlite3_column_text(pXList,1);
        if( zIdx ){
          p->rc = prepareFreeAndCollectError(p->dbMain, &pXInfo, &p->zErrmsg,
              sqlite3_mprintf("PRAGMA main.index_xinfo = %Q", zIdx)
          );
        }
        break;
      }
    }
    rbuFinalize(p, pXList);

    while( p->rc==SQLITE_OK && SQLITE_ROW==sqlite3_step(pXInfo) ){
      if( sqlite3_column_int(pXInfo, 5) ){
        /* int iCid = sqlite3_column_int(pXInfo, 0); */
        const char *zCol = (const char*)sqlite3_column_text(pXInfo, 2);
        const char *zDesc = sqlite3_column_int(pXInfo, 3) ? " DESC" : "";
        z = rbuMPrintf(p, "%z%s\"%w\"%s", z, zSep, zCol, zDesc);
        zSep = ", ";
      }
    }
    z = rbuMPrintf(p, "%z)", z);
    rbuFinalize(p, pXInfo);
  }
  return z;
}

/*
** This function creates the second imposter table used when writing to
** a table b-tree where the table has an external primary key. If the
** iterator passed as the second argument does not currently point to
** a table (not index) with an external primary key, this function is a
** no-op.
**
** Assuming the iterator does point to a table with an external PK, this
** function creates a WITHOUT ROWID imposter table named "rbu_imposter2"
** used to access that PK index. For example, if the target table is
** declared as follows:
**
**   CREATE TABLE t1(a, b TEXT, c REAL, PRIMARY KEY(b, c));
**
** then the imposter table schema is:
**
**   CREATE TABLE rbu_imposter2(c1 TEXT, c2 REAL, id INTEGER) WITHOUT ROWID;
**
*/
static void rbuCreateImposterTable2(sqlite3rbu *p, RbuObjIter *pIter){
  if( p->rc==SQLITE_OK && pIter->eType==RBU_PK_EXTERNAL ){
    int tnum = pIter->iPkTnum;    /* Root page of PK index */
    sqlite3_stmt *pQuery = 0;     /* SELECT name ... WHERE rootpage = $tnum */
    const char *zIdx = 0;         /* Name of PK index */
    sqlite3_stmt *pXInfo = 0;     /* PRAGMA main.index_xinfo = $zIdx */
    const char *zComma = "";
    char *zCols = 0;              /* Used to build up list of table cols */
    char *zPk = 0;                /* Used to build up table PK declaration */

    /* Figure out the name of the primary key index for the current table.
    ** This is needed for the argument to "PRAGMA index_xinfo". Set
    ** zIdx to point to a nul-terminated string containing this name. */
    p->rc = prepareAndCollectError(p->dbMain, &pQuery, &p->zErrmsg,
        "SELECT name FROM sqlite_schema WHERE rootpage = ?"
    );
    if( p->rc==SQLITE_OK ){
      sqlite3_bind_int(pQuery, 1, tnum);
      if( SQLITE_ROW==sqlite3_step(pQuery) ){
        zIdx = (const char*)sqlite3_column_text(pQuery, 0);
      }
    }
    if( zIdx ){
      p->rc = prepareFreeAndCollectError(p->dbMain, &pXInfo, &p->zErrmsg,
          sqlite3_mprintf("PRAGMA main.index_xinfo = %Q", zIdx)
      );
    }
    rbuFinalize(p, pQuery);

    while( p->rc==SQLITE_OK && SQLITE_ROW==sqlite3_step(pXInfo) ){
      int bKey = sqlite3_column_int(pXInfo, 5);
      if( bKey ){
        int iCid = sqlite3_column_int(pXInfo, 1);
        int bDesc = sqlite3_column_int(pXInfo, 3);
        const char *zCollate = (const char*)sqlite3_column_text(pXInfo, 4);
        zCols = rbuMPrintf(p, "%z%sc%d %s COLLATE %Q", zCols, zComma,
            iCid, pIter->azTblType[iCid], zCollate
        );
        zPk = rbuMPrintf(p, "%z%sc%d%s", zPk, zComma, iCid, bDesc?" DESC":"");
        zComma = ", ";
      }
    }
    zCols = rbuMPrintf(p, "%z, id INTEGER", zCols);
    rbuFinalize(p, pXInfo);

    sqlite3_test_control(SQLITE_TESTCTRL_IMPOSTER, p->dbMain, "main", 1, tnum);
    rbuMPrintfExec(p, p->dbMain,
        "CREATE TABLE rbu_imposter2(%z, PRIMARY KEY(%z)) WITHOUT ROWID",
        zCols, zPk
    );
    sqlite3_test_control(SQLITE_TESTCTRL_IMPOSTER, p->dbMain, "main", 0, 0);
  }
}

/*
** If an error has already occurred when this function is called, it
** immediately returns zero (without doing any work). Or, if an error
** occurs during the execution of this function, it sets the error code
** in the sqlite3rbu object indicated by the first argument and returns
** zero.
**
** The iterator passed as the second argument is guaranteed to point to
** a table (not an index) when this function is called. This function
** attempts to create any imposter table required to write to the main
** table b-tree of the table before returning. Non-zero is returned if
** an imposter table are created, or zero otherwise.
**
** An imposter table is required in all cases except RBU_PK_VTAB. Only
** virtual tables are written to directly. The imposter table has the
** same schema as the actual target table (less any UNIQUE constraints).
** More precisely, the "same schema" means the same columns, types,
** collation sequences. For tables that do not have an external PRIMARY
** KEY, it also means the same PRIMARY KEY declaration.
*/
static void rbuCreateImposterTable(sqlite3rbu *p, RbuObjIter *pIter){
  if( p->rc==SQLITE_OK && pIter->eType!=RBU_PK_VTAB ){
    int tnum = pIter->iTnum;
    const char *zComma = "";
    char *zSql = 0;
    int iCol;
    sqlite3_test_control(SQLITE_TESTCTRL_IMPOSTER, p->dbMain, "main", 0, 1);

    for(iCol=0; p->rc==SQLITE_OK && iCol<pIter->nTblCol; iCol++){
      const char *zPk = "";
      const char *zCol = pIter->azTblCol[iCol];
      const char *zColl = 0;

      p->rc = sqlite3_table_column_metadata(
          p->dbMain, "main", pIter->zTbl, zCol, 0, &zColl, 0, 0, 0
      );

      if( pIter->eType==RBU_PK_IPK && pIter->abTblPk[iCol] ){
        /* If the target table column is an "INTEGER PRIMARY KEY", add
        ** "PRIMARY KEY" to the imposter table column declaration. */
        zPk = "PRIMARY KEY ";
      }
      zSql = rbuMPrintf(p, "%z%s\"%w\" %s %sCOLLATE %Q%s",
          zSql, zComma, zCol, pIter->azTblType[iCol], zPk, zColl,
          (pIter->abNotNull[iCol] ? " NOT NULL" : "")
      );
      zComma = ", ";
    }

    if( pIter->eType==RBU_PK_WITHOUT_ROWID ){
      char *zPk = rbuWithoutRowidPK(p, pIter);
      if( zPk ){
        zSql = rbuMPrintf(p, "%z, %z", zSql, zPk);
      }
    }

    sqlite3_test_control(SQLITE_TESTCTRL_IMPOSTER, p->dbMain, "main", 1, tnum);
    rbuMPrintfExec(p, p->dbMain, "CREATE TABLE \"rbu_imp_%w\"(%z)%s",
        pIter->zTbl, zSql,
        (pIter->eType==RBU_PK_WITHOUT_ROWID ? " WITHOUT ROWID" : "")
    );
    sqlite3_test_control(SQLITE_TESTCTRL_IMPOSTER, p->dbMain, "main", 0, 0);
  }
}

/*
** Prepare a statement used to insert rows into the "rbu_tmp_xxx" table.
** Specifically a statement of the form:
**
**     INSERT INTO rbu_tmp_xxx VALUES(?, ?, ? ...);
**
** The number of bound variables is equal to the number of columns in
** the target table, plus one (for the rbu_control column), plus one more
** (for the rbu_rowid column) if the target table is an implicit IPK or
** virtual table.
*/
static void rbuObjIterPrepareTmpInsert(
  sqlite3rbu *p,
  RbuObjIter *pIter,
  const char *zCollist,
  const char *zRbuRowid
){
  int bRbuRowid = (pIter->eType==RBU_PK_EXTERNAL || pIter->eType==RBU_PK_NONE);
  char *zBind = rbuObjIterGetBindlist(p, pIter->nTblCol + 1 + bRbuRowid);
  if( zBind ){
    assert( pIter->pTmpInsert==0 );
    p->rc = prepareFreeAndCollectError(
        p->dbRbu, &pIter->pTmpInsert, &p->zErrmsg, sqlite3_mprintf(
          "INSERT INTO %s.'rbu_tmp_%q'(rbu_control,%s%s) VALUES(%z)",
          p->zStateDb, pIter->zDataTbl, zCollist, zRbuRowid, zBind
    ));
  }
}

static void rbuTmpInsertFunc(
  sqlite3_context *pCtx,
  int nVal,
  sqlite3_value **apVal
){
  sqlite3rbu *p = sqlite3_user_data(pCtx);
  int rc = SQLITE_OK;
  int i;

  assert( sqlite3_value_int(apVal[0])!=0
      || p->objiter.eType==RBU_PK_EXTERNAL
      || p->objiter.eType==RBU_PK_NONE
  );
  if( sqlite3_value_int(apVal[0])!=0 ){
    p->nPhaseOneStep += p->objiter.nIndex;
  }

  for(i=0; rc==SQLITE_OK && i<nVal; i++){
    rc = sqlite3_bind_value(p->objiter.pTmpInsert, i+1, apVal[i]);
  }
  if( rc==SQLITE_OK ){
    sqlite3_step(p->objiter.pTmpInsert);
    rc = sqlite3_reset(p->objiter.pTmpInsert);
  }

  if( rc!=SQLITE_OK ){
    sqlite3_result_error_code(pCtx, rc);
  }
}

static char *rbuObjIterGetIndexWhere(sqlite3rbu *p, RbuObjIter *pIter){
  sqlite3_stmt *pStmt = 0;
  int rc = p->rc;
  char *zRet = 0;

  assert( pIter->zIdxSql==0 && pIter->nIdxCol==0 && pIter->aIdxCol==0 );

  if( rc==SQLITE_OK ){
    rc = prepareAndCollectError(p->dbMain, &pStmt, &p->zErrmsg,
        "SELECT trim(sql) FROM sqlite_schema WHERE type='index' AND name=?"
    );
  }
  if( rc==SQLITE_OK ){
    int rc2;
    rc = sqlite3_bind_text(pStmt, 1, pIter->zIdx, -1, SQLITE_STATIC);
    if( rc==SQLITE_OK && SQLITE_ROW==sqlite3_step(pStmt) ){
      char *zSql = (char*)sqlite3_column_text(pStmt, 0);
      if( zSql ){
        pIter->zIdxSql = zSql = rbuStrndup(zSql, &rc);
      }
      if( zSql ){
        int nParen = 0;           /* Number of open parenthesis */
        int i;
        int iIdxCol = 0;
        int nIdxAlloc = 0;
        for(i=0; zSql[i]; i++){
          char c = zSql[i];

          /* If necessary, grow the pIter->aIdxCol[] array */
          if( iIdxCol==nIdxAlloc ){
            RbuSpan *aIdxCol = (RbuSpan*)sqlite3_realloc64(
                pIter->aIdxCol, nIdxAlloc*sizeof(RbuSpan) + 16*sizeof(RbuSpan)
            );
            if( aIdxCol==0 ){
              rc = SQLITE_NOMEM;
              break;
            }
            pIter->aIdxCol = aIdxCol;
            nIdxAlloc += 16;
          }

          if( c=='(' ){
            if( nParen==0 ){
              assert( iIdxCol==0 );
              pIter->aIdxCol[0].zSpan = &zSql[i+1];
            }
            nParen++;
          }
          else if( c==')' ){
            nParen--;
            if( nParen==0 ){
              int nSpan = (int)(&zSql[i] - pIter->aIdxCol[iIdxCol].zSpan);
              pIter->aIdxCol[iIdxCol++].nSpan = nSpan;
              i++;
              break;
            }
          }else if( c==',' && nParen==1 ){
            int nSpan = (int)(&zSql[i] - pIter->aIdxCol[iIdxCol].zSpan);
            pIter->aIdxCol[iIdxCol++].nSpan = nSpan;
            pIter->aIdxCol[iIdxCol].zSpan = &zSql[i+1];
          }else if( c=='"' || c=='\'' || c=='`' ){
            for(i++; 1; i++){
              if( zSql[i]==c ){
                if( zSql[i+1]!=c ) break;
                i++;
              }
            }
          }else if( c=='[' ){
            for(i++; 1; i++){
              if( zSql[i]==']' ) break;
            }
          }else if( c=='-' && zSql[i+1]=='-' ){
            for(i=i+2; zSql[i] && zSql[i]!='\n'; i++);
            if( zSql[i]=='\0' ) break;
          }else if( c=='/' && zSql[i+1]=='*' ){
            for(i=i+2; zSql[i] && (zSql[i]!='*' || zSql[i+1]!='/'); i++);
            if( zSql[i]=='\0' ) break;
            i++;
          }
        }
        if( zSql[i] ){
          zRet = rbuStrndup(&zSql[i], &rc);
        }
        pIter->nIdxCol = iIdxCol;
      }
    }

    rc2 = sqlite3_finalize(pStmt);
    if( rc==SQLITE_OK ) rc = rc2;
  }

  p->rc = rc;
  return zRet;
}

/*
** Ensure that the SQLite statement handles required to update the
** target database object currently indicated by the iterator passed
** as the second argument are available.
*/
static int rbuObjIterPrepareAll(
  sqlite3rbu *p,
  RbuObjIter *pIter,
  int nOffset                     /* Add "LIMIT -1 OFFSET $nOffset" to SELECT */
){
  assert( pIter->bCleanup==0 );
  if( pIter->pSelect==0 && rbuObjIterCacheTableInfo(p, pIter)==SQLITE_OK ){
    const int tnum = pIter->iTnum;
    char *zCollist = 0;           /* List of indexed columns */
    char **pz = &p->zErrmsg;
    const char *zIdx = pIter->zIdx;
    char *zLimit = 0;

    if( nOffset ){
      zLimit = sqlite3_mprintf(" LIMIT -1 OFFSET %d", nOffset);
      if( !zLimit ) p->rc = SQLITE_NOMEM;
    }

    if( zIdx ){
      const char *zTbl = pIter->zTbl;
      char *zImposterCols = 0;    /* Columns for imposter table */
      char *zImposterPK = 0;      /* Primary key declaration for imposter */
      char *zWhere = 0;           /* WHERE clause on PK columns */
      char *zBind = 0;
      char *zPart = 0;
      int nBind = 0;

      assert( pIter->eType!=RBU_PK_VTAB );
      zPart = rbuObjIterGetIndexWhere(p, pIter);
      zCollist = rbuObjIterGetIndexCols(
          p, pIter, &zImposterCols, &zImposterPK, &zWhere, &nBind
      );
      zBind = rbuObjIterGetBindlist(p, nBind);

      /* Create the imposter table used to write to this index. */
      sqlite3_test_control(SQLITE_TESTCTRL_IMPOSTER, p->dbMain, "main", 0, 1);
      sqlite3_test_control(SQLITE_TESTCTRL_IMPOSTER, p->dbMain, "main", 1,tnum);
      rbuMPrintfExec(p, p->dbMain,
          "CREATE TABLE \"rbu_imp_%w\"( %s, PRIMARY KEY( %s ) ) WITHOUT ROWID",
          zTbl, zImposterCols, zImposterPK
      );
      sqlite3_test_control(SQLITE_TESTCTRL_IMPOSTER, p->dbMain, "main", 0, 0);

      /* Create the statement to insert index entries */
      pIter->nCol = nBind;
      if( p->rc==SQLITE_OK ){
        p->rc = prepareFreeAndCollectError(
            p->dbMain, &pIter->pInsert, &p->zErrmsg,
          sqlite3_mprintf("INSERT INTO \"rbu_imp_%w\" VALUES(%s)", zTbl, zBind)
        );
      }

      /* And to delete index entries */
      if( rbuIsVacuum(p)==0 && p->rc==SQLITE_OK ){
        p->rc = prepareFreeAndCollectError(
            p->dbMain, &pIter->pDelete, &p->zErrmsg,
          sqlite3_mprintf("DELETE FROM \"rbu_imp_%w\" WHERE %s", zTbl, zWhere)
        );
      }

      /* Create the SELECT statement to read keys in sorted order */
      if( p->rc==SQLITE_OK ){
        char *zSql;
        if( rbuIsVacuum(p) ){
          char *zStart = 0;
          if( nOffset ){
            zStart = rbuVacuumIndexStart(p, pIter);
            if( zStart ){
              sqlite3_free(zLimit);
              zLimit = 0;
            }
          }

          zSql = sqlite3_mprintf(
              "SELECT %s, 0 AS rbu_control FROM '%q' %s %s %s ORDER BY %s%s",
              zCollist,
              pIter->zDataTbl,
              zPart,
              (zStart ? (zPart ? "AND" : "WHERE") : ""), zStart,
              zCollist, zLimit
          );
          sqlite3_free(zStart);
        }else

        if( pIter->eType==RBU_PK_EXTERNAL || pIter->eType==RBU_PK_NONE ){
          zSql = sqlite3_mprintf(
              "SELECT %s, rbu_control FROM %s.'rbu_tmp_%q' %s ORDER BY %s%s",
              zCollist, p->zStateDb, pIter->zDataTbl,
              zPart, zCollist, zLimit
          );
        }else{
          zSql = sqlite3_mprintf(
              "SELECT %s, rbu_control FROM %s.'rbu_tmp_%q' %s "
              "UNION ALL "
              "SELECT %s, rbu_control FROM '%q' "
              "%s %s typeof(rbu_control)='integer' AND rbu_control!=1 "
              "ORDER BY %s%s",
              zCollist, p->zStateDb, pIter->zDataTbl, zPart,
              zCollist, pIter->zDataTbl,
              zPart,
              (zPart ? "AND" : "WHERE"),
              zCollist, zLimit
          );
        }
        if( p->rc==SQLITE_OK ){
          p->rc = prepareFreeAndCollectError(p->dbRbu,&pIter->pSelect,pz,zSql);
        }else{
          sqlite3_free(zSql);
        }
      }

      sqlite3_free(zImposterCols);
      sqlite3_free(zImposterPK);
      sqlite3_free(zWhere);
      sqlite3_free(zBind);
      sqlite3_free(zPart);
    }else{
      int bRbuRowid = (pIter->eType==RBU_PK_VTAB)
                    ||(pIter->eType==RBU_PK_NONE)
                    ||(pIter->eType==RBU_PK_EXTERNAL && rbuIsVacuum(p));
      const char *zTbl = pIter->zTbl;       /* Table this step applies to */
      const char *zWrite;                   /* Imposter table name */

      char *zBindings = rbuObjIterGetBindlist(p, pIter->nTblCol + bRbuRowid);
      char *zWhere = rbuObjIterGetWhere(p, pIter);
      char *zOldlist = rbuObjIterGetOldlist(p, pIter, "old");
      char *zNewlist = rbuObjIterGetOldlist(p, pIter, "new");

      zCollist = rbuObjIterGetCollist(p, pIter);
      pIter->nCol = pIter->nTblCol;

      /* Create the imposter table or tables (if required). */
      rbuCreateImposterTable(p, pIter);
      rbuCreateImposterTable2(p, pIter);
      zWrite = (pIter->eType==RBU_PK_VTAB ? "" : "rbu_imp_");

      /* Create the INSERT statement to write to the target PK b-tree */
      if( p->rc==SQLITE_OK ){
        p->rc = prepareFreeAndCollectError(p->dbMain, &pIter->pInsert, pz,
            sqlite3_mprintf(
              "INSERT INTO \"%s%w\"(%s%s) VALUES(%s)",
              zWrite, zTbl, zCollist, (bRbuRowid ? ", _rowid_" : ""), zBindings
            )
        );
      }

      /* Create the DELETE statement to write to the target PK b-tree.
      ** Because it only performs INSERT operations, this is not required for
      ** an rbu vacuum handle.  */
      if( rbuIsVacuum(p)==0 && p->rc==SQLITE_OK ){
        p->rc = prepareFreeAndCollectError(p->dbMain, &pIter->pDelete, pz,
            sqlite3_mprintf(
              "DELETE FROM \"%s%w\" WHERE %s", zWrite, zTbl, zWhere
            )
        );
      }

      if( rbuIsVacuum(p)==0 && pIter->abIndexed ){
        const char *zRbuRowid = "";
        if( pIter->eType==RBU_PK_EXTERNAL || pIter->eType==RBU_PK_NONE ){
          zRbuRowid = ", rbu_rowid";
        }

        /* Create the rbu_tmp_xxx table and the triggers to populate it. */
        rbuMPrintfExec(p, p->dbRbu,
            "CREATE TABLE IF NOT EXISTS %s.'rbu_tmp_%q' AS "
            "SELECT *%s FROM '%q' WHERE 0;"
            , p->zStateDb, pIter->zDataTbl
            , (pIter->eType==RBU_PK_EXTERNAL ? ", 0 AS rbu_rowid" : "")
            , pIter->zDataTbl
        );

        rbuMPrintfExec(p, p->dbMain,
            "CREATE TEMP TRIGGER rbu_delete_tr BEFORE DELETE ON \"%s%w\" "
            "BEGIN "
            "  SELECT rbu_tmp_insert(3, %s);"
            "END;"

            "CREATE TEMP TRIGGER rbu_update1_tr BEFORE UPDATE ON \"%s%w\" "
            "BEGIN "
            "  SELECT rbu_tmp_insert(3, %s);"
            "END;"

            "CREATE TEMP TRIGGER rbu_update2_tr AFTER UPDATE ON \"%s%w\" "
            "BEGIN "
            "  SELECT rbu_tmp_insert(4, %s);"
            "END;",
            zWrite, zTbl, zOldlist,
            zWrite, zTbl, zOldlist,
            zWrite, zTbl, zNewlist
        );

        if( pIter->eType==RBU_PK_EXTERNAL || pIter->eType==RBU_PK_NONE ){
          rbuMPrintfExec(p, p->dbMain,
              "CREATE TEMP TRIGGER rbu_insert_tr AFTER INSERT ON \"%s%w\" "
              "BEGIN "
              "  SELECT rbu_tmp_insert(0, %s);"
              "END;",
              zWrite, zTbl, zNewlist
          );
        }

        rbuObjIterPrepareTmpInsert(p, pIter, zCollist, zRbuRowid);
      }

      /* Create the SELECT statement to read keys from data_xxx */
      if( p->rc==SQLITE_OK ){
        const char *zRbuRowid = "";
        char *zStart = 0;
        char *zOrder = 0;
        if( bRbuRowid ){
          zRbuRowid = rbuIsVacuum(p) ? ",_rowid_ " : ",rbu_rowid";
        }

        if( rbuIsVacuum(p) ){
          if( nOffset ){
            zStart = rbuVacuumTableStart(p, pIter, bRbuRowid, zWrite);
            if( zStart ){
              sqlite3_free(zLimit);
              zLimit = 0;
            }
          }
          if( bRbuRowid ){
            zOrder = rbuMPrintf(p, "_rowid_");
          }else{
            zOrder = rbuObjIterGetPkList(p, pIter, "", ", ", "");
          }
        }

        if( p->rc==SQLITE_OK ){
          p->rc = prepareFreeAndCollectError(p->dbRbu, &pIter->pSelect, pz,
              sqlite3_mprintf(
                "SELECT %s,%s rbu_control%s FROM '%q'%s %s %s %s",
                zCollist,
                (rbuIsVacuum(p) ? "0 AS " : ""),
                zRbuRowid,
                pIter->zDataTbl, (zStart ? zStart : ""),
                (zOrder ? "ORDER BY" : ""), zOrder,
                zLimit
              )
          );
        }
        sqlite3_free(zStart);
        sqlite3_free(zOrder);
      }

      sqlite3_free(zWhere);
      sqlite3_free(zOldlist);
      sqlite3_free(zNewlist);
      sqlite3_free(zBindings);
    }
    sqlite3_free(zCollist);
    sqlite3_free(zLimit);
  }

  return p->rc;
}

/*
** Set output variable *ppStmt to point to an UPDATE statement that may
** be used to update the imposter table for the main table b-tree of the
** table object that pIter currently points to, assuming that the
** rbu_control column of the data_xyz table contains zMask.
**
** If the zMask string does not specify any columns to update, then this
** is not an error. Output variable *ppStmt is set to NULL in this case.
*/
static int rbuGetUpdateStmt(
  sqlite3rbu *p,                  /* RBU handle */
  RbuObjIter *pIter,              /* Object iterator */
  const char *zMask,              /* rbu_control value ('x.x.') */
  sqlite3_stmt **ppStmt           /* OUT: UPDATE statement handle */
){
  RbuUpdateStmt **pp;
  RbuUpdateStmt *pUp = 0;
  int nUp = 0;

  /* In case an error occurs */
  *ppStmt = 0;

  /* Search for an existing statement. If one is found, shift it to the front
  ** of the LRU queue and return immediately. Otherwise, leave nUp pointing
  ** to the number of statements currently in the cache and pUp to the
  ** last object in the list.  */
  for(pp=&pIter->pRbuUpdate; *pp; pp=&((*pp)->pNext)){
    pUp = *pp;
    if( strcmp(pUp->zMask, zMask)==0 ){
      *pp = pUp->pNext;
      pUp->pNext = pIter->pRbuUpdate;
      pIter->pRbuUpdate = pUp;
      *ppStmt = pUp->pUpdate;
      return SQLITE_OK;
    }
    nUp++;
  }
  assert( pUp==0 || pUp->pNext==0 );

  if( nUp>=SQLITE_RBU_UPDATE_CACHESIZE ){
    for(pp=&pIter->pRbuUpdate; *pp!=pUp; pp=&((*pp)->pNext));
    *pp = 0;
    sqlite3_finalize(pUp->pUpdate);
    pUp->pUpdate = 0;
  }else{
    pUp = (RbuUpdateStmt*)rbuMalloc(p, sizeof(RbuUpdateStmt)+pIter->nTblCol+1);
  }

  if( pUp ){
    char *zWhere = rbuObjIterGetWhere(p, pIter);
    char *zSet = rbuObjIterGetSetlist(p, pIter, zMask);
    char *zUpdate = 0;

    pUp->zMask = (char*)&pUp[1];
    memcpy(pUp->zMask, zMask, pIter->nTblCol);
    pUp->pNext = pIter->pRbuUpdate;
    pIter->pRbuUpdate = pUp;

    if( zSet ){
      const char *zPrefix = "";

      if( pIter->eType!=RBU_PK_VTAB ) zPrefix = "rbu_imp_";
      zUpdate = sqlite3_mprintf("UPDATE \"%s%w\" SET %s WHERE %s",
          zPrefix, pIter->zTbl, zSet, zWhere
      );
      p->rc = prepareFreeAndCollectError(
          p->dbMain, &pUp->pUpdate, &p->zErrmsg, zUpdate
      );
      *ppStmt = pUp->pUpdate;
    }
    sqlite3_free(zWhere);
    sqlite3_free(zSet);
  }

  return p->rc;
}

static sqlite3 *rbuOpenDbhandle(
  sqlite3rbu *p,
  const char *zName,
  int bUseVfs
){
  sqlite3 *db = 0;
  if( p->rc==SQLITE_OK ){
    const int flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_URI;
    p->rc = sqlite3_open_v2(zName, &db, flags, bUseVfs ? p->zVfsName : 0);
    if( p->rc ){
      p->zErrmsg = sqlite3_mprintf("%s", sqlite3_errmsg(db));
      sqlite3_close(db);
      db = 0;
    }
  }
  return db;
}

/*
** Free an RbuState object allocated by rbuLoadState().
*/
static void rbuFreeState(RbuState *p){
  if( p ){
    sqlite3_free(p->zTbl);
    sqlite3_free(p->zDataTbl);
    sqlite3_free(p->zIdx);
    sqlite3_free(p);
  }
}

/*
** Allocate an RbuState object and load the contents of the rbu_state
** table into it. Return a pointer to the new object. It is the
** responsibility of the caller to eventually free the object using
** sqlite3_free().
**
** If an error occurs, leave an error code and message in the rbu handle
** and return NULL.
*/
static RbuState *rbuLoadState(sqlite3rbu *p){
  RbuState *pRet = 0;
  sqlite3_stmt *pStmt = 0;
  int rc;
  int rc2;

  pRet = (RbuState*)rbuMalloc(p, sizeof(RbuState));
  if( pRet==0 ) return 0;

  rc = prepareFreeAndCollectError(p->dbRbu, &pStmt, &p->zErrmsg,
      sqlite3_mprintf("SELECT k, v FROM %s.rbu_state", p->zStateDb)
  );
  while( rc==SQLITE_OK && SQLITE_ROW==sqlite3_step(pStmt) ){
    switch( sqlite3_column_int(pStmt, 0) ){
      case RBU_STATE_STAGE:
        pRet->eStage = sqlite3_column_int(pStmt, 1);
        if( pRet->eStage!=RBU_STAGE_OAL
         && pRet->eStage!=RBU_STAGE_MOVE
         && pRet->eStage!=RBU_STAGE_CKPT
        ){
          p->rc = SQLITE_CORRUPT;
        }
        break;

      case RBU_STATE_TBL:
        pRet->zTbl = rbuStrndup((char*)sqlite3_column_text(pStmt, 1), &rc);
        break;

      case RBU_STATE_IDX:
        pRet->zIdx = rbuStrndup((char*)sqlite3_column_text(pStmt, 1), &rc);
        break;

      case RBU_STATE_ROW:
        pRet->nRow = sqlite3_column_int(pStmt, 1);
        break;

      case RBU_STATE_PROGRESS:
        pRet->nProgress = sqlite3_column_int64(pStmt, 1);
        break;

      case RBU_STATE_CKPT:
        pRet->iWalCksum = sqlite3_column_int64(pStmt, 1);
        break;

      case RBU_STATE_COOKIE:
        pRet->iCookie = (u32)sqlite3_column_int64(pStmt, 1);
        break;

      case RBU_STATE_OALSZ:
        pRet->iOalSz = sqlite3_column_int64(pStmt, 1);
        break;

      case RBU_STATE_PHASEONESTEP:
        pRet->nPhaseOneStep = sqlite3_column_int64(pStmt, 1);
        break;

      case RBU_STATE_DATATBL:
        pRet->zDataTbl = rbuStrndup((char*)sqlite3_column_text(pStmt, 1), &rc);
        break;

      default:
        rc = SQLITE_CORRUPT;
        break;
    }
  }
  rc2 = sqlite3_finalize(pStmt);
  if( rc==SQLITE_OK ) rc = rc2;

  p->rc = rc;
  return pRet;
}


/*
** Open the database handle and attach the RBU database as "rbu". If an
** error occurs, leave an error code and message in the RBU handle.
**
** If argument dbMain is not NULL, then it is a database handle already
** open on the target database. Use this handle instead of opening a new
** one.
*/
static void rbuOpenDatabase(sqlite3rbu *p, sqlite3 *dbMain, int *pbRetry){
  assert( p->rc || (p->dbMain==0 && p->dbRbu==0) );
  assert( p->rc || rbuIsVacuum(p) || p->zTarget!=0 );
  assert( dbMain==0 || rbuIsVacuum(p)==0 );

  /* Open the RBU database */
  p->dbRbu = rbuOpenDbhandle(p, p->zRbu, 1);
  p->dbMain = dbMain;

  if( p->rc==SQLITE_OK && rbuIsVacuum(p) ){
    sqlite3_file_control(p->dbRbu, "main", SQLITE_FCNTL_RBUCNT, (void*)p);
    if( p->zState==0 ){
      const char *zFile = sqlite3_db_filename(p->dbRbu, "main");
      p->zState = rbuMPrintf(p, "file:///%s-vacuum?modeof=%s", zFile, zFile);
    }
  }

  /* If using separate RBU and state databases, attach the state database to
  ** the RBU db handle now.  */
  if( p->zState ){
    rbuMPrintfExec(p, p->dbRbu, "ATTACH %Q AS stat", p->zState);
    memcpy(p->zStateDb, "stat", 4);
  }else{
    memcpy(p->zStateDb, "main", 4);
  }

#if 0
  if( p->rc==SQLITE_OK && rbuIsVacuum(p) ){
    p->rc = sqlite3_exec(p->dbRbu, "BEGIN", 0, 0, 0);
  }
#endif

  /* If it has not already been created, create the rbu_state table */
  rbuMPrintfExec(p, p->dbRbu, RBU_CREATE_STATE, p->zStateDb);

#if 0
  if( rbuIsVacuum(p) ){
    if( p->rc==SQLITE_OK ){
      int rc2;
      int bOk = 0;
      sqlite3_stmt *pCnt = 0;
      p->rc = prepareAndCollectError(p->dbRbu, &pCnt, &p->zErrmsg,
          "SELECT count(*) FROM stat.sqlite_schema"
      );
      if( p->rc==SQLITE_OK
       && sqlite3_step(pCnt)==SQLITE_ROW
       && 1==sqlite3_column_int(pCnt, 0)
      ){
        bOk = 1;
      }
      rc2 = sqlite3_finalize(pCnt);
      if( p->rc==SQLITE_OK ) p->rc = rc2;

      if( p->rc==SQLITE_OK && bOk==0 ){
        p->rc = SQLITE_ERROR;
        p->zErrmsg = sqlite3_mprintf("invalid state database");
      }

      if( p->rc==SQLITE_OK ){
        p->rc = sqlite3_exec(p->dbRbu, "COMMIT", 0, 0, 0);
      }
    }
  }
#endif

  if( p->rc==SQLITE_OK && rbuIsVacuum(p) ){
    int bOpen = 0;
    int rc;
    p->nRbu = 0;
    p->pRbuFd = 0;
    rc = sqlite3_file_control(p->dbRbu, "main", SQLITE_FCNTL_RBUCNT, (void*)p);
    if( rc!=SQLITE_NOTFOUND ) p->rc = rc;
    if( p->eStage>=RBU_STAGE_MOVE ){
      bOpen = 1;
    }else{
      RbuState *pState = rbuLoadState(p);
      if( pState ){
        bOpen = (pState->eStage>=RBU_STAGE_MOVE);
        rbuFreeState(pState);
      }
    }
    if( bOpen ) p->dbMain = rbuOpenDbhandle(p, p->zRbu, p->nRbu<=1);
  }

  p->eStage = 0;
  if( p->rc==SQLITE_OK && p->dbMain==0 ){
    if( !rbuIsVacuum(p) ){
      p->dbMain = rbuOpenDbhandle(p, p->zTarget, 1);
    }else if( p->pRbuFd->pWalFd ){
      if( pbRetry ){
        p->pRbuFd->bNolock = 0;
        sqlite3_close(p->dbRbu);
        sqlite3_close(p->dbMain);
        p->dbMain = 0;
        p->dbRbu = 0;
        *pbRetry = 1;
        return;
      }
      p->rc = SQLITE_ERROR;
      p->zErrmsg = sqlite3_mprintf("cannot vacuum wal mode database");
    }else{
      char *zTarget;
      char *zExtra = 0;
      if( strlen(p->zRbu)>=5 && 0==memcmp("file:", p->zRbu, 5) ){
        zExtra = &p->zRbu[5];
        while( *zExtra ){
          if( *zExtra++=='?' ) break;
        }
        if( *zExtra=='\0' ) zExtra = 0;
      }

      zTarget = sqlite3_mprintf("file:%s-vactmp?rbu_memory=1%s%s",
          sqlite3_db_filename(p->dbRbu, "main"),
          (zExtra==0 ? "" : "&"), (zExtra==0 ? "" : zExtra)
      );

      if( zTarget==0 ){
        p->rc = SQLITE_NOMEM;
        return;
      }
      p->dbMain = rbuOpenDbhandle(p, zTarget, p->nRbu<=1);
      sqlite3_free(zTarget);
    }
  }

  if( p->rc==SQLITE_OK ){
    p->rc = sqlite3_create_function(p->dbMain,
        "rbu_tmp_insert", -1, SQLITE_UTF8, (void*)p, rbuTmpInsertFunc, 0, 0
    );
  }

  if( p->rc==SQLITE_OK ){
    p->rc = sqlite3_create_function(p->dbMain,
        "rbu_fossil_delta", 2, SQLITE_UTF8, 0, rbuFossilDeltaFunc, 0, 0
    );
  }

  if( p->rc==SQLITE_OK ){
    p->rc = sqlite3_create_function(p->dbRbu,
        "rbu_target_name", -1, SQLITE_UTF8, (void*)p, rbuTargetNameFunc, 0, 0
    );
  }

  if( p->rc==SQLITE_OK ){
    p->rc = sqlite3_file_control(p->dbMain, "main", SQLITE_FCNTL_RBU, (void*)p);
  }
  rbuMPrintfExec(p, p->dbMain, "SELECT * FROM sqlite_schema");

  /* Mark the database file just opened as an RBU target database. If
  ** this call returns SQLITE_NOTFOUND, then the RBU vfs is not in use.
  ** This is an error.  */
  if( p->rc==SQLITE_OK ){
    p->rc = sqlite3_file_control(p->dbMain, "main", SQLITE_FCNTL_RBU, (void*)p);
  }

  if( p->rc==SQLITE_NOTFOUND ){
    p->rc = SQLITE_ERROR;
    p->zErrmsg = sqlite3_mprintf("rbu vfs not found");
  }
}

/*
** This routine is a copy of the sqlite3FileSuffix3() routine from the core.
** It is a no-op unless SQLITE_ENABLE_8_3_NAMES is defined.
**
** If SQLITE_ENABLE_8_3_NAMES is set at compile-time and if the database
** filename in zBaseFilename is a URI with the "8_3_names=1" parameter and
** if filename in z[] has a suffix (a.k.a. "extension") that is longer than
** three characters, then shorten the suffix on z[] to be the last three
** characters of the original suffix.
**
** If SQLITE_ENABLE_8_3_NAMES is set to 2 at compile-time, then always
** do the suffix shortening regardless of URI parameter.
**
** Examples:
**
**     test.db-journal    =>   test.nal
**     test.db-wal        =>   test.wal
**     test.db-shm        =>   test.shm
**     test.db-mj7f3319fa =>   test.9fa
*/
static void rbuFileSuffix3(const char *zBase, char *z){
#ifdef SQLITE_ENABLE_8_3_NAMES
#if SQLITE_ENABLE_8_3_NAMES<2
  if( sqlite3_uri_boolean(zBase, "8_3_names", 0) )
#endif
  {
    int i, sz;
    sz = (int)strlen(z)&0xffffff;
    for(i=sz-1; i>0 && z[i]!='/' && z[i]!='.'; i--){}
    if( z[i]=='.' && sz>i+4 ) memmove(&z[i+1], &z[sz-3], 4);
  }
#else
  UNUSED_PARAMETER2(zBase,z);
#endif
}

/*
** Return the current wal-index header checksum for the target database
** as a 64-bit integer.
**
** The checksum is store in the first page of xShmMap memory as an 8-byte
** blob starting at byte offset 40.
*/
static i64 rbuShmChecksum(sqlite3rbu *p){
  i64 iRet = 0;
  if( p->rc==SQLITE_OK ){
    sqlite3_file *pDb = p->pTargetFd->pReal;
    u32 volatile *ptr;
    p->rc = pDb->pMethods->xShmMap(pDb, 0, 32*1024, 0, (void volatile**)&ptr);
    if( p->rc==SQLITE_OK ){
      iRet = (i64)(((u64)ptr[10] << 32) + ptr[11]);
    }
  }
  return iRet;
}

/*
** This function is called as part of initializing or reinitializing an
** incremental checkpoint.
**
** It populates the sqlite3rbu.aFrame[] array with the set of
** (wal frame -> db page) copy operations required to checkpoint the
** current wal file, and obtains the set of shm locks required to safely
** perform the copy operations directly on the file-system.
**
** If argument pState is not NULL, then the incremental checkpoint is
** being resumed. In this case, if the checksum of the wal-index-header
** following recovery is not the same as the checksum saved in the RbuState
** object, then the rbu handle is set to DONE state. This occurs if some
** other client appends a transaction to the wal file in the middle of
** an incremental checkpoint.
*/
static void rbuSetupCheckpoint(sqlite3rbu *p, RbuState *pState){

  /* If pState is NULL, then the wal file may not have been opened and
  ** recovered. Running a read-statement here to ensure that doing so
  ** does not interfere with the "capture" process below.  */
  if( pState==0 ){
    p->eStage = 0;
    if( p->rc==SQLITE_OK ){
      p->rc = sqlite3_exec(p->dbMain, "SELECT * FROM sqlite_schema", 0, 0, 0);
    }
  }

  /* Assuming no error has occurred, run a "restart" checkpoint with the
  ** sqlite3rbu.eStage variable set to CAPTURE. This turns on the following
  ** special behaviour in the rbu VFS:
  **
  **   * If the exclusive shm WRITER or READ0 lock cannot be obtained,
  **     the checkpoint fails with SQLITE_BUSY (normally SQLite would
  **     proceed with running a passive checkpoint instead of failing).
  **
  **   * Attempts to read from the *-wal file or write to the database file
  **     do not perform any IO. Instead, the frame/page combinations that
  **     would be read/written are recorded in the sqlite3rbu.aFrame[]
  **     array.
  **
  **   * Calls to xShmLock(UNLOCK) to release the exclusive shm WRITER,
  **     READ0 and CHECKPOINT locks taken as part of the checkpoint are
  **     no-ops. These locks will not be released until the connection
  **     is closed.
  **
  **   * Attempting to xSync() the database file causes an SQLITE_NOTICE
  **     error.
  **
  ** As a result, unless an error (i.e. OOM or SQLITE_BUSY) occurs, the
  ** checkpoint below fails with SQLITE_NOTICE, and leaves the aFrame[]
  ** array populated with a set of (frame -> page) mappings. Because the
  ** WRITER, CHECKPOINT and READ0 locks are still held, it is safe to copy
  ** data from the wal file into the database file according to the
  ** contents of aFrame[].
  */
  if( p->rc==SQLITE_OK ){
    int rc2;
    p->eStage = RBU_STAGE_CAPTURE;
    rc2 = sqlite3_exec(p->dbMain, "PRAGMA main.wal_checkpoint=restart", 0, 0,0);
    if( rc2!=SQLITE_NOTICE ) p->rc = rc2;
  }

  if( p->rc==SQLITE_OK && p->nFrame>0 ){
    p->eStage = RBU_STAGE_CKPT;
    p->nStep = (pState ? pState->nRow : 0);
    p->aBuf = rbuMalloc(p, p->pgsz);
    p->iWalCksum = rbuShmChecksum(p);
  }

  if( p->rc==SQLITE_OK ){
    if( p->nFrame==0 || (pState && pState->iWalCksum!=p->iWalCksum) ){
      p->rc = SQLITE_DONE;
      p->eStage = RBU_STAGE_DONE;
    }else{
      int nSectorSize;
      sqlite3_file *pDb = p->pTargetFd->pReal;
      sqlite3_file *pWal = p->pTargetFd->pWalFd->pReal;
      assert( p->nPagePerSector==0 );
      nSectorSize = pDb->pMethods->xSectorSize(pDb);
      if( nSectorSize>p->pgsz ){
        p->nPagePerSector = nSectorSize / p->pgsz;
      }else{
        p->nPagePerSector = 1;
      }

      /* Call xSync() on the wal file. This causes SQLite to sync the
      ** directory in which the target database and the wal file reside, in
      ** case it has not been synced since the rename() call in
      ** rbuMoveOalFile(). */
      p->rc = pWal->pMethods->xSync(pWal, SQLITE_SYNC_NORMAL);
    }
  }
}

/*
** Called when iAmt bytes are read from offset iOff of the wal file while
** the rbu object is in capture mode. Record the frame number of the frame
** being read in the aFrame[] array.
*/
static int rbuCaptureWalRead(sqlite3rbu *pRbu, i64 iOff, int iAmt){
  const u32 mReq = (1<<WAL_LOCK_WRITE)|(1<<WAL_LOCK_CKPT)|(1<<WAL_LOCK_READ0);
  u32 iFrame;

  if( pRbu->mLock!=mReq ){
    pRbu->rc = SQLITE_BUSY;
    return SQLITE_NOTICE_RBU;
  }

  pRbu->pgsz = iAmt;
  if( pRbu->nFrame==pRbu->nFrameAlloc ){
    int nNew = (pRbu->nFrameAlloc ? pRbu->nFrameAlloc : 64) * 2;
    RbuFrame *aNew;
    aNew = (RbuFrame*)sqlite3_realloc64(pRbu->aFrame, nNew * sizeof(RbuFrame));
    if( aNew==0 ) return SQLITE_NOMEM;
    pRbu->aFrame = aNew;
    pRbu->nFrameAlloc = nNew;
  }

  iFrame = (u32)((iOff-32) / (i64)(iAmt+24)) + 1;
  if( pRbu->iMaxFrame<iFrame ) pRbu->iMaxFrame = iFrame;
  pRbu->aFrame[pRbu->nFrame].iWalFrame = iFrame;
  pRbu->aFrame[pRbu->nFrame].iDbPage = 0;
  pRbu->nFrame++;
  return SQLITE_OK;
}

/*
** Called when a page of data is written to offset iOff of the database
** file while the rbu handle is in capture mode. Record the page number
** of the page being written in the aFrame[] array.
*/
static int rbuCaptureDbWrite(sqlite3rbu *pRbu, i64 iOff){
  pRbu->aFrame[pRbu->nFrame-1].iDbPage = (u32)(iOff / pRbu->pgsz) + 1;
  return SQLITE_OK;
}

/*
** This is called as part of an incremental checkpoint operation. Copy
** a single frame of data from the wal file into the database file, as
** indicated by the RbuFrame object.
*/
static void rbuCheckpointFrame(sqlite3rbu *p, RbuFrame *pFrame){
  sqlite3_file *pWal = p->pTargetFd->pWalFd->pReal;
  sqlite3_file *pDb = p->pTargetFd->pReal;
  i64 iOff;

  assert( p->rc==SQLITE_OK );
  iOff = (i64)(pFrame->iWalFrame-1) * (p->pgsz + 24) + 32 + 24;
  p->rc = pWal->pMethods->xRead(pWal, p->aBuf, p->pgsz, iOff);
  if( p->rc ) return;

  iOff = (i64)(pFrame->iDbPage-1) * p->pgsz;
  p->rc = pDb->pMethods->xWrite(pDb, p->aBuf, p->pgsz, iOff);
}

/*
** This value is copied from the definition of ZIPVFS_CTRL_FILE_POINTER
** in zipvfs.h.
*/
#define RBU_ZIPVFS_CTRL_FILE_POINTER 230439

/*
** Take an EXCLUSIVE lock on the database file. Return SQLITE_OK if
** successful, or an SQLite error code otherwise.
*/
static int rbuLockDatabase(sqlite3 *db){
  int rc = SQLITE_OK;
  sqlite3_file *fd = 0;

  sqlite3_file_control(db, "main", RBU_ZIPVFS_CTRL_FILE_POINTER, &fd);
  if( fd ){
    sqlite3_file_control(db, "main", SQLITE_FCNTL_FILE_POINTER, &fd);
    rc = fd->pMethods->xLock(fd, SQLITE_LOCK_SHARED);
    if( rc==SQLITE_OK ){
      rc = fd->pMethods->xUnlock(fd, SQLITE_LOCK_NONE);
    }
    sqlite3_file_control(db, "main", RBU_ZIPVFS_CTRL_FILE_POINTER, &fd);
  }else{
    sqlite3_file_control(db, "main", SQLITE_FCNTL_FILE_POINTER, &fd);
  }

  if( rc==SQLITE_OK && fd->pMethods ){
    rc = fd->pMethods->xLock(fd, SQLITE_LOCK_SHARED);
    if( rc==SQLITE_OK ){
      rc = fd->pMethods->xLock(fd, SQLITE_LOCK_EXCLUSIVE);
    }
  }
  return rc;
}

/*
** Return true if the database handle passed as the only argument
** was opened with the rbu_exclusive_checkpoint=1 URI parameter
** specified. Or false otherwise.
*/
static int rbuExclusiveCheckpoint(sqlite3 *db){
  const char *zUri = sqlite3_db_filename(db, 0);
  return sqlite3_uri_boolean(zUri, RBU_EXCLUSIVE_CHECKPOINT, 0);
}

#if defined(_WIN32_WCE)
static LPWSTR rbuWinUtf8ToUnicode(const char *zFilename){
  int nChar;
  LPWSTR zWideFilename;

  nChar = MultiByteToWideChar(CP_UTF8, 0, zFilename, -1, NULL, 0);
  if( nChar==0 ){
    return 0;
  }
  zWideFilename = sqlite3_malloc64( nChar*sizeof(zWideFilename[0]) );
  if( zWideFilename==0 ){
    return 0;
  }
  memset(zWideFilename, 0, nChar*sizeof(zWideFilename[0]));
  nChar = MultiByteToWideChar(CP_UTF8, 0, zFilename, -1, zWideFilename,
                                nChar);
  if( nChar==0 ){
    sqlite3_free(zWideFilename);
    zWideFilename = 0;
  }
  return zWideFilename;
}
#endif

/*
** The RBU handle is currently in RBU_STAGE_OAL state, with a SHARED lock
** on the database file. This proc moves the *-oal file to the *-wal path,
** then reopens the database file (this time in vanilla, non-oal, WAL mode).
** If an error occurs, leave an error code and error message in the rbu
** handle.
*/
static void rbuMoveOalFile(sqlite3rbu *p){
  const char *zBase = sqlite3_db_filename(p->dbMain, "main");
  const char *zMove = zBase;
  char *zOal;
  char *zWal;

  if( rbuIsVacuum(p) ){
    zMove = sqlite3_db_filename(p->dbRbu, "main");
  }
  zOal = sqlite3_mprintf("%s-oal", zMove);
  zWal = sqlite3_mprintf("%s-wal", zMove);

  assert( p->eStage==RBU_STAGE_MOVE );
  assert( p->rc==SQLITE_OK && p->zErrmsg==0 );
  if( zWal==0 || zOal==0 ){
    p->rc = SQLITE_NOMEM;
  }else{
    /* Move the *-oal file to *-wal. At this point connection p->db is
    ** holding a SHARED lock on the target database file (because it is
    ** in WAL mode). So no other connection may be writing the db.
    **
    ** In order to ensure that there are no database readers, an EXCLUSIVE
    ** lock is obtained here before the *-oal is moved to *-wal.
    */
    sqlite3 *dbMain = 0;
    rbuFileSuffix3(zBase, zWal);
    rbuFileSuffix3(zBase, zOal);

    /* Re-open the databases. */
    rbuObjIterFinalize(&p->objiter);
    sqlite3_close(p->dbRbu);
    sqlite3_close(p->dbMain);
    p->dbMain = 0;
    p->dbRbu = 0;

    dbMain = rbuOpenDbhandle(p, p->zTarget, 1);
    if( dbMain ){
      assert( p->rc==SQLITE_OK );
      p->rc = rbuLockDatabase(dbMain);
    }

    if( p->rc==SQLITE_OK ){
      p->rc = p->xRename(p->pRenameArg, zOal, zWal);
    }

    if( p->rc!=SQLITE_OK
     || rbuIsVacuum(p)
     || rbuExclusiveCheckpoint(dbMain)==0
    ){
      sqlite3_close(dbMain);
      dbMain = 0;
    }

    if( p->rc==SQLITE_OK ){
      rbuOpenDatabase(p, dbMain, 0);
      rbuSetupCheckpoint(p, 0);
    }
  }

  sqlite3_free(zWal);
  sqlite3_free(zOal);
}

/*
** The SELECT statement iterating through the keys for the current object
** (p->objiter.pSelect) currently points to a valid row. This function
** determines the type of operation requested by this row and returns
** one of the following values to indicate the result:
**
**     * RBU_INSERT
**     * RBU_DELETE
**     * RBU_IDX_DELETE
**     * RBU_UPDATE
**
** If RBU_UPDATE is returned, then output variable *pzMask is set to
** point to the text value indicating the columns to update.
**
** If the rbu_control field contains an invalid value, an error code and
** message are left in the RBU handle and zero returned.
*/
static int rbuStepType(sqlite3rbu *p, const char **pzMask){
  int iCol = p->objiter.nCol;     /* Index of rbu_control column */
  int res = 0;                    /* Return value */

  switch( sqlite3_column_type(p->objiter.pSelect, iCol) ){
    case SQLITE_INTEGER: {
      int iVal = sqlite3_column_int(p->objiter.pSelect, iCol);
      switch( iVal ){
        case 0: res = RBU_INSERT;     break;
        case 1: res = RBU_DELETE;     break;
        case 2: res = RBU_REPLACE;    break;
        case 3: res = RBU_IDX_DELETE; break;
        case 4: res = RBU_IDX_INSERT; break;
      }
      break;
    }

    case SQLITE_TEXT: {
      const unsigned char *z = sqlite3_column_text(p->objiter.pSelect, iCol);
      if( z==0 ){
        p->rc = SQLITE_NOMEM;
      }else{
        *pzMask = (const char*)z;
      }
      res = RBU_UPDATE;

      break;
    }

    default:
      break;
  }

  if( res==0 ){
    rbuBadControlError(p);
  }
  return res;
}

#ifdef SQLITE_DEBUG
/*
** Assert that column iCol of statement pStmt is named zName.
*/
static void assertColumnName(sqlite3_stmt *pStmt, int iCol, const char *zName){
  const char *zCol = sqlite3_column_name(pStmt, iCol);
  assert( 0==sqlite3_stricmp(zName, zCol) );
}
#else
# define assertColumnName(x,y,z)
#endif

/*
** Argument eType must be one of RBU_INSERT, RBU_DELETE, RBU_IDX_INSERT or
** RBU_IDX_DELETE. This function performs the work of a single
** sqlite3rbu_step() call for the type of operation specified by eType.
*/
static void rbuStepOneOp(sqlite3rbu *p, int eType){
  RbuObjIter *pIter = &p->objiter;
  sqlite3_value *pVal;
  sqlite3_stmt *pWriter;
  int i;

  assert( p->rc==SQLITE_OK );
  assert( eType!=RBU_DELETE || pIter->zIdx==0 );
  assert( eType==RBU_DELETE || eType==RBU_IDX_DELETE
       || eType==RBU_INSERT || eType==RBU_IDX_INSERT
  );

  /* If this is a delete, decrement nPhaseOneStep by nIndex. If the DELETE
  ** statement below does actually delete a row, nPhaseOneStep will be
  ** incremented by the same amount when SQL function rbu_tmp_insert()
  ** is invoked by the trigger.  */
  if( eType==RBU_DELETE ){
    p->nPhaseOneStep -= p->objiter.nIndex;
  }

  if( eType==RBU_IDX_DELETE || eType==RBU_DELETE ){
    pWriter = pIter->pDelete;
  }else{
    pWriter = pIter->pInsert;
  }

  for(i=0; i<pIter->nCol; i++){
    /* If this is an INSERT into a table b-tree and the table has an
    ** explicit INTEGER PRIMARY KEY, check that this is not an attempt
    ** to write a NULL into the IPK column. That is not permitted.  */
    if( eType==RBU_INSERT
     && pIter->zIdx==0 && pIter->eType==RBU_PK_IPK && pIter->abTblPk[i]
     && sqlite3_column_type(pIter->pSelect, i)==SQLITE_NULL
    ){
      p->rc = SQLITE_MISMATCH;
      p->zErrmsg = sqlite3_mprintf("datatype mismatch");
      return;
    }

    if( eType==RBU_DELETE && pIter->abTblPk[i]==0 ){
      continue;
    }

    pVal = sqlite3_column_value(pIter->pSelect, i);
    p->rc = sqlite3_bind_value(pWriter, i+1, pVal);
    if( p->rc ) return;
  }
  if( pIter->zIdx==0 ){
    if( pIter->eType==RBU_PK_VTAB
     || pIter->eType==RBU_PK_NONE
     || (pIter->eType==RBU_PK_EXTERNAL && rbuIsVacuum(p))
    ){
      /* For a virtual table, or a table with no primary key, the
      ** SELECT statement is:
      **
      **   SELECT <cols>, rbu_control, rbu_rowid FROM ....
      **
      ** Hence column_value(pIter->nCol+1).
      */
      assertColumnName(pIter->pSelect, pIter->nCol+1,
          rbuIsVacuum(p) ? "rowid" : "rbu_rowid"
      );
      pVal = sqlite3_column_value(pIter->pSelect, pIter->nCol+1);
      p->rc = sqlite3_bind_value(pWriter, pIter->nCol+1, pVal);
    }
  }
  if( p->rc==SQLITE_OK ){
    sqlite3_step(pWriter);
    p->rc = resetAndCollectError(pWriter, &p->zErrmsg);
  }
}

/*
** This function does the work for an sqlite3rbu_step() call.
**
** The object-iterator (p->objiter) currently points to a valid object,
** and the input cursor (p->objiter.pSelect) currently points to a valid
** input row. Perform whatever processing is required and return.
**
** If no  error occurs, SQLITE_OK is returned. Otherwise, an error code
** and message is left in the RBU handle and a copy of the error code
** returned.
*/
static int rbuStep(sqlite3rbu *p){
  RbuObjIter *pIter = &p->objiter;
  const char *zMask = 0;
  int eType = rbuStepType(p, &zMask);

  if( eType ){
    assert( eType==RBU_INSERT     || eType==RBU_DELETE
         || eType==RBU_REPLACE    || eType==RBU_IDX_DELETE
         || eType==RBU_IDX_INSERT || eType==RBU_UPDATE
    );
    assert( eType!=RBU_UPDATE || pIter->zIdx==0 );

    if( pIter->zIdx==0 && (eType==RBU_IDX_DELETE || eType==RBU_IDX_INSERT) ){
      rbuBadControlError(p);
    }
    else if( eType==RBU_REPLACE ){
      if( pIter->zIdx==0 ){
        p->nPhaseOneStep += p->objiter.nIndex;
        rbuStepOneOp(p, RBU_DELETE);
      }
      if( p->rc==SQLITE_OK ) rbuStepOneOp(p, RBU_INSERT);
    }
    else if( eType!=RBU_UPDATE ){
      rbuStepOneOp(p, eType);
    }
    else{
      sqlite3_value *pVal;
      sqlite3_stmt *pUpdate = 0;
      assert( eType==RBU_UPDATE );
      p->nPhaseOneStep -= p->objiter.nIndex;
      rbuGetUpdateStmt(p, pIter, zMask, &pUpdate);
      if( pUpdate ){
        int i;
        for(i=0; p->rc==SQLITE_OK && i<pIter->nCol; i++){
          char c = zMask[pIter->aiSrcOrder[i]];
          pVal = sqlite3_column_value(pIter->pSelect, i);
          if( pIter->abTblPk[i] || c!='.' ){
            p->rc = sqlite3_bind_value(pUpdate, i+1, pVal);
          }
        }
        if( p->rc==SQLITE_OK
         && (pIter->eType==RBU_PK_VTAB || pIter->eType==RBU_PK_NONE)
        ){
          /* Bind the rbu_rowid value to column _rowid_ */
          assertColumnName(pIter->pSelect, pIter->nCol+1, "rbu_rowid");
          pVal = sqlite3_column_value(pIter->pSelect, pIter->nCol+1);
          p->rc = sqlite3_bind_value(pUpdate, pIter->nCol+1, pVal);
        }
        if( p->rc==SQLITE_OK ){
          sqlite3_step(pUpdate);
          p->rc = resetAndCollectError(pUpdate, &p->zErrmsg);
        }
      }
    }
  }
  return p->rc;
}

/*
** Increment the schema cookie of the main database opened by p->dbMain.
**
** Or, if this is an RBU vacuum, set the schema cookie of the main db
** opened by p->dbMain to one more than the schema cookie of the main
** db opened by p->dbRbu.
*/
static void rbuIncrSchemaCookie(sqlite3rbu *p){
  if( p->rc==SQLITE_OK ){
    sqlite3 *dbread = (rbuIsVacuum(p) ? p->dbRbu : p->dbMain);
    int iCookie = 1000000;
    sqlite3_stmt *pStmt;

    p->rc = prepareAndCollectError(dbread, &pStmt, &p->zErrmsg,
        "PRAGMA schema_version"
    );
    if( p->rc==SQLITE_OK ){
      /* Coverage: it may be that this sqlite3_step() cannot fail. There
      ** is already a transaction open, so the prepared statement cannot
      ** throw an SQLITE_SCHEMA exception. The only database page the
      ** statement reads is page 1, which is guaranteed to be in the cache.
      ** And no memory allocations are required.  */
      if( SQLITE_ROW==sqlite3_step(pStmt) ){
        iCookie = sqlite3_column_int(pStmt, 0);
      }
      rbuFinalize(p, pStmt);
    }
    if( p->rc==SQLITE_OK ){
      rbuMPrintfExec(p, p->dbMain, "PRAGMA schema_version = %d", iCookie+1);
    }
  }
}

/*
** Update the contents of the rbu_state table within the rbu database. The
** value stored in the RBU_STATE_STAGE column is eStage. All other values
** are determined by inspecting the rbu handle passed as the first argument.
*/
static void rbuSaveState(sqlite3rbu *p, int eStage){
  if( p->rc==SQLITE_OK || p->rc==SQLITE_DONE ){
    sqlite3_stmt *pInsert = 0;
    rbu_file *pFd = (rbuIsVacuum(p) ? p->pRbuFd : p->pTargetFd);
    int rc;

    assert( p->zErrmsg==0 );
    rc = prepareFreeAndCollectError(p->dbRbu, &pInsert, &p->zErrmsg,
        sqlite3_mprintf(
          "INSERT OR REPLACE INTO %s.rbu_state(k, v) VALUES "
          "(%d, %d), "
          "(%d, %Q), "
          "(%d, %Q), "
          "(%d, %d), "
          "(%d, %lld), "
          "(%d, %lld), "
          "(%d, %lld), "
          "(%d, %lld), "
          "(%d, %lld), "
          "(%d, %Q)  ",
          p->zStateDb,
          RBU_STATE_STAGE, eStage,
          RBU_STATE_TBL, p->objiter.zTbl,
          RBU_STATE_IDX, p->objiter.zIdx,
          RBU_STATE_ROW, p->nStep,
          RBU_STATE_PROGRESS, p->nProgress,
          RBU_STATE_CKPT, p->iWalCksum,
          RBU_STATE_COOKIE, (i64)pFd->iCookie,
          RBU_STATE_OALSZ, p->iOalSz,
          RBU_STATE_PHASEONESTEP, p->nPhaseOneStep,
          RBU_STATE_DATATBL, p->objiter.zDataTbl
      )
    );
    assert( pInsert==0 || rc==SQLITE_OK );

    if( rc==SQLITE_OK ){
      sqlite3_step(pInsert);
      rc = sqlite3_finalize(pInsert);
    }
    if( rc!=SQLITE_OK ) p->rc = rc;
  }
}


/*
** The second argument passed to this function is the name of a PRAGMA
** setting - "page_size", "auto_vacuum", "user_version" or "application_id".
** This function executes the following on sqlite3rbu.dbRbu:
**
**   "PRAGMA main.$zPragma"
**
** where $zPragma is the string passed as the second argument, then
** on sqlite3rbu.dbMain:
**
**   "PRAGMA main.$zPragma = $val"
**
** where $val is the value returned by the first PRAGMA invocation.
**
** In short, it copies the value  of the specified PRAGMA setting from
** dbRbu to dbMain.
*/
static void rbuCopyPragma(sqlite3rbu *p, const char *zPragma){
  if( p->rc==SQLITE_OK ){
    sqlite3_stmt *pPragma = 0;
    p->rc = prepareFreeAndCollectError(p->dbRbu, &pPragma, &p->zErrmsg,
        sqlite3_mprintf("PRAGMA main.%s", zPragma)
    );
    if( p->rc==SQLITE_OK && SQLITE_ROW==sqlite3_step(pPragma) ){
      p->rc = rbuMPrintfExec(p, p->dbMain, "PRAGMA main.%s = %d",
          zPragma, sqlite3_column_int(pPragma, 0)
      );
    }
    rbuFinalize(p, pPragma);
  }
}

/*
** The RBU handle passed as the only argument has just been opened and
** the state database is empty. If this RBU handle was opened for an
** RBU vacuum operation, create the schema in the target db.
*/
static void rbuCreateTargetSchema(sqlite3rbu *p){
  sqlite3_stmt *pSql = 0;
  sqlite3_stmt *pInsert = 0;

  assert( rbuIsVacuum(p) );
  p->rc = sqlite3_exec(p->dbMain, "PRAGMA writable_schema=1", 0,0, &p->zErrmsg);
  if( p->rc==SQLITE_OK ){
    p->rc = prepareAndCollectError(p->dbRbu, &pSql, &p->zErrmsg,
      "SELECT sql FROM sqlite_schema WHERE sql!='' AND rootpage!=0"
      " AND name!='sqlite_sequence' "
      " ORDER BY type DESC"
    );
  }

  while( p->rc==SQLITE_OK && sqlite3_step(pSql)==SQLITE_ROW ){
    const char *zSql = (const char*)sqlite3_column_text(pSql, 0);
    p->rc = sqlite3_exec(p->dbMain, zSql, 0, 0, &p->zErrmsg);
  }
  rbuFinalize(p, pSql);
  if( p->rc!=SQLITE_OK ) return;

  if( p->rc==SQLITE_OK ){
    p->rc = prepareAndCollectError(p->dbRbu, &pSql, &p->zErrmsg,
        "SELECT * FROM sqlite_schema WHERE rootpage=0 OR rootpage IS NULL"
    );
  }

  if( p->rc==SQLITE_OK ){
    p->rc = prepareAndCollectError(p->dbMain, &pInsert, &p->zErrmsg,
        "INSERT INTO sqlite_schema VALUES(?,?,?,?,?)"
    );
  }

  while( p->rc==SQLITE_OK && sqlite3_step(pSql)==SQLITE_ROW ){
    int i;
    for(i=0; i<5; i++){
      sqlite3_bind_value(pInsert, i+1, sqlite3_column_value(pSql, i));
    }
    sqlite3_step(pInsert);
    p->rc = sqlite3_reset(pInsert);
  }
  if( p->rc==SQLITE_OK ){
    p->rc = sqlite3_exec(p->dbMain, "PRAGMA writable_schema=0",0,0,&p->zErrmsg);
  }

  rbuFinalize(p, pSql);
  rbuFinalize(p, pInsert);
}

/*
** Step the RBU object.
*/
SQLITE_API int sqlite3rbu_step(sqlite3rbu *p){
  if( p ){
    switch( p->eStage ){
      case RBU_STAGE_OAL: {
        RbuObjIter *pIter = &p->objiter;

        /* If this is an RBU vacuum operation and the state table was empty
        ** when this handle was opened, create the target database schema. */
        if( rbuIsVacuum(p) && p->nProgress==0 && p->rc==SQLITE_OK ){
          rbuCreateTargetSchema(p);
          rbuCopyPragma(p, "user_version");
          rbuCopyPragma(p, "application_id");
        }

        while( p->rc==SQLITE_OK && pIter->zTbl ){

          if( pIter->bCleanup ){
            /* Clean up the rbu_tmp_xxx table for the previous table. It
            ** cannot be dropped as there are currently active SQL statements.
            ** But the contents can be deleted.  */
            if( rbuIsVacuum(p)==0 && pIter->abIndexed ){
              rbuMPrintfExec(p, p->dbRbu,
                  "DELETE FROM %s.'rbu_tmp_%q'", p->zStateDb, pIter->zDataTbl
              );
            }
          }else{
            rbuObjIterPrepareAll(p, pIter, 0);

            /* Advance to the next row to process. */
            if( p->rc==SQLITE_OK ){
              int rc = sqlite3_step(pIter->pSelect);
              if( rc==SQLITE_ROW ){
                p->nProgress++;
                p->nStep++;
                return rbuStep(p);
              }
              p->rc = sqlite3_reset(pIter->pSelect);
              p->nStep = 0;
            }
          }

          rbuObjIterNext(p, pIter);
        }

        if( p->rc==SQLITE_OK ){
          assert( pIter->zTbl==0 );
          rbuSaveState(p, RBU_STAGE_MOVE);
          rbuIncrSchemaCookie(p);
          if( p->rc==SQLITE_OK ){
            p->rc = sqlite3_exec(p->dbMain, "COMMIT", 0, 0, &p->zErrmsg);
          }
          if( p->rc==SQLITE_OK ){
            p->rc = sqlite3_exec(p->dbRbu, "COMMIT", 0, 0, &p->zErrmsg);
          }
          p->eStage = RBU_STAGE_MOVE;
        }
        break;
      }

      case RBU_STAGE_MOVE: {
        if( p->rc==SQLITE_OK ){
          rbuMoveOalFile(p);
          p->nProgress++;
        }
        break;
      }

      case RBU_STAGE_CKPT: {
        if( p->rc==SQLITE_OK ){
          if( p->nStep>=p->nFrame ){
            sqlite3_file *pDb = p->pTargetFd->pReal;

            /* Sync the db file */
            p->rc = pDb->pMethods->xSync(pDb, SQLITE_SYNC_NORMAL);

            /* Update nBackfill */
            if( p->rc==SQLITE_OK ){
              void volatile *ptr;
              p->rc = pDb->pMethods->xShmMap(pDb, 0, 32*1024, 0, &ptr);
              if( p->rc==SQLITE_OK ){
                ((u32 volatile*)ptr)[24] = p->iMaxFrame;
              }
            }

            if( p->rc==SQLITE_OK ){
              p->eStage = RBU_STAGE_DONE;
              p->rc = SQLITE_DONE;
            }
          }else{
            /* At one point the following block copied a single frame from the
            ** wal file to the database file. So that one call to sqlite3rbu_step()
            ** checkpointed a single frame.
            **
            ** However, if the sector-size is larger than the page-size, and the
            ** application calls sqlite3rbu_savestate() or close() immediately
            ** after this step, then rbu_step() again, then a power failure occurs,
            ** then the database page written here may be damaged. Work around
            ** this by checkpointing frames until the next page in the aFrame[]
            ** lies on a different disk sector to the current one. */
            u32 iSector;
            do{
              RbuFrame *pFrame = &p->aFrame[p->nStep];
              iSector = (pFrame->iDbPage-1) / p->nPagePerSector;
              rbuCheckpointFrame(p, pFrame);
              p->nStep++;
            }while( p->nStep<p->nFrame
                 && iSector==((p->aFrame[p->nStep].iDbPage-1) / p->nPagePerSector)
                 && p->rc==SQLITE_OK
            );
          }
          p->nProgress++;
        }
        break;
      }

      default:
        break;
    }
    return p->rc;
  }else{
    return SQLITE_NOMEM;
  }
}

/*
** Compare strings z1 and z2, returning 0 if they are identical, or non-zero
** otherwise. Either or both argument may be NULL. Two NULL values are
** considered equal, and NULL is considered distinct from all other values.
*/
static int rbuStrCompare(const char *z1, const char *z2){
  if( z1==0 && z2==0 ) return 0;
  if( z1==0 || z2==0 ) return 1;
  return (sqlite3_stricmp(z1, z2)!=0);
}

/*
** This function is called as part of sqlite3rbu_open() when initializing
** an rbu handle in OAL stage. If the rbu update has not started (i.e.
** the rbu_state table was empty) it is a no-op. Otherwise, it arranges
** things so that the next call to sqlite3rbu_step() continues on from
** where the previous rbu handle left off.
**
** If an error occurs, an error code and error message are left in the
** rbu handle passed as the first argument.
*/
static void rbuSetupOal(sqlite3rbu *p, RbuState *pState){
  assert( p->rc==SQLITE_OK );
  if( pState->zTbl ){
    RbuObjIter *pIter = &p->objiter;
    int rc = SQLITE_OK;

    while( rc==SQLITE_OK && pIter->zTbl && (pIter->bCleanup
       || rbuStrCompare(pIter->zIdx, pState->zIdx)
       || (pState->zDataTbl==0 && rbuStrCompare(pIter->zTbl, pState->zTbl))
       || (pState->zDataTbl && rbuStrCompare(pIter->zDataTbl, pState->zDataTbl))
    )){
      rc = rbuObjIterNext(p, pIter);
    }

    if( rc==SQLITE_OK && !pIter->zTbl ){
      rc = SQLITE_ERROR;
      p->zErrmsg = sqlite3_mprintf("rbu_state mismatch error");
    }

    if( rc==SQLITE_OK ){
      p->nStep = pState->nRow;
      rc = rbuObjIterPrepareAll(p, &p->objiter, p->nStep);
    }

    p->rc = rc;
  }
}

/*
** If there is a "*-oal" file in the file-system corresponding to the
** target database in the file-system, delete it. If an error occurs,
** leave an error code and error message in the rbu handle.
*/
static void rbuDeleteOalFile(sqlite3rbu *p){
  char *zOal = rbuMPrintf(p, "%s-oal", p->zTarget);
  if( zOal ){
    sqlite3_vfs *pVfs = 0;
    sqlite3_file_control(p->dbMain, "main", SQLITE_FCNTL_VFS_POINTER, &pVfs);
    assert( pVfs && p->rc==SQLITE_OK && p->zErrmsg==0 );
    pVfs->xDelete(pVfs, zOal, 0);
    sqlite3_free(zOal);
  }
}

/*
** Allocate a private rbu VFS for the rbu handle passed as the only
** argument. This VFS will be used unless the call to sqlite3rbu_open()
** specified a URI with a vfs=? option in place of a target database
** file name.
*/
static void rbuCreateVfs(sqlite3rbu *p){
  int rnd;
  char zRnd[64];

  assert( p->rc==SQLITE_OK );
  sqlite3_randomness(sizeof(int), (void*)&rnd);
  sqlite3_snprintf(sizeof(zRnd), zRnd, "rbu_vfs_%d", rnd);
  p->rc = sqlite3rbu_create_vfs(zRnd, 0);
  if( p->rc==SQLITE_OK ){
    sqlite3_vfs *pVfs = sqlite3_vfs_find(zRnd);
    assert( pVfs );
    p->zVfsName = pVfs->zName;
    ((rbu_vfs*)pVfs)->pRbu = p;
  }
}

/*
** Destroy the private VFS created for the rbu handle passed as the only
** argument by an earlier call to rbuCreateVfs().
*/
static void rbuDeleteVfs(sqlite3rbu *p){
  if( p->zVfsName ){
    sqlite3rbu_destroy_vfs(p->zVfsName);
    p->zVfsName = 0;
  }
}

/*
** This user-defined SQL function is invoked with a single argument - the
** name of a table expected to appear in the target database. It returns
** the number of auxilliary indexes on the table.
*/
static void rbuIndexCntFunc(
  sqlite3_context *pCtx,
  int nVal,
  sqlite3_value **apVal
){
  sqlite3rbu *p = (sqlite3rbu*)sqlite3_user_data(pCtx);
  sqlite3_stmt *pStmt = 0;
  char *zErrmsg = 0;
  int rc;
  sqlite3 *db = (rbuIsVacuum(p) ? p->dbRbu : p->dbMain);

  assert( nVal==1 );
  UNUSED_PARAMETER(nVal);

  rc = prepareFreeAndCollectError(db, &pStmt, &zErrmsg,
      sqlite3_mprintf("SELECT count(*) FROM sqlite_schema "
        "WHERE type='index' AND tbl_name = %Q", sqlite3_value_text(apVal[0]))
  );
  if( rc!=SQLITE_OK ){
    sqlite3_result_error(pCtx, zErrmsg, -1);
  }else{
    int nIndex = 0;
    if( SQLITE_ROW==sqlite3_step(pStmt) ){
      nIndex = sqlite3_column_int(pStmt, 0);
    }
    rc = sqlite3_finalize(pStmt);
    if( rc==SQLITE_OK ){
      sqlite3_result_int(pCtx, nIndex);
    }else{
      sqlite3_result_error(pCtx, sqlite3_errmsg(db), -1);
    }
  }

  sqlite3_free(zErrmsg);
}

/*
** If the RBU database contains the rbu_count table, use it to initialize
** the sqlite3rbu.nPhaseOneStep variable. The schema of the rbu_count table
** is assumed to contain the same columns as:
**
**   CREATE TABLE rbu_count(tbl TEXT PRIMARY KEY, cnt INTEGER) WITHOUT ROWID;
**
** There should be one row in the table for each data_xxx table in the
** database. The 'tbl' column should contain the name of a data_xxx table,
** and the cnt column the number of rows it contains.
**
** sqlite3rbu.nPhaseOneStep is initialized to the sum of (1 + nIndex) * cnt
** for all rows in the rbu_count table, where nIndex is the number of
** indexes on the corresponding target database table.
*/
static void rbuInitPhaseOneSteps(sqlite3rbu *p){
  if( p->rc==SQLITE_OK ){
    sqlite3_stmt *pStmt = 0;
    int bExists = 0;                /* True if rbu_count exists */

    p->nPhaseOneStep = -1;

    p->rc = sqlite3_create_function(p->dbRbu,
        "rbu_index_cnt", 1, SQLITE_UTF8, (void*)p, rbuIndexCntFunc, 0, 0
    );

    /* Check for the rbu_count table. If it does not exist, or if an error
    ** occurs, nPhaseOneStep will be left set to -1. */
    if( p->rc==SQLITE_OK ){
      p->rc = prepareAndCollectError(p->dbRbu, &pStmt, &p->zErrmsg,
          "SELECT 1 FROM sqlite_schema WHERE tbl_name = 'rbu_count'"
      );
    }
    if( p->rc==SQLITE_OK ){
      if( SQLITE_ROW==sqlite3_step(pStmt) ){
        bExists = 1;
      }
      p->rc = sqlite3_finalize(pStmt);
    }

    if( p->rc==SQLITE_OK && bExists ){
      p->rc = prepareAndCollectError(p->dbRbu, &pStmt, &p->zErrmsg,
          "SELECT sum(cnt * (1 + rbu_index_cnt(rbu_target_name(tbl))))"
          "FROM rbu_count"
      );
      if( p->rc==SQLITE_OK ){
        if( SQLITE_ROW==sqlite3_step(pStmt) ){
          p->nPhaseOneStep = sqlite3_column_int64(pStmt, 0);
        }
        p->rc = sqlite3_finalize(pStmt);
      }
    }
  }
}


static sqlite3rbu *openRbuHandle(
  const char *zTarget,
  const char *zRbu,
  const char *zState
){
  sqlite3rbu *p;
  size_t nTarget = zTarget ? strlen(zTarget) : 0;
  size_t nRbu = strlen(zRbu);
  size_t nByte = sizeof(sqlite3rbu) + nTarget+1 + nRbu+1;

  p = (sqlite3rbu*)sqlite3_malloc64(nByte);
  if( p ){
    RbuState *pState = 0;

    /* Create the custom VFS. */
    memset(p, 0, sizeof(sqlite3rbu));
    sqlite3rbu_rename_handler(p, 0, 0);
    rbuCreateVfs(p);

    /* Open the target, RBU and state databases */
    if( p->rc==SQLITE_OK ){
      char *pCsr = (char*)&p[1];
      int bRetry = 0;
      if( zTarget ){
        p->zTarget = pCsr;
        memcpy(p->zTarget, zTarget, nTarget+1);
        pCsr += nTarget+1;
      }
      p->zRbu = pCsr;
      memcpy(p->zRbu, zRbu, nRbu+1);
      pCsr += nRbu+1;
      if( zState ){
        p->zState = rbuMPrintf(p, "%s", zState);
      }

      /* If the first attempt to open the database file fails and the bRetry
      ** flag it set, this means that the db was not opened because it seemed
      ** to be a wal-mode db. But, this may have happened due to an earlier
      ** RBU vacuum operation leaving an old wal file in the directory.
      ** If this is the case, it will have been checkpointed and deleted
      ** when the handle was closed and a second attempt to open the
      ** database may succeed.  */
      rbuOpenDatabase(p, 0, &bRetry);
      if( bRetry ){
        rbuOpenDatabase(p, 0, 0);
      }
    }

    if( p->rc==SQLITE_OK ){
      pState = rbuLoadState(p);
      assert( pState || p->rc!=SQLITE_OK );
      if( p->rc==SQLITE_OK ){

        if( pState->eStage==0 ){
          rbuDeleteOalFile(p);
          rbuInitPhaseOneSteps(p);
          p->eStage = RBU_STAGE_OAL;
        }else{
          p->eStage = pState->eStage;
          p->nPhaseOneStep = pState->nPhaseOneStep;
        }
        p->nProgress = pState->nProgress;
        p->iOalSz = pState->iOalSz;
      }
    }
    assert( p->rc!=SQLITE_OK || p->eStage!=0 );

    if( p->rc==SQLITE_OK && p->pTargetFd->pWalFd ){
      if( p->eStage==RBU_STAGE_OAL ){
        p->rc = SQLITE_ERROR;
        p->zErrmsg = sqlite3_mprintf("cannot update wal mode database");
      }else if( p->eStage==RBU_STAGE_MOVE ){
        p->eStage = RBU_STAGE_CKPT;
        p->nStep = 0;
      }
    }

    if( p->rc==SQLITE_OK
     && (p->eStage==RBU_STAGE_OAL || p->eStage==RBU_STAGE_MOVE)
     && pState->eStage!=0
    ){
      rbu_file *pFd = (rbuIsVacuum(p) ? p->pRbuFd : p->pTargetFd);
      if( pFd->iCookie!=pState->iCookie ){
        /* At this point (pTargetFd->iCookie) contains the value of the
        ** change-counter cookie (the thing that gets incremented when a
        ** transaction is committed in rollback mode) currently stored on
        ** page 1 of the database file. */
        p->rc = SQLITE_BUSY;
        p->zErrmsg = sqlite3_mprintf("database modified during rbu %s",
            (rbuIsVacuum(p) ? "vacuum" : "update")
        );
      }
    }

    if( p->rc==SQLITE_OK ){
      if( p->eStage==RBU_STAGE_OAL ){
        sqlite3 *db = p->dbMain;
        p->rc = sqlite3_exec(p->dbRbu, "BEGIN", 0, 0, &p->zErrmsg);

        /* Point the object iterator at the first object */
        if( p->rc==SQLITE_OK ){
          p->rc = rbuObjIterFirst(p, &p->objiter);
        }

        /* If the RBU database contains no data_xxx tables, declare the RBU
        ** update finished.  */
        if( p->rc==SQLITE_OK && p->objiter.zTbl==0 ){
          p->rc = SQLITE_DONE;
          p->eStage = RBU_STAGE_DONE;
        }else{
          if( p->rc==SQLITE_OK && pState->eStage==0 && rbuIsVacuum(p) ){
            rbuCopyPragma(p, "page_size");
            rbuCopyPragma(p, "auto_vacuum");
          }

          /* Open transactions both databases. The *-oal file is opened or
          ** created at this point. */
          if( p->rc==SQLITE_OK ){
            p->rc = sqlite3_exec(db, "BEGIN IMMEDIATE", 0, 0, &p->zErrmsg);
          }

          /* Check if the main database is a zipvfs db. If it is, set the upper
          ** level pager to use "journal_mode=off". This prevents it from
          ** generating a large journal using a temp file.  */
          if( p->rc==SQLITE_OK ){
            int frc = sqlite3_file_control(db, "main", SQLITE_FCNTL_ZIPVFS, 0);
            if( frc==SQLITE_OK ){
              p->rc = sqlite3_exec(
                db, "PRAGMA journal_mode=off",0,0,&p->zErrmsg);
            }
          }

          if( p->rc==SQLITE_OK ){
            rbuSetupOal(p, pState);
          }
        }
      }else if( p->eStage==RBU_STAGE_MOVE ){
        /* no-op */
      }else if( p->eStage==RBU_STAGE_CKPT ){
        if( !rbuIsVacuum(p) && rbuExclusiveCheckpoint(p->dbMain) ){
          /* If the rbu_exclusive_checkpoint=1 URI parameter was specified
          ** and an incremental checkpoint is being resumed, attempt an
          ** exclusive lock on the db file. If this fails, so be it.  */
          p->eStage = RBU_STAGE_DONE;
          rbuLockDatabase(p->dbMain);
          p->eStage = RBU_STAGE_CKPT;
        }
        rbuSetupCheckpoint(p, pState);
      }else if( p->eStage==RBU_STAGE_DONE ){
        p->rc = SQLITE_DONE;
      }else{
        p->rc = SQLITE_CORRUPT;
      }
    }

    rbuFreeState(pState);
  }

  return p;
}

/*
** Allocate and return an RBU handle with all fields zeroed except for the
** error code, which is set to SQLITE_MISUSE.
*/
static sqlite3rbu *rbuMisuseError(void){
  sqlite3rbu *pRet;
  pRet = sqlite3_malloc64(sizeof(sqlite3rbu));
  if( pRet ){
    memset(pRet, 0, sizeof(sqlite3rbu));
    pRet->rc = SQLITE_MISUSE;
  }
  return pRet;
}

/*
** Open and return a new RBU handle.
*/
SQLITE_API sqlite3rbu *sqlite3rbu_open(
  const char *zTarget,
  const char *zRbu,
  const char *zState
){
  if( zTarget==0 || zRbu==0 ){ return rbuMisuseError(); }
  return openRbuHandle(zTarget, zRbu, zState);
}

/*
** Open a handle to begin or resume an RBU VACUUM operation.
*/
SQLITE_API sqlite3rbu *sqlite3rbu_vacuum(
  const char *zTarget,
  const char *zState
){
  if( zTarget==0 ){ return rbuMisuseError(); }
  if( zState ){
    size_t n = strlen(zState);
    if( n>=7 && 0==memcmp("-vactmp", &zState[n-7], 7) ){
      return rbuMisuseError();
    }
  }
  /* TODO: Check that both arguments are non-NULL */
  return openRbuHandle(0, zTarget, zState);
}

/*
** Return the database handle used by pRbu.
*/
SQLITE_API sqlite3 *sqlite3rbu_db(sqlite3rbu *pRbu, int bRbu){
  sqlite3 *db = 0;
  if( pRbu ){
    db = (bRbu ? pRbu->dbRbu : pRbu->dbMain);
  }
  return db;
}


/*
** If the error code currently stored in the RBU handle is SQLITE_CONSTRAINT,
** then edit any error message string so as to remove all occurrences of
** the pattern "rbu_imp_[0-9]*".
*/
static void rbuEditErrmsg(sqlite3rbu *p){
  if( p->rc==SQLITE_CONSTRAINT && p->zErrmsg ){
    unsigned int i;
    size_t nErrmsg = strlen(p->zErrmsg);
    for(i=0; i<(nErrmsg-8); i++){
      if( memcmp(&p->zErrmsg[i], "rbu_imp_", 8)==0 ){
        int nDel = 8;
        while( p->zErrmsg[i+nDel]>='0' && p->zErrmsg[i+nDel]<='9' ) nDel++;
        memmove(&p->zErrmsg[i], &p->zErrmsg[i+nDel], nErrmsg + 1 - i - nDel);
        nErrmsg -= nDel;
      }
    }
  }
}

/*
** Close the RBU handle.
*/
SQLITE_API int sqlite3rbu_close(sqlite3rbu *p, char **pzErrmsg){
  int rc;
  if( p ){

    /* Commit the transaction to the *-oal file. */
    if( p->rc==SQLITE_OK && p->eStage==RBU_STAGE_OAL ){
      p->rc = sqlite3_exec(p->dbMain, "COMMIT", 0, 0, &p->zErrmsg);
    }

    /* Sync the db file if currently doing an incremental checkpoint */
    if( p->rc==SQLITE_OK && p->eStage==RBU_STAGE_CKPT ){
      sqlite3_file *pDb = p->pTargetFd->pReal;
      p->rc = pDb->pMethods->xSync(pDb, SQLITE_SYNC_NORMAL);
    }

    rbuSaveState(p, p->eStage);

    if( p->rc==SQLITE_OK && p->eStage==RBU_STAGE_OAL ){
      p->rc = sqlite3_exec(p->dbRbu, "COMMIT", 0, 0, &p->zErrmsg);
    }

    /* Close any open statement handles. */
    rbuObjIterFinalize(&p->objiter);

    /* If this is an RBU vacuum handle and the vacuum has either finished
    ** successfully or encountered an error, delete the contents of the
    ** state table. This causes the next call to sqlite3rbu_vacuum()
    ** specifying the current target and state databases to start a new
    ** vacuum from scratch.  */
    if( rbuIsVacuum(p) && p->rc!=SQLITE_OK && p->dbRbu ){
      int rc2 = sqlite3_exec(p->dbRbu, "DELETE FROM stat.rbu_state", 0, 0, 0);
      if( p->rc==SQLITE_DONE && rc2!=SQLITE_OK ) p->rc = rc2;
    }

    /* Close the open database handle and VFS object. */
    sqlite3_close(p->dbRbu);
    sqlite3_close(p->dbMain);
    assert( p->szTemp==0 );
    rbuDeleteVfs(p);
    sqlite3_free(p->aBuf);
    sqlite3_free(p->aFrame);

    rbuEditErrmsg(p);
    rc = p->rc;
    if( pzErrmsg ){
      *pzErrmsg = p->zErrmsg;
    }else{
      sqlite3_free(p->zErrmsg);
    }
    sqlite3_free(p->zState);
    sqlite3_free(p);
  }else{
    rc = SQLITE_NOMEM;
    *pzErrmsg = 0;
  }
  return rc;
}

/*
** Return the total number of key-value operations (inserts, deletes or
** updates) that have been performed on the target database since the
** current RBU update was started.
*/
SQLITE_API sqlite3_int64 sqlite3rbu_progress(sqlite3rbu *pRbu){
  return pRbu->nProgress;
}

/*
** Return permyriadage progress indications for the two main stages of
** an RBU update.
*/
SQLITE_API void sqlite3rbu_bp_progress(sqlite3rbu *p, int *pnOne, int *pnTwo){
  const int MAX_PROGRESS = 10000;
  switch( p->eStage ){
    case RBU_STAGE_OAL:
      if( p->nPhaseOneStep>0 ){
        *pnOne = (int)(MAX_PROGRESS * (i64)p->nProgress/(i64)p->nPhaseOneStep);
      }else{
        *pnOne = -1;
      }
      *pnTwo = 0;
      break;

    case RBU_STAGE_MOVE:
      *pnOne = MAX_PROGRESS;
      *pnTwo = 0;
      break;

    case RBU_STAGE_CKPT:
      *pnOne = MAX_PROGRESS;
      *pnTwo = (int)(MAX_PROGRESS * (i64)p->nStep / (i64)p->nFrame);
      break;

    case RBU_STAGE_DONE:
      *pnOne = MAX_PROGRESS;
      *pnTwo = MAX_PROGRESS;
      break;

    default:
      assert( 0 );
  }
}

/*
** Return the current state of the RBU vacuum or update operation.
*/
SQLITE_API int sqlite3rbu_state(sqlite3rbu *p){
  int aRes[] = {
    0, SQLITE_RBU_STATE_OAL, SQLITE_RBU_STATE_MOVE,
    0, SQLITE_RBU_STATE_CHECKPOINT, SQLITE_RBU_STATE_DONE
  };

  assert( RBU_STAGE_OAL==1 );
  assert( RBU_STAGE_MOVE==2 );
  assert( RBU_STAGE_CKPT==4 );
  assert( RBU_STAGE_DONE==5 );
  assert( aRes[RBU_STAGE_OAL]==SQLITE_RBU_STATE_OAL );
  assert( aRes[RBU_STAGE_MOVE]==SQLITE_RBU_STATE_MOVE );
  assert( aRes[RBU_STAGE_CKPT]==SQLITE_RBU_STATE_CHECKPOINT );
  assert( aRes[RBU_STAGE_DONE]==SQLITE_RBU_STATE_DONE );

  if( p->rc!=SQLITE_OK && p->rc!=SQLITE_DONE ){
    return SQLITE_RBU_STATE_ERROR;
  }else{
    assert( p->rc!=SQLITE_DONE || p->eStage==RBU_STAGE_DONE );
    assert( p->eStage==RBU_STAGE_OAL
         || p->eStage==RBU_STAGE_MOVE
         || p->eStage==RBU_STAGE_CKPT
         || p->eStage==RBU_STAGE_DONE
    );
    return aRes[p->eStage];
  }
}

SQLITE_API int sqlite3rbu_savestate(sqlite3rbu *p){
  int rc = p->rc;
  if( rc==SQLITE_DONE ) return SQLITE_OK;

  assert( p->eStage>=RBU_STAGE_OAL && p->eStage<=RBU_STAGE_DONE );
  if( p->eStage==RBU_STAGE_OAL ){
    assert( rc!=SQLITE_DONE );
    if( rc==SQLITE_OK ) rc = sqlite3_exec(p->dbMain, "COMMIT", 0, 0, 0);
  }

  /* Sync the db file */
  if( rc==SQLITE_OK && p->eStage==RBU_STAGE_CKPT ){
    sqlite3_file *pDb = p->pTargetFd->pReal;
    rc = pDb->pMethods->xSync(pDb, SQLITE_SYNC_NORMAL);
  }

  p->rc = rc;
  rbuSaveState(p, p->eStage);
  rc = p->rc;

  if( p->eStage==RBU_STAGE_OAL ){
    assert( rc!=SQLITE_DONE );
    if( rc==SQLITE_OK ) rc = sqlite3_exec(p->dbRbu, "COMMIT", 0, 0, 0);
    if( rc==SQLITE_OK ){
      const char *zBegin = rbuIsVacuum(p) ? "BEGIN" : "BEGIN IMMEDIATE";
      rc = sqlite3_exec(p->dbRbu, zBegin, 0, 0, 0);
    }
    if( rc==SQLITE_OK ) rc = sqlite3_exec(p->dbMain, "BEGIN IMMEDIATE", 0, 0,0);
  }

  p->rc = rc;
  return rc;
}

/*
** Default xRename callback for RBU.
*/
static int xDefaultRename(void *pArg, const char *zOld, const char *zNew){
  int rc = SQLITE_OK;
  UNUSED_PARAMETER(pArg);
#if defined(_WIN32_WCE)
  {
    LPWSTR zWideOld;
    LPWSTR zWideNew;

    zWideOld = rbuWinUtf8ToUnicode(zOld);
    if( zWideOld ){
      zWideNew = rbuWinUtf8ToUnicode(zNew);
      if( zWideNew ){
        if( MoveFileW(zWideOld, zWideNew) ){
          rc = SQLITE_OK;
        }else{
          rc = SQLITE_IOERR;
        }
        sqlite3_free(zWideNew);
      }else{
        rc = SQLITE_IOERR_NOMEM;
      }
      sqlite3_free(zWideOld);
    }else{
      rc = SQLITE_IOERR_NOMEM;
    }
  }
#else
  rc = rename(zOld, zNew) ? SQLITE_IOERR : SQLITE_OK;
#endif
  return rc;
}

SQLITE_API void sqlite3rbu_rename_handler(
  sqlite3rbu *pRbu,
  void *pArg,
  int (*xRename)(void *pArg, const char *zOld, const char *zNew)
){
  if( xRename ){
    pRbu->xRename = xRename;
    pRbu->pRenameArg = pArg;
  }else{
    pRbu->xRename = xDefaultRename;
    pRbu->pRenameArg = 0;
  }
}

/**************************************************************************
** Beginning of RBU VFS shim methods. The VFS shim modifies the behaviour
** of a standard VFS in the following ways:
**
** 1. Whenever the first page of a main database file is read or
**    written, the value of the change-counter cookie is stored in
**    rbu_file.iCookie. Similarly, the value of the "write-version"
**    database header field is stored in rbu_file.iWriteVer. This ensures
**    that the values are always trustworthy within an open transaction.
**
** 2. Whenever an SQLITE_OPEN_WAL file is opened, the (rbu_file.pWalFd)
**    member variable of the associated database file descriptor is set
**    to point to the new file. A mutex protected linked list of all main
**    db fds opened using a particular RBU VFS is maintained at
**    rbu_vfs.pMain to facilitate this.
**
** 3. Using a new file-control "SQLITE_FCNTL_RBU", a main db rbu_file
**    object can be marked as the target database of an RBU update. This
**    turns on the following extra special behaviour:
**
** 3a. If xAccess() is called to check if there exists a *-wal file
**     associated with an RBU target database currently in RBU_STAGE_OAL
**     stage (preparing the *-oal file), the following special handling
**     applies:
**
**      * if the *-wal file does exist, return SQLITE_CANTOPEN. An RBU
**        target database may not be in wal mode already.
**
**      * if the *-wal file does not exist, set the output parameter to
**        non-zero (to tell SQLite that it does exist) anyway.
**
**     Then, when xOpen() is called to open the *-wal file associated with
**     the RBU target in RBU_STAGE_OAL stage, instead of opening the *-wal
**     file, the rbu vfs opens the corresponding *-oal file instead.
**
** 3b. The *-shm pages returned by xShmMap() for a target db file in
**     RBU_STAGE_OAL mode are actually stored in heap memory. This is to
**     avoid creating a *-shm file on disk. Additionally, xShmLock() calls
**     are no-ops on target database files in RBU_STAGE_OAL mode. This is
**     because assert() statements in some VFS implementations fail if
**     xShmLock() is called before xShmMap().
**
** 3c. If an EXCLUSIVE lock is attempted on a target database file in any
**     mode except RBU_STAGE_DONE (all work completed and checkpointed), it
**     fails with an SQLITE_BUSY error. This is to stop RBU connections
**     from automatically checkpointing a *-wal (or *-oal) file from within
**     sqlite3_close().
**
** 3d. In RBU_STAGE_CAPTURE mode, all xRead() calls on the wal file, and
**     all xWrite() calls on the target database file perform no IO.
**     Instead the frame and page numbers that would be read and written
**     are recorded. Additionally, successful attempts to obtain exclusive
**     xShmLock() WRITER, CHECKPOINTER and READ0 locks on the target
**     database file are recorded. xShmLock() calls to unlock the same
**     locks are no-ops (so that once obtained, these locks are never
**     relinquished). Finally, calls to xSync() on the target database
**     file fail with SQLITE_NOTICE errors.
*/

static void rbuUnlockShm(rbu_file *p){
  assert( p->openFlags & SQLITE_OPEN_MAIN_DB );
  if( p->pRbu ){
    int (*xShmLock)(sqlite3_file*,int,int,int) = p->pReal->pMethods->xShmLock;
    int i;
    for(i=0; i<SQLITE_SHM_NLOCK;i++){
      if( (1<<i) & p->pRbu->mLock ){
        xShmLock(p->pReal, i, 1, SQLITE_SHM_UNLOCK|SQLITE_SHM_EXCLUSIVE);
      }
    }
    p->pRbu->mLock = 0;
  }
}

/*
*/
static int rbuUpdateTempSize(rbu_file *pFd, sqlite3_int64 nNew){
  sqlite3rbu *pRbu = pFd->pRbu;
  i64 nDiff = nNew - pFd->sz;
  pRbu->szTemp += nDiff;
  pFd->sz = nNew;
  assert( pRbu->szTemp>=0 );
  if( pRbu->szTempLimit && pRbu->szTemp>pRbu->szTempLimit ) return SQLITE_FULL;
  return SQLITE_OK;
}

/*
** Add an item to the main-db lists, if it is not already present.
**
** There are two main-db lists. One for all file descriptors, and one
** for all file descriptors with rbu_file.pDb!=0. If the argument has
** rbu_file.pDb!=0, then it is assumed to already be present on the
** main list and is only added to the pDb!=0 list.
*/
static void rbuMainlistAdd(rbu_file *p){
  rbu_vfs *pRbuVfs = p->pRbuVfs;
  rbu_file *pIter;
  assert( (p->openFlags & SQLITE_OPEN_MAIN_DB) );
  sqlite3_mutex_enter(pRbuVfs->mutex);
  if( p->pRbu==0 ){
    for(pIter=pRbuVfs->pMain; pIter; pIter=pIter->pMainNext);
    p->pMainNext = pRbuVfs->pMain;
    pRbuVfs->pMain = p;
  }else{
    for(pIter=pRbuVfs->pMainRbu; pIter && pIter!=p; pIter=pIter->pMainRbuNext){}
    if( pIter==0 ){
      p->pMainRbuNext = pRbuVfs->pMainRbu;
      pRbuVfs->pMainRbu = p;
    }
  }
  sqlite3_mutex_leave(pRbuVfs->mutex);
}

/*
** Remove an item from the main-db lists.
*/
static void rbuMainlistRemove(rbu_file *p){
  rbu_file **pp;
  sqlite3_mutex_enter(p->pRbuVfs->mutex);
  for(pp=&p->pRbuVfs->pMain; *pp && *pp!=p; pp=&((*pp)->pMainNext)){}
  if( *pp ) *pp = p->pMainNext;
  p->pMainNext = 0;
  for(pp=&p->pRbuVfs->pMainRbu; *pp && *pp!=p; pp=&((*pp)->pMainRbuNext)){}
  if( *pp ) *pp = p->pMainRbuNext;
  p->pMainRbuNext = 0;
  sqlite3_mutex_leave(p->pRbuVfs->mutex);
}

/*
** Given that zWal points to a buffer containing a wal file name passed to
** either the xOpen() or xAccess() VFS method, search the main-db list for
** a file-handle opened by the same database connection on the corresponding
** database file.
**
** If parameter bRbu is true, only search for file-descriptors with
** rbu_file.pDb!=0.
*/
static rbu_file *rbuFindMaindb(rbu_vfs *pRbuVfs, const char *zWal, int bRbu){
  rbu_file *pDb;
  sqlite3_mutex_enter(pRbuVfs->mutex);
  if( bRbu ){
    for(pDb=pRbuVfs->pMainRbu; pDb && pDb->zWal!=zWal; pDb=pDb->pMainRbuNext){}
  }else{
    for(pDb=pRbuVfs->pMain; pDb && pDb->zWal!=zWal; pDb=pDb->pMainNext){}
  }
  sqlite3_mutex_leave(pRbuVfs->mutex);
  return pDb;
}

/*
** Close an rbu file.
*/
static int rbuVfsClose(sqlite3_file *pFile){
  rbu_file *p = (rbu_file*)pFile;
  int rc;
  int i;

  /* Free the contents of the apShm[] array. And the array itself. */
  for(i=0; i<p->nShm; i++){
    sqlite3_free(p->apShm[i]);
  }
  sqlite3_free(p->apShm);
  p->apShm = 0;
  sqlite3_free(p->zDel);

  if( p->openFlags & SQLITE_OPEN_MAIN_DB ){
    const sqlite3_io_methods *pMeth = p->pReal->pMethods;
    rbuMainlistRemove(p);
    rbuUnlockShm(p);
    if( pMeth->iVersion>1 && pMeth->xShmUnmap ){
      pMeth->xShmUnmap(p->pReal, 0);
    }
  }
  else if( (p->openFlags & SQLITE_OPEN_DELETEONCLOSE) && p->pRbu ){
    rbuUpdateTempSize(p, 0);
  }
  assert( p->pMainNext==0 && p->pRbuVfs->pMain!=p );

  /* Close the underlying file handle */
  rc = p->pReal->pMethods->xClose(p->pReal);
  return rc;
}


/*
** Read and return an unsigned 32-bit big-endian integer from the buffer
** passed as the only argument.
*/
static u32 rbuGetU32(u8 *aBuf){
  return ((u32)aBuf[0] << 24)
       + ((u32)aBuf[1] << 16)
       + ((u32)aBuf[2] <<  8)
       + ((u32)aBuf[3]);
}

/*
** Write an unsigned 32-bit value in big-endian format to the supplied
** buffer.
*/
static void rbuPutU32(u8 *aBuf, u32 iVal){
  aBuf[0] = (iVal >> 24) & 0xFF;
  aBuf[1] = (iVal >> 16) & 0xFF;
  aBuf[2] = (iVal >>  8) & 0xFF;
  aBuf[3] = (iVal >>  0) & 0xFF;
}

static void rbuPutU16(u8 *aBuf, u16 iVal){
  aBuf[0] = (iVal >>  8) & 0xFF;
  aBuf[1] = (iVal >>  0) & 0xFF;
}

/*
** Read data from an rbuVfs-file.
*/
static int rbuVfsRead(
  sqlite3_file *pFile,
  void *zBuf,
  int iAmt,
  sqlite_int64 iOfst
){
  rbu_file *p = (rbu_file*)pFile;
  sqlite3rbu *pRbu = p->pRbu;
  int rc;

  if( pRbu && pRbu->eStage==RBU_STAGE_CAPTURE ){
    assert( p->openFlags & SQLITE_OPEN_WAL );
    rc = rbuCaptureWalRead(p->pRbu, iOfst, iAmt);
  }else{
    if( pRbu && pRbu->eStage==RBU_STAGE_OAL
     && (p->openFlags & SQLITE_OPEN_WAL)
     && iOfst>=pRbu->iOalSz
    ){
      rc = SQLITE_OK;
      memset(zBuf, 0, iAmt);
    }else{
      rc = p->pReal->pMethods->xRead(p->pReal, zBuf, iAmt, iOfst);
#if 1
      /* If this is being called to read the first page of the target
      ** database as part of an rbu vacuum operation, synthesize the
      ** contents of the first page if it does not yet exist. Otherwise,
      ** SQLite will not check for a *-wal file.  */
      if( pRbu && rbuIsVacuum(pRbu)
          && rc==SQLITE_IOERR_SHORT_READ && iOfst==0
          && (p->openFlags & SQLITE_OPEN_MAIN_DB)
          && pRbu->rc==SQLITE_OK
      ){
        sqlite3_file *pFd = (sqlite3_file*)pRbu->pRbuFd;
        rc = pFd->pMethods->xRead(pFd, zBuf, iAmt, iOfst);
        if( rc==SQLITE_OK ){
          u8 *aBuf = (u8*)zBuf;
          u32 iRoot = rbuGetU32(&aBuf[52]) ? 1 : 0;
          rbuPutU32(&aBuf[52], iRoot);      /* largest root page number */
          rbuPutU32(&aBuf[36], 0);          /* number of free pages */
          rbuPutU32(&aBuf[32], 0);          /* first page on free list trunk */
          rbuPutU32(&aBuf[28], 1);          /* size of db file in pages */
          rbuPutU32(&aBuf[24], pRbu->pRbuFd->iCookie+1);  /* Change counter */

          if( iAmt>100 ){
            memset(&aBuf[100], 0, iAmt-100);
            rbuPutU16(&aBuf[105], iAmt & 0xFFFF);
            aBuf[100] = 0x0D;
          }
        }
      }
#endif
    }
    if( rc==SQLITE_OK && iOfst==0 && (p->openFlags & SQLITE_OPEN_MAIN_DB) ){
      /* These look like magic numbers. But they are stable, as they are part
       ** of the definition of the SQLite file format, which may not change. */
      u8 *pBuf = (u8*)zBuf;
      p->iCookie = rbuGetU32(&pBuf[24]);
      p->iWriteVer = pBuf[19];
    }
  }
  return rc;
}

/*
** Write data to an rbuVfs-file.
*/
static int rbuVfsWrite(
  sqlite3_file *pFile,
  const void *zBuf,
  int iAmt,
  sqlite_int64 iOfst
){
  rbu_file *p = (rbu_file*)pFile;
  sqlite3rbu *pRbu = p->pRbu;
  int rc;

  if( pRbu && pRbu->eStage==RBU_STAGE_CAPTURE ){
    assert( p->openFlags & SQLITE_OPEN_MAIN_DB );
    rc = rbuCaptureDbWrite(p->pRbu, iOfst);
  }else{
    if( pRbu ){
      if( pRbu->eStage==RBU_STAGE_OAL
       && (p->openFlags & SQLITE_OPEN_WAL)
       && iOfst>=pRbu->iOalSz
      ){
        pRbu->iOalSz = iAmt + iOfst;
      }else if( p->openFlags & SQLITE_OPEN_DELETEONCLOSE ){
        i64 szNew = iAmt+iOfst;
        if( szNew>p->sz ){
          rc = rbuUpdateTempSize(p, szNew);
          if( rc!=SQLITE_OK ) return rc;
        }
      }
    }
    rc = p->pReal->pMethods->xWrite(p->pReal, zBuf, iAmt, iOfst);
    if( rc==SQLITE_OK && iOfst==0 && (p->openFlags & SQLITE_OPEN_MAIN_DB) ){
      /* These look like magic numbers. But they are stable, as they are part
      ** of the definition of the SQLite file format, which may not change. */
      u8 *pBuf = (u8*)zBuf;
      p->iCookie = rbuGetU32(&pBuf[24]);
      p->iWriteVer = pBuf[19];
    }
  }
  return rc;
}

/*
** Truncate an rbuVfs-file.
*/
static int rbuVfsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  rbu_file *p = (rbu_file*)pFile;
  if( (p->openFlags & SQLITE_OPEN_DELETEONCLOSE) && p->pRbu ){
    int rc = rbuUpdateTempSize(p, size);
    if( rc!=SQLITE_OK ) return rc;
  }
  return p->pReal->pMethods->xTruncate(p->pReal, size);
}

/*
** Sync an rbuVfs-file.
*/
static int rbuVfsSync(sqlite3_file *pFile, int flags){
  rbu_file *p = (rbu_file *)pFile;
  if( p->pRbu && p->pRbu->eStage==RBU_STAGE_CAPTURE ){
    if( p->openFlags & SQLITE_OPEN_MAIN_DB ){
      return SQLITE_NOTICE_RBU;
    }
    return SQLITE_OK;
  }
  return p->pReal->pMethods->xSync(p->pReal, flags);
}

/*
** Return the current file-size of an rbuVfs-file.
*/
static int rbuVfsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  rbu_file *p = (rbu_file *)pFile;
  int rc;
  rc = p->pReal->pMethods->xFileSize(p->pReal, pSize);

  /* If this is an RBU vacuum operation and this is the target database,
  ** pretend that it has at least one page. Otherwise, SQLite will not
  ** check for the existence of a *-wal file. rbuVfsRead() contains
  ** similar logic.  */
  if( rc==SQLITE_OK && *pSize==0
   && p->pRbu && rbuIsVacuum(p->pRbu)
   && (p->openFlags & SQLITE_OPEN_MAIN_DB)
  ){
    *pSize = 1024;
  }
  return rc;
}

/*
** Lock an rbuVfs-file.
*/
static int rbuVfsLock(sqlite3_file *pFile, int eLock){
  rbu_file *p = (rbu_file*)pFile;
  sqlite3rbu *pRbu = p->pRbu;
  int rc = SQLITE_OK;

  assert( p->openFlags & (SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_TEMP_DB) );
  if( eLock==SQLITE_LOCK_EXCLUSIVE
   && (p->bNolock || (pRbu && pRbu->eStage!=RBU_STAGE_DONE))
  ){
    /* Do not allow EXCLUSIVE locks. Preventing SQLite from taking this
    ** prevents it from checkpointing the database from sqlite3_close(). */
    rc = SQLITE_BUSY;
  }else{
    rc = p->pReal->pMethods->xLock(p->pReal, eLock);
  }

  return rc;
}

/*
** Unlock an rbuVfs-file.
*/
static int rbuVfsUnlock(sqlite3_file *pFile, int eLock){
  rbu_file *p = (rbu_file *)pFile;
  return p->pReal->pMethods->xUnlock(p->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an rbuVfs-file.
*/
static int rbuVfsCheckReservedLock(sqlite3_file *pFile, int *pResOut){
  rbu_file *p = (rbu_file *)pFile;
  return p->pReal->pMethods->xCheckReservedLock(p->pReal, pResOut);
}

/*
** File control method. For custom operations on an rbuVfs-file.
*/
static int rbuVfsFileControl(sqlite3_file *pFile, int op, void *pArg){
  rbu_file *p = (rbu_file *)pFile;
  int (*xControl)(sqlite3_file*,int,void*) = p->pReal->pMethods->xFileControl;
  int rc;

  assert( p->openFlags & (SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_TEMP_DB)
       || p->openFlags & (SQLITE_OPEN_TRANSIENT_DB|SQLITE_OPEN_TEMP_JOURNAL)
  );
  if( op==SQLITE_FCNTL_RBU ){
    sqlite3rbu *pRbu = (sqlite3rbu*)pArg;

    /* First try to find another RBU vfs lower down in the vfs stack. If
    ** one is found, this vfs will operate in pass-through mode. The lower
    ** level vfs will do the special RBU handling.  */
    rc = xControl(p->pReal, op, pArg);

    if( rc==SQLITE_NOTFOUND ){
      /* Now search for a zipvfs instance lower down in the VFS stack. If
      ** one is found, this is an error.  */
      void *dummy = 0;
      rc = xControl(p->pReal, SQLITE_FCNTL_ZIPVFS, &dummy);
      if( rc==SQLITE_OK ){
        rc = SQLITE_ERROR;
        pRbu->zErrmsg = sqlite3_mprintf("rbu/zipvfs setup error");
      }else if( rc==SQLITE_NOTFOUND ){
        pRbu->pTargetFd = p;
        p->pRbu = pRbu;
        rbuMainlistAdd(p);
        if( p->pWalFd ) p->pWalFd->pRbu = pRbu;
        rc = SQLITE_OK;
      }
    }
    return rc;
  }
  else if( op==SQLITE_FCNTL_RBUCNT ){
    sqlite3rbu *pRbu = (sqlite3rbu*)pArg;
    pRbu->nRbu++;
    pRbu->pRbuFd = p;
    p->bNolock = 1;
  }

  rc = xControl(p->pReal, op, pArg);
  if( rc==SQLITE_OK && op==SQLITE_FCNTL_VFSNAME ){
    rbu_vfs *pRbuVfs = p->pRbuVfs;
    char *zIn = *(char**)pArg;
    char *zOut = sqlite3_mprintf("rbu(%s)/%z", pRbuVfs->base.zName, zIn);
    *(char**)pArg = zOut;
    if( zOut==0 ) rc = SQLITE_NOMEM;
  }

  return rc;
}

/*
** Return the sector-size in bytes for an rbuVfs-file.
*/
static int rbuVfsSectorSize(sqlite3_file *pFile){
  rbu_file *p = (rbu_file *)pFile;
  return p->pReal->pMethods->xSectorSize(p->pReal);
}

/*
** Return the device characteristic flags supported by an rbuVfs-file.
*/
static int rbuVfsDeviceCharacteristics(sqlite3_file *pFile){
  rbu_file *p = (rbu_file *)pFile;
  return p->pReal->pMethods->xDeviceCharacteristics(p->pReal);
}

/*
** Take or release a shared-memory lock.
*/
static int rbuVfsShmLock(sqlite3_file *pFile, int ofst, int n, int flags){
  rbu_file *p = (rbu_file*)pFile;
  sqlite3rbu *pRbu = p->pRbu;
  int rc = SQLITE_OK;

#ifdef SQLITE_AMALGAMATION
    assert( WAL_CKPT_LOCK==1 );
#endif

  assert( p->openFlags & (SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_TEMP_DB) );
  if( pRbu && (
       pRbu->eStage==RBU_STAGE_OAL
    || pRbu->eStage==RBU_STAGE_MOVE
    || pRbu->eStage==RBU_STAGE_DONE
  )){
    /* Prevent SQLite from taking a shm-lock on the target file when it
    ** is supplying heap memory to the upper layer in place of *-shm
    ** segments. */
    if( ofst==WAL_LOCK_CKPT && n==1 ) rc = SQLITE_BUSY;
  }else{
    int bCapture = 0;
    if( pRbu && pRbu->eStage==RBU_STAGE_CAPTURE ){
      bCapture = 1;
    }
    if( bCapture==0 || 0==(flags & SQLITE_SHM_UNLOCK) ){
      rc = p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
      if( bCapture && rc==SQLITE_OK ){
        pRbu->mLock |= ((1<<n) - 1) << ofst;
      }
    }
  }

  return rc;
}

/*
** Obtain a pointer to a mapping of a single 32KiB page of the *-shm file.
*/
static int rbuVfsShmMap(
  sqlite3_file *pFile,
  int iRegion,
  int szRegion,
  int isWrite,
  void volatile **pp
){
  rbu_file *p = (rbu_file*)pFile;
  int rc = SQLITE_OK;
  int eStage = (p->pRbu ? p->pRbu->eStage : 0);

  /* If not in RBU_STAGE_OAL, allow this call to pass through. Or, if this
  ** rbu is in the RBU_STAGE_OAL state, use heap memory for *-shm space
  ** instead of a file on disk.  */
  assert( p->openFlags & (SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_TEMP_DB) );
  if( eStage==RBU_STAGE_OAL ){
    sqlite3_int64 nByte = (iRegion+1) * sizeof(char*);
    char **apNew = (char**)sqlite3_realloc64(p->apShm, nByte);

    /* This is an RBU connection that uses its own heap memory for the
    ** pages of the *-shm file. Since no other process can have run
    ** recovery, the connection must request *-shm pages in order
    ** from start to finish.  */
    assert( iRegion==p->nShm );
    if( apNew==0 ){
      rc = SQLITE_NOMEM;
    }else{
      memset(&apNew[p->nShm], 0, sizeof(char*) * (1 + iRegion - p->nShm));
      p->apShm = apNew;
      p->nShm = iRegion+1;
    }

    if( rc==SQLITE_OK ){
      char *pNew = (char*)sqlite3_malloc64(szRegion);
      if( pNew==0 ){
        rc = SQLITE_NOMEM;
      }else{
        memset(pNew, 0, szRegion);
        p->apShm[iRegion] = pNew;
      }
    }

    if( rc==SQLITE_OK ){
      *pp = p->apShm[iRegion];
    }else{
      *pp = 0;
    }
  }else{
    assert( p->apShm==0 );
    rc = p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
  }

  return rc;
}

/*
** Memory barrier.
*/
static void rbuVfsShmBarrier(sqlite3_file *pFile){
  rbu_file *p = (rbu_file *)pFile;
  p->pReal->pMethods->xShmBarrier(p->pReal);
}

/*
** The xShmUnmap method.
*/
static int rbuVfsShmUnmap(sqlite3_file *pFile, int delFlag){
  rbu_file *p = (rbu_file*)pFile;
  int rc = SQLITE_OK;
  int eStage = (p->pRbu ? p->pRbu->eStage : 0);

  assert( p->openFlags & (SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_TEMP_DB) );
  if( eStage==RBU_STAGE_OAL || eStage==RBU_STAGE_MOVE ){
    /* no-op */
  }else{
    /* Release the checkpointer and writer locks */
    rbuUnlockShm(p);
    rc = p->pReal->pMethods->xShmUnmap(p->pReal, delFlag);
  }
  return rc;
}

/*
** Open an rbu file handle.
*/
static int rbuVfsOpen(
  sqlite3_vfs *pVfs,
  const char *zName,
  sqlite3_file *pFile,
  int flags,
  int *pOutFlags
){
  static sqlite3_io_methods rbuvfs_io_methods = {
    2,                            /* iVersion */
    rbuVfsClose,                  /* xClose */
    rbuVfsRead,                   /* xRead */
    rbuVfsWrite,                  /* xWrite */
    rbuVfsTruncate,               /* xTruncate */
    rbuVfsSync,                   /* xSync */
    rbuVfsFileSize,               /* xFileSize */
    rbuVfsLock,                   /* xLock */
    rbuVfsUnlock,                 /* xUnlock */
    rbuVfsCheckReservedLock,      /* xCheckReservedLock */
    rbuVfsFileControl,            /* xFileControl */
    rbuVfsSectorSize,             /* xSectorSize */
    rbuVfsDeviceCharacteristics,  /* xDeviceCharacteristics */
    rbuVfsShmMap,                 /* xShmMap */
    rbuVfsShmLock,                /* xShmLock */
    rbuVfsShmBarrier,             /* xShmBarrier */
    rbuVfsShmUnmap,               /* xShmUnmap */
    0, 0                          /* xFetch, xUnfetch */
  };
  static sqlite3_io_methods rbuvfs_io_methods1 = {
    1,                            /* iVersion */
    rbuVfsClose,                  /* xClose */
    rbuVfsRead,                   /* xRead */
    rbuVfsWrite,                  /* xWrite */
    rbuVfsTruncate,               /* xTruncate */
    rbuVfsSync,                   /* xSync */
    rbuVfsFileSize,               /* xFileSize */
    rbuVfsLock,                   /* xLock */
    rbuVfsUnlock,                 /* xUnlock */
    rbuVfsCheckReservedLock,      /* xCheckReservedLock */
    rbuVfsFileControl,            /* xFileControl */
    rbuVfsSectorSize,             /* xSectorSize */
    rbuVfsDeviceCharacteristics,  /* xDeviceCharacteristics */
    0, 0, 0, 0, 0, 0
  };



  rbu_vfs *pRbuVfs = (rbu_vfs*)pVfs;
  sqlite3_vfs *pRealVfs = pRbuVfs->pRealVfs;
  rbu_file *pFd = (rbu_file *)pFile;
  int rc = SQLITE_OK;
  const char *zOpen = zName;
  int oflags = flags;

  memset(pFd, 0, sizeof(rbu_file));
  pFd->pReal = (sqlite3_file*)&pFd[1];
  pFd->pRbuVfs = pRbuVfs;
  pFd->openFlags = flags;
  if( zName ){
    if( flags & SQLITE_OPEN_MAIN_DB ){
      /* A main database has just been opened. The following block sets
      ** (pFd->zWal) to point to a buffer owned by SQLite that contains
      ** the name of the *-wal file this db connection will use. SQLite
      ** happens to pass a pointer to this buffer when using xAccess()
      ** or xOpen() to operate on the *-wal file.  */
      pFd->zWal = sqlite3_filename_wal(zName);
    }
    else if( flags & SQLITE_OPEN_WAL ){
      rbu_file *pDb = rbuFindMaindb(pRbuVfs, zName, 0);
      if( pDb ){
        if( pDb->pRbu && pDb->pRbu->eStage==RBU_STAGE_OAL ){
          /* This call is to open a *-wal file. Intead, open the *-oal. */
          size_t nOpen;
          if( rbuIsVacuum(pDb->pRbu) ){
            zOpen = sqlite3_db_filename(pDb->pRbu->dbRbu, "main");
            zOpen = sqlite3_filename_wal(zOpen);
          }
          nOpen = strlen(zOpen);
          ((char*)zOpen)[nOpen-3] = 'o';
          pFd->pRbu = pDb->pRbu;
        }
        pDb->pWalFd = pFd;
      }
    }
  }else{
    pFd->pRbu = pRbuVfs->pRbu;
  }

  if( oflags & SQLITE_OPEN_MAIN_DB
   && sqlite3_uri_boolean(zName, "rbu_memory", 0)
  ){
    assert( oflags & SQLITE_OPEN_MAIN_DB );
    oflags =  SQLITE_OPEN_TEMP_DB | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
              SQLITE_OPEN_EXCLUSIVE | SQLITE_OPEN_DELETEONCLOSE;
    zOpen = 0;
  }

  if( rc==SQLITE_OK ){
    rc = pRealVfs->xOpen(pRealVfs, zOpen, pFd->pReal, oflags, pOutFlags);
  }
  if( pFd->pReal->pMethods ){
    const sqlite3_io_methods *pMeth = pFd->pReal->pMethods;
    /* The xOpen() operation has succeeded. Set the sqlite3_file.pMethods
    ** pointer and, if the file is a main database file, link it into the
    ** mutex protected linked list of all such files.  */
    if( pMeth->iVersion<2 || pMeth->xShmLock==0 ){
      pFile->pMethods = &rbuvfs_io_methods1;
    }else{
      pFile->pMethods = &rbuvfs_io_methods;
    }
    if( flags & SQLITE_OPEN_MAIN_DB ){
      rbuMainlistAdd(pFd);
    }
  }else{
    sqlite3_free(pFd->zDel);
  }

  return rc;
}

/*
** Delete the file located at zPath.
*/
static int rbuVfsDelete(sqlite3_vfs *pVfs, const char *zPath, int dirSync){
  sqlite3_vfs *pRealVfs = ((rbu_vfs*)pVfs)->pRealVfs;
  return pRealVfs->xDelete(pRealVfs, zPath, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static int rbuVfsAccess(
  sqlite3_vfs *pVfs,
  const char *zPath,
  int flags,
  int *pResOut
){
  rbu_vfs *pRbuVfs = (rbu_vfs*)pVfs;
  sqlite3_vfs *pRealVfs = pRbuVfs->pRealVfs;
  int rc;

  rc = pRealVfs->xAccess(pRealVfs, zPath, flags, pResOut);

  /* If this call is to check if a *-wal file associated with an RBU target
  ** database connection exists, and the RBU update is in RBU_STAGE_OAL,
  ** the following special handling is activated:
  **
  **   a) if the *-wal file does exist, return SQLITE_CANTOPEN. This
  **      ensures that the RBU extension never tries to update a database
  **      in wal mode, even if the first page of the database file has
  **      been damaged.
  **
  **   b) if the *-wal file does not exist, claim that it does anyway,
  **      causing SQLite to call xOpen() to open it. This call will also
  **      be intercepted (see the rbuVfsOpen() function) and the *-oal
  **      file opened instead.
  */
  if( rc==SQLITE_OK && flags==SQLITE_ACCESS_EXISTS ){
    rbu_file *pDb = rbuFindMaindb(pRbuVfs, zPath, 1);
    if( pDb && pDb->pRbu->eStage==RBU_STAGE_OAL ){
      assert( pDb->pRbu );
      if( *pResOut ){
        rc = SQLITE_CANTOPEN;
      }else{
        sqlite3_int64 sz = 0;
        rc = rbuVfsFileSize(&pDb->base, &sz);
        *pResOut = (sz>0);
      }
    }
  }

  return rc;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
static int rbuVfsFullPathname(
  sqlite3_vfs *pVfs,
  const char *zPath,
  int nOut,
  char *zOut
){
  sqlite3_vfs *pRealVfs = ((rbu_vfs*)pVfs)->pRealVfs;
  return pRealVfs->xFullPathname(pRealVfs, zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *rbuVfsDlOpen(sqlite3_vfs *pVfs, const char *zPath){
  sqlite3_vfs *pRealVfs = ((rbu_vfs*)pVfs)->pRealVfs;
  return pRealVfs->xDlOpen(pRealVfs, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated
** with dynamic libraries.
*/
static void rbuVfsDlError(sqlite3_vfs *pVfs, int nByte, char *zErrMsg){
  sqlite3_vfs *pRealVfs = ((rbu_vfs*)pVfs)->pRealVfs;
  pRealVfs->xDlError(pRealVfs, nByte, zErrMsg);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
static void (*rbuVfsDlSym(
  sqlite3_vfs *pVfs,
  void *pArg,
  const char *zSym
))(void){
  sqlite3_vfs *pRealVfs = ((rbu_vfs*)pVfs)->pRealVfs;
  return pRealVfs->xDlSym(pRealVfs, pArg, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
static void rbuVfsDlClose(sqlite3_vfs *pVfs, void *pHandle){
  sqlite3_vfs *pRealVfs = ((rbu_vfs*)pVfs)->pRealVfs;
  pRealVfs->xDlClose(pRealVfs, pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
static int rbuVfsRandomness(sqlite3_vfs *pVfs, int nByte, char *zBufOut){
  sqlite3_vfs *pRealVfs = ((rbu_vfs*)pVfs)->pRealVfs;
  return pRealVfs->xRandomness(pRealVfs, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds
** actually slept.
*/
static int rbuVfsSleep(sqlite3_vfs *pVfs, int nMicro){
  sqlite3_vfs *pRealVfs = ((rbu_vfs*)pVfs)->pRealVfs;
  return pRealVfs->xSleep(pRealVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static int rbuVfsCurrentTime(sqlite3_vfs *pVfs, double *pTimeOut){
  sqlite3_vfs *pRealVfs = ((rbu_vfs*)pVfs)->pRealVfs;
  return pRealVfs->xCurrentTime(pRealVfs, pTimeOut);
}

/*
** No-op.
*/
static int rbuVfsGetLastError(sqlite3_vfs *pVfs, int a, char *b){
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(a);
  UNUSED_PARAMETER(b);
  return 0;
}

/*
** Deregister and destroy an RBU vfs created by an earlier call to
** sqlite3rbu_create_vfs().
*/
SQLITE_API void sqlite3rbu_destroy_vfs(const char *zName){
  sqlite3_vfs *pVfs = sqlite3_vfs_find(zName);
  if( pVfs && pVfs->xOpen==rbuVfsOpen ){
    sqlite3_mutex_free(((rbu_vfs*)pVfs)->mutex);
    sqlite3_vfs_unregister(pVfs);
    sqlite3_free(pVfs);
  }
}

/*
** Create an RBU VFS named zName that accesses the underlying file-system
** via existing VFS zParent. The new object is registered as a non-default
** VFS with SQLite before returning.
*/
SQLITE_API int sqlite3rbu_create_vfs(const char *zName, const char *zParent){

  /* Template for VFS */
  static sqlite3_vfs vfs_template = {
    1,                            /* iVersion */
    0,                            /* szOsFile */
    0,                            /* mxPathname */
    0,                            /* pNext */
    0,                            /* zName */
    0,                            /* pAppData */
    rbuVfsOpen,                   /* xOpen */
    rbuVfsDelete,                 /* xDelete */
    rbuVfsAccess,                 /* xAccess */
    rbuVfsFullPathname,           /* xFullPathname */

#ifndef SQLITE_OMIT_LOAD_EXTENSION
    rbuVfsDlOpen,                 /* xDlOpen */
    rbuVfsDlError,                /* xDlError */
    rbuVfsDlSym,                  /* xDlSym */
    rbuVfsDlClose,                /* xDlClose */
#else
    0, 0, 0, 0,
#endif

    rbuVfsRandomness,             /* xRandomness */
    rbuVfsSleep,                  /* xSleep */
    rbuVfsCurrentTime,            /* xCurrentTime */
    rbuVfsGetLastError,           /* xGetLastError */
    0,                            /* xCurrentTimeInt64 (version 2) */
    0, 0, 0                       /* Unimplemented version 3 methods */
  };

  rbu_vfs *pNew = 0;              /* Newly allocated VFS */
  int rc = SQLITE_OK;
  size_t nName;
  size_t nByte;

  nName = strlen(zName);
  nByte = sizeof(rbu_vfs) + nName + 1;
  pNew = (rbu_vfs*)sqlite3_malloc64(nByte);
  if( pNew==0 ){
    rc = SQLITE_NOMEM;
  }else{
    sqlite3_vfs *pParent;           /* Parent VFS */
    memset(pNew, 0, nByte);
    pParent = sqlite3_vfs_find(zParent);
    if( pParent==0 ){
      rc = SQLITE_NOTFOUND;
    }else{
      char *zSpace;
      memcpy(&pNew->base, &vfs_template, sizeof(sqlite3_vfs));
      pNew->base.mxPathname = pParent->mxPathname;
      pNew->base.szOsFile = sizeof(rbu_file) + pParent->szOsFile;
      pNew->pRealVfs = pParent;
      pNew->base.zName = (const char*)(zSpace = (char*)&pNew[1]);
      memcpy(zSpace, zName, nName);

      /* Allocate the mutex and register the new VFS (not as the default) */
      pNew->mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_RECURSIVE);
      if( pNew->mutex==0 ){
        rc = SQLITE_NOMEM;
      }else{
        rc = sqlite3_vfs_register(&pNew->base, 0);
      }
    }

    if( rc!=SQLITE_OK ){
      sqlite3_mutex_free(pNew->mutex);
      sqlite3_free(pNew);
    }
  }

  return rc;
}

/*
** Configure the aggregate temp file size limit for this RBU handle.
*/
SQLITE_API sqlite3_int64 sqlite3rbu_temp_size_limit(sqlite3rbu *pRbu, sqlite3_int64 n){
  if( n>=0 ){
    pRbu->szTempLimit = n;
  }
  return pRbu->szTempLimit;
}

SQLITE_API sqlite3_int64 sqlite3rbu_temp_size(sqlite3rbu *pRbu){
  return pRbu->szTemp;
}


/**************************************************************************/

#endif /* !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_RBU) */

/************** End of sqlite3rbu.c ******************************************/
/************** Begin file dbstat.c ******************************************/
/*
** 2010 July 12
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file contains an implementation of the "dbstat" virtual table.
**
** The dbstat virtual table is used to extract low-level storage
** information from an SQLite database in order to implement the
** "sqlite3_analyzer" utility.  See the ../tool/spaceanal.tcl script
** for an example implementation.
**
** Additional information is available on the "dbstat.html" page of the
** official SQLite documentation.
*/

/* #include "sqliteInt.h"   ** Requires access to internal data structures ** */
#if (defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)) \
    && !defined(SQLITE_OMIT_VIRTUALTABLE)

/*
** The pager and btree modules arrange objects in memory so that there are
** always approximately 200 bytes of addressable memory following each page
** buffer. This way small buffer overreads caused by corrupt database pages
** do not cause undefined behaviour. This module pads each page buffer
** by the following number of bytes for the same purpose.
*/
#define DBSTAT_PAGE_PADDING_BYTES 256

/*
** Page paths:
**
**   The value of the 'path' column describes the path taken from the
**   root-node of the b-tree structure to each page. The value of the
**   root-node path is '/'.
**
**   The value of the path for the left-most child page of the root of
**   a b-tree is '/000/'. (Btrees store content ordered from left to right
**   so the pages to the left have smaller keys than the pages to the right.)
**   The next to left-most child of the root page is
**   '/001', and so on, each sibling page identified by a 3-digit hex
**   value. The children of the 451st left-most sibling have paths such
**   as '/1c2/000/, '/1c2/001/' etc.
**
**   Overflow pages are specified by appending a '+' character and a
**   six-digit hexadecimal value to the path to the cell they are linked
**   from. For example, the three overflow pages in a chain linked from
**   the left-most cell of the 450th child of the root page are identified
**   by the paths:
**
**      '/1c2/000+000000'         // First page in overflow chain
**      '/1c2/000+000001'         // Second page in overflow chain
**      '/1c2/000+000002'         // Third page in overflow chain
**
**   If the paths are sorted using the BINARY collation sequence, then
**   the overflow pages associated with a cell will appear earlier in the
**   sort-order than its child page:
**
**      '/1c2/000/'               // Left-most child of 451st child of root
*/
static const char zDbstatSchema[] =
  "CREATE TABLE x("
  " name       TEXT,"          /*  0 Name of table or index */
  " path       TEXT,"          /*  1 Path to page from root (NULL for agg) */
  " pageno     INTEGER,"       /*  2 Page number (page count for aggregates) */
  " pagetype   TEXT,"          /*  3 'internal', 'leaf', 'overflow', or NULL */
  " ncell      INTEGER,"       /*  4 Cells on page (0 for overflow) */
  " payload    INTEGER,"       /*  5 Bytes of payload on this page */
  " unused     INTEGER,"       /*  6 Bytes of unused space on this page */
  " mx_payload INTEGER,"       /*  7 Largest payload size of all cells */
  " pgoffset   INTEGER,"       /*  8 Offset of page in file (NULL for agg) */
  " pgsize     INTEGER,"       /*  9 Size of the page (sum for aggregate) */
  " schema     TEXT HIDDEN,"   /* 10 Database schema being analyzed */
  " aggregate  BOOLEAN HIDDEN" /* 11 aggregate info for each table */
  ")"
;

/* Forward reference to data structured used in this module */
typedef struct StatTable StatTable;
typedef struct StatCursor StatCursor;
typedef struct StatPage StatPage;
typedef struct StatCell StatCell;

/* Size information for a single cell within a btree page */
struct StatCell {
  int nLocal;                     /* Bytes of local payload */
  u32 iChildPg;                   /* Child node (or 0 if this is a leaf) */
  int nOvfl;                      /* Entries in aOvfl[] */
  u32 *aOvfl;                     /* Array of overflow page numbers */
  int nLastOvfl;                  /* Bytes of payload on final overflow page */
  int iOvfl;                      /* Iterates through aOvfl[] */
};

/* Size information for a single btree page */
struct StatPage {
  u32 iPgno;                      /* Page number */
  u8 *aPg;                        /* Page buffer from sqlite3_malloc() */
  int iCell;                      /* Current cell */
  char *zPath;                    /* Path to this page */

  /* Variables populated by statDecodePage(): */
  u8 flags;                       /* Copy of flags byte */
  int nCell;                      /* Number of cells on page */
  int nUnused;                    /* Number of unused bytes on page */
  StatCell *aCell;                /* Array of parsed cells */
  u32 iRightChildPg;              /* Right-child page number (or 0) */
  int nMxPayload;                 /* Largest payload of any cell on the page */
};

/* The cursor for scanning the dbstat virtual table */
struct StatCursor {
  sqlite3_vtab_cursor base;       /* base class.  MUST BE FIRST! */
  sqlite3_stmt *pStmt;            /* Iterates through set of root pages */
  u8 isEof;                       /* After pStmt has returned SQLITE_DONE */
  u8 isAgg;                       /* Aggregate results for each table */
  int iDb;                        /* Schema used for this query */

  StatPage aPage[32];             /* Pages in path to current page */
  int iPage;                      /* Current entry in aPage[] */

  /* Values to return. */
  u32 iPageno;                    /* Value of 'pageno' column */
  char *zName;                    /* Value of 'name' column */
  char *zPath;                    /* Value of 'path' column */
  char *zPagetype;                /* Value of 'pagetype' column */
  int nPage;                      /* Number of pages in current btree */
  int nCell;                      /* Value of 'ncell' column */
  int nMxPayload;                 /* Value of 'mx_payload' column */
  i64 nUnused;                    /* Value of 'unused' column */
  i64 nPayload;                   /* Value of 'payload' column */
  i64 iOffset;                    /* Value of 'pgOffset' column */
  i64 szPage;                     /* Value of 'pgSize' column */
};

/* An instance of the DBSTAT virtual table */
struct StatTable {
  sqlite3_vtab base;              /* base class.  MUST BE FIRST! */
  sqlite3 *db;                    /* Database connection that owns this vtab */
  int iDb;                        /* Index of database to analyze */
};

#ifndef get2byte
# define get2byte(x)   ((x)[0]<<8 | (x)[1])
#endif

/*
** Connect to or create a new DBSTAT virtual table.
*/
static int statConnect(
  sqlite3 *db,
  void *pAux,
  int argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  StatTable *pTab = 0;
  int rc = SQLITE_OK;
  int iDb;
  (void)pAux;

  if( argc>=4 ){
    Token nm;
    sqlite3TokenInit(&nm, (char*)argv[3]);
    iDb = sqlite3FindDb(db, &nm);
    if( iDb<0 ){
      *pzErr = sqlite3_mprintf("no such database: %s", argv[3]);
      return SQLITE_ERROR;
    }
  }else{
    iDb = 0;
  }
  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  rc = sqlite3_declare_vtab(db, zDbstatSchema);
  if( rc==SQLITE_OK ){
    pTab = (StatTable *)sqlite3_malloc64(sizeof(StatTable));
    if( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  assert( rc==SQLITE_OK || pTab==0 );
  if( rc==SQLITE_OK ){
    memset(pTab, 0, sizeof(StatTable));
    pTab->db = db;
    pTab->iDb = iDb;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  return rc;
}

/*
** Disconnect from or destroy the DBSTAT virtual table.
*/
static int statDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  return SQLITE_OK;
}

/*
** Compute the best query strategy and return the result in idxNum.
**
**   idxNum-Bit        Meaning
**   ----------        ----------------------------------------------
**      0x01           There is a schema=? term in the WHERE clause
**      0x02           There is a name=? term in the WHERE clause
**      0x04           There is an aggregate=? term in the WHERE clause
**      0x08           Output should be ordered by name and path
*/
static int statBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  int i;
  int iSchema = -1;
  int iName = -1;
  int iAgg = -1;
  (void)tab;

  /* Look for a valid schema=? constraint.  If found, change the idxNum to
  ** 1 and request the value of that constraint be sent to xFilter.  And
  ** lower the cost estimate to encourage the constrained version to be
  ** used.
  */
  for(i=0; i<pIdxInfo->nConstraint; i++){
    if( pIdxInfo->aConstraint[i].op!=SQLITE_INDEX_CONSTRAINT_EQ ) continue;
    if( pIdxInfo->aConstraint[i].usable==0 ){
      /* Force DBSTAT table should always be the right-most table in a join */
      return SQLITE_CONSTRAINT;
    }
    switch( pIdxInfo->aConstraint[i].iColumn ){
      case 0: {    /* name */
        iName = i;
        break;
      }
      case 10: {   /* schema */
        iSchema = i;
        break;
      }
      case 11: {   /* aggregate */
        iAgg = i;
        break;
      }
    }
  }
  i = 0;
  if( iSchema>=0 ){
    pIdxInfo->aConstraintUsage[iSchema].argvIndex = ++i;
    pIdxInfo->aConstraintUsage[iSchema].omit = 1;
    pIdxInfo->idxNum |= 0x01;
  }
  if( iName>=0 ){
    pIdxInfo->aConstraintUsage[iName].argvIndex = ++i;
    pIdxInfo->idxNum |= 0x02;
  }
  if( iAgg>=0 ){
    pIdxInfo->aConstraintUsage[iAgg].argvIndex = ++i;
    pIdxInfo->idxNum |= 0x04;
  }
  pIdxInfo->estimatedCost = 1.0;

  /* Records are always returned in ascending order of (name, path).
  ** If this will satisfy the client, set the orderByConsumed flag so that
  ** SQLite does not do an external sort.
  */
  if( ( pIdxInfo->nOrderBy==1
     && pIdxInfo->aOrderBy[0].iColumn==0
     && pIdxInfo->aOrderBy[0].desc==0
     ) ||
      ( pIdxInfo->nOrderBy==2
     && pIdxInfo->aOrderBy[0].iColumn==0
     && pIdxInfo->aOrderBy[0].desc==0
     && pIdxInfo->aOrderBy[1].iColumn==1
     && pIdxInfo->aOrderBy[1].desc==0
     )
  ){
    pIdxInfo->orderByConsumed = 1;
    pIdxInfo->idxNum |= 0x08;
  }
  pIdxInfo->idxFlags |= SQLITE_INDEX_SCAN_HEX;

  return SQLITE_OK;
}

/*
** Open a new DBSTAT cursor.
*/
static int statOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  StatTable *pTab = (StatTable *)pVTab;
  StatCursor *pCsr;

  pCsr = (StatCursor *)sqlite3_malloc64(sizeof(StatCursor));
  if( pCsr==0 ){
    return SQLITE_NOMEM_BKPT;
  }else{
    memset(pCsr, 0, sizeof(StatCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->iDb = pTab->iDb;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  return SQLITE_OK;
}

static void statClearCells(StatPage *p){
  int i;
  if( p->aCell ){
    for(i=0; i<p->nCell; i++){
      sqlite3_free(p->aCell[i].aOvfl);
    }
    sqlite3_free(p->aCell);
  }
  p->nCell = 0;
  p->aCell = 0;
}

static void statClearPage(StatPage *p){
  u8 *aPg = p->aPg;
  statClearCells(p);
  sqlite3_free(p->zPath);
  memset(p, 0, sizeof(StatPage));
  p->aPg = aPg;
}

static void statResetCsr(StatCursor *pCsr){
  int i;
  /* In some circumstances, specifically if an OOM has occurred, the call
  ** to sqlite3_reset() may cause the pager to be reset (emptied). It is
  ** important that statClearPage() is called to free any page refs before
  ** this happens. dbsqlfuzz 9ed3e4e3816219d3509d711636c38542bf3f40b1. */
  for(i=0; i<ArraySize(pCsr->aPage); i++){
    statClearPage(&pCsr->aPage[i]);
    sqlite3_free(pCsr->aPage[i].aPg);
    pCsr->aPage[i].aPg = 0;
  }
  sqlite3_reset(pCsr->pStmt);
  pCsr->iPage = 0;
  sqlite3_free(pCsr->zPath);
  pCsr->zPath = 0;
  pCsr->isEof = 0;
}

/* Resize the space-used counters inside of the cursor */
static void statResetCounts(StatCursor *pCsr){
  pCsr->nCell = 0;
  pCsr->nMxPayload = 0;
  pCsr->nUnused = 0;
  pCsr->nPayload = 0;
  pCsr->szPage = 0;
  pCsr->nPage = 0;
}

/*
** Close a DBSTAT cursor.
*/
static int statClose(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  sqlite3_free(pCsr);
  return SQLITE_OK;
}

/*
** For a single cell on a btree page, compute the number of bytes of
** content (payload) stored on that page.  That is to say, compute the
** number of bytes of content not found on overflow pages.
*/
static int getLocalPayload(
  int nUsable,                    /* Usable bytes per page */
  u8 flags,                       /* Page flags */
  int nTotal                      /* Total record (payload) size */
){
  int nLocal;
  int nMinLocal;
  int nMaxLocal;

  if( flags==0x0D ){              /* Table leaf node */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = nUsable - 35;
  }else{                          /* Index interior and leaf nodes */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = (nUsable - 12) * 64 / 255 - 23;
  }

  nLocal = nMinLocal + (nTotal - nMinLocal) % (nUsable - 4);
  if( nLocal>nMaxLocal ) nLocal = nMinLocal;
  return nLocal;
}

/* Populate the StatPage object with information about the all
** cells found on the page currently under analysis.
*/
static int statDecodePage(Btree *pBt, StatPage *p){
  int nUnused;
  int iOff;
  int nHdr;
  int isLeaf;
  int szPage;

  u8 *aData = p->aPg;
  u8 *aHdr = &aData[p->iPgno==1 ? 100 : 0];

  p->flags = aHdr[0];
  if( p->flags==0x0A || p->flags==0x0D ){
    isLeaf = 1;
    nHdr = 8;
  }else if( p->flags==0x05 || p->flags==0x02 ){
    isLeaf = 0;
    nHdr = 12;
  }else{
    goto statPageIsCorrupt;
  }
  if( p->iPgno==1 ) nHdr += 100;
  p->nCell = get2byte(&aHdr[3]);
  p->nMxPayload = 0;
  szPage = sqlite3BtreeGetPageSize(pBt);

  nUnused = get2byte(&aHdr[5]) - nHdr - 2*p->nCell;
  nUnused += (int)aHdr[7];
  iOff = get2byte(&aHdr[1]);
  while( iOff ){
    int iNext;
    if( iOff>=szPage ) goto statPageIsCorrupt;
    nUnused += get2byte(&aData[iOff+2]);
    iNext = get2byte(&aData[iOff]);
    if( iNext<iOff+4 && iNext>0 ) goto statPageIsCorrupt;
    iOff = iNext;
  }
  p->nUnused = nUnused;
  p->iRightChildPg = isLeaf ? 0 : sqlite3Get4byte(&aHdr[8]);

  if( p->nCell ){
    int i;                        /* Used to iterate through cells */
    int nUsable;                  /* Usable bytes per page */

    sqlite3BtreeEnter(pBt);
    nUsable = szPage - sqlite3BtreeGetReserveNoMutex(pBt);
    sqlite3BtreeLeave(pBt);
    p->aCell = sqlite3_malloc64((p->nCell+1) * sizeof(StatCell));
    if( p->aCell==0 ) return SQLITE_NOMEM_BKPT;
    memset(p->aCell, 0, (p->nCell+1) * sizeof(StatCell));

    for(i=0; i<p->nCell; i++){
      StatCell *pCell = &p->aCell[i];

      iOff = get2byte(&aData[nHdr+i*2]);
      if( iOff<nHdr || iOff>=szPage ) goto statPageIsCorrupt;
      if( !isLeaf ){
        pCell->iChildPg = sqlite3Get4byte(&aData[iOff]);
        iOff += 4;
      }
      if( p->flags==0x05 ){
        /* A table interior node. nPayload==0. */
      }else{
        u32 nPayload;             /* Bytes of payload total (local+overflow) */
        int nLocal;               /* Bytes of payload stored locally */
        iOff += getVarint32(&aData[iOff], nPayload);
        if( p->flags==0x0D ){
          u64 dummy;
          iOff += sqlite3GetVarint(&aData[iOff], &dummy);
        }
        if( nPayload>(u32)p->nMxPayload ) p->nMxPayload = nPayload;
        nLocal = getLocalPayload(nUsable, p->flags, nPayload);
        if( nLocal<0 ) goto statPageIsCorrupt;
        pCell->nLocal = nLocal;
        assert( nPayload>=(u32)nLocal );
        assert( nLocal<=(nUsable-35) );
        if( nPayload>(u32)nLocal ){
          int j;
          int nOvfl = ((nPayload - nLocal) + nUsable-4 - 1) / (nUsable - 4);
          if( iOff+nLocal+4>nUsable || nPayload>0x7fffffff ){
            goto statPageIsCorrupt;
          }
          pCell->nLastOvfl = (nPayload-nLocal) - (nOvfl-1) * (nUsable-4);
          pCell->nOvfl = nOvfl;
          pCell->aOvfl = sqlite3_malloc64(sizeof(u32)*nOvfl);
          if( pCell->aOvfl==0 ) return SQLITE_NOMEM_BKPT;
          pCell->aOvfl[0] = sqlite3Get4byte(&aData[iOff+nLocal]);
          for(j=1; j<nOvfl; j++){
            int rc;
            u32 iPrev = pCell->aOvfl[j-1];
            DbPage *pPg = 0;
            rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPrev, &pPg, 0);
            if( rc!=SQLITE_OK ){
              assert( pPg==0 );
              return rc;
            }
            pCell->aOvfl[j] = sqlite3Get4byte(sqlite3PagerGetData(pPg));
            sqlite3PagerUnref(pPg);
          }
        }
      }
    }
  }

  return SQLITE_OK;

statPageIsCorrupt:
  p->flags = 0;
  statClearCells(p);
  return SQLITE_OK;
}

/*
** Populate the pCsr->iOffset and pCsr->szPage member variables. Based on
** the current value of pCsr->iPageno.
*/
static void statSizeAndOffset(StatCursor *pCsr){
  StatTable *pTab = (StatTable *)((sqlite3_vtab_cursor *)pCsr)->pVtab;
  Btree *pBt = pTab->db->aDb[pTab->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);
  sqlite3_file *fd;
  sqlite3_int64 x[2];

  /* If connected to a ZIPVFS backend, find the page size and
  ** offset from ZIPVFS.
  */
  fd = sqlite3PagerFile(pPager);
  x[0] = pCsr->iPageno;
  if( sqlite3OsFileControl(fd, 230440, &x)==SQLITE_OK ){
    pCsr->iOffset = x[0];
    pCsr->szPage += x[1];
  }else{
    /* Not ZIPVFS: The default page size and offset */
    pCsr->szPage += sqlite3BtreeGetPageSize(pBt);
    pCsr->iOffset = (i64)pCsr->szPage * (pCsr->iPageno - 1);
  }
}

/*
** Load a copy of the page data for page iPg into the buffer belonging
** to page object pPg. Allocate the buffer if necessary. Return SQLITE_OK
** if successful, or an SQLite error code otherwise.
*/
static int statGetPage(
  Btree *pBt,                     /* Load page from this b-tree */
  u32 iPg,                        /* Page number to load */
  StatPage *pPg                   /* Load page into this object */
){
  int pgsz = sqlite3BtreeGetPageSize(pBt);
  DbPage *pDbPage = 0;
  int rc;

  if( pPg->aPg==0 ){
    pPg->aPg = (u8*)sqlite3_malloc(pgsz + DBSTAT_PAGE_PADDING_BYTES);
    if( pPg->aPg==0 ){
      return SQLITE_NOMEM_BKPT;
    }
    memset(&pPg->aPg[pgsz], 0, DBSTAT_PAGE_PADDING_BYTES);
  }

  rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPg, &pDbPage, 0);
  if( rc==SQLITE_OK ){
    const u8 *a = sqlite3PagerGetData(pDbPage);
    memcpy(pPg->aPg, a, pgsz);
    sqlite3PagerUnref(pDbPage);
  }

  return rc;
}

/*
** Move a DBSTAT cursor to the next entry.  Normally, the next
** entry will be the next page, but in aggregated mode (pCsr->isAgg!=0),
** the next entry is the next btree.
*/
static int statNext(sqlite3_vtab_cursor *pCursor){
  int rc;
  int nPayload;
  char *z;
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable *)pCursor->pVtab;
  Btree *pBt = pTab->db->aDb[pCsr->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);

  sqlite3_free(pCsr->zPath);
  pCsr->zPath = 0;

statNextRestart:
  if( pCsr->iPage<0 ){
    /* Start measuring space on the next btree */
    statResetCounts(pCsr);
    rc = sqlite3_step(pCsr->pStmt);
    if( rc==SQLITE_ROW ){
      int nPage;
      u32 iRoot = (u32)sqlite3_column_int64(pCsr->pStmt, 1);
      sqlite3PagerPagecount(pPager, &nPage);
      if( nPage==0 ){
        pCsr->isEof = 1;
        return sqlite3_reset(pCsr->pStmt);
      }
      rc = statGetPage(pBt, iRoot, &pCsr->aPage[0]);
      pCsr->aPage[0].iPgno = iRoot;
      pCsr->aPage[0].iCell = 0;
      if( !pCsr->isAgg ){
        pCsr->aPage[0].zPath = z = sqlite3_mprintf("/");
        if( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      pCsr->iPage = 0;
      pCsr->nPage = 1;
    }else{
      pCsr->isEof = 1;
      return sqlite3_reset(pCsr->pStmt);
    }
  }else{
    /* Continue analyzing the btree previously started */
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    if( !pCsr->isAgg ) statResetCounts(pCsr);
    while( p->iCell<p->nCell ){
      StatCell *pCell = &p->aCell[p->iCell];
      while( pCell->iOvfl<pCell->nOvfl ){
        int nUsable, iOvfl;
        sqlite3BtreeEnter(pBt);
        nUsable = sqlite3BtreeGetPageSize(pBt) -
                        sqlite3BtreeGetReserveNoMutex(pBt);
        sqlite3BtreeLeave(pBt);
        pCsr->nPage++;
        statSizeAndOffset(pCsr);
        if( pCell->iOvfl<pCell->nOvfl-1 ){
          pCsr->nPayload += nUsable - 4;
        }else{
          pCsr->nPayload += pCell->nLastOvfl;
          pCsr->nUnused += nUsable - 4 - pCell->nLastOvfl;
        }
        iOvfl = pCell->iOvfl;
        pCell->iOvfl++;
        if( !pCsr->isAgg ){
          pCsr->zName = (char *)sqlite3_column_text(pCsr->pStmt, 0);
          pCsr->iPageno = pCell->aOvfl[iOvfl];
          pCsr->zPagetype = "overflow";
          pCsr->zPath = z = sqlite3_mprintf(
              "%s%.3x+%.6x", p->zPath, p->iCell, iOvfl
          );
          return z==0 ? SQLITE_NOMEM_BKPT : SQLITE_OK;
        }
      }
      if( p->iRightChildPg ) break;
      p->iCell++;
    }

    if( !p->iRightChildPg || p->iCell>p->nCell ){
      statClearPage(p);
      pCsr->iPage--;
      if( pCsr->isAgg && pCsr->iPage<0 ){
        /* label-statNext-done:  When computing aggregate space usage over
        ** an entire btree, this is the exit point from this function */
        return SQLITE_OK;
      }
      goto statNextRestart; /* Tail recursion */
    }
    pCsr->iPage++;
    if( pCsr->iPage>=ArraySize(pCsr->aPage) ){
      statResetCsr(pCsr);
      return SQLITE_CORRUPT_BKPT;
    }
    assert( p==&pCsr->aPage[pCsr->iPage-1] );

    if( p->iCell==p->nCell ){
      p[1].iPgno = p->iRightChildPg;
    }else{
      p[1].iPgno = p->aCell[p->iCell].iChildPg;
    }
    rc = statGetPage(pBt, p[1].iPgno, &p[1]);
    pCsr->nPage++;
    p[1].iCell = 0;
    if( !pCsr->isAgg ){
      p[1].zPath = z = sqlite3_mprintf("%s%.3x/", p->zPath, p->iCell);
      if( z==0 ) rc = SQLITE_NOMEM_BKPT;
    }
    p->iCell++;
  }


  /* Populate the StatCursor fields with the values to be returned
  ** by the xColumn() and xRowid() methods.
  */
  if( rc==SQLITE_OK ){
    int i;
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    pCsr->zName = (char *)sqlite3_column_text(pCsr->pStmt, 0);
    pCsr->iPageno = p->iPgno;

    rc = statDecodePage(pBt, p);
    if( rc==SQLITE_OK ){
      statSizeAndOffset(pCsr);

      switch( p->flags ){
        case 0x05:             /* table internal */
        case 0x02:             /* index internal */
          pCsr->zPagetype = "internal";
          break;
        case 0x0D:             /* table leaf */
        case 0x0A:             /* index leaf */
          pCsr->zPagetype = "leaf";
          break;
        default:
          pCsr->zPagetype = "corrupted";
          break;
      }
      pCsr->nCell += p->nCell;
      pCsr->nUnused += p->nUnused;
      if( p->nMxPayload>pCsr->nMxPayload ) pCsr->nMxPayload = p->nMxPayload;
      if( !pCsr->isAgg ){
        pCsr->zPath = z = sqlite3_mprintf("%s", p->zPath);
        if( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      nPayload = 0;
      for(i=0; i<p->nCell; i++){
        nPayload += p->aCell[i].nLocal;
      }
      pCsr->nPayload += nPayload;

      /* If computing aggregate space usage by btree, continue with the
      ** next page.  The loop will exit via the return at label-statNext-done
      */
      if( pCsr->isAgg ) goto statNextRestart;
    }
  }

  return rc;
}

static int statEof(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  return pCsr->isEof;
}

/* Initialize a cursor according to the query plan idxNum using the
** arguments in argv[0].  See statBestIndex() for a description of the
** meaning of the bits in idxNum.
*/
static int statFilter(
  sqlite3_vtab_cursor *pCursor,
  int idxNum, const char *idxStr,
  int argc, sqlite3_value **argv
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable*)(pCursor->pVtab);
  sqlite3_str *pSql;      /* Query of btrees to analyze */
  char *zSql;             /* String value of pSql */
  int iArg = 0;           /* Count of argv[] parameters used so far */
  int rc = SQLITE_OK;     /* Result of this operation */
  const char *zName = 0;  /* Only provide analysis of this table */
  (void)argc;
  (void)idxStr;

  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  if( idxNum & 0x01 ){
    /* schema=? constraint is present.  Get its value */
    const char *zDbase = (const char*)sqlite3_value_text(argv[iArg++]);
    pCsr->iDb = sqlite3FindDbName(pTab->db, zDbase);
    if( pCsr->iDb<0 ){
      pCsr->iDb = 0;
      pCsr->isEof = 1;
      return SQLITE_OK;
    }
  }else{
    pCsr->iDb = pTab->iDb;
  }
  if( idxNum & 0x02 ){
    /* name=? constraint is present */
    zName = (const char*)sqlite3_value_text(argv[iArg++]);
  }
  if( idxNum & 0x04 ){
    /* aggregate=? constraint is present */
    pCsr->isAgg = sqlite3_value_double(argv[iArg++])!=0.0;
  }else{
    pCsr->isAgg = 0;
  }
  pSql = sqlite3_str_new(pTab->db);
  sqlite3_str_appendf(pSql,
      "SELECT * FROM ("
        "SELECT 'sqlite_schema' AS name,1 AS rootpage,'table' AS type"
        " UNION ALL "
        "SELECT name,rootpage,type"
        " FROM \"%w\".sqlite_schema WHERE rootpage!=0)",
      pTab->db->aDb[pCsr->iDb].zDbSName);
  if( zName ){
    sqlite3_str_appendf(pSql, "WHERE name=%Q", zName);
  }
  if( idxNum & 0x08 ){
    sqlite3_str_appendf(pSql, " ORDER BY name");
  }
  zSql = sqlite3_str_finish(pSql);
  if( zSql==0 ){
    return SQLITE_NOMEM_BKPT;
  }else{
    rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
    sqlite3_free(zSql);
  }

  if( rc==SQLITE_OK ){
    pCsr->iPage = -1;
    rc = statNext(pCursor);
  }
  return rc;
}

static int statColumn(
  sqlite3_vtab_cursor *pCursor,
  sqlite3_context *ctx,
  int i
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  switch( i ){
    case 0:            /* name */
      sqlite3_result_text(ctx, pCsr->zName, -1, SQLITE_TRANSIENT);
      break;
    case 1:            /* path */
      if( !pCsr->isAgg ){
        sqlite3_result_text(ctx, pCsr->zPath, -1, SQLITE_TRANSIENT);
      }
      break;
    case 2:            /* pageno */
      if( pCsr->isAgg ){
        sqlite3_result_int64(ctx, pCsr->nPage);
      }else{
        sqlite3_result_int64(ctx, pCsr->iPageno);
      }
      break;
    case 3:            /* pagetype */
      if( !pCsr->isAgg ){
        sqlite3_result_text(ctx, pCsr->zPagetype, -1, SQLITE_STATIC);
      }
      break;
    case 4:            /* ncell */
      sqlite3_result_int64(ctx, pCsr->nCell);
      break;
    case 5:            /* payload */
      sqlite3_result_int64(ctx, pCsr->nPayload);
      break;
    case 6:            /* unused */
      sqlite3_result_int64(ctx, pCsr->nUnused);
      break;
    case 7:            /* mx_payload */
      sqlite3_result_int64(ctx, pCsr->nMxPayload);
      break;
    case 8:            /* pgoffset */
      if( !pCsr->isAgg ){
        sqlite3_result_int64(ctx, pCsr->iOffset);
      }
      break;
    case 9:            /* pgsize */
      sqlite3_result_int64(ctx, pCsr->szPage);
      break;
    case 10: {         /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      int iDb = pCsr->iDb;
      sqlite3_result_text(ctx, db->aDb[iDb].zDbSName, -1, SQLITE_STATIC);
      break;
    }
    default: {         /* aggregate */
      sqlite3_result_int(ctx, pCsr->isAgg);
      break;
    }
  }
  return SQLITE_OK;
}

static int statRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  StatCursor *pCsr = (StatCursor *)pCursor;
  *pRowid = pCsr->iPageno;
  return SQLITE_OK;
}

/*
** Invoke this routine to register the "dbstat" virtual table module
*/
SQLITE_PRIVATE int sqlite3DbstatRegister(sqlite3 *db){
  static sqlite3_module dbstat_module = {
    0,                            /* iVersion */
    statConnect,                  /* xCreate */
    statConnect,                  /* xConnect */
    statBestIndex,                /* xBestIndex */
    statDisconnect,               /* xDisconnect */
    statDisconnect,               /* xDestroy */
    statOpen,                     /* xOpen - open a cursor */
    statClose,                    /* xClose - close a cursor */
    statFilter,                   /* xFilter - configure scan constraints */
    statNext,                     /* xNext - advance a cursor */
    statEof,                      /* xEof - check for end of scan */
    statColumn,                   /* xColumn - read data */
    statRowid,                    /* xRowid - read data */
    0,                            /* xUpdate */
    0,                            /* xBegin */
    0,                            /* xSync */
    0,                            /* xCommit */
    0,                            /* xRollback */
    0,                            /* xFindMethod */
    0,                            /* xRename */
    0,                            /* xSavepoint */
    0,                            /* xRelease */
    0,                            /* xRollbackTo */
    0,                            /* xShadowName */
    0                             /* xIntegrity */
  };
  return sqlite3_create_module(db, "dbstat", &dbstat_module, 0);
}
#elif defined(SQLITE_ENABLE_DBSTAT_VTAB)
SQLITE_PRIVATE int sqlite3DbstatRegister(sqlite3 *db){ return SQLITE_OK; }
#endif /* SQLITE_ENABLE_DBSTAT_VTAB */

/************** End of dbstat.c **********************************************/
/************** Begin file dbpage.c ******************************************/
/*
** 2017-10-11
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file contains an implementation of the "sqlite_dbpage" virtual table.
**
** The sqlite_dbpage virtual table is used to read or write whole raw
** pages of the database file.  The pager interface is used so that
** uncommitted changes and changes recorded in the WAL file are correctly
** retrieved.
**
** Usage example:
**
**    SELECT data FROM sqlite_dbpage('aux1') WHERE pgno=123;
**
** This is an eponymous virtual table so it does not need to be created before
** use.  The optional argument to the sqlite_dbpage() table name is the
** schema for the database file that is to be read.  The default schema is
** "main".
**
** The data field of sqlite_dbpage table can be updated.  The new
** value must be a BLOB which is the correct page size, otherwise the
** update fails.  INSERT operations also work, and operate as if they
** where REPLACE.  The size of the database can be extended by INSERT-ing
** new pages on the end.
**
** Rows may not be deleted.  However, doing an INSERT to page number N
** with NULL page data causes the N-th page and all subsequent pages to be
** deleted and the database to be truncated.
*/

/* #include "sqliteInt.h"   ** Requires access to internal data structures ** */
#if (defined(SQLITE_ENABLE_DBPAGE_VTAB) || defined(SQLITE_TEST)) \
    && !defined(SQLITE_OMIT_VIRTUALTABLE)

typedef struct DbpageTable DbpageTable;
typedef struct DbpageCursor DbpageCursor;

struct DbpageCursor {
  sqlite3_vtab_cursor base;       /* Base class.  Must be first */
  Pgno pgno;                      /* Current page number */
  Pgno mxPgno;                    /* Last page to visit on this scan */
  Pager *pPager;                  /* Pager being read/written */
  DbPage *pPage1;                 /* Page 1 of the database */
  int iDb;                        /* Index of database to analyze */
  int szPage;                     /* Size of each page in bytes */
};

struct DbpageTable {
  sqlite3_vtab base;              /* Base class.  Must be first */
  sqlite3 *db;                    /* The database */
  int iDbTrunc;                   /* Database to truncate */
  Pgno pgnoTrunc;                 /* Size to truncate to */
};

/* Columns */
#define DBPAGE_COLUMN_PGNO    0
#define DBPAGE_COLUMN_DATA    1
#define DBPAGE_COLUMN_SCHEMA  2


/*
** Connect to or create a dbpagevfs virtual table.
*/
static int dbpageConnect(
  sqlite3 *db,
  void *pAux,
  int argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  DbpageTable *pTab = 0;
  int rc = SQLITE_OK;
  (void)pAux;
  (void)argc;
  (void)argv;
  (void)pzErr;

  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  sqlite3_vtab_config(db, SQLITE_VTAB_USES_ALL_SCHEMAS);
  rc = sqlite3_declare_vtab(db,
          "CREATE TABLE x(pgno INTEGER PRIMARY KEY, data BLOB, schema HIDDEN)");
  if( rc==SQLITE_OK ){
    pTab = (DbpageTable *)sqlite3_malloc64(sizeof(DbpageTable));
    if( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  assert( rc==SQLITE_OK || pTab==0 );
  if( rc==SQLITE_OK ){
    memset(pTab, 0, sizeof(DbpageTable));
    pTab->db = db;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  return rc;
}

/*
** Disconnect from or destroy a dbpagevfs virtual table.
*/
static int dbpageDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  return SQLITE_OK;
}

/*
** idxNum:
**
**     0     schema=main, full table scan
**     1     schema=main, pgno=?1
**     2     schema=?1, full table scan
**     3     schema=?1, pgno=?2
*/
static int dbpageBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  int i;
  int iPlan = 0;
  (void)tab;

  /* If there is a schema= constraint, it must be honored.  Report a
  ** ridiculously large estimated cost if the schema= constraint is
  ** unavailable
  */
  for(i=0; i<pIdxInfo->nConstraint; i++){
    struct sqlite3_index_constraint *p = &pIdxInfo->aConstraint[i];
    if( p->iColumn!=DBPAGE_COLUMN_SCHEMA ) continue;
    if( p->op!=SQLITE_INDEX_CONSTRAINT_EQ ) continue;
    if( !p->usable ){
      /* No solution. */
      return SQLITE_CONSTRAINT;
    }
    iPlan = 2;
    pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    break;
  }

  /* If we reach this point, it means that either there is no schema=
  ** constraint (in which case we use the "main" schema) or else the
  ** schema constraint was accepted.  Lower the estimated cost accordingly
  */
  pIdxInfo->estimatedCost = 1.0e6;

  /* Check for constraints against pgno */
  for(i=0; i<pIdxInfo->nConstraint; i++){
    struct sqlite3_index_constraint *p = &pIdxInfo->aConstraint[i];
    if( p->usable && p->iColumn<=0 && p->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      pIdxInfo->estimatedRows = 1;
      pIdxInfo->idxFlags = SQLITE_INDEX_SCAN_UNIQUE;
      pIdxInfo->estimatedCost = 1.0;
      pIdxInfo->aConstraintUsage[i].argvIndex = iPlan ? 2 : 1;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      iPlan |= 1;
      break;
    }
  }
  pIdxInfo->idxNum = iPlan;

  if( pIdxInfo->nOrderBy>=1
   && pIdxInfo->aOrderBy[0].iColumn<=0
   && pIdxInfo->aOrderBy[0].desc==0
  ){
    pIdxInfo->orderByConsumed = 1;
  }
  return SQLITE_OK;
}

/*
** Open a new dbpagevfs cursor.
*/
static int dbpageOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  DbpageCursor *pCsr;

  pCsr = (DbpageCursor *)sqlite3_malloc64(sizeof(DbpageCursor));
  if( pCsr==0 ){
    return SQLITE_NOMEM_BKPT;
  }else{
    memset(pCsr, 0, sizeof(DbpageCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->pgno = 0;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  return SQLITE_OK;
}

/*
** Close a dbpagevfs cursor.
*/
static int dbpageClose(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  if( pCsr->pPage1 ) sqlite3PagerUnrefPageOne(pCsr->pPage1);
  sqlite3_free(pCsr);
  return SQLITE_OK;
}

/*
** Move a dbpagevfs cursor to the next entry in the file.
*/
static int dbpageNext(sqlite3_vtab_cursor *pCursor){
  int rc = SQLITE_OK;
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  pCsr->pgno++;
  return rc;
}

static int dbpageEof(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  return pCsr->pgno > pCsr->mxPgno;
}

/*
** idxNum:
**
**     0     schema=main, full table scan
**     1     schema=main, pgno=?1
**     2     schema=?1, full table scan
**     3     schema=?1, pgno=?2
**
** idxStr is not used
*/
static int dbpageFilter(
  sqlite3_vtab_cursor *pCursor,
  int idxNum, const char *idxStr,
  int argc, sqlite3_value **argv
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  DbpageTable *pTab = (DbpageTable *)pCursor->pVtab;
  int rc;
  sqlite3 *db = pTab->db;
  Btree *pBt;

  UNUSED_PARAMETER(idxStr);
  UNUSED_PARAMETER(argc);

  /* Default setting is no rows of result */
  pCsr->pgno = 1;
  pCsr->mxPgno = 0;

  if( idxNum & 2 ){
    const char *zSchema;
    assert( argc>=1 );
    zSchema = (const char*)sqlite3_value_text(argv[0]);
    pCsr->iDb = sqlite3FindDbName(db, zSchema);
    if( pCsr->iDb<0 ) return SQLITE_OK;
  }else{
    pCsr->iDb = 0;
  }
  pBt = db->aDb[pCsr->iDb].pBt;
  if( NEVER(pBt==0) ) return SQLITE_OK;
  pCsr->pPager = sqlite3BtreePager(pBt);
  pCsr->szPage = sqlite3BtreeGetPageSize(pBt);
  pCsr->mxPgno = sqlite3BtreeLastPage(pBt);
  if( idxNum & 1 ){
    assert( argc>(idxNum>>1) );
    pCsr->pgno = sqlite3_value_int(argv[idxNum>>1]);
    if( pCsr->pgno<1 || pCsr->pgno>pCsr->mxPgno ){
      pCsr->pgno = 1;
      pCsr->mxPgno = 0;
    }else{
      pCsr->mxPgno = pCsr->pgno;
    }
  }else{
    assert( pCsr->pgno==1 );
  }
  if( pCsr->pPage1 ) sqlite3PagerUnrefPageOne(pCsr->pPage1);
  rc = sqlite3PagerGet(pCsr->pPager, 1, &pCsr->pPage1, 0);
  return rc;
}

static int dbpageColumn(
  sqlite3_vtab_cursor *pCursor,
  sqlite3_context *ctx,
  int i
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  int rc = SQLITE_OK;
  switch( i ){
    case 0: {           /* pgno */
      sqlite3_result_int64(ctx, (sqlite3_int64)pCsr->pgno);
      break;
    }
    case 1: {           /* data */
      DbPage *pDbPage = 0;
      if( pCsr->pgno==(Pgno)((PENDING_BYTE/pCsr->szPage)+1) ){
        /* The pending byte page. Assume it is zeroed out. Attempting to
        ** request this page from the page is an SQLITE_CORRUPT error. */
        sqlite3_result_zeroblob(ctx, pCsr->szPage);
      }else{
        rc = sqlite3PagerGet(pCsr->pPager, pCsr->pgno, (DbPage**)&pDbPage, 0);
        if( rc==SQLITE_OK ){
          sqlite3_result_blob(ctx, sqlite3PagerGetData(pDbPage), pCsr->szPage,
              SQLITE_TRANSIENT);
        }
        sqlite3PagerUnref(pDbPage);
      }
      break;
    }
    default: {          /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      sqlite3_result_text(ctx, db->aDb[pCsr->iDb].zDbSName, -1, SQLITE_STATIC);
      break;
    }
  }
  return rc;
}

static int dbpageRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  *pRowid = pCsr->pgno;
  return SQLITE_OK;
}

/*
** Open write transactions. Since we do not know in advance which database
** files will be written by the sqlite_dbpage virtual table, start a write
** transaction on them all.
**
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
static int dbpageBeginTrans(DbpageTable *pTab){
  sqlite3 *db = pTab->db;
  int rc = SQLITE_OK;
  int i;
  for(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    if( pBt ) rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  }
  return rc;
}

static int dbpageUpdate(
  sqlite3_vtab *pVtab,
  int argc,
  sqlite3_value **argv,
  sqlite_int64 *pRowid
){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  Pgno pgno;
  DbPage *pDbPage = 0;
  int rc = SQLITE_OK;
  char *zErr = 0;
  int iDb;
  Btree *pBt;
  Pager *pPager;
  int szPage;
  int isInsert;

  (void)pRowid;
  if( pTab->db->flags & SQLITE_Defensive ){
    zErr = "read-only";
    goto update_fail;
  }
  if( argc==1 ){
    zErr = "cannot delete";
    goto update_fail;
  }
  if( sqlite3_value_type(argv[0])==SQLITE_NULL ){
    pgno = (Pgno)sqlite3_value_int64(argv[2]);
    isInsert = 1;
  }else{
    pgno = (Pgno)sqlite3_value_int64(argv[0]);
    if( (Pgno)sqlite3_value_int(argv[1])!=pgno ){
      zErr = "cannot insert";
      goto update_fail;
    }
    isInsert = 0;
  }
  if( sqlite3_value_type(argv[4])==SQLITE_NULL ){
    iDb = 0;
  }else{
    const char *zSchema = (const char*)sqlite3_value_text(argv[4]);
    iDb = sqlite3FindDbName(pTab->db, zSchema);
    if( iDb<0 ){
      zErr = "no such schema";
      goto update_fail;
    }
  }
  pBt = pTab->db->aDb[iDb].pBt;
  if( pgno<1 || NEVER(pBt==0) ){
    zErr = "bad page number";
    goto update_fail;
  }
  szPage = sqlite3BtreeGetPageSize(pBt);
  if( sqlite3_value_type(argv[3])!=SQLITE_BLOB
   || sqlite3_value_bytes(argv[3])!=szPage
  ){
    if( sqlite3_value_type(argv[3])==SQLITE_NULL && isInsert && pgno>1 ){
      /* "INSERT INTO dbpage($PGNO,NULL)" causes page number $PGNO and
      ** all subsequent pages to be deleted. */
      pTab->iDbTrunc = iDb;
      pTab->pgnoTrunc = pgno-1;
      pgno = 1;
    }else{
      zErr = "bad page value";
      goto update_fail;
    }
  }

  if( dbpageBeginTrans(pTab)!=SQLITE_OK ){
    zErr = "failed to open transaction";
    goto update_fail;
  }

  pPager = sqlite3BtreePager(pBt);
  rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pDbPage, 0);
  if( rc==SQLITE_OK ){
    const void *pData = sqlite3_value_blob(argv[3]);
    if( (rc = sqlite3PagerWrite(pDbPage))==SQLITE_OK && pData ){
      unsigned char *aPage = sqlite3PagerGetData(pDbPage);
      memcpy(aPage, pData, szPage);
      pTab->pgnoTrunc = 0;
    }
  }
  if( rc!=SQLITE_OK ){
    pTab->pgnoTrunc = 0;
  }
  sqlite3PagerUnref(pDbPage);
  return rc;

update_fail:
  pTab->pgnoTrunc = 0;
  sqlite3_free(pVtab->zErrMsg);
  pVtab->zErrMsg = sqlite3_mprintf("%s", zErr);
  return SQLITE_ERROR;
}

static int dbpageBegin(sqlite3_vtab *pVtab){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  pTab->pgnoTrunc = 0;
  return SQLITE_OK;
}

/* Invoke sqlite3PagerTruncate() as necessary, just prior to COMMIT
*/
static int dbpageSync(sqlite3_vtab *pVtab){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  if( pTab->pgnoTrunc>0 ){
    Btree *pBt = pTab->db->aDb[pTab->iDbTrunc].pBt;
    Pager *pPager = sqlite3BtreePager(pBt);
    sqlite3BtreeEnter(pBt);
    if( pTab->pgnoTrunc<sqlite3BtreeLastPage(pBt) ){
      sqlite3PagerTruncateImage(pPager, pTab->pgnoTrunc);
    }
    sqlite3BtreeLeave(pBt);
  }
  pTab->pgnoTrunc = 0;
  return SQLITE_OK;
}

/* Cancel any pending truncate.
*/
static int dbpageRollbackTo(sqlite3_vtab *pVtab, int notUsed1){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  pTab->pgnoTrunc = 0;
  (void)notUsed1;
  return SQLITE_OK;
}

/*
** Invoke this routine to register the "dbpage" virtual table module
*/
SQLITE_PRIVATE int sqlite3DbpageRegister(sqlite3 *db){
  static sqlite3_module dbpage_module = {
    2,                            /* iVersion */
    dbpageConnect,                /* xCreate */
    dbpageConnect,                /* xConnect */
    dbpageBestIndex,              /* xBestIndex */
    dbpageDisconnect,             /* xDisconnect */
    dbpageDisconnect,             /* xDestroy */
    dbpageOpen,                   /* xOpen - open a cursor */
    dbpageClose,                  /* xClose - close a cursor */
    dbpageFilter,                 /* xFilter - configure scan constraints */
    dbpageNext,                   /* xNext - advance a cursor */
    dbpageEof,                    /* xEof - check for end of scan */
    dbpageColumn,                 /* xColumn - read data */
    dbpageRowid,                  /* xRowid - read data */
    dbpageUpdate,                 /* xUpdate */
    dbpageBegin,                  /* xBegin */
    dbpageSync,                   /* xSync */
    0,                            /* xCommit */
    0,                            /* xRollback */
    0,                            /* xFindMethod */
    0,                            /* xRename */
    0,                            /* xSavepoint */
    0,                            /* xRelease */
    dbpageRollbackTo,             /* xRollbackTo */
    0,                            /* xShadowName */
    0                             /* xIntegrity */
  };
  return sqlite3_create_module(db, "sqlite_dbpage", &dbpage_module, 0);
}
#elif defined(SQLITE_ENABLE_DBPAGE_VTAB)
SQLITE_PRIVATE int sqlite3DbpageRegister(sqlite3 *db){ return SQLITE_OK; }
#endif /* SQLITE_ENABLE_DBSTAT_VTAB */

/************** End of dbpage.c **********************************************/
/************** Begin file carray.c ******************************************/
/*
** 2016-06-29
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file implements a table-valued-function that
** returns the values in a C-language array.
** Examples:
**
**      SELECT * FROM carray($ptr,5)
**
** The query above returns 5 integers contained in a C-language array
** at the address $ptr.  $ptr is a pointer to the array of integers.
** The pointer value must be assigned to $ptr using the
** sqlite3_bind_pointer() interface with a pointer type of "carray".
** For example:
**
**    static int aX[] = { 53, 9, 17, 2231, 4, 99 };
**    int i = sqlite3_bind_parameter_index(pStmt, "$ptr");
**    sqlite3_bind_pointer(pStmt, i, aX, "carray", 0);
**
** There is an optional third parameter to determine the datatype of
** the C-language array.  Allowed values of the third parameter are
** 'int32', 'int64', 'double', 'char*', 'struct iovec'.  Example:
**
**      SELECT * FROM carray($ptr,10,'char*');
**
** The default value of the third parameter is 'int32'.
**
** HOW IT WORKS
**
** The carray "function" is really a virtual table with the
** following schema:
**
**     CREATE TABLE carray(
**       value,
**       pointer HIDDEN,
**       count HIDDEN,
**       ctype TEXT HIDDEN
**     );
**
** If the hidden columns "pointer" and "count" are unconstrained, then
** the virtual table has no rows.  Otherwise, the virtual table interprets
** the integer value of "pointer" as a pointer to the array and "count"
** as the number of elements in the array.  The virtual table steps through
** the array, element by element.
*/
#if !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY)
/* #include "sqliteInt.h" */
#if defined(_WIN32) || defined(__RTP__) || defined(_WRS_KERNEL)
  struct iovec {
    void *iov_base;
    size_t iov_len;
  };
#else
# include <sys/uio.h>
#endif

/*
** Names of allowed datatypes
*/
static const char *azCarrayType[] = {
  "int32", "int64", "double", "char*", "struct iovec"
};

/*
** Structure used to hold the sqlite3_carray_bind() information
*/
typedef struct carray_bind carray_bind;
struct carray_bind {
  void *aData;                /* The data */
  int nData;                  /* Number of elements */
  int mFlags;                 /* Control flags */
  void (*xDel)(void*);        /* Destructor for aData */
  void *pDel;                 /* Alternative argument to xDel() */
};


/* carray_cursor is a subclass of sqlite3_vtab_cursor which will
** serve as the underlying representation of a cursor that scans
** over rows of the result
*/
typedef struct carray_cursor carray_cursor;
struct carray_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_int64 iRowid;      /* The rowid */
  void *pPtr;                /* Pointer to the array of values */
  sqlite3_int64 iCnt;        /* Number of integers in the array */
  unsigned char eType;       /* One of the CARRAY_type values */
};

/*
** The carrayConnect() method is invoked to create a new
** carray_vtab that describes the carray virtual table.
**
** Think of this routine as the constructor for carray_vtab objects.
**
** All this routine needs to do is:
**
**    (1) Allocate the carray_vtab object and initialize all fields.
**
**    (2) Tell SQLite (via the sqlite3_declare_vtab() interface) what the
**        result set of queries against carray will look like.
*/
static int carrayConnect(
  sqlite3 *db,
  void *pAux,
  int argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  sqlite3_vtab *pNew;
  int rc;

/* Column numbers */
#define CARRAY_COLUMN_VALUE   0
#define CARRAY_COLUMN_POINTER 1
#define CARRAY_COLUMN_COUNT   2
#define CARRAY_COLUMN_CTYPE   3

  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(value,pointer hidden,count hidden,ctype hidden)");
  if( rc==SQLITE_OK ){
    pNew = *ppVtab = sqlite3_malloc( sizeof(*pNew) );
    if( pNew==0 ) return SQLITE_NOMEM;
    memset(pNew, 0, sizeof(*pNew));
  }
  return rc;
}

/*
** This method is the destructor for carray_cursor objects.
*/
static int carrayDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  return SQLITE_OK;
}

/*
** Constructor for a new carray_cursor object.
*/
static int carrayOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  carray_cursor *pCur;
  pCur = sqlite3_malloc( sizeof(*pCur) );
  if( pCur==0 ) return SQLITE_NOMEM;
  memset(pCur, 0, sizeof(*pCur));
  *ppCursor = &pCur->base;
  return SQLITE_OK;
}

/*
** Destructor for a carray_cursor.
*/
static int carrayClose(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur);
  return SQLITE_OK;
}


/*
** Advance a carray_cursor to its next row of output.
*/
static int carrayNext(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  pCur->iRowid++;
  return SQLITE_OK;
}

/*
** Return values of columns for the row at which the carray_cursor
** is currently pointing.
*/
static int carrayColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  int i                       /* Which column to return */
){
  carray_cursor *pCur = (carray_cursor*)cur;
  sqlite3_int64 x = 0;
  switch( i ){
    case CARRAY_COLUMN_POINTER:   return SQLITE_OK;
    case CARRAY_COLUMN_COUNT:     x = pCur->iCnt;   break;
    case CARRAY_COLUMN_CTYPE: {
      sqlite3_result_text(ctx, azCarrayType[pCur->eType], -1, SQLITE_STATIC);
      return SQLITE_OK;
    }
    default: {
      switch( pCur->eType ){
        case CARRAY_INT32: {
          int *p = (int*)pCur->pPtr;
          sqlite3_result_int(ctx, p[pCur->iRowid-1]);
          return SQLITE_OK;
        }
        case CARRAY_INT64: {
          sqlite3_int64 *p = (sqlite3_int64*)pCur->pPtr;
          sqlite3_result_int64(ctx, p[pCur->iRowid-1]);
          return SQLITE_OK;
        }
        case CARRAY_DOUBLE: {
          double *p = (double*)pCur->pPtr;
          sqlite3_result_double(ctx, p[pCur->iRowid-1]);
          return SQLITE_OK;
        }
        case CARRAY_TEXT: {
          const char **p = (const char**)pCur->pPtr;
          sqlite3_result_text(ctx, p[pCur->iRowid-1], -1, SQLITE_TRANSIENT);
          return SQLITE_OK;
        }
        default: {
          const struct iovec *p = (struct iovec*)pCur->pPtr;
          assert( pCur->eType==CARRAY_BLOB );
          sqlite3_result_blob(ctx, p[pCur->iRowid-1].iov_base,
                              (int)p[pCur->iRowid-1].iov_len, SQLITE_TRANSIENT);
          return SQLITE_OK;
        }
      }
    }
  }
  sqlite3_result_int64(ctx, x);
  return SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
static int carrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  carray_cursor *pCur = (carray_cursor*)cur;
  *pRowid = pCur->iRowid;
  return SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
static int carrayEof(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  return pCur->iRowid>pCur->iCnt;
}

/*
** This method is called to "rewind" the carray_cursor object back
** to the first row of output.
*/
static int carrayFilter(
  sqlite3_vtab_cursor *pVtabCursor,
  int idxNum, const char *idxStr,
  int argc, sqlite3_value **argv
){
  carray_cursor *pCur = (carray_cursor *)pVtabCursor;
  pCur->pPtr = 0;
  pCur->iCnt = 0;
  switch( idxNum ){
    case 1: {
      carray_bind *pBind = sqlite3_value_pointer(argv[0], "carray-bind");
      if( pBind==0 ) break;
      pCur->pPtr = pBind->aData;
      pCur->iCnt = pBind->nData;
      pCur->eType = pBind->mFlags & 0x07;
      break;
    }
    case 2:
    case 3: {
      pCur->pPtr = sqlite3_value_pointer(argv[0], "carray");
      pCur->iCnt = pCur->pPtr ? sqlite3_value_int64(argv[1]) : 0;
      if( idxNum<3 ){
        pCur->eType = CARRAY_INT32;
      }else{
        unsigned char i;
        const char *zType = (const char*)sqlite3_value_text(argv[2]);
        for(i=0; i<sizeof(azCarrayType)/sizeof(azCarrayType[0]); i++){
          if( sqlite3_stricmp(zType, azCarrayType[i])==0 ) break;
        }
        if( i>=sizeof(azCarrayType)/sizeof(azCarrayType[0]) ){
          pVtabCursor->pVtab->zErrMsg = sqlite3_mprintf(
            "unknown datatype: %Q", zType);
          return SQLITE_ERROR;
        }else{
          pCur->eType = i;
        }
      }
      break;
    }
  }
  pCur->iRowid = 1;
  return SQLITE_OK;
}

/*
** SQLite will invoke this method one or more times while planning a query
** that uses the carray virtual table.  This routine needs to create
** a query plan for each invocation and compute an estimated cost for that
** plan.
**
** In this implementation idxNum is used to represent the
** query plan.  idxStr is unused.
**
** idxNum is:
**
**    1    If only the pointer= constraint exists.  In this case, the
**         parameter must be bound using sqlite3_carray_bind().
**
**    2    if the pointer= and count= constraints exist.
**
**    3    if the ctype= constraint also exists.
**
** idxNum is 0 otherwise and carray becomes an empty table.
*/
static int carrayBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  int i;                 /* Loop over constraints */
  int ptrIdx = -1;       /* Index of the pointer= constraint, or -1 if none */
  int cntIdx = -1;       /* Index of the count= constraint, or -1 if none */
  int ctypeIdx = -1;     /* Index of the ctype= constraint, or -1 if none */
  unsigned seen = 0;     /* Bitmask of == constrainted columns */

  const struct sqlite3_index_constraint *pConstraint;
  pConstraint = pIdxInfo->aConstraint;
  for(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    if( pConstraint->op!=SQLITE_INDEX_CONSTRAINT_EQ ) continue;
    if( pConstraint->iColumn>=0 ) seen |= 1 << pConstraint->iColumn;
    if( pConstraint->usable==0 ) continue;
    switch( pConstraint->iColumn ){
      case CARRAY_COLUMN_POINTER:
        ptrIdx = i;
        break;
      case CARRAY_COLUMN_COUNT:
        cntIdx = i;
        break;
      case CARRAY_COLUMN_CTYPE:
        ctypeIdx = i;
        break;
    }
  }
  if( ptrIdx>=0 ){
    pIdxInfo->aConstraintUsage[ptrIdx].argvIndex = 1;
    pIdxInfo->aConstraintUsage[ptrIdx].omit = 1;
    pIdxInfo->estimatedCost = (double)1;
    pIdxInfo->estimatedRows = 100;
    pIdxInfo->idxNum = 1;
    if( cntIdx>=0 ){
      pIdxInfo->aConstraintUsage[cntIdx].argvIndex = 2;
      pIdxInfo->aConstraintUsage[cntIdx].omit = 1;
      pIdxInfo->idxNum = 2;
      if( ctypeIdx>=0 ){
        pIdxInfo->aConstraintUsage[ctypeIdx].argvIndex = 3;
        pIdxInfo->aConstraintUsage[ctypeIdx].omit = 1;
        pIdxInfo->idxNum = 3;
      }else if( seen & (1<<CARRAY_COLUMN_CTYPE) ){
        /* In a three-argument carray(), we need to know the value of all
        ** three arguments */
        return SQLITE_CONSTRAINT;
      }
    }else if( seen & (1<<CARRAY_COLUMN_COUNT) ){
      /* In a two-argument carray(), we need to know the value of both
      ** arguments */
      return SQLITE_CONSTRAINT;
    }
  }else{
    pIdxInfo->estimatedCost = (double)2147483647;
    pIdxInfo->estimatedRows = 2147483647;
    pIdxInfo->idxNum = 0;
  }
  return SQLITE_OK;
}

/*
** This following structure defines all the methods for the
** carray virtual table.
*/
static sqlite3_module carrayModule = {
  0,                         /* iVersion */
  0,                         /* xCreate */
  carrayConnect,             /* xConnect */
  carrayBestIndex,           /* xBestIndex */
  carrayDisconnect,          /* xDisconnect */
  0,                         /* xDestroy */
  carrayOpen,                /* xOpen - open a cursor */
  carrayClose,               /* xClose - close a cursor */
  carrayFilter,              /* xFilter - configure scan constraints */
  carrayNext,                /* xNext - advance a cursor */
  carrayEof,                 /* xEof - check for end of scan */
  carrayColumn,              /* xColumn - read data */
  carrayRowid,               /* xRowid - read data */
  0,                         /* xUpdate */
  0,                         /* xBegin */
  0,                         /* xSync */
  0,                         /* xCommit */
  0,                         /* xRollback */
  0,                         /* xFindMethod */
  0,                         /* xRename */
  0,                         /* xSavepoint */
  0,                         /* xRelease */
  0,                         /* xRollbackTo */
  0,                         /* xShadow */
  0                          /* xIntegrity */
};

/*
** Destructor for the carray_bind object
*/
static void carrayBindDel(void *pPtr){
  carray_bind *p = (carray_bind*)pPtr;
  if( p->xDel!=SQLITE_STATIC ){
    p->xDel(p->pDel);
  }
  sqlite3_free(p);
}

/*
** Invoke this interface in order to bind to the single-argument
** version of CARRAY().
**
**    pStmt        The prepared statement to which to bind
**    idx          The index of the parameter of pStmt to which to bind
**    aData        The data to be bound
**    nData        The number of elements in aData
**    mFlags       One of SQLITE_CARRAY_xxxx indicating datatype of aData
**    xDestroy     Destructor for pDestroy or aData if pDestroy==NULL.
**    pDestroy     Invoke xDestroy on this pointer if not NULL
**
** The destructor is called pDestroy if pDestroy!=NULL, or against
** aData if pDestroy==NULL.
*/
SQLITE_API int sqlite3_carray_bind_v2(
  sqlite3_stmt *pStmt,
  int idx,
  void *aData,
  int nData,
  int mFlags,
  void (*xDestroy)(void*),
  void *pDestroy
){
  carray_bind *pNew = 0;
  int i;
  int rc = SQLITE_OK;

  /* Ensure that the mFlags value is acceptable. */
  assert( CARRAY_INT32==0 && CARRAY_INT64==1 && CARRAY_DOUBLE==2 );
  assert( CARRAY_TEXT==3 && CARRAY_BLOB==4 );
  if( mFlags<CARRAY_INT32 || mFlags>CARRAY_BLOB ){
    rc = SQLITE_ERROR;
    goto carray_bind_error;
  }

  pNew = sqlite3_malloc64(sizeof(*pNew));
  if( pNew==0 ){
    rc = SQLITE_NOMEM;
    goto carray_bind_error;
  }

  pNew->nData = nData;
  pNew->mFlags = mFlags;
  if( xDestroy==SQLITE_TRANSIENT ){
    sqlite3_int64 sz = nData;
    switch( mFlags ){
      case CARRAY_INT32:   sz *= 4;                     break;
      case CARRAY_INT64:   sz *= 8;                     break;
      case CARRAY_DOUBLE:  sz *= 8;                     break;
      case CARRAY_TEXT:    sz *= sizeof(char*);         break;
      default:             sz *= sizeof(struct iovec);  break;
    }
    if( mFlags==CARRAY_TEXT ){
      for(i=0; i<nData; i++){
        const char *z = ((char**)aData)[i];
        if( z ) sz += strlen(z) + 1;
      }
    }else if( mFlags==CARRAY_BLOB ){
      for(i=0; i<nData; i++){
        sz += ((struct iovec*)aData)[i].iov_len;
      }
    }

    pNew->aData = sqlite3_malloc64( sz );
    if( pNew->aData==0 ){
      rc = SQLITE_NOMEM;
      goto carray_bind_error;
    }

    if( mFlags==CARRAY_TEXT ){
      char **az = (char**)pNew->aData;
      char *z = (char*)&az[nData];
      for(i=0; i<nData; i++){
        const char *zData = ((char**)aData)[i];
        sqlite3_int64 n;
        if( zData==0 ){
          az[i] = 0;
          continue;
        }
        az[i] = z;
        n = strlen(zData);
        memcpy(z, zData, n+1);
        z += n+1;
      }
    }else if( mFlags==CARRAY_BLOB ){
      struct iovec *p = (struct iovec*)pNew->aData;
      unsigned char *z = (unsigned char*)&p[nData];
      for(i=0; i<nData; i++){
        size_t n = ((struct iovec*)aData)[i].iov_len;
        p[i].iov_len = n;
        p[i].iov_base = z;
        z += n;
        memcpy(p[i].iov_base, ((struct iovec*)aData)[i].iov_base, n);
      }
    }else{
      memcpy(pNew->aData, aData, sz);
    }
    pNew->xDel = sqlite3_free;
    pNew->pDel = pNew->aData;
  }else{
    pNew->aData = aData;
    pNew->xDel = xDestroy;
    pNew->pDel = pDestroy;
  }
  return sqlite3_bind_pointer(pStmt, idx, pNew, "carray-bind", carrayBindDel);

 carray_bind_error:
  if( xDestroy!=SQLITE_STATIC && xDestroy!=SQLITE_TRANSIENT ){
    xDestroy(pDestroy);
  }
  sqlite3_free(pNew);
  return rc;
}

/*
** Invoke this interface in order to bind to the single-argument
** version of CARRAY().  Same as sqlite3_carray_bind_v2() with the
** pDestroy parameter set to NULL.
*/
SQLITE_API int sqlite3_carray_bind(
  sqlite3_stmt *pStmt,
  int idx,
  void *aData,
  int nData,
  int mFlags,
  void (*xDestroy)(void*)
){
  return sqlite3_carray_bind_v2(pStmt,idx,aData,nData,mFlags,xDestroy,aData);
}

/*
** Invoke this routine to register the carray() function.
*/
SQLITE_PRIVATE Module *sqlite3CarrayRegister(sqlite3 *db){
  return sqlite3VtabCreateModule(db, "carray", &carrayModule, 0, 0);
}

#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY) */

/************** End of carray.c **********************************************/
/************** Begin file sqlite3session.c **********************************/

#if defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
/* #include "sqlite3session.h" */
/* #include <assert.h> */
/* #include <string.h> */

#ifndef SQLITE_AMALGAMATION
/* # include "sqliteInt.h" */
/* # include "vdbeInt.h" */
#endif

typedef struct SessionTable SessionTable;
typedef struct SessionChange SessionChange;
typedef struct SessionBuffer SessionBuffer;
typedef struct SessionInput SessionInput;

/*
** Minimum chunk size used by streaming versions of functions.
*/
#ifndef SESSIONS_STRM_CHUNK_SIZE
# ifdef SQLITE_TEST
#   define SESSIONS_STRM_CHUNK_SIZE 64
# else
#   define SESSIONS_STRM_CHUNK_SIZE 1024
# endif
#endif

#define SESSIONS_ROWID "_rowid_"

static int sessions_strm_chunk_size = SESSIONS_STRM_CHUNK_SIZE;

typedef struct SessionHook SessionHook;
struct SessionHook {
  void *pCtx;
  int (*xOld)(void*,int,sqlite3_value**);
  int (*xNew)(void*,int,sqlite3_value**);
  int (*xCount)(void*);
  int (*xDepth)(void*);
};

/*
** Session handle structure.
*/
struct sqlite3_session {
  sqlite3 *db;                    /* Database handle session is attached to */
  char *zDb;                      /* Name of database session is attached to */
  int bEnableSize;                /* True if changeset_size() enabled */
  int bEnable;                    /* True if currently recording */
  int bIndirect;                  /* True if all changes are indirect */
  int bAutoAttach;                /* True to auto-attach tables */
  int bImplicitPK;                /* True to handle tables with implicit PK */
  int rc;                         /* Non-zero if an error has occurred */
  void *pFilterCtx;               /* First argument to pass to xTableFilter */
  int (*xTableFilter)(void *pCtx, const char *zTab);
  i64 nMalloc;                    /* Number of bytes of data allocated */
  i64 nMaxChangesetSize;
  sqlite3_value *pZeroBlob;       /* Value containing X'' */
  sqlite3_session *pNext;         /* Next session object on same db. */
  SessionTable *pTable;           /* List of attached tables */
  SessionHook hook;               /* APIs to grab new and old data with */
};

/*
** Instances of this structure are used to build strings or binary records.
*/
struct SessionBuffer {
  u8 *aBuf;                       /* Pointer to changeset buffer */
  int nBuf;                       /* Size of buffer aBuf */
  int nAlloc;                     /* Size of allocation containing aBuf */
};

/*
** An object of this type is used internally as an abstraction for
** input data. Input data may be supplied either as a single large buffer
** (e.g. sqlite3changeset_start()) or using a stream function (e.g.
**  sqlite3changeset_start_strm()).
**
** bNoDiscard:
**   If true, then the only time data is discarded is as a result of explicit
**   sessionDiscardData() calls. Not within every sessionInputBuffer() call.
*/
struct SessionInput {
  int bNoDiscard;                 /* If true, do not discard in InputBuffer() */
  int iCurrent;                   /* Offset in aData[] of current change */
  int iNext;                      /* Offset in aData[] of next change */
  u8 *aData;                      /* Pointer to buffer containing changeset */
  int nData;                      /* Number of bytes in aData */

  SessionBuffer buf;              /* Current read buffer */
  int (*xInput)(void*, void*, int*);        /* Input stream call (or NULL) */
  void *pIn;                                /* First argument to xInput */
  int bEof;                       /* Set to true after xInput finished */
};

/*
** Structure for changeset iterators.
*/
struct sqlite3_changeset_iter {
  SessionInput in;                /* Input buffer or stream */
  SessionBuffer tblhdr;           /* Buffer to hold apValue/zTab/abPK/ */
  int bPatchset;                  /* True if this is a patchset */
  int bInvert;                    /* True to invert changeset */
  int bSkipEmpty;                 /* Skip noop UPDATE changes */
  int rc;                         /* Iterator error code */
  sqlite3_stmt *pConflict;        /* Points to conflicting row, if any */
  char *zTab;                     /* Current table */
  int nCol;                       /* Number of columns in zTab */
  int op;                         /* Current operation */
  int bIndirect;                  /* True if current change was indirect */
  u8 *abPK;                       /* Primary key array */
  sqlite3_value **apValue;        /* old.* and new.* values */
};

/*
** Each session object maintains a set of the following structures, one
** for each table the session object is monitoring. The structures are
** stored in a linked list starting at sqlite3_session.pTable.
**
** The keys of the SessionTable.aChange[] hash table are all rows that have
** been modified in any way since the session object was attached to the
** table.
**
** The data associated with each hash-table entry is a structure containing
** a subset of the initial values that the modified row contained at the
** start of the session. Or no initial values if the row was inserted.
**
** pDfltStmt:
**   This is only used by the sqlite3changegroup_xxx() APIs, not by
**   regular sqlite3_session objects. It is a SELECT statement that
**   selects the default value for each table column. For example,
**   if the table is
**
**      CREATE TABLE xx(a DEFAULT 1, b, c DEFAULT 'abc')
**
**   then this variable is the compiled version of:
**
**      SELECT 1, NULL, 'abc'
*/
struct SessionTable {
  SessionTable *pNext;
  char *zName;                    /* Local name of table */
  int nCol;                       /* Number of non-hidden columns */
  int nTotalCol;                  /* Number of columns including hidden */
  int bStat1;                     /* True if this is sqlite_stat1 */
  int bRowid;                     /* True if this table uses rowid for PK */
  const char **azCol;             /* Column names */
  const char **azDflt;            /* Default value expressions */
  int *aiIdx;                     /* Index to pass to xNew/xOld */
  u8 *abPK;                       /* Array of primary key flags */
  int nEntry;                     /* Total number of entries in hash table */
  int nChange;                    /* Size of apChange[] array */
  SessionChange **apChange;       /* Hash table buckets */
  sqlite3_stmt *pDfltStmt;
};

/*
** RECORD FORMAT:
**
** The following record format is similar to (but not compatible with) that
** used in SQLite database files. This format is used as part of the
** change-set binary format, and so must be architecture independent.
**
** Unlike the SQLite database record format, each field is self-contained -
** there is no separation of header and data. Each field begins with a
** single byte describing its type, as follows:
**
**       0x00: Undefined value.
**       0x01: Integer value.
**       0x02: Real value.
**       0x03: Text value.
**       0x04: Blob value.
**       0x05: SQL NULL value.
**
** Note that the above match the definitions of SQLITE_INTEGER, SQLITE_TEXT
** and so on in sqlite3.h. For undefined and NULL values, the field consists
** only of the single type byte. For other types of values, the type byte
** is followed by:
**
**   Text values:
**     A varint containing the number of bytes in the value (encoded using
**     UTF-8). Followed by a buffer containing the UTF-8 representation
**     of the text value. There is no nul terminator.
**
**   Blob values:
**     A varint containing the number of bytes in the value, followed by
**     a buffer containing the value itself.
**
**   Integer values:
**     An 8-byte big-endian integer value.
**
**   Real values:
**     An 8-byte big-endian IEEE 754-2008 real value.
**
** Varint values are encoded in the same way as varints in the SQLite
** record format.
**
** CHANGESET FORMAT:
**
** A changeset is a collection of DELETE, UPDATE and INSERT operations on
** one or more tables. Operations on a single table are grouped together,
** but may occur in any order (i.e. deletes, updates and inserts are all
** mixed together).
**
** Each group of changes begins with a table header:
**
**   1 byte: Constant 0x54 (capital 'T')
**   Varint: Number of columns in the table.
**   nCol bytes: 0x01 for PK columns, 0x00 otherwise.
**   N bytes: Unqualified table name (encoded using UTF-8). Nul-terminated.
**
** Followed by one or more changes to the table.
**
**   1 byte: Either SQLITE_INSERT (0x12), UPDATE (0x17) or DELETE (0x09).
**   1 byte: The "indirect-change" flag.
**   old.* record: (delete and update only)
**   new.* record: (insert and update only)
**
** The "old.*" and "new.*" records, if present, are N field records in the
** format described above under "RECORD FORMAT", where N is the number of
** columns in the table. The i'th field of each record is associated with
** the i'th column of the table, counting from left to right in the order
** in which columns were declared in the CREATE TABLE statement.
**
** The new.* record that is part of each INSERT change contains the values
** that make up the new row. Similarly, the old.* record that is part of each
** DELETE change contains the values that made up the row that was deleted
** from the database. In the changeset format, the records that are part
** of INSERT or DELETE changes never contain any undefined (type byte 0x00)
** fields.
**
** Within the old.* record associated with an UPDATE change, all fields
** associated with table columns that are not PRIMARY KEY columns and are
** not modified by the UPDATE change are set to "undefined". Other fields
** are set to the values that made up the row before the UPDATE that the
** change records took place. Within the new.* record, fields associated
** with table columns modified by the UPDATE change contain the new
** values. Fields associated with table columns that are not modified
** are set to "undefined".
**
** PATCHSET FORMAT:
**
** A patchset is also a collection of changes. It is similar to a changeset,
** but leaves undefined those fields that are not useful if no conflict
** resolution is required when applying the changeset.
**
** Each group of changes begins with a table header:
**
**   1 byte: Constant 0x50 (capital 'P')
**   Varint: Number of columns in the table.
**   nCol bytes: 0x01 for PK columns, 0x00 otherwise.
**   N bytes: Unqualified table name (encoded using UTF-8). Nul-terminated.
**
** Followed by one or more changes to the table.
**
**   1 byte: Either SQLITE_INSERT (0x12), UPDATE (0x17) or DELETE (0x09).
**   1 byte: The "indirect-change" flag.
**   single record: (PK fields for DELETE, PK and modified fields for UPDATE,
**                   full record for INSERT).
**
** As in the changeset format, each field of the single record that is part
** of a patchset change is associated with the correspondingly positioned
** table column, counting from left to right within the CREATE TABLE
** statement.
**
** For a DELETE change, all fields within the record except those associated
** with PRIMARY KEY columns are omitted. The PRIMARY KEY fields contain the
** values identifying the row to delete.
**
** For an UPDATE change, all fields except those associated with PRIMARY KEY
** columns and columns that are modified by the UPDATE are set to "undefined".
** PRIMARY KEY fields contain the values identifying the table row to update,
** and fields associated with modified columns contain the new column values.
**
** The records associated with INSERT changes are in the same format as for
** changesets. It is not possible for a record associated with an INSERT
** change to contain a field set to "undefined".
**
** REBASE BLOB FORMAT:
**
** A rebase blob may be output by sqlite3changeset_apply_v2() and its
** streaming equivalent for use with the sqlite3_rebaser APIs to rebase
** existing changesets. A rebase blob contains one entry for each conflict
** resolved using either the OMIT or REPLACE strategies within the apply_v2()
** call.
**
** The format used for a rebase blob is very similar to that used for
** changesets. All entries related to a single table are grouped together.
**
** Each group of entries begins with a table header in changeset format:
**
**   1 byte: Constant 0x54 (capital 'T')
**   Varint: Number of columns in the table.
**   nCol bytes: 0x01 for PK columns, 0x00 otherwise.
**   N bytes: Unqualified table name (encoded using UTF-8). Nul-terminated.
**
** Followed by one or more entries associated with the table.
**
**   1 byte: Either SQLITE_INSERT (0x12), DELETE (0x09).
**   1 byte: Flag. 0x01 for REPLACE, 0x00 for OMIT.
**   record: (in the record format defined above).
**
** In a rebase blob, the first field is set to SQLITE_INSERT if the change
** that caused the conflict was an INSERT or UPDATE, or to SQLITE_DELETE if
** it was a DELETE. The second field is set to 0x01 if the conflict
** resolution strategy was REPLACE, or 0x00 if it was OMIT.
**
** If the change that caused the conflict was a DELETE, then the single
** record is a copy of the old.* record from the original changeset. If it
** was an INSERT, then the single record is a copy of the new.* record. If
** the conflicting change was an UPDATE, then the single record is a copy
** of the new.* record with the PK fields filled in based on the original
** old.* record.
*/

/*
** For each row modified during a session, there exists a single instance of
** this structure stored in a SessionTable.aChange[] hash table.
*/
struct SessionChange {
  u8 op;                          /* One of UPDATE, DELETE, INSERT */
  u8 bIndirect;                   /* True if this change is "indirect" */
  u16 nRecordField;               /* Number of fields in aRecord[] */
  int nMaxSize;                   /* Max size of eventual changeset record */
  int nRecord;                    /* Number of bytes in buffer aRecord[] */
  u8 *aRecord;                    /* Buffer containing old.* record */
  SessionChange *pNext;           /* For hash-table collisions */
};

/*
** Write a varint with value iVal into the buffer at aBuf. Return the
** number of bytes written.
*/
static int sessionVarintPut(u8 *aBuf, int iVal){
  return putVarint32(aBuf, iVal);
}

/*
** Return the number of bytes required to store value iVal as a varint.
*/
static int sessionVarintLen(int iVal){
  return sqlite3VarintLen(iVal);
}

/*
** Read a varint value from aBuf[] into *piVal. Return the number of
** bytes read.
*/
static int sessionVarintGet(const u8 *aBuf, int *piVal){
  return getVarint32(aBuf, *piVal);
}

/*
** Read a varint value from buffer aBuf[], size nBuf bytes, into *piVal.
** Return the number of bytes read.
*/
static int sessionVarintGetSafe(const u8 *aBuf, int nBuf, int *piVal){
  u8 aCopy[9];
  const u8 *aRead = aBuf;
  memset(aCopy, 0, sizeof(aCopy));
  if( nBuf<sizeof(aCopy) ){
    memcpy(aCopy, aBuf, nBuf);
    aRead = aCopy;
  }
  return getVarint32(aRead, *piVal);
}

/* Load an unaligned and unsigned 32-bit integer */
#define SESSION_UINT32(x) (((u32)(x)[0]<<24)|((x)[1]<<16)|((x)[2]<<8)|(x)[3])

/*
** Read a 64-bit big-endian integer value from buffer aRec[]. Return
** the value read.
*/
static sqlite3_int64 sessionGetI64(u8 *aRec){
  u64 x = SESSION_UINT32(aRec);
  u32 y = SESSION_UINT32(aRec+4);
  x = (x<<32) + y;
  return (sqlite3_int64)x;
}

/*
** Write a 64-bit big-endian integer value to the buffer aBuf[].
*/
static void sessionPutI64(u8 *aBuf, sqlite3_int64 i){
  aBuf[0] = (i>>56) & 0xFF;
  aBuf[1] = (i>>48) & 0xFF;
  aBuf[2] = (i>>40) & 0xFF;
  aBuf[3] = (i>>32) & 0xFF;
  aBuf[4] = (i>>24) & 0xFF;
  aBuf[5] = (i>>16) & 0xFF;
  aBuf[6] = (i>> 8) & 0xFF;
  aBuf[7] = (i>> 0) & 0xFF;
}

/*
** Write a double value to the buffer aBuf[].
*/
static void sessionPutDouble(u8 *aBuf, double r){
  /* TODO: SQLite does something special to deal with mixed-endian
  ** floating point values (e.g. ARM7). This code probably should
  ** too.  */
  u64 i;
  assert( sizeof(double)==8 && sizeof(u64)==8 );
  memcpy(&i, &r, 8);
  sessionPutI64(aBuf, i);
}

/*
** This function is used to serialize the contents of value pValue (see
** comment titled "RECORD FORMAT" above).
**
** If it is non-NULL, the serialized form of the value is written to
** buffer aBuf. *pnWrite is set to the number of bytes written before
** returning. Or, if aBuf is NULL, the only thing this function does is
** set *pnWrite.
**
** If no error occurs, SQLITE_OK is returned. Or, if an OOM error occurs
** within a call to sqlite3_value_text() (may fail if the db is utf-16))
** SQLITE_NOMEM is returned.
*/
static int sessionSerializeValue(
  u8 *aBuf,                       /* If non-NULL, write serialized value here */
  sqlite3_value *pValue,          /* Value to serialize */
  sqlite3_int64 *pnWrite          /* IN/OUT: Increment by bytes written */
){
  int nByte;                      /* Size of serialized value in bytes */

  if( pValue ){
    int eType;                    /* Value type (SQLITE_NULL, TEXT etc.) */

    eType = sqlite3_value_type(pValue);
    if( aBuf ) aBuf[0] = eType;

    switch( eType ){
      case SQLITE_NULL:
        nByte = 1;
        break;

      case SQLITE_INTEGER:
      case SQLITE_FLOAT:
        if( aBuf ){
          /* TODO: SQLite does something special to deal with mixed-endian
          ** floating point values (e.g. ARM7). This code probably should
          ** too.  */
          if( eType==SQLITE_INTEGER ){
            u64 i = (u64)sqlite3_value_int64(pValue);
            sessionPutI64(&aBuf[1], i);
          }else{
            double r = sqlite3_value_double(pValue);
            sessionPutDouble(&aBuf[1], r);
          }
        }
        nByte = 9;
        break;

      default: {
        u8 *z;
        int n;
        int nVarint;

        assert( eType==SQLITE_TEXT || eType==SQLITE_BLOB );
        if( eType==SQLITE_TEXT ){
          z = (u8 *)sqlite3_value_text(pValue);
        }else{
          z = (u8 *)sqlite3_value_blob(pValue);
        }
        n = sqlite3_value_bytes(pValue);
        if( z==0 && (eType!=SQLITE_BLOB || n>0) ) return SQLITE_NOMEM;
        nVarint = sessionVarintLen(n);

        if( aBuf ){
          sessionVarintPut(&aBuf[1], n);
          if( n>0 ) memcpy(&aBuf[nVarint + 1], z, n);
        }

        nByte = 1 + nVarint + n;
        break;
      }
    }
  }else{
    nByte = 1;
    if( aBuf ) aBuf[0] = '\0';
  }

  if( pnWrite ) *pnWrite += nByte;
  return SQLITE_OK;
}

/*
** Allocate and return a pointer to a buffer nByte bytes in size. If
** pSession is not NULL, increase the sqlite3_session.nMalloc variable
** by the number of bytes allocated.
*/
static void *sessionMalloc64(sqlite3_session *pSession, i64 nByte){
  void *pRet = sqlite3_malloc64(nByte);
  if( pSession ) pSession->nMalloc += sqlite3_msize(pRet);
  return pRet;
}

/*
** Free buffer pFree, which must have been allocated by an earlier
** call to sessionMalloc64(). If pSession is not NULL, decrease the
** sqlite3_session.nMalloc counter by the number of bytes freed.
*/
static void sessionFree(sqlite3_session *pSession, void *pFree){
  if( pSession ) pSession->nMalloc -= sqlite3_msize(pFree);
  sqlite3_free(pFree);
}

/*
** This macro is used to calculate hash key values for data structures. In
** order to use this macro, the entire data structure must be represented
** as a series of unsigned integers. In order to calculate a hash-key value
** for a data structure represented as three such integers, the macro may
** then be used as follows:
**
**    int hash_key_value;
**    hash_key_value = HASH_APPEND(0, <value 1>);
**    hash_key_value = HASH_APPEND(hash_key_value, <value 2>);
**    hash_key_value = HASH_APPEND(hash_key_value, <value 3>);
**
** In practice, the data structures this macro is used for are the primary
** key values of modified rows.
*/
#define HASH_APPEND(hash, add) ((hash) << 3) ^ (hash) ^ (unsigned int)(add)

/*
** Append the hash of the 64-bit integer passed as the second argument to the
** hash-key value passed as the first. Return the new hash-key value.
*/
static unsigned int sessionHashAppendI64(unsigned int h, i64 i){
  h = HASH_APPEND(h, i & 0xFFFFFFFF);
  return HASH_APPEND(h, (i>>32)&0xFFFFFFFF);
}

/*
** Append the hash of the blob passed via the second and third arguments to
** the hash-key value passed as the first. Return the new hash-key value.
*/
static unsigned int sessionHashAppendBlob(unsigned int h, int n, const u8 *z){
  int i;
  for(i=0; i<n; i++) h = HASH_APPEND(h, z[i]);
  return h;
}

/*
** Append the hash of the data type passed as the second argument to the
** hash-key value passed as the first. Return the new hash-key value.
*/
static unsigned int sessionHashAppendType(unsigned int h, int eType){
  return HASH_APPEND(h, eType);
}

/*
** This function may only be called from within a pre-update callback.
** It calculates a hash based on the primary key values of the old.* or
** new.* row currently available and, assuming no error occurs, writes it to
** *piHash before returning. If the primary key contains one or more NULL
** values, *pbNullPK is set to true before returning.
**
** If an error occurs, an SQLite error code is returned and the final values
** of *piHash asn *pbNullPK are undefined. Otherwise, SQLITE_OK is returned
** and the output variables are set as described above.
*/
static int sessionPreupdateHash(
  sqlite3_session *pSession,      /* Session object that owns pTab */
  i64 iRowid,
  SessionTable *pTab,             /* Session table handle */
  int bNew,                       /* True to hash the new.* PK */
  int *piHash,                    /* OUT: Hash value */
  int *pbNullPK                   /* OUT: True if there are NULL values in PK */
){
  unsigned int h = 0;             /* Hash value to return */
  int i;                          /* Used to iterate through columns */

  assert( pTab->nTotalCol==pSession->hook.xCount(pSession->hook.pCtx) );
  if( pTab->bRowid ){
    h = sessionHashAppendI64(h, iRowid);
  }else{
    assert( *pbNullPK==0 );
    for(i=0; i<pTab->nCol; i++){
      if( pTab->abPK[i] ){
        int rc;
        int eType;
        sqlite3_value *pVal;
        int iIdx = pTab->aiIdx[i];

        if( bNew ){
          rc = pSession->hook.xNew(pSession->hook.pCtx, iIdx, &pVal);
        }else{
          rc = pSession->hook.xOld(pSession->hook.pCtx, iIdx, &pVal);
        }
        if( rc!=SQLITE_OK ) return rc;

        eType = sqlite3_value_type(pVal);
        h = sessionHashAppendType(h, eType);
        if( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT ){
          i64 iVal;
          if( eType==SQLITE_INTEGER ){
            iVal = sqlite3_value_int64(pVal);
          }else{
            double rVal = sqlite3_value_double(pVal);
            assert( sizeof(iVal)==8 && sizeof(rVal)==8 );
            memcpy(&iVal, &rVal, 8);
          }
          h = sessionHashAppendI64(h, iVal);
        }else if( eType==SQLITE_TEXT || eType==SQLITE_BLOB ){
          const u8 *z;
          int n;
          if( eType==SQLITE_TEXT ){
            z = (const u8 *)sqlite3_value_text(pVal);
          }else{
            z = (const u8 *)sqlite3_value_blob(pVal);
          }
          n = sqlite3_value_bytes(pVal);
          if( !z && (eType!=SQLITE_BLOB || n>0) ) return SQLITE_NOMEM;
          h = sessionHashAppendBlob(h, n, z);
        }else{
          assert( eType==SQLITE_NULL );
          assert( pTab->bStat1==0 || i!=1 );
          *pbNullPK = 1;
        }
      }
    }
  }

  *piHash = (h % pTab->nChange);
  return SQLITE_OK;
}

/*
** The buffer that the argument points to contains a serialized SQL value.
** Return the number of bytes of space occupied by the value (including
** the type byte).
*/
static int sessionSerialLen(const u8 *a){
  int e;
  int n;
  assert( a!=0 );
  e = *a;
  if( e==0 || e==0xFF ) return 1;
  if( e==SQLITE_NULL ) return 1;
  if( e==SQLITE_INTEGER || e==SQLITE_FLOAT ) return 9;
  return sessionVarintGet(&a[1], &n) + 1 + n;
}

/*
** Based on the primary key values stored in change aRecord, calculate a
** hash key. Assume the has table has nBucket buckets. The hash keys
** calculated by this function are compatible with those calculated by
** sessionPreupdateHash().
**
** The bPkOnly argument is non-zero if the record at aRecord[] is from
** a patchset DELETE. In this case the non-PK fields are omitted entirely.
*/
static unsigned int sessionChangeHash(
  SessionTable *pTab,             /* Table handle */
  int bPkOnly,                    /* Record consists of PK fields only */
  u8 *aRecord,                    /* Change record */
  int nBucket                     /* Assume this many buckets in hash table */
){
  unsigned int h = 0;             /* Value to return */
  int i;                          /* Used to iterate through columns */
  u8 *a = aRecord;                /* Used to iterate through change record */

  for(i=0; i<pTab->nCol; i++){
    int eType = *a;
    int isPK = pTab->abPK[i];
    if( bPkOnly && isPK==0 ) continue;

    assert( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT
         || eType==SQLITE_TEXT || eType==SQLITE_BLOB
         || eType==SQLITE_NULL || eType==0
    );

    if( isPK ){
      a++;
      h = sessionHashAppendType(h, eType);
      if( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT ){
        h = sessionHashAppendI64(h, sessionGetI64(a));
        a += 8;
      }else if( eType==SQLITE_TEXT || eType==SQLITE_BLOB ){
        int n;
        a += sessionVarintGet(a, &n);
        h = sessionHashAppendBlob(h, n, a);
        a += n;
      }
      /* It should not be possible for eType to be SQLITE_NULL or 0x00 here,
      ** as the session module does not record changes for rows with NULL
      ** values stored in primary key columns. But a corrupt changesets
      ** may contain such a value.  */
    }else{
      a += sessionSerialLen(a);
    }
  }
  return (h % nBucket);
}

/*
** Arguments aLeft and aRight are pointers to change records for table pTab.
** This function returns true if the two records apply to the same row (i.e.
** have the same values stored in the primary key columns), or false
** otherwise.
*/
static int sessionChangeEqual(
  SessionTable *pTab,             /* Table used for PK definition */
  int bLeftPkOnly,                /* True if aLeft[] contains PK fields only */
  u8 *aLeft,                      /* Change record */
  int bRightPkOnly,               /* True if aRight[] contains PK fields only */
  u8 *aRight                      /* Change record */
){
  u8 *a1 = aLeft;                 /* Cursor to iterate through aLeft */
  u8 *a2 = aRight;                /* Cursor to iterate through aRight */
  int iCol;                       /* Used to iterate through table columns */

  for(iCol=0; iCol<pTab->nCol; iCol++){
    if( pTab->abPK[iCol] ){
      int n1 = sessionSerialLen(a1);
      int n2 = sessionSerialLen(a2);

      if( n1!=n2 || memcmp(a1, a2, n1) ){
        return 0;
      }
      a1 += n1;
      a2 += n2;
    }else{
      if( bLeftPkOnly==0 ) a1 += sessionSerialLen(a1);
      if( bRightPkOnly==0 ) a2 += sessionSerialLen(a2);
    }
  }

  return 1;
}

/*
** Arguments aLeft and aRight both point to buffers containing change
** records with nCol columns. This function "merges" the two records into
** a single records which is written to the buffer at *paOut. *paOut is
** then set to point to one byte after the last byte written before
** returning.
**
** The merging of records is done as follows: For each column, if the
** aRight record contains a value for the column, copy the value from
** their. Otherwise, if aLeft contains a value, copy it. If neither
** record contains a value for a given column, then neither does the
** output record.
*/
static void sessionMergeRecord(
  u8 **paOut,
  int nCol,
  u8 *aLeft,
  u8 *aRight
){
  u8 *a1 = aLeft;                 /* Cursor used to iterate through aLeft */
  u8 *a2 = aRight;                /* Cursor used to iterate through aRight */
  u8 *aOut = *paOut;              /* Output cursor */
  int iCol;                       /* Used to iterate from 0 to nCol */

  for(iCol=0; iCol<nCol; iCol++){
    int n1 = sessionSerialLen(a1);
    int n2 = sessionSerialLen(a2);
    if( *a2 ){
      memcpy(aOut, a2, n2);
      aOut += n2;
    }else{
      memcpy(aOut, a1, n1);
      aOut += n1;
    }
    a1 += n1;
    a2 += n2;
  }

  *paOut = aOut;
}

/*
** This is a helper function used by sessionMergeUpdate().
**
** When this function is called, both *paOne and *paTwo point to a value
** within a change record. Before it returns, both have been advanced so
** as to point to the next value in the record.
**
** If, when this function is called, *paTwo points to a valid value (i.e.
** *paTwo[0] is not 0x00 - the "no value" placeholder), a copy of the *paTwo
** pointer is returned and *pnVal is set to the number of bytes in the
** serialized value. Otherwise, a copy of *paOne is returned and *pnVal
** set to the number of bytes in the value at *paOne. If *paOne points
** to the "no value" placeholder, *pnVal is set to 1. In other words:
**
**   if( *paTwo is valid ) return *paTwo;
**   return *paOne;
**
*/
static u8 *sessionMergeValue(
  u8 **paOne,                     /* IN/OUT: Left-hand buffer pointer */
  u8 **paTwo,                     /* IN/OUT: Right-hand buffer pointer */
  int *pnVal                      /* OUT: Bytes in returned value */
){
  u8 *a1 = *paOne;
  u8 *a2 = *paTwo;
  u8 *pRet = 0;
  int n1;

  assert( a1 );
  if( a2 ){
    int n2 = sessionSerialLen(a2);
    if( *a2 ){
      *pnVal = n2;
      pRet = a2;
    }
    *paTwo = &a2[n2];
  }

  n1 = sessionSerialLen(a1);
  if( pRet==0 ){
    *pnVal = n1;
    pRet = a1;
  }
  *paOne = &a1[n1];

  return pRet;
}

/*
** This function is used by changeset_concat() to merge two UPDATE changes
** on the same row.
*/
static int sessionMergeUpdate(
  u8 **paOut,                     /* IN/OUT: Pointer to output buffer */
  SessionTable *pTab,             /* Table change pertains to */
  int bPatchset,                  /* True if records are patchset records */
  u8 *aOldRecord1,                /* old.* record for first change */
  u8 *aOldRecord2,                /* old.* record for second change */
  u8 *aNewRecord1,                /* new.* record for first change */
  u8 *aNewRecord2                 /* new.* record for second change */
){
  u8 *aOld1 = aOldRecord1;
  u8 *aOld2 = aOldRecord2;
  u8 *aNew1 = aNewRecord1;
  u8 *aNew2 = aNewRecord2;

  u8 *aOut = *paOut;
  int i;

  if( bPatchset==0 ){
    int bRequired = 0;

    assert( aOldRecord1 && aNewRecord1 );

    /* Write the old.* vector first. */
    for(i=0; i<pTab->nCol; i++){
      int nOld;
      u8 *aOld;
      int nNew;
      u8 *aNew;

      aOld = sessionMergeValue(&aOld1, &aOld2, &nOld);
      aNew = sessionMergeValue(&aNew1, &aNew2, &nNew);
      if( pTab->abPK[i] || nOld!=nNew || memcmp(aOld, aNew, nNew) ){
        if( pTab->abPK[i]==0 ) bRequired = 1;
        memcpy(aOut, aOld, nOld);
        aOut += nOld;
      }else{
        *(aOut++) = '\0';
      }
    }

    if( !bRequired ) return 0;
  }

  /* Write the new.* vector */
  aOld1 = aOldRecord1;
  aOld2 = aOldRecord2;
  aNew1 = aNewRecord1;
  aNew2 = aNewRecord2;
  for(i=0; i<pTab->nCol; i++){
    int nOld;
    u8 *aOld;
    int nNew;
    u8 *aNew;

    aOld = sessionMergeValue(&aOld1, &aOld2, &nOld);
    aNew = sessionMergeValue(&aNew1, &aNew2, &nNew);
    if( bPatchset==0
     && (pTab->abPK[i] || (nOld==nNew && 0==memcmp(aOld, aNew, nNew)))
    ){
      *(aOut++) = '\0';
    }else{
      memcpy(aOut, aNew, nNew);
      aOut += nNew;
    }
  }

  *paOut = aOut;
  return 1;
}

/*
** This function is only called from within a pre-update-hook callback.
** It determines if the current pre-update-hook change affects the same row
** as the change stored in argument pChange. If so, it returns true. Otherwise
** if the pre-update-hook does not affect the same row as pChange, it returns
** false.
*/
static int sessionPreupdateEqual(
  sqlite3_session *pSession,      /* Session object that owns SessionTable */
  i64 iRowid,                     /* Rowid value if pTab->bRowid */
  SessionTable *pTab,             /* Table associated with change */
  SessionChange *pChange,         /* Change to compare to */
  int op                          /* Current pre-update operation */
){
  int iCol;                       /* Used to iterate through columns */
  u8 *a = pChange->aRecord;       /* Cursor used to scan change record */

  if( pTab->bRowid ){
    if( a[0]!=SQLITE_INTEGER ) return 0;
    return sessionGetI64(&a[1])==iRowid;
  }

  assert( op==SQLITE_INSERT || op==SQLITE_UPDATE || op==SQLITE_DELETE );
  for(iCol=0; iCol<pTab->nCol; iCol++){
    if( !pTab->abPK[iCol] ){
      a += sessionSerialLen(a);
    }else{
      sqlite3_value *pVal;        /* Value returned by preupdate_new/old */
      int rc;                     /* Error code from preupdate_new/old */
      int eType = *a++;           /* Type of value from change record */
      int iIdx = pTab->aiIdx[iCol];

      /* The following calls to preupdate_new() and preupdate_old() can not
      ** fail. This is because they cache their return values, and by the
      ** time control flows to here they have already been called once from
      ** within sessionPreupdateHash(). The first two asserts below verify
      ** this (that the method has already been called). */
      if( op==SQLITE_INSERT ){
        /* assert( db->pPreUpdate->pNewUnpacked || db->pPreUpdate->aNew ); */
        rc = pSession->hook.xNew(pSession->hook.pCtx, iIdx, &pVal);
      }else{
        /* assert( db->pPreUpdate->pUnpacked ); */
        rc = pSession->hook.xOld(pSession->hook.pCtx, iIdx, &pVal);
      }
      assert( rc==SQLITE_OK );
      (void)rc;                   /* Suppress warning about unused variable */
      if( sqlite3_value_type(pVal)!=eType ) return 0;

      /* A SessionChange object never has a NULL value in a PK column */
      assert( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT
           || eType==SQLITE_BLOB    || eType==SQLITE_TEXT
      );

      if( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT ){
        i64 iVal = sessionGetI64(a);
        a += 8;
        if( eType==SQLITE_INTEGER ){
          if( sqlite3_value_int64(pVal)!=iVal ) return 0;
        }else{
          double rVal;
          assert( sizeof(iVal)==8 && sizeof(rVal)==8 );
          memcpy(&rVal, &iVal, 8);
          if( sqlite3_value_double(pVal)!=rVal ) return 0;
        }
      }else{
        int n;
        const u8 *z;
        a += sessionVarintGet(a, &n);
        if( sqlite3_value_bytes(pVal)!=n ) return 0;
        if( eType==SQLITE_TEXT ){
          z = sqlite3_value_text(pVal);
        }else{
          z = sqlite3_value_blob(pVal);
        }
        if( n>0 && memcmp(a, z, n) ) return 0;
        a += n;
      }
    }
  }

  return 1;
}

/*
** If required, grow the hash table used to store changes on table pTab
** (part of the session pSession). If a fatal OOM error occurs, set the
** session object to failed and return SQLITE_ERROR. Otherwise, return
** SQLITE_OK.
**
** It is possible that a non-fatal OOM error occurs in this function. In
** that case the hash-table does not grow, but SQLITE_OK is returned anyway.
** Growing the hash table in this case is a performance optimization only,
** it is not required for correct operation.
*/
static int sessionGrowHash(
  sqlite3_session *pSession,      /* For memory accounting. May be NULL */
  int bPatchset,
  SessionTable *pTab
){
  if( pTab->nChange==0 || pTab->nEntry>=(pTab->nChange/2) ){
    int i;
    SessionChange **apNew;
    sqlite3_int64 nNew = 2*(sqlite3_int64)(pTab->nChange ? pTab->nChange : 128);

    apNew = (SessionChange**)sessionMalloc64(
        pSession, sizeof(SessionChange*) * nNew
    );
    if( apNew==0 ){
      if( pTab->nChange==0 ){
        return SQLITE_ERROR;
      }
      return SQLITE_OK;
    }
    memset(apNew, 0, sizeof(SessionChange *) * nNew);

    for(i=0; i<pTab->nChange; i++){
      SessionChange *p;
      SessionChange *pNext;
      for(p=pTab->apChange[i]; p; p=pNext){
        int bPkOnly = (p->op==SQLITE_DELETE && bPatchset);
        int iHash = sessionChangeHash(pTab, bPkOnly, p->aRecord, nNew);
        pNext = p->pNext;
        p->pNext = apNew[iHash];
        apNew[iHash] = p;
      }
    }

    sessionFree(pSession, pTab->apChange);
    pTab->nChange = nNew;
    pTab->apChange = apNew;
  }

  return SQLITE_OK;
}

/*
** This function queries the database for the names of the columns of table
** zThis, in schema zDb.
**
** Otherwise, if they are not NULL, variable *pnCol is set to the number
** of columns in the database table and variable *pzTab is set to point to a
** nul-terminated copy of the table name. *pazCol (if not NULL) is set to
** point to an array of pointers to column names. And *pabPK (again, if not
** NULL) is set to point to an array of booleans - true if the corresponding
** column is part of the primary key.
**
** For example, if the table is declared as:
**
**     CREATE TABLE tbl1(w, x DEFAULT 'abc', y, z, PRIMARY KEY(w, z));
**
** Then the five output variables are populated as follows:
**
**     *pnCol  = 4
**     *pzTab  = "tbl1"
**     *pazCol = {"w", "x", "y", "z"}
**     *pazDflt = {NULL, 'abc', NULL, NULL}
**     *pabPK  = {1, 0, 0, 1}
**
** All returned buffers are part of the same single allocation, which must
** be freed using sqlite3_free() by the caller
*/
static int sessionTableInfo(
  sqlite3_session *pSession,      /* For memory accounting. May be NULL */
  sqlite3 *db,                    /* Database connection */
  const char *zDb,                /* Name of attached database (e.g. "main") */
  const char *zThis,              /* Table name */
  int *pnCol,                     /* OUT: number of columns */
  int *pnTotalCol,                /* OUT: number of hidden columns */
  const char **pzTab,             /* OUT: Copy of zThis */
  const char ***pazCol,           /* OUT: Array of column names for table */
  const char ***pazDflt,          /* OUT: Array of default value expressions */
  int **paiIdx,                   /* OUT: Array of xNew/xOld indexes */
  u8 **pabPK,                     /* OUT: Array of booleans - true for PK col */
  int *pbRowid                    /* OUT: True if only PK is a rowid */
){
  char *zPragma;
  sqlite3_stmt *pStmt;
  int rc;
  sqlite3_int64 nByte;
  int nDbCol = 0;
  int nThis;
  int i;
  u8 *pAlloc = 0;
  char **azCol = 0;
  char **azDflt = 0;
  u8 *abPK = 0;
  int *aiIdx = 0;
  int bRowid = 0;                 /* Set to true to use rowid as PK */

  assert( pazCol && pabPK );

  *pazCol = 0;
  *pabPK = 0;
  *pnCol = 0;
  if( pnTotalCol ) *pnTotalCol = 0;
  if( paiIdx ) *paiIdx = 0;
  if( pzTab ) *pzTab = 0;
  if( pazDflt ) *pazDflt = 0;

  nThis = sqlite3Strlen30(zThis);
  if( nThis==12 && 0==sqlite3_stricmp("sqlite_stat1", zThis) ){
    rc = sqlite3_table_column_metadata(db, zDb, zThis, 0, 0, 0, 0, 0, 0);
    if( rc==SQLITE_OK ){
      /* For sqlite_stat1, pretend that (tbl,idx) is the PRIMARY KEY. */
      zPragma = sqlite3_mprintf(
          "SELECT 0, 'tbl',  '', 0, '', 1, 0     UNION ALL "
          "SELECT 1, 'idx',  '', 0, '', 2, 0     UNION ALL "
          "SELECT 2, 'stat', '', 0, '', 0, 0"
      );
    }else if( rc==SQLITE_ERROR ){
      zPragma = sqlite3_mprintf("");
    }else{
      return rc;
    }
  }else{
    zPragma = sqlite3_mprintf("PRAGMA '%q'.table_xinfo('%q')", zDb, zThis);
  }
  if( !zPragma ){
    return SQLITE_NOMEM;
  }

  rc = sqlite3_prepare_v2(db, zPragma, -1, &pStmt, 0);
  sqlite3_free(zPragma);
  if( rc!=SQLITE_OK ){
    return rc;
  }

  nByte = nThis + 1;
  bRowid = (pbRowid!=0);
  while( SQLITE_ROW==sqlite3_step(pStmt) ){
    nByte += sqlite3_column_bytes(pStmt, 1);          /* name */
    nByte += sqlite3_column_bytes(pStmt, 4);          /* dflt_value */
    if( sqlite3_column_int(pStmt, 6)==0 ){            /* !hidden */
      nDbCol++;
    }
    if( sqlite3_column_int(pStmt, 5) ) bRowid = 0;    /* pk */
  }
  if( nDbCol==0 ) bRowid = 0;
  nDbCol += bRowid;
  nByte += strlen(SESSIONS_ROWID);
  rc = sqlite3_reset(pStmt);

  if( rc==SQLITE_OK ){
    nByte += nDbCol * (sizeof(const char *)*2 +sizeof(int)+sizeof(u8) + 1 + 1);
    pAlloc = sessionMalloc64(pSession, nByte);
    if( pAlloc==0 ){
      rc = SQLITE_NOMEM;
    }else{
      memset(pAlloc, 0, nByte);
    }
  }
  if( rc==SQLITE_OK ){
    azCol = (char **)pAlloc;
    azDflt = (char**)&azCol[nDbCol];
    aiIdx = (int*)&azDflt[nDbCol];
    abPK = (u8 *)&aiIdx[nDbCol];
    pAlloc = &abPK[nDbCol];
    if( pzTab ){
      memcpy(pAlloc, zThis, nThis+1);
      *pzTab = (char *)pAlloc;
      pAlloc += nThis+1;
    }

    i = 0;
    if( bRowid ){
      size_t nName = strlen(SESSIONS_ROWID);
      memcpy(pAlloc, SESSIONS_ROWID, nName+1);
      azCol[i] = (char*)pAlloc;
      pAlloc += nName+1;
      abPK[i] = 1;
      aiIdx[i] = -1;
      i++;
    }
    while( SQLITE_ROW==sqlite3_step(pStmt) ){
      if( sqlite3_column_int(pStmt, 6)==0 ){            /* !hidden */
        int nName = sqlite3_column_bytes(pStmt, 1);
        int nDflt = sqlite3_column_bytes(pStmt, 4);
        const unsigned char *zName = sqlite3_column_text(pStmt, 1);
        const unsigned char *zDflt = sqlite3_column_text(pStmt, 4);

        if( zName==0 ) break;
        memcpy(pAlloc, zName, nName+1);
        azCol[i] = (char *)pAlloc;
        pAlloc += nName+1;
        if( zDflt ){
          memcpy(pAlloc, zDflt, nDflt+1);
          azDflt[i] = (char *)pAlloc;
          pAlloc += nDflt+1;
        }else{
          azDflt[i] = 0;
        }
        abPK[i] = sqlite3_column_int(pStmt, 5);
        aiIdx[i] = sqlite3_column_int(pStmt, 0);
        i++;
      }
      if( pnTotalCol ) (*pnTotalCol)++;
    }
    rc = sqlite3_reset(pStmt);
  }

  /* If successful, populate the output variables. Otherwise, zero them and
  ** free any allocation made. An error code will be returned in this case.
  */
  if( rc==SQLITE_OK ){
    *pazCol = (const char**)azCol;
    if( pazDflt ) *pazDflt = (const char**)azDflt;
    *pabPK = abPK;
    *pnCol = nDbCol;
    if( paiIdx ) *paiIdx = aiIdx;
  }else{
    sessionFree(pSession, azCol);
  }
  if( pbRowid ) *pbRowid = bRowid;
  sqlite3_finalize(pStmt);
  return rc;
}

/*
** This function is called to initialize the SessionTable.nCol, azCol[]
** abPK[] and azDflt[] members of SessionTable object pTab. If these
** fields are already initialized, this function is a no-op.
**
** If an error occurs, an error code is stored in sqlite3_session.rc and
** non-zero returned. Or, if no error occurs but the table has no primary
** key, sqlite3_session.rc is left set to SQLITE_OK and non-zero returned to
** indicate that updates on this table should be ignored. SessionTable.abPK
** is set to NULL in this case.
*/
static int sessionInitTable(
  sqlite3_session *pSession,      /* Optional session handle */
  SessionTable *pTab,             /* Table object to initialize */
  sqlite3 *db,                    /* Database handle to read schema from */
  const char *zDb                 /* Name of db - "main", "temp" etc. */
){
  int rc = SQLITE_OK;

  if( pTab->nCol==0 ){
    u8 *abPK;
    assert( pTab->azCol==0 || pTab->abPK==0 );
    sqlite3_free(pTab->azCol);
    pTab->abPK = 0;
    rc = sessionTableInfo(pSession, db, zDb,
        pTab->zName, &pTab->nCol, &pTab->nTotalCol, 0, &pTab->azCol,
        &pTab->azDflt, &pTab->aiIdx, &abPK,
        ((pSession==0 || pSession->bImplicitPK) ? &pTab->bRowid : 0)
    );
    if( rc==SQLITE_OK ){
      int i;
      for(i=0; i<pTab->nCol; i++){
        if( abPK[i] ){
          pTab->abPK = abPK;
          break;
        }
      }
      if( 0==sqlite3_stricmp("sqlite_stat1", pTab->zName) ){
        pTab->bStat1 = 1;
      }

      if( pSession && pSession->bEnableSize ){
        pSession->nMaxChangesetSize += (
          1 + sessionVarintLen(pTab->nCol) + pTab->nCol + strlen(pTab->zName)+1
        );
      }
    }
  }

  if( pSession ){
    pSession->rc = rc;
    return (rc || pTab->abPK==0);
  }
  return rc;
}

/*
** Re-initialize table object pTab.
*/
static int sessionReinitTable(sqlite3_session *pSession, SessionTable *pTab){
  int nCol = 0;
  int nTotalCol = 0;
  const char **azCol = 0;
  const char **azDflt = 0;
  int *aiIdx = 0;
  u8 *abPK = 0;
  int bRowid = 0;

  assert( pSession->rc==SQLITE_OK );

  pSession->rc = sessionTableInfo(pSession, pSession->db, pSession->zDb,
      pTab->zName, &nCol, &nTotalCol, 0, &azCol, &azDflt, &aiIdx, &abPK,
      (pSession->bImplicitPK ? &bRowid : 0)
  );
  if( pSession->rc==SQLITE_OK ){
    if( pTab->nCol>nCol || pTab->bRowid!=bRowid ){
      pSession->rc = SQLITE_SCHEMA;
    }else{
      int ii;
      int nOldCol = pTab->nCol;
      for(ii=0; ii<nCol; ii++){
        if( ii<pTab->nCol ){
          if( pTab->abPK[ii]!=abPK[ii] ){
            pSession->rc = SQLITE_SCHEMA;
          }
        }else if( abPK[ii] ){
          pSession->rc = SQLITE_SCHEMA;
        }
      }

      if( pSession->rc==SQLITE_OK ){
        const char **a = pTab->azCol;
        pTab->azCol = azCol;
        pTab->nCol = nCol;
        pTab->nTotalCol = nTotalCol;
        pTab->azDflt = azDflt;
        pTab->abPK = abPK;
        pTab->aiIdx = aiIdx;
        azCol = a;
      }
      if( pSession->bEnableSize ){
        pSession->nMaxChangesetSize += (nCol - nOldCol);
        pSession->nMaxChangesetSize += sessionVarintLen(nCol);
        pSession->nMaxChangesetSize -= sessionVarintLen(nOldCol);
      }
    }
  }

  sqlite3_free((char*)azCol);
  return pSession->rc;
}

/*
** Session-change object (*pp) contains an old.* record with fewer than
** nCol fields. This function updates it with the default values for
** the missing fields.
*/
static void sessionUpdateOneChange(
  sqlite3_session *pSession,      /* For memory accounting */
  int *pRc,                       /* IN/OUT: Error code */
  SessionChange **pp,             /* IN/OUT: Change object to update */
  int nCol,                       /* Number of columns now in table */
  sqlite3_stmt *pDflt             /* SELECT <default-values...> */
){
  SessionChange *pOld = *pp;

  while( pOld->nRecordField<nCol ){
    SessionChange *pNew = 0;
    int nByte = 0;
    int nIncr = 0;
    int iField = pOld->nRecordField;
    int eType = sqlite3_column_type(pDflt, iField);
    switch( eType ){
      case SQLITE_NULL:
        nIncr = 1;
        break;
      case SQLITE_INTEGER:
      case SQLITE_FLOAT:
        nIncr = 9;
        break;
      default: {
        int n = sqlite3_column_bytes(pDflt, iField);
        nIncr = 1 + sessionVarintLen(n) + n;
        assert( eType==SQLITE_TEXT || eType==SQLITE_BLOB );
        break;
      }
    }

    nByte = nIncr + (sizeof(SessionChange) + pOld->nRecord);
    pNew = sessionMalloc64(pSession, nByte);
    if( pNew==0 ){
      *pRc = SQLITE_NOMEM;
      return;
    }else{
      memcpy(pNew, pOld, sizeof(SessionChange));
      pNew->aRecord = (u8*)&pNew[1];
      memcpy(pNew->aRecord, pOld->aRecord, pOld->nRecord);
      pNew->aRecord[pNew->nRecord++] = (u8)eType;
      switch( eType ){
        case SQLITE_INTEGER: {
          i64 iVal = sqlite3_column_int64(pDflt, iField);
          sessionPutI64(&pNew->aRecord[pNew->nRecord], iVal);
          pNew->nRecord += 8;
          break;
        }

        case SQLITE_FLOAT: {
          double rVal = sqlite3_column_double(pDflt, iField);
          sessionPutDouble(&pNew->aRecord[pNew->nRecord], rVal);
          pNew->nRecord += 8;
          break;
        }

        case SQLITE_TEXT: {
          int n = sqlite3_column_bytes(pDflt, iField);
          const char *z = (const char*)sqlite3_column_text(pDflt, iField);
          pNew->nRecord += sessionVarintPut(&pNew->aRecord[pNew->nRecord], n);
          memcpy(&pNew->aRecord[pNew->nRecord], z, n);
          pNew->nRecord += n;
          break;
        }

        case SQLITE_BLOB: {
          int n = sqlite3_column_bytes(pDflt, iField);
          const u8 *z = (const u8*)sqlite3_column_blob(pDflt, iField);
          pNew->nRecord += sessionVarintPut(&pNew->aRecord[pNew->nRecord], n);
          memcpy(&pNew->aRecord[pNew->nRecord], z, n);
          pNew->nRecord += n;
          break;
        }

        default:
          assert( eType==SQLITE_NULL );
          break;
      }

      sessionFree(pSession, pOld);
      *pp = pOld = pNew;
      pNew->nRecordField++;
      pNew->nMaxSize += nIncr;
      if( pSession ){
        pSession->nMaxChangesetSize += nIncr;
      }
    }
  }
}

/*
** Ensure that there is room in the buffer to append nByte bytes of data.
** If not, use sqlite3_realloc() to grow the buffer so that there is.
**
** If successful, return zero. Otherwise, if an OOM condition is encountered,
** set *pRc to SQLITE_NOMEM and return non-zero.
*/
static int sessionBufferGrow(SessionBuffer *p, i64 nByte, int *pRc){
#define SESSION_MAX_BUFFER_SZ (0x7FFFFF00 - 1)
  i64 nReq = p->nBuf + nByte;
  if( *pRc==SQLITE_OK && nReq>p->nAlloc ){
    u8 *aNew;
    i64 nNew = p->nAlloc ? p->nAlloc : 128;

    do {
      nNew = nNew*2;
    }while( nNew<nReq );

    /* The value of SESSION_MAX_BUFFER_SZ is copied from the implementation
    ** of sqlite3_realloc64(). Allocations greater than this size in bytes
    ** always fail. It is used here to ensure that this routine can always
    ** allocate up to this limit - instead of up to the largest power of
    ** two smaller than the limit.  */
    if( nNew>SESSION_MAX_BUFFER_SZ ){
      nNew = SESSION_MAX_BUFFER_SZ;
      if( nNew<nReq ){
        *pRc = SQLITE_NOMEM;
        return 1;
      }
    }

    aNew = (u8 *)sqlite3_realloc64(p->aBuf, nNew);
    if( 0==aNew ){
      *pRc = SQLITE_NOMEM;
    }else{
      p->aBuf = aNew;
      p->nAlloc = nNew;
    }
  }
  return (*pRc!=SQLITE_OK);
}


/*
** This function is a no-op if *pRc is other than SQLITE_OK when it is
** called. Otherwise, append a string to the buffer. All bytes in the string
** up to (but not including) the nul-terminator are written to the buffer.
**
** If an OOM condition is encountered, set *pRc to SQLITE_NOMEM before
** returning.
*/
static void sessionAppendStr(
  SessionBuffer *p,
  const char *zStr,
  int *pRc
){
  int nStr = sqlite3Strlen30(zStr);
  if( 0==sessionBufferGrow(p, nStr+1, pRc) ){
    memcpy(&p->aBuf[p->nBuf], zStr, nStr);
    p->nBuf += nStr;
    p->aBuf[p->nBuf] = 0x00;
  }
}

/*
** Format a string using printf() style formatting and then append it to the
** buffer using sessionAppendString().
*/
static void sessionAppendPrintf(
  SessionBuffer *p,               /* Buffer to append to */
  int *pRc,
  const char *zFmt,
  ...
){
  if( *pRc==SQLITE_OK ){
    char *zApp = 0;
    va_list ap;
    va_start(ap, zFmt);
    zApp = sqlite3_vmprintf(zFmt, ap);
    if( zApp==0 ){
      *pRc = SQLITE_NOMEM;
    }else{
      sessionAppendStr(p, zApp, pRc);
    }
    va_end(ap);
    sqlite3_free(zApp);
  }
}

/*
** Prepare a statement against database handle db that SELECTs a single
** row containing the default values for each column in table pTab. For
** example, if pTab is declared as:
**
**   CREATE TABLE pTab(a PRIMARY KEY, b DEFAULT 123, c DEFAULT 'abcd');
**
** Then this function prepares and returns the SQL statement:
**
**   SELECT NULL, 123, 'abcd';
*/
static int sessionPrepareDfltStmt(
  sqlite3 *db,                    /* Database handle */
  SessionTable *pTab,             /* Table to prepare statement for */
  sqlite3_stmt **ppStmt           /* OUT: Statement handle */
){
  SessionBuffer sql = {0,0,0};
  int rc = SQLITE_OK;
  const char *zSep = " ";
  int ii = 0;

  *ppStmt = 0;
  sessionAppendPrintf(&sql, &rc, "SELECT");
  for(ii=0; ii<pTab->nCol; ii++){
    const char *zDflt = pTab->azDflt[ii] ? pTab->azDflt[ii] : "NULL";
    sessionAppendPrintf(&sql, &rc, "%s%s", zSep, zDflt);
    zSep = ", ";
  }
  if( rc==SQLITE_OK ){
    rc = sqlite3_prepare_v2(db, (const char*)sql.aBuf, -1, ppStmt, 0);
  }
  sqlite3_free(sql.aBuf);

  return rc;
}

/*
** Table pTab has one or more existing change-records with old.* records
** with fewer than pTab->nCol columns. This function updates all such
** change-records with the default values for the missing columns.
*/
static int sessionUpdateChanges(sqlite3_session *pSession, SessionTable *pTab){
  sqlite3_stmt *pStmt = 0;
  int rc = pSession->rc;

  rc = sessionPrepareDfltStmt(pSession->db, pTab, &pStmt);
  if( rc==SQLITE_OK && SQLITE_ROW==sqlite3_step(pStmt) ){
    int ii = 0;
    SessionChange **pp = 0;
    for(ii=0; ii<pTab->nChange; ii++){
      for(pp=&pTab->apChange[ii]; *pp; pp=&((*pp)->pNext)){
        if( (*pp)->nRecordField!=pTab->nCol ){
          sessionUpdateOneChange(pSession, &rc, pp, pTab->nCol, pStmt);
        }
      }
    }
  }

  pSession->rc = rc;
  rc = sqlite3_finalize(pStmt);
  if( pSession->rc==SQLITE_OK ) pSession->rc = rc;
  return pSession->rc;
}

/*
** Versions of the four methods in object SessionHook for use with the
** sqlite_stat1 table. The purpose of this is to substitute a zero-length
** blob each time a NULL value is read from the "idx" column of the
** sqlite_stat1 table.
*/
typedef struct SessionStat1Ctx SessionStat1Ctx;
struct SessionStat1Ctx {
  SessionHook hook;
  sqlite3_session *pSession;
};
static int sessionStat1Old(void *pCtx, int iCol, sqlite3_value **ppVal){
  SessionStat1Ctx *p = (SessionStat1Ctx*)pCtx;
  sqlite3_value *pVal = 0;
  int rc = p->hook.xOld(p->hook.pCtx, iCol, &pVal);
  if( rc==SQLITE_OK && iCol==1 && sqlite3_value_type(pVal)==SQLITE_NULL ){
    pVal = p->pSession->pZeroBlob;
  }
  *ppVal = pVal;
  return rc;
}
static int sessionStat1New(void *pCtx, int iCol, sqlite3_value **ppVal){
  SessionStat1Ctx *p = (SessionStat1Ctx*)pCtx;
  sqlite3_value *pVal = 0;
  int rc = p->hook.xNew(p->hook.pCtx, iCol, &pVal);
  if( rc==SQLITE_OK && iCol==1 && sqlite3_value_type(pVal)==SQLITE_NULL ){
    pVal = p->pSession->pZeroBlob;
  }
  *ppVal = pVal;
  return rc;
}
static int sessionStat1Count(void *pCtx){
  SessionStat1Ctx *p = (SessionStat1Ctx*)pCtx;
  return p->hook.xCount(p->hook.pCtx);
}
static int sessionStat1Depth(void *pCtx){
  SessionStat1Ctx *p = (SessionStat1Ctx*)pCtx;
  return p->hook.xDepth(p->hook.pCtx);
}

static int sessionUpdateMaxSize(
  int op,
  sqlite3_session *pSession,      /* Session object pTab is attached to */
  SessionTable *pTab,             /* Table that change applies to */
  SessionChange *pC               /* Update pC->nMaxSize */
){
  i64 nNew = 2;
  if( pC->op==SQLITE_INSERT ){
    if( pTab->bRowid ) nNew += 9;
    if( op!=SQLITE_DELETE ){
      int ii;
      for(ii=0; ii<pTab->nCol; ii++){
        sqlite3_value *p = 0;
        pSession->hook.xNew(pSession->hook.pCtx, pTab->aiIdx[ii], &p);
        sessionSerializeValue(0, p, &nNew);
      }
    }
  }else if( op==SQLITE_DELETE ){
    nNew += pC->nRecord;
    if( sqlite3_preupdate_blobwrite(pSession->db)>=0 ){
      nNew += pC->nRecord;
    }
  }else{
    int ii;
    u8 *pCsr = pC->aRecord;
    if( pTab->bRowid ){
      nNew += 9 + 1;
      pCsr += 9;
    }
    for(ii=pTab->bRowid; ii<pTab->nCol; ii++){
      int bChanged = 1;
      int nOld = 0;
      int eType;
      int iIdx = pTab->aiIdx[ii];
      sqlite3_value *p = 0;
      pSession->hook.xNew(pSession->hook.pCtx, iIdx, &p);
      if( p==0 ){
        return SQLITE_NOMEM;
      }

      eType = *pCsr++;
      switch( eType ){
        case SQLITE_NULL:
          bChanged = sqlite3_value_type(p)!=SQLITE_NULL;
          break;

        case SQLITE_FLOAT:
        case SQLITE_INTEGER: {
          if( eType==sqlite3_value_type(p) ){
            sqlite3_int64 iVal = sessionGetI64(pCsr);
            if( eType==SQLITE_INTEGER ){
              bChanged = (iVal!=sqlite3_value_int64(p));
            }else{
              double dVal;
              memcpy(&dVal, &iVal, 8);
              bChanged = (dVal!=sqlite3_value_double(p));
            }
          }
          nOld = 8;
          pCsr += 8;
          break;
        }

        default: {
          int nByte;
          nOld = sessionVarintGet(pCsr, &nByte);
          pCsr += nOld;
          nOld += nByte;
          assert( eType==SQLITE_TEXT || eType==SQLITE_BLOB );
          if( eType==sqlite3_value_type(p)
           && nByte==sqlite3_value_bytes(p)
           && (nByte==0 || 0==memcmp(pCsr, sqlite3_value_blob(p), nByte))
          ){
            bChanged = 0;
          }
          pCsr += nByte;
          break;
        }
      }

      if( bChanged && pTab->abPK[ii] ){
        nNew = pC->nRecord + 2;
        break;
      }

      if( bChanged ){
        nNew += 1 + nOld;
        sessionSerializeValue(0, p, &nNew);
      }else if( pTab->abPK[ii] ){
        nNew += 2 + nOld;
      }else{
        nNew += 2;
      }
    }
  }

  if( nNew>pC->nMaxSize ){
    int nIncr = nNew - pC->nMaxSize;
    pC->nMaxSize = nNew;
    pSession->nMaxChangesetSize += nIncr;
  }
  return SQLITE_OK;
}

/*
** This function is only called from with a pre-update-hook reporting a
** change on table pTab (attached to session pSession). The type of change
** (UPDATE, INSERT, DELETE) is specified by the first argument.
**
** Unless one is already present or an error occurs, an entry is added
** to the changed-rows hash table associated with table pTab.
*/
static void sessionPreupdateOneChange(
  int op,                         /* One of SQLITE_UPDATE, INSERT, DELETE */
  i64 iRowid,
  sqlite3_session *pSession,      /* Session object pTab is attached to */
  SessionTable *pTab              /* Table that change applies to */
){
  int iHash;
  int bNull = 0;
  int rc = SQLITE_OK;
  int nExpect = 0;
  SessionStat1Ctx stat1 = {{0,0,0,0,0},0};

  if( pSession->rc ) return;

  /* Load table details if required */
  if( sessionInitTable(pSession, pTab, pSession->db, pSession->zDb) ) return;

  /* Check the number of columns in this xPreUpdate call matches the
  ** number of columns in the table.  */
  nExpect = pSession->hook.xCount(pSession->hook.pCtx);
  if( pTab->nTotalCol<nExpect ){
    if( sessionReinitTable(pSession, pTab) ) return;
    if( sessionUpdateChanges(pSession, pTab) ) return;
  }
  if( pTab->nTotalCol!=nExpect ){
    pSession->rc = SQLITE_SCHEMA;
    return;
  }

  /* Grow the hash table if required */
  if( sessionGrowHash(pSession, 0, pTab) ){
    pSession->rc = SQLITE_NOMEM;
    return;
  }

  if( pTab->bStat1 ){
    stat1.hook = pSession->hook;
    stat1.pSession = pSession;
    pSession->hook.pCtx = (void*)&stat1;
    pSession->hook.xNew = sessionStat1New;
    pSession->hook.xOld = sessionStat1Old;
    pSession->hook.xCount = sessionStat1Count;
    pSession->hook.xDepth = sessionStat1Depth;
    if( pSession->pZeroBlob==0 ){
      sqlite3_value *p = sqlite3ValueNew(0);
      if( p==0 ){
        rc = SQLITE_NOMEM;
        goto error_out;
      }
      sqlite3ValueSetStr(p, 0, "", 0, SQLITE_STATIC);
      pSession->pZeroBlob = p;
    }
  }

  /* Calculate the hash-key for this change. If the primary key of the row
  ** includes a NULL value, exit early. Such changes are ignored by the
  ** session module. */
  rc = sessionPreupdateHash(
      pSession, iRowid, pTab, op==SQLITE_INSERT, &iHash, &bNull
  );
  if( rc!=SQLITE_OK ) goto error_out;

  if( bNull==0 ){
    /* Search the hash table for an existing record for this row. */
    SessionChange *pC;
    for(pC=pTab->apChange[iHash]; pC; pC=pC->pNext){
      if( sessionPreupdateEqual(pSession, iRowid, pTab, pC, op) ) break;
    }

    if( pC==0 ){
      /* Create a new change object containing all the old values (if
      ** this is an SQLITE_UPDATE or SQLITE_DELETE), or just the PK
      ** values (if this is an INSERT). */
      sqlite3_int64 nByte;    /* Number of bytes to allocate */
      int i;                  /* Used to iterate through columns */

      assert( rc==SQLITE_OK );
      pTab->nEntry++;

      /* Figure out how large an allocation is required */
      nByte = sizeof(SessionChange);
      for(i=pTab->bRowid; i<pTab->nCol; i++){
        int iIdx = pTab->aiIdx[i];
        sqlite3_value *p = 0;
        if( op!=SQLITE_INSERT ){
          /* This may fail if the column has a non-NULL default and was added
          ** using ALTER TABLE ADD COLUMN after this record was created. */
          rc = pSession->hook.xOld(pSession->hook.pCtx, iIdx, &p);
        }else if( pTab->abPK[i] ){
          TESTONLY(int trc = ) pSession->hook.xNew(pSession->hook.pCtx,iIdx,&p);
          assert( trc==SQLITE_OK );
        }

        if( rc==SQLITE_OK ){
          /* This may fail if SQLite value p contains a utf-16 string that must
          ** be converted to utf-8 and an OOM error occurs while doing so. */
          rc = sessionSerializeValue(0, p, &nByte);
        }
        if( rc!=SQLITE_OK ) goto error_out;
      }
      if( pTab->bRowid ){
        nByte += 9;               /* Size of rowid field - an integer */
      }

      /* Allocate the change object */
      pC = (SessionChange*)sessionMalloc64(pSession, nByte);
      if( !pC ){
        rc = SQLITE_NOMEM;
        goto error_out;
      }else{
        memset(pC, 0, sizeof(SessionChange));
        pC->aRecord = (u8 *)&pC[1];
      }

      /* Populate the change object. None of the preupdate_old(),
      ** preupdate_new() or SerializeValue() calls below may fail as all
      ** required values and encodings have already been cached in memory.
      ** It is not possible for an OOM to occur in this block. */
      nByte = 0;
      if( pTab->bRowid ){
        pC->aRecord[0] = SQLITE_INTEGER;
        sessionPutI64(&pC->aRecord[1], iRowid);
        nByte = 9;
      }
      for(i=pTab->bRowid; i<pTab->nCol; i++){
        sqlite3_value *p = 0;
        int iIdx = pTab->aiIdx[i];
        if( op!=SQLITE_INSERT ){
          pSession->hook.xOld(pSession->hook.pCtx, iIdx, &p);
        }else if( pTab->abPK[i] ){
          pSession->hook.xNew(pSession->hook.pCtx, iIdx, &p);
        }
        sessionSerializeValue(&pC->aRecord[nByte], p, &nByte);
      }

      /* Add the change to the hash-table */
      if( pSession->bIndirect || pSession->hook.xDepth(pSession->hook.pCtx) ){
        pC->bIndirect = 1;
      }
      pC->nRecordField = pTab->nCol;
      pC->nRecord = nByte;
      pC->op = op;
      pC->pNext = pTab->apChange[iHash];
      pTab->apChange[iHash] = pC;

    }else if( pC->bIndirect ){
      /* If the existing change is considered "indirect", but this current
      ** change is "direct", mark the change object as direct. */
      if( pSession->hook.xDepth(pSession->hook.pCtx)==0
       && pSession->bIndirect==0
      ){
        pC->bIndirect = 0;
      }
    }

    assert( rc==SQLITE_OK );
    if( pSession->bEnableSize ){
      rc = sessionUpdateMaxSize(op, pSession, pTab, pC);
    }
  }


  /* If an error has occurred, mark the session object as failed. */
 error_out:
  if( pTab->bStat1 ){
    pSession->hook = stat1.hook;
  }
  if( rc!=SQLITE_OK ){
    pSession->rc = rc;
  }
}

static int sessionFindTable(
  sqlite3_session *pSession,
  const char *zName,
  SessionTable **ppTab
){
  int rc = SQLITE_OK;
  int nName = sqlite3Strlen30(zName);
  SessionTable *pRet;

  /* Search for an existing table */
  for(pRet=pSession->pTable; pRet; pRet=pRet->pNext){
    if( 0==sqlite3_strnicmp(pRet->zName, zName, nName+1) ) break;
  }

  if( pRet==0 && pSession->bAutoAttach ){
    /* If there is a table-filter configured, invoke it. If it returns 0,
    ** do not automatically add the new table. */
    if( pSession->xTableFilter==0
     || pSession->xTableFilter(pSession->pFilterCtx, zName)
    ){
      rc = sqlite3session_attach(pSession, zName);
      if( rc==SQLITE_OK ){
        pRet = pSession->pTable;
        while( ALWAYS(pRet) && pRet->pNext ){
          pRet = pRet->pNext;
        }
        assert( pRet!=0 );
        assert( 0==sqlite3_strnicmp(pRet->zName, zName, nName+1) );
      }
    }
  }

  assert( rc==SQLITE_OK || pRet==0 );
  *ppTab = pRet;
  return rc;
}

/*
** The 'pre-update' hook registered by this module with SQLite databases.
*/
static void xPreUpdate(
  void *pCtx,                     /* Copy of third arg to preupdate_hook() */
  sqlite3 *db,                    /* Database handle */
  int op,                         /* SQLITE_UPDATE, DELETE or INSERT */
  char const *zDb,                /* Database name */
  char const *zName,              /* Table name */
  sqlite3_int64 iKey1,            /* Rowid of row about to be deleted/updated */
  sqlite3_int64 iKey2             /* New rowid value (for a rowid UPDATE) */
){
  sqlite3_session *pSession;
  int nDb = sqlite3Strlen30(zDb);

  assert( sqlite3_mutex_held(db->mutex) );
  (void)iKey1;
  (void)iKey2;

  for(pSession=(sqlite3_session *)pCtx; pSession; pSession=pSession->pNext){
    SessionTable *pTab;

    /* If this session is attached to a different database ("main", "temp"
    ** etc.), or if it is not currently enabled, there is nothing to do. Skip
    ** to the next session object attached to this database. */
    if( pSession->bEnable==0 ) continue;
    if( pSession->rc ) continue;
    if( sqlite3_strnicmp(zDb, pSession->zDb, nDb+1) ) continue;

    pSession->rc = sessionFindTable(pSession, zName, &pTab);
    if( pTab ){
      assert( pSession->rc==SQLITE_OK );
      assert( op==SQLITE_UPDATE || iKey1==iKey2 );
      sessionPreupdateOneChange(op, iKey1, pSession, pTab);
      if( op==SQLITE_UPDATE ){
        sessionPreupdateOneChange(SQLITE_INSERT, iKey2, pSession, pTab);
      }
    }
  }
}

/*
** The pre-update hook implementations.
*/
static int sessionPreupdateOld(void *pCtx, int iVal, sqlite3_value **ppVal){
  return sqlite3_preupdate_old((sqlite3*)pCtx, iVal, ppVal);
}
static int sessionPreupdateNew(void *pCtx, int iVal, sqlite3_value **ppVal){
  return sqlite3_preupdate_new((sqlite3*)pCtx, iVal, ppVal);
}
static int sessionPreupdateCount(void *pCtx){
  return sqlite3_preupdate_count((sqlite3*)pCtx);
}
static int sessionPreupdateDepth(void *pCtx){
  return sqlite3_preupdate_depth((sqlite3*)pCtx);
}

/*
** Install the pre-update hooks on the session object passed as the only
** argument.
*/
static void sessionPreupdateHooks(
  sqlite3_session *pSession
){
  pSession->hook.pCtx = (void*)pSession->db;
  pSession->hook.xOld = sessionPreupdateOld;
  pSession->hook.xNew = sessionPreupdateNew;
  pSession->hook.xCount = sessionPreupdateCount;
  pSession->hook.xDepth = sessionPreupdateDepth;
}

typedef struct SessionDiffCtx SessionDiffCtx;
struct SessionDiffCtx {
  sqlite3_stmt *pStmt;
  int bRowid;
  int nOldOff;
};

/*
** The diff hook implementations.
*/
static int sessionDiffOld(void *pCtx, int iVal, sqlite3_value **ppVal){
  SessionDiffCtx *p = (SessionDiffCtx*)pCtx;
  *ppVal = sqlite3_column_value(p->pStmt, iVal+p->nOldOff+p->bRowid);
  return SQLITE_OK;
}
static int sessionDiffNew(void *pCtx, int iVal, sqlite3_value **ppVal){
  SessionDiffCtx *p = (SessionDiffCtx*)pCtx;
  *ppVal = sqlite3_column_value(p->pStmt, iVal+p->bRowid);
   return SQLITE_OK;
}
static int sessionDiffCount(void *pCtx){
  SessionDiffCtx *p = (SessionDiffCtx*)pCtx;
  return (p->nOldOff ? p->nOldOff : sqlite3_column_count(p->pStmt)) - p->bRowid;
}
static int sessionDiffDepth(void *pCtx){
  (void)pCtx;
  return 0;
}

/*
** Install the diff hooks on the session object passed as the only
** argument.
*/
static void sessionDiffHooks(
  sqlite3_session *pSession,
  SessionDiffCtx *pDiffCtx
){
  pSession->hook.pCtx = (void*)pDiffCtx;
  pSession->hook.xOld = sessionDiffOld;
  pSession->hook.xNew = sessionDiffNew;
  pSession->hook.xCount = sessionDiffCount;
  pSession->hook.xDepth = sessionDiffDepth;
}

static char *sessionExprComparePK(
  int nCol,
  const char *zDb1, const char *zDb2,
  const char *zTab,
  const char **azCol, u8 *abPK
){
  int i;
  const char *zSep = "";
  char *zRet = 0;

  for(i=0; i<nCol; i++){
    if( abPK[i] ){
      zRet = sqlite3_mprintf("%z%s\"%w\".\"%w\".\"%w\"=\"%w\".\"%w\".\"%w\"",
          zRet, zSep, zDb1, zTab, azCol[i], zDb2, zTab, azCol[i]
      );
      zSep = " AND ";
      if( zRet==0 ) break;
    }
  }

  return zRet;
}

static char *sessionExprCompareOther(
  int nCol,
  const char *zDb1, const char *zDb2,
  const char *zTab,
  const char **azCol, u8 *abPK
){
  int i;
  const char *zSep = "";
  char *zRet = 0;
  int bHave = 0;

  for(i=0; i<nCol; i++){
    if( abPK[i]==0 ){
      bHave = 1;
      zRet = sqlite3_mprintf(
          "%z%s\"%w\".\"%w\".\"%w\" IS NOT \"%w\".\"%w\".\"%w\"",
          zRet, zSep, zDb1, zTab, azCol[i], zDb2, zTab, azCol[i]
      );
      zSep = " OR ";
      if( zRet==0 ) break;
    }
  }

  if( bHave==0 ){
    assert( zRet==0 );
    zRet = sqlite3_mprintf("0");
  }

  return zRet;
}

static char *sessionSelectFindNew(
  const char *zDb1,      /* Pick rows in this db only */
  const char *zDb2,      /* But not in this one */
  int bRowid,
  const char *zTbl,      /* Table name */
  const char *zExpr
){
  const char *zSel = (bRowid ? SESSIONS_ROWID ", *" : "*");
  char *zRet = sqlite3_mprintf(
      "SELECT %s FROM \"%w\".\"%w\" WHERE NOT EXISTS ("
      "  SELECT 1 FROM \"%w\".\"%w\" WHERE %s"
      ")",
      zSel, zDb1, zTbl, zDb2, zTbl, zExpr
  );
  return zRet;
}

static int sessionDiffFindNew(
  int op,
  sqlite3_session *pSession,
  SessionTable *pTab,
  const char *zDb1,
  const char *zDb2,
  char *zExpr
){
  int rc = SQLITE_OK;
  char *zStmt = sessionSelectFindNew(
      zDb1, zDb2, pTab->bRowid, pTab->zName, zExpr
  );

  if( zStmt==0 ){
    rc = SQLITE_NOMEM;
  }else{
    sqlite3_stmt *pStmt;
    rc = sqlite3_prepare(pSession->db, zStmt, -1, &pStmt, 0);
    if( rc==SQLITE_OK ){
      SessionDiffCtx *pDiffCtx = (SessionDiffCtx*)pSession->hook.pCtx;
      pDiffCtx->pStmt = pStmt;
      pDiffCtx->nOldOff = 0;
      pDiffCtx->bRowid = pTab->bRowid;
      while( SQLITE_ROW==sqlite3_step(pStmt) ){
        i64 iRowid = (pTab->bRowid ? sqlite3_column_int64(pStmt, 0) : 0);
        sessionPreupdateOneChange(op, iRowid, pSession, pTab);
      }
      rc = sqlite3_finalize(pStmt);
    }
    sqlite3_free(zStmt);
  }

  return rc;
}

/*
** Return a comma-separated list of the fully-qualified (with both database
** and table name) column names from table pTab. e.g.
**
**    "main"."t1"."a", "main"."t1"."b", "main"."t1"."c"
*/
static char *sessionAllCols(
  const char *zDb,
  SessionTable *pTab
){
  int ii;
  char *zRet = 0;
  for(ii=0; ii<pTab->nCol; ii++){
    zRet = sqlite3_mprintf("%z%s\"%w\".\"%w\".\"%w\"",
        zRet, (zRet ? ", " : ""), zDb, pTab->zName, pTab->azCol[ii]
    );
    if( !zRet ) break;
  }
  return zRet;
}

static int sessionDiffFindModified(
  sqlite3_session *pSession,
  SessionTable *pTab,
  const char *zFrom,
  const char *zExpr
){
  int rc = SQLITE_OK;

  char *zExpr2 = sessionExprCompareOther(pTab->nCol,
      pSession->zDb, zFrom, pTab->zName, pTab->azCol, pTab->abPK
  );
  if( zExpr2==0 ){
    rc = SQLITE_NOMEM;
  }else{
    char *z1 = sessionAllCols(pSession->zDb, pTab);
    char *z2 = sessionAllCols(zFrom, pTab);
    char *zStmt = sqlite3_mprintf(
        "SELECT %s,%s FROM \"%w\".\"%w\", \"%w\".\"%w\" WHERE %s AND (%z)",
        z1, z2, pSession->zDb, pTab->zName, zFrom, pTab->zName, zExpr, zExpr2
    );
    if( zStmt==0 || z1==0 || z2==0 ){
      rc = SQLITE_NOMEM;
    }else{
      sqlite3_stmt *pStmt;
      rc = sqlite3_prepare(pSession->db, zStmt, -1, &pStmt, 0);

      if( rc==SQLITE_OK ){
        SessionDiffCtx *pDiffCtx = (SessionDiffCtx*)pSession->hook.pCtx;
        pDiffCtx->pStmt = pStmt;
        pDiffCtx->nOldOff = pTab->nCol;
        while( SQLITE_ROW==sqlite3_step(pStmt) ){
          i64 iRowid = (pTab->bRowid ? sqlite3_column_int64(pStmt, 0) : 0);
          sessionPreupdateOneChange(SQLITE_UPDATE, iRowid, pSession, pTab);
        }
        rc = sqlite3_finalize(pStmt);
      }
    }
    sqlite3_free(zStmt);
    sqlite3_free(z1);
    sqlite3_free(z2);
  }

  return rc;
}

SQLITE_API int sqlite3session_diff(
  sqlite3_session *pSession,
  const char *zFrom,
  const char *zTbl,
  char **pzErrMsg
){
  const char *zDb = pSession->zDb;
  int rc = pSession->rc;
  SessionDiffCtx d;

  memset(&d, 0, sizeof(d));
  sessionDiffHooks(pSession, &d);

  sqlite3_mutex_enter(sqlite3_db_mutex(pSession->db));
  if( pzErrMsg ) *pzErrMsg = 0;
  if( rc==SQLITE_OK ){
    char *zExpr = 0;
    sqlite3 *db = pSession->db;
    SessionTable *pTo;            /* Table zTbl */

    /* Locate and if necessary initialize the target table object */
    pSession->bAutoAttach++;
    rc = sessionFindTable(pSession, zTbl, &pTo);
    pSession->bAutoAttach--;
    if( pTo==0 ) goto diff_out;
    if( sessionInitTable(pSession, pTo, pSession->db, pSession->zDb) ){
      rc = pSession->rc;
      goto diff_out;
    }

    /* Check the table schemas match */
    if( rc==SQLITE_OK ){
      int bHasPk = 0;
      int bMismatch = 0;
      int nCol = 0;               /* Columns in zFrom.zTbl */
      int bRowid = 0;
      u8 *abPK = 0;
      const char **azCol = 0;
      char *zDbExists = 0;

      /* Check that database zFrom is attached.  */
      zDbExists = sqlite3_mprintf("SELECT * FROM %Q.sqlite_schema", zFrom);
      if( zDbExists==0 ){
        rc = SQLITE_NOMEM;
      }else{
        sqlite3_stmt *pDbExists = 0;
        rc = sqlite3_prepare_v2(db, zDbExists, -1, &pDbExists, 0);
        if( rc==SQLITE_ERROR ){
          rc = SQLITE_OK;
          nCol = -1;
        }
        sqlite3_finalize(pDbExists);
        sqlite3_free(zDbExists);
      }

      if( rc==SQLITE_OK && nCol==0 ){
        rc = sessionTableInfo(0, db, zFrom, zTbl,
            &nCol, 0, 0, &azCol, 0, 0, &abPK,
            pSession->bImplicitPK ? &bRowid : 0
        );
      }
      if( rc==SQLITE_OK ){
        if( pTo->nCol!=nCol ){
          if( nCol<=0 ){
            rc = SQLITE_SCHEMA;
            if( pzErrMsg ){
              *pzErrMsg = sqlite3_mprintf("no such table: %s.%s", zFrom, zTbl);
            }
          }else{
            bMismatch = 1;
          }
        }else{
          int i;
          for(i=0; i<nCol; i++){
            if( pTo->abPK[i]!=abPK[i] ) bMismatch = 1;
            if( sqlite3_stricmp(azCol[i], pTo->azCol[i]) ) bMismatch = 1;
            if( abPK[i] ) bHasPk = 1;
          }
        }
      }
      sqlite3_free((char*)azCol);
      if( bMismatch ){
        if( pzErrMsg ){
          *pzErrMsg = sqlite3_mprintf("table schemas do not match");
        }
        rc = SQLITE_SCHEMA;
      }
      if( bHasPk==0 ){
        /* Ignore tables with no primary keys */
        goto diff_out;
      }
    }

    if( rc==SQLITE_OK ){
      zExpr = sessionExprComparePK(pTo->nCol,
          zDb, zFrom, pTo->zName, pTo->azCol, pTo->abPK
      );
    }

    /* Find new rows */
    if( rc==SQLITE_OK ){
      rc = sessionDiffFindNew(SQLITE_INSERT, pSession, pTo, zDb, zFrom, zExpr);
    }

    /* Find old rows */
    if( rc==SQLITE_OK ){
      rc = sessionDiffFindNew(SQLITE_DELETE, pSession, pTo, zFrom, zDb, zExpr);
    }

    /* Find modified rows */
    if( rc==SQLITE_OK ){
      rc = sessionDiffFindModified(pSession, pTo, zFrom, zExpr);
    }

    sqlite3_free(zExpr);
  }

 diff_out:
  sessionPreupdateHooks(pSession);
  sqlite3_mutex_leave(sqlite3_db_mutex(pSession->db));
  return rc;
}

/*
** Create a session object. This session object will record changes to
** database zDb attached to connection db.
*/
SQLITE_API int sqlite3session_create(
  sqlite3 *db,                    /* Database handle */
  const char *zDb,                /* Name of db (e.g. "main") */
  sqlite3_session **ppSession     /* OUT: New session object */
){
  sqlite3_session *pNew;          /* Newly allocated session object */
  sqlite3_session *pOld;          /* Session object already attached to db */
  int nDb = sqlite3Strlen30(zDb); /* Length of zDb in bytes */

  /* Zero the output value in case an error occurs. */
  *ppSession = 0;

  /* Allocate and populate the new session object. */
  pNew = (sqlite3_session *)sqlite3_malloc64(sizeof(sqlite3_session) + nDb + 1);
  if( !pNew ) return SQLITE_NOMEM;
  memset(pNew, 0, sizeof(sqlite3_session));
  pNew->db = db;
  pNew->zDb = (char *)&pNew[1];
  pNew->bEnable = 1;
  memcpy(pNew->zDb, zDb, nDb+1);
  sessionPreupdateHooks(pNew);

  /* Add the new session object to the linked list of session objects
  ** attached to database handle $db. Do this under the cover of the db
  ** handle mutex.  */
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  pOld = (sqlite3_session*)sqlite3_preupdate_hook(db, xPreUpdate, (void*)pNew);
  pNew->pNext = pOld;
  sqlite3_mutex_leave(sqlite3_db_mutex(db));

  *ppSession = pNew;
  return SQLITE_OK;
}

/*
** Free the list of table objects passed as the first argument. The contents
** of the changed-rows hash tables are also deleted.
*/
static void sessionDeleteTable(sqlite3_session *pSession, SessionTable *pList){
  SessionTable *pNext;
  SessionTable *pTab;

  for(pTab=pList; pTab; pTab=pNext){
    int i;
    pNext = pTab->pNext;
    for(i=0; i<pTab->nChange; i++){
      SessionChange *p;
      SessionChange *pNextChange;
      for(p=pTab->apChange[i]; p; p=pNextChange){
        pNextChange = p->pNext;
        sessionFree(pSession, p);
      }
    }
    sqlite3_finalize(pTab->pDfltStmt);
    sessionFree(pSession, (char*)pTab->azCol);  /* cast works around VC++ bug */
    sessionFree(pSession, pTab->apChange);
    sessionFree(pSession, pTab);
  }
}

/*
** Delete a session object previously allocated using sqlite3session_create().
*/
SQLITE_API void sqlite3session_delete(sqlite3_session *pSession){
  sqlite3 *db = pSession->db;
  sqlite3_session *pHead;
  sqlite3_session **pp;

  /* Unlink the session from the linked list of sessions attached to the
  ** database handle. Hold the db mutex while doing so.  */
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  pHead = (sqlite3_session*)sqlite3_preupdate_hook(db, 0, 0);
  for(pp=&pHead; ALWAYS((*pp)!=0); pp=&((*pp)->pNext)){
    if( (*pp)==pSession ){
      *pp = (*pp)->pNext;
      if( pHead ) sqlite3_preupdate_hook(db, xPreUpdate, (void*)pHead);
      break;
    }
  }
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  sqlite3ValueFree(pSession->pZeroBlob);

  /* Delete all attached table objects. And the contents of their
  ** associated hash-tables. */
  sessionDeleteTable(pSession, pSession->pTable);

  /* Free the session object. */
  sqlite3_free(pSession);
}

/*
** Set a table filter on a Session Object.
*/
SQLITE_API void sqlite3session_table_filter(
  sqlite3_session *pSession,
  int(*xFilter)(void*, const char*),
  void *pCtx                      /* First argument passed to xFilter */
){
  pSession->bAutoAttach = 1;
  pSession->pFilterCtx = pCtx;
  pSession->xTableFilter = xFilter;
}

/*
** Attach a table to a session. All subsequent changes made to the table
** while the session object is enabled will be recorded.
**
** Only tables that have a PRIMARY KEY defined may be attached. It does
** not matter if the PRIMARY KEY is an "INTEGER PRIMARY KEY" (rowid alias)
** or not.
*/
SQLITE_API int sqlite3session_attach(
  sqlite3_session *pSession,      /* Session object */
  const char *zName               /* Table name */
){
  int rc = SQLITE_OK;
  sqlite3_mutex_enter(sqlite3_db_mutex(pSession->db));

  if( !zName ){
    pSession->bAutoAttach = 1;
  }else{
    SessionTable *pTab;           /* New table object (if required) */
    int nName;                    /* Number of bytes in string zName */

    /* First search for an existing entry. If one is found, this call is
    ** a no-op. Return early. */
    nName = sqlite3Strlen30(zName);
    for(pTab=pSession->pTable; pTab; pTab=pTab->pNext){
      if( 0==sqlite3_strnicmp(pTab->zName, zName, nName+1) ) break;
    }

    if( !pTab ){
      /* Allocate new SessionTable object. */
      int nByte = sizeof(SessionTable) + nName + 1;
      pTab = (SessionTable*)sessionMalloc64(pSession, nByte);
      if( !pTab ){
        rc = SQLITE_NOMEM;
      }else{
        /* Populate the new SessionTable object and link it into the list.
        ** The new object must be linked onto the end of the list, not
        ** simply added to the start of it in order to ensure that tables
        ** appear in the correct order when a changeset or patchset is
        ** eventually generated. */
        SessionTable **ppTab;
        memset(pTab, 0, sizeof(SessionTable));
        pTab->zName = (char *)&pTab[1];
        memcpy(pTab->zName, zName, nName+1);
        for(ppTab=&pSession->pTable; *ppTab; ppTab=&(*ppTab)->pNext);
        *ppTab = pTab;
      }
    }
  }

  sqlite3_mutex_leave(sqlite3_db_mutex(pSession->db));
  return rc;
}

/*
** Append the value passed as the second argument to the buffer passed
** as the first.
**
** This function is a no-op if *pRc is non-zero when it is called.
** Otherwise, if an error occurs, *pRc is set to an SQLite error code
** before returning.
*/
static void sessionAppendValue(SessionBuffer *p, sqlite3_value *pVal, int *pRc){
  int rc = *pRc;
  if( rc==SQLITE_OK ){
    sqlite3_int64 nByte = 0;
    rc = sessionSerializeValue(0, pVal, &nByte);
    sessionBufferGrow(p, nByte, &rc);
    if( rc==SQLITE_OK ){
      rc = sessionSerializeValue(&p->aBuf[p->nBuf], pVal, 0);
      p->nBuf += nByte;
    }else{
      *pRc = rc;
    }
  }
}

/*
** This function is a no-op if *pRc is other than SQLITE_OK when it is
** called. Otherwise, append a single byte to the buffer.
**
** If an OOM condition is encountered, set *pRc to SQLITE_NOMEM before
** returning.
*/
static void sessionAppendByte(SessionBuffer *p, u8 v, int *pRc){
  if( 0==sessionBufferGrow(p, 1, pRc) ){
    p->aBuf[p->nBuf++] = v;
  }
}

/*
** This function is a no-op if *pRc is other than SQLITE_OK when it is
** called. Otherwise, append a single varint to the buffer.
**
** If an OOM condition is encountered, set *pRc to SQLITE_NOMEM before
** returning.
*/
static void sessionAppendVarint(SessionBuffer *p, int v, int *pRc){
  if( 0==sessionBufferGrow(p, 9, pRc) ){
    p->nBuf += sessionVarintPut(&p->aBuf[p->nBuf], v);
  }
}

/*
** This function is a no-op if *pRc is other than SQLITE_OK when it is
** called. Otherwise, append a blob of data to the buffer.
**
** If an OOM condition is encountered, set *pRc to SQLITE_NOMEM before
** returning.
*/
static void sessionAppendBlob(
  SessionBuffer *p,
  const u8 *aBlob,
  int nBlob,
  int *pRc
){
  if( nBlob>0 && 0==sessionBufferGrow(p, nBlob, pRc) ){
    memcpy(&p->aBuf[p->nBuf], aBlob, nBlob);
    p->nBuf += nBlob;
  }
}

/*
** This function is a no-op if *pRc is other than SQLITE_OK when it is
** called. Otherwise, append the string representation of integer iVal
** to the buffer. No nul-terminator is written.
**
** If an OOM condition is encountered, set *pRc to SQLITE_NOMEM before
** returning.
*/
static void sessionAppendInteger(
  SessionBuffer *p,               /* Buffer to append to */
  int iVal,                       /* Value to write the string rep. of */
  int *pRc                        /* IN/OUT: Error code */
){
  char aBuf[24];
  sqlite3_snprintf(sizeof(aBuf)-1, aBuf, "%d", iVal);
  sessionAppendStr(p, aBuf, pRc);
}

/*
** This function is a no-op if *pRc is other than SQLITE_OK when it is
** called. Otherwise, append the string zStr enclosed in quotes (") and
** with any embedded quote characters escaped to the buffer. No
** nul-terminator byte is written.
**
** If an OOM condition is encountered, set *pRc to SQLITE_NOMEM before
** returning.
*/
static void sessionAppendIdent(
  SessionBuffer *p,               /* Buffer to a append to */
  const char *zStr,               /* String to quote, escape and append */
  int *pRc                        /* IN/OUT: Error code */
){
  int nStr = sqlite3Strlen30(zStr)*2 + 2 + 2;
  if( 0==sessionBufferGrow(p, nStr, pRc) ){
    char *zOut = (char *)&p->aBuf[p->nBuf];
    const char *zIn = zStr;
    *zOut++ = '"';
    if( zIn!=0 ){
      while( *zIn ){
        if( *zIn=='"' ) *zOut++ = '"';
        *zOut++ = *(zIn++);
      }
    }
    *zOut++ = '"';
    p->nBuf = (int)((u8 *)zOut - p->aBuf);
    p->aBuf[p->nBuf] = 0x00;
  }
}

/*
** This function is a no-op if *pRc is other than SQLITE_OK when it is
** called. Otherwse, it appends the serialized version of the value stored
** in column iCol of the row that SQL statement pStmt currently points
** to to the buffer.
*/
static void sessionAppendCol(
  SessionBuffer *p,               /* Buffer to append to */
  sqlite3_stmt *pStmt,            /* Handle pointing to row containing value */
  int iCol,                       /* Column to read value from */
  int *pRc                        /* IN/OUT: Error code */
){
  if( *pRc==SQLITE_OK ){
    int eType = sqlite3_column_type(pStmt, iCol);
    sessionAppendByte(p, (u8)eType, pRc);
    if( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT ){
      u8 aBuf[8];
      if( eType==SQLITE_INTEGER ){
        sqlite3_int64 i = sqlite3_column_int64(pStmt, iCol);
        sessionPutI64(aBuf, i);
      }else{
        double r = sqlite3_column_double(pStmt, iCol);
        sessionPutDouble(aBuf, r);
      }
      sessionAppendBlob(p, aBuf, 8, pRc);
    }
    if( eType==SQLITE_BLOB || eType==SQLITE_TEXT ){
      u8 *z;
      int nByte;
      if( eType==SQLITE_BLOB ){
        z = (u8 *)sqlite3_column_blob(pStmt, iCol);
      }else{
        z = (u8 *)sqlite3_column_text(pStmt, iCol);
      }
      nByte = sqlite3_column_bytes(pStmt, iCol);
      if( z || (eType==SQLITE_BLOB && nByte==0) ){
        sessionAppendVarint(p, nByte, pRc);
        sessionAppendBlob(p, z, nByte, pRc);
      }else{
        *pRc = SQLITE_NOMEM;
      }
    }
  }
}

/*
**
** This function appends an update change to the buffer (see the comments
** under "CHANGESET FORMAT" at the top of the file). An update change
** consists of:
**
**   1 byte:  SQLITE_UPDATE (0x17)
**   n bytes: old.* record (see RECORD FORMAT)
**   m bytes: new.* record (see RECORD FORMAT)
**
** The SessionChange object passed as the third argument contains the
** values that were stored in the row when the session began (the old.*
** values). The statement handle passed as the second argument points
** at the current version of the row (the new.* values).
**
** If all of the old.* values are equal to their corresponding new.* value
** (i.e. nothing has changed), then no data at all is appended to the buffer.
**
** Otherwise, the old.* record contains all primary key values and the
** original values of any fields that have been modified. The new.* record
** contains the new values of only those fields that have been modified.
*/
static int sessionAppendUpdate(
  SessionBuffer *pBuf,            /* Buffer to append to */
  int bPatchset,                  /* True for "patchset", 0 for "changeset" */
  sqlite3_stmt *pStmt,            /* Statement handle pointing at new row */
  SessionChange *p,               /* Object containing old values */
  u8 *abPK                        /* Boolean array - true for PK columns */
){
  int rc = SQLITE_OK;
  SessionBuffer buf2 = {0,0,0}; /* Buffer to accumulate new.* record in */
  int bNoop = 1;                /* Set to zero if any values are modified */
  int nRewind = pBuf->nBuf;     /* Set to zero if any values are modified */
  int i;                        /* Used to iterate through columns */
  u8 *pCsr = p->aRecord;        /* Used to iterate through old.* values */

  assert( abPK!=0 );
  sessionAppendByte(pBuf, SQLITE_UPDATE, &rc);
  sessionAppendByte(pBuf, p->bIndirect, &rc);
  for(i=0; i<sqlite3_column_count(pStmt); i++){
    int bChanged = 0;
    int nAdvance;
    int eType = *pCsr;
    switch( eType ){
      case SQLITE_NULL:
        nAdvance = 1;
        if( sqlite3_column_type(pStmt, i)!=SQLITE_NULL ){
          bChanged = 1;
        }
        break;

      case SQLITE_FLOAT:
      case SQLITE_INTEGER: {
        nAdvance = 9;
        if( eType==sqlite3_column_type(pStmt, i) ){
          sqlite3_int64 iVal = sessionGetI64(&pCsr[1]);
          if( eType==SQLITE_INTEGER ){
            if( iVal==sqlite3_column_int64(pStmt, i) ) break;
          }else{
            double dVal;
            memcpy(&dVal, &iVal, 8);
            if( dVal==sqlite3_column_double(pStmt, i) ) break;
          }
        }
        bChanged = 1;
        break;
      }

      default: {
        int n;
        int nHdr = 1 + sessionVarintGet(&pCsr[1], &n);
        assert( eType==SQLITE_TEXT || eType==SQLITE_BLOB );
        nAdvance = nHdr + n;
        if( eType==sqlite3_column_type(pStmt, i)
         && n==sqlite3_column_bytes(pStmt, i)
         && (n==0 || 0==memcmp(&pCsr[nHdr], sqlite3_column_blob(pStmt, i), n))
        ){
          break;
        }
        bChanged = 1;
      }
    }

    /* If at least one field has been modified, this is not a no-op. */
    if( bChanged ) bNoop = 0;

    /* Add a field to the old.* record. This is omitted if this module is
    ** currently generating a patchset. */
    if( bPatchset==0 ){
      if( bChanged || abPK[i] ){
        sessionAppendBlob(pBuf, pCsr, nAdvance, &rc);
      }else{
        sessionAppendByte(pBuf, 0, &rc);
      }
    }

    /* Add a field to the new.* record. Or the only record if currently
    ** generating a patchset.  */
    if( bChanged || (bPatchset && abPK[i]) ){
      sessionAppendCol(&buf2, pStmt, i, &rc);
    }else{
      sessionAppendByte(&buf2, 0, &rc);
    }

    pCsr += nAdvance;
  }

  if( bNoop ){
    pBuf->nBuf = nRewind;
  }else{
    sessionAppendBlob(pBuf, buf2.aBuf, buf2.nBuf, &rc);
  }
  sqlite3_free(buf2.aBuf);

  return rc;
}

/*
** Append a DELETE change to the buffer passed as the first argument. Use
** the changeset format if argument bPatchset is zero, or the patchset
** format otherwise.
*/
static int sessionAppendDelete(
  SessionBuffer *pBuf,            /* Buffer to append to */
  int bPatchset,                  /* True for "patchset", 0 for "changeset" */
  SessionChange *p,               /* Object containing old values */
  int nCol,                       /* Number of columns in table */
  u8 *abPK                        /* Boolean array - true for PK columns */
){
  int rc = SQLITE_OK;

  sessionAppendByte(pBuf, SQLITE_DELETE, &rc);
  sessionAppendByte(pBuf, p->bIndirect, &rc);

  if( bPatchset==0 ){
    sessionAppendBlob(pBuf, p->aRecord, p->nRecord, &rc);
  }else{
    int i;
    u8 *a = p->aRecord;
    for(i=0; i<nCol; i++){
      u8 *pStart = a;
      int eType = *a++;

      switch( eType ){
        case 0:
        case SQLITE_NULL:
          assert( abPK[i]==0 );
          break;

        case SQLITE_FLOAT:
        case SQLITE_INTEGER:
          a += 8;
          break;

        default: {
          int n;
          a += sessionVarintGet(a, &n);
          a += n;
          break;
        }
      }
      if( abPK[i] ){
        sessionAppendBlob(pBuf, pStart, (int)(a-pStart), &rc);
      }
    }
    assert( (a - p->aRecord)==p->nRecord );
  }

  return rc;
}

static int sessionPrepare(
  sqlite3 *db,
  sqlite3_stmt **pp,
  char **pzErrmsg,
  const char *zSql
){
  int rc = sqlite3_prepare_v2(db, zSql, -1, pp, 0);
  if( pzErrmsg && rc!=SQLITE_OK ){
    *pzErrmsg = sqlite3_mprintf("%s", sqlite3_errmsg(db));
  }
  return rc;
}

/*
** Formulate and prepare a SELECT statement to retrieve a row from table
** zTab in database zDb based on its primary key. i.e.
**
**   SELECT *, <noop-test> FROM zDb.zTab WHERE (pk1, pk2,...) IS (?1, ?2,...)
**
** where <noop-test> is:
**
**   1 AND (?A OR ?1 IS <column>) AND ...
**
** for each non-pk <column>.
*/
static int sessionSelectStmt(
  sqlite3 *db,                    /* Database handle */
  int bIgnoreNoop,
  const char *zDb,                /* Database name */
  const char *zTab,               /* Table name */
  int bRowid,
  int nCol,                       /* Number of columns in table */
  const char **azCol,             /* Names of table columns */
  u8 *abPK,                       /* PRIMARY KEY  array */
  sqlite3_stmt **ppStmt,          /* OUT: Prepared SELECT statement */
  char **pzErrmsg                 /* OUT: Error message */
){
  int rc = SQLITE_OK;
  char *zSql = 0;
  const char *zSep = "";
  int i;

  SessionBuffer cols = {0, 0, 0};
  SessionBuffer nooptest = {0, 0, 0};
  SessionBuffer pkfield = {0, 0, 0};
  SessionBuffer pkvar = {0, 0, 0};

  sessionAppendStr(&nooptest, ", 1", &rc);

  if( 0==sqlite3_stricmp("sqlite_stat1", zTab) ){
    sessionAppendStr(&nooptest, " AND (?6 OR ?3 IS stat)", &rc);
    sessionAppendStr(&pkfield, "tbl, idx", &rc);
    sessionAppendStr(&pkvar,
        "?1, (CASE WHEN ?2=X'' THEN NULL ELSE ?2 END)", &rc
    );
    sessionAppendStr(&cols, "tbl, ?2, stat", &rc);
  }else{
  #if 0
    if( bRowid ){
      sessionAppendStr(&cols, SESSIONS_ROWID, &rc);
    }
    #endif
    for(i=0; i<nCol; i++){
      if( cols.nBuf ) sessionAppendStr(&cols, ", ", &rc);
      sessionAppendIdent(&cols, azCol[i], &rc);
      if( abPK[i] ){
        sessionAppendStr(&pkfield, zSep, &rc);
        sessionAppendStr(&pkvar, zSep, &rc);
        zSep = ", ";
        sessionAppendIdent(&pkfield, azCol[i], &rc);
        sessionAppendPrintf(&pkvar, &rc, "?%d", i+1);
      }else{
        sessionAppendPrintf(&nooptest, &rc,
            " AND (?%d OR ?%d IS %w.%w)", i+1+nCol, i+1, zTab, azCol[i]
        );
      }
    }
  }

  if( rc==SQLITE_OK ){
    zSql = sqlite3_mprintf(
        "SELECT %s%s FROM %Q.%Q WHERE (%s) IS (%s)",
        (char*)cols.aBuf, (bIgnoreNoop ? (char*)nooptest.aBuf : ""),
        zDb, zTab, (char*)pkfield.aBuf, (char*)pkvar.aBuf
    );
    if( zSql==0 ) rc = SQLITE_NOMEM;
  }

#if 0
  if( 0==sqlite3_stricmp("sqlite_stat1", zTab) ){
    zSql = sqlite3_mprintf(
        "SELECT tbl, ?2, stat FROM %Q.sqlite_stat1 WHERE tbl IS ?1 AND "
        "idx IS (CASE WHEN ?2=X'' THEN NULL ELSE ?2 END)", zDb
    );
    if( zSql==0 ) rc = SQLITE_NOMEM;
  }else{
    const char *zSep = "";
    SessionBuffer buf = {0, 0, 0};

    sessionAppendStr(&buf, "SELECT * FROM ", &rc);
    sessionAppendIdent(&buf, zDb, &rc);
    sessionAppendStr(&buf, ".", &rc);
    sessionAppendIdent(&buf, zTab, &rc);
    sessionAppendStr(&buf, " WHERE ", &rc);
    for(i=0; i<nCol; i++){
      if( abPK[i] ){
        sessionAppendStr(&buf, zSep, &rc);
        sessionAppendIdent(&buf, azCol[i], &rc);
        sessionAppendStr(&buf, " IS ?", &rc);
        sessionAppendInteger(&buf, i+1, &rc);
        zSep = " AND ";
      }
    }
    zSql = (char*)buf.aBuf;
    nSql = buf.nBuf;
  }
#endif

  if( rc==SQLITE_OK ){
    rc = sessionPrepare(db, ppStmt, pzErrmsg, zSql);
  }
  sqlite3_free(zSql);
  sqlite3_free(nooptest.aBuf);
  sqlite3_free(pkfield.aBuf);
  sqlite3_free(pkvar.aBuf);
  sqlite3_free(cols.aBuf);
  return rc;
}

/*
** Bind the PRIMARY KEY values from the change passed in argument pChange
** to the SELECT statement passed as the first argument. The SELECT statement
** is as prepared by function sessionSelectStmt().
**
** Return SQLITE_OK if all PK values are successfully bound, or an SQLite
** error code (e.g. SQLITE_NOMEM) otherwise.
*/
static int sessionSelectBind(
  sqlite3_stmt *pSelect,          /* SELECT from sessionSelectStmt() */
  int nCol,                       /* Number of columns in table */
  u8 *abPK,                       /* PRIMARY KEY array */
  SessionChange *pChange          /* Change structure */
){
  int i;
  int rc = SQLITE_OK;
  u8 *a = pChange->aRecord;

  for(i=0; i<nCol && rc==SQLITE_OK; i++){
    int eType = *a++;

    switch( eType ){
      case 0:
      case SQLITE_NULL:
        assert( abPK[i]==0 );
        break;

      case SQLITE_INTEGER: {
        if( abPK[i] ){
          i64 iVal = sessionGetI64(a);
          rc = sqlite3_bind_int64(pSelect, i+1, iVal);
        }
        a += 8;
        break;
      }

      case SQLITE_FLOAT: {
        if( abPK[i] ){
          double rVal;
          i64 iVal = sessionGetI64(a);
          memcpy(&rVal, &iVal, 8);
          rc = sqlite3_bind_double(pSelect, i+1, rVal);
        }
        a += 8;
        break;
      }

      case SQLITE_TEXT: {
        int n;
        a += sessionVarintGet(a, &n);
        if( abPK[i] ){
          rc = sqlite3_bind_text(pSelect, i+1, (char *)a, n, SQLITE_TRANSIENT);
        }
        a += n;
        break;
      }

      default: {
        int n;
        assert( eType==SQLITE_BLOB );
        a += sessionVarintGet(a, &n);
        if( abPK[i] ){
          rc = sqlite3_bind_blob(pSelect, i+1, a, n, SQLITE_TRANSIENT);
        }
        a += n;
        break;
      }
    }
  }

  return rc;
}

/*
** This function is a no-op if *pRc is set to other than SQLITE_OK when it
** is called. Otherwise, append a serialized table header (part of the binary
** changeset format) to buffer *pBuf. If an error occurs, set *pRc to an
** SQLite error code before returning.
*/
static void sessionAppendTableHdr(
  SessionBuffer *pBuf,            /* Append header to this buffer */
  int bPatchset,                  /* Use the patchset format if true */
  SessionTable *pTab,             /* Table object to append header for */
  int *pRc                        /* IN/OUT: Error code */
){
  /* Write a table header */
  sessionAppendByte(pBuf, (bPatchset ? 'P' : 'T'), pRc);
  sessionAppendVarint(pBuf, pTab->nCol, pRc);
  sessionAppendBlob(pBuf, pTab->abPK, pTab->nCol, pRc);
  sessionAppendBlob(pBuf, (u8 *)pTab->zName, (int)strlen(pTab->zName)+1, pRc);
}

/*
** Generate either a changeset (if argument bPatchset is zero) or a patchset
** (if it is non-zero) based on the current contents of the session object
** passed as the first argument.
**
** If no error occurs, SQLITE_OK is returned and the new changeset/patchset
** stored in output variables *pnChangeset and *ppChangeset. Or, if an error
** occurs, an SQLite error code is returned and both output variables set
** to 0.
*/
static int sessionGenerateChangeset(
  sqlite3_session *pSession,      /* Session object */
  int bPatchset,                  /* True for patchset, false for changeset */
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut,                     /* First argument for xOutput */
  int *pnChangeset,               /* OUT: Size of buffer at *ppChangeset */
  void **ppChangeset              /* OUT: Buffer containing changeset */
){
  sqlite3 *db = pSession->db;     /* Source database handle */
  SessionTable *pTab;             /* Used to iterate through attached tables */
  SessionBuffer buf = {0,0,0};    /* Buffer in which to accumulate changeset */
  int rc;                         /* Return code */

  assert( xOutput==0 || (pnChangeset==0 && ppChangeset==0) );
  assert( xOutput!=0 || (pnChangeset!=0 && ppChangeset!=0) );

  /* Zero the output variables in case an error occurs. If this session
  ** object is already in the error state (sqlite3_session.rc != SQLITE_OK),
  ** this call will be a no-op.  */
  if( xOutput==0 ){
    assert( pnChangeset!=0  && ppChangeset!=0 );
    *pnChangeset = 0;
    *ppChangeset = 0;
  }

  if( pSession->rc ) return pSession->rc;

  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  rc = sqlite3_exec(pSession->db, "SAVEPOINT changeset", 0, 0, 0);
  if( rc!=SQLITE_OK ){
    sqlite3_mutex_leave(sqlite3_db_mutex(db));
    return rc;
  }

  for(pTab=pSession->pTable; rc==SQLITE_OK && pTab; pTab=pTab->pNext){
    if( pTab->nEntry ){
      const char *zName = pTab->zName;
      int i;                      /* Used to iterate through hash buckets */
      sqlite3_stmt *pSel = 0;     /* SELECT statement to query table pTab */
      int nRewind = buf.nBuf;     /* Initial size of write buffer */
      int nNoop;                  /* Size of buffer after writing tbl header */
      int nOldCol = pTab->nCol;

      /* Check the table schema is still Ok. */
      rc = sessionReinitTable(pSession, pTab);
      if( rc==SQLITE_OK && pTab->nCol!=nOldCol ){
        rc = sessionUpdateChanges(pSession, pTab);
      }

      /* Write a table header */
      sessionAppendTableHdr(&buf, bPatchset, pTab, &rc);

      /* Build and compile a statement to execute: */
      if( rc==SQLITE_OK ){
        rc = sessionSelectStmt(db, 0, pSession->zDb,
            zName, pTab->bRowid, pTab->nCol, pTab->azCol, pTab->abPK, &pSel, 0
        );
      }

      nNoop = buf.nBuf;
      for(i=0; i<pTab->nChange && rc==SQLITE_OK; i++){
        SessionChange *p;         /* Used to iterate through changes */

        for(p=pTab->apChange[i]; rc==SQLITE_OK && p; p=p->pNext){
          rc = sessionSelectBind(pSel, pTab->nCol, pTab->abPK, p);
          if( rc!=SQLITE_OK ) continue;
          if( sqlite3_step(pSel)==SQLITE_ROW ){
            if( p->op==SQLITE_INSERT ){
              int iCol;
              sessionAppendByte(&buf, SQLITE_INSERT, &rc);
              sessionAppendByte(&buf, p->bIndirect, &rc);
              for(iCol=0; iCol<pTab->nCol; iCol++){
                sessionAppendCol(&buf, pSel, iCol, &rc);
              }
            }else{
              assert( pTab->abPK!=0 );
              rc = sessionAppendUpdate(&buf, bPatchset, pSel, p, pTab->abPK);
            }
          }else if( p->op!=SQLITE_INSERT ){
            rc = sessionAppendDelete(&buf, bPatchset, p, pTab->nCol,pTab->abPK);
          }
          if( rc==SQLITE_OK ){
            rc = sqlite3_reset(pSel);
          }

          /* If the buffer is now larger than sessions_strm_chunk_size, pass
          ** its contents to the xOutput() callback. */
          if( xOutput
           && rc==SQLITE_OK
           && buf.nBuf>nNoop
           && buf.nBuf>sessions_strm_chunk_size
          ){
            rc = xOutput(pOut, (void*)buf.aBuf, buf.nBuf);
            nNoop = -1;
            buf.nBuf = 0;
          }

        }
      }

      sqlite3_finalize(pSel);
      if( buf.nBuf==nNoop ){
        buf.nBuf = nRewind;
      }
    }
  }

  if( rc==SQLITE_OK ){
    if( xOutput==0 ){
      *pnChangeset = buf.nBuf;
      *ppChangeset = buf.aBuf;
      buf.aBuf = 0;
    }else if( buf.nBuf>0 ){
      rc = xOutput(pOut, (void*)buf.aBuf, buf.nBuf);
    }
  }

  sqlite3_free(buf.aBuf);
  sqlite3_exec(db, "RELEASE changeset", 0, 0, 0);
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  return rc;
}

/*
** Obtain a changeset object containing all changes recorded by the
** session object passed as the first argument.
**
** It is the responsibility of the caller to eventually free the buffer
** using sqlite3_free().
*/
SQLITE_API int sqlite3session_changeset(
  sqlite3_session *pSession,      /* Session object */
  int *pnChangeset,               /* OUT: Size of buffer at *ppChangeset */
  void **ppChangeset              /* OUT: Buffer containing changeset */
){
  int rc;

  if( pnChangeset==0 || ppChangeset==0 ) return SQLITE_MISUSE;
  rc = sessionGenerateChangeset(pSession, 0, 0, 0, pnChangeset, ppChangeset);
  assert( rc || pnChangeset==0
       || pSession->bEnableSize==0 || *pnChangeset<=pSession->nMaxChangesetSize
  );
  return rc;
}

/*
** Streaming version of sqlite3session_changeset().
*/
SQLITE_API int sqlite3session_changeset_strm(
  sqlite3_session *pSession,
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut
){
  if( xOutput==0 ) return SQLITE_MISUSE;
  return sessionGenerateChangeset(pSession, 0, xOutput, pOut, 0, 0);
}

/*
** Streaming version of sqlite3session_patchset().
*/
SQLITE_API int sqlite3session_patchset_strm(
  sqlite3_session *pSession,
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut
){
  if( xOutput==0 ) return SQLITE_MISUSE;
  return sessionGenerateChangeset(pSession, 1, xOutput, pOut, 0, 0);
}

/*
** Obtain a patchset object containing all changes recorded by the
** session object passed as the first argument.
**
** It is the responsibility of the caller to eventually free the buffer
** using sqlite3_free().
*/
SQLITE_API int sqlite3session_patchset(
  sqlite3_session *pSession,      /* Session object */
  int *pnPatchset,                /* OUT: Size of buffer at *ppChangeset */
  void **ppPatchset               /* OUT: Buffer containing changeset */
){
  if( pnPatchset==0 || ppPatchset==0 ) return SQLITE_MISUSE;
  return sessionGenerateChangeset(pSession, 1, 0, 0, pnPatchset, ppPatchset);
}

/*
** Enable or disable the session object passed as the first argument.
*/
SQLITE_API int sqlite3session_enable(sqlite3_session *pSession, int bEnable){
  int ret;
  sqlite3_mutex_enter(sqlite3_db_mutex(pSession->db));
  if( bEnable>=0 ){
    pSession->bEnable = bEnable;
  }
  ret = pSession->bEnable;
  sqlite3_mutex_leave(sqlite3_db_mutex(pSession->db));
  return ret;
}

/*
** Enable or disable the session object passed as the first argument.
*/
SQLITE_API int sqlite3session_indirect(sqlite3_session *pSession, int bIndirect){
  int ret;
  sqlite3_mutex_enter(sqlite3_db_mutex(pSession->db));
  if( bIndirect>=0 ){
    pSession->bIndirect = bIndirect;
  }
  ret = pSession->bIndirect;
  sqlite3_mutex_leave(sqlite3_db_mutex(pSession->db));
  return ret;
}

/*
** Return true if there have been no changes to monitored tables recorded
** by the session object passed as the only argument.
*/
SQLITE_API int sqlite3session_isempty(sqlite3_session *pSession){
  int ret = 0;
  SessionTable *pTab;

  sqlite3_mutex_enter(sqlite3_db_mutex(pSession->db));
  for(pTab=pSession->pTable; pTab && ret==0; pTab=pTab->pNext){
    ret = (pTab->nEntry>0);
  }
  sqlite3_mutex_leave(sqlite3_db_mutex(pSession->db));

  return (ret==0);
}

/*
** Return the amount of heap memory in use.
*/
SQLITE_API sqlite3_int64 sqlite3session_memory_used(sqlite3_session *pSession){
  return pSession->nMalloc;
}

/*
** Configure the session object passed as the first argument.
*/
SQLITE_API int sqlite3session_object_config(sqlite3_session *pSession, int op, void *pArg){
  int rc = SQLITE_OK;
  switch( op ){
    case SQLITE_SESSION_OBJCONFIG_SIZE: {
      int iArg = *(int*)pArg;
      if( iArg>=0 ){
        if( pSession->pTable ){
          rc = SQLITE_MISUSE;
        }else{
          pSession->bEnableSize = (iArg!=0);
        }
      }
      *(int*)pArg = pSession->bEnableSize;
      break;
    }

    case SQLITE_SESSION_OBJCONFIG_ROWID: {
      int iArg = *(int*)pArg;
      if( iArg>=0 ){
        if( pSession->pTable ){
          rc = SQLITE_MISUSE;
        }else{
          pSession->bImplicitPK = (iArg!=0);
        }
      }
      *(int*)pArg = pSession->bImplicitPK;
      break;
    }

    default:
      rc = SQLITE_MISUSE;
  }

  return rc;
}

/*
** Return the maximum size of sqlite3session_changeset() output.
*/
SQLITE_API sqlite3_int64 sqlite3session_changeset_size(sqlite3_session *pSession){
  return pSession->nMaxChangesetSize;
}

/*
** Do the work for either sqlite3changeset_start() or start_strm().
*/
static int sessionChangesetStart(
  sqlite3_changeset_iter **pp,    /* OUT: Changeset iterator handle */
  int (*xInput)(void *pIn, void *pData, int *pnData),
  void *pIn,
  int nChangeset,                 /* Size of buffer pChangeset in bytes */
  void *pChangeset,               /* Pointer to buffer containing changeset */
  int bInvert,                    /* True to invert changeset */
  int bSkipEmpty                  /* True to skip empty UPDATE changes */
){
  sqlite3_changeset_iter *pRet;   /* Iterator to return */
  int nByte;                      /* Number of bytes to allocate for iterator */

  assert( xInput==0 || (pChangeset==0 && nChangeset==0) );

  /* Zero the output variable in case an error occurs. */
  *pp = 0;

  /* Allocate and initialize the iterator structure. */
  nByte = sizeof(sqlite3_changeset_iter);
  pRet = (sqlite3_changeset_iter *)sqlite3_malloc(nByte);
  if( !pRet ) return SQLITE_NOMEM;
  memset(pRet, 0, sizeof(sqlite3_changeset_iter));
  pRet->in.aData = (u8 *)pChangeset;
  pRet->in.nData = nChangeset;
  pRet->in.xInput = xInput;
  pRet->in.pIn = pIn;
  pRet->in.bEof = (xInput ? 0 : 1);
  pRet->bInvert = bInvert;
  pRet->bSkipEmpty = bSkipEmpty;

  /* Populate the output variable and return success. */
  *pp = pRet;
  return SQLITE_OK;
}

/*
** Create an iterator used to iterate through the contents of a changeset.
*/
SQLITE_API int sqlite3changeset_start(
  sqlite3_changeset_iter **pp,    /* OUT: Changeset iterator handle */
  int nChangeset,                 /* Size of buffer pChangeset in bytes */
  void *pChangeset                /* Pointer to buffer containing changeset */
){
  return sessionChangesetStart(pp, 0, 0, nChangeset, pChangeset, 0, 0);
}
SQLITE_API int sqlite3changeset_start_v2(
  sqlite3_changeset_iter **pp,    /* OUT: Changeset iterator handle */
  int nChangeset,                 /* Size of buffer pChangeset in bytes */
  void *pChangeset,               /* Pointer to buffer containing changeset */
  int flags
){
  int bInvert = !!(flags & SQLITE_CHANGESETSTART_INVERT);
  return sessionChangesetStart(pp, 0, 0, nChangeset, pChangeset, bInvert, 0);
}

/*
** Streaming version of sqlite3changeset_start().
*/
SQLITE_API int sqlite3changeset_start_strm(
  sqlite3_changeset_iter **pp,    /* OUT: Changeset iterator handle */
  int (*xInput)(void *pIn, void *pData, int *pnData),
  void *pIn
){
  return sessionChangesetStart(pp, xInput, pIn, 0, 0, 0, 0);
}
SQLITE_API int sqlite3changeset_start_v2_strm(
  sqlite3_changeset_iter **pp,    /* OUT: Changeset iterator handle */
  int (*xInput)(void *pIn, void *pData, int *pnData),
  void *pIn,
  int flags
){
  int bInvert = !!(flags & SQLITE_CHANGESETSTART_INVERT);
  return sessionChangesetStart(pp, xInput, pIn, 0, 0, bInvert, 0);
}

/*
** If the SessionInput object passed as the only argument is a streaming
** object and the buffer is full, discard some data to free up space.
*/
static void sessionDiscardData(SessionInput *pIn){
  if( pIn->xInput && pIn->iCurrent>=sessions_strm_chunk_size ){
    int nMove = pIn->buf.nBuf - pIn->iCurrent;
    assert( nMove>=0 );
    if( nMove>0 ){
      memmove(pIn->buf.aBuf, &pIn->buf.aBuf[pIn->iCurrent], nMove);
    }
    pIn->buf.nBuf -= pIn->iCurrent;
    pIn->iNext -= pIn->iCurrent;
    pIn->iCurrent = 0;
    pIn->nData = pIn->buf.nBuf;
  }
}

/*
** Ensure that there are at least nByte bytes available in the buffer. Or,
** if there are not nByte bytes remaining in the input, that all available
** data is in the buffer.
**
** Return an SQLite error code if an error occurs, or SQLITE_OK otherwise.
*/
static int sessionInputBuffer(SessionInput *pIn, int nByte){
  int rc = SQLITE_OK;
  if( pIn->xInput ){
    while( !pIn->bEof && (pIn->iNext+nByte)>=pIn->nData && rc==SQLITE_OK ){
      int nNew = sessions_strm_chunk_size;

      if( pIn->bNoDiscard==0 ) sessionDiscardData(pIn);
      if( SQLITE_OK==sessionBufferGrow(&pIn->buf, nNew, &rc) ){
        rc = pIn->xInput(pIn->pIn, &pIn->buf.aBuf[pIn->buf.nBuf], &nNew);
        if( nNew==0 ){
          pIn->bEof = 1;
        }else{
          pIn->buf.nBuf += nNew;
        }
      }

      pIn->aData = pIn->buf.aBuf;
      pIn->nData = pIn->buf.nBuf;
    }
  }
  return rc;
}

/*
** When this function is called, *ppRec points to the start of a record
** that contains nCol values. This function advances the pointer *ppRec
** until it points to the byte immediately following that record.
*/
static void sessionSkipRecord(
  u8 **ppRec,                     /* IN/OUT: Record pointer */
  int nCol                        /* Number of values in record */
){
  u8 *aRec = *ppRec;
  int i;
  for(i=0; i<nCol; i++){
    int eType = *aRec++;
    if( eType==SQLITE_TEXT || eType==SQLITE_BLOB ){
      int nByte;
      aRec += sessionVarintGet((u8*)aRec, &nByte);
      aRec += nByte;
    }else if( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT ){
      aRec += 8;
    }
  }

  *ppRec = aRec;
}

/*
** This function sets the value of the sqlite3_value object passed as the
** first argument to a copy of the string or blob held in the aData[]
** buffer. SQLITE_OK is returned if successful, or SQLITE_NOMEM if an OOM
** error occurs.
*/
static int sessionValueSetStr(
  sqlite3_value *pVal,            /* Set the value of this object */
  u8 *aData,                      /* Buffer containing string or blob data */
  int nData,                      /* Size of buffer aData[] in bytes */
  u8 enc                          /* String encoding (0 for blobs) */
){
  /* In theory this code could just pass SQLITE_TRANSIENT as the final
  ** argument to sqlite3ValueSetStr() and have the copy created
  ** automatically. But doing so makes it difficult to detect any OOM
  ** error. Hence the code to create the copy externally. */
  u8 *aCopy = sqlite3_malloc64((sqlite3_int64)nData+1);
  if( aCopy==0 ) return SQLITE_NOMEM;
  memcpy(aCopy, aData, nData);
  sqlite3ValueSetStr(pVal, nData, (char*)aCopy, enc, sqlite3_free);
  return SQLITE_OK;
}

/*
** Deserialize a single record from a buffer in memory. See "RECORD FORMAT"
** for details.
**
** When this function is called, *paChange points to the start of the record
** to deserialize. Assuming no error occurs, *paChange is set to point to
** one byte after the end of the same record before this function returns.
** If the argument abPK is NULL, then the record contains nCol values. Or,
** if abPK is other than NULL, then the record contains only the PK fields
** (in other words, it is a patchset DELETE record).
**
** If successful, each element of the apOut[] array (allocated by the caller)
** is set to point to an sqlite3_value object containing the value read
** from the corresponding position in the record. If that value is not
** included in the record (i.e. because the record is part of an UPDATE change
** and the field was not modified), the corresponding element of apOut[] is
** set to NULL.
**
** It is the responsibility of the caller to free all sqlite_value structures
** using sqlite3_free().
**
** If an error occurs, an SQLite error code (e.g. SQLITE_NOMEM) is returned.
** The apOut[] array may have been partially populated in this case.
*/
static int sessionReadRecord(
  SessionInput *pIn,              /* Input data */
  int nCol,                       /* Number of values in record */
  u8 *abPK,                       /* Array of primary key flags, or NULL */
  sqlite3_value **apOut,          /* Write values to this array */
  int *pbEmpty
){
  int i;                          /* Used to iterate through columns */
  int rc = SQLITE_OK;

  assert( pbEmpty==0 || *pbEmpty==0 );
  if( pbEmpty ) *pbEmpty = 1;
  for(i=0; i<nCol && rc==SQLITE_OK; i++){
    int eType = 0;                /* Type of value (SQLITE_NULL, TEXT etc.) */
    if( abPK && abPK[i]==0 ) continue;
    rc = sessionInputBuffer(pIn, 9);
    if( rc==SQLITE_OK ){
      if( pIn->iNext>=pIn->nData ){
        rc = SQLITE_CORRUPT_BKPT;
      }else{
        eType = pIn->aData[pIn->iNext++];
        assert( apOut[i]==0 );
        if( eType ){
          if( pbEmpty ) *pbEmpty = 0;
          apOut[i] = sqlite3ValueNew(0);
          if( !apOut[i] ) rc = SQLITE_NOMEM;
        }
      }
    }

    if( rc==SQLITE_OK ){
      u8 *aVal = &pIn->aData[pIn->iNext];
      if( eType==SQLITE_TEXT || eType==SQLITE_BLOB ){
        int nByte;
        int nRem = pIn->nData - pIn->iNext;
        pIn->iNext += sessionVarintGetSafe(aVal, nRem, &nByte);
        rc = sessionInputBuffer(pIn, nByte);
        if( rc==SQLITE_OK ){
          if( nByte<0 || nByte>pIn->nData-pIn->iNext ){
            rc = SQLITE_CORRUPT_BKPT;
          }else{
            u8 enc = (eType==SQLITE_TEXT ? SQLITE_UTF8 : 0);
            rc = sessionValueSetStr(apOut[i],&pIn->aData[pIn->iNext],nByte,enc);
            pIn->iNext += nByte;
          }
        }
      }
      if( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT ){
        if( (pIn->nData-pIn->iNext)<8 ){
          rc = SQLITE_CORRUPT_BKPT;
        }else{
          sqlite3_int64 v = sessionGetI64(aVal);
          if( eType==SQLITE_INTEGER ){
            sqlite3VdbeMemSetInt64(apOut[i], v);
          }else{
            double d;
            memcpy(&d, &v, 8);
            sqlite3VdbeMemSetDouble(apOut[i], d);
          }
          pIn->iNext += 8;
        }
      }
    }
  }

  return rc;
}

/*
** The input pointer currently points to the second byte of a table-header.
** Specifically, to the following:
**
**   + number of columns in table (varint)
**   + array of PK flags (1 byte per column),
**   + table name (nul terminated).
**
** This function ensures that all of the above is present in the input
** buffer (i.e. that it can be accessed without any calls to xInput()).
** If successful, SQLITE_OK is returned. Otherwise, an SQLite error code.
** The input pointer is not moved.
*/
static int sessionChangesetBufferTblhdr(SessionInput *pIn, int *pnByte){
  int rc = SQLITE_OK;
  int nCol = 0;
  int nRead = 0;

  rc = sessionInputBuffer(pIn, 9);
  if( rc==SQLITE_OK ){
    int nBuf = pIn->nData - pIn->iNext;
    nRead += sessionVarintGetSafe(&pIn->aData[pIn->iNext], nBuf, &nCol);
    /* The hard upper limit for the number of columns in an SQLite
    ** database table is, according to sqliteLimit.h, 32676. So
    ** consider any table-header that purports to have more than 65536
    ** columns to be corrupt. This is convenient because otherwise,
    ** if the (nCol>65536) condition below were omitted, a sufficiently
    ** large value for nCol may cause nRead to wrap around and become
    ** negative. Leading to a crash. */
    if( nCol<0 || nCol>65536 ){
      rc = SQLITE_CORRUPT_BKPT;
    }else{
      rc = sessionInputBuffer(pIn, nRead+nCol+100);
      nRead += nCol;
    }
  }

  while( rc==SQLITE_OK ){
    while( (pIn->iNext + nRead)<pIn->nData && pIn->aData[pIn->iNext + nRead] ){
      nRead++;
    }

    /* Break out of the loop if if the nul-terminator byte has been found.
    ** Otherwise, read some more input data and keep seeking. If there is
    ** no more input data, consider the changeset corrupt.  */
    if( (pIn->iNext + nRead)<pIn->nData ) break;
    rc = sessionInputBuffer(pIn, nRead + 100);
    if( rc==SQLITE_OK && (pIn->iNext + nRead)>=pIn->nData ){
      rc = SQLITE_CORRUPT_BKPT;
    }
  }
  *pnByte = nRead+1;
  return rc;
}

/*
** The input pointer currently points to the first byte of the first field
** of a record consisting of nCol columns. This function ensures the entire
** record is buffered. It does not move the input pointer.
**
** If successful, SQLITE_OK is returned and *pnByte is set to the size of
** the record in bytes. Otherwise, an SQLite error code is returned. The
** final value of *pnByte is undefined in this case.
*/
static int sessionChangesetBufferRecord(
  SessionInput *pIn,              /* Input data */
  int nCol,                       /* Number of columns in record */
  int *pnByte                     /* OUT: Size of record in bytes */
){
  int rc = SQLITE_OK;
  i64 nByte = 0;
  int i;
  for(i=0; rc==SQLITE_OK && i<nCol; i++){
    int eType;
    rc = sessionInputBuffer(pIn, nByte + 10);
    if( rc==SQLITE_OK ){
      eType = pIn->aData[pIn->iNext + nByte++];
      if( eType==SQLITE_TEXT || eType==SQLITE_BLOB ){
        int n;
        int nRem = pIn->nData - (pIn->iNext + nByte);
        nByte += sessionVarintGetSafe(&pIn->aData[pIn->iNext+nByte], nRem, &n);
        nByte += n;
        rc = sessionInputBuffer(pIn, nByte);
      }else if( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT ){
        nByte += 8;
      }
    }
    if( (pIn->iNext+nByte)>pIn->nData ){
      rc = SQLITE_CORRUPT_BKPT;
    }
  }
  *pnByte = nByte;
  return rc;
}

/*
** The input pointer currently points to the second byte of a table-header.
** Specifically, to the following:
**
**   + number of columns in table (varint)
**   + array of PK flags (1 byte per column),
**   + table name (nul terminated).
**
** This function decodes the table-header and populates the p->nCol,
** p->zTab and p->abPK[] variables accordingly. The p->apValue[] array is
** also allocated or resized according to the new value of p->nCol. The
** input pointer is left pointing to the byte following the table header.
**
** If successful, SQLITE_OK is returned. Otherwise, an SQLite error code
** is returned and the final values of the various fields enumerated above
** are undefined.
*/
static int sessionChangesetReadTblhdr(sqlite3_changeset_iter *p){
  int rc;
  int nCopy;
  assert( p->rc==SQLITE_OK );

  rc = sessionChangesetBufferTblhdr(&p->in, &nCopy);
  if( rc==SQLITE_OK ){
    int nByte;
    int nVarint;
    nVarint = sessionVarintGet(&p->in.aData[p->in.iNext], &p->nCol);
    if( p->nCol>0 ){
      nCopy -= nVarint;
      p->in.iNext += nVarint;
      nByte = p->nCol * sizeof(sqlite3_value*) * 2 + nCopy;
      p->tblhdr.nBuf = 0;
      sessionBufferGrow(&p->tblhdr, nByte, &rc);
    }else{
      rc = SQLITE_CORRUPT_BKPT;
    }
  }

  if( rc==SQLITE_OK ){
    size_t iPK = sizeof(sqlite3_value*)*p->nCol*2;
    memset(p->tblhdr.aBuf, 0, iPK);
    memcpy(&p->tblhdr.aBuf[iPK], &p->in.aData[p->in.iNext], nCopy);
    p->in.iNext += nCopy;
  }

  p->apValue = (sqlite3_value**)p->tblhdr.aBuf;
  if( p->apValue==0 ){
    p->abPK = 0;
    p->zTab = 0;
  }else{
    p->abPK = (u8*)&p->apValue[p->nCol*2];
    p->zTab = p->abPK ? (char*)&p->abPK[p->nCol] : 0;
  }
  return (p->rc = rc);
}

/*
** Advance the changeset iterator to the next change. The differences between
** this function and sessionChangesetNext() are that
**
**   * If pbEmpty is not NULL and the change is a no-op UPDATE (an UPDATE
**     that modifies no columns), this function sets (*pbEmpty) to 1.
**
**   * If the iterator is configured to skip no-op UPDATEs,
**     sessionChangesetNext() does that. This function does not.
*/
static int sessionChangesetNextOne(
  sqlite3_changeset_iter *p,      /* Changeset iterator */
  u8 **paRec,                     /* If non-NULL, store record pointer here */
  int *pnRec,                     /* If non-NULL, store size of record here */
  int *pbNew,                     /* If non-NULL, true if new table */
  int *pbEmpty
){
  int i;
  u8 op;

  assert( (paRec==0 && pnRec==0) || (paRec && pnRec) );
  assert( pbEmpty==0 || *pbEmpty==0 );

  /* If the iterator is in the error-state, return immediately. */
  if( p->rc!=SQLITE_OK ) return p->rc;

  /* Free the current contents of p->apValue[], if any. */
  if( p->apValue ){
    for(i=0; i<p->nCol*2; i++){
      sqlite3ValueFree(p->apValue[i]);
    }
    memset(p->apValue, 0, sizeof(sqlite3_value*)*p->nCol*2);
  }

  /* Make sure the buffer contains at least 2 bytes of input data, or all
  ** remaining data if there are less than 2 bytes available. This is
  ** sufficient either for the 'T' or 'P' byte that begins a new table,
  ** or for the "op" and "bIndirect" single bytes otherwise. */
  p->rc = sessionInputBuffer(&p->in, 2);
  if( p->rc!=SQLITE_OK ) return p->rc;

  p->in.iCurrent = p->in.iNext;
  sessionDiscardData(&p->in);

  /* If the iterator is already at the end of the changeset, return DONE. */
  if( p->in.iNext>=p->in.nData ){
    return SQLITE_DONE;
  }

  op = p->in.aData[p->in.iNext++];
  while( op=='T' || op=='P' ){
    if( pbNew ) *pbNew = 1;
    p->bPatchset = (op=='P');
    if( sessionChangesetReadTblhdr(p) ) return p->rc;
    if( (p->rc = sessionInputBuffer(&p->in, 2)) ) return p->rc;
    p->in.iCurrent = p->in.iNext;
    if( p->in.iNext>=p->in.nData ) return SQLITE_DONE;
    op = p->in.aData[p->in.iNext++];
  }

  if( p->zTab==0 || (p->bPatchset && p->bInvert) ){
    /* The first record in the changeset is not a table header. Must be a
    ** corrupt changeset. */
    assert( p->in.iNext==1 || p->zTab );
    return (p->rc = SQLITE_CORRUPT_BKPT);
  }

  if( (op!=SQLITE_UPDATE && op!=SQLITE_DELETE && op!=SQLITE_INSERT)
   || (p->in.iNext>=p->in.nData)
  ){
    return (p->rc = SQLITE_CORRUPT_BKPT);
  }
  p->op = op;
  p->bIndirect = p->in.aData[p->in.iNext++];

  if( paRec ){
    int nVal;                     /* Number of values to buffer */
    if( p->bPatchset==0 && op==SQLITE_UPDATE ){
      nVal = p->nCol * 2;
    }else if( p->bPatchset && op==SQLITE_DELETE ){
      nVal = 0;
      for(i=0; i<p->nCol; i++) if( p->abPK[i] ) nVal++;
    }else{
      nVal = p->nCol;
    }
    p->rc = sessionChangesetBufferRecord(&p->in, nVal, pnRec);
    if( p->rc!=SQLITE_OK ) return p->rc;
    *paRec = &p->in.aData[p->in.iNext];
    p->in.iNext += *pnRec;
  }else{
    sqlite3_value **apOld = (p->bInvert ? &p->apValue[p->nCol] : p->apValue);
    sqlite3_value **apNew = (p->bInvert ? p->apValue : &p->apValue[p->nCol]);

    /* If this is an UPDATE or DELETE, read the old.* record. */
    if( p->op!=SQLITE_INSERT && (p->bPatchset==0 || p->op==SQLITE_DELETE) ){
      u8 *abPK = p->bPatchset ? p->abPK : 0;
      p->rc = sessionReadRecord(&p->in, p->nCol, abPK, apOld, 0);
      if( p->rc!=SQLITE_OK ) return p->rc;
    }

    /* If this is an INSERT or UPDATE, read the new.* record. */
    if( p->op!=SQLITE_DELETE ){
      p->rc = sessionReadRecord(&p->in, p->nCol, 0, apNew, pbEmpty);
      if( p->rc!=SQLITE_OK ) return p->rc;
    }

    if( (p->bPatchset || p->bInvert) && p->op==SQLITE_UPDATE ){
      /* If this is an UPDATE that is part of a patchset, then all PK and
      ** modified fields are present in the new.* record. The old.* record
      ** is currently completely empty. This block shifts the PK fields from
      ** new.* to old.*, to accommodate the code that reads these arrays.  */
      for(i=0; i<p->nCol; i++){
        assert( p->bPatchset==0 || p->apValue[i]==0 );
        if( p->abPK[i] ){
          assert( p->apValue[i]==0 );
          p->apValue[i] = p->apValue[i+p->nCol];
          if( p->apValue[i]==0 ) return (p->rc = SQLITE_CORRUPT_BKPT);
          p->apValue[i+p->nCol] = 0;
        }
      }
    }else if( p->bInvert ){
      if( p->op==SQLITE_INSERT ) p->op = SQLITE_DELETE;
      else if( p->op==SQLITE_DELETE ) p->op = SQLITE_INSERT;
    }

    /* If this is an UPDATE that is part of a changeset, then check that
    ** there are no fields in the old.* record that are not (a) PK fields,
    ** or (b) also present in the new.* record.
    **
    ** Such records are technically corrupt, but the rebaser was at one
    ** point generating them. Under most circumstances this is benign, but
    ** can cause spurious SQLITE_RANGE errors when applying the changeset. */
    if( p->bPatchset==0 && p->op==SQLITE_UPDATE){
      for(i=0; i<p->nCol; i++){
        if( p->abPK[i]==0 && p->apValue[i+p->nCol]==0 ){
          sqlite3ValueFree(p->apValue[i]);
          p->apValue[i] = 0;
        }
      }
    }
  }

  return SQLITE_ROW;
}

/*
** Advance the changeset iterator to the next change.
**
** If both paRec and pnRec are NULL, then this function works like the public
** API sqlite3changeset_next(). If SQLITE_ROW is returned, then the
** sqlite3changeset_new() and old() APIs may be used to query for values.
**
** Otherwise, if paRec and pnRec are not NULL, then a pointer to the change
** record is written to *paRec before returning and the number of bytes in
** the record to *pnRec.
**
** Either way, this function returns SQLITE_ROW if the iterator is
** successfully advanced to the next change in the changeset, an SQLite
** error code if an error occurs, or SQLITE_DONE if there are no further
** changes in the changeset.
*/
static int sessionChangesetNext(
  sqlite3_changeset_iter *p,      /* Changeset iterator */
  u8 **paRec,                     /* If non-NULL, store record pointer here */
  int *pnRec,                     /* If non-NULL, store size of record here */
  int *pbNew                      /* If non-NULL, true if new table */
){
  int bEmpty;
  int rc;
  do {
    bEmpty = 0;
    rc = sessionChangesetNextOne(p, paRec, pnRec, pbNew, &bEmpty);
  }while( rc==SQLITE_ROW && p->bSkipEmpty && bEmpty);
  return rc;
}

/*
** Advance an iterator created by sqlite3changeset_start() to the next
** change in the changeset. This function may return SQLITE_ROW, SQLITE_DONE
** or SQLITE_CORRUPT.
**
** This function may not be called on iterators passed to a conflict handler
** callback by changeset_apply().
*/
SQLITE_API int sqlite3changeset_next(sqlite3_changeset_iter *p){
  return sessionChangesetNext(p, 0, 0, 0);
}

/*
** The following function extracts information on the current change
** from a changeset iterator. It may only be called after changeset_next()
** has returned SQLITE_ROW.
*/
SQLITE_API int sqlite3changeset_op(
  sqlite3_changeset_iter *pIter,  /* Iterator handle */
  const char **pzTab,             /* OUT: Pointer to table name */
  int *pnCol,                     /* OUT: Number of columns in table */
  int *pOp,                       /* OUT: SQLITE_INSERT, DELETE or UPDATE */
  int *pbIndirect                 /* OUT: True if change is indirect */
){
  *pOp = pIter->op;
  *pnCol = pIter->nCol;
  *pzTab = pIter->zTab;
  if( pbIndirect ) *pbIndirect = pIter->bIndirect;
  return SQLITE_OK;
}

/*
** Return information regarding the PRIMARY KEY and number of columns in
** the database table affected by the change that pIter currently points
** to. This function may only be called after changeset_next() returns
** SQLITE_ROW.
*/
SQLITE_API int sqlite3changeset_pk(
  sqlite3_changeset_iter *pIter,  /* Iterator object */
  unsigned char **pabPK,          /* OUT: Array of boolean - true for PK cols */
  int *pnCol                      /* OUT: Number of entries in output array */
){
  *pabPK = pIter->abPK;
  if( pnCol ) *pnCol = pIter->nCol;
  return SQLITE_OK;
}

/*
** This function may only be called while the iterator is pointing to an
** SQLITE_UPDATE or SQLITE_DELETE change (see sqlite3changeset_op()).
** Otherwise, SQLITE_MISUSE is returned.
**
** It sets *ppValue to point to an sqlite3_value structure containing the
** iVal'th value in the old.* record. Or, if that particular value is not
** included in the record (because the change is an UPDATE and the field
** was not modified and is not a PK column), set *ppValue to NULL.
**
** If value iVal is out-of-range, SQLITE_RANGE is returned and *ppValue is
** not modified. Otherwise, SQLITE_OK.
*/
SQLITE_API int sqlite3changeset_old(
  sqlite3_changeset_iter *pIter,  /* Changeset iterator */
  int iVal,                       /* Index of old.* value to retrieve */
  sqlite3_value **ppValue         /* OUT: Old value (or NULL pointer) */
){
  if( pIter->op!=SQLITE_UPDATE && pIter->op!=SQLITE_DELETE ){
    return SQLITE_MISUSE;
  }
  if( iVal<0 || iVal>=pIter->nCol ){
    return SQLITE_RANGE;
  }
  *ppValue = pIter->apValue[iVal];
  return SQLITE_OK;
}

/*
** This function may only be called while the iterator is pointing to an
** SQLITE_UPDATE or SQLITE_INSERT change (see sqlite3changeset_op()).
** Otherwise, SQLITE_MISUSE is returned.
**
** It sets *ppValue to point to an sqlite3_value structure containing the
** iVal'th value in the new.* record. Or, if that particular value is not
** included in the record (because the change is an UPDATE and the field
** was not modified), set *ppValue to NULL.
**
** If value iVal is out-of-range, SQLITE_RANGE is returned and *ppValue is
** not modified. Otherwise, SQLITE_OK.
*/
SQLITE_API int sqlite3changeset_new(
  sqlite3_changeset_iter *pIter,  /* Changeset iterator */
  int iVal,                       /* Index of new.* value to retrieve */
  sqlite3_value **ppValue         /* OUT: New value (or NULL pointer) */
){
  if( pIter->op!=SQLITE_UPDATE && pIter->op!=SQLITE_INSERT ){
    return SQLITE_MISUSE;
  }
  if( iVal<0 || iVal>=pIter->nCol ){
    return SQLITE_RANGE;
  }
  *ppValue = pIter->apValue[pIter->nCol+iVal];
  return SQLITE_OK;
}

/*
** The following two macros are used internally. They are similar to the
** sqlite3changeset_new() and sqlite3changeset_old() functions, except that
** they omit all error checking and return a pointer to the requested value.
*/
#define sessionChangesetNew(pIter, iVal) (pIter)->apValue[(pIter)->nCol+(iVal)]
#define sessionChangesetOld(pIter, iVal) (pIter)->apValue[(iVal)]

/*
** This function may only be called with a changeset iterator that has been
** passed to an SQLITE_CHANGESET_DATA or SQLITE_CHANGESET_CONFLICT
** conflict-handler function. Otherwise, SQLITE_MISUSE is returned.
**
** If successful, *ppValue is set to point to an sqlite3_value structure
** containing the iVal'th value of the conflicting record.
**
** If value iVal is out-of-range or some other error occurs, an SQLite error
** code is returned. Otherwise, SQLITE_OK.
*/
SQLITE_API int sqlite3changeset_conflict(
  sqlite3_changeset_iter *pIter,  /* Changeset iterator */
  int iVal,                       /* Index of conflict record value to fetch */
  sqlite3_value **ppValue         /* OUT: Value from conflicting row */
){
  if( !pIter->pConflict ){
    return SQLITE_MISUSE;
  }
  if( iVal<0 || iVal>=pIter->nCol ){
    return SQLITE_RANGE;
  }
  *ppValue = sqlite3_column_value(pIter->pConflict, iVal);
  return SQLITE_OK;
}

/*
** This function may only be called with an iterator passed to an
** SQLITE_CHANGESET_FOREIGN_KEY conflict handler callback. In this case
** it sets the output variable to the total number of known foreign key
** violations in the destination database and returns SQLITE_OK.
**
** In all other cases this function returns SQLITE_MISUSE.
*/
SQLITE_API int sqlite3changeset_fk_conflicts(
  sqlite3_changeset_iter *pIter,  /* Changeset iterator */
  int *pnOut                      /* OUT: Number of FK violations */
){
  if( pIter->pConflict || pIter->apValue ){
    return SQLITE_MISUSE;
  }
  *pnOut = pIter->nCol;
  return SQLITE_OK;
}


/*
** Finalize an iterator allocated with sqlite3changeset_start().
**
** This function may not be called on iterators passed to a conflict handler
** callback by changeset_apply().
*/
SQLITE_API int sqlite3changeset_finalize(sqlite3_changeset_iter *p){
  int rc = SQLITE_OK;
  if( p ){
    int i;                        /* Used to iterate through p->apValue[] */
    rc = p->rc;
    if( p->apValue ){
      for(i=0; i<p->nCol*2; i++) sqlite3ValueFree(p->apValue[i]);
    }
    sqlite3_free(p->tblhdr.aBuf);
    sqlite3_free(p->in.buf.aBuf);
    sqlite3_free(p);
  }
  return rc;
}

static int sessionChangesetInvert(
  SessionInput *pInput,           /* Input changeset */
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut,
  int *pnInverted,                /* OUT: Number of bytes in output changeset */
  void **ppInverted               /* OUT: Inverse of pChangeset */
){
  int rc = SQLITE_OK;             /* Return value */
  SessionBuffer sOut;             /* Output buffer */
  int nCol = 0;                   /* Number of cols in current table */
  u8 *abPK = 0;                   /* PK array for current table */
  sqlite3_value **apVal = 0;      /* Space for values for UPDATE inversion */
  SessionBuffer sPK = {0, 0, 0};  /* PK array for current table */

  /* Initialize the output buffer */
  memset(&sOut, 0, sizeof(SessionBuffer));

  /* Zero the output variables in case an error occurs. */
  if( ppInverted ){
    *ppInverted = 0;
    *pnInverted = 0;
  }

  while( 1 ){
    u8 eType;

    /* Test for EOF. */
    if( (rc = sessionInputBuffer(pInput, 2)) ) goto finished_invert;
    if( pInput->iNext>=pInput->nData ) break;
    eType = pInput->aData[pInput->iNext];

    switch( eType ){
      case 'T': {
        /* A 'table' record consists of:
        **
        **   * A constant 'T' character,
        **   * Number of columns in said table (a varint),
        **   * An array of nCol bytes (sPK),
        **   * A nul-terminated table name.
        */
        int nByte;
        int nVar;
        pInput->iNext++;
        if( (rc = sessionChangesetBufferTblhdr(pInput, &nByte)) ){
          goto finished_invert;
        }
        nVar = sessionVarintGet(&pInput->aData[pInput->iNext], &nCol);
        sPK.nBuf = 0;
        sessionAppendBlob(&sPK, &pInput->aData[pInput->iNext+nVar], nCol, &rc);
        sessionAppendByte(&sOut, eType, &rc);
        sessionAppendBlob(&sOut, &pInput->aData[pInput->iNext], nByte, &rc);
        if( rc ) goto finished_invert;

        pInput->iNext += nByte;
        sqlite3_free(apVal);
        apVal = 0;
        abPK = sPK.aBuf;
        break;
      }

      case SQLITE_INSERT:
      case SQLITE_DELETE: {
        int nByte;
        int bIndirect = pInput->aData[pInput->iNext+1];
        int eType2 = (eType==SQLITE_DELETE ? SQLITE_INSERT : SQLITE_DELETE);
        pInput->iNext += 2;
        assert( rc==SQLITE_OK );
        rc = sessionChangesetBufferRecord(pInput, nCol, &nByte);
        sessionAppendByte(&sOut, eType2, &rc);
        sessionAppendByte(&sOut, bIndirect, &rc);
        sessionAppendBlob(&sOut, &pInput->aData[pInput->iNext], nByte, &rc);
        pInput->iNext += nByte;
        if( rc ) goto finished_invert;
        break;
      }

      case SQLITE_UPDATE: {
        int iCol;

        if( 0==apVal ){
          apVal = (sqlite3_value **)sqlite3_malloc64(sizeof(apVal[0])*nCol*2);
          if( 0==apVal ){
            rc = SQLITE_NOMEM;
            goto finished_invert;
          }
          memset(apVal, 0, sizeof(apVal[0])*nCol*2);
        }

        /* Write the header for the new UPDATE change. Same as the original. */
        sessionAppendByte(&sOut, eType, &rc);
        sessionAppendByte(&sOut, pInput->aData[pInput->iNext+1], &rc);

        /* Read the old.* and new.* records for the update change. */
        pInput->iNext += 2;
        rc = sessionReadRecord(pInput, nCol, 0, &apVal[0], 0);
        if( rc==SQLITE_OK ){
          rc = sessionReadRecord(pInput, nCol, 0, &apVal[nCol], 0);
        }

        /* Write the new old.* record. Consists of the PK columns from the
        ** original old.* record, and the other values from the original
        ** new.* record. */
        for(iCol=0; iCol<nCol; iCol++){
          sqlite3_value *pVal = apVal[iCol + (abPK[iCol] ? 0 : nCol)];
          sessionAppendValue(&sOut, pVal, &rc);
        }

        /* Write the new new.* record. Consists of a copy of all values
        ** from the original old.* record, except for the PK columns, which
        ** are set to "undefined". */
        for(iCol=0; iCol<nCol; iCol++){
          sqlite3_value *pVal = (abPK[iCol] ? 0 : apVal[iCol]);
          sessionAppendValue(&sOut, pVal, &rc);
        }

        for(iCol=0; iCol<nCol*2; iCol++){
          sqlite3ValueFree(apVal[iCol]);
        }
        memset(apVal, 0, sizeof(apVal[0])*nCol*2);
        if( rc!=SQLITE_OK ){
          goto finished_invert;
        }

        break;
      }

      default:
        rc = SQLITE_CORRUPT_BKPT;
        goto finished_invert;
    }

    assert( rc==SQLITE_OK );
    if( xOutput && sOut.nBuf>=sessions_strm_chunk_size ){
      rc = xOutput(pOut, sOut.aBuf, sOut.nBuf);
      sOut.nBuf = 0;
      if( rc!=SQLITE_OK ) goto finished_invert;
    }
  }

  assert( rc==SQLITE_OK );
  if( pnInverted && ALWAYS(ppInverted) ){
    *pnInverted = sOut.nBuf;
    *ppInverted = sOut.aBuf;
    sOut.aBuf = 0;
  }else if( sOut.nBuf>0 && ALWAYS(xOutput!=0) ){
    rc = xOutput(pOut, sOut.aBuf, sOut.nBuf);
  }

 finished_invert:
  sqlite3_free(sOut.aBuf);
  sqlite3_free(apVal);
  sqlite3_free(sPK.aBuf);
  return rc;
}


/*
** Invert a changeset object.
*/
SQLITE_API int sqlite3changeset_invert(
  int nChangeset,                 /* Number of bytes in input */
  const void *pChangeset,         /* Input changeset */
  int *pnInverted,                /* OUT: Number of bytes in output changeset */
  void **ppInverted               /* OUT: Inverse of pChangeset */
){
  SessionInput sInput;

  /* Set up the input stream */
  memset(&sInput, 0, sizeof(SessionInput));
  sInput.nData = nChangeset;
  sInput.aData = (u8*)pChangeset;

  return sessionChangesetInvert(&sInput, 0, 0, pnInverted, ppInverted);
}

/*
** Streaming version of sqlite3changeset_invert().
*/
SQLITE_API int sqlite3changeset_invert_strm(
  int (*xInput)(void *pIn, void *pData, int *pnData),
  void *pIn,
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut
){
  SessionInput sInput;
  int rc;

  /* Set up the input stream */
  memset(&sInput, 0, sizeof(SessionInput));
  sInput.xInput = xInput;
  sInput.pIn = pIn;

  rc = sessionChangesetInvert(&sInput, xOutput, pOut, 0, 0);
  sqlite3_free(sInput.buf.aBuf);
  return rc;
}


typedef struct SessionUpdate SessionUpdate;
struct SessionUpdate {
  sqlite3_stmt *pStmt;
  u32 *aMask;
  SessionUpdate *pNext;
};

typedef struct SessionApplyCtx SessionApplyCtx;
struct SessionApplyCtx {
  sqlite3 *db;
  sqlite3_stmt *pDelete;          /* DELETE statement */
  sqlite3_stmt *pInsert;          /* INSERT statement */
  sqlite3_stmt *pSelect;          /* SELECT statement */
  int nCol;                       /* Size of azCol[] and abPK[] arrays */
  const char **azCol;             /* Array of column names */
  u8 *abPK;                       /* Boolean array - true if column is in PK */
  u32 *aUpdateMask;               /* Used by sessionUpdateFind */
  SessionUpdate *pUp;
  int bStat1;                     /* True if table is sqlite_stat1 */
  int bDeferConstraints;          /* True to defer constraints */
  int bInvertConstraints;         /* Invert when iterating constraints buffer */
  SessionBuffer constraints;      /* Deferred constraints are stored here */
  SessionBuffer rebase;           /* Rebase information (if any) here */
  u8 bRebaseStarted;              /* If table header is already in rebase */
  u8 bRebase;                     /* True to collect rebase information */
  u8 bIgnoreNoop;                 /* True to ignore no-op conflicts */
  int bRowid;
  char *zErr;                     /* Error message, if any */
};

/* Number of prepared UPDATE statements to cache. */
#define SESSION_UPDATE_CACHE_SZ 12

/*
** Find a prepared UPDATE statement suitable for the UPDATE step currently
** being visited by the iterator. The UPDATE is of the form:
**
**   UPDATE tbl SET col = ?, col2 = ? WHERE pk1 IS ? AND pk2 IS ?
*/
static int sessionUpdateFind(
  sqlite3_changeset_iter *pIter,
  SessionApplyCtx *p,
  int bPatchset,
  sqlite3_stmt **ppStmt
){
  int rc = SQLITE_OK;
  SessionUpdate *pUp = 0;
  int nCol = pIter->nCol;
  int nU32 = (pIter->nCol+33)/32;
  int ii;

  if( p->aUpdateMask==0 ){
    p->aUpdateMask = sqlite3_malloc(nU32*sizeof(u32));
    if( p->aUpdateMask==0 ){
      rc = SQLITE_NOMEM;
    }
  }

  if( rc==SQLITE_OK ){
    memset(p->aUpdateMask, 0, nU32*sizeof(u32));
    rc = SQLITE_CORRUPT;
    for(ii=0; ii<pIter->nCol; ii++){
      if( sessionChangesetNew(pIter, ii) ){
        p->aUpdateMask[ii/32] |= (1<<(ii%32));
        rc = SQLITE_OK;
      }
    }
  }

  if( rc==SQLITE_OK ){
    if( bPatchset ) p->aUpdateMask[nCol/32] |= (1<<(nCol%32));

    if( p->pUp ){
      int nUp = 0;
      SessionUpdate **pp = &p->pUp;
      while( 1 ){
        nUp++;
        if( 0==memcmp(p->aUpdateMask, (*pp)->aMask, nU32*sizeof(u32)) ){
          pUp = *pp;
          *pp = pUp->pNext;
          pUp->pNext = p->pUp;
          p->pUp = pUp;
          break;
        }

        if( (*pp)->pNext ){
          pp = &(*pp)->pNext;
        }else{
          if( nUp>=SESSION_UPDATE_CACHE_SZ ){
            sqlite3_finalize((*pp)->pStmt);
            sqlite3_free(*pp);
            *pp = 0;
          }
          break;
        }
      }
    }

    if( pUp==0 ){
      int nByte = sizeof(SessionUpdate) * nU32*sizeof(u32);
      int bStat1 = (sqlite3_stricmp(pIter->zTab, "sqlite_stat1")==0);
      pUp = (SessionUpdate*)sqlite3_malloc(nByte);
      if( pUp==0 ){
        rc = SQLITE_NOMEM;
      }else{
        const char *zSep = "";
        SessionBuffer buf;

        memset(&buf, 0, sizeof(buf));
        pUp->aMask = (u32*)&pUp[1];
        memcpy(pUp->aMask, p->aUpdateMask, nU32*sizeof(u32));

        sessionAppendStr(&buf, "UPDATE main.", &rc);
        sessionAppendIdent(&buf, pIter->zTab, &rc);
        sessionAppendStr(&buf, " SET ", &rc);

        /* Create the assignments part of the UPDATE */
        for(ii=0; ii<pIter->nCol; ii++){
          if( p->abPK[ii]==0 && sessionChangesetNew(pIter, ii) ){
            sessionAppendStr(&buf, zSep, &rc);
            sessionAppendIdent(&buf, p->azCol[ii], &rc);
            sessionAppendStr(&buf, " = ?", &rc);
            sessionAppendInteger(&buf, ii*2+1, &rc);
            zSep = ", ";
          }
        }

        /* Create the WHERE clause part of the UPDATE */
        zSep = "";
        sessionAppendStr(&buf, " WHERE ", &rc);
        for(ii=0; ii<pIter->nCol; ii++){
          if( p->abPK[ii] || (bPatchset==0 && sessionChangesetOld(pIter, ii)) ){
            sessionAppendStr(&buf, zSep, &rc);
            if( bStat1 && ii==1 ){
              assert( sqlite3_stricmp(p->azCol[ii], "idx")==0 );
              sessionAppendStr(&buf,
                  "idx IS CASE "
                  "WHEN length(?4)=0 AND typeof(?4)='blob' THEN NULL "
                  "ELSE ?4 END ", &rc
              );
            }else{
              sessionAppendIdent(&buf, p->azCol[ii], &rc);
              sessionAppendStr(&buf, " IS ?", &rc);
              sessionAppendInteger(&buf, ii*2+2, &rc);
            }
            zSep = " AND ";
          }
        }

        if( rc==SQLITE_OK ){
          char *zSql = (char*)buf.aBuf;
          rc = sqlite3_prepare_v2(p->db, zSql, buf.nBuf, &pUp->pStmt, 0);
        }

        if( rc!=SQLITE_OK ){
          sqlite3_free(pUp);
          pUp = 0;
        }else{
          pUp->pNext = p->pUp;
          p->pUp = pUp;
        }
        sqlite3_free(buf.aBuf);
      }
    }
  }

  assert( (rc==SQLITE_OK)==(pUp!=0) );
  if( pUp ){
    *ppStmt = pUp->pStmt;
  }else{
    *ppStmt = 0;
  }
  return rc;
}

/*
** Free all cached UPDATE statements.
*/
static void sessionUpdateFree(SessionApplyCtx *p){
  SessionUpdate *pUp;
  SessionUpdate *pNext;
  for(pUp=p->pUp; pUp; pUp=pNext){
    pNext = pUp->pNext;
    sqlite3_finalize(pUp->pStmt);
    sqlite3_free(pUp);
  }
  p->pUp = 0;
  sqlite3_free(p->aUpdateMask);
  p->aUpdateMask = 0;
}

/*
** Formulate a statement to DELETE a row from database db. Assuming a table
** structure like this:
**
**     CREATE TABLE x(a, b, c, d, PRIMARY KEY(a, c));
**
** The DELETE statement looks like this:
**
**     DELETE FROM x WHERE a = :1 AND c = :3 AND (:5 OR b IS :2 AND d IS :4)
**
** Variable :5 (nCol+1) is a boolean. It should be set to 0 if we require
** matching b and d values, or 1 otherwise. The second case comes up if the
** conflict handler is invoked with NOTFOUND and returns CHANGESET_REPLACE.
**
** If successful, SQLITE_OK is returned and SessionApplyCtx.pDelete is left
** pointing to the prepared version of the SQL statement.
*/
static int sessionDeleteRow(
  sqlite3 *db,                    /* Database handle */
  const char *zTab,               /* Table name */
  SessionApplyCtx *p              /* Session changeset-apply context */
){
  int i;
  const char *zSep = "";
  int rc = SQLITE_OK;
  SessionBuffer buf = {0, 0, 0};
  int nPk = 0;

  sessionAppendStr(&buf, "DELETE FROM main.", &rc);
  sessionAppendIdent(&buf, zTab, &rc);
  sessionAppendStr(&buf, " WHERE ", &rc);

  for(i=0; i<p->nCol; i++){
    if( p->abPK[i] ){
      nPk++;
      sessionAppendStr(&buf, zSep, &rc);
      sessionAppendIdent(&buf, p->azCol[i], &rc);
      sessionAppendStr(&buf, " = ?", &rc);
      sessionAppendInteger(&buf, i+1, &rc);
      zSep = " AND ";
    }
  }

  if( nPk<p->nCol ){
    sessionAppendStr(&buf, " AND (?", &rc);
    sessionAppendInteger(&buf, p->nCol+1, &rc);
    sessionAppendStr(&buf, " OR ", &rc);

    zSep = "";
    for(i=0; i<p->nCol; i++){
      if( !p->abPK[i] ){
        sessionAppendStr(&buf, zSep, &rc);
        sessionAppendIdent(&buf, p->azCol[i], &rc);
        sessionAppendStr(&buf, " IS ?", &rc);
        sessionAppendInteger(&buf, i+1, &rc);
        zSep = "AND ";
      }
    }
    sessionAppendStr(&buf, ")", &rc);
  }

  if( rc==SQLITE_OK ){
    rc = sessionPrepare(db, &p->pDelete, &p->zErr, (char*)buf.aBuf);
  }
  sqlite3_free(buf.aBuf);

  return rc;
}

/*
** Formulate and prepare an SQL statement to query table zTab by primary
** key. Assuming the following table structure:
**
**     CREATE TABLE x(a, b, c, d, PRIMARY KEY(a, c));
**
** The SELECT statement looks like this:
**
**     SELECT * FROM x WHERE a = ?1 AND c = ?3
**
** If successful, SQLITE_OK is returned and SessionApplyCtx.pSelect is left
** pointing to the prepared version of the SQL statement.
*/
static int sessionSelectRow(
  sqlite3 *db,                    /* Database handle */
  const char *zTab,               /* Table name */
  SessionApplyCtx *p              /* Session changeset-apply context */
){
  /* TODO */
  return sessionSelectStmt(db, p->bIgnoreNoop,
      "main", zTab, p->bRowid, p->nCol, p->azCol, p->abPK, &p->pSelect, &p->zErr
  );
}

/*
** Formulate and prepare an INSERT statement to add a record to table zTab.
** For example:
**
**     INSERT INTO main."zTab" VALUES(?1, ?2, ?3 ...);
**
** If successful, SQLITE_OK is returned and SessionApplyCtx.pInsert is left
** pointing to the prepared version of the SQL statement.
*/
static int sessionInsertRow(
  sqlite3 *db,                    /* Database handle */
  const char *zTab,               /* Table name */
  SessionApplyCtx *p              /* Session changeset-apply context */
){
  int rc = SQLITE_OK;
  int i;
  SessionBuffer buf = {0, 0, 0};

  sessionAppendStr(&buf, "INSERT INTO main.", &rc);
  sessionAppendIdent(&buf, zTab, &rc);
  sessionAppendStr(&buf, "(", &rc);
  for(i=0; i<p->nCol; i++){
    if( i!=0 ) sessionAppendStr(&buf, ", ", &rc);
    sessionAppendIdent(&buf, p->azCol[i], &rc);
  }

  sessionAppendStr(&buf, ") VALUES(?", &rc);
  for(i=1; i<p->nCol; i++){
    sessionAppendStr(&buf, ", ?", &rc);
  }
  sessionAppendStr(&buf, ")", &rc);

  if( rc==SQLITE_OK ){
    rc = sessionPrepare(db, &p->pInsert, &p->zErr, (char*)buf.aBuf);
  }
  sqlite3_free(buf.aBuf);
  return rc;
}

/*
** Prepare statements for applying changes to the sqlite_stat1 table.
** These are similar to those created by sessionSelectRow(),
** sessionInsertRow(), sessionUpdateRow() and sessionDeleteRow() for
** other tables.
*/
static int sessionStat1Sql(sqlite3 *db, SessionApplyCtx *p){
  int rc = sessionSelectRow(db, "sqlite_stat1", p);
  if( rc==SQLITE_OK ){
    rc = sessionPrepare(db, &p->pInsert, 0,
        "INSERT INTO main.sqlite_stat1 VALUES(?1, "
        "CASE WHEN length(?2)=0 AND typeof(?2)='blob' THEN NULL ELSE ?2 END, "
        "?3)"
    );
  }
  if( rc==SQLITE_OK ){
    rc = sessionPrepare(db, &p->pDelete, 0,
        "DELETE FROM main.sqlite_stat1 WHERE tbl=?1 AND idx IS "
        "CASE WHEN length(?2)=0 AND typeof(?2)='blob' THEN NULL ELSE ?2 END "
        "AND (?4 OR stat IS ?3)"
    );
  }
  return rc;
}

/*
** A wrapper around sqlite3_bind_value() that detects an extra problem.
** See comments in the body of this function for details.
*/
static int sessionBindValue(
  sqlite3_stmt *pStmt,            /* Statement to bind value to */
  int i,                          /* Parameter number to bind to */
  sqlite3_value *pVal             /* Value to bind */
){
  int eType = sqlite3_value_type(pVal);
  /* COVERAGE: The (pVal->z==0) branch is never true using current versions
  ** of SQLite. If a malloc fails in an sqlite3_value_xxx() function, either
  ** the (pVal->z) variable remains as it was or the type of the value is
  ** set to SQLITE_NULL.  */
  if( (eType==SQLITE_TEXT || eType==SQLITE_BLOB) && pVal->z==0 ){
    /* This condition occurs when an earlier OOM in a call to
    ** sqlite3_value_text() or sqlite3_value_blob() (perhaps from within
    ** a conflict-handler) has zeroed the pVal->z pointer. Return NOMEM. */
    return SQLITE_NOMEM;
  }
  return sqlite3_bind_value(pStmt, i, pVal);
}

/*
** Iterator pIter must point to an SQLITE_INSERT entry. This function
** transfers new.* values from the current iterator entry to statement
** pStmt. The table being inserted into has nCol columns.
**
** New.* value $i from the iterator is bound to variable ($i+1) of
** statement pStmt. If parameter abPK is NULL, all values from 0 to (nCol-1)
** are transfered to the statement. Otherwise, if abPK is not NULL, it points
** to an array nCol elements in size. In this case only those values for
** which abPK[$i] is true are read from the iterator and bound to the
** statement.
**
** An SQLite error code is returned if an error occurs. Otherwise, SQLITE_OK.
*/
static int sessionBindRow(
  sqlite3_changeset_iter *pIter,  /* Iterator to read values from */
  int(*xValue)(sqlite3_changeset_iter *, int, sqlite3_value **),
  int nCol,                       /* Number of columns */
  u8 *abPK,                       /* If not NULL, bind only if true */
  sqlite3_stmt *pStmt             /* Bind values to this statement */
){
  int i;
  int rc = SQLITE_OK;

  /* Neither sqlite3changeset_old or sqlite3changeset_new can fail if the
  ** argument iterator points to a suitable entry. Make sure that xValue
  ** is one of these to guarantee that it is safe to ignore the return
  ** in the code below. */
  assert( xValue==sqlite3changeset_old || xValue==sqlite3changeset_new );

  for(i=0; rc==SQLITE_OK && i<nCol; i++){
    if( !abPK || abPK[i] ){
      sqlite3_value *pVal = 0;
      (void)xValue(pIter, i, &pVal);
      if( pVal==0 ){
        /* The value in the changeset was "undefined". This indicates a
        ** corrupt changeset blob.  */
        rc = SQLITE_CORRUPT_BKPT;
      }else{
        rc = sessionBindValue(pStmt, i+1, pVal);
      }
    }
  }
  return rc;
}

/*
** SQL statement pSelect is as generated by the sessionSelectRow() function.
** This function binds the primary key values from the change that changeset
** iterator pIter points to to the SELECT and attempts to seek to the table
** entry. If a row is found, the SELECT statement left pointing at the row
** and SQLITE_ROW is returned. Otherwise, if no row is found and no error
** has occured, the statement is reset and SQLITE_OK is returned. If an
** error occurs, the statement is reset and an SQLite error code is returned.
**
** If this function returns SQLITE_ROW, the caller must eventually reset()
** statement pSelect. If any other value is returned, the statement does
** not require a reset().
**
** If the iterator currently points to an INSERT record, bind values from the
** new.* record to the SELECT statement. Or, if it points to a DELETE or
** UPDATE, bind values from the old.* record.
*/
static int sessionSeekToRow(
  sqlite3_changeset_iter *pIter,  /* Changeset iterator */
  SessionApplyCtx *p
){
  sqlite3_stmt *pSelect = p->pSelect;
  int rc;                         /* Return code */
  int nCol;                       /* Number of columns in table */
  int op;                         /* Changset operation (SQLITE_UPDATE etc.) */
  const char *zDummy;             /* Unused */

  sqlite3_clear_bindings(pSelect);
  sqlite3changeset_op(pIter, &zDummy, &nCol, &op, 0);
  rc = sessionBindRow(pIter,
      op==SQLITE_INSERT ? sqlite3changeset_new : sqlite3changeset_old,
      nCol, p->abPK, pSelect
  );

  if( op!=SQLITE_DELETE && p->bIgnoreNoop ){
    int ii;
    for(ii=0; rc==SQLITE_OK && ii<nCol; ii++){
      if( p->abPK[ii]==0 ){
        sqlite3_value *pVal = 0;
        sqlite3changeset_new(pIter, ii, &pVal);
        sqlite3_bind_int(pSelect, ii+1+nCol, (pVal==0));
        if( pVal ) rc = sessionBindValue(pSelect, ii+1, pVal);
      }
    }
  }

  if( rc==SQLITE_OK ){
    rc = sqlite3_step(pSelect);
    if( rc!=SQLITE_ROW ) rc = sqlite3_reset(pSelect);
  }

  return rc;
}

/*
** This function is called from within sqlite3changeset_apply_v2() when
** a conflict is encountered and resolved using conflict resolution
** mode eType (either SQLITE_CHANGESET_OMIT or SQLITE_CHANGESET_REPLACE)..
** It adds a conflict resolution record to the buffer in
** SessionApplyCtx.rebase, which will eventually be returned to the caller
** of apply_v2() as the "rebase" buffer.
**
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
static int sessionRebaseAdd(
  SessionApplyCtx *p,             /* Apply context */
  int eType,                      /* Conflict resolution (OMIT or REPLACE) */
  sqlite3_changeset_iter *pIter   /* Iterator pointing at current change */
){
  int rc = SQLITE_OK;
  if( p->bRebase ){
    int i;
    int eOp = pIter->op;
    if( p->bRebaseStarted==0 ){
      /* Append a table-header to the rebase buffer */
      const char *zTab = pIter->zTab;
      sessionAppendByte(&p->rebase, 'T', &rc);
      sessionAppendVarint(&p->rebase, p->nCol, &rc);
      sessionAppendBlob(&p->rebase, p->abPK, p->nCol, &rc);
      sessionAppendBlob(&p->rebase, (u8*)zTab, (int)strlen(zTab)+1, &rc);
      p->bRebaseStarted = 1;
    }

    assert( eType==SQLITE_CHANGESET_REPLACE||eType==SQLITE_CHANGESET_OMIT );
    assert( eOp==SQLITE_DELETE || eOp==SQLITE_INSERT || eOp==SQLITE_UPDATE );

    sessionAppendByte(&p->rebase,
        (eOp==SQLITE_DELETE ? SQLITE_DELETE : SQLITE_INSERT), &rc
        );
    sessionAppendByte(&p->rebase, (eType==SQLITE_CHANGESET_REPLACE), &rc);
    for(i=0; i<p->nCol; i++){
      sqlite3_value *pVal = 0;
      if( eOp==SQLITE_DELETE || (eOp==SQLITE_UPDATE && p->abPK[i]) ){
        sqlite3changeset_old(pIter, i, &pVal);
      }else{
        sqlite3changeset_new(pIter, i, &pVal);
      }
      sessionAppendValue(&p->rebase, pVal, &rc);
    }
  }
  return rc;
}

/*
** Invoke the conflict handler for the change that the changeset iterator
** currently points to.
**
** Argument eType must be either CHANGESET_DATA or CHANGESET_CONFLICT.
** If argument pbReplace is NULL, then the type of conflict handler invoked
** depends solely on eType, as follows:
**
**    eType value                 Value passed to xConflict
**    -------------------------------------------------
**    CHANGESET_DATA              CHANGESET_NOTFOUND
**    CHANGESET_CONFLICT          CHANGESET_CONSTRAINT
**
** Or, if pbReplace is not NULL, then an attempt is made to find an existing
** record with the same primary key as the record about to be deleted, updated
** or inserted. If such a record can be found, it is available to the conflict
** handler as the "conflicting" record. In this case the type of conflict
** handler invoked is as follows:
**
**    eType value         PK Record found?   Value passed to xConflict
**    ----------------------------------------------------------------
**    CHANGESET_DATA      Yes                CHANGESET_DATA
**    CHANGESET_DATA      No                 CHANGESET_NOTFOUND
**    CHANGESET_CONFLICT  Yes                CHANGESET_CONFLICT
**    CHANGESET_CONFLICT  No                 CHANGESET_CONSTRAINT
**
** If pbReplace is not NULL, and a record with a matching PK is found, and
** the conflict handler function returns SQLITE_CHANGESET_REPLACE, *pbReplace
** is set to non-zero before returning SQLITE_OK.
**
** If the conflict handler returns SQLITE_CHANGESET_ABORT, SQLITE_ABORT is
** returned. Or, if the conflict handler returns an invalid value,
** SQLITE_MISUSE. If the conflict handler returns SQLITE_CHANGESET_OMIT,
** this function returns SQLITE_OK.
*/
static int sessionConflictHandler(
  int eType,                      /* Either CHANGESET_DATA or CONFLICT */
  SessionApplyCtx *p,             /* changeset_apply() context */
  sqlite3_changeset_iter *pIter,  /* Changeset iterator */
  int(*xConflict)(void *, int, sqlite3_changeset_iter*),
  void *pCtx,                     /* First argument for conflict handler */
  int *pbReplace                  /* OUT: Set to true if PK row is found */
){
  int res = SQLITE_CHANGESET_OMIT;/* Value returned by conflict handler */
  int rc;
  int nCol;
  int op;
  const char *zDummy;

  sqlite3changeset_op(pIter, &zDummy, &nCol, &op, 0);

  assert( eType==SQLITE_CHANGESET_CONFLICT || eType==SQLITE_CHANGESET_DATA );
  assert( SQLITE_CHANGESET_CONFLICT+1==SQLITE_CHANGESET_CONSTRAINT );
  assert( SQLITE_CHANGESET_DATA+1==SQLITE_CHANGESET_NOTFOUND );

  /* Bind the new.* PRIMARY KEY values to the SELECT statement. */
  if( pbReplace ){
    rc = sessionSeekToRow(pIter, p);
  }else{
    rc = SQLITE_OK;
  }

  if( rc==SQLITE_ROW ){
    /* There exists another row with the new.* primary key. */
    if( 0==p->bIgnoreNoop
     || 0==sqlite3_column_int(p->pSelect, sqlite3_column_count(p->pSelect)-1)
    ){
      pIter->pConflict = p->pSelect;
      res = xConflict(pCtx, eType, pIter);
      pIter->pConflict = 0;
    }
    rc = sqlite3_reset(p->pSelect);
  }else if( rc==SQLITE_OK ){
    if( p->bDeferConstraints && eType==SQLITE_CHANGESET_CONFLICT ){
      /* Instead of invoking the conflict handler, append the change blob
      ** to the SessionApplyCtx.constraints buffer. */
      u8 *aBlob = &pIter->in.aData[pIter->in.iCurrent];
      int nBlob = pIter->in.iNext - pIter->in.iCurrent;
      sessionAppendBlob(&p->constraints, aBlob, nBlob, &rc);
      return SQLITE_OK;
    }else if( p->bIgnoreNoop==0 || op!=SQLITE_DELETE
           || eType==SQLITE_CHANGESET_CONFLICT
    ){
      /* No other row with the new.* primary key. */
      res = xConflict(pCtx, eType+1, pIter);
      if( res==SQLITE_CHANGESET_REPLACE ) rc = SQLITE_MISUSE;
    }
  }

  if( rc==SQLITE_OK ){
    switch( res ){
      case SQLITE_CHANGESET_REPLACE:
        assert( pbReplace );
        *pbReplace = 1;
        break;

      case SQLITE_CHANGESET_OMIT:
        break;

      case SQLITE_CHANGESET_ABORT:
        rc = SQLITE_ABORT;
        break;

      default:
        rc = SQLITE_MISUSE;
        break;
    }
    if( rc==SQLITE_OK ){
      rc = sessionRebaseAdd(p, res, pIter);
    }
  }

  return rc;
}

/*
** Attempt to apply the change that the iterator passed as the first argument
** currently points to to the database. If a conflict is encountered, invoke
** the conflict handler callback.
**
** If argument pbRetry is NULL, then ignore any CHANGESET_DATA conflict. If
** one is encountered, update or delete the row with the matching primary key
** instead. Or, if pbRetry is not NULL and a CHANGESET_DATA conflict occurs,
** invoke the conflict handler. If it returns CHANGESET_REPLACE, set *pbRetry
** to true before returning. In this case the caller will invoke this function
** again, this time with pbRetry set to NULL.
**
** If argument pbReplace is NULL and a CHANGESET_CONFLICT conflict is
** encountered invoke the conflict handler with CHANGESET_CONSTRAINT instead.
** Or, if pbReplace is not NULL, invoke it with CHANGESET_CONFLICT. If such
** an invocation returns SQLITE_CHANGESET_REPLACE, set *pbReplace to true
** before retrying. In this case the caller attempts to remove the conflicting
** row before invoking this function again, this time with pbReplace set
** to NULL.
**
** If any conflict handler returns SQLITE_CHANGESET_ABORT, this function
** returns SQLITE_ABORT. Otherwise, if no error occurs, SQLITE_OK is
** returned.
*/
static int sessionApplyOneOp(
  sqlite3_changeset_iter *pIter,  /* Changeset iterator */
  SessionApplyCtx *p,             /* changeset_apply() context */
  int(*xConflict)(void *, int, sqlite3_changeset_iter *),
  void *pCtx,                     /* First argument for the conflict handler */
  int *pbReplace,                 /* OUT: True to remove PK row and retry */
  int *pbRetry                    /* OUT: True to retry. */
){
  const char *zDummy;
  int op;
  int nCol;
  int rc = SQLITE_OK;

  assert( p->pDelete && p->pInsert && p->pSelect );
  assert( p->azCol && p->abPK );
  assert( !pbReplace || *pbReplace==0 );

  sqlite3changeset_op(pIter, &zDummy, &nCol, &op, 0);

  if( op==SQLITE_DELETE ){

    /* Bind values to the DELETE statement. If conflict handling is required,
    ** bind values for all columns and set bound variable (nCol+1) to true.
    ** Or, if conflict handling is not required, bind just the PK column
    ** values and, if it exists, set (nCol+1) to false. Conflict handling
    ** is not required if:
    **
    **   * this is a patchset, or
    **   * (pbRetry==0), or
    **   * all columns of the table are PK columns (in this case there is
    **     no (nCol+1) variable to bind to).
    */
    u8 *abPK = (pIter->bPatchset ? p->abPK : 0);
    rc = sessionBindRow(pIter, sqlite3changeset_old, nCol, abPK, p->pDelete);
    if( rc==SQLITE_OK && sqlite3_bind_parameter_count(p->pDelete)>nCol ){
      rc = sqlite3_bind_int(p->pDelete, nCol+1, (pbRetry==0 || abPK));
    }
    if( rc!=SQLITE_OK ) return rc;

    sqlite3_step(p->pDelete);
    rc = sqlite3_reset(p->pDelete);
    if( rc==SQLITE_OK && sqlite3_changes(p->db)==0 ){
      rc = sessionConflictHandler(
          SQLITE_CHANGESET_DATA, p, pIter, xConflict, pCtx, pbRetry
      );
    }else if( (rc&0xff)==SQLITE_CONSTRAINT ){
      rc = sessionConflictHandler(
          SQLITE_CHANGESET_CONFLICT, p, pIter, xConflict, pCtx, 0
      );
    }

  }else if( op==SQLITE_UPDATE ){
    int i;
    sqlite3_stmt *pUp = 0;
    int bPatchset = (pbRetry==0 || pIter->bPatchset);

    rc = sessionUpdateFind(pIter, p, bPatchset, &pUp);

    /* Bind values to the UPDATE statement. */
    for(i=0; rc==SQLITE_OK && i<nCol; i++){
      sqlite3_value *pOld = sessionChangesetOld(pIter, i);
      sqlite3_value *pNew = sessionChangesetNew(pIter, i);
      if( p->abPK[i] || (bPatchset==0 && pOld) ){
        rc = sessionBindValue(pUp, i*2+2, pOld);
      }
      if( rc==SQLITE_OK && pNew ){
        rc = sessionBindValue(pUp, i*2+1, pNew);
      }
    }
    if( rc!=SQLITE_OK ) return rc;

    /* Attempt the UPDATE. In the case of a NOTFOUND or DATA conflict,
    ** the result will be SQLITE_OK with 0 rows modified. */
    sqlite3_step(pUp);
    rc = sqlite3_reset(pUp);

    if( rc==SQLITE_OK && sqlite3_changes(p->db)==0 ){
      /* A NOTFOUND or DATA error. Search the table to see if it contains
      ** a row with a matching primary key. If so, this is a DATA conflict.
      ** Otherwise, if there is no primary key match, it is a NOTFOUND. */

      rc = sessionConflictHandler(
          SQLITE_CHANGESET_DATA, p, pIter, xConflict, pCtx, pbRetry
      );

    }else if( (rc&0xff)==SQLITE_CONSTRAINT ){
      /* This is always a CONSTRAINT conflict. */
      rc = sessionConflictHandler(
          SQLITE_CHANGESET_CONFLICT, p, pIter, xConflict, pCtx, 0
      );
    }

  }else{
    assert( op==SQLITE_INSERT );
    if( p->bStat1 ){
      /* Check if there is a conflicting row. For sqlite_stat1, this needs
      ** to be done using a SELECT, as there is no PRIMARY KEY in the
      ** database schema to throw an exception if a duplicate is inserted.  */
      rc = sessionSeekToRow(pIter, p);
      if( rc==SQLITE_ROW ){
        rc = SQLITE_CONSTRAINT;
        sqlite3_reset(p->pSelect);
      }
    }

    if( rc==SQLITE_OK ){
      rc = sessionBindRow(pIter, sqlite3changeset_new, nCol, 0, p->pInsert);
      if( rc!=SQLITE_OK ) return rc;

      sqlite3_step(p->pInsert);
      rc = sqlite3_reset(p->pInsert);
    }

    if( (rc&0xff)==SQLITE_CONSTRAINT ){
      rc = sessionConflictHandler(
          SQLITE_CHANGESET_CONFLICT, p, pIter, xConflict, pCtx, pbReplace
      );
    }
  }

  return rc;
}

/*
** Attempt to apply the change that the iterator passed as the first argument
** currently points to to the database. If a conflict is encountered, invoke
** the conflict handler callback.
**
** The difference between this function and sessionApplyOne() is that this
** function handles the case where the conflict-handler is invoked and
** returns SQLITE_CHANGESET_REPLACE - indicating that the change should be
** retried in some manner.
*/
static int sessionApplyOneWithRetry(
  sqlite3 *db,                    /* Apply change to "main" db of this handle */
  sqlite3_changeset_iter *pIter,  /* Changeset iterator to read change from */
  SessionApplyCtx *pApply,        /* Apply context */
  int(*xConflict)(void*, int, sqlite3_changeset_iter*),
  void *pCtx                      /* First argument passed to xConflict */
){
  int bReplace = 0;
  int bRetry = 0;
  int rc;

  rc = sessionApplyOneOp(pIter, pApply, xConflict, pCtx, &bReplace, &bRetry);
  if( rc==SQLITE_OK ){
    /* If the bRetry flag is set, the change has not been applied due to an
    ** SQLITE_CHANGESET_DATA problem (i.e. this is an UPDATE or DELETE and
    ** a row with the correct PK is present in the db, but one or more other
    ** fields do not contain the expected values) and the conflict handler
    ** returned SQLITE_CHANGESET_REPLACE. In this case retry the operation,
    ** but pass NULL as the final argument so that sessionApplyOneOp() ignores
    ** the SQLITE_CHANGESET_DATA problem.  */
    if( bRetry ){
      assert( pIter->op==SQLITE_UPDATE || pIter->op==SQLITE_DELETE );
      rc = sessionApplyOneOp(pIter, pApply, xConflict, pCtx, 0, 0);
    }

    /* If the bReplace flag is set, the change is an INSERT that has not
    ** been performed because the database already contains a row with the
    ** specified primary key and the conflict handler returned
    ** SQLITE_CHANGESET_REPLACE. In this case remove the conflicting row
    ** before reattempting the INSERT.  */
    else if( bReplace ){
      assert( pIter->op==SQLITE_INSERT );
      rc = sqlite3_exec(db, "SAVEPOINT replace_op", 0, 0, 0);
      if( rc==SQLITE_OK ){
        rc = sessionBindRow(pIter,
            sqlite3changeset_new, pApply->nCol, pApply->abPK, pApply->pDelete);
        sqlite3_bind_int(pApply->pDelete, pApply->nCol+1, 1);
      }
      if( rc==SQLITE_OK ){
        sqlite3_step(pApply->pDelete);
        rc = sqlite3_reset(pApply->pDelete);
      }
      if( rc==SQLITE_OK ){
        rc = sessionApplyOneOp(pIter, pApply, xConflict, pCtx, 0, 0);
      }
      if( rc==SQLITE_OK ){
        rc = sqlite3_exec(db, "RELEASE replace_op", 0, 0, 0);
      }
    }
  }

  return rc;
}

/*
** Retry the changes accumulated in the pApply->constraints buffer.
*/
static int sessionRetryConstraints(
  sqlite3 *db,
  int bPatchset,
  const char *zTab,
  SessionApplyCtx *pApply,
  int(*xConflict)(void*, int, sqlite3_changeset_iter*),
  void *pCtx                      /* First argument passed to xConflict */
){
  int rc = SQLITE_OK;

  while( pApply->constraints.nBuf ){
    sqlite3_changeset_iter *pIter2 = 0;
    SessionBuffer cons = pApply->constraints;
    memset(&pApply->constraints, 0, sizeof(SessionBuffer));

    rc = sessionChangesetStart(
        &pIter2, 0, 0, cons.nBuf, cons.aBuf, pApply->bInvertConstraints, 1
    );
    if( rc==SQLITE_OK ){
      size_t nByte = 2*pApply->nCol*sizeof(sqlite3_value*);
      int rc2;
      pIter2->bPatchset = bPatchset;
      pIter2->zTab = (char*)zTab;
      pIter2->nCol = pApply->nCol;
      pIter2->abPK = pApply->abPK;
      sessionBufferGrow(&pIter2->tblhdr, nByte, &rc);
      pIter2->apValue = (sqlite3_value**)pIter2->tblhdr.aBuf;
      if( rc==SQLITE_OK ) memset(pIter2->apValue, 0, nByte);

      while( rc==SQLITE_OK && SQLITE_ROW==sqlite3changeset_next(pIter2) ){
        rc = sessionApplyOneWithRetry(db, pIter2, pApply, xConflict, pCtx);
      }

      rc2 = sqlite3changeset_finalize(pIter2);
      if( rc==SQLITE_OK ) rc = rc2;
    }
    assert( pApply->bDeferConstraints || pApply->constraints.nBuf==0 );

    sqlite3_free(cons.aBuf);
    if( rc!=SQLITE_OK ) break;
    if( pApply->constraints.nBuf>=cons.nBuf ){
      /* No progress was made on the last round. */
      pApply->bDeferConstraints = 0;
    }
  }

  return rc;
}

/*
** Argument pIter is a changeset iterator that has been initialized, but
** not yet passed to sqlite3changeset_next(). This function applies the
** changeset to the main database attached to handle "db". The supplied
** conflict handler callback is invoked to resolve any conflicts encountered
** while applying the change.
*/
static int sessionChangesetApply(
  sqlite3 *db,                    /* Apply change to "main" db of this handle */
  sqlite3_changeset_iter *pIter,  /* Changeset to apply */
  int(*xFilter)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    const char *zTab              /* Table name */
  ),
  int(*xFilterIter)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    sqlite3_changeset_iter *p
  ),
  int(*xConflict)(
    void *pCtx,                   /* Copy of fifth arg to _apply() */
    int eConflict,                /* DATA, MISSING, CONFLICT, CONSTRAINT */
    sqlite3_changeset_iter *p     /* Handle describing change and conflict */
  ),
  void *pCtx,                     /* First argument passed to xConflict */
  void **ppRebase, int *pnRebase, /* OUT: Rebase information */
  int flags                       /* SESSION_APPLY_XXX flags */
){
  int schemaMismatch = 0;
  int rc = SQLITE_OK;             /* Return code */
  const char *zTab = 0;           /* Name of current table */
  int nTab = 0;                   /* Result of sqlite3Strlen30(zTab) */
  SessionApplyCtx sApply;         /* changeset_apply() context object */
  int bPatchset;
  u64 savedFlag = db->flags & SQLITE_FkNoAction;

  assert( xConflict!=0 );

  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  if( flags & SQLITE_CHANGESETAPPLY_FKNOACTION ){
    db->flags |= ((u64)SQLITE_FkNoAction);
    db->aDb[0].pSchema->schema_cookie -= 32;
  }

  pIter->in.bNoDiscard = 1;
  memset(&sApply, 0, sizeof(sApply));
  sApply.bRebase = (ppRebase && pnRebase);
  sApply.bInvertConstraints = !!(flags & SQLITE_CHANGESETAPPLY_INVERT);
  sApply.bIgnoreNoop = !!(flags & SQLITE_CHANGESETAPPLY_IGNORENOOP);
  if( (flags & SQLITE_CHANGESETAPPLY_NOSAVEPOINT)==0 ){
    rc = sqlite3_exec(db, "SAVEPOINT changeset_apply", 0, 0, 0);
  }
  if( rc==SQLITE_OK ){
    rc = sqlite3_exec(db, "PRAGMA defer_foreign_keys = 1", 0, 0, 0);
  }
  while( rc==SQLITE_OK && SQLITE_ROW==sqlite3changeset_next(pIter) ){
    int nCol;
    int op;
    const char *zNew;

    sqlite3changeset_op(pIter, &zNew, &nCol, &op, 0);

    if( zTab==0 || sqlite3_strnicmp(zNew, zTab, nTab+1) ){
      u8 *abPK;

      rc = sessionRetryConstraints(
          db, pIter->bPatchset, zTab, &sApply, xConflict, pCtx
      );
      if( rc!=SQLITE_OK ) break;

      sessionUpdateFree(&sApply);
      sqlite3_free((char*)sApply.azCol);  /* cast works around VC++ bug */
      sqlite3_finalize(sApply.pDelete);
      sqlite3_finalize(sApply.pInsert);
      sqlite3_finalize(sApply.pSelect);
      sApply.db = db;
      sApply.pDelete = 0;
      sApply.pInsert = 0;
      sApply.pSelect = 0;
      sApply.nCol = 0;
      sApply.azCol = 0;
      sApply.abPK = 0;
      sApply.bStat1 = 0;
      sApply.bDeferConstraints = 1;
      sApply.bRebaseStarted = 0;
      sApply.bRowid = 0;
      memset(&sApply.constraints, 0, sizeof(SessionBuffer));

      /* If an xFilter() callback was specified, invoke it now. If the
      ** xFilter callback returns zero, skip this table. If it returns
      ** non-zero, proceed. */
      schemaMismatch = (xFilter && (0==xFilter(pCtx, zNew)));
      if( schemaMismatch ){
        zTab = sqlite3_mprintf("%s", zNew);
        if( zTab==0 ){
          rc = SQLITE_NOMEM;
          break;
        }
        nTab = (int)strlen(zTab);
        sApply.azCol = (const char **)zTab;
      }else{
        int nMinCol = 0;
        int i;

        sqlite3changeset_pk(pIter, &abPK, 0);
        rc = sessionTableInfo(0, db, "main", zNew,
            &sApply.nCol, 0, &zTab, &sApply.azCol, 0, 0,
            &sApply.abPK, &sApply.bRowid
        );
        if( rc!=SQLITE_OK ) break;
        for(i=0; i<sApply.nCol; i++){
          if( sApply.abPK[i] ) nMinCol = i+1;
        }

        if( sApply.nCol==0 ){
          schemaMismatch = 1;
          sqlite3_log(SQLITE_SCHEMA,
              "sqlite3changeset_apply(): no such table: %s", zTab
          );
        }
        else if( sApply.nCol<nCol ){
          schemaMismatch = 1;
          sqlite3_log(SQLITE_SCHEMA,
              "sqlite3changeset_apply(): table %s has %d columns, "
              "expected %d or more",
              zTab, sApply.nCol, nCol
          );
        }
        else if( nCol<nMinCol || memcmp(sApply.abPK, abPK, nCol)!=0 ){
          schemaMismatch = 1;
          sqlite3_log(SQLITE_SCHEMA, "sqlite3changeset_apply(): "
              "primary key mismatch for table %s", zTab
          );
        }
        else{
          sApply.nCol = nCol;
          if( 0==sqlite3_stricmp(zTab, "sqlite_stat1") ){
            if( (rc = sessionStat1Sql(db, &sApply) ) ){
              break;
            }
            sApply.bStat1 = 1;
          }else{
            if( (rc = sessionSelectRow(db, zTab, &sApply))
             || (rc = sessionDeleteRow(db, zTab, &sApply))
             || (rc = sessionInsertRow(db, zTab, &sApply))
            ){
              break;
            }
            sApply.bStat1 = 0;
          }
        }
        nTab = sqlite3Strlen30(zTab);
      }
    }

    /* If there is a schema mismatch on the current table, proceed to the
    ** next change. A log message has already been issued. */
    if( schemaMismatch ) continue;

    /* If this is a call to apply_v3(), invoke xFilterIter here. */
    if( xFilterIter && 0==xFilterIter(pCtx, pIter) ) continue;

    rc = sessionApplyOneWithRetry(db, pIter, &sApply, xConflict, pCtx);
  }

  bPatchset = pIter->bPatchset;
  if( rc==SQLITE_OK ){
    rc = sqlite3changeset_finalize(pIter);
  }else{
    sqlite3changeset_finalize(pIter);
  }

  if( rc==SQLITE_OK ){
    rc = sessionRetryConstraints(db, bPatchset, zTab, &sApply, xConflict, pCtx);
  }

  if( rc==SQLITE_OK ){
    int nFk, notUsed;
    sqlite3_db_status(db, SQLITE_DBSTATUS_DEFERRED_FKS, &nFk, &notUsed, 0);
    if( nFk!=0 ){
      int res = SQLITE_CHANGESET_ABORT;
      sqlite3_changeset_iter sIter;
      memset(&sIter, 0, sizeof(sIter));
      sIter.nCol = nFk;
      res = xConflict(pCtx, SQLITE_CHANGESET_FOREIGN_KEY, &sIter);
      if( res!=SQLITE_CHANGESET_OMIT ){
        rc = SQLITE_CONSTRAINT;
      }
    }
  }

  {
    int rc2 = sqlite3_exec(db, "PRAGMA defer_foreign_keys = 0", 0, 0, 0);
    if( rc==SQLITE_OK ) rc = rc2;
  }

  if( (flags & SQLITE_CHANGESETAPPLY_NOSAVEPOINT)==0 ){
    if( rc==SQLITE_OK ){
      rc = sqlite3_exec(db, "RELEASE changeset_apply", 0, 0, 0);
    }
    if( rc!=SQLITE_OK ){
      sqlite3_exec(db, "ROLLBACK TO changeset_apply", 0, 0, 0);
      sqlite3_exec(db, "RELEASE changeset_apply", 0, 0, 0);
    }
  }

  assert( sApply.bRebase || sApply.rebase.nBuf==0 );
  if( rc==SQLITE_OK && bPatchset==0 && sApply.bRebase ){
    assert( ppRebase!=0 && pnRebase!=0 );
    *ppRebase = (void*)sApply.rebase.aBuf;
    *pnRebase = sApply.rebase.nBuf;
    sApply.rebase.aBuf = 0;
  }
  sessionUpdateFree(&sApply);
  sqlite3_finalize(sApply.pInsert);
  sqlite3_finalize(sApply.pDelete);
  sqlite3_finalize(sApply.pSelect);
  sqlite3_free((char*)sApply.azCol);  /* cast works around VC++ bug */
  sqlite3_free((char*)sApply.constraints.aBuf);
  sqlite3_free((char*)sApply.rebase.aBuf);

  if( (flags & SQLITE_CHANGESETAPPLY_FKNOACTION) && savedFlag==0 ){
    assert( db->flags & SQLITE_FkNoAction );
    db->flags &= ~((u64)SQLITE_FkNoAction);
    db->aDb[0].pSchema->schema_cookie -= 32;
  }

  assert( rc!=SQLITE_OK || sApply.zErr==0 );
  sqlite3_set_errmsg(db, rc, sApply.zErr);
  sqlite3_free(sApply.zErr);

  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  return rc;
}

/*
** This function is called by all six sqlite3changeset_apply() variants:
**
**   +  sqlite3changeset_apply()
**   +  sqlite3changeset_apply_v2()
**   +  sqlite3changeset_apply_v3()
**   +  sqlite3changeset_apply_strm()
**   +  sqlite3changeset_apply_strm_v2()
**   +  sqlite3changeset_apply_strm_v3()
**
** Arguments passed to this function are as follows:
**
** db:
**   Database handle to apply changeset to main database of.
**
** nChangeset/pChangeset:
**   These are both passed zero for the streaming variants. For the normal
**   apply() functions, these are passed the size of and the buffer containing
**   the changeset, respectively.
**
** xInput/pIn:
**   These are both passed zero for the normal variants. For the streaming
**   apply() functions, these are passed the input callback and context
**   pointer, respectively.
**
** xFilter:
**   The filter function as passed to apply() or apply_v2() (to filter by
**   table name), if any. This is always NULL for apply_v3() calls.
**
** xFilterIter:
**   The filter function as passed to apply_v3(), if any.
**
** xConflict:
**   The conflict handler callback (must not be NULL).
**
** pCtx:
**   The context pointer passed to the xFilter and xConflict handler callbacks.
**
** ppRebase, pnRebase:
**   Zero for apply(). The rebase changeset output pointers, if any, for
**   apply_v2() and apply_v3().
**
** flags:
**   Zero for apply(). The flags parameter for apply_v2() and apply_v3().
*/
static int sessionChangesetApplyV23(
  sqlite3 *db,                    /* Apply change to "main" db of this handle */
  int nChangeset,                 /* Size of changeset in bytes */
  void *pChangeset,               /* Changeset blob */
  int (*xInput)(void *pIn, void *pData, int *pnData), /* Input function */
  void *pIn,                                          /* First arg for xInput */
  int(*xFilter)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    const char *zTab              /* Table name */
  ),
  int(*xFilterIter)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    sqlite3_changeset_iter *p     /* Handle describing current change */
  ),
  int(*xConflict)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    int eConflict,                /* DATA, MISSING, CONFLICT, CONSTRAINT */
    sqlite3_changeset_iter *p     /* Handle describing change and conflict */
  ),
  void *pCtx,                     /* First argument passed to xConflict */
  void **ppRebase, int *pnRebase,
  int flags
){
  sqlite3_changeset_iter *pIter;  /* Iterator to skip through changeset */
  int bInverse = !!(flags & SQLITE_CHANGESETAPPLY_INVERT);
  int rc = sessionChangesetStart(
      &pIter, xInput, pIn, nChangeset, pChangeset, bInverse, 1
  );
  if( rc==SQLITE_OK ){
    rc = sessionChangesetApply(db, pIter,
        xFilter, xFilterIter, xConflict, pCtx, ppRebase, pnRebase, flags
    );
  }
  return rc;
}

/*
** Apply the changeset passed via pChangeset/nChangeset to the main
** database attached to handle "db".
*/
SQLITE_API int sqlite3changeset_apply_v2(
  sqlite3 *db,                    /* Apply change to "main" db of this handle */
  int nChangeset,                 /* Size of changeset in bytes */
  void *pChangeset,               /* Changeset blob */
  int(*xFilter)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    const char *zTab              /* Table name */
  ),
  int(*xConflict)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    int eConflict,                /* DATA, MISSING, CONFLICT, CONSTRAINT */
    sqlite3_changeset_iter *p     /* Handle describing change and conflict */
  ),
  void *pCtx,                     /* First argument passed to xConflict */
  void **ppRebase, int *pnRebase,
  int flags
){
  return sessionChangesetApplyV23(db,
      nChangeset, pChangeset, 0, 0,
      xFilter, 0, xConflict, pCtx,
      ppRebase, pnRebase, flags
  );
}

/*
** Apply the changeset passed via pChangeset/nChangeset to the main
** database attached to handle "db".
*/
SQLITE_API int sqlite3changeset_apply_v3(
  sqlite3 *db,                    /* Apply change to "main" db of this handle */
  int nChangeset,                 /* Size of changeset in bytes */
  void *pChangeset,               /* Changeset blob */
  int(*xFilter)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    sqlite3_changeset_iter *p     /* Handle describing current change */
  ),
  int(*xConflict)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    int eConflict,                /* DATA, MISSING, CONFLICT, CONSTRAINT */
    sqlite3_changeset_iter *p     /* Handle describing change and conflict */
  ),
  void *pCtx,                     /* First argument passed to xConflict */
  void **ppRebase, int *pnRebase,
  int flags
){
  return sessionChangesetApplyV23(db,
      nChangeset, pChangeset, 0, 0,
      0, xFilter, xConflict, pCtx,
      ppRebase, pnRebase, flags
  );
}

/*
** Apply the changeset passed via pChangeset/nChangeset to the main database
** attached to handle "db". Invoke the supplied conflict handler callback
** to resolve any conflicts encountered while applying the change.
*/
SQLITE_API int sqlite3changeset_apply(
  sqlite3 *db,                    /* Apply change to "main" db of this handle */
  int nChangeset,                 /* Size of changeset in bytes */
  void *pChangeset,               /* Changeset blob */
  int(*xFilter)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    const char *zTab              /* Table name */
  ),
  int(*xConflict)(
    void *pCtx,                   /* Copy of fifth arg to _apply() */
    int eConflict,                /* DATA, MISSING, CONFLICT, CONSTRAINT */
    sqlite3_changeset_iter *p     /* Handle describing change and conflict */
  ),
  void *pCtx                      /* First argument passed to xConflict */
){
  return sessionChangesetApplyV23(db,
      nChangeset, pChangeset, 0, 0,
      xFilter, 0, xConflict, pCtx,
      0, 0, 0
  );
}

/*
** Apply the changeset passed via xInput/pIn to the main database
** attached to handle "db". Invoke the supplied conflict handler callback
** to resolve any conflicts encountered while applying the change.
*/
SQLITE_API int sqlite3changeset_apply_v3_strm(
  sqlite3 *db,                    /* Apply change to "main" db of this handle */
  int (*xInput)(void *pIn, void *pData, int *pnData), /* Input function */
  void *pIn,                                          /* First arg for xInput */
  int(*xFilter)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    sqlite3_changeset_iter *p
  ),
  int(*xConflict)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    int eConflict,                /* DATA, MISSING, CONFLICT, CONSTRAINT */
    sqlite3_changeset_iter *p     /* Handle describing change and conflict */
  ),
  void *pCtx,                     /* First argument passed to xConflict */
  void **ppRebase, int *pnRebase,
  int flags
){
  return sessionChangesetApplyV23(db,
      0, 0, xInput, pIn,
      0, xFilter, xConflict, pCtx,
      ppRebase, pnRebase, flags
  );
}
SQLITE_API int sqlite3changeset_apply_v2_strm(
  sqlite3 *db,                    /* Apply change to "main" db of this handle */
  int (*xInput)(void *pIn, void *pData, int *pnData), /* Input function */
  void *pIn,                                          /* First arg for xInput */
  int(*xFilter)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    const char *zTab              /* Table name */
  ),
  int(*xConflict)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    int eConflict,                /* DATA, MISSING, CONFLICT, CONSTRAINT */
    sqlite3_changeset_iter *p     /* Handle describing change and conflict */
  ),
  void *pCtx,                     /* First argument passed to xConflict */
  void **ppRebase, int *pnRebase,
  int flags
){
  return sessionChangesetApplyV23(db,
      0, 0, xInput, pIn,
      xFilter, 0, xConflict, pCtx,
      ppRebase, pnRebase, flags
  );
}
SQLITE_API int sqlite3changeset_apply_strm(
  sqlite3 *db,                    /* Apply change to "main" db of this handle */
  int (*xInput)(void *pIn, void *pData, int *pnData), /* Input function */
  void *pIn,                                          /* First arg for xInput */
  int(*xFilter)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    const char *zTab              /* Table name */
  ),
  int(*xConflict)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    int eConflict,                /* DATA, MISSING, CONFLICT, CONSTRAINT */
    sqlite3_changeset_iter *p     /* Handle describing change and conflict */
  ),
  void *pCtx                      /* First argument passed to xConflict */
){
  return sessionChangesetApplyV23(db,
      0, 0, xInput, pIn,
      xFilter, 0, xConflict, pCtx,
      0, 0, 0
  );
}

/*
** The parts of the sqlite3_changegroup structure used by the
** sqlite3changegroup_change_xxx() APIs.
*/
typedef struct ChangeData ChangeData;
struct ChangeData {
  SessionTable *pTab;
  int bIndirect;
  int eOp;

  int nBufAlloc;
  SessionBuffer *aBuf;
  SessionBuffer record;
};

/*
** sqlite3_changegroup handle.
*/
struct sqlite3_changegroup {
  int rc;                         /* Error code */
  int bPatch;                     /* True to accumulate patchsets */
  SessionTable *pList;            /* List of tables in current patch */
  SessionBuffer rec;

  sqlite3 *db;                    /* Configured by changegroup_schema() */
  char *zDb;                      /* Configured by changegroup_schema() */
  ChangeData cd;                  /* Used by changegroup_change_xxx() APIs. */
};

/*
** This function is called to merge two changes to the same row together as
** part of an sqlite3changeset_concat() operation. A new change object is
** allocated and a pointer to it stored in *ppNew.
**
** Because they have been vetted by sqlite3changegroup_add() or similar,
** both the aRec[] change and the pExist change are safe to use without
** checking for buffer overflows.
*/
static int sessionChangeMerge(
  SessionTable *pTab,             /* Table structure */
  int bRebase,                    /* True for a rebase hash-table */
  int bPatchset,                  /* True for patchsets */
  SessionChange *pExist,          /* Existing change */
  int op2,                        /* Second change operation */
  int bIndirect,                  /* True if second change is indirect */
  u8 *aRec,                       /* Second change record */
  int nRec,                       /* Number of bytes in aRec */
  SessionChange **ppNew           /* OUT: Merged change */
){
  SessionChange *pNew = 0;
  int rc = SQLITE_OK;
  assert( aRec!=0 );

  if( !pExist ){
    pNew = (SessionChange *)sqlite3_malloc64(sizeof(SessionChange) + nRec);
    if( !pNew ){
      return SQLITE_NOMEM;
    }
    memset(pNew, 0, sizeof(SessionChange));
    pNew->op = op2;
    pNew->bIndirect = bIndirect;
    pNew->aRecord = (u8*)&pNew[1];
    if( bIndirect==0 || bRebase==0 ){
      pNew->nRecord = nRec;
      memcpy(pNew->aRecord, aRec, nRec);
    }else{
      int i;
      u8 *pIn = aRec;
      u8 *pOut = pNew->aRecord;
      for(i=0; i<pTab->nCol; i++){
        int nIn = sessionSerialLen(pIn);
        if( *pIn==0 ){
          *pOut++ = 0;
        }else if( pTab->abPK[i]==0 ){
          *pOut++ = 0xFF;
        }else{
          memcpy(pOut, pIn, nIn);
          pOut += nIn;
        }
        pIn += nIn;
      }
      pNew->nRecord = pOut - pNew->aRecord;
    }
  }else if( bRebase ){
    if( pExist->op==SQLITE_DELETE && pExist->bIndirect ){
      *ppNew = pExist;
    }else{
      sqlite3_int64 nByte = nRec + pExist->nRecord + sizeof(SessionChange);
      pNew = (SessionChange*)sqlite3_malloc64(nByte);
      if( pNew==0 ){
        rc = SQLITE_NOMEM;
      }else{
        int i;
        u8 *a1 = pExist->aRecord;
        u8 *a2 = aRec;
        u8 *pOut;

        memset(pNew, 0, nByte);
        pNew->bIndirect = bIndirect || pExist->bIndirect;
        pNew->op = op2;
        pOut = pNew->aRecord = (u8*)&pNew[1];

        for(i=0; i<pTab->nCol; i++){
          int n1 = sessionSerialLen(a1);
          int n2 = sessionSerialLen(a2);
          if( *a1==0xFF || (pTab->abPK[i]==0 && bIndirect) ){
            *pOut++ = 0xFF;
          }else if( *a2==0 ){
            memcpy(pOut, a1, n1);
            pOut += n1;
          }else{
            memcpy(pOut, a2, n2);
            pOut += n2;
          }
          a1 += n1;
          a2 += n2;
        }
        pNew->nRecord = pOut - pNew->aRecord;
      }
      sqlite3_free(pExist);
    }
  }else{
    int op1 = pExist->op;

    /*
    **   op1=INSERT, op2=INSERT      ->      Unsupported. Discard op2.
    **   op1=INSERT, op2=UPDATE      ->      INSERT.
    **   op1=INSERT, op2=DELETE      ->      (none)
    **
    **   op1=UPDATE, op2=INSERT      ->      Unsupported. Discard op2.
    **   op1=UPDATE, op2=UPDATE      ->      UPDATE.
    **   op1=UPDATE, op2=DELETE      ->      DELETE.
    **
    **   op1=DELETE, op2=INSERT      ->      UPDATE.
    **   op1=DELETE, op2=UPDATE      ->      Unsupported. Discard op2.
    **   op1=DELETE, op2=DELETE      ->      Unsupported. Discard op2.
    */
    if( (op1==SQLITE_INSERT && op2==SQLITE_INSERT)
     || (op1==SQLITE_UPDATE && op2==SQLITE_INSERT)
     || (op1==SQLITE_DELETE && op2==SQLITE_UPDATE)
     || (op1==SQLITE_DELETE && op2==SQLITE_DELETE)
    ){
      pNew = pExist;
    }else if( op1==SQLITE_INSERT && op2==SQLITE_DELETE ){
      sqlite3_free(pExist);
      assert( pNew==0 );
    }else{
      u8 *aExist = pExist->aRecord;
      sqlite3_int64 nByte;
      u8 *aCsr;

      /* Allocate a new SessionChange object. Ensure that the aRecord[]
      ** buffer of the new object is large enough to hold any record that
      ** may be generated by combining the input records.  */
      nByte = sizeof(SessionChange) + pExist->nRecord + nRec;
      pNew = (SessionChange *)sqlite3_malloc64(nByte);
      if( !pNew ){
        sqlite3_free(pExist);
        return SQLITE_NOMEM;
      }
      memset(pNew, 0, sizeof(SessionChange));
      pNew->bIndirect = (bIndirect && pExist->bIndirect);
      aCsr = pNew->aRecord = (u8 *)&pNew[1];

      if( op1==SQLITE_INSERT ){             /* INSERT + UPDATE */
        u8 *a1 = aRec;
        assert( op2==SQLITE_UPDATE );
        pNew->op = SQLITE_INSERT;
        if( bPatchset==0 ) sessionSkipRecord(&a1, pTab->nCol);
        sessionMergeRecord(&aCsr, pTab->nCol, aExist, a1);
      }else if( op1==SQLITE_DELETE ){       /* DELETE + INSERT */
        assert( op2==SQLITE_INSERT );
        pNew->op = SQLITE_UPDATE;
        if( bPatchset ){
          memcpy(aCsr, aRec, nRec);
          aCsr += nRec;
        }else{
          if( 0==sessionMergeUpdate(&aCsr, pTab, bPatchset, aExist,0,aRec,0) ){
            sqlite3_free(pNew);
            pNew = 0;
          }
        }
      }else if( op2==SQLITE_UPDATE ){       /* UPDATE + UPDATE */
        u8 *a1 = aExist;
        u8 *a2 = aRec;
        assert( op1==SQLITE_UPDATE );
        if( bPatchset==0 ){
          sessionSkipRecord(&a1, pTab->nCol);
          sessionSkipRecord(&a2, pTab->nCol);
        }
        pNew->op = SQLITE_UPDATE;
        if( 0==sessionMergeUpdate(&aCsr, pTab, bPatchset, aRec, aExist,a1,a2) ){
          sqlite3_free(pNew);
          pNew = 0;
        }
      }else{                                /* UPDATE + DELETE */
        assert( op1==SQLITE_UPDATE && op2==SQLITE_DELETE );
        pNew->op = SQLITE_DELETE;
        if( bPatchset ){
          memcpy(aCsr, aRec, nRec);
          aCsr += nRec;
        }else{
          sessionMergeRecord(&aCsr, pTab->nCol, aRec, aExist);
        }
      }

      if( pNew ){
        pNew->nRecord = (int)(aCsr - pNew->aRecord);
      }
      sqlite3_free(pExist);
    }
  }

  *ppNew = pNew;
  return rc;
}

/*
** Check if a changeset entry with nCol columns and the PK array passed
** as the final argument to this function is compatible with SessionTable
** pTab. If so, return 1. Otherwise, if they are incompatible in some way,
** return 0.
*/
static int sessionChangesetCheckCompat(
  SessionTable *pTab,
  int nCol,
  u8 *abPK
){
  if( pTab->azCol && nCol<pTab->nCol ){
    int ii;
    for(ii=0; ii<pTab->nCol; ii++){
      u8 bPK = (ii < nCol) ? abPK[ii] : 0;
      if( pTab->abPK[ii]!=bPK ) return 0;
    }
    return 1;
  }
  return (pTab->nCol==nCol && 0==memcmp(abPK, pTab->abPK, nCol));
}

static int sessionChangesetExtendRecord(
  sqlite3_changegroup *pGrp,
  SessionTable *pTab,
  int nCol,
  int op,
  const u8 *aRec,
  int nRec,
  SessionBuffer *pOut
){
  int rc = SQLITE_OK;
  int ii = 0;

  assert( pTab->azCol );
  assert( nCol<pTab->nCol );

  pOut->nBuf = 0;
  if( op==SQLITE_INSERT || (op==SQLITE_DELETE && pGrp->bPatch==0) ){
    /* Append the missing default column values to the record. */
    sessionAppendBlob(pOut, aRec, nRec, &rc);
    if( rc==SQLITE_OK && pTab->pDfltStmt==0 ){
      rc = sessionPrepareDfltStmt(pGrp->db, pTab, &pTab->pDfltStmt);
      if( rc==SQLITE_OK && SQLITE_ROW!=sqlite3_step(pTab->pDfltStmt) ){
        rc = sqlite3_errcode(pGrp->db);
      }
    }
    for(ii=nCol; rc==SQLITE_OK && ii<pTab->nCol; ii++){
      int eType = sqlite3_column_type(pTab->pDfltStmt, ii);
      sessionAppendByte(pOut, eType, &rc);
      switch( eType ){
        case SQLITE_FLOAT:
        case SQLITE_INTEGER: {
          if( SQLITE_OK==sessionBufferGrow(pOut, 8, &rc) ){
            if( eType==SQLITE_INTEGER ){
              sqlite3_int64 iVal = sqlite3_column_int64(pTab->pDfltStmt, ii);
              sessionPutI64(&pOut->aBuf[pOut->nBuf], iVal);
            }else{
              double rVal = sqlite3_column_double(pTab->pDfltStmt, ii);
              sessionPutDouble(&pOut->aBuf[pOut->nBuf], rVal);
            }
            pOut->nBuf += 8;
          }
          break;
        }

        case SQLITE_BLOB:
        case SQLITE_TEXT: {
          int n = sqlite3_column_bytes(pTab->pDfltStmt, ii);
          sessionAppendVarint(pOut, n, &rc);
          if( eType==SQLITE_TEXT ){
            const u8 *z = (const u8*)sqlite3_column_text(pTab->pDfltStmt, ii);
            sessionAppendBlob(pOut, z, n, &rc);
          }else{
            const u8 *z = (const u8*)sqlite3_column_blob(pTab->pDfltStmt, ii);
            sessionAppendBlob(pOut, z, n, &rc);
          }
          break;
        }

        default:
          assert( eType==SQLITE_NULL );
          break;
      }
    }
  }else if( op==SQLITE_UPDATE ){
    /* Append missing "undefined" entries to the old.* record. And, if this
    ** is an UPDATE, to the new.* record as well.  */
    int iOff = 0;
    if( pGrp->bPatch==0 ){
      for(ii=0; ii<nCol; ii++){
        iOff += sessionSerialLen(&aRec[iOff]);
      }
      sessionAppendBlob(pOut, aRec, iOff, &rc);
      for(ii=0; ii<(pTab->nCol-nCol); ii++){
        sessionAppendByte(pOut, 0x00, &rc);
      }
    }

    sessionAppendBlob(pOut, &aRec[iOff], nRec-iOff, &rc);
    for(ii=0; ii<(pTab->nCol-nCol); ii++){
      sessionAppendByte(pOut, 0x00, &rc);
    }
  }else{
    assert( op==SQLITE_DELETE && pGrp->bPatch );
    sessionAppendBlob(pOut, aRec, nRec, &rc);
  }

  return rc;
}

/*
** Locate or create a SessionTable object that may be used to add the
** change currently pointed to by iterator pIter to changegroup pGrp.
** If successful, set output variable (*ppTab) to point to the table
** object and return SQLITE_OK. Otherwise, if some error occurs, return
** an SQLite error code and leave (*ppTab) set to NULL.
*/
static int sessionChangesetFindTable(
  sqlite3_changegroup *pGrp,
  const char *zTab,
  sqlite3_changeset_iter *pIter,
  SessionTable **ppTab
){
  int rc = SQLITE_OK;
  SessionTable *pTab = 0;
  int nTab = (int)strlen(zTab);
  u8 *abPK = 0;
  int nCol = 0;

  *ppTab = 0;

  /* Search the list for an existing table */
  for(pTab = pGrp->pList; pTab; pTab=pTab->pNext){
    if( 0==sqlite3_strnicmp(pTab->zName, zTab, nTab+1) ) break;
  }


  if( pIter ){
    sqlite3changeset_pk(pIter, &abPK, &nCol);
  }else if( !pTab && !pGrp->db ){
    return SQLITE_OK;
  }

  /* If one was not found above, create a new table now */
  if( !pTab ){
    SessionTable **ppNew;

    pTab = sqlite3_malloc64(sizeof(SessionTable) + nCol + nTab+1);
    if( !pTab ){
      return SQLITE_NOMEM;
    }
    memset(pTab, 0, sizeof(SessionTable));
    pTab->nCol = nCol;
    pTab->abPK = (u8*)&pTab[1];
    if( nCol>0 ){
      memcpy(pTab->abPK, abPK, nCol);
    }
    pTab->zName = (char*)&pTab->abPK[nCol];
    memcpy(pTab->zName, zTab, nTab+1);

    if( pGrp->db ){
      pTab->nCol = 0;
      rc = sessionInitTable(0, pTab, pGrp->db, pGrp->zDb);
      if( rc || pTab->nCol==0 ){
        sqlite3_free(pTab->azCol);
        sqlite3_free(pTab);
        return rc;
      }
    }

    /* The new object must be linked on to the end of the list, not
    ** simply added to the start of it. This is to ensure that the
    ** tables within the output of sqlite3changegroup_output() are in
    ** the right order.  */
    for(ppNew=&pGrp->pList; *ppNew; ppNew=&(*ppNew)->pNext);
    *ppNew = pTab;
  }

  /* Check that the table is compatible. */
  if( pIter && !sessionChangesetCheckCompat(pTab, nCol, abPK) ){
    rc = SQLITE_SCHEMA;
  }

  *ppTab = pTab;
  return rc;
}

/*
** Add a single change to the changegroup pGrp.
*/
static int sessionOneChangeToHash(
  sqlite3_changegroup *pGrp,      /* Changegroup to update */
  SessionTable *pTab,             /* Table change pertains to */
  int op,                         /* One of SQLITE_INSERT, UPDATE, DELETE */
  int bIndirect,                  /* True to flag change as "indirect" */
  int nCol,                       /* Number of columns in record(s) */
  u8 *aRec,                       /* Serialized change record(s) */
  int nRec,                       /* Size of aRec[] in bytes */
  int bRebase                     /* True if this is a rebase blob */
){
  int rc = SQLITE_OK;
  int iHash = 0;
  SessionChange *pChange = 0;
  SessionChange *pExist = 0;
  SessionChange **pp = 0;

  assert( nRec>0 );

  if( nCol<pTab->nCol ){
    SessionBuffer *pBuf = &pGrp->rec;
    rc = sessionChangesetExtendRecord(pGrp, pTab, nCol, op, aRec, nRec, pBuf);
    aRec = pBuf->aBuf;
    nRec = pBuf->nBuf;
    assert( pGrp->db );
  }

  if( rc==SQLITE_OK && sessionGrowHash(0, pGrp->bPatch, pTab) ){
    rc = SQLITE_NOMEM;
  }

  if( rc==SQLITE_OK ){
    /* Search for existing entry. If found, remove it from the hash table.
    ** Code below may link it back in.  */
    iHash = sessionChangeHash(
        pTab, (pGrp->bPatch && op==SQLITE_DELETE), aRec, pTab->nChange
    );
    for(pp=&pTab->apChange[iHash]; *pp; pp=&(*pp)->pNext){
      int bPkOnly1 = 0;
      int bPkOnly2 = 0;
      if( pGrp->bPatch ){
        bPkOnly1 = (*pp)->op==SQLITE_DELETE;
        bPkOnly2 = op==SQLITE_DELETE;
      }
      if( sessionChangeEqual(pTab, bPkOnly1, (*pp)->aRecord, bPkOnly2, aRec) ){
        pExist = *pp;
        *pp = (*pp)->pNext;
        pTab->nEntry--;
        break;
      }
    }
  }

  if( rc==SQLITE_OK ){
    rc = sessionChangeMerge(pTab, bRebase,
        pGrp->bPatch, pExist, op, bIndirect, aRec, nRec, &pChange
    );
  }
  if( rc==SQLITE_OK && pChange ){
    pChange->pNext = pTab->apChange[iHash];
    pTab->apChange[iHash] = pChange;
    pTab->nEntry++;
  }

  return rc;
}

/*
** Add the change currently indicated by iterator pIter to the hash table
** belonging to changegroup pGrp.
*/
static int sessionOneChangeIterToHash(
  sqlite3_changegroup *pGrp,
  sqlite3_changeset_iter *pIter,
  int bRebase
){
  u8 *aRec = &pIter->in.aData[pIter->in.iCurrent + 2];
  int nRec = (pIter->in.iNext - pIter->in.iCurrent) - 2;
  const char *zTab = 0;
  int nCol = 0;
  int op = 0;
  int bIndirect = 0;
  int rc = SQLITE_OK;
  SessionTable *pTab = 0;

  /* Ensure that only changesets, or only patchsets, but not a mixture
  ** of both, are being combined. It is an error to try to combine a
  ** changeset and a patchset.  */
  if( pGrp->pList==0 ){
    pGrp->bPatch = pIter->bPatchset;
  }else if( pIter->bPatchset!=pGrp->bPatch ){
    rc = SQLITE_ERROR;
  }

  if( rc==SQLITE_OK ){
    sqlite3changeset_op(pIter, &zTab, &nCol, &op, &bIndirect);
    rc = sessionChangesetFindTable(pGrp, zTab, pIter, &pTab);
  }

  if( rc==SQLITE_OK ){
    rc = sessionOneChangeToHash(
        pGrp, pTab, op, bIndirect, nCol, aRec, nRec, bRebase
    );
  }

  if( rc==SQLITE_OK ) rc = pIter->rc;
  return rc;
}

/*
** Add all changes in the changeset traversed by the iterator passed as
** the first argument to the changegroup hash tables.
*/
static int sessionChangesetToHash(
  sqlite3_changeset_iter *pIter,   /* Iterator to read from */
  sqlite3_changegroup *pGrp,       /* Changegroup object to add changeset to */
  int bRebase                      /* True if hash table is for rebasing */
){
  u8 *aRec;
  int nRec;
  int rc = SQLITE_OK;

  pIter->in.bNoDiscard = 1;
  while( SQLITE_ROW==(sessionChangesetNext(pIter, &aRec, &nRec, 0)) ){
    rc = sessionOneChangeIterToHash(pGrp, pIter, bRebase);
    if( rc!=SQLITE_OK ) break;
  }

  if( rc==SQLITE_OK ) rc = pIter->rc;
  return rc;
}

/*
** Serialize a changeset (or patchset) based on all changesets (or patchsets)
** added to the changegroup object passed as the first argument.
**
** If xOutput is not NULL, then the changeset/patchset is returned to the
** user via one or more calls to xOutput, as with the other streaming
** interfaces.
**
** Or, if xOutput is NULL, then (*ppOut) is populated with a pointer to a
** buffer containing the output changeset before this function returns. In
** this case (*pnOut) is set to the size of the output buffer in bytes. It
** is the responsibility of the caller to free the output buffer using
** sqlite3_free() when it is no longer required.
**
** If successful, SQLITE_OK is returned. Or, if an error occurs, an SQLite
** error code. If an error occurs and xOutput is NULL, (*ppOut) and (*pnOut)
** are both set to 0 before returning.
*/
static int sessionChangegroupOutput(
  sqlite3_changegroup *pGrp,
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut,
  int *pnOut,
  void **ppOut
){
  int rc = SQLITE_OK;
  SessionBuffer buf = {0, 0, 0};
  SessionTable *pTab;
  assert( xOutput==0 || (ppOut==0 && pnOut==0) );

  /* Create the serialized output changeset based on the contents of the
  ** hash tables attached to the SessionTable objects in list p->pList.
  */
  for(pTab=pGrp->pList; rc==SQLITE_OK && pTab; pTab=pTab->pNext){
    int i;
    if( pTab->nEntry==0 ) continue;

    sessionAppendTableHdr(&buf, pGrp->bPatch, pTab, &rc);
    for(i=0; i<pTab->nChange; i++){
      SessionChange *p;
      for(p=pTab->apChange[i]; p; p=p->pNext){
        sessionAppendByte(&buf, p->op, &rc);
        sessionAppendByte(&buf, p->bIndirect, &rc);
        sessionAppendBlob(&buf, p->aRecord, p->nRecord, &rc);
        if( rc==SQLITE_OK && xOutput && buf.nBuf>=sessions_strm_chunk_size ){
          rc = xOutput(pOut, buf.aBuf, buf.nBuf);
          buf.nBuf = 0;
        }
      }
    }
  }

  if( rc==SQLITE_OK ){
    if( xOutput ){
      if( buf.nBuf>0 ) rc = xOutput(pOut, buf.aBuf, buf.nBuf);
    }else if( ppOut ){
      *ppOut = buf.aBuf;
      if( pnOut ) *pnOut = buf.nBuf;
      buf.aBuf = 0;
    }
  }
  sqlite3_free(buf.aBuf);

  return rc;
}

/*
** Allocate a new, empty, sqlite3_changegroup.
*/
SQLITE_API int sqlite3changegroup_new(sqlite3_changegroup **pp){
  int rc = SQLITE_OK;             /* Return code */
  sqlite3_changegroup *p;         /* New object */
  p = (sqlite3_changegroup*)sqlite3_malloc(sizeof(sqlite3_changegroup));
  if( p==0 ){
    rc = SQLITE_NOMEM;
  }else{
    memset(p, 0, sizeof(sqlite3_changegroup));
  }
  *pp = p;
  return rc;
}

/*
** Configure a changegroup object.
*/
SQLITE_API int sqlite3changegroup_config(
  sqlite3_changegroup *pGrp,
  int op,
  void *pArg
){
  int rc = SQLITE_OK;

  switch( op ){
    case SQLITE_CHANGEGROUP_CONFIG_PATCHSET: {
      int arg = *(int*)pArg;
      if( pGrp->pList==0  && arg>=0 ){
        pGrp->bPatch = (arg>0);
      }
      *(int*)pArg = pGrp->bPatch;
      break;
    }
    default:
      rc = SQLITE_MISUSE;
      break;
  }

  return rc;
}

/*
** Provide a database schema to the changegroup object.
*/
SQLITE_API int sqlite3changegroup_schema(
  sqlite3_changegroup *pGrp,
  sqlite3 *db,
  const char *zDb
){
  int rc = SQLITE_OK;

  if( pGrp->pList || pGrp->db ){
    /* Cannot add a schema after one or more calls to sqlite3changegroup_add(),
    ** or after sqlite3changegroup_schema() has already been called. */
    rc = SQLITE_MISUSE;
  }else{
    pGrp->zDb = sqlite3_mprintf("%s", zDb);
    if( pGrp->zDb==0 ){
      rc = SQLITE_NOMEM;
    }else{
      pGrp->db = db;
    }
  }
  return rc;
}

/*
** Add the changeset currently stored in buffer pData, size nData bytes,
** to changeset-group p.
*/
SQLITE_API int sqlite3changegroup_add(sqlite3_changegroup *pGrp, int nData, void *pData){
  sqlite3_changeset_iter *pIter;  /* Iterator opened on pData/nData */
  int rc;                         /* Return code */

  rc = sqlite3changeset_start(&pIter, nData, pData);
  if( rc==SQLITE_OK ){
    rc = sessionChangesetToHash(pIter, pGrp, 0);
  }
  sqlite3changeset_finalize(pIter);
  return rc;
}

/*
** Add a single change to a changeset-group.
*/
SQLITE_API int sqlite3changegroup_add_change(
  sqlite3_changegroup *pGrp,
  sqlite3_changeset_iter *pIter
){
  int rc = SQLITE_OK;

  if( pIter->in.iCurrent==pIter->in.iNext
   || pIter->rc!=SQLITE_OK
   || pIter->bInvert
  ){
    /* Iterator does not point to any valid entry or is an INVERT iterator. */
    rc = SQLITE_ERROR;
  }else{
    pIter->in.bNoDiscard = 1;
    rc = sessionOneChangeIterToHash(pGrp, pIter, 0);
  }
  return rc;
}

/*
** Obtain a buffer containing a changeset representing the concatenation
** of all changesets added to the group so far.
*/
SQLITE_API int sqlite3changegroup_output(
    sqlite3_changegroup *pGrp,
    int *pnData,
    void **ppData
){
  return sessionChangegroupOutput(pGrp, 0, 0, pnData, ppData);
}

/*
** Streaming versions of changegroup_add().
*/
SQLITE_API int sqlite3changegroup_add_strm(
  sqlite3_changegroup *pGrp,
  int (*xInput)(void *pIn, void *pData, int *pnData),
  void *pIn
){
  sqlite3_changeset_iter *pIter;  /* Iterator opened on pData/nData */
  int rc;                         /* Return code */

  rc = sqlite3changeset_start_strm(&pIter, xInput, pIn);
  if( rc==SQLITE_OK ){
    rc = sessionChangesetToHash(pIter, pGrp, 0);
  }
  sqlite3changeset_finalize(pIter);
  return rc;
}

/*
** Streaming versions of changegroup_output().
*/
SQLITE_API int sqlite3changegroup_output_strm(
  sqlite3_changegroup *pGrp,
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut
){
  return sessionChangegroupOutput(pGrp, xOutput, pOut, 0, 0);
}

/*
** Delete a changegroup object.
*/
SQLITE_API void sqlite3changegroup_delete(sqlite3_changegroup *pGrp){
  if( pGrp ){
    int ii;
    for(ii=0; ii<pGrp->cd.nBufAlloc; ii++){
      sqlite3_free(pGrp->cd.aBuf[ii].aBuf);
    }
    sqlite3_free(pGrp->cd.record.aBuf);
    sqlite3_free(pGrp->cd.aBuf);
    sqlite3_free(pGrp->zDb);
    sessionDeleteTable(0, pGrp->pList);
    sqlite3_free(pGrp->rec.aBuf);
    sqlite3_free(pGrp);
  }
}

/*
** Combine two changesets together.
*/
SQLITE_API int sqlite3changeset_concat(
  int nLeft,                      /* Number of bytes in lhs input */
  void *pLeft,                    /* Lhs input changeset */
  int nRight                      /* Number of bytes in rhs input */,
  void *pRight,                   /* Rhs input changeset */
  int *pnOut,                     /* OUT: Number of bytes in output changeset */
  void **ppOut                    /* OUT: changeset (left <concat> right) */
){
  sqlite3_changegroup *pGrp;
  int rc;

  rc = sqlite3changegroup_new(&pGrp);
  if( rc==SQLITE_OK ){
    rc = sqlite3changegroup_add(pGrp, nLeft, pLeft);
  }
  if( rc==SQLITE_OK ){
    rc = sqlite3changegroup_add(pGrp, nRight, pRight);
  }
  if( rc==SQLITE_OK ){
    rc = sqlite3changegroup_output(pGrp, pnOut, ppOut);
  }
  sqlite3changegroup_delete(pGrp);

  return rc;
}

/*
** Streaming version of sqlite3changeset_concat().
*/
SQLITE_API int sqlite3changeset_concat_strm(
  int (*xInputA)(void *pIn, void *pData, int *pnData),
  void *pInA,
  int (*xInputB)(void *pIn, void *pData, int *pnData),
  void *pInB,
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut
){
  sqlite3_changegroup *pGrp;
  int rc;

  rc = sqlite3changegroup_new(&pGrp);
  if( rc==SQLITE_OK ){
    rc = sqlite3changegroup_add_strm(pGrp, xInputA, pInA);
  }
  if( rc==SQLITE_OK ){
    rc = sqlite3changegroup_add_strm(pGrp, xInputB, pInB);
  }
  if( rc==SQLITE_OK ){
    rc = sqlite3changegroup_output_strm(pGrp, xOutput, pOut);
  }
  sqlite3changegroup_delete(pGrp);

  return rc;
}

/*
** Changeset rebaser handle.
*/
struct sqlite3_rebaser {
  sqlite3_changegroup grp;        /* Hash table */
};

/*
** Buffers a1 and a2 must both contain a sessions module record nCol
** fields in size. This function appends an nCol sessions module
** record to buffer pBuf that is a copy of a1, except that for
** each field that is undefined in a1[], swap in the field from a2[].
*/
static void sessionAppendRecordMerge(
  SessionBuffer *pBuf,            /* Buffer to append to */
  int nCol,                       /* Number of columns in each record */
  u8 *a1, int n1,                 /* Record 1 */
  u8 *a2, int n2,                 /* Record 2 */
  int *pRc                        /* IN/OUT: error code */
){
  sessionBufferGrow(pBuf, n1+n2, pRc);
  if( *pRc==SQLITE_OK ){
    int i;
    u8 *pOut = &pBuf->aBuf[pBuf->nBuf];
    for(i=0; i<nCol; i++){
      int nn1 = sessionSerialLen(a1);
      int nn2 = sessionSerialLen(a2);
      if( *a1==0 || *a1==0xFF ){
        memcpy(pOut, a2, nn2);
        pOut += nn2;
      }else{
        memcpy(pOut, a1, nn1);
        pOut += nn1;
      }
      a1 += nn1;
      a2 += nn2;
    }

    pBuf->nBuf = pOut-pBuf->aBuf;
    assert( pBuf->nBuf<=pBuf->nAlloc );
  }
}

/*
** This function is called when rebasing a local UPDATE change against one
** or more remote UPDATE changes. The aRec/nRec buffer contains the current
** old.* and new.* records for the change. The rebase buffer (a single
** record) is in aChange/nChange. The rebased change is appended to buffer
** pBuf.
**
** Rebasing the UPDATE involves:
**
**   * Removing any changes to fields for which the corresponding field
**     in the rebase buffer is set to "replaced" (type 0xFF). If this
**     means the UPDATE change updates no fields, nothing is appended
**     to the output buffer.
**
**   * For each field modified by the local change for which the
**     corresponding field in the rebase buffer is not "undefined" (0x00)
**     or "replaced" (0xFF), the old.* value is replaced by the value
**     in the rebase buffer.
*/
static void sessionAppendPartialUpdate(
  SessionBuffer *pBuf,            /* Append record here */
  sqlite3_changeset_iter *pIter,  /* Iterator pointed at local change */
  u8 *aRec, int nRec,             /* Local change */
  u8 *aChange, int nChange,       /* Record to rebase against */
  int *pRc                        /* IN/OUT: Return Code */
){
  sessionBufferGrow(pBuf, 2+nRec+nChange, pRc);
  if( *pRc==SQLITE_OK ){
    int bData = 0;
    u8 *pOut = &pBuf->aBuf[pBuf->nBuf];
    int i;
    u8 *a1 = aRec;
    u8 *a2 = aChange;

    *pOut++ = SQLITE_UPDATE;
    *pOut++ = pIter->bIndirect;
    for(i=0; i<pIter->nCol; i++){
      int n1 = sessionSerialLen(a1);
      int n2 = sessionSerialLen(a2);
      if( pIter->abPK[i] || a2[0]==0 ){
        if( !pIter->abPK[i] && a1[0] ) bData = 1;
        memcpy(pOut, a1, n1);
        pOut += n1;
      }else if( a2[0]!=0xFF && a1[0] ){
        bData = 1;
        memcpy(pOut, a2, n2);
        pOut += n2;
      }else{
        *pOut++ = '\0';
      }
      a1 += n1;
      a2 += n2;
    }
    if( bData ){
      a2 = aChange;
      for(i=0; i<pIter->nCol; i++){
        int n1 = sessionSerialLen(a1);
        int n2 = sessionSerialLen(a2);
        if( pIter->abPK[i] || a2[0]!=0xFF ){
          memcpy(pOut, a1, n1);
          pOut += n1;
        }else{
          *pOut++ = '\0';
        }
        a1 += n1;
        a2 += n2;
      }
      pBuf->nBuf = (pOut - pBuf->aBuf);
    }
  }
}

/*
** pIter is configured to iterate through a changeset. This function rebases
** that changeset according to the current configuration of the rebaser
** object passed as the first argument. If no error occurs and argument xOutput
** is not NULL, then the changeset is returned to the caller by invoking
** xOutput zero or more times and SQLITE_OK returned. Or, if xOutput is NULL,
** then (*ppOut) is set to point to a buffer containing the rebased changeset
** before this function returns. In this case (*pnOut) is set to the size of
** the buffer in bytes.  It is the responsibility of the caller to eventually
** free the (*ppOut) buffer using sqlite3_free().
**
** If an error occurs, an SQLite error code is returned. If ppOut and
** pnOut are not NULL, then the two output parameters are set to 0 before
** returning.
*/
static int sessionRebase(
  sqlite3_rebaser *p,             /* Rebaser hash table */
  sqlite3_changeset_iter *pIter,  /* Input data */
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut,                     /* Context for xOutput callback */
  int *pnOut,                     /* OUT: Number of bytes in output changeset */
  void **ppOut                    /* OUT: Inverse of pChangeset */
){
  int rc = SQLITE_OK;
  u8 *aRec = 0;
  int nRec = 0;
  int bNew = 0;
  SessionTable *pTab = 0;
  SessionBuffer sOut = {0,0,0};

  while( SQLITE_ROW==sessionChangesetNext(pIter, &aRec, &nRec, &bNew) ){
    SessionChange *pChange = 0;
    int bDone = 0;

    if( bNew ){
      const char *zTab = pIter->zTab;
      for(pTab=p->grp.pList; pTab; pTab=pTab->pNext){
        if( 0==sqlite3_stricmp(pTab->zName, zTab) ) break;
      }
      bNew = 0;

      /* A patchset may not be rebased */
      if( pIter->bPatchset ){
        rc = SQLITE_ERROR;
      }

      /* Append a table header to the output for this new table */
      sessionAppendByte(&sOut, pIter->bPatchset ? 'P' : 'T', &rc);
      sessionAppendVarint(&sOut, pIter->nCol, &rc);
      sessionAppendBlob(&sOut, pIter->abPK, pIter->nCol, &rc);
      sessionAppendBlob(&sOut,(u8*)pIter->zTab,(int)strlen(pIter->zTab)+1,&rc);
    }

    if( pTab && rc==SQLITE_OK ){
      int iHash = sessionChangeHash(pTab, 0, aRec, pTab->nChange);

      for(pChange=pTab->apChange[iHash]; pChange; pChange=pChange->pNext){
        if( sessionChangeEqual(pTab, 0, aRec, 0, pChange->aRecord) ){
          break;
        }
      }
    }

    if( pChange ){
      assert( pChange->op==SQLITE_DELETE || pChange->op==SQLITE_INSERT );
      switch( pIter->op ){
        case SQLITE_INSERT:
          if( pChange->op==SQLITE_INSERT ){
            bDone = 1;
            if( pChange->bIndirect==0 ){
              sessionAppendByte(&sOut, SQLITE_UPDATE, &rc);
              sessionAppendByte(&sOut, pIter->bIndirect, &rc);
              sessionAppendBlob(&sOut, pChange->aRecord, pChange->nRecord, &rc);
              sessionAppendBlob(&sOut, aRec, nRec, &rc);
            }
          }
          break;

        case SQLITE_UPDATE:
          bDone = 1;
          if( pChange->op==SQLITE_DELETE ){
            if( pChange->bIndirect==0 ){
              u8 *pCsr = aRec;
              sessionSkipRecord(&pCsr, pIter->nCol);
              sessionAppendByte(&sOut, SQLITE_INSERT, &rc);
              sessionAppendByte(&sOut, pIter->bIndirect, &rc);
              sessionAppendRecordMerge(&sOut, pIter->nCol,
                  pCsr, nRec-(pCsr-aRec),
                  pChange->aRecord, pChange->nRecord, &rc
              );
            }
          }else{
            sessionAppendPartialUpdate(&sOut, pIter,
                aRec, nRec, pChange->aRecord, pChange->nRecord, &rc
            );
          }
          break;

        default:
          assert( pIter->op==SQLITE_DELETE );
          bDone = 1;
          if( pChange->op==SQLITE_INSERT ){
            sessionAppendByte(&sOut, SQLITE_DELETE, &rc);
            sessionAppendByte(&sOut, pIter->bIndirect, &rc);
            sessionAppendRecordMerge(&sOut, pIter->nCol,
                pChange->aRecord, pChange->nRecord, aRec, nRec, &rc
            );
          }
          break;
      }
    }

    if( bDone==0 ){
      sessionAppendByte(&sOut, pIter->op, &rc);
      sessionAppendByte(&sOut, pIter->bIndirect, &rc);
      sessionAppendBlob(&sOut, aRec, nRec, &rc);
    }
    if( rc==SQLITE_OK && xOutput && sOut.nBuf>sessions_strm_chunk_size ){
      rc = xOutput(pOut, sOut.aBuf, sOut.nBuf);
      sOut.nBuf = 0;
    }
    if( rc ) break;
  }

  if( rc!=SQLITE_OK ){
    sqlite3_free(sOut.aBuf);
    memset(&sOut, 0, sizeof(sOut));
  }

  if( rc==SQLITE_OK ){
    if( xOutput ){
      if( sOut.nBuf>0 ){
        rc = xOutput(pOut, sOut.aBuf, sOut.nBuf);
      }
    }else if( ppOut ){
      *ppOut = (void*)sOut.aBuf;
      *pnOut = sOut.nBuf;
      sOut.aBuf = 0;
    }
  }
  sqlite3_free(sOut.aBuf);
  return rc;
}

/*
** Create a new rebaser object.
*/
SQLITE_API int sqlite3rebaser_create(sqlite3_rebaser **ppNew){
  int rc = SQLITE_OK;
  sqlite3_rebaser *pNew;

  pNew = sqlite3_malloc(sizeof(sqlite3_rebaser));
  if( pNew==0 ){
    rc = SQLITE_NOMEM;
  }else{
    memset(pNew, 0, sizeof(sqlite3_rebaser));
  }
  *ppNew = pNew;
  return rc;
}

/*
** Call this one or more times to configure a rebaser.
*/
SQLITE_API int sqlite3rebaser_configure(
  sqlite3_rebaser *p,
  int nRebase, const void *pRebase
){
  sqlite3_changeset_iter *pIter = 0;   /* Iterator opened on pData/nData */
  int rc;                              /* Return code */
  rc = sqlite3changeset_start(&pIter, nRebase, (void*)pRebase);
  if( rc==SQLITE_OK ){
    rc = sessionChangesetToHash(pIter, &p->grp, 1);
  }
  sqlite3changeset_finalize(pIter);
  return rc;
}

/*
** Rebase a changeset according to current rebaser configuration
*/
SQLITE_API int sqlite3rebaser_rebase(
  sqlite3_rebaser *p,
  int nIn, const void *pIn,
  int *pnOut, void **ppOut
){
  sqlite3_changeset_iter *pIter = 0;   /* Iterator to skip through input */
  int rc = sqlite3changeset_start(&pIter, nIn, (void*)pIn);

  if( rc==SQLITE_OK ){
    rc = sessionRebase(p, pIter, 0, 0, pnOut, ppOut);
    sqlite3changeset_finalize(pIter);
  }

  return rc;
}

/*
** Rebase a changeset according to current rebaser configuration
*/
SQLITE_API int sqlite3rebaser_rebase_strm(
  sqlite3_rebaser *p,
  int (*xInput)(void *pIn, void *pData, int *pnData),
  void *pIn,
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut
){
  sqlite3_changeset_iter *pIter = 0;   /* Iterator to skip through input */
  int rc = sqlite3changeset_start_strm(&pIter, xInput, pIn);

  if( rc==SQLITE_OK ){
    rc = sessionRebase(p, pIter, xOutput, pOut, 0, 0);
    sqlite3changeset_finalize(pIter);
  }

  return rc;
}

/*
** Destroy a rebaser object
*/
SQLITE_API void sqlite3rebaser_delete(sqlite3_rebaser *p){
  if( p ){
    sessionDeleteTable(0, p->grp.pList);
    sqlite3_free(p->grp.rec.aBuf);
    sqlite3_free(p);
  }
}

/*
** Global configuration
*/
SQLITE_API int sqlite3session_config(int op, void *pArg){
  int rc = SQLITE_OK;
  switch( op ){
    case SQLITE_SESSION_CONFIG_STRMSIZE: {
      int *pInt = (int*)pArg;
      if( *pInt>0 ){
        sessions_strm_chunk_size = *pInt;
      }
      *pInt = sessions_strm_chunk_size;
      break;
    }
    default:
      rc = SQLITE_MISUSE;
      break;
  }
  return rc;
}

/*
** Begin adding a change to a changegroup object.
*/
SQLITE_API int sqlite3changegroup_change_begin(
  sqlite3_changegroup *pGrp,
  int eOp,
  const char *zTab,
  int bIndirect,
  char **pzErr
){
  SessionTable *pTab = 0;
  int rc = SQLITE_OK;

  if( pGrp->cd.pTab ){
    rc = SQLITE_MISUSE;
  }else if( eOp!=SQLITE_INSERT && eOp!=SQLITE_UPDATE && eOp!=SQLITE_DELETE ){
    rc = SQLITE_ERROR;
  }else{
    rc = sessionChangesetFindTable(pGrp, zTab, 0, &pTab);
  }
  if( rc==SQLITE_OK ){
    if( pTab==0 ){
      if( pzErr ){
        *pzErr = sqlite3_mprintf("no such table: %s", zTab);
      }
      rc = SQLITE_ERROR;
    }else{
      int nReq = pTab->nCol * (eOp==SQLITE_UPDATE ? 2 : 1);
      pGrp->cd.pTab = pTab;
      pGrp->cd.eOp = eOp;
      pGrp->cd.bIndirect = bIndirect;

      if( pGrp->cd.nBufAlloc<nReq ){
        SessionBuffer *aBuf = (SessionBuffer*)sqlite3_realloc(
            pGrp->cd.aBuf, nReq * sizeof(SessionBuffer)
        );
        if( aBuf==0 ){
          rc = SQLITE_NOMEM;
        }else{
          memset(&aBuf[pGrp->cd.nBufAlloc], 0,
              sizeof(SessionBuffer) * (nReq - pGrp->cd.nBufAlloc)
          );
          pGrp->cd.aBuf = aBuf;
          pGrp->cd.nBufAlloc = nReq;
        }
      }

#ifdef SQLITE_DEBUG
      {
        /* Assert that all column values are currently undefined */
        int ii;
        for(ii=0; ii<pGrp->cd.nBufAlloc; ii++){
          assert( pGrp->cd.aBuf[ii].nBuf==0 );
        }
      }
#endif
    }
  }

  return rc;
}

/*
** This function does processing common to the _change_int64(), _change_text()
** and other similar APIs.
*/
static int checkChangeParams(
  sqlite3_changegroup *pGrp,
  int bNew,
  int iCol,
  sqlite3_int64 nReq,
  SessionBuffer **ppBuf
){
  int rc = SQLITE_OK;
  if( pGrp->cd.pTab==0 ){
    rc = SQLITE_MISUSE;
  }else if( iCol<0 || iCol>=pGrp->cd.pTab->nCol ){
    rc = SQLITE_RANGE;
  }else if(
      (bNew && pGrp->cd.eOp==SQLITE_DELETE)
   || (!bNew && pGrp->cd.eOp==SQLITE_INSERT)
  ){
    rc = SQLITE_ERROR;
  }else{
    SessionBuffer *pBuf = &pGrp->cd.aBuf[iCol];
    if( pGrp->cd.eOp==SQLITE_UPDATE && bNew ){
      pBuf += pGrp->cd.pTab->nCol;
    }
    pBuf->nBuf = 0;
    sessionBufferGrow(pBuf, nReq, &rc);
    pBuf->nBuf = nReq;
    *ppBuf = pBuf;
  }
  return rc;
}

/*
** Configure the change currently under construction with an integer value.
*/
SQLITE_API int sqlite3changegroup_change_int64(
  sqlite3_changegroup *pGrp,
  int bNew,
  int iCol,
  sqlite3_int64 iVal
){
  int rc = SQLITE_OK;
  SessionBuffer *pBuf = 0;

  if( SQLITE_OK!=(rc = checkChangeParams(pGrp, bNew, iCol, 9, &pBuf)) ){
    return rc;
  }

  pBuf->aBuf[0] = SQLITE_INTEGER;
  sessionPutI64(&pBuf->aBuf[1], iVal);
  return SQLITE_OK;
}

/*
** Configure the change currently under construction with a null value.
*/
SQLITE_API int sqlite3changegroup_change_null(
  sqlite3_changegroup *pGrp,
  int bNew,
  int iCol
){
  int rc = SQLITE_OK;
  SessionBuffer *pBuf = 0;

  if( SQLITE_OK!=(rc = checkChangeParams(pGrp, bNew, iCol, 1, &pBuf)) ){
    return rc;
  }

  pBuf->aBuf[0] = SQLITE_NULL;
  return SQLITE_OK;
}

/*
** Configure the change currently under construction with a real value.
*/
SQLITE_API int sqlite3changegroup_change_double(
  sqlite3_changegroup *pGrp,
  int bNew,
  int iCol,
  double fVal
){
  int rc = SQLITE_OK;
  SessionBuffer *pBuf = 0;

  if( SQLITE_OK!=(rc = checkChangeParams(pGrp, bNew, iCol, 9, &pBuf)) ){
    return rc;
  }

  pBuf->aBuf[0] = SQLITE_FLOAT;
  sessionPutDouble(&pBuf->aBuf[1], fVal);
  return SQLITE_OK;
}

/*
** Configure the change currently under construction with a text value.
*/
SQLITE_API int sqlite3changegroup_change_text(
  sqlite3_changegroup *pGrp,
  int bNew,
  int iCol,
  const char *pVal,
  int nVal
){
  int nText = nVal>=0 ? nVal : strlen(pVal);
  sqlite3_int64 nByte = 1 + sessionVarintLen(nText) + nText;
  int rc = SQLITE_OK;
  SessionBuffer *pBuf = 0;

  if( SQLITE_OK!=(rc = checkChangeParams(pGrp, bNew, iCol, nByte, &pBuf)) ){
    return rc;
  }

  pBuf->aBuf[0] = SQLITE_TEXT;
  pBuf->nBuf = (1 + sessionVarintPut(&pBuf->aBuf[1], nText));
  memcpy(&pBuf->aBuf[pBuf->nBuf], pVal, nText);
  pBuf->nBuf += nText;

  return SQLITE_OK;
}

/*
** Configure the change currently under construction with a blob value.
*/
SQLITE_API int sqlite3changegroup_change_blob(
  sqlite3_changegroup *pGrp,
  int bNew,
  int iCol,
  const void *pVal,
  int nVal
){
  sqlite3_int64 nByte = 1 + sessionVarintLen(nVal) + nVal;
  int rc = SQLITE_OK;
  SessionBuffer *pBuf = 0;

  if( SQLITE_OK!=(rc = checkChangeParams(pGrp, bNew, iCol, nByte, &pBuf)) ){
    return rc;
  }

  pBuf->aBuf[0] = SQLITE_BLOB;
  pBuf->nBuf = (1 + sessionVarintPut(&pBuf->aBuf[1], nVal));
  memcpy(&pBuf->aBuf[pBuf->nBuf], pVal, nVal);
  pBuf->nBuf += nVal;

  return SQLITE_OK;
}

/*
** Finish any change currently being constructed by the changegroup object.
*/
SQLITE_API int sqlite3changegroup_change_finish(
  sqlite3_changegroup *pGrp,
  int bDiscard,
  char **pzErr
){
  int rc = SQLITE_OK;
  if( pGrp->cd.pTab ){
    SessionBuffer *aBuf = pGrp->cd.aBuf;
    int ii;

    if( bDiscard==0 ){
      int nBuf = pGrp->cd.pTab->nCol;
      u8 eUndef = SQLITE_NULL;
      if( pGrp->cd.eOp==SQLITE_UPDATE ){
        for(ii=0; ii<nBuf; ii++){
          if( pGrp->cd.pTab->abPK[ii] ){
            if( aBuf[ii].nBuf<=1 ){
              *pzErr = sqlite3_mprintf(
                  "invalid change: %s value in PK of old.* record",
                  aBuf[ii].nBuf==1 ? "null" : "undefined"
              );
              rc = SQLITE_ERROR;
              break;
            }else if( aBuf[ii + nBuf].nBuf>0 ){
              *pzErr = sqlite3_mprintf(
                  "invalid change: defined value in PK of new.* record"
              );
              rc = SQLITE_ERROR;
              break;
            }
          }else
          if( pGrp->bPatch==0 && (aBuf[ii].nBuf>0)!=(aBuf[ii+nBuf].nBuf>0) ){
            *pzErr = sqlite3_mprintf(
                "invalid change: column %d "
                "- old.* value is %sdefined but new.* is %sdefined",
                ii, aBuf[ii].nBuf ? "" : "un", aBuf[ii+nBuf].nBuf ? "" : "un"
            );
            rc = SQLITE_ERROR;
            break;
          }
        }
        eUndef = 0x00;
        if( pGrp->bPatch==0 ) nBuf = nBuf * 2;
      }else{
        for(ii=0; ii<nBuf; ii++){
          int isPK = pGrp->cd.pTab->abPK[ii];
          if( (pGrp->cd.eOp==SQLITE_INSERT || pGrp->bPatch==0 || isPK)
           && aBuf[ii].nBuf==0
          ){
            *pzErr = sqlite3_mprintf(
                "invalid change: column %d is undefined", ii
            );
            rc = SQLITE_ERROR;
            break;
          }
          if( aBuf[ii].nBuf==1 && isPK ){
            *pzErr = sqlite3_mprintf(
                "invalid change: null value in PK"
            );
            rc = SQLITE_ERROR;
            break;
          }
        }
      }

      pGrp->cd.record.nBuf = 0;
      for(ii=0; ii<nBuf; ii++){
        SessionBuffer *p = &pGrp->cd.aBuf[ii];
        if( pGrp->bPatch ){
          if( pGrp->cd.pTab->abPK[ii]==0 ){
            if( pGrp->cd.eOp==SQLITE_UPDATE ){
              p += pGrp->cd.pTab->nCol;
            }else if( pGrp->cd.eOp==SQLITE_DELETE ){
              continue;
            }
          }
        }
        if( 0==sessionBufferGrow(&pGrp->cd.record, p->nBuf?p->nBuf:1, &rc) ){
          if( p->nBuf ){
            memcpy(&pGrp->cd.record.aBuf[pGrp->cd.record.nBuf],p->aBuf,p->nBuf);
            pGrp->cd.record.nBuf += p->nBuf;
          }else{
            pGrp->cd.record.aBuf[pGrp->cd.record.nBuf++] = eUndef;
          }
        }
      }
      if( rc==SQLITE_OK ){
        rc = sessionOneChangeToHash(
            pGrp, pGrp->cd.pTab,
            pGrp->cd.eOp, pGrp->cd.bIndirect, pGrp->cd.pTab->nCol,
            pGrp->cd.record.aBuf, pGrp->cd.record.nBuf, 0
        );
      }
    }

    /* Reset all aBuf[] entries to "undefined". */
    {
      int nZero = pGrp->cd.pTab->nCol;
      if( pGrp->cd.eOp==SQLITE_UPDATE ) nZero += nZero;
      for(ii=0; ii<nZero; ii++){
        pGrp->cd.aBuf[ii].nBuf = 0;
      }
    }
    pGrp->cd.pTab = 0;
  }

  return rc;
}

#endif /* SQLITE_ENABLE_SESSION && SQLITE_ENABLE_PREUPDATE_HOOK */

/************** End of sqlite3session.c **************************************/
