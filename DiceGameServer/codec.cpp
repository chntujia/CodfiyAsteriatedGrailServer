#include "codec.h"

bool proto_encoder(uint16_t type, ::google::protobuf::Message& body, string& msg)
{
	MsgHeader header;

	string str;
	body.SerializeToString(&str);
	header.len = str.length() + SIZEOF_HEADER;
	header.type = type;
	
	msg.clear();
	msg.append((char*)&header, SIZEOF_HEADER);
	msg.append(str);
	
	return true;
}

/*
msg：接收到的信息
type：信息类型（返回值）

return：反串化后的协议指针（请记得删除）
*/
void* proto_decoder(const char* msg, uint16_t& type)
{
	MsgHeader* header_ptr = (MsgHeader*)msg;
	::google::protobuf::Message *proto = NULL;

	type = header_ptr->type;
	switch (header_ptr->type)
	{
	case MSG_REGISTER_REQ:
		proto = new RegisterRequest();
		proto->ParseFromArray(msg + SIZEOF_HEADER, header_ptr->len - SIZEOF_HEADER);
		break;
	case MSG_LOGIN_REQ:
		proto = new LoginRequest();
		proto->ParseFromArray(msg + SIZEOF_HEADER, header_ptr->len - SIZEOF_HEADER);
		break;
	case MSG_LOGOUT_REQ:
		proto = new LogoutRequest();
		proto->ParseFromArray(msg + SIZEOF_HEADER, header_ptr->len - SIZEOF_HEADER);
		break;
	case MSG_ENTER_ROOM:
		proto = new EnterRoom();
		proto->ParseFromArray(msg + SIZEOF_HEADER, header_ptr->len - SIZEOF_HEADER);
		break;
	case MSG_START_REP:
		proto = new StartReply();
		proto->ParseFromArray(msg + SIZEOF_HEADER, header_ptr->len - SIZEOF_HEADER);
		break;
	case MSG_TALK:
		proto = new Talk();
		proto->ParseFromArray(msg + SIZEOF_HEADER, header_ptr->len - SIZEOF_HEADER);
		break;
	case MSG_PICK:
		proto = new Pick();
		proto->ParseFromArray(msg + SIZEOF_HEADER, header_ptr->len - SIZEOF_HEADER);
		break;
	case MSG_ACTION:
		proto = new Action();
		proto->ParseFromArray(msg + SIZEOF_HEADER, header_ptr->len - SIZEOF_HEADER);
		break;
	case MSG_RESPOND:
		proto = new Respond();
		proto->ParseFromArray(msg + SIZEOF_HEADER, header_ptr->len - SIZEOF_HEADER);
		break;
	default:
		type = 0;
		return NULL;
		break;
	}
	return proto;
}