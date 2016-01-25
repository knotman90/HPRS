#include<iostream>
#include<memory>
#include<string>
#include<vector>

#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include <Hbase.h>


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::hadoop::hbase::thrift;

#define THRIFT_DEFAULT_PORT (9090)

template <typename ADDRESS = std::string , typename PORT = int>
class server_details{
public:
	server_details() : hostname("localhost"), port(THRIFT_DEFAULT_PORT) {}
	server_details(ADDRESS a, PORT p) : hostname(a), port(p) {}
	~server_details() {
	}

	ADDRESS getHostname() const {
		return hostname;
	}

	PORT getPort() const {
		return port;
	};


	const ADDRESS hostname;
	const PORT port;
};


template<
typename RETURN ,
typename Iterator ,
typename Lambda
>
RETURN forall( Iterator s, Iterator e, RETURN D, Lambda l){
	RETURN r = D;
	for(Iterator i = s; i!=e; i++){
		l(*i, r);
	}
	return r;
}

template<
typename Iterator ,
typename Lambda
>
void forall( Iterator s, Iterator e, Lambda l){
	for(Iterator i = s; i!=e; i++){
		l(*i);
	}
}



template< typename Iterator  , typename OutStream = std::ostream>
void print(const Iterator start, const Iterator end, const OutStream &s = std::cout){
	for(auto i=start;i!=end;i++){
		std::cout << *i << " ";
	}
	std::cout << std::endl;
}


int main(int argc, char**argv){

	server_details<std::string> sd("localhost",9090);

	/* Connection to the Thrift Server */
	boost::shared_ptr<TSocket> socket(new TSocket(sd.hostname, sd.port));
	boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
	boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

	/* Create the Hbase client */
	HbaseClient client(protocol);
	std::vector<std::string> table_names;
	try{
		transport->open();
		/* Scan all tables*/
		client.getTableNames(table_names);
		print(table_names.begin(),table_names.end(), std::cout);

		auto f = [&] (std::string &s, int& acc)
								 {
				if(client.isTableEnabled(s)){
					std::cout<<"Table "<<s<<" enabled"<<std::endl;
					acc++;
				}
				else{
					std::cout<<"Table "<<s<<" NOT enabled"<<std::endl;
				}
				return acc;

				};


		int a =forall(table_names.begin(),table_names.end(), 0,f);
		std::cout<<"Velue is "<<a<<std::endl;

	}catch (const TException &tx) {
		std::cerr << "ERROR: " << tx.what() << std::endl;
	}

	return 0;
}
