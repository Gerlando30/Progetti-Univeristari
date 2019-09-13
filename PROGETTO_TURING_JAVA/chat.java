import java.net.InetAddress;
/**
 * 
 * @author GERLANDO GRAMAGLIA 530269
 *
 */
public class chat {
	private InetAddress address; //indirizzo multicast
	private int port; //porta associata
	
	public chat (InetAddress address, int port) {
		this.setAddress(address);
		this.setPort(port);
	}

	public int getPort() {
		return port;
	}

	public void setPort(int port) {
		this.port = port;
	}

	public InetAddress getAddress() {
		return address;
	}

	public void setAddress(InetAddress address) {
		this.address = address;
	}
}
