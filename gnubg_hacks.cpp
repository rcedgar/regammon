#include "myutils.h"
#include "bgboard.h"

/***
Code copied and modified from gnubg source code under GPL license.
For original source and license information, go to:
http://files.gnubg.org/media/
***/

/***
1.6 A technical description of the Position ID

This section describes a method for compactly recording a backgammon position.
It demonstrates how to encode a position into 10 binary bytes, which is useful
for minimizing the space used when recording large numbers of positions in memory
or on disk. There is also an ASCII representation in 14 characters, which is
convenient for output to the screen, for copying and pasting to transfer positions
between programs which support the format, and for communicating positions via
Usenet news or e-mail. The 10 byte binary format is called the key, and the 14
character ASCII format is the ID.

The key is essentially a bit string (imagine you start with an empty sequence of bits,
and continue adding either 0 or 1 to the end). The way to build up a sequence that
corresponds to a given position is:

    For every point around the board (starting at the ace point of the player on roll,
	  continuing around to the 24 point and ending at the bar):

    append as many 1s as the player on roll has on that point (if any).
    append a 0.
    For every point around the board (starting at the ace point of the opponent, continuing around
	  to the opponent's 24 point and ending at the bar):
    append as many 1s as the opponent has on that point (if any).
    append a 0.
    Pad out the string to 80 bits with 0s. 

The worst-case representation will require 80 bits: you can see that there are
always 50 0 bits even if there are no checkers at all. Each player has a maximum
of 15 checkers in play (not yet borne off) which require a 1 bit wherever they are
positioned. That's 30 bits to take of all checkers, plus the 50 bits of overhead
for a total of 80 bits (the last bit is always 0 and isn't strictly necessary, but
it makes the code slightly easier). This bit string should be stored in little-endian
order when packed into bytes (i.e. the first bits in the string are stored in the
least significant bits of the first byte).

As an example, here's what the starting position looks like in the key format:

0 0 0 0 0	player on roll has no checkers on ace to 5 points
11111 0		5 checkers on the 6 point
0			empty bar
111 0		3 on the 8
0 0 0 0		no others in our outfield
11111 0		5 on the midpoint
0 0 0 0 0	none in the opponent's outfield
0 0 0 0 0	or in opponent's board, until...
11 0		two on the 24 point
0			none on the bar
0 0 0 0 0	opponent has no checkers on ace to 5 points
11111 0		5	checkers on the 6 point
0			empty bar
111 0		3 on the 8
0 0 0 0		no others in opponent's outfield
11111 0		5 on the midpoint
0 0 0 0 0	none in our outfield
0 0 0 0 0	or in our board, until...
11 0		two on the 24 point
0			none on the bar

so altogether it's:

00000111110011100000111110000000000011000000011111001110000011111000000000001100

00000111 11001110 00001111 10000000 00001100 00000111 11001110 00001111 10000000 00001100 

In little endian bytes it looks like:

11100000	01110011	11110000	00000001	00110000	= 1110000001110011111100000000000100110000
	0xE0		0x73		0xF0		0x01		0x30	= 0xE00x730xF00x010x30

so the 10 byte key (in hex) is E0 73 F0 01 30 E0 73 F0 01 30.

The ID format is simply the Base64 encoding of the key. (Technically, a Base64 encoding
of 80 binary bits should consist of 14 characters followed by two = padding characters,
but this padding is omitted in the ID format.)

To continue the above example, splitting the 10 8-bit bytes into 14 6-bit groups gives:

111000 000111 001111 110000 000000 010011 000011 100000 011100 111111 000000 000001 001100 000000

In Base64 encoding, these groups are respectively represented as:

4 H P w A T D g c / A B M A

So, the position ID of the checkers at the start of the game is simply:

4HPwATDgc/ABMA 
***/

#define TRUE	1
#define FALSE	0
#define g_assert	assert
#define L_POSITIONID 14
#define L_MATCHID 12
#define	_(s)	s
#define	outputf printf
#define START_POS_ID	"4HPwATDgc/ABMA"

typedef const unsigned int (*ConstTanBoard)[25];
typedef union _positionkey {
    unsigned int data[7];
} positionkey;

typedef union _oldpositionkey {
    unsigned char auch[10];
} oldpositionkey;

static char *
NextToken(char **ppch);

/* get the next token from the input and convert as an
 * integer. Returns INT_MIN on empty input or non-numerics found. Does
 * handle negative integers. On failure, one token (if any were available
 * will have been consumed, it is not pushed back into the input.
 */
static int
ParseNumber(char **ppch)
{

    char *pch, *pchOrig;

    if (!ppch || !(pchOrig = NextToken(ppch)))
        return INT_MIN;

    for (pch = pchOrig; *pch; pch++)
        if (!isdigit(*pch) && *pch != '-')
            return INT_MIN;

    return atoi(pchOrig);
}

static int
CheckPosition(const TanBoard anBoard)
{
    unsigned int ac[2], i;

    /* Check for a player with over 15 chequers */
    for (i = ac[0] = ac[1] = 0; i < 25; i++)
        if ((ac[0] += anBoard[0][i]) > 15 || (ac[1] += anBoard[1][i]) > 15) {
			Die("> 15 checkers");
            errno = EINVAL;
            return 0;
        }

    /* Check for both players having chequers on the same point */
    for (i = 0; i < 24; i++)
        if (anBoard[0][i] && anBoard[1][23 - i]) {
			Die("b&w checkers on same point");
            errno = EINVAL;
            return 0;
        }

    /* Check for both players on the bar against closed boards */
    for (i = 0; i < 6; i++)
        if (anBoard[0][i] < 2 || anBoard[1][i] < 2)
            return 1;

    if (!anBoard[0][24] || !anBoard[1][24])
        return 1;

	Die("both on bar against closed boards");
    errno = EINVAL;
    return 0;
}

static void
oldPositionFromKey(TanBoard anBoard, const oldpositionkey * pkey)
{
    int i = 0, j = 0, k;
    const unsigned char *a;

    memset(anBoard[0], 0, sizeof(anBoard[0]));
    memset(anBoard[1], 0, sizeof(anBoard[1]));

    for (a = pkey->auch; a < pkey->auch + 10; ++a) {
        unsigned char cur = *a;

        for (k = 0; k < 8; ++k) {
            if ((cur & 0x1)) {
                if (i >= 2 || j >= 25) {        /* Error, so return - will probably show error message */
                    return;
                }
                ++anBoard[i][j];
            } else {
                if (++j == 25) {
                    ++i;
                    j = 0;
                }
            }
            cur >>= 1;
        }
    }
}

static unsigned char
Base64(const unsigned char ch)
{
    if (ch >= 'A' && ch <= 'Z')
        return ch - 'A';

    if (ch >= 'a' && ch <= 'z')
        return (ch - 'a') + 26;

    if (ch >= '0' && ch <= '9')
        return (ch - '0') + 52;

    if (ch == '+')
        return 62;

    if (ch == '/')
        return 63;

    return 255;
}

static int
PositionFromID(TanBoard anBoard, const char *pchEnc)
{
    oldpositionkey key;
	size_t bytes = sizeof(key.auch);
	memset(key.auch, 0, bytes);
    unsigned char ach[L_POSITIONID + 1], *pch = ach, *puch = key.auch;
    int i;

    memset(ach, 0, L_POSITIONID + 1);

    for (i = 0; i < L_POSITIONID && pchEnc[i]; i++)
        pch[i] = Base64((unsigned char) pchEnc[i]);

    for (i = 0; i < 3; i++)
		{
        *puch++ = (unsigned char) (pch[0] << 2) | (pch[1] >> 4);
        *puch++ = (unsigned char) (pch[1] << 4) | (pch[2] >> 2);
        *puch++ = (unsigned char) (pch[2] << 6) | pch[3];
        pch += 4;
		}

    *puch = (unsigned char) (pch[0] << 2) | (pch[1] >> 4);
    oldPositionFromKey(anBoard, &key);
    return CheckPosition((ConstTanBoard) anBoard);
}

/*
 * general token extraction
 input: ppch pointer to pointer to command
 szToekns - string of token separators
 output: NULL if no token found
 ptr to extracted token if found. Token is in original location
 in input string, but null terminated if not quoted, token
 will have been moved forward over quote character when quoted
 ie: 
 input:  '  abcd efgh'
 output  '  abcd\0efgh'
 return value points to abcd, ppch points to efgh
 input   '  "jklm" nopq'
 output  ;  jklm\0 nopq'
 return value points to jklm, ppch points to space before 
 the 'n'
 ppch points past null terminator
 
 ignores leading whitespace, advances ppch over token and trailing
 whitespace
 
 matching single or double quotes are allowed, any character outside
 of quotes or in doubly quoted strings can be escaped with a
 backslash and will be taken as literal.  Backslashes within single
 quoted strings are taken literally. Multiple quoted strings can be
 concatenated.  
 
 For example: input ' abc\"d"e f\"g h i"jk'l m n \" o p q'rst uvwzyz'
 with the terminator list ' \t\r\n\v\f'
 The returned token will be the string
 <abc"de f"g h j ijkl m n \" o p qrst>
 ppch will point to the 'u'
 The \" between c and d is not in a single quoted string, so is reduced to 
 a double quote and is *not* the start of a quoted string.
 The " before the 'd' begins a double quoted string, so spaces and tabs are
 not terminators. The \" between f and g is reduced to a double quote and 
 does not terminate the quoted string. which ends with the double quote 
 between i and j. The \" between n and o is taken as a pair of literal
 characters because they are within the single quoted string beginning
 before l and ending after q.
 It is not possible to put a single quote within a single quoted string. 
 You can have single quotes unescaped within double quoted strings and
 double quotes unescaped within single quoted strings.
 */
static char *
NextTokenGeneral(char **ppch, const char *szTokens)
{


    char *pch, *pchSave, chQuote = 0;
    int fEnd = FALSE;
#if !defined(G_DISABLE_ASSERT)
    char *pchEnd;
#endif

    if (!*ppch)
        return NULL;

#if !defined(G_DISABLE_ASSERT)
    pchEnd = strchr(*ppch, 0);
#endif

    /* skip leading whitespace */
    while (isspace(**ppch))
        (*ppch)++;

    if (!*(pch = pchSave = *ppch))
        /* nothing left */
        return NULL;

    while (!fEnd) {

        if (**ppch && strchr(szTokens, **ppch)) {
            /* this character ends token */
            if (!chQuote) {
                fEnd = TRUE;
                (*ppch)++;      /* step over token */
            } else
                *pchSave++ = **ppch;
        } else {
            switch (**ppch) {
            case '"':
                /* quote mark */
                if (!chQuote)
                    /* start quoting */
                    chQuote = **ppch;
                else if (chQuote == **ppch)
                    /* end quoting */
                    chQuote = 0;
                else
                    /* literal */
                    *pchSave++ = **ppch;
                break;

#ifdef NO_BACKSLASH_ESCAPES
            case '%':
#else
            case '\\':
#endif
                /* backslash */
                if (chQuote == '\'')
                    /* literal */
                    *pchSave++ = **ppch;
                else {
                    (*ppch)++;

                    if (**ppch)
                        /* next character is literal */
                        *pchSave++ = **ppch;
                    else {
                        /* end of string -- the backlash doesn't quote anything */
#ifdef NO_BACKSLASH_ESCAPES
                        *pchSave++ = '%';
#else
                        *pchSave++ = '\\';
#endif
                        fEnd = TRUE;
                    }
                }
                break;

            case 0:
                /* end of string -- always ends token */
                fEnd = TRUE;
                break;

            default:
                *pchSave++ = **ppch;
            }

        }

        if (!fEnd)
            (*ppch)++;
    }
    while (isspace(**ppch))
        (*ppch)++;

    *pchSave = 0;

#if !defined(G_DISABLE_ASSERT)
    g_assert(pchSave <= pchEnd);
    g_assert(*ppch <= pchEnd);
    g_assert(pch <= pchEnd);
#endif

    return pch;

}


/* extrace a token from a string. Tokens are terminated by tab, newline, 
 * carriage return, vertical tab or form feed.
 * Input:
 * 
 * ppch = pointer to pointer to input string. This will be updated
 * to point past any token found. If the string is exhausetd, 
 * the pointer at ppch will point to the terminating NULL, so it is
 * safe to call this function repeatedly after failure
 * 
 * Output:
 * null terminated token if found or NULL if no tokens present.
 */
static char *
NextToken(char **ppch)
{
    return NextTokenGeneral(ppch, " \t\n\r\v\f");
}

/* Convert a string to a board array.  Currently allows the string to
 * be a position ID, a gnubg-nn position string, "=n" notation,
 * or empty (in which case the current board is used).
 * 
 * The input string should be specified in *ppch; this string must be
 * modifiable, and the pointer will be updated to point to the token
 * following a board specification if possible (see NextToken()).  The
 * board will be returned in an, and if pchDesc is non-NULL, then
 * descriptive text (the position ID, formatted move, or "Current
 * position", depending on the input) will be stored there.
 * 
 * Returns -1 on failure, 0 on success, or 1 on success if the position
 * specified has the opponent on roll (e.g. because it used "=n" notation). */
static int
ParsePosition(TanBoard &an, const char *s)
{
	// NNNGABIDAAFNABAAAAAA
	// 12345678901234567890
    if (strlen(s) == 20) {    /* gnubg-nn position string */
        static oldpositionkey key;

        for (int i = 0; i < 10; ++i) {
            if (s[2 * i + 0] >= 'A' && s[2 * i + 0] <= 'P' && s[2 * i + 1] >= 'A' && s[2 * i + 1] <= 'P')
                key.auch[i] = (unsigned char) (((s[2 * i + 0] - 'A') << 4) + (s[2 * i + 1] - 'A'));
            else {
                Die("Illegal position.");
                return -1;
            }
        }

        oldPositionFromKey(an, &key);

        return 0;
    }

    if (!PositionFromID(an, s)) {
        Die("Illegal position.");
        return -1;
    }

    return 0;
}

static inline void
addBits(unsigned char auchKey[10], unsigned int bitPos, unsigned int nBits)
{
    unsigned int k = bitPos / 8;
    unsigned int r = (bitPos & 0x7);
    unsigned int b = (((unsigned int) 0x1 << nBits) - 1) << r;

    auchKey[k] |= (unsigned char) b;

    if (k < 8) {
        auchKey[k + 1] |= (unsigned char) (b >> 8);
        auchKey[k + 2] |= (unsigned char) (b >> 16);
    } else if (k == 8) {
        auchKey[k + 1] |= (unsigned char) (b >> 8);
    }
}

static void
oldPositionKey(const TanBoard anBoard, oldpositionkey * pkey)
{
    unsigned int i, iBit = 0;
    const unsigned int *j;

    memset(pkey, 0, sizeof(oldpositionkey));

    for (i = 0; i < 2; ++i) {
        const unsigned int *const b = anBoard[i];
        for (j = b; j < b + 25; ++j) {
            const unsigned int nc = *j;

            if (nc) {
                addBits(pkey->auch, iBit, nc);
                iBit += nc + 1;
            } else {
                ++iBit;
            }
        }
    }
}

// szID must be at least 15 bytes
static char *
oldPositionIDFromKey(const oldpositionkey * pkey, char *szID)
{
    unsigned char const *puch = pkey->auch;
//    static char szID[L_POSITIONID + 1];
    char *pch = szID;
    static char aszBase64[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i;

    for (i = 0; i < 3; i++) {
        *pch++ = aszBase64[puch[0] >> 2];
        *pch++ = aszBase64[((puch[0] & 0x03) << 4) | (puch[1] >> 4)];
        *pch++ = aszBase64[((puch[1] & 0x0F) << 2) | (puch[2] >> 6)];
        *pch++ = aszBase64[puch[2] & 0x3F];

        puch += 3;
    }

    *pch++ = aszBase64[*puch >> 2];
    *pch++ = aszBase64[(*puch & 0x03) << 4];

    *pch = 0;

    return szID;
}

// 0=over, 1=contact, 2=race, 3=crashed
static int
ClassifyPosition(const TanBoard &anBoard)
{
    int nOppBack = -1, nBack = -1;

    for (nOppBack = 24; nOppBack >= 0; --nOppBack) {
        if (anBoard[0][nOppBack]) {
            break;
        }
    }

    for (nBack = 24; nBack >= 0; --nBack) {
        if (anBoard[1][nBack]) {
            break;
        }
    }

    if (nBack < 0 || nOppBack < 0)
        return 0;

        /* normal backgammon */

        if (nBack + nOppBack > 22) {

            /* contact position */

            unsigned int const N = 6;
            unsigned int i;
            unsigned int side;

            for (side = 0; side < 2; ++side) {
                unsigned int tot = 0;

                const unsigned int *board = anBoard[side];

                for (i = 0; i < 25; ++i) {
                    tot += board[i];
                }

                if (tot <= N) {
                    return 3;
                } else {
                    if (board[0] > 1) {
                        if (tot <= (N + board[0])) {
                            return 3;
                        } else {
                            if (board[1] > 1 && (1 + tot - (board[0] + board[1])) <= N) {
                                return 3;
                            }
                        }
                    } else {
                        if (tot <= (N + (board[1] - 1))) {
                            return 3;
                        }
                    }
                }
            }

            return 1;
        } else {

            return 2;

        }

	asserta(false);
    return -999;                   /* for fussy compilers */
}

void BgBoard::GetTanBoard(TanBoard &TB) const
	{
    memset(TB[0], 0, sizeof(TB[0]));
    memset(TB[1], 0, sizeof(TB[1]));
	for (unsigned i = 0; i < 24; ++i)
		{
		TB[1][i] = m_w[24-i];
		TB[0][i] = m_b[24-i];
		}
	TB[1][24] = m_w[BAR];
	TB[0][24] = m_b[BAR];
	}

void BgBoard::GetTanBoard_Flipped(TanBoard &TB) const
	{
    memset(TB[0], 0, sizeof(TB[0]));
    memset(TB[1], 0, sizeof(TB[1]));
	for (unsigned i = 0; i < 24; ++i)
		{
		TB[1][i] = m_b[24-i];
		TB[0][i] = m_w[24-i];
		}
	TB[1][24] = m_b[BAR];
	TB[0][24] = m_w[BAR];
	}

void BgBoard::FromStr(const string &PosId)
	{
	Clear();

	TanBoard an;
	ParsePosition(an, PosId.c_str());

// Bar is 24, men off not encoded, calculate as off=(total-15)
	int SumW = 0;
	int SumB = 0;
	for (unsigned i = 0; i < 24; ++i)
		{
		int w = an[1][i];
		int b = an[0][i];
		SumW += w;
		SumB += b;

		m_w[24-i] = (byte) w;
		m_b[24-i] = (byte) b;
		}

	int w = an[1][24];
	int b = an[0][24];
	SumW += w;
	SumB += b;

	m_w[0] = w;
	m_b[0] = b;

	asserta(SumW <= 15);
	asserta(SumB <= 15);

	m_w[25] = 15 - SumW;
	m_b[25] = 15 - SumB;

	AssertValid();
	}

const char *BgBoard::ToStr(string &s) const
	{
	TanBoard TB;
	GetTanBoard(TB);
	oldpositionkey key;
	oldPositionKey(TB, &key);
	char szID[16];
	oldPositionIDFromKey(&key, szID);
	s = string(szID);
	return s.c_str();
	}

// 0=over, 1=contact, 2=race, 3=crashed
int BgBoard::GetGnuClass() const
	{
	TanBoard an;
	GetTanBoard(an);
	int i = ClassifyPosition(an);
	return i;
	}
