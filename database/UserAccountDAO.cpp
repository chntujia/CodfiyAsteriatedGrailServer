#include "UserAccountDAO.h"
#include <string>
#include <mysql.h>
#include <prepared_statement.h>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/format.hpp"
using namespace std;
using namespace sql;
using namespace boost::posix_time; 

void UserAccountDAO::insert(string username, string password, string nickname, int status)
{
   PreparedStatement* update = connection->prepare("insert into user(UserName,Password,NickName,Status) values(?,?,?,?)");
   update->setString(1, username);
   update->setString(2, password);
   update->setString(3, nickname);
   update->setInt(4, status);
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
	   PreparedStatement* update = connection->prepare("update user set LastLogInTime=? where UserName=? ");
	   ptime now = second_clock::local_time();
	   const boost::wformat f = boost::wformat(L"%s-%02d-%02d %02d:%02d:%02d")
		        % now.date().year_month_day().year
				% now.date().year_month_day().month.as_number()
		        % now.date().year_month_day().day.as_number()
                                
                % now.time_of_day().hours()
                % now.time_of_day().minutes()
				% now.time_of_day().seconds();

	   const std::wstring result = f.str();
	   string lastLogInTime(result.begin(), result.end());
	   update->setString(1, lastLogInTime);
	   update->setString(2, username);
	   connection->executeUpdate(update);
   }
   delete res;
   struct UserAccount account = {username, nickname, status};
   return account;
}
