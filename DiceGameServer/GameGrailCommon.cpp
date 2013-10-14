#include "GameGrailCommon.h"
CardEntity* getCardByID(int id)
{
    if(id>=0 && id<150)
        return cardList[id];
	ztLoggerWrite(ZONE, e_Error, "Card: %d Requested out of Rank!", id);
	throw GE_INVALID_CARDID;
}
//按通讯协议的格式连接各QString
string combMessage(string item1,string item2, string item3, string item4, string item5, string item6, string item7)
{
    string message;
    message = item1;
    message += ";";
    if(item2 != "")
    {
        message += item2;
        message += ";";
    }
    if(item3 != "")
    {
        message += item3;
        message += ";";
    }
    if(item4 != "")
    {
        message += item4;
        message += ";";
    }
    if(item5 != "")
    {
        message += item5;
        message += ";";
    }
    if(item6 != "")
    {
        message += item6;
        message += ";";
    }
    if(item7 != "")
    {
        message += item7;
        message += ";";
    }
	return message;
}

string Coder::useCardNotice(int cardID, int dstID, int srcID,int realCard)
{
    return combMessage("28",TOQSTR(cardID),TOQSTR(dstID),TOQSTR(srcID),TOQSTR(realCard));
}

string Coder::getCardNotice(int sum, vector < int > cards, int dstID, bool show)
{
    string cardsID;
    for(int i = 0;i < cards.size();i++)
    {
        cardsID += TOQSTR(cards[i]);
        if(i < cards.size() - 1)
            cardsID += ",";
    }
    return combMessage("41",TOQSTR(dstID),TOQSTR(show),TOQSTR(sum),cardsID);
}
string Coder::discardNotice(int ID,int sum,string show,vector < int > cards)
{
    string message = combMessage("13",TOQSTR(ID),TOQSTR(sum),show);
    for(int i = 0;i < cards.size();i++)
    {
        message += TOQSTR(cards[i]);
        if(i != cards.size() - 1)
            message += ",";
        else
            message += ";";
    }
    return message;
}

string Coder::askForRolePick(int howMany, int*roles)
{
    string message="46;"+TOQSTR(howMany)+";";
    for(int i=0;i<howMany;i++)
        message+=TOQSTR(roles[i])+";";
    return message;
}

string Coder::coverCardNotice(int playerID, int howMany, vector < int > cards, bool remove, bool show)
{
    string message = "48;" + TOQSTR(playerID) + ";" + TOQSTR(howMany) + ";";
    for(int i = 0;i < howMany;i++)
    {
        message += TOQSTR(cards[i]);
        if(i != howMany - 1)
            message += ",";
    }
    message += ";" + TOQSTR((int)remove) + ";" + TOQSTR((int)show) + ";";
    return message;
}

string Coder::optionalRoleNotice(int num, int *roles)
{
    string message="51;"+TOQSTR(num)+";";
    for(int i=0;i<num;i++)
        message+=TOQSTR(roles[i])+";";
    return message;
}

string Coder::askForBan(int ID)
{
    string message="52;"+TOQSTR(ID)+";";
    return message;
}

string Coder::banNotice(int ID, int role)
{
    string message = "54;"+TOQSTR(ID)+";"+TOQSTR(role)+";";
    return message;
}

string Coder::askForPick(int ID)
{
    string message="55;"+TOQSTR(ID)+";";
    return message;
}

string Coder::pickNotice(int ID, int role)
{
    string message = "57;"+TOQSTR(ID)+";"+TOQSTR(role)+";";
    return message;
}