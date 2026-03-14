{ self }:
{ config, lib, pkgs, ... }:
let
  cfg = config.services.embermug-tray;
in
{
  options.services.embermug-tray = {
    enable = lib.mkEnableOption "EmberMug Tray system tray application";

    package = lib.mkPackageOption pkgs "embermug-tray" {
      default = self.packages.${pkgs.stdenv.hostPlatform.system}.default;
    };
  };

  config = lib.mkIf cfg.enable {
    home.packages = [ cfg.package ];

    systemd.user.services.embermug-tray = {
      Unit = {
        Description = "EmberMug Tray application";
        PartOf = [ "graphical-session.target" ];
        After = [ "graphical-session.target" ];
      };

      Service = {
        Type = "simple";
        ExecStart = "${cfg.package}/bin/embermug-tray";
        Restart = "on-failure";
        RestartSec = 5;
      };

      Install = {
        WantedBy = [ "graphical-session.target" ];
      };
    };
  };
}
