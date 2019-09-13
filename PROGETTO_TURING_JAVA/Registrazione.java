import java.rmi.Remote;
import java.rmi.RemoteException;
/**
 * 
 * @author GERLANDO GRAMAGLIA 530269
 *
 */
public interface Registrazione extends Remote {
	
	/**
	 * @param Name nome cliente da registrare
	 * @param Password del cliente
	 * @return true se ho registrato il cliente correttamente, false altrimenti
	 * @throws RemoteException
	 */
	public boolean registrami (String Name, String Password) throws RemoteException;
	/**
	 * 
	 * @param Name nome dell'utente 
	 * @param Password 
	 * @return oggetto associato all'utente registrato con <Name, Password>, null altrimenti
	 * @throws RemoteException
	 */
	public User is_register (String Name, String Password) throws RemoteException;
	/**
	 * 
	 * @param Name nome dell'utente
	 * @return oggetto associato all'utente registrato con Name, null altrimenti
	 * @throws RemoteException
	 */
	public User is_register (String Name) throws RemoteException;

}
