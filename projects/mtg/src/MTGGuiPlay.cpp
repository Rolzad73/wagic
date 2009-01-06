/* This class handles the display on the main game screen :
   cards in play, graveyard, library, games phases, Players avatars
*/

#include "../include/config.h"
#include "../include/MTGGuiPlay.h"
#include "../include/MTGCardInstance.h"
#include "../include/CardGui.h"
#include "../include/CardDisplay.h"

#define ZX_MAIN 100
#define ZY_MAIN 22
#define ZH_CREATURES 50
#define Z_CARDWIDTH 30
#define Z_CARDHEIGHT 40
#define Z_MAIN_NBCARDS 7
#define Z_SPELLS_NBCARDS 3
#define ZX_SPELL 450
#define ZY_SPELL 22




MTGGuiPlay::MTGGuiPlay(int id, GameObserver * _game):PlayGuiObjectController(id, _game){
  currentPlayer = NULL;
  offset = 0;


  mPhaseBarTexture = JRenderer::GetInstance()->LoadTexture("graphics/phasebar.png", TEX_TYPE_USE_VRAM);
  for (int i=0; i < 12; i++){
    phaseIcons[2*i] = NEW JQuad(mPhaseBarTexture, i*28, 0, 28, 28);
    phaseIcons[2*i + 1] = NEW JQuad(mPhaseBarTexture, i*28, 28, 28, 28);
  }
  mGlitter = NEW JQuad(mPhaseBarTexture, 392, 0, 5, 5);
  mGlitter->SetHotSpot(2.5,2.5);
  mGlitterAlpha = -1;
  mFont= GameApp::CommonRes->GetJLBFont("graphics/simon");

  mIconsTexture = JRenderer::GetInstance()->LoadTexture("graphics/menuicons.png", TEX_TYPE_USE_VRAM);
  //load all the icon images
  mIcons[Constants::MTG_COLOR_ARTIFACT] = NEW JQuad(mIconsTexture, 2+6*36, 38, 32, 32);
  mIcons[Constants::MTG_COLOR_LAND] = NEW JQuad(mIconsTexture, 2+5*36, 38, 32, 32);
  mIcons[Constants::MTG_COLOR_WHITE] = NEW JQuad(mIconsTexture, 2+4*36, 38, 32, 32);
  mIcons[Constants::MTG_COLOR_RED] = NEW JQuad(mIconsTexture, 2+3*36, 38, 32, 32);
  mIcons[Constants::MTG_COLOR_BLACK] = NEW JQuad(mIconsTexture, 2+2*36, 38, 32, 32);
  mIcons[Constants::MTG_COLOR_BLUE] = NEW JQuad(mIconsTexture, 2+1*36, 38, 32, 32);
  mIcons[Constants::MTG_COLOR_GREEN] = NEW JQuad(mIconsTexture, 2+0*36, 38, 32, 32);
  for (int i=0; i < 7; i++){
    mIcons[i]->SetHotSpot(16,16);
  }

  mBgTex = JRenderer::GetInstance()->LoadTexture("graphics/background.png", TEX_TYPE_USE_VRAM);
  mBg = NEW JQuad(mBgTex, 0, 0, 480, 272);

  mBgTex2 = JRenderer::GetInstance()->LoadTexture("graphics/back.jpg", TEX_TYPE_USE_VRAM);
  mBg2 = NEW JQuad(mBgTex2, 0, 0, 480, 255);
  for (int i= 0; i < 4; i++){
    alphaBg[i] = 255;
  }
  alphaBg[0] = 0;
  AddPlayersGuiInfo();
}


CardGui * MTGGuiPlay::getByCard(MTGCardInstance * card){
  for (int i = offset; i < mCount; i++){
    CardGui * cardg = (CardGui *)mObjects[i];
    if(cardg && cardg->card == card){
      return cardg;
    }
  }
  return NULL;

}

void MTGGuiPlay::initCardsDisplay(){
  for (int i = 0; i < SCREEN_WIDTH/5; i++){
    for(int j=0; j < SCREEN_HEIGHT/5; j++){
      cardsGrid[i][j] = NULL;
    }
  }
  cards_x_limit = 12;
  nb_creatures = 0;
  nb_lands = 0;
  nb_spells = 0;
}


void MTGGuiPlay::adjustCardPosition(CardGui * cardg){
  int x5 = cardg->x / 5;
  int y5 = cardg->y / 5;

  while (cardsGrid[x5][y5] && x5 <SCREEN_WIDTH/5 && y5 < SCREEN_HEIGHT/5 ){
    x5++;
    y5++;
  }
  cardg->x = x5 * 5;
  cardg->y = y5 * 5;
  cardsGrid[x5][y5] = cardg->card;
}

void MTGGuiPlay::setCardPosition(CardGui * cardg, int player, int playerTurn, int spellMode){
  MTGCardInstance * card = cardg->card;
  if (card->target)
    return;
  if (spellMode && (card->isACreature() || card->hasType("land"))) return;
  if (!spellMode && !card->isACreature() && !card->hasType("land")) return;
  if (card->isACreature()){
    int x_offset = nb_creatures % cards_x_limit;
    int y_offset = nb_creatures / cards_x_limit;
    cardg->x= ZX_MAIN + (Z_CARDWIDTH * x_offset);
    cardg->y=ZY_MAIN +  ZH_CREATURES + (Z_CARDHEIGHT * y_offset) + 100 * (1-player);
    nb_creatures++;

    if (playerTurn){
      if (card->isAttacker()){
	cardg->y=122 + 30 * (1-player);
      }
    }else{
      if (card->isDefenser()){
	CardGui * targetg = getByCard(card->isDefenser());
	if (targetg) cardg->x = targetg->x;
	cardg->y=122 + 30 * (1-player);
      }
    }

  }else if(card->hasType("land")){
    int x_offset = nb_lands % cards_x_limit;
    int y_offset = nb_lands/ cards_x_limit;
    cardg->x=ZX_MAIN + (Z_CARDWIDTH * x_offset);
    cardg->y=ZY_MAIN + (Z_CARDHEIGHT * y_offset) + 200 * (1-player);
    nb_lands++;
  }else{
    int y_offset = nb_spells % Z_SPELLS_NBCARDS;
    int x_offset = nb_spells/ Z_SPELLS_NBCARDS;
    cardg->x=ZX_SPELL - (Z_CARDWIDTH * x_offset);
    cardg->y=ZY_SPELL + (Z_CARDHEIGHT * y_offset) + 125 * (1-player);
    nb_spells++;
    cards_x_limit = 12 - (nb_spells + 2)/ Z_SPELLS_NBCARDS;
  }
  adjustCardPosition(cardg);
}


void MTGGuiPlay::setTargettingCardPosition(CardGui * cardg, int player, int playerTurn){
  MTGCardInstance * card = cardg->card;
  MTGCardInstance * target = card->target;
  if (!target)
    return;
  CardGui * targetg = getByCard(target);
  if (targetg){
    cardg->y=targetg->y + 5;
    cardg->x=targetg->x + 5;
  }
  adjustCardPosition(cardg);
  return;
}

void MTGGuiPlay::updateCards(){
  GameObserver * game = GameObserver::GetInstance();
  Player * player = game->players[0];
  int player0Mode =(game->currentPlayer == player);
  int nb_cards = player->game->inPlay->nb_cards;
  MTGCardInstance * attackers[MAX_ATTACKERS];
  for (int i = 0; i <MAX_ATTACKERS; i++){
    attackers[i] = NULL;
  }

  offset = 6;

  Player * opponent = game->players[1];
  int opponent_cards = opponent ->game->inPlay->nb_cards;
  if (mCount - offset != (nb_cards+opponent_cards) || game->currentPlayer != currentPlayer ){ //if the number of cards has changed, then an update occured (is this test engouh ?)
    resetObjects();
    AddPlayersGuiInfo();
    offset = mCount;

    bool hasFocus = player0Mode;

    for (int i = 0;i<nb_cards; i++){
      if (hasFocus) mCurr = mCount ;
      CardGui * object = NEW CardGui(mCount, player->game->inPlay->cards[i],40, i*35 + 10, 200, hasFocus);
      Add(object);
      hasFocus = false;
    }
    hasFocus = !player0Mode;
    for (int i = 0;i<opponent_cards; i++){
      if (hasFocus) mCurr = mCount ;
      CardGui * object = NEW CardGui(mCount, opponent->game->inPlay->cards[i],40, i*35 + 10, 10, hasFocus);
      Add(object);
      hasFocus = false;
    }

    currentPlayer = game->currentPlayer;
  }


  //This is just so that we display the cards of the current player first, so that blockers are correctly positionned
  for (int j= 0; j < 2; j++){
    initCardsDisplay();
    if (j != player0Mode){
      for (int i =0; i<nb_cards; i++){
	CardGui * cardGui = (CardGui *)mObjects[i + offset];
	setCardPosition(cardGui, 0, player0Mode, 1);
      }
      for (int i =0; i<nb_cards; i++){
	CardGui * cardGui = (CardGui *)mObjects[i + offset];
	setCardPosition(cardGui, 0, player0Mode, 0);
      }
    }else{
      for (int i =0; i<opponent_cards; i++){
	CardGui * cardGui = (CardGui *)mObjects[nb_cards + i + offset];
	setCardPosition(cardGui, 1, !player0Mode,1);
      }
      for (int i =0; i<opponent_cards; i++){
	CardGui * cardGui = (CardGui *)mObjects[nb_cards + i + offset];
	setCardPosition(cardGui, 1, !player0Mode,0);
      }
    }
  }

  for (int i =0; i<nb_cards; i++){
    CardGui * cardGui = (CardGui *)mObjects[i + offset ];
    setTargettingCardPosition(cardGui, 0, player0Mode);
  }

  for (int i =0; i<opponent_cards; i++){
    CardGui * cardGui = (CardGui *)mObjects[nb_cards + i + offset];
    setTargettingCardPosition(cardGui, 1, !player0Mode);
  }



}


void MTGGuiPlay::AddPlayersGuiInfo(){
  //init with the players objects
  if (mCount == 0){
    Add(NEW GuiAvatar(-1,50,2,155,false, GameObserver::GetInstance()->players[0]));
    Add(NEW GuiAvatar(-2,50,2,30,false,GameObserver::GetInstance()->players[1]));

    Add(NEW GuiGraveyard(-3,30,40,150,false, GameObserver::GetInstance()->players[0]));
    Add(NEW GuiLibrary(-4,30,40,180,false, GameObserver::GetInstance()->players[0]));


    Add(NEW GuiGraveyard(-5,30,40,30,false, GameObserver::GetInstance()->players[1]));
    Add(NEW GuiLibrary(-6,30,40,60,false, GameObserver::GetInstance()->players[1]));
  }
}

void MTGGuiPlay::Update(float dt){
  updateCards();
  PlayGuiObjectController::Update(dt);
}



bool MTGGuiPlay::CheckUserInput(u32 key){
  for (int i = 2; i<6;i++){
    GuiGameZone * zone = (GuiGameZone *)mObjects[i];
    if (zone->showCards){
      return zone->cd->CheckUserInput(key);
    }
  }
  return PlayGuiObjectController::CheckUserInput(key);
}


void MTGGuiPlay::RenderPlayerInfo(int playerid){
  JRenderer * r = JRenderer::GetInstance();
  Player * player = GameObserver::GetInstance()->players[playerid];

  //Avatar
  GuiAvatar * avatar = (GuiAvatar *)mObjects[3*playerid];
  avatar->Render();



  //Mana
  ManaCost * cost = player->getManaPool();
  int nbicons = 0;
  for (int j=0; j<6;j++){
    int value = cost->getCost(j);
    for (int i=0; i<value; i++){
      float x = 10 + (nbicons %4) * 15;
      float y = 90 + 125 * (1-playerid) + (15 * (nbicons / 4));
      r->RenderQuad(mIcons[j],x,y,0,0.5, 0.5);
      nbicons++;
    }
  }
}


void MTGGuiPlay::RenderPhaseBar(){
  GameObserver * game = GameObserver::GetInstance();
  JRenderer * renderer = JRenderer::GetInstance();
  int currentPhase = game->getCurrentGamePhase();
  for (int i=0; i < 12; i++){
    int index = 2*i + 1 ;
    if (i==currentPhase-1){
      index-=1;
    }
    renderer->RenderQuad(phaseIcons[index], 200 + 14*i,0,0,0.5,0.5);
  }
  if (game->currentlyActing()->isAI()){
    mFont->SetColor(ARGB(255,128,128,128));
  }else{
    mFont->SetColor(ARGB(255,255,255,255));
  }
  mFont->DrawString(Constants::MTGPhaseNames[currentPhase], 375, 0);
}

void MTGGuiPlay::Render(){
  JRenderer * renderer = JRenderer::GetInstance();

  //alphaBg[1] = 255;
  //alphaBg[2]= 255;
  //alphaBg[3] = 255;
  //mBg2->SetColor(ARGB(alphaBg[0], alphaBg[1],alphaBg[2],alphaBg[3]));
  renderer->RenderQuad(mBg2,0,17);

  if (game->currentGamePhase >= Constants::MTG_PHASE_COMBATBEGIN && game->currentGamePhase < Constants::MTG_PHASE_COMBATEND){
    if (alphaBg[0] < 50){
      alphaBg[3]-=12;
      alphaBg[2]-=12;
      alphaBg[0]+=3;
    }
    alphaBg[1] = 255;

  }else{
    if (alphaBg[0]){
      alphaBg[0]-=3;
      alphaBg[3]+=12;
      alphaBg[2]+=12;
    }
    alphaBg[1] = 255;
  }
  renderer->FillRect(0,0,480,272,ARGB(alphaBg[0], alphaBg[1],alphaBg[2],alphaBg[3]));

  renderer->RenderQuad(mBg,0,0);

  for (int i=0;i<mCount;i++){
    if (mObjects[i]!=NULL && i!=mCurr){
      mObjects[i]->Render();
    }
  }

  RenderPhaseBar();
  RenderPlayerInfo(0);
  RenderPlayerInfo(1);

  if (mGlitterAlpha < 0){
    mGlitterAlpha = 510;
    int position = rand() % 2;
    if (position){
      mGlitterX = 65 + rand() % (420);
      mGlitterY = 17 + rand() % (5);
    }else{
      mGlitterX = 65 + rand() % (5);
      mGlitterY = 15 + rand() % (250);
    }
  }
  mGlitter->SetColor(ARGB((255-abs(255-mGlitterAlpha)),240,240,255));
  renderer->RenderQuad(mGlitter,mGlitterX,mGlitterY, (float)(mGlitterAlpha)/(float)255, 1.2*float(mGlitterAlpha)/float(255),1.2*float(mGlitterAlpha)/float(255));
  mGlitterAlpha-=10;

  if (mCount && mObjects[mCurr] != NULL){
    mObjects[mCurr]->Render();
    if (hasFocus && mCurr >= offset && showBigCards){
        ((CardGui *)mObjects[mCurr])->RenderBig(-1,-1,showBigCards-1);
    }
  }
}

MTGGuiPlay::~MTGGuiPlay(){
  LOG("==Destroying MTGGuiPlay==");
  delete mBg;
  delete mBgTex;
  for (int i=0; i < 7; i++){
    delete mIcons[i];
  }
  delete 	mIconsTexture;
  delete 	mGlitter;
  for (int i=0; i < 12; i++){
    delete phaseIcons[2*i] ;
    delete phaseIcons[2*i + 1];
  }
  delete	mPhaseBarTexture;

  SAFE_DELETE(mBg2);
  SAFE_DELETE(mBgTex2);

  LOG("==Destroying MTGGuiPlay Successful==");

}
