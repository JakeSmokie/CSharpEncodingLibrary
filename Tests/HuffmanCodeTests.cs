using System;
using System.Text;
using Xunit;

namespace Tests {
    public class HuffmanCodeTests {
        private const int TestsAmount = 100;

        [Fact]
        public void CheckDataIsNotCorrupted() {
            for (var i = 0; i < TestsAmount; i++) {
                var randomString = Guid.NewGuid().ToString();
                var bytes = Encoding.UTF8.GetBytes(randomString);

            }
        }
    }
}