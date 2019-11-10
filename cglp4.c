/* cglp4.c               Thomas R. Nicely          2018.10.05.0010
 *                    http://www.trnicely.net
 *
 * Freeware copyright (c) 2018 Thomas R. Nicely <http://www.trnicely.net>.
 * Released into the public domain by the author, who disclaims any legal
 * liability arising from its use.
 *
 * Checks prime gap listings (in standard format) for
 * validity, using the strong Baillie-PSW test; see
 * <http://www.trnicely.net/misc/bpsw.html> for details.
 *
 * Must be linked with support routines defined in "trn.h"
 * and "trn.c"; i.e., compile with a command such as
 *
 *   gcc cglp4.c trn.c [conio3.c] -lm -lgmp -D__NOMPFR__
 *
 * with cglp4.c, trn.c, and trn.h in the search path. See
 * "trn.c" for the source code of routines such as iPrP,
 * iMillerRabin, and iGetGapRecExt. For platforms other
 * than DJGPP and Borland, conio3.c must also be compiled
 * and linked to provide support for the conio routines
 * (gotoxy, wherex, etc.). GMP is required, but MPFR is not
 * used.
 *
 * SYNTAX:
 *
 * cglp4 infile [x|m|M|z|b] [mingap maxgap] [mindig maxdig] [n]
 *
 * where infile is the input data file; only gaps within
 * mingap <= gap <= maxgap and mindig <= digits <= maxdig are checked.
 * If the first argument following infile is "x" or "m", only the
 * endpoints (not the interiors) of the gaps will be checked; if
 * "M" or "m", only a single base-2 Miller-Rabin test will be used
 * (along with small prime divisors) to check primality. ); if x or b is
 * specified, _all_ endpoints will be checked with the full BPSW test.
 * If the z parameter is given, no checking is done; instead a file
 * nocheck.dat is created, containing * the gaps (in standard format)
 * which _would have been_ checked. The optional final parameter n
 * will suppress reporting of intermediate results to the screen.
 *
 * If maxgap=0 is specified, ulMaxGap=ulMinGap will be assumed.
 *
 * Note that if the command-line executable of the primality tester
 * PFGW (PrimeForm/GW, written by Yves Gallot and George Woltman)
 * is present in the file c:\pfgw\pfgw32.exe (or c:\pfgw\pfgw64.exe
 * on a 64-bit system), this code will attempt to call it as a
 * "helper" code when testing interior points of very large gaps
 * (1200 digits or more). The code has been tested specifically
 * with PFGW v3.5.7 (November 2011). For further details of PFGW,
 * see <http://sourceforge.net/projects/openpfgw/files/>. Use of
 * PFGW may be disabled by temporarily renaming its executable files
 * and/or directory, or by setting the environment variable NOPFGW.
 *
 * INPUT: Each line of the ASCII text input file is presumed to
 * have the following format.
 *
 * Col 1-6......g
 * Col 41-xxx...p1
 *
 * or
 *
 * Col 1-9......g
 * Col 47-xxx...p1
 *
 * or else (for continuation lines)
 *
 * Col 1-40.....Blanks
 * Col 41-xxx...p1 continued
 *
 * Continuation of p to succeeding lines is to be indicated by a
 * trailing backslash, underscore, or tilde. The line may have
 * trailing whitespace and a continuation character.
 *
 * The alternate format is simply
 *
 * gggg  pppppppppp
 * gggg  pppppppppp
 * .....
 *
 * without line continuation.
 *
 * OUTPUT:
 *
 * (1) Confirmations, errors, and elapsed run time are printed
 *     to the screen. Serious errors result in pauses and prompts.
 * (2) Analysis is printed to the output file cglp4.out.
 *
 * NOTES:
 *
 * (1) The maximum gap and integer size allowed are determined by
 * available memory. The default limits are 300K digits and gaps
 * to 3M. Worst case (approximate): 30K digits, gaps
 * to 80K. Best case: 3.8M digits, gaps to 10M. However, gaps
 * beyond about 2K digits and measure 100K may require *VERY*
 * long run times. If endpoints only are checked, the gap size
 * is irrelevant, but the practical limit for the bounding
 * primes will still be a few thousand digits.
 *
 * (2) The use of mpz_init2 is intended to work around bugs in GMP's
 * memory allocation algorithms by implementing quasi-static memory
 * management.
 *
 * (3) If the endpoints option is not invoked, and the environmental
 * variable INTERIOR is set to a non-null value, only the interiors
 * of the gaps will be tested (for compositeness); the endpoints will
 * not be tested for primality.
 *
 * (4) If the environmental variable CGLP4_BACKUP is non-null, periodic
 * backups of intermediate data for an individual gap are recorded
 * in the file CGLP4.BAK, allowing the code to be interrupted by
 * Control-Break and then later resumed at the interruption point.
 * The backup interval is 60 seconds by default; it may be set
 * externally through the environmental variable CGLP4BUI. The backup
 * feature is intended for use with input files containing a single
 * gap with a great many digits (thousands) in P1 and/or a very
 * large gap size (tens of thousands), to prevent the waste of
 * computations interrupted intentionally or accidentally. The sieve
 * array is also backed up, in the file CGLP4.SIV. Backups do not
 * apply to end-point only computations.
 *
 * (5) The level of probabilistic primality testing (number of
 * repetitions of Miller-Rabin and Lucas tests) can be controlled
 * in either of two ways: (A) Change the value of DEFAULT_BASES
 * and recompile, or (B) Set the environment variable MRREPS to
 * the desired value. The variables DEFAULT_BASES and MRREPS
 * will determine the number of Miller-Rabin tests used to test
 * the endpoint integers for primality; in addition, a proportional
 * number of strong and/or extra strong Lucas tests may be applied
 * (see the routine iPrP in trn.c for specifics).
 *
 * (6) By default, trial prime divisors to 65519 are used for sieving.
 * A greater upper bound is chosen empirically for gaps with more
 * than 300 digits, and may be as large as 1e8.
 *
 * (7) This is a command-line (shell) console application, optimized
 * for a window size of at least 80x25. Some real-time screen messages
 * may be truncated in smaller windows.
 *
 * (8) The non-standard "cputs" is used in an effort to speed up
 * screen updating.
 *
 * (9) Must be linked with GMP, but MPFR is not used.
 *
 */

#if !defined(_TRN_H_)
  #include "trn.h"
#endif

#if !defined(__DJGPP__) && !defined(__BORLANDC__) && !defined(_CONIO3_H_)
  #include "conio3.h"
#endif

#define DEFAULT_BASES 1  /* Number of Miller-Rabin tests in BPSW */
#define OUTFILE "cglp4.out"
#define NOCHECKFILE "nocheck.dat"
#define BACKUP_INTERVAL 60
#define MIN_PFGW_DIGITS 1000
#define MIN_MR2_DIGITS 1000000UL  /* This feature thus currently disabled */

static unsigned long    mpz_gap(void);
static unsigned long    mpz_gap_pfgw(void);
static void             vSieve2(void);
static void             vSyntax(void);

/* Static declarations keeps functions private, prevents linker clashes. */

extern           unsigned long ulPrime16[];  /* 16-bit primes; see trn.c */

char      sz[256], sz2[256], sz3[256], ch, szSieveFile[256],
		 szDivFile[256], szBackupFile[256], *szBuffer, szPFGW[128];
uint8_t   *uchSieve;
int       iMR2Base=0, iBackup=0, iSpecial=0, iSW=79,
		 iCheckSieve=0, ix, iy, iInsideGap=0, iBackupAll=0,
		 iNoCheck=0, i, iNCGaps=0, iScreen=1, iNumArgs, iPFGW=0,
		 iMR2ThisGap, iAllBPSW;
unsigned long    ulD1;
unsigned long
		 ulGap, ulMD, ulMG, ulP1Rem, ulGBack=2,
		 ulMRReps=DEFAULT_BASES, *ulLPD;
double    lft0, lfLastBackupTime, lfBUI=BACKUP_INTERVAL, lfTstart,
		 dt;
FILE      *fpIn, *fpOut, *fpBackup, *fpSieve, *fp, *fpDiv, *fpNoCheck;
mpz_t		 mpzP1, mpzP2, mpz, mpzRem, mpzTwo, mpzD, mpzR;

static const char signature[]=
  "\n __cglp4.c__Version 2018.10.05.0010__Freeware copyright (c) 2018"
  "\n Thomas R. Nicely <http://www.trnicely.net>. Released into the"
  "\n public domain by the author, who disclaims any legal liability"
  "\n arising from its use.\n";

/**********************************************************************/
int main(int argc, char *argv[])
{
char             *szP1, *szP2, *szScratch, *pch, *ep, *ep2, ch,
		 szP1tt[256], szP1t[256], szP1t2[256], szCmd[512], *pch2,
                  *szInBuffer;
long		 iStat, iEPO=0, iInterior=0, iG, iLen, ibs;
unsigned long    lfMem, ulMinGap, ulMaxGap, ulMinDigits, ulMaxDigits,
		 ulLen, ulD2, ulD1a, ulGap2, ulOK=0, ulErrors=0, ulFactor,
		 ulChunk, ul;
double           t1, t2, lf;

lft0=lfSeconds2();
lfTstart=lft0;

if(argc==1)
  {
  vSyntax();
  exit(EXIT_FAILURE);
  }

iLen=__lRFile(argv[1]);  /* input file size */
lfMem=lfPMA();
ibs=__MIN2(iLen + 3, lfMem/4);
szInBuffer=(char *)malloc(ibs);
fpIn=fopen(argv[1], "rt");
if(!fpIn)
  {
  fprintf(stderr,
    "\n ERROR: Unable to open specified input file %s.", argv[1]);
  exit(EXIT_FAILURE);
  }
iStat=setvbuf(fpIn, szInBuffer, _IOFBF, ibs);  /* fully buffer input file */

fpOut=fopen(OUTFILE, "at");
if(!fpOut)
  {
  fprintf(stderr, "\n ERROR: Unable to open output file %s.", OUTFILE);
  exit(EXIT_FAILURE);
  }
else
  {
  fprintf(fpOut, "=======================================");
  fprintf(fpOut, "======================================\n");
  fclose(fpOut);
  }
fprintf(stdout, "\n");

/* The following command-line parsing logic is somewhat convoluted
   in an effort to simplify the user input syntax. This section
   is marked for eventual improvement. */

ulMinGap=0; ulMaxGap=10*__MAX_DIGITS__;
ulMinDigits=0; ulMaxDigits=__MAX_DIGITS__;
iAllBPSW=0;
iMR2Base=0;
iNumArgs=argc;  /* Includes executable name */
if((*argv[argc-1]=='n') || (*argv[argc-1]=='N'))
  {
  iScreen=0;
  iNumArgs=argc - 1;
  }
if(iNumArgs > 2)
  {
  ch=*argv[2];  /* First character of argv[2] */
  if((ch=='m') || (ch=='M'))iMR2Base=1;
  if((ch=='x') || (ch=='X') || (ch=='b') || (ch=='B'))iAllBPSW=1;
  if((ch=='x') || (ch=='X'))iEPO=1;
  if((ch=='b') || (ch=='B'))iEPO=0;
  if((ch=='z') || (ch=='Z'))
    {
    iNoCheck=1;
    fpNoCheck=fopen(NOCHECKFILE, "wt");
    if(!fpNoCheck)
      {
      fprintf(stderr, "\n ERROR: Unable to open nocheck file %s.",
	NOCHECKFILE);
      exit(EXIT_FAILURE);
      }
    }
  if((ch=='x') || (ch=='z') || (ch=='m') || (ch=='X') || (ch=='Z'))
    {
    iEPO=1;
    if(iNumArgs > 3)ulMinGap=strtoul(argv[3], NULL, 0);
    if(iNumArgs > 4)ulMaxGap=strtoul(argv[4], NULL, 0);
    if(ulMaxGap==0)ulMaxGap=ulMinGap;
    if(iNumArgs > 5)ulMinDigits=strtoul(argv[5], NULL, 0);
    if(iNumArgs > 6)ulMaxDigits=strtoul(argv[6], NULL, 0);
    if(ulMinGap > ulMaxGap)ulMinGap=ulMaxGap;
    if(ulMinDigits > ulMaxDigits)ulMinDigits=ulMaxDigits;
    ulMRReps=1;
    }
  else if((ch != 'M') && (ch != 'b') && (ch != 'B'))
    {
    iEPO=0;
    ulMinGap=strtoul(argv[2], NULL, 0);
    ulMaxGap=ulMinGap;
    if(iNumArgs > 3)ulMaxGap=strtoul(argv[3], NULL, 0);
    if(ulMaxGap==0)ulMaxGap=ulMinGap;
    if(iNumArgs > 4)ulMinDigits=strtoul(argv[4], NULL, 0);
    if(iNumArgs > 5)ulMaxDigits=strtoul(argv[5], NULL, 0);
    if(ulMinGap > ulMaxGap)ulMinGap=ulMaxGap;
    if(ulMinDigits > ulMaxDigits)ulMinDigits=ulMaxDigits;
    ulMRReps=DEFAULT_BASES;
    }
  }
if(iEPO)ulMRReps=1;
pch=getenv("MRREPS");
if(pch!=NULL)ulMRReps=strtoul(pch,NULL,0);
if(ulMRReps > 999)ulMRReps=999;
pch=getenv("INTERIOR");
if(!iEPO && (pch!=NULL))iInterior=1;
if(ulMRReps==0)ulMRReps=DEFAULT_BASES;
pch=getenv("CGLP4_BACKUP");
if(pch!=NULL)iBackupAll=1;
pch=getenv("CGLP4BUI");
if(pch!=NULL)lfBUI=atof(pch);
if(lfBUI < 5)lfBUI=5;
pch=getenv("SPECIAL");
if(pch!=NULL)iSpecial=1;
pch=getenv("CHECK_SIEVE");
if(pch!=NULL)iCheckSieve=1;

/* Allocate memory for the major mpz's, arrays, and buffers. ulMD is
   the maximum number of decimal digits in P1 and P2 for which memory
   will be reserved. ulMG is the maximum gap size for which memory
   will be reserved in the sieve and divisor arrays. */

ulMD=__MAX_DIGITS__;  /* See trn.h */
mpz_init(mpzP1);
mpz_init(mpzP2);
mpz_init(mpz);
mpz_init(mpzRem);
mpz_init(mpzD);
mpz_init(mpzR);
mpz_init_set_ui(mpzTwo, 2);
szBuffer=(char *)malloc(ulMD);
szP1=(char *)malloc(ulMD);
szP2=(char *)malloc(ulMD);
szScratch=NULL;
szScratch=(char *)malloc(ulMD);
if(!szScratch)  /* This verifies all allocations to this point */
  {
  fprintf(stderr,
    "\n ERROR: Unable to allocate szScratch array (%lu bytes).\n", ulMD);
  exit(EXIT_FAILURE);
  }

/* Check for the presence of the Windows command-line pfgw code, 32-bit
   or 64-bit. */

iPFGW=0;
szPFGW[0]=0;
pch=getenv("NOPFGW");  /* Is PFGW excluded by the environment? */
if(pch!=NULL)goto PFGW_CHECKED;

if(__lRFile("c:\\windows\\SysWOW64\\winver.exe") > 0)
  {
  if(__lRFile("pfgw64.exe") > 0)
    {
    iPFGW=2;
    strcpy(szPFGW,
      "pfgw64 -k -r -f0 -e1 -r -u0 -Cquiet ");
    }
  else if(__lRFile("c:\\pfgw\\pfgw64.exe") > 0)
    {
    iPFGW=2;
    strcpy(szPFGW,
      "c:\\pfgw\\pfgw64 -k -r -f0 -e1 -r -u0 -Cquiet ");
    }
  }

if(!iPFGW)  /* 64-bit PFGW not found */
  {
  if(__lRFile("pfgw32.exe") > 0)
    {
    iPFGW=1;
    strcpy(szPFGW,
      "pfgw32 -k -r -f0 -e1 -r -u0 -Cquiet ");
    }
  else if(__lRFile("c:\\pfgw\\pfgw32.exe") > 0)
    {
    iPFGW=1;
    strcpy(szPFGW,
      "c:\\pfgw\\pfgw32 -k -r -f0 -e1 -r -u0 -Cquiet ");
    }
  }

PFGW_CHECKED: ;

ulMG=__MAX2(ulMaxGap, 10*__MAX_DIGITS__);
if(!iEPO)
  {
  uchSieve=NULL;  /* sieving array */
  uchSieve=(uint8_t *)malloc(ulMG);
  if(!uchSieve)
    {
    fprintf(stderr,
     "\n ERROR: Unable to allocate uchSieve array (%lu bytes).\n", ulMG);
    exit(EXIT_FAILURE);
    }
  ulLPD=NULL;  /* array of least positive divisors */
  ulLPD=(unsigned long *)malloc(4*ulMG);
  if(!ulLPD)
    {
    fprintf(stderr,
      "\n ERROR: Unable to allocate ulLPD array (%lu bytes).\n", 4*ulMG);
    exit(EXIT_FAILURE);
    }
  }

vGenPrimes16();  /* Initializes ulPrime16[] for use in vSieve2 */

/* Save the command line */

strcpy(szCmd, argv[0]);
for(i=1; i < argc; i++)
  {
  strcat(szCmd, " ");
  strcat(szCmd, argv[i]);
  }
if(iScreen)
  {
  __clearline();
  printf("...Searching for specified gaps...");
  }

while(1)  /* Check the candidate gaps one at a time */
  {
  iMR2ThisGap=iMR2Base;
  if(!iBackupAll)iBackup=0;
  iStat=iGetGapRecExt(szBuffer, fpIn);
  if(!iStat)
    {
    if(feof(fpIn))break;
    continue;
    }
  iG=atol(szBuffer);  /* Check for valid gap measure iG */
  if(iG <= 0)continue;
  if(iG & 1)if(iG != 1)continue;
  ulGap=strtoul(szBuffer, &ep, 10);  /* To locate ep for later use */
  if((ulGap != iG) || (ulGap > 999999999UL))continue;
  if(iStat==6)  // Standard format gap6 entry
    {
    ulD1=strtoul(szBuffer+32, NULL, 10);
    strcpy(szP1, szBuffer+40);
    }
  else if(iStat==9)  // Standard format gap9 entry
    {
    ulD1=strtoul(szBuffer+37, NULL, 10);
    strcpy(szP1, szBuffer+47);
    }
  else if(iStat==1)  // G and P1 only in entry (type 1)
    {
    szTrimMWS(ep);
    pch=strstr(ep, "  ");
    if(pch)*pch=0;
    strcpy(szP1, ep);
    szTrimMWS(szP1);
    if(mpz_set_str(mpzP1, szP1, 10))  /* is it a literal? */
      if(iEvalExpr(mpzP1, szP1))  /* is it a formula? */
	{
	sprintf(sz, "G=%7lu P1=%-20s ERROR: Unable to parse P1.\n",
	  ulGap, szP1tt);
	fprintf(stderr, "%s", sz);
	fpOut=fopen(OUTFILE, "at");
	fprintf(fpOut, sz);
	fclose(fpOut);
	continue;
	}
    ulD1=strlen(mpz_get_str(szScratch, 10, mpzP1));
    }
  else  /* Invalid or unrecognized record */
    continue;
  if(ulGap < ulMinGap)continue;
  if(ulGap > ulMaxGap)break;
  sprintf(szSieveFile, "g%lu.siv", ulGap);
  sprintf(szDivFile, "g%lu.div", ulGap);
  sprintf(szBackupFile, "g%lu.bak", ulGap);
  if(((ulD1 >= MIN_MR2_DIGITS) && (iAllBPSW==0)) || (iMR2Base==1))
    iMR2ThisGap=1;
  else
    iMR2ThisGap=0;
  if(ulD1 > 1999)iBackup=1;
  if(ulGap > 99999UL)iBackup=1;
  if(strstr(szP1, ".."))continue;  /* Ellipsed prime---skip */
  if((ulGap < ulMinGap) || (ulGap > ulMaxGap))continue;
  if((ulD1 < ulMinDigits) || (ulD1 > ulMaxDigits))continue;
  /* Make ellipsed forms of P1 for reporting */
  szTrimMWS(szP1);
  ulLen=strlen(szP1);
  if(ulLen > 20)
    {
    sprintf(sz, "..(%luD)..", ulD1);
    strncpy(szP1tt, szP1, 20-strlen(sz));
    szP1tt[20-strlen(sz)]=0;
    strcat(szP1tt, sz);
    }
  else
    strcpy(szP1tt, szP1);
  if(ulLen > 39)
    {
    sprintf(sz, "..(%luD)..", ulD1);
    strncpy(szP1t2, szP1, 39-strlen(sz));
    szP1t2[39-strlen(sz)]=0;
    strcat(szP1t2, sz);
    }
  else
    strcpy(szP1t2, szP1);
  if(ulLen > 54)
    {
    sprintf(sz, "..(%luD)..", ulD1);
    strncpy(szP1t, szP1, 54-strlen(sz));
    szP1t[54-strlen(sz)]=0;
    strcat(szP1t, sz);
    }
  else
    strcpy(szP1t, szP1);
  iStat=mpz_set_str(mpzP1, szP1, 10);  /* is it a literal? */
  if(iStat)  /* Is it a formula? */
    {
    iStat=iEvalExpr(mpzP1, szP1);
    if(iStat)
      {
      sprintf(sz, "G=%7lu P1=%-20s ERROR: Unable to parse P1.\n",
	ulGap, szP1tt);
      fprintf(stderr, "%s", sz);
      fpOut=fopen(OUTFILE, "at");
      fprintf(fpOut, sz);
      fclose(fpOut);
      continue;
      }
    }
  if(mpz_cmp(mpzP1, mpzTwo) < 0)continue;
  if((ulGap==1) && mpz_cmp(mpzP1, mpzTwo))
    {
    ulErrors++;
    sprintf(sz, "G=%7lu P1=%-20s ERROR: P1 composite (%lu|P1) Gtrue=0\n",
      ulGap, szP1tt, 2UL);
    fprintf(stderr, "%s", sz);
    fpOut=fopen(OUTFILE, "at");
    fprintf(fpOut, sz);
    fclose(fpOut);
    continue;
    }
  if((ulGap==1) && (!mpz_cmp(mpzP1, mpzTwo)))
    {
    if(iScreen)
      {
      dt=lfSeconds2() - lft0 + 0.000500000000001;
      sprintf(sz, "G=%7lu P1=%-39s OK certfd (%.3fs)\n", 1UL, "2", dt);
      __clearline();
      cputs(sz);
      }
    fpOut=fopen(OUTFILE, "at");
    fprintf(fpOut, "G=%7lu P1=%-54s OK certfd\n", 1UL, "2");
    fclose(fpOut);
    ulOK++;
    continue;
    }
  if(ulGap & 1)
    {
    ulErrors++;
    sprintf(sz, "\n ERROR: G odd and G != 1 in the line:\n==>%s", szBuffer);
    fprintf(stderr, "%s", sz);
    fpOut=fopen(OUTFILE, "at");
    fprintf(fpOut, sz);
    fclose(fpOut);
    continue;
    }
  if(mpz_even_p(mpzP1))
    {
    ulErrors++;
    sprintf(sz, "G=%7lu P1=%-20s ERROR: P1 composite (%lu|P1) Gtrue=0\n",
      ulGap, szP1tt, 2UL);
    __clearline();
    cputs(sz);
    fpOut=fopen(OUTFILE, "at");
    fprintf(fpOut, sz);
    fclose(fpOut);
    continue;
    }
  mpz_add_ui(mpzP2, mpzP1, ulGap);
  ulP1Rem=mpz_get_ui(mpzP1);
/*
 * Check the initiating prime P1.
 */
  ulD1a=strlen(mpz_get_str(szScratch, 10, mpzP1));
  if(ulD1a != ulD1)
    {
    szBuffer[iSW-3]=0;
    fprintf(stderr, " ERROR: Conflicting counts of digits in this gap:");
    fprintf(stderr, "\n\n %s...\n\n", szBuffer);
    fprintf(stderr, " ...Stated number=%lu  Actual number=%lu\n",
      ulD1, ulD1a);
    ulErrors++;
    continue;
    }
  if(iNoCheck)  /* Just list the gaps that would have been checked */
    {
    fprintf(fpNoCheck, "%s\n", szBuffer);
    iNCGaps++;
    continue;
    }
  if(iEPO)
    {
    iBackup=0;
    goto CHECK_P1;
    }
  if(iInterior && !iBackup)goto INTERIOR;
  fpBackup=fopen(szBackupFile, "rt");
  if(fpBackup)
    {
    fgets(sz, 128, fpBackup);
    fclose(fpBackup);
    ul=strtoul(sz, &ep, 10);
    if(ul != ulGap)goto CHECK_P1;
    ul=strtoul(ep, &ep2, 10);
    if(ul != ulP1Rem)goto CHECK_P1;
    ulGBack=strtoul(ep2, &ep, 10);
    t1=strtod(ep, &ep2);
    t2=atof(ep2);
    if(t2 < t1)t2=t1;
    lfTstart -= t2;
    lft0 -= t1;
    goto INTERIOR;
    }

CHECK_P1: ;
  if(iScreen)
    {
    dt=lfSeconds2() - lft0 + 0.000500000000001;
    sprintf(sz, "G=%7lu ...Checking P1 (%luD)...", ulGap, ulD1);
    dt=lfSeconds2() - lft0 + 0.000500000000001;
    sprintf(sz3, "%-62s (%.3fs)", sz, dt); sz3[iSW]=0;
    __clearline();
    cputs(sz3);
    }
  if(iInterior)goto INTERIOR;
  if(ulD1 >= MIN_PFGW_DIGITS)vFlush();  /* Safety feature for power outages */
  if(iMR2ThisGap)
    iStat=iMillerRabin(mpzP1, 2);
  else
    iStat=iPrP(mpzP1, ulMRReps, 1000);
  if(iStat==0)
    {
    ulErrors++;
    ulFactor=ulPrmDiv(mpzP1, 1000000UL);
    if(ulFactor > 1)
      sprintf(sz,
	"G=%7lu P1=%-20s ERROR: P1 composite (%lu|P1) Gtrue=0\n",
	ulGap, szP1tt, ulFactor);
    else
      {
      iStat=iMillerRabin(mpzP1, 2);
      if(iStat==0)
	sprintf(sz,
	  "G=%7lu P1=%-20s ERROR: P1 composite (xMR2) Gtrue=0\n",
	  ulGap, szP1tt);
      else  /* MR2 passed but iPrP failed, so BPSW test failed P1 */
	sprintf(sz,
	  "G=%7lu P1=%-20s ERROR: P1 composite (xBPSW) Gtrue=0\n",
	  ulGap, szP1tt);
      }
    __clearline();
    cputs(sz);
    fpOut=fopen(OUTFILE, "at");
    fprintf(fpOut, sz);
    fclose(fpOut);
    continue;
    }
  if(iScreen)
    {
    if(iEPO)
      {
      ulD2=strlen(mpz_get_str(szScratch, 10, mpzP2));
      sprintf(sz, "G=%7lu ...Checking P2 (%luD)...", ulGap, ulD2);
      }
    else
      sprintf(sz, "G=%7lu ...Checking P1 (%luD) + 2...", ulGap, ulD1);
    dt=lfSeconds2() - lft0 + 0.000500000000001;
    sprintf(sz3, "%-62s (%.3fs)", sz, dt); sz3[iSW]=0;
    __clearline();
    cputs(sz3);
    }
  if(iEPO)
    {
    if(iMR2ThisGap)
      iStat=iMillerRabin(mpzP2, 2);
    else
      iStat=iPrP(mpzP2, ulMRReps, 1000);
    if(iStat==0)
      {
      ulErrors++;
      ulFactor=ulPrmDiv(mpzP2, 1000000UL);
      if(ulFactor > 1)
        sprintf(sz,
          "G=%7lu P1=%-20s ERROR: P2 composite (%lu|P2) Gtrue=??\n",
          ulGap, szP1tt, ulFactor);
      else
	{
	iStat=iMillerRabin(mpzP2, 2);
	if(iStat==0)
	  sprintf(sz,
	    "G=%7lu P1=%-20s ERROR: P2 composite (xMR2) Gtrue=??\n",
	    ulGap, szP1tt);
	else  /* MR2 passed but iPrP failed, so BPSW test failed P2 */
	  sprintf(sz,
	    "G=%7lu P1=%-20s ERROR: P2 composite (xBPSW) Gtrue=??\n",
	    ulGap, szP1tt);
	}
      __clearline();
      cputs(sz);
      fpOut=fopen(OUTFILE, "at");
      fprintf(fpOut, sz);
      fclose(fpOut);
      }
    else
      {
      ulOK++;
      if(iScreen)
	{
	dt=lfSeconds2() - lft0 + 0.000500000000001;
	sprintf(sz, "G=%7lu P1=%-39s OK epo", ulGap, szP1t2);
	if(iMR2ThisGap)
	  sprintf(sz2, "MR2");
	else
	  sprintf(sz2, "B*%lu", ulMRReps);
	strcat(sz, sz2);
	sprintf(sz3, "%-62s (%.3fs)\n", sz, dt);
	__clearline();
	cputs(sz3);
	}
      fpOut=fopen(OUTFILE, "at");
      fprintf(fpOut, "G=%7lu P1=%-54s OK epo", ulGap, szP1t);
      if(iMR2ThisGap)
        fprintf(fpOut, "MR2\n");
      else
        fprintf(fpOut, "B*%lu\n", ulMRReps);
      fclose(fpOut);
      }
    continue;
    }
INTERIOR: ;
  /* Now treat full gap analysis (non-EPO) */
  lfLastBackupTime=lfSeconds2();
  if(iPFGW && (ulD1 >= MIN_PFGW_DIGITS))
    ulGap2=mpz_gap_pfgw();
  else
    ulGap2=mpz_gap();
  if(ulGap2 != ulGap)
    {
    ulErrors++;
    if(ulGap2 < ulGap)
      sprintf(sz,
	"G=%7lu P1=%-20s ERROR: Intermediate prime at P1 + %lu\n",
	ulGap, szP1tt, ulGap2);
    else
      {
      ulFactor=ulPrmDiv(mpzP2, 1000000UL);
      if(ulFactor > 1)
	sprintf(sz,
	  "G=%7lu P1=%-20s ERROR: P2 composite (%lu|P2) Gtrue=%lu\n",
	  ulGap, szP1tt, ulFactor, ulGap2);
      else
	{
	iStat=iMillerRabin(mpzP2, 2);
	if(iStat==0)
	  sprintf(sz,
	    "G=%7lu P1=%-20s ERROR: P2 composite (xMR2) Gtrue=%lu\n",
	    ulGap, szP1tt, ulGap2);
	else
	  sprintf(sz,
	    "G=%7lu P1=%-20s ERROR: P2 composite (xBPSW) Gtrue=%lu\n",
	    ulGap, szP1tt, ulGap2);
	}
      }
    __clearline();
    cputs(sz);
    fpOut=fopen(OUTFILE, "at");
    fprintf(fpOut, sz);
    fclose(fpOut);
    }
  else
    {
    ulOK++;
    if(iScreen)
      {
      sprintf(sz, "G=%7lu P1=%-39s OK ", ulGap, szP1t2);
      if(iInterior)strcat(sz, "int");
      if(iMR2ThisGap)
	sprintf(sz2, "MR2");
      else
	sprintf(sz2, "BPSW*%lu", ulMRReps);
      strcat(sz, sz2);
      dt=lfSeconds2() - lft0 + 0.000500000000001;
      lft0=lfSeconds2();
      sprintf(sz3, "%-62s (%.3fs)\n", sz, dt);
      __clearline();
      cputs(sz3);
      }
    fpOut=fopen(OUTFILE, "at");
    fprintf(fpOut, "G=%7lu P1=%-54s OK ", ulGap, szP1t);
    if(iMR2ThisGap)
      fprintf(fpOut, "MR2\n");
    else
      fprintf(fpOut, "BPSW*%lu\n", ulMRReps);
    fclose(fpOut);
    }
  if(ulD1 >= MIN_PFGW_DIGITS)vFlush();  /* Safety feature for power outages */
  }  /* Process the next gap */

__clearline();
fpOut=fopen(OUTFILE, "at");
fprintf(fpOut, "=======================================");
fprintf(fpOut, "======================================\n");
dt=lfSeconds2()-lfTstart + 0.005000000000001;
fprintf(fpOut, " Errors=%lu.  OK=%lu.  T=%.3f seconds.\n", ulErrors,
  ulOK, dt);
fprintf(fpOut, " Input=%s.  CL==>%s<==.\n", argv[1], szCmd);
fprintf(fpOut, "=======================================");
fprintf(fpOut, "======================================\n");
fclose(fpOut);
vFlush();
if(!iNoCheck)
  {
  fprintf(stderr,
    "\n Errors=%lu.  OK=%lu.  T=%.3f seconds.", ulErrors, ulOK, dt);
  fprintf(stderr, "\n Input=%s.  Output=%s.", argv[1], OUTFILE);
  fprintf(stderr, "\n CL==>%s<==.\n", szCmd);
  }
else
  {
  vFlush();
  remove(OUTFILE);
  vFlush();
  if(iNCGaps)
    {
    fprintf(stderr, " ...See %s for the %d selected gaps.", NOCHECKFILE,
      iNCGaps);
    }
  else
    {
    fprintf(stderr, " ...No such gaps were found.");
    fclose(fpNoCheck);
    vFlush();
    remove(NOCHECKFILE);
    vFlush();
    }
  }
return(EXIT_SUCCESS);
}
/**********************************************************************/
static unsigned long mpz_gap(void)
{
/* mpzP1 is presumed the initial prime (previously verified) of a gap,
   and the return value is ulG, the distance to the succeeding prime
   mpzP2. */

int iStat, iCount, iPrintCount, iTest;
unsigned long ulG;
double tnow, Tgap, Ttotal;

if(mpz_cmp_ui(mpzP1, 2)==0)return(1);

if(iScreen)
  {
  sprintf(sz, "G=%7lu ...Checking P1 (%luD) + %lu...sieving...",
    ulGap, ulD1, ulGBack);
  dt=lfSeconds2() - lft0 + 0.000500000000001;
  sprintf(sz2, "%-62s (%.3fs)", sz, dt);
  __clearline();
  cputs(sz2);
  }

vSieve2();

iPrintCount=floor((4200 - ulD1)/100);
ulG=ulGBack;
mpz_add_ui(mpz, mpzP1, ulG);  /* re-start point within gap */
iInsideGap=1;

while(1)
  {
  iTest=(ulG >= 2*ulGap);
  if(!iTest)iTest=uchSieve[(ulG - 2)/2];
  if(iTest)
    {
    iCount++;
    if(iScreen && (iCount >= iPrintCount))
      {
      sprintf(sz, "G=%7lu ...Checking P1 (%luD) + %lu...", ulGap, ulD1, ulG);
      dt=lfSeconds2() - lft0;
      sprintf(sz2, "%-62s (%.3fs)", sz, dt);
      __clearline();
      cputs(sz2);
      iCount=0;
      }
    mpz_powm(mpzRem, mpzTwo, mpz, mpz);
    iStat=mpz_cmp(mpzRem, mpzTwo);
    if(iStat==0)
      {
      if(ulG==ulGap)
	{
	if(iMR2ThisGap)
	  {
	  if(iMillerRabin(mpzP2, 2))break;
	  }
	else
	  {
	  if(iPrP(mpzP2, ulMRReps, 2))break;
	  }
	}
      else
	{
	if(iPrP(mpz, ulMRReps, 2))break;
	}
      }
    }
  mpz_add_ui(mpz, mpz, 2);
  ulG += 2;
  }

if(!iCheckSieve)
  {
  remove(szSieveFile);
  remove(szDivFile);
  }
ulGBack=2;
iInsideGap=0;

return(ulG);
}
/**********************************************************************/
static unsigned long mpz_gap_pfgw(void)
{
/* mpzP1 is presumed the initial prime (previously verified) of a gap,
   and the return value is ulG, the distance to the succeeding prime
   mpzP2. Note that iScreen is not meaningful in conjunction with pfgw. */

char ch, sz[128], szNFile[128], szPFGW1[128];
int iStat, iTest, iPrintCount, iCount, iCount2, iBackupCount;
unsigned long ulG, ulRP1, ulRmpz;
double tnow, Tgap, Ttotal;
FILE *fpLocal;

if(mpz_cmp_ui(mpzP1, 2)==0)return(1);

/* Insert the gap size into the name of the temporary input file
   for pfgw, to prevent parallel processing clashes. */

sprintf(szNFile, "N%lu.dat", ulGap);
sprintf(sz, "%s > nul", szNFile);
strcpy(szPFGW1, szPFGW);
strcat(szPFGW1, sz);

if(ulD1 > 3999)
  iPrintCount=1;
else
  iPrintCount=floor((4200 - ulD1)/100);

if(iPrintCount < 1)iPrintCount=1;
iBackupCount=10*iPrintCount;

ulG=ulGBack;

sprintf(sz, "G=%7lu ...Checking P1 (%luD) + %lu...",
  ulGap, ulD1, ulG);
if(ulG < 3)strcat(sz, "sieving...");
dt=lfSeconds2() - lft0 + 0.000500000000001;
sprintf(sz2, "%-62s (%.3fs)", sz, dt);
__clearline();
cputs(sz2);

vSieve2();

mpz_add_ui(mpz, mpzP1, ulG);  /* re-start point within gap */
iInsideGap=1;
iCount=0;
iCount2=0;
ulRP1=mpz_fdiv_ui(mpzP1, 10000000UL);

while(1)
  {
  iTest=(ulG >= 2*ulGap);
  if(!iTest)iTest=uchSieve[(ulG - 2)/2];
  if(iTest)
    {
    iCount++;
    iCount2++;
    if(iCount >= iPrintCount)
      {
      tnow=lfSeconds2();
      Tgap=tnow - lft0 + 0.000500000000001;
      Ttotal=tnow - lfTstart;
      if(Ttotal < Tgap)Ttotal=Tgap;
      sprintf(sz, "G=%7lu ...Checking P1 (%luD) + %lu...", ulGap, ulD1, ulG);
      sprintf(sz2, "%-62s (%.3fs)\n", sz, Tgap);
      clrscr();
      cputs(sz2);
      iCount=0;
      if((iBackup) && (iCount2 >= iBackupCount))
	{
	fpBackup=fopen(szBackupFile, "wt");
	fprintf(fpBackup, "%lu  %lu  %lu  %.3f  %.3f\n",
	  ulGap, ulP1Rem, ulG, Tgap, Ttotal);
	fclose(fpBackup);
	vFlush();
	iCount2=0;
	}
      }
    fpLocal=fopen(szNFile, "wt");
    gmp_fprintf(fpLocal, "%Zd\n", mpz);
    fclose(fpLocal);
#ifdef DEBUG
    ulRmpz=mpz_fdiv_ui(mpz, 10000000UL);
    printf("\n P1ð%lu    ulG=%lu    mpzð%lu", ulRP1, ulG, ulRmpz);
#endif
    iStat=system(szPFGW1);
    if(iStat != 1)
      {
      if(ulG==ulGap)
	{
	if(iMR2ThisGap)
	  {
	  if(iMillerRabin(mpzP2, 2))break;
	  }
	else
	  {
	  if(iPrP(mpzP2, ulMRReps, 2))break;
	  }
	}
      else
	{
	if(iPrP(mpz, ulMRReps, 2))break;
	}
      }
    }
  mpz_add_ui(mpz, mpz, 2);
  ulG += 2;
  }

remove(szBackupFile);
if(!iCheckSieve)
  {
  remove(szSieveFile);
  remove(szDivFile);
  }
remove("pfgw.ini");
remove("pfgw.log");
remove(szNFile);
clrscr();
fpLocal=fopen(OUTFILE, "rt");
if(fpLocal)
  {
  while(1)
    {
    if(!fgets(sz, 80, fpLocal))break;
    fprintf(stderr, "%s", sz);
    }
  fclose(fpLocal);
  }

ulGBack=2;
iInsideGap=0;

return(ulG);
}
/**********************************************************************/
static void vSieve2(void)
{
/* Sieves the interior of the gap by finding which members are multiples
   of small primes (up to ulMaxDiv). The interval is represented
   by the byte array uchSieve, with uchSieve[n] := P1 + 2n + 2,
   uchSieve[0] := P1 + 2, uchSieve[(ulGap - 2)/2] := P2, and
   uchSieve[ulGap-1] := P1 + 2*ulGap. The sieved interval is
   double the length of the gap to allow for the possibility that the
   true gap is up to double the conjectured length. uchSieve[n]=0 indicates
   a definite composite, uchSieve[n]=1 indicates a possible prime (no small
   prime divisors found). */

char *pch, *pch2;
static int d[8]={1,7,11,13,17,19,23,29};
unsigned long ulMaxDiv, ulRem, ulDiv, ulOffset, ul, ul2, ul3,
  ulUnfactored, ulBase;
double lfCheckSum, lfCheckSum2, lft0Sieve, lfSqrt;

fpSieve=fopen(szSieveFile, "rt");
if(fpSieve)
  {
  fgets(sz, 128, fpSieve);
  ul=strtoul(sz, &pch, 0);
  ul2=strtoul(pch, &pch2, 0);
  lfCheckSum=strtod(pch2, NULL);
  if((ul==ulGap) && (ul2==ulP1Rem))  /* Sieve backup for this gap */
    {
    for(ul=0; ul < ulGap; ul++)uchSieve[ul]=0;  /* File has "on" values */
    lfCheckSum2=0;
    while(1)
      {
      fgets(sz, 128, fpSieve);
      if(feof(fpSieve))
        {
        fclose(fpSieve);
        if(lfCheckSum2==lfCheckSum)
          return;
        else
          break;  /* bad checksum */
        }
      ul2=strtod(sz, NULL);
      lfCheckSum2 += ul2;
      /* convert offset from P1 to array index */
      if(ul2 <= 2*ulGap)uchSieve[(ul2 - 2)/2]=1;
      }
    }
  fclose(fpSieve);
  }

for(ul=0; ul < ulGap; ul++)uchSieve[ul]=1;  /* No small prime divisors */

if(ulD1 < 3)return;  /* Don't sieve very small gaps or intervals */

lft0Sieve=lfSeconds2();

/* Maximum sieve divisor is chosen based on the number of decimal digits
   in P1, and reflects empirical data. */

ulMaxDiv=65519UL;  /* default value */
if(ulD1 >=   500)ulMaxDiv=1e6;
if(ulD1 >=  1500)ulMaxDiv=5e6;
if(ulD1 >=  5000)ulMaxDiv=20e6;
if(ulD1 >= 10000)ulMaxDiv=100e6;
if(ulD1 >= 18000)ulMaxDiv=4294967291UL;

if(ulD1 < 21)
  {
  lfSqrt=sqrt(mpz_get_d(mpzP1) + 2.0*ulGap);
  if(ulMaxDiv > lfSqrt)ulMaxDiv=ceil(lfSqrt);
  }

/* Now sieve the interval from P1+2 to P1+2*ulGap=P2+ulGap. A ulGap
   length overflow buffer is created, in case P2 and all integers in
   P1 < n < P2 are composite and the gap is larger than advertised.
   The sieve will use prime divisors to ulMaxDiv. This will leave a
   few composite elements marked as prime (perhaps hundreds for very
   large P1's), to be checked later by the Fermat test with base 2. */

for(ul=0; ul <= ulGap; ul += 2)ulLPD[ul]=0;
for(ul=1; ul <  ulGap; ul += 2)ulLPD[ul]=2;

/* First sieve with prime divisors 3 to 65519. */

mpz_add_ui(mpz, mpzP1, 2);  /* starting point for sieve */
for(ul=2; ul < 6542 ; ul++)
  {
  ulDiv=ulPrime16[ul];
  if(ulDiv > ulMaxDiv)break;
  ulRem=mpz_fdiv_ui(mpz, ulDiv);
  /* Calculate the offset of the first multiple of D in uchSieve. */
  if(ulRem&1)ulOffset=(ulDiv-ulRem)/2;  /* if ulRem is odd */
  else if(!ulRem)ulOffset=0;            /* if ulRem=0 */
  else ulOffset=ulDiv-(ulRem/2);        /* if ulRem is even and not zero */
  for(ul2=ulOffset; ul2 < ulGap; ul2 += ulDiv)
    {
    if(uchSieve[ul2]==1)
      {
      uchSieve[ul2]=0;
      ul3=2*ul2+2;
      if(ul3 <= ulGap)ulLPD[ul3]=ulDiv;
      }
    }
  }

if(iSpecial)
  {
  ix=wherex(); iy=wherey();
  gotoxy(1, 13);
  ulUnfactored=0;
  for(ul2=0; ul2 < ulGap/2 - 1; ul2++)ulUnfactored += uchSieve[ul2];
  if(iScreen)
    {
    dt=lfSeconds2() - lft0Sieve + 0.000500000000001;
    sprintf(sz, "  D=%lu/%lu  U=%lu  dT=%.3f  ", ulDiv, ulMaxDiv,
      ulUnfactored, dt);
    cputs(sz);
    gotoxy(ix, iy);
    }
  }

if(ulDiv > ulMaxDiv)goto CHECK_SIEVE;

/* Once the 16-bit divisors have been exhausted, sieve with
   divisors of the form 30n + d, where d=1,7,11,13,17,19,23,29. */

ulBase=65520UL;
while(1)
  {
  for(ul=0; ul < 8; ul++)
    {
    ulDiv=ulBase + d[ul];
    if(ulDiv > ulMaxDiv)break;
    if(!iIsPrime32(ulDiv))continue;  /* skip composite divisors */
    ulRem=mpz_fdiv_ui(mpz, ulDiv);
    /* Calculate the offset of the first multiple of D in uchSieve. */
    if(ulRem&1)ulOffset=(ulDiv-ulRem)/2;  /* if ulRem is odd */
    else if(!ulRem)ulOffset=0;            /* if ulRem=0 */
    else ulOffset=ulDiv-(ulRem/2);        /* if ulRem is even and not zero */
    for(ul2=ulOffset; ul2 < ulGap; ul2 += ulDiv)
      {
      if(uchSieve[ul2]==1)
	{
	uchSieve[ul2]=0;
	ul3=2*ul2+2;
	if(ul3 <= ulGap)ulLPD[ul3]=ulDiv;
	}
      }
    }
  if((iSpecial) && (ulBase % 1000000UL < 30))
    {
    if(iScreen)
      {
      ix=wherex(); iy=wherey();
      gotoxy(1, 13);
      ulUnfactored=0;
      for(ul2=0; ul2 < ulGap/2 - 1; ul2++)ulUnfactored += uchSieve[ul2];
      dt=lfSeconds2() - lft0Sieve + 0.000500000000001;
      sprintf(sz, "  D=%lu/%lu  U=%lu  dT=%.3f  ", ulDiv, ulMaxDiv,
	ulUnfactored, dt);
      cputs(sz);
      gotoxy(ix, iy);
      }
    }
  if(ulDiv > ulMaxDiv)break;
  ulBase += 30;
  }

CHECK_SIEVE: ;

/* If the environmental variable CHECK_SIEVE is set, check the zero
   elements of the sieve array directly for divisibility by the detected
   least prime divisors. Skip the even integers (odd offsets), which
   are automatically divisible by 2, and the buffer elements beyond P2,
   which are retained in the unlikely event of a composite P2. */

if(iCheckSieve)
  {
  mpz_set(mpz, mpzP1);
  for(ul=2; ul < ulGap; ul += 2)
    {
    mpz_add_ui(mpz, mpz, 2);
    ul3=ulLPD[ul];
    if(ul3 < 3)continue;
    mpz_set_ui(mpzD, ul3);
    if(!mpz_divisible_p(mpz, mpzD))
      {
      fprintf(stderr,
	"\n SIEVING ERROR: P1 + %lu not divisible by %lu.\n",
	ul, ul3);
      }
    }
  if(iSpecial)
    {
    fpDiv=fopen(szDivFile, "wt");
    for(ul=2; ul < ulGap; ul += 2)
      fprintf(fpDiv, "%10lu %10lu\n", ul, ulLPD[ul]);
    fclose(fpDiv);
    }
  }

/* If backup is on, write the sieve array to disk. Only the indices
   (actually the offsets from P1, offset=2*index + 2, index=(offset-2)/2)
   for which uchSieve[ul]=1 are written, following a header which
   identifies G and P1. A checksum is calculated for the array. */

lfCheckSum=0;
for(ul=0; ul < ulGap; ul++)
  if(uchSieve[ul])lfCheckSum += 2.0*ul + 2;

if(iBackup)
  {
  fpSieve=fopen(szSieveFile, "wt");
  fprintf(fpSieve, "%lu  %lu  %.0f\n", ulGap, ulP1Rem, lfCheckSum);
  for(ul=0; ul < ulGap; ul++)if(uchSieve[ul])fprintf(fpSieve, "%lu\n",
    2*ul + 2);
  fclose(fpSieve); vFlush();
  }

return;
}
/**********************************************************************/
static void vSyntax(void)
{
printf("\n cglp4.c              Thomas R. Nicely           2017.09.01.2230");
printf("\n");
printf("\n Freeware copyright (C) 2017 Thomas R. Nicely "
       "<http://www.trnicely.net>.");
printf("\n");
printf("\n Released into the public domain by the author, who disclaims");
printf("\n any legal liability arising from its use.");
printf("\n");
printf("\n Checks prime gap listings (in standard format) for validity, ");
printf("\n using the strong Baillie-PSW test; for details, see");
printf("\n <http://www.trnicely.net/misc/bpsw.html>. GMP required.");
printf("\n");
printf("\n The input file should contain one gap per line. The format");
printf("\n should be similar to one of the following: (...press ENTER...)");
printf("\n");
getch();
printf("\r   618  CFC RP.Brent 1980  21.27    13  4165633395149");
printf("\n");
printf("\n or");
printf("\n");
printf("\n   618  4165633395149");
printf("\n");
printf("\n The initiating prime may also be specified using certain BASIC");
printf("\n and FORTRAN style formulas; the first format also has a line");
printf("\n continuation option. Results are recorded in the file cglp4.out.");
printf("\n See <http://www.trnicely.net/gaps/gaplist.html> for additional");
printf("\n details and examples.");
printf("\n");
printf("\n The (exported) environmental (shell) variables MRREPS,");
printf("\n CGLP4_BACKUP, CGLP4BUI, and INTERIOR may also be");
printf("\n used (through SET, export, declare -x, setenv, unset, etc.) to");
printf("\n modify execution; see the source code cglp4.c for details.");
printf("\n");
printf(
  "\n SYNTAX: cglp4 infile [x|m|M|z|b] [mingap maxgap] [mindig maxdig] [n]");
exit(EXIT_FAILURE);
}
/**********************************************************************/
/**********************************************************************/
