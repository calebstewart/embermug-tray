{
  description = "EmberMug Tray - Qt6 CMake project";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        packages.default = pkgs.callPackage ./default.nix { };
        devShells.default = pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } {
          name = "embermug-tray-dev";

          nativeBuildInputs = with pkgs; [
            clang-tools
          ];

          buildInputs =
            with pkgs;
            [
              clang-tools
              cmake
              ninja
              pkg-config
            ]
            ++ (with qt6; [
              qtbase
              qttools
              qtwayland
              qtconnectivity
              qtsvg
            ]);

          shellHook = ''
            echo "EmberMug Tray devshell activated (clang)"
          '';
        };
      }
    )
    // {
      homeManagerModules.default =
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
        };
    };
}
