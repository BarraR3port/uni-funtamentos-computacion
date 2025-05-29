{ pkgs }: {
	deps = [
		pkgs.mpi
  pkgs.python39Packages.pip
  pkgs.clang_12
		pkgs.ccls
		pkgs.gdb
		pkgs.gnumake
	];
}