import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;
import java.util.Hashtable;
/**
 * utilizzata dal client per potersi registrare al servizio e dal server per controllare se un utente è registrato
 * @author GERLANDO GRAMAGLIA 530269
 *
 */
public class RegistrazioneImpl extends UnicastRemoteObject implements Registrazione{
	private static final long serialVersionUID = 1L;
	private Hashtable<String, User> tab_clienti_reg;
	
	public RegistrazioneImpl () throws RemoteException {
		tab_clienti_reg = new Hashtable<String, User>();
	}
	//"synchronized" non va messo nel metodo definito nell'interfaccia perchè è un dettaglio implementativo
	public synchronized boolean registrami (String Name, String Password) throws RemoteException {
		
		if (this.tab_clienti_reg.containsKey(Name)) {
			System.out.println("Impossibile registrare "+ Name + " nome utente già utilizzato");
			return false;
		}
		User user = new User (Password);
		this.tab_clienti_reg.put(Name, user);
		System.out.println(Name +" è stato registrato con successo");
		return true;
	}
	
	public synchronized User is_register (String Name, String Password) {
		User user = null;
		if ((user =this.tab_clienti_reg.get(Name)) != null)
			if (user.getPassword().equals(Password)) 
				return user;
		return null;
	}
	public synchronized User is_register (String Name) {
		return this.tab_clienti_reg.get(Name);
		
	}

}
