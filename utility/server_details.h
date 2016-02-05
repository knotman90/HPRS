/*
 * TimeStep.h
 *
 *  Created on: Jan 7, 2016
 *      Author: knotman
 */

#ifndef VELASSCO_PARAMS_H_
#define VELASSCO_PARAMS_H_
#include <string.h>

namespace HPRS {

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


}

#endif /* VELASSCO_PARAMS_H_ */
