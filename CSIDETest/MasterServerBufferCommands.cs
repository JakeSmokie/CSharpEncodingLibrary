namespace CSIDETest
{
    public enum MasterServerBufferCommands : long {
        BeginServerList,
        Server,
        EndServerList,
        IpBanned,
        RequestIgnored,
        WrongVersion,
        BeginServerListPart,
        EndServerListPart,
        ServerBlock,
    }
}