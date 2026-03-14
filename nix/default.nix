{
  lib,
  clangStdenv,
  cmake,
  ninja,
  pkg-config,
  qt6,
}:

clangStdenv.mkDerivation rec {
  pname = "embermug-tray";
  version = "0.2.0";

  src = ./..;

  nativeBuildInputs = [
    cmake
    ninja
    pkg-config
    qt6.wrapQtAppsHook
  ];

  buildInputs = with qt6; [
    qtbase
    qttools
    qtwayland
    qtconnectivity
    qtsvg
  ];

  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=Release"
    "-DCMAKE_INSTALL_PREFIX=${placeholder "out"}"
  ];

  meta = with lib; {
    description = "EmberMug Tray - Qt6 system tray application";
    longDescription = ''
      A system tray application for Linux that integrates with EmberMug
      to provide easy access to mug controls from the system notification area.
    '';
    homepage = "https://github.com/embermug/embermug-tray";
    license = licenses.mit;
    platforms = platforms.linux;
    maintainers = with maintainers; [ ];
  };
}
