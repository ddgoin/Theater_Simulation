import java.util.concurrent.Semaphore;

class Globals{
	public int i = 2;
	public int[] arr;
}

class Project2{
	public static void main(String[] args){
		Globals g = new Globals();
		g.arr = new int[10];
		for(int x = 0; x < 10; x++){
			g.arr[x] = g.i + x;
		}

		System.out.println(g.i + " and " + g.arr[9]);
	}
}

/*

class thread_customer extends Thread
{
	private int custnr;

	public thread_customer(int i)
}

public static void main(String [] args){

	final int total_customer = 3;

	System.out.println("Busy waiting…");

	//Semaphore(int permits, boolean fair)
	//Semaphore sem = new Semaphore(total_customer, true);

	thread_customer[] t_cust = new thread_customer2[total_customer];

	for (int i = 0; i < total_customer; i++)
	{
		p[i] = new Process2(i);
		p[i].start();
	}
} */