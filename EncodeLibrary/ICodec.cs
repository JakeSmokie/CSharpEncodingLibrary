namespace EncodeLibrary {
    public interface ICodec {
        byte[] Decode(byte[] data);
        byte[] Encode(byte[] data);
    }
}