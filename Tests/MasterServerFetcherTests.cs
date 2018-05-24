using CSIDETest;
using Xunit;

namespace Tests
{
    public class MasterServerFetcherTests {
        private readonly IMasterServerFetcher _fetcher = new MasterServerFetcher();
        private readonly (string host, int port)[] _masterServers = {
            ("master.zandronum.com", 15300)
        };

        [Fact]
        public void CheckThatFetcherReturnsAnyServersFromWorkingMaster() {
            var serverList = _fetcher.FetchServerList(_masterServers);
            Assert.NotEmpty(serverList);
        }
    }
}
