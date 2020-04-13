#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <thread>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <vector>
#include <list>
#include <map>
#include <mutex>

#define MAX_CLIENTS 100
#define MAX_STR_SIZE 500

using namespace std;

class ClientInfo
{
  public:
	int client_fd;
	string client_name;
	ClientInfo(int cfd, string cn)
	{
		client_fd = cfd;
		client_name = cn;
	}
};

vector<pair<thread, ClientInfo*>> thread_client;
thread answer_thread[MAX_CLIENTS];
// list<pair<thread, ClientInfo>> thread_list;

void answer_client(ClientInfo *client_info)
{
	char msg[100];

	while (1)
	{
		bzero(msg, 100);
		string msg_plus_name = client_info->client_name + ": ";

		if (read(client_info->client_fd, msg, 100))
		{
			cout << "Recebi do cliente: " << string(msg) << flush;
			msg_plus_name += string(msg);

			for (auto &t : thread_client)
			{
				if (t.second->client_fd == 0 || client_info->client_fd == t.second->client_fd)
					continue;
				write(t.second->client_fd, msg_plus_name.c_str(), msg_plus_name.length() + 1);
			}

			// write(client_info.client_fd, msg, strlen(msg) + 1);
		}

		else
		{
			client_info->client_fd = 0;
			cout << client_info->client_name << " se desconectou" << endl;
			break;
		}
	}
}

int main(int argc, char **argv)
{
	int server_port = atoi(argv[1]);
	string server_name = string(argv[2]);

	cout << "Iniciando bate-papo " << server_name << " na porta " << server_port << endl;

	/* Construção do servidor */
	struct sockaddr_in server_addr;
	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&server_addr, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(server_port);

	bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	listen(listen_fd, MAX_CLIENTS);

	int thread_count = 0;

	while (1)
	{
		struct sockaddr_in client_addr;
		int client_addr_len = sizeof(client_addr);
		int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len);
		char username[MAX_STR_SIZE];
		read(client_fd, username, 100);

		cout << string(username) << " se conectou com o IP " << inet_ntoa(client_addr.sin_addr) << endl;

		ClientInfo *client_info = new ClientInfo(client_fd, string(username));

		// answer_thread[thread_count++] = thread(answer_client, client_info);
		thread_client.push_back({thread(answer_client, client_info), client_info});
	}

	return 0;
}
