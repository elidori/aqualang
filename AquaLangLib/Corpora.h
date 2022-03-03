#ifndef _CORPORA_HEADER_
#define _CORPORA_HEADER_

#define MAXADLCHARS 20
#define MAXCHARS 100

class Corpora 
{
protected:
	Corpora(LPCTSTR Language, HKL hKL);                  // constructor of class

	bool Open();
public:
	static Corpora *Create(LPCTSTR Language, HKL hKL);
	virtual ~Corpora();

	void save();

	float probOfStr(const char *str);

	HKL GetKL() const					{ return m_hKL; }
	const _tstring &GetLanguage() const	{ return m_Language; }
	int GetCodePage() const				{ return m_CodePage; }
private: 
	int indexOf(unsigned char ch);
	unsigned char reverseIndex(int x);
	void addTrigram(unsigned char *trigS, unsigned char *trigE, float count);
	bool isWordTerminator(unsigned char c);
	unsigned char *getWordCount(FILE *fp, float *count, unsigned char *word, int sizeofWord);
	void buildCube(FILE *fp);
	float trigProb(const char *trigS, const char *trigE);

private:
	const _tstring m_Language;
	const HKL m_hKL;
	const int m_CodePage;

	int  nofchars;                          // # of chars in language
	unsigned char firstchar, lastchar;            // first and last chars of alphabet
	int  nofadlchars;                        // # of additional characters
	unsigned char addlchars[MAXADLCHARS];         // Additional chars in language such as ", ' etc.
	float cube[MAXCHARS][MAXCHARS][MAXCHARS] ; // the cube of corpora with trigrams
	float doubles[MAXCHARS][MAXCHARS];        // the array of doble char words i.e. "at" in english
	float singletons[MAXCHARS];               // the array of singletons (words with 1 char only like "a" in english
	float corpusSize;                         // how many words were counted in corpus
	float nofwords;							  // sum of all frquencies of words
	
	float noftrigrams;                      // Sum of all frequencies of trigrams
	float enoft;                            // extrapulated # of trigrams
};

#endif // _CORPORA_HEADER_