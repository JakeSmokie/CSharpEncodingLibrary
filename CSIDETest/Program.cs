using System.IO;
using System.Text;
using HuffmanEncodeLibrary;

namespace CSIDETest
{
    internal class Program {
        static void Main(string[] args) {
            var fetcher = new MasterServerFetcher();

            var poem = File.ReadAllText("poem.txt", Encoding.UTF8);
            var bytes = Encoding.UTF8.GetBytes(poem);

            var huffmanTreeBuilder = new HuffmanTreeBuilder();
            var tableFromDataset = huffmanTreeBuilder.CreateTableFromDataset(bytes);

            var huffmanCodec = new HuffmanCodec(tableFromDataset);

            var codedMessage = huffmanCodec.Encode(bytes);

            var decodedMessage = huffmanCodec.Decode(codedMessage);
            var outString = Encoding.UTF8.GetString(decodedMessage);
        }
    }
}