{
  description = "Development environment for AIC Planner";

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

        build-dir = "build";
        executable = "aic_planner";

        configure = pkgs.writeShellScriptBin "configure" ''
          cmake -B ${build-dir} -S . -DCMAKE_BUILD_TYPE=Release
        '';

        build = pkgs.writeShellScriptBin "build" ''
          if [ ! -f ${build-dir}/CMakeCache.txt ]; then
              echo "Configuration not found. Running configure first..."
              configure
          fi
          cmake --build ${build-dir} --parallel
        '';

        clean = pkgs.writeShellScriptBin "clean" ''
          rm -rf ${build-dir}
        '';

        run = pkgs.writeShellScriptBin "run" ''
          if [ ! -f ${build-dir}/bin/${executable} ]; then
              echo "Executable not found. Running build first..."
              build
          fi
          ./${build-dir}/bin/${executable} "$@"
        '';
      in
      {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            # dependencies
            cmake
            pkg-config
            clp
            or-tools
            gnumake
            clang
            fast-cpp-csv-parser
            zlib
            bzip2
            abseil-cpp
            protobuf
            re2
            clp
            cbc
            glpk
            highs
            eigen
            scip

            # custom commands
            configure
            build
            clean
            run
          ];

          shellHook = ''
            export ORTOOLS_DIR=${pkgs.or-tools}
            export CSV_PARSER_DIR=${pkgs.fast-cpp-csv-parser}
          '';
        };
      }
    );
}
