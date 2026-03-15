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
        packages.default = pkgs.callPackage ./nix { };
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
              clazy
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
      homeManagerModules.default = import ./nix/hm-module.nix { inherit self; };
    };
}
