
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Vector;
/**
 * Classe che rappresenta un Documento
 * @author GERLANDO GRAMAGLIA 530269
 *
 */
public class Documento {
	
	private String PathDocumento;
	private String NomeDocumento;
	private String Creatore; 
	private int numero_sezioni; 
	private int editing; //numero di utenti che stanno editando il documento
	private Vector<String> collaboratori; //lista utenti che possono effettuare l'editing sul documento
	private ArrayList<Integer> sezioni_occupate; //l'i-esima posizione indica se l'i-esima sezione è in fase di editing da un utente
	
	//CONTROLLO DA FARE DA QUALCHE PARTE: DOCUMENTI CON STESSO NOME!
	public Documento (String creatore, String NomeDocumento, int num_sezioni)  {
		this.Creatore = creatore;
		this.numero_sezioni = num_sezioni;
		this.NomeDocumento = NomeDocumento;
		this.PathDocumento = "C:\\DOCUMENTI-TURING\\"+NomeDocumento;
		this.sezioni_occupate = new ArrayList<Integer>(num_sezioni);
		for (int i=0; i<num_sezioni; i++)
			this.sezioni_occupate.add(i, 0);
		
		this.editing = 0;
		this.collaboratori = new Vector<String>();
		this.collaboratori.add(creatore);
		if (!Files.exists(Paths.get(this.PathDocumento))) { //crea il documento come directory
			System.out.println("creazione del documento "+ this.PathDocumento);
			try {
				Files.createDirectories(Paths.get(this.PathDocumento));
			} catch (IOException e) {
				System.out.println("Impossibile creare directory");
			}
			for (int i = 0; i<num_sezioni; i++) { //crea sezioni come file .txt
				String n = this.PathDocumento + "\\sezione"+i+".txt";
				try {
					Files.createFile(Paths.get(n));
				} catch (IOException e) {
					System.out.println("Impossibile creare sezione");
				}
			}
		}
	}
	
	
	/**
	 * 
	 * @param Collabolatore nome utente che fa richiesta di editing di una sezione
	 * @param sezione numero della sezione richiesta
	 * @return path della sezione se gli viene concesso l'editing, null altrimenti
	 */
	//COME RESETTIAMO A ZERO SE IL CLIENT CHIUDE BRUSCAMENTE????
	public synchronized String locksezione (String Collabolatore, int sezione)  {
		if (!this.collaboratori.contains(Collabolatore) || (this.numero_sezioni <= sezione) || sezione < 0)
			return null;
		if (this.sezioni_occupate.get(sezione) == 1)
			return null;
		
		this.sezioni_occupate.set(sezione, 1);
		return this.PathDocumento+"\\sezione"+sezione+".txt";
	}
	/**
	 * 
	 * @param sezione numero della sezione che non è più in fase di editing
	 * @return true se la sezione esiste e viene liberata, false altrimenti
	 */
	public synchronized boolean unlocksezione (int sezione) {
		if (this.numero_sezioni > sezione && sezione >= 0) {
			this.sezioni_occupate.set(sezione, 0);
			return true;
		}
		return false;
	}
	
	/**
	 * 
	 * @param creatore nome del creatore del documento
	 * @param collaboratore che si vuole aggiungere alla lista degli utenti che possono effettuare l'editing
	 * @return true se il collaboratore viene aggiunto correttamente, false altriementi
	 */
	//MANCA LA PARTE DI NOTIFICA DA PARTE DEL CREATORE AL COLLABORATORE
	public synchronized boolean invitaCollaboratore (String creatore, String collaboratore) {
		if (creatore.equals(this.Creatore)) {
			if (!this.collaboratori.contains(collaboratore)) {
				this.collaboratori.add(collaboratore); 
				return true;
			}
		}
		return false;
	}
	
	/**
	 * 
	 * @return lista dei collaboratori sottoforma di stringa
	 */
	public synchronized String getCollaboratori () {
		String Collaboratori = "Collaboratori: ";
		for (int i = 0; i<this.collaboratori.size(); i++)
			Collaboratori += this.collaboratori.get(i) + " ";
		return Collaboratori;
	}
	/**
	 * 
	 * @param val valore di cui incrementiamo/decrementiamo il numero di utenti che stanno editando il documento
	 */
	public void incEditing(int val) {
		this.editing += val;
	}
	public String getCreatore () {
		return this.Creatore;
	}
	public int getNumero_sezioni () {
		return this.numero_sezioni;
	}
	public String getNomeDocumento() {
		return this.NomeDocumento;
	}
	
	public String getPathSezione (int sezione) {
		if (this.numero_sezioni>sezione) 
			return this.PathDocumento+"\\sezione"+sezione+".txt";
		return null;
	}

	public String getPathDocumento() {
		return this.PathDocumento;
	}

	public int getEditing() {
		return editing;
	}

	public ArrayList<Integer> getSezioni_Occupate () {
		return new ArrayList<Integer>(sezioni_occupate);
	}
}
