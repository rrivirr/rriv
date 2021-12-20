#ifndef WATERBEAR_WRITE_CACHE
#define WATERBEAR_WRITE_CACHE

#define MAX_CACHE_SIZE 1000

class OutputDevice
{
  public:
    virtual void writeString(char * string);


};

class WriteCache 
{

  public:
  // methods
  WriteCache(OutputDevice * outputDevice);
  void writeString(char * string);
  void endOfLine();
  void flushCache();

  // variables
  unsigned int cacheSize = MAX_CACHE_SIZE; // must be MAX_CACHE_SIZE or less

  private:
  // methods
  void initCache();

  // variables
  OutputDevice * outputDevice;
  char cache[MAX_CACHE_SIZE];
  unsigned int nextPosition = 0;

};


#endif