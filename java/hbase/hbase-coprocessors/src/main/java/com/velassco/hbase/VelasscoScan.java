package com.velassco.hbase;

import java.io.IOException;
import java.util.Map;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.hbase.HBaseConfiguration;
import org.apache.hadoop.hbase.TableName;
import org.apache.hadoop.hbase.client.Connection;
import org.apache.hadoop.hbase.client.ConnectionFactory;
import org.apache.hadoop.hbase.client.HBaseAdmin;
import org.apache.hadoop.hbase.client.HTable;
import org.apache.hadoop.hbase.client.Result;
import org.apache.hadoop.hbase.client.ResultScanner;
import org.apache.hadoop.hbase.client.Scan;
import org.apache.hadoop.hbase.client.Table;
import org.apache.hadoop.hbase.client.coprocessor.Batch;
import org.apache.hadoop.hbase.ipc.BlockingRpcCallback;
import org.apache.hadoop.hbase.util.Bytes;

import com.velassco.hbase.coprocessors.RowCountRPC;
public class VelasscoScan {


	static void scanTable(){
		// Instantiating Configuration class
		Configuration config = HBaseConfiguration.create();
		System.out.println(config);
		

		// Instantiating HTable class
		HTable table;
	
		try {
			table = new HTable(config, "Simulations_Data");


			// Instantiating the Scan class
			Scan scan = new Scan();
			scan.addFamily(Bytes.toBytes("M"));
			// Scanning the required columns

			// Getting the scan result
			ResultScanner scanner = table.getScanner(scan);
			// Reading values from scan result
			for (Result result = scanner.next(); result != null; result = scanner.next())

				System.out.println("Found row : " + result);
			//closing the scanner
			scanner.close();
			
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	
	static void  testRowCountCoprocessor(String tablename) throws Throwable{
		Configuration config = HBaseConfiguration.create();
		Connection connection = ConnectionFactory.createConnection(config);
		Table table = connection.getTable(TableName.valueOf(tablename));
		
	
		
		final RowCountRPC.CountRequest request = RowCountRPC.CountRequest.getDefaultInstance();
		Map<byte[],Long> results = table.coprocessorService(
				RowCountRPC.RowCountService.class, // the protocol interface we're invoking
		    null, null,                          // start and end row keys
		    new Batch.Call<RowCountRPC.RowCountService,Long>() {
		        public Long call(RowCountRPC.RowCountService counter) throws IOException {
		          BlockingRpcCallback<RowCountRPC.CountResponse> rpcCallback =
		              new BlockingRpcCallback<RowCountRPC.CountResponse>();
		          counter.getRowCount(null, request, rpcCallback);
		          RowCountRPC.CountResponse response = rpcCallback.get();
		          return response.hasCount() ? response.getCount() : 0;
		        }
		    });
		for (byte[] key : results.keySet()) {
			System.out.println(results.get(key));
		}
	}
	


	public static void main(String[] args) {
		try {
			VelasscoScan.testRowCountCoprocessor("Simulations_Data");
		} catch (Throwable e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		//VelasscoScan.scanTable();
	}
}
