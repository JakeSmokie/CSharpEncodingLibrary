using System.Collections.Generic;
using System.Net;

namespace CSIDETest {
    public interface IMasterServerFetcher {
        IEnumerable<IPEndPoint> FetchServerList(IEnumerable<(string host, int port)> masterServers);
    }
}