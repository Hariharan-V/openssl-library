#ifndef TRUSTED_AUTHORITY_HH_
#define TRUSTED_AUTHORITY_HH_


/**;
	 One stop shopping for server information
	 Client can ask whether anything is up to date, and if so

 */
class TrustedAuthority {
private:
	KeyPair thisServer; // for authenticating this server
	class ServerRecord {
	public:
		string name;       // url of server
		string kpub;       // public key for the server
		Datestamp issued;  // date when the key was issued
		Datestamp expired; // date when the key expires
		uint64_t ip;       // ip address(es) of the server
	};
	vector<ServerRecord> servers;
public:
	TrustedAuthority() {}
	bool changedSinceTimestamp(const Timestamp& t);
	void downloadChanges(Repository& r);
	
};
#endif
