#include "DBManager.h"
#include "GameServer.h"

void show_error(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER iError;
	WCHAR wszMessage[1000];
	WCHAR wszState[SQL_SQLSTATE_SIZE + 1];
	if (RetCode == SQL_INVALID_HANDLE) {
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5)) {
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}

DBManager::DBManager(GameServer* server)
{
	henv = 0;
	hdbc = 0;
	game_server = server;

	ConnectDB(L"DeathMetal");
}

DBManager::~DBManager()
{
	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

void DBManager::ConnectDB(const std::wstring& odbc_name)
{
	SQLRETURN retcode;

	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv); // Allocate environment handle
	retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0); // Set the ODBC version environment attribute  
	retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc); // Allocate connection handle  
	SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0); // Set login timeout to 5 seconds
	retcode = SQLConnect(hdbc, (SQLWCHAR*)L"DeathMetalDB", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0); // Connect to data source  

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		std::cout << "Completed Connecting DB\n";
	}
	else {
		show_error(hdbc, SQL_HANDLE_STMT, retcode);
		exit(-1);
	}
}

void DBManager::PushEvent(DB_BUF& query)
{
	query_queue.push(query);
}

void DBManager::ExecLogin(const short s_id, const char* login_id, const char* login_pw)
{
	std::wstring id(login_id, &login_id[strlen(login_id)]);
	std::wstring pw(login_pw, &login_pw[strlen(login_pw)]);
	std::wstring query{ L"EXEC Get_Login_Info " + id + L", " + pw};

	SQLHSTMT hstmt;
	SQLRETURN retcode;
	SQLCHAR player_name[USER_NAME_SIZE]{};
	SQLINTEGER player_rating{}, player_state{};
	SQLLEN cb_name = 0, cb_rating = 0, cb_state = 0;

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)query.c_str(), SQL_NTS);

	if (retcode == SQL_SUCCESS) {
		retcode = SQLBindCol(hstmt, 1, SQL_C_CHAR, player_name, USER_NAME_SIZE, &cb_name);
		retcode = SQLBindCol(hstmt, 2, SQL_C_LONG, &player_rating, 10, &cb_rating);
		retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &player_state, 2, &cb_state);

		retcode = SQLFetch(hstmt);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			DS_LOGIN_INFO packet{};

			packet.rating = player_rating;
			packet.state = (NET_STATE) player_state;
			strcpy_s(packet.name, (char*) player_name);

			OVER_EXP* exover = new OVER_EXP;
			exover->comp_type = OP_TYPE::RESULT_LOGIN;
			memcpy(exover->io_buf, &packet, sizeof(DS_LOGIN_INFO));

			game_server->PQCS(exover, s_id);
		}
		else {
			retcode = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			return;
		}
	}
	else {
		show_error(hstmt, SQL_HANDLE_STMT, retcode);
	}

	retcode = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
}

void DBManager::ExecRegister(const short s_id, const char* login_id, const char* login_pw, const char* name)
{
	std::wstring id(login_id, &login_id[strlen(login_id)]);
	std::wstring pw(login_pw, &login_pw[strlen(login_pw)]);
	std::wstring nick_name(name, &name[strlen(name)]);
	std::wstring query{ L"EXEC Create_Account " + id + L", " + pw + L", " +nick_name};

	SQLHSTMT hstmt;
	SQLRETURN retcode;

	SQLCHAR player_name[USER_NAME_SIZE]{};
	SQLLEN cb_name = 0;

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)query.c_str(), SQL_NTS);

	if (retcode == SQL_SUCCESS) {
		retcode = SQLBindCol(hstmt, 1, SQL_C_CHAR, player_name, USER_NAME_SIZE, &cb_name);
		retcode = SQLFetch(hstmt);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			DS_LOGIN_INFO packet{};

			packet.rating = 0;
			packet.state = NET_STATE::LOBBY;
			strcpy_s(packet.name, (char*) player_name);

			OVER_EXP* exover = new OVER_EXP;
			exover->comp_type = OP_TYPE::RESULT_LOGIN;
			memcpy(exover->io_buf, &packet, sizeof(DS_LOGIN_INFO));

			game_server->PQCS(exover, s_id);
		}
		else {
			retcode = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			// register fail
			return;
		}
	}
	else {
		show_error(hstmt, SQL_HANDLE_STMT, retcode);
	}

	retcode = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
}

void DBManager::ExecLogout(const char* login_id, const vector3 loc, const vector3 rot, const int hp)
{

}

void DBManager::DatabaseThread()
{
	DB_BUF query;
	while (true) {
		if (true == query_queue.try_pop(query)) {
			DB_QUERY type = static_cast<DB_QUERY>(query.buf[0]);
			switch (type) {
				case DB_QUERY::LOGIN: {
					SD_LOGIN_INFO* p = reinterpret_cast<SD_LOGIN_INFO*>(query.buf);
					ExecLogin(p->client_id, p->id, p->pw);
				}
					break;
				case DB_QUERY::CREATE: {
					SD_REGISTER* p = reinterpret_cast<SD_REGISTER*>(query.buf);
					ExecRegister(p->client_id, p->id, p->pw, p->name);
				}
					break;
				case DB_QUERY::LOGOUT: {
					//ExecLogOut();
				}
					break;
				default: break;
			}
		}
	}
}
