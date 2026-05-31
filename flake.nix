{
  description = "A cpp development enviroment! Refer more to https://github.com/BruceChanJianLe/nix-cpp-project";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs = { self, nixpkgs }:
    let
      systems = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];
      forAllSystems = f: nixpkgs.lib.genAttrs systems (system: f {
        pkgs = import nixpkgs { inherit system; };
      });
    in
    {
      devShells = forAllSystems ({ pkgs }: {
        default = pkgs.mkShell {
          buildInputs = [
            pkgs.gcc
            pkgs.cmake
            pkgs.llvmPackages.clang
            pkgs.gdb
          ];

        shellHook = ''
          echo "--- C++ Development Environment ---"
          echo "GCC version: $(gcc --version | head -n 1)"
          echo "Clang version: $(clang --version | head -n 1)"
          echo "CMake version: $(cmake --version | head -n 1)"
          echo "GDB version: $(gdb --version | head -n 1)"
          '';
        };
      });
    };
}
