{
  description = "Development environment for EndField Factory";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs {
          inherit system;
        };
      in
      {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            or-tools
            gnumake
            clang
            bear
          ];

          shellHook = ''
            export ORTOOLS_DIR=${pkgs.or-tools}
            echo "EndField Factory dev shell loaded with or-tools"
          '';
        };
      }
    );
}
