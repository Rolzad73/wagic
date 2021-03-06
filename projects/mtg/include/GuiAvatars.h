#ifndef _GUIAVATARS_H_
#define _GUIAVATARS_H_

#include "GuiLayers.h"

struct GuiAvatar;
class GuiGraveyard;
class GuiLibrary;
class GuiOpponentHand;
class GuiExile;
class GuiAvatars: public GuiLayer
{
protected:
    GuiAvatar* self, *opponent;
    GuiGraveyard* selfGraveyard, *opponentGraveyard;
    GuiLibrary* selfLibrary, *opponentLibrary;
    GuiOpponentHand *opponentHand;
    GuiExile* selfExile, *opponentExile;
    GuiAvatar* active;

public:
    GuiAvatars(DuelLayers* duelLayers);
    ~GuiAvatars();

    GuiAvatar* GetSelf();
    GuiAvatar* GetOpponent();
    void Update(float dt);
    void Render();
    void Activate(PlayGuiObject* c);
    void Deactivate(PlayGuiObject* c);
    int receiveEventPlus(WEvent*);
    int receiveEventMinus(WEvent*);
    bool CheckUserInput(JButton key);
    float LeftBoundarySelf();
};

#endif // _GUIAVATARS_H_
