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

    settings = lib.mkOption {
      type = lib.types.attrsOf (lib.types.attrsOf lib.types.str);
      default = { };
      description = "Settings to write to the embermug-tray QSettings config file.";
      example = lib.literalExpression ''
        {
          device.address = "AA:BB:CC:DD:EE:FF";
          notifications = {
            batteryLow = "15";
            batteryCritical = "5";
          };
        }
      '';
    };
  };

  config = lib.mkIf cfg.enable {
    home.packages = [ cfg.package ];

    xdg.configFile."embermug/tray.conf" = lib.mkIf (cfg.settings != { }) {
      text = lib.generators.toINI { } cfg.settings;
    };

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
