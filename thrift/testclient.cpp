#include <poll.h>
#include <iostream>
#include <string.h>
#include <vector>

#include <boost/lexical_cast.hpp>

#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include <Hbase.h>


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::hadoop::hbase::thrift;

typedef std::vector<std::string> StrVec;
typedef std::map<std::string,std::string> StrMap;
typedef std::vector<ColumnDescriptor> ColVec;
typedef std::map<std::string,ColumnDescriptor> ColMap;
typedef std::vector<TCell> CellVec;
typedef std::map<std::string,TCell> CellMap;

/* The function to print rows */
static void printRow(const std::vector<TRowResult> &);

/* The function to print versions */
static void printVersions(const std::string &row, const CellVec &);

int main(int argc, char** argv)
{
    /* Connection to the Thrift Server */
    boost::shared_ptr<TSocket> socket(new TSocket("localhost", 9090));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

    /* Create the Hbase client */
    HbaseClient client(protocol);

    try {
        transport->open();
        std::string t("demo_table");

        /* Scan all tables, look for the demo table and delete it. */
        std::cout << "scanning tables..." << std::endl;
        StrVec tables;
        client.getTableNames(tables);
        for (StrVec::const_iterator it = tables.begin(); it != tables.end(); ++it) {
            std::cout << " found: " << *it << std::endl;
            if (t == *it) {
                if (client.isTableEnabled(*it)) {
                    std::cout << " disabling table: " << *it << std::endl;
                    client.disableTable(*it);
                }
                std::cout << " deleting table: " << *it << std::endl;
                client.deleteTable(*it);
            }
        }

	/* Create the demo table with two column families, entry: and unused: */
	ColVec columns;
	StrMap attr;
	columns.push_back(ColumnDescriptor());
	columns.back().name = "entry:";
	columns.back().maxVersions = 10;
	columns.push_back(ColumnDescriptor());
	columns.back().name = "unused:";
	std::cout << "creating table: " << t << std::endl;
	try {
		client.createTable(t, columns);
	} catch (const AlreadyExists &ae) {
		std::cerr << "WARN: " << ae.message << std::endl;
	}

	ColMap columnMap;
	client.getColumnDescriptors(columnMap, t);
	std::cout << "column families in " << t << ": " << std::endl;
	for (ColMap::const_iterator it = columnMap.begin(); it != columnMap.end(); ++it) {
		std::cout << " column: " << it->second.name << ", maxVer: " << it->second.maxVersions << std::endl;
	}

	/* Test UTF-8 handling */
	std::string invalid("foo-\xfc\xa1\xa1\xa1\xa1\xa1");
	std::string valid("foo-\xE7\x94\x9F\xE3\x83\x93\xE3\x83\xBC\xE3\x83\xAB");

	/* Non-utf8 is fine for data */
	std::vector<Mutation> mutations;
	mutations.push_back(Mutation());
	mutations.back().column = "entry:foo";
	mutations.back().value = invalid;
	client.mutateRow(t, "foo", mutations, attr);

	/* Trying empty strings is not valid
 * 	mutations.clear();
 * 		mutations.push_back(Mutation());
 * 			mutations.back().column = "entry:";
 * 				mutations.back().value = "";
 * 					client.mutateRow(t, "", mutations, attr); */

	/* This row name is valid utf8 */
	mutations.clear();
	mutations.push_back(Mutation());
	mutations.back().column = "entry:foo";
	mutations.back().value = valid;
	client.mutateRow(t, valid, mutations, attr);

	/* Non-utf8 is now allowed in row names because HBase stores values as binary */
	mutations.clear();
	mutations.push_back(Mutation());
	mutations.back().column = "entry:foo";
	mutations.back().value = invalid;
	client.mutateRow(t, invalid, mutations, attr);

	/* Run a scanner on the rows we just created */
	StrVec columnNames;
	columnNames.push_back("entry:");
	std::cout << "Starting scanner..." << std::endl;
	int scanner = client.scannerOpen(t, "", columnNames, attr);
	try {
		while (true) {
			std::vector<TRowResult> value;
			client.scannerGet(value, scanner);
			if (value.size() == 0)
				break;
			printRow(value);
		}
	} catch (const IOError &ioe) {
		std::cerr << "FATAL: Scanner raised IOError" << std::endl;
	}

	client.scannerClose(scanner);
	std::cout << "Scanner finished" << std::endl;

	/* Run some operations on a bunch of rows */
	for (int i = 0; i <= 11; i++) {
	    /* Format row keys as "00000" to "00100" */
	    char buf[32];
	    sprintf(buf, "%05d", i);
	    std::string row(buf);
		
	    std::vector<TRowResult> rowResult;
		
	    mutations.clear();
	    mutations.push_back(Mutation());
	    mutations.back().column = "unused:";
	    mutations.back().value = "DELETE_ME";
		
	    client.mutateRow(t, row, mutations, attr);
	    client.getRow(rowResult, t, row, attr);
	    printRow(rowResult);
	    client.deleteAllRow(t, row, attr);
		
	    mutations.clear();
	    mutations.push_back(Mutation());
	    mutations.back().column = "entry:num";
	    mutations.back().value = "0";
	    mutations.push_back(Mutation());
	    mutations.back().column = "entry:foo";
	    mutations.back().value = "FOO";
	    client.mutateRow(t, row, mutations, attr);
	    client.getRow(rowResult, t, row, attr);
	    printRow(rowResult);
		
	    /* Sleep to force later timestamp */
	    poll(0, 0, 50);
		
	    mutations.clear();
	    mutations.push_back(Mutation());
	    mutations.back().column = "entry:foo";
	    mutations.back().isDelete = true;
	    mutations.push_back(Mutation());
	    mutations.back().column = "entry:num";
	    mutations.back().value = "-1";
	    client.mutateRow(t, row, mutations, attr);
	    client.getRow(rowResult, t, row, attr);
	    printRow(rowResult);
		
	    mutations.clear();
	    mutations.push_back(Mutation());
	    mutations.back().column = "entry:num";
	    mutations.back().value = boost::lexical_cast<std::string>(i);
	    mutations.push_back(Mutation());
	    mutations.back().column = "entry:sqr";
	    mutations.back().value = boost::lexical_cast<std::string>(i*i);
	    client.mutateRow(t, row, mutations, attr);
	    client.getRow(rowResult, t, row, attr);
	    printRow(rowResult);
		
	    mutations.clear();
	    mutations.push_back(Mutation());
	    mutations.back().column = "entry:num";
	    mutations.back().value = "-999";
	    mutations.push_back(Mutation());
	    mutations.back().column = "entry:sqr";
	    mutations.back().isDelete = true;
	    client.mutateRowTs(t, row, mutations, 1, attr); /* Shouldn't override latest */
	    client.getRow(rowResult, t, row, attr);
	    printRow(rowResult);
		
	    CellVec versions;
	    client.getVer(versions, t, row, "entry:num", 10, attr);
	    printVersions(row, versions);
	    assert(versions.size());
	    std::cout << std::endl;
		
	    try {
	        std::vector<TCell> value;
	        client.get(value, t, row, "entry:foo", attr);
	        if (value.size()) {
	            std::cerr << "FATAL: shouldn't get here!" << std::endl;
	            return -1;
	        }
	    } catch (const IOError &ioe) {
	        /* Blank */
	    }
	}

	/* Scan all rows/columns */
	columnNames.clear();
	client.getColumnDescriptors(columnMap, t);
	std::cout << "The number of columns: " << columnMap.size() << std::endl;
	for (ColMap::const_iterator it = columnMap.begin(); it != columnMap.end(); ++it) {
	    std::cout << " column with name: " + it->second.name << std::endl;
	    columnNames.push_back(it->second.name);
	}
	std::cout << std::endl;

	std::cout << "Starting scanner..." << std::endl;
	scanner = client.scannerOpenWithStop(t, "00020", "00040", columnNames, attr);
	try {
	    while (true) {
	        std::vector<TRowResult> value;
	        client.scannerGet(value, scanner);
	        if (value.size() == 0)
	            break;
	        printRow(value);
	    }
	} catch (const IOError &ioe) {
	    std::cerr << "FATAL: Scanner raised IOError" << std::endl;
	}

	client.scannerClose(scanner);
	std::cout << "Scanner finished" << std::endl;
	transport->close();
    } catch (const TException &tx) {
        std::cerr << "ERROR: " << tx.what() << std::endl;
    }
}

/* The function to print rows */
static void printRow(const std::vector<TRowResult> &rowResult)
{
    for (size_t i = 0; i < rowResult.size(); i++) {
        std::cout << "row: " << rowResult[i].row << ", cols: ";
        for (CellMap::const_iterator it = rowResult[i].columns.begin();it != rowResult[i].columns.end(); ++it) {
            std::cout << it->first << " => " << it->second.value << "; ";
        }
        std::cout << std::endl;
    }
}

/* The function to print versions */
static void printVersions(const std::string &row, const CellVec &versions)
{
    std::cout << "row: " << row << ", values: ";
    for (CellVec::const_iterator it = versions.begin(); it != versions.end(); ++it) {
        std::cout << (*it).value << "; ";
    }
    std::cout << std::endl;
}
