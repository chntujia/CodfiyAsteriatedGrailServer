#include "UserAccountDAO.h"
#include <string>
#include <mysql.h>
#include <prepared_statement.h>
using namespace std;
using namespace sql;

void UserAccountDAO::insert(string username, string password, string nickname, int status)
{
   PreparedStatement* update = connection->prepare("insert into user(UserName,Password,NickName,Status) values(?,?,?,?)");
   update->setString(1, username);
   update->setString(2, password);
   update->setString(3, nickname);
   update->setInt(4, status);
   connection->executeUpdate(update);
}
void UserAccountDAO::gameComplete(string username)
{
	PreparedStatement* update = connection->prepare("update user set FinishTimes=FinishTimes+1 where UserName=?");

	update->setString(1, username);
	connection->executeUpdate(update);
}

void UserAccountDAO::gameStart(string username)
{
	PreparedStatement* update = connection->prepare("update user set PlayTimes=PlayTimes+1 where UserName=?");
	update->setString(1, username);
	connection->executeUpdate(update);
}

struct UserAccount UserAccountDAO::query(string username, string password)
{
   ACCOUNT_STATUS status;
   string nickname = "NA";
   PreparedStatement* query = connection->prepare("select NickName,Status from user where UserName=? and Password=?");
   query->setString(1, username);
   query->setString(2, password);
   ResultSet* res = connection->executeQuery(query);
   if(res && res->next()){
       status = (ACCOUNT_STATUS)res->getInt("Status");
	   nickname = res->getString("NickName");
   }
   else{
	   status = STATUS_LOGIN_FAILED;
   }
   if(status == STATUS_NORMAL || status == STATUS_VIP){
	   PreparedStatement* update = connection->prepare("update user set LastLogInTime=NOW() where UserName=? ");

	   update->setString(1, username);
	   connection->executeUpdate(update);
   }
   delete res;
   struct UserAccount account = {username, nickname, status};
   return account;
}
