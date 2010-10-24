#include "PrecompiledHeader.h"

#include "DeckMetaData.h"
#include "DeckStats.h"
#include "MTGDeck.h"
#include "utils.h"

//Possible improvements:
//Merge this with DeckStats
//Have this class handle all the Meta Data rather than relying on MTGDeck. Then MTGDeck would have a MetaData object...


DeckMetaDataList * DeckMetaDataList::decksMetaData = NEW DeckMetaDataList();

DeckMetaData::DeckMetaData(){
  
}

DeckMetaData::DeckMetaData(string filename, Player * statsPlayer){
  load(filename);
}


void DeckMetaData::loadStatsForPlayer( Player * statsPlayer, string deckStatsFileName )
{
  DeckStats * stats = DeckStats::GetInstance();
  if ( statsPlayer )
  {
    stats->load(statsPlayer);
    DeckStat * opponentDeckStats = stats->getDeckStat(deckStatsFileName);
    if ( opponentDeckStats )
    {
      _percentVictories = stats->percentVictories(deckStatsFileName);
      _victories = opponentDeckStats->victories;
      _nbGamesPlayed = opponentDeckStats->nbgames;
      if (_percentVictories < 34)
      {
        _difficulty = HARD;
      }
      else if (_percentVictories < 67)
      {
        _difficulty = NORMAL;
      }
      else
      {
        _difficulty = EASY;
      }
    }
  }
  else
  {
    if(fileExists(deckStatsFileName.c_str())){
      stats->load(deckStatsFileName.c_str());
      _nbGamesPlayed = stats->nbGames();
      _percentVictories = stats->percentVictories();
    }
  }
  stats = NULL;
}
   
void DeckMetaData::load(string filename){
  MTGDeck * mtgd = NEW MTGDeck(filename.c_str(),NULL,1);
  _name = trim( mtgd->meta_name );
  _desc =  trim( mtgd->meta_desc );
  _deckid = atoi( (filename.substr( filename.find("deck") + 4, filename.find(".txt") )).c_str() );
  _percentVictories = 0;
  _nbGamesPlayed = 0;
  _filename = filename;
  _victories = 0;
  delete(mtgd);
}


DeckMetaDataList::~DeckMetaDataList(){
  for(map<string,DeckMetaData *>::iterator it = values.begin(); it != values.end(); ++it){
    SAFE_DELETE(it->second);
  }
  values.clear();
}

void DeckMetaDataList::invalidate(string filename){
   map<string,DeckMetaData *>::iterator it = values.find(filename);
   if (it !=values.end()){
     SAFE_DELETE(it->second);
     values.erase(it);
   }
}

DeckMetaData * DeckMetaDataList::get(string filename, Player * statsPlayer){
  map<string,DeckMetaData *>::iterator it = values.find(filename);
  if (it ==values.end()){
    if (fileExists(filename.c_str())) {
      values[filename] = NEW DeckMetaData(filename, statsPlayer);
    }
  }

   return values[filename]; //this creates a NULL entry if the file does not exist
}

//Accessors

string DeckMetaData::getFilename()
{
  return _filename;
}

string DeckMetaData::getName()
{
  return _name;
}

int DeckMetaData::getDeckId()
{
  return _deckid;
}


int DeckMetaData::getGamesPlayed()
{
  return _nbGamesPlayed;
}


int DeckMetaData::getVictories()
{
  return _victories;
}

int DeckMetaData::getVictoryPercentage()
{
  return _percentVictories;
}

int DeckMetaData::getDifficulty()
{
  return _difficulty;
}

string DeckMetaData::getDescription()
{
    char deckDesc[512];
    string difficultyString = "";
    switch( _difficulty )
    {
        case HARD: 
            difficultyString = "Hard";
            break;
        case EASY:
            difficultyString = "Easy";
            break;
    }
    if ( _nbGamesPlayed > 0 && difficultyString != "")    
      sprintf(deckDesc, "Deck: %s\nDifficulty: %s\nVictory %%: %i\nGames Played: %i\n\n%s", _name.c_str(), difficultyString.c_str(), _percentVictories, _nbGamesPlayed, _desc.c_str() );
    else if ( _nbGamesPlayed > 0 )
      sprintf(deckDesc, "Deck: %s\nVictory %%: %i\nGames Played: %i\n\n%s", _name.c_str(), _percentVictories, _nbGamesPlayed, _desc.c_str() );
    else
      sprintf(deckDesc, "Deck: %s\n\n%s", _name.c_str(), _desc.c_str() );

    return deckDesc;
}
