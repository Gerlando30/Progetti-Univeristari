import java.rmi.Remote;
import java.rmi.RemoteException;
import java.util.ArrayList;
import java.util.Vector;

public interface UserInt extends Remote {
	
	//insieme di metodi con operazioni elementari (poiché intuitivi è sufficiente leggere i metodi nella classe User per capire il loro funzionamento)
	
	public void add_doc(String doc) throws RemoteException;
	public ArrayList<String> getListaDocumenti()  throws RemoteException;
	public void addMessaggioPendente (String msg) throws RemoteException;
	public Vector<String> getMsg () throws RemoteException;
	public void removeMsg ()  throws RemoteException;
	public String getPassword() throws RemoteException;

}
