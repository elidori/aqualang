#include "Pch.h"
#include "Corpora.h"
#include "ApplicationDataPath.h"
#include "StringConvert.h"
#include "Log.h"

#define MAXFILENAME 2048
#define MAXLINELEN  4096
#define SMALLESTVALUE 1e-30			// equivalent to zero value
#define MINHITVALUE 1E-15			// minimal value to be considered as HIT

#define USED_CODEPAGE 38598 // Hebrew (ISO-Logical)

Corpora::Corpora(LPCTSTR Language, HKL hKL)
	: m_Language(Language),
	m_hKL(hKL),
	m_CodePage(USED_CODEPAGE)
{
	if (m_Language == _T("HE")) 
	{
		nofadlchars = 2;
		addlchars[0]='\'' ; addlchars[1] = '\"' ;addlchars[2] = 0;
		firstchar = (unsigned char)'à' ; lastchar = (unsigned char)'ú' ;
	}
	if (m_Language == _T("EN"))
	{
		nofadlchars = 1;
		firstchar='a'; lastchar='z';
		addlchars[0] = '\''; 
	}
	nofchars = (lastchar - firstchar) + 1  + nofadlchars;
	memset( (void *) cube, 0, sizeof(float) * MAXCHARS * MAXCHARS * MAXCHARS); // make sure cube starts with 0
	memset( (void *) singletons, 0, sizeof(float) * MAXCHARS);
	memset( (void *) doubles, 0 , sizeof(float) * MAXCHARS * MAXCHARS);
	noftrigrams = 0;
	nofwords=0;
}

bool Corpora::Open()
{
	ApplicationDataPath Path(NULL);
	_tstring FileName;
	Path.ComposeRelativePath(_T("Corpora"), FileName);
	FileName += m_Language;
	FileName += _T(".txt");

	FILE *fp = NULL;
	_tfopen_s(&fp, FileName.c_str(), _T("r"));
	if (fp == NULL) 
	{
		Log(_T("Corpora::Open() - failed loading file %s\n"), FileName.c_str());
		return false;
	}

	buildCube(fp);
	fclose(fp);
	return true;
}

Corpora *Corpora::Create(LPCTSTR Language, HKL hKL)
{
	Corpora *pNew = new Corpora(Language, hKL);
	if(pNew == NULL)
		return NULL;
	if(!pNew->Open())
	{
		delete pNew;
		return NULL;
	}
	return pNew;
}

Corpora::~Corpora()
{
}

void Corpora::save()
{
	int x, y, z;
	int upperbound;
	unsigned char c, cc, ccc;
	float val;

	upperbound = nofchars;

	for (x=0; x< upperbound; x++) {
		c = reverseIndex(x);
		val = singletons[x] / enoft;
		if (val != 0.0) 
			printf (" %c    ,     %e \n", c, val);
	}
	for (x=0; x < upperbound; x++)
		for (y=0; y < upperbound; y++) {
			c = reverseIndex(x);
			cc = reverseIndex(y);
			val = doubles[x][y] / enoft;
			if (val != 0.0)
				printf("%c%c    ,    %e \n", c, cc, val);
		}
	for (x=0; x<upperbound; x++)
		for (y=0; y<upperbound; y++)
			for (z=0; z<upperbound; z++) {
				c = reverseIndex(x);
				cc = reverseIndex(y);
				ccc = reverseIndex(z);
				val = cube[x][y][z] / enoft;
				if (val != 0.0)
					printf("%c%c%c    ,    %e   \n", c, cc, ccc, val);
			}
}

float Corpora::probOfStr(const char *str)
{
	int state,  i;
	const char *trigS, *trigE;
	state = 0;
	trigS = trigE = str;
	float rv = 1.0;
	float pr, arv = 0;
	int hitCount = 0;
	bool loop = true;

	for(i=0; loop && i<MAXLINELEN; i++) {
		switch (state) {
		case 0:
			if ( isWordTerminator(str[i])) { 
				trigS = str+i;
				trigE = trigS;
				if (str[i]==0 || str[i]==10) loop = false;
				else                         continue; 
			}
			else {
				trigS=str+i;
				trigE=str+i+1;
				state = 1;	 }
			break;
		case 1:
		case 2:
			if ( isWordTerminator(str[i])) {
				pr = trigProb(trigS, trigE);
				if (pr > MINHITVALUE) hitCount++ ;
				else                  hitCount--;
				rv *= pr;
				arv +=rv;
				trigS = str + i;
				trigE = trigS;
				state = 0;	rv = 1.0;	 }
			else {
				state ++;
				trigE++; 		 }
			 
			break;
		case 3:
			if (isWordTerminator(str[i])) {
				pr = trigProb(trigS, trigE);
				if (pr > MINHITVALUE) hitCount++ ;
				else                  hitCount--;
				rv *= pr;
				arv += rv;
				trigS = str + i;
				trigE = trigS;
				state = 0; rv = 1.0;
			}
			else { // word continues more than 3 chars
				pr =trigProb(trigS, trigE);
				if (pr > MINHITVALUE) hitCount++ ;
				else                  hitCount--;
				rv *= pr;
				trigS++;
				trigE++;
			}
			break;
		}
		if (str[i]==0 || str[i]==10) break;
	}
	return (hitCount + arv);
}

int Corpora::indexOf(unsigned char ch)
{
	int r=-1;
	int i;
	unsigned char c;
	
	if (m_Language == _T("EN"))
		c = (unsigned char)tolower(ch);
	else 
		c = ch;
	if ( c>= firstchar && c <= lastchar)
		r = c - firstchar;
	else {   // perhaps char is one of the additional chars of the alphabet
		for (i=0; i<nofadlchars; i++)
			if ( c == addlchars[i] ) {
				r = (lastchar-firstchar) + i +1;
				break;
			}
	}
	return r;
}

unsigned char Corpora::reverseIndex(int x)
{
	if (x >= nofchars)
		return 0;

	if (x <= (lastchar - firstchar) ) 
		return (unsigned char)(firstchar + x);
	else
		return ( addlchars[ (x - (lastchar - firstchar)) ] );
}


void Corpora::addTrigram( unsigned char *trigS, unsigned char *trigE, float count )
{
	int x, y, z;
	size_t state;

	state = trigE - trigS;
	if (state == 0) return;

	if (state == 1) 
		state=state;

	if (m_Language == _T("EN")) // we save only lowercase trigrams
	{
		for (index_t i=0; i<(index_t)state; i++)
		{
			trigS[i] = (unsigned char)tolower(trigS[i]);
		}
	}

	switch (state) 
	{
	case 1:
		x = indexOf(trigS[0]);
		
		if (x==26)
			x=26;

		if (x<0) return;
		if (x>=nofchars) return;
		singletons[x] +=count;
		noftrigrams +=count;
		break;
	case 2:
		x = indexOf(trigS[0]);
		y = indexOf(trigS[1]);
		if (x<0 || y<0) return;
		if (x>=nofchars || y>=nofchars) return;
		doubles[x][y] +=count;
		noftrigrams +=count;
		break;
	case 3:
		x = indexOf(trigS[0]);
		y = indexOf(trigS[1]);
		z = indexOf(trigS[2]);
		if (x<0 || y<0 || z<0) return;
		if (x>=nofchars || y>=nofchars || z>=nofchars) return;
		cube[x][y][z] +=count;
		noftrigrams +=count;
		break;
	default:
		return;
	}
}

bool Corpora::isWordTerminator(unsigned char c) 
{
	if (c == ' ' || c == '.' || c==',' || c== '!' || c=='?' || c==0 || c == 10 || c == ';' || c == ':')
		return true;
	else 
		return false;
}

unsigned char *Corpora::getWordCount(FILE *fp, float *count, unsigned char *word, int sizeofWord)
{
	static bool readNextLine=true;
	static unsigned char line[MAXLINELEN];
	static unsigned char *linePoint;

	unsigned char *rp;
	char tempStr[MAXLINELEN];
	int i;
	char *retVal;

	while (readNextLine) 
	{
		retVal = fgets( (char *) line, MAXLINELEN, fp);
		if (retVal == NULL) return NULL;
		for (rp=line; *rp == ' '; rp++) ; // skip empty lines
		if (*rp == '\n' || *rp == 0)
			continue;
		else { 
			linePoint = line;
			readNextLine = false;
		}
	}

	if (m_Language == _T("EN"))
	{
		for(rp=linePoint; *rp; rp++) 
			if ( *rp != ' ') 
				break;                   // skip leading blanks
		for(; *rp; rp++) 
			if ( *rp == ' ' || *rp=='=') 
				break;                   // pass the word
		for(; *rp; rp++) 
			if ( *rp>='0' && *rp <='9') 
				break;                    // run to count
		for(; *rp; rp++) 
			if ( (*rp<'0' || *rp > '9' ) && *rp!='.') 
				break;                   // run on count
		for(;*rp==' ';rp++) ;            //skip trailing blanks
		}
	else {
		for(rp=linePoint; *rp; rp++) 
			if (*rp =='\n') 
				break;                               // take all content until new-line
	}
	
	for(i=0; linePoint+i < rp; i++) 
		tempStr[i] = linePoint[i];                   // copy temporary string
	tempStr[i] = 0;
	if (m_Language == _T("EN"))
		sscanf_s(tempStr, "%s = %f", word, sizeofWord, count);
	else 
		sscanf_s(tempStr, "%f\t%s", count, word, sizeofWord);

	linePoint = rp;
	if (*rp=='\n') readNextLine = true;
	return linePoint;
}

void Corpora::buildCube(FILE *fp)
{
	unsigned char *trigS, *trigE;
	int i; int state = 0; float count;
	unsigned char word[MAXLINELEN];
	//get language essetials first-char, last char, additional-chars
	float  atpw, rwords, csize;
	fscanf_s(fp, "%s = %f", word, sizeof(word), &csize);
	if (strcmp( (char *) word, "CorpusSize") == 0)
	 corpusSize = csize;

	while ( getWordCount(fp, &count, word, sizeof(word)) !=NULL)
	{   
	 state = 0;
	 trigS = trigE = word;
	 for(i=0; i<MAXLINELEN; i++) {
		 switch (state) {
		 case 0:
			 if ( isWordTerminator(word[i])) { 
				 continue; 
			 }
			 else {
				 trigS=word+i;
				 trigE=word+i+1;
				 state = 1;	 }
			 break;
		 case 1:
		 case 2:
			 if ( isWordTerminator(word[i])) {
				 addTrigram(trigS, trigE, count);
				 nofwords += count;
				 state = 0;		 }
			 else {
				 state ++;
				 trigE++; 		 }
		 
			 break;
		 case 3:
			 if (isWordTerminator(word[i])) {
				 addTrigram(trigS, trigE, count);
				 nofwords += count;
				 state = 0;
			 }
			 else { // word continues more than 3 chars
				 addTrigram(trigS, trigE, count);
				 trigS++;
				 trigE++;
			 }
			 break;
		 }
		 if (word[i]==0 || word[i]==10) break;
	 }
	}
	atpw = noftrigrams / nofwords ;                          // avarage trigrams per word
	rwords = corpusSize - nofwords;                          // number of remaining words not listed in this file
	enoft  = noftrigrams + rwords * atpw ;                   // extrapulated # of trigrams
}
		 

float Corpora::trigProb(const char *trigS, const char *trigE)
{
	size_t i = trigE - trigS;
	int x, y, z;
	float rv;

	switch (i) 
	{
	case 1: 
		x = indexOf(trigS[0]);
		if (x<0 || x>=nofchars)
		 rv = 0.0;
		else rv = ((float) (1e-15)) * singletons[x];         // singletons have very low significance
	case 2:
		x = indexOf(trigS[0]);
		y = indexOf(trigS[1]);
		if (x<0 || x >= nofchars || y<0 || y >= nofchars)
		 rv = 0.0;
		else rv = 2 * doubles[x][y];
		break;
	case 3:
		x = indexOf(trigS[0]);
		y = indexOf(trigS[1]);
		z = indexOf(trigS[2]);
		if (x<0 || x>=nofchars || y<0 || y>=nofchars || z<0 || z>=nofchars)
		 rv = 0;
		else rv = 3 * cube[x][y][z];
		break;
	default:
		rv = 0.0;
	}
	if (rv == 0.0)
		return (float) SMALLESTVALUE;
	else 
		return rv / enoft;
}
