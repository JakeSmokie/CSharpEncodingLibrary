using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Text;
using System.Threading.Tasks;
using EncodeLibrary.Huffman;
using Xunit;

namespace Tests {
    public class HuffmanCodeTests {
        [Fact]
        public static void CheckCorruptionOnGuids() {
            const int testsAmount = 100;

            var random = new Random();
            var treeBuilder = new HuffmanTreeBuilder();
            
            Parallel.For(0, testsAmount, i => {
                var data = Guid.NewGuid().ToByteArray();
                var table = treeBuilder.CreateTable(data);
                var codec = new HuffmanCodec(table);

                var encodedData = codec.Encode(data);
                var decodedData = codec.Decode(encodedData);
                
                Assert.Equal(data, decodedData);
            });
        }

        [Fact]
        public void CheckCorruptionOnRealText() {
            var text = File.ReadAllText("../../../poem.txt", Encoding.UTF8);
            var data = Encoding.UTF8.GetBytes(text);

            var treeBuilder = new HuffmanTreeBuilder();
            var table = treeBuilder.CreateTable(data);
            var codec = new HuffmanCodec(table);

            var encodedData = codec.Encode(data);
            var decodedData = codec.Decode(encodedData);

            Assert.Equal(data, decodedData);
        }

        [Fact]
        public void CheckCompressionOnRealText()
        {
            var text = File.ReadAllText("../../../poem.txt", Encoding.UTF8);
            var data = Encoding.UTF8.GetBytes(text);

            var treeBuilder = new HuffmanTreeBuilder();
            var table = treeBuilder.CreateTable(data);
            var codec = new HuffmanCodec(table);

            var encodedData = codec.Encode(data);

            Assert.True(encodedData.Length < data.Length);
        }
    }
}