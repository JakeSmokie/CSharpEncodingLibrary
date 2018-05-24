namespace CSIDETest {
    public enum MasterServerCommands : long {
        // Server is letting master server of its existence.
        ServerMasterChallenge = 5660020,

        // Server is sending some statistics to the master server.
        ServerMasterStatistics = 5660022,

        // Server is sending its info to the launcher.
        ServerLauncherChallenge,

        // Server is telling a launcher that it's ignoring it.
        ServerLauncherIgnoring,

        // Server is telling a launcher that his IP is banned from the server.
        ServerLauncherBanned,

        // Client is trying to create a new account with the master server.
        ClientMasterNewaccount,

        // Client is trying to log in with the master server.
        ClientMasterLogin,

        // [BB] Launcher is querying the master server for a full server list, possibly split into several packets.
        LauncherMasterChallenge,

        // [BB] Server is answering a MasterBanlistVerificationString verification request.
        ServerMasterVerification,

        // [BB] Server is acknowledging the receipt of a ban list.
        ServerMasterBanlistReceipt,
    };
}