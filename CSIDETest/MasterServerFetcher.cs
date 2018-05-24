using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;

namespace CSIDETest {
    public class MasterServerFetcher : IMasterServerFetcher {
        public IEnumerable<IPEndPoint> FetchServerList(IEnumerable<(string host, int port)> masterServers) {
            return masterServers.SelectMany(FetchServerListFromSingleMaster);
        }

        private static IEnumerable<IPEndPoint> FetchServerListFromSingleMaster((string host, int port) server) {
            var client = new UdpClient(server.host, server.port);


            return null;
        }
    }
}