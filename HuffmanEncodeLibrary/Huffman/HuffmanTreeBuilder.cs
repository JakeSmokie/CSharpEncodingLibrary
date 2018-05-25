using System.Collections.Generic;
using System.Linq;

namespace EncodeLibrary.Huffman {
    public class HuffmanTreeBuilder {
        private class Node {
            public int Amount;
            public byte? Value;
            public Node[] Childs;

            public override string ToString() {
                return $"{nameof(Amount)}: {Amount}, {nameof(Value)}: {(char?) Value}";
            }
        }

        public byte[] CreateTableFromDataset(byte[] data) {
            var nodes = CountSymbols(data);

            BuildTree(nodes);
            var table = CreateTable(nodes.Last());

            return table;
        }

        private static byte[] CreateTable(Node node) {
            var table = new List<byte>();
            TraverseTree(node, table);

            return table.ToArray();
        }

        private static void TraverseTree(Node node, ICollection<byte> table) {
            if (node.Value != null) {
                table.Add((byte) node.Value);
                return;
            }

            table.Add((byte) ((node.Childs[0].Value is null ? 0 : 1) | (node.Childs[1].Value is null ? 0 : 2)));

            for (var i = 0; i < 2; i++) {
                if (node.Childs[i] != null)
                    TraverseTree(node.Childs[i], table);
            }
        }

        private static List<Node> CountSymbols(IEnumerable<byte> data) {
            return data.GroupBy(b => b)
                .Select(symbol => new Node {
                    Value = symbol.Key,
                    Amount = symbol.Count()
                })
                .ToList();
        }

        private static void BuildTree(ICollection<Node> nodes) {
            for (var i = 0; i < nodes.Count - 1; i += 2) {
                var leastAmountNodes = nodes.OrderBy(n => n.Amount).Skip(i).Take(2).ToArray();

                nodes.Add(new Node {
                    Childs = leastAmountNodes,
                    Amount = leastAmountNodes.Sum(n => n.Amount)
                });
            }
        }
    }
}