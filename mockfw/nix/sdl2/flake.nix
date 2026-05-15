{
  description = "SDL2 dev shell";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
  let
    system = "x86_64-linux";
    pkgs = import nixpkgs { inherit system; };
  in {
    devShells.${system}.default = pkgs.mkShell {
      buildInputs = [
        pkgs.SDL2
        pkgs.SDL2.dev
        pkgs.SDL2_ttf
      ];

      shellHook = ''
        export SDL2=${pkgs.SDL2}
        export SDL2_inc=${pkgs.SDL2.dev}
        export SDL2_ttf=${pkgs.SDL2_ttf}
      '';
    };
  };
}