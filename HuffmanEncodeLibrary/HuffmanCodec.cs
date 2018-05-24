using System.Collections.Generic;
using BitStreams;

namespace HuffmanEncodeLibrary {
    public class HuffmanCodec {
        public static readonly byte[] SkulltagCompatibleHuffmanTree = {
            0, 0, 0, 1, 128, 0, 0, 0, 3, 38, 34, 2, 1, 80, 3, 110,
            144, 67, 0, 2, 1, 74, 3, 243, 142, 37, 2, 3, 124, 58, 182, 0,
            0, 1, 36, 0, 3, 221, 131, 3, 245, 163, 1, 35, 3, 113, 85, 0,
            1, 41, 1, 77, 3, 199, 130, 0, 1, 206, 3, 185, 153, 3, 70, 118,
            0, 3, 3, 5, 0, 0, 1, 24, 0, 2, 3, 198, 190, 63, 2, 3,
            139, 186, 75, 0, 1, 44, 2, 3, 240, 218, 56, 3, 40, 39, 0, 0,
            2, 2, 3, 244, 247, 81, 65, 0, 3, 9, 125, 3, 68, 60, 0, 0,
            1, 25, 3, 191, 138, 3, 86, 17, 0, 1, 23, 3, 220, 178, 2, 3,
            165, 194, 14, 1, 0, 2, 2, 0, 0, 2, 1, 208, 3, 150, 157, 181,
            1, 222, 2, 3, 216, 230, 211, 0, 2, 2, 3, 252, 141, 10, 42, 0,
            2, 3, 134, 135, 104, 1, 103, 3, 187, 225, 95, 32, 0, 0, 0, 0,
            0, 0, 1, 57, 1, 61, 3, 183, 237, 0, 0, 3, 233, 234, 3, 246,
            203, 2, 3, 250, 147, 79, 1, 129, 0, 1, 7, 3, 143, 136, 1, 20,
            3, 179, 148, 0, 0, 0, 3, 28, 106, 3, 101, 87, 1, 66, 0, 3,
            180, 219, 3, 227, 241, 0, 1, 26, 1, 251, 3, 229, 214, 3, 54, 69,
            0, 0, 0, 0, 0, 3, 231, 212, 3, 156, 176, 3, 93, 83, 0, 3,
            96, 253, 3, 30, 13, 0, 0, 2, 3, 175, 254, 94, 3, 159, 27, 2,
            1, 8, 3, 204, 226, 78, 0, 0, 0, 3, 107, 88, 1, 31, 3, 137,
            169, 2, 2, 3, 215, 145, 6, 4, 1, 127, 0, 1, 99, 3, 209, 217,
            0, 3, 213, 238, 3, 177, 170, 1, 132, 0, 0, 0, 2, 3, 22, 12,
            114, 2, 2, 3, 158, 197, 97, 45, 0, 1, 46, 1, 112, 3, 174, 249,
            0, 3, 224, 102, 2, 3, 171, 151, 193, 0, 0, 0, 3, 15, 16, 3,
            2, 168, 1, 49, 3, 91, 146, 0, 1, 48, 3, 173, 29, 0, 3, 19,
            126, 3, 92, 242, 0, 0, 0, 0, 0, 0, 3, 205, 192, 2, 3, 235,
            149, 255, 2, 3, 223, 184, 248, 0, 0, 3, 108, 236, 3, 111, 90, 2,
            3, 117, 115, 71, 0, 0, 3, 11, 50, 0, 3, 188, 119, 1, 122, 3,
            167, 162, 1, 160, 1, 133, 3, 123, 21, 0, 0, 2, 1, 59, 2, 3,
            155, 154, 98, 43, 0, 3, 76, 51, 2, 3, 201, 116, 72, 2, 0, 2,
            3, 109, 100, 121, 2, 3, 195, 232, 18, 1, 0, 2, 0, 1, 164, 2,
            3, 120, 189, 73, 0, 1, 196, 3, 239, 210, 3, 64, 62, 89, 0, 0,
            1, 33, 2, 3, 228, 161, 55, 2, 3, 84, 152, 47, 0, 0, 2, 3,
            207, 172, 140, 3, 82, 166, 0, 3, 53, 105, 1, 52, 3, 202, 200
        };

        private readonly byte[] _huffmanTree;
        private readonly HuffmanNode[] _codeTable;
        private HuffmanNode _root;

        public HuffmanCodec(byte[] huffmanTree) {
            _codeTable = new HuffmanNode[256];
            _huffmanTree = huffmanTree;

            CreateCodeTable();
        }

        private void CreateCodeTable() {
            _root = new HuffmanNode {
                Value = -1,
                Code = new List<Bit>()
            };

            BuildTree(_root, 0);
        }

        private int BuildTree(HuffmanNode node, int index) {
            if (index >= _huffmanTree.Length)
                return -1;

            var branchDescription = _huffmanTree[index];

            index += 1;
            node.Childs = new HuffmanNode[2];

            for (var i = 0; i < node.Childs.Length; i++) {
                node.Childs[i] = new HuffmanNode {
                    Value = -1,
                    Code = new List<Bit>(node.Code)
                };

                node.Childs[i].Code.Add(i);

                if ((branchDescription & (1 << i)) == 0) {
                    index = BuildTree(node.Childs[i], index);

                    if (index < 0)
                        return -1;

                    continue;
                }

                if (index >= _huffmanTree.Length)
                    return -1;

                node.Childs[i].Value = _huffmanTree[index];
                node.Childs[i].Childs = null;

                _codeTable[node.Childs[i].Value] = node.Childs[i];
                index += 1;
            }

            return index;
        }

        public byte[] Encode(byte[] data) {
            var bitStream = new BitStream(new byte[] { });

            foreach (var b in data) {
                var node = _codeTable[b];

                bitStream.ChangeLength(bitStream.Length + node.Code.Count);
                bitStream.WriteBits(node.Code);
            }

            return bitStream.GetStreamData();
        }

        public byte[] Decode(byte[] data) {
            var node = _root;
            var bitStream = new BitStream(data);
            var output = new List<byte>();

            long symbolsRead = 0;

            while (symbolsRead < bitStream.Length) {
                var index = bitStream.ReadBit().AsInt();
                symbolsRead += 1;

                node = node.Childs[index];

                if (node.Childs != null)
                    continue;

                output.Add((byte) (node.Value & 0xFF));
                node = _root;
            }

            return output.ToArray();
        }
    }
}